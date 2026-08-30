 #include "pch.h"

#include "Material.h"
#include "MaterialInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Lights/LightManager/LightManager.h"

namespace
{
	TextureAtlasCategory GetTextureAtlasCategory(MaterialBlendModel blendModel)
	{
		return blendModel == MaterialBlendModel::Transparent ?
			TextureAtlasCategory::Transparent :
			TextureAtlasCategory::Opaque;
	}
}

Material::Material() :
	IMaterialBase()
{
	engine->GetResourceManager()->AddMaterial(this);
}

Material::Material(const Material* other) :
	IMaterialBase(dynamic_cast<const IMaterialBase*>(other))
{
	renderPassTypeShaderMap_ = other->renderPassTypeShaderMap_;
}

Material::~Material()
{
	for (auto derivedMaterialInstance : derivedMaterialInstances_)
	{
		delete derivedMaterialInstance;
	}

	ClearMaterialTextureProxies();

	delete renderPassTypeShaderMap_[RenderPassType::Forward];
	delete renderPassTypeShaderMap_[RenderPassType::GeometryBuffer];
	delete renderPassTypeShaderMap_[RenderPassType::Shadow];
	delete renderPassTypeShaderMap_[RenderPassType::PointLightShadow];
	delete renderPassTypeShaderMap_[RenderPassType::CubemapCapture];

	delete initializationData_;
}

void Material::ClearMaterialTextureProxies()
{
	for (MaterialTextureProxy& materialTextureProxy : materialTextureProxies_)
	{
		if (materialTextureProxy.image && materialTextureProxy.texture)
		{
			const_cast<Image*>(materialTextureProxy.image)->UnregisterTextureAtlasProxy(materialTextureProxy.texture.get());
		}
	}

	materialTextureProxies_.clear();
}

Texture* Material::GetTextureForShader(const Image* constImage, bool useTextureAtlas)
{
	if (!constImage)
	{
		return nullptr;
	}

	Image* image = const_cast<Image*>(constImage);
	ResourceManager* resourceManager = engine ? engine->GetResourceManager() : nullptr;
	ResourceContainer* resourceContainer = resourceManager ? resourceManager->GetResourceContainer() : nullptr;
	const bool shouldUseTextureAtlas = useTextureAtlas || (resourceContainer && resourceContainer->GetUseTextureAtlasForAllImages());
	const TextureAtlasCategory atlasCategory = GetTextureAtlasCategory(blendModel_);

	if (!shouldUseTextureAtlas)
	{
		Texture* standaloneTexture = image->GetOrCreateGeneratedTexture();
		standaloneTexture->SetWaitsForTextureAtlas(false);
		return standaloneTexture;
	}

	image->SetCanUseTextureAtlas(true);
	bool registeredToAtlas = false;
	if (resourceContainer)
	{
		registeredToAtlas = resourceContainer->RegisterImageToTextureAtlas(image, atlasCategory);
	}

	if (!registeredToAtlas)
	{
		Texture* standaloneTexture = image->GetOrCreateGeneratedTexture();
		standaloneTexture->SetWaitsForTextureAtlas(false);
		return standaloneTexture;
	}

	MaterialTextureProxy materialTextureProxy;
	materialTextureProxy.image = image;
	materialTextureProxy.texture = std::unique_ptr<Texture>(new Texture());

	Texture* texture = materialTextureProxy.texture.get();
	if (!image->GetName().empty())
	{
		texture->SetName(image->GetName());
	}

	texture->SetTextureImagePathAbsolute(image->GetPath());
	texture->SetSize(image->GetWidth(), image->GetHeight());
	texture->SetChannels(image->GetChannels());
	texture->SetTextureUsage(image->GetTextureUsage());
	texture->SetTextureWrappingR(image->GetTextureWrappingR());
	texture->SetTextureWrappingT(image->GetTextureWrappingT());
	texture->SetTextureWrappingS(image->GetTextureWrappingS());
	texture->SetTextureAtlasCategory(atlasCategory);
	texture->SetWaitsForTextureAtlas(true);

	image->RegisterTextureAtlasProxy(texture, atlasCategory);
	materialTextureProxies_.push_back(std::move(materialTextureProxy));

	return texture;
}

void Material::Build(MeshGeometry* meshUnit)
{
	ClearMaterialTextureProxies();

	int ownerMeshBoneCount = 0;
	if (SkeletalMeshGeometry* skeletalMeshGeometry = dynamic_cast<SkeletalMeshGeometry*>(meshUnit))
	{
		ownerMeshBoneCount = skeletalMeshGeometry->GetOwner()->GetBoneSize();
	}

	initializationData_->boneCount = ownerMeshBoneCount;
	initializationData_->meshType = meshUnit ? meshUnit->GetMeshType() : MeshType::None;
	const bool isInstancedStaticMesh = initializationData_->meshType == MeshType::InstancedStatic;

	std::vector<Texture*> materialTextures;
	materialTextures.reserve(textureImages_.size());

	bool hasTextureAtlasCandidate = false;
	for (size_t textureImageIndex = 0; textureImageIndex < textureImages_.size(); ++textureImageIndex)
	{
		const Image* image = textureImages_[textureImageIndex];
		const bool useTextureAtlas = GetTextureImageUsesTextureAtlas(textureImageIndex);
		Texture* texture = GetTextureForShader(image, useTextureAtlas);
		if (texture)
		{
			materialTextures.push_back(texture);
		}

		hasTextureAtlasCandidate = hasTextureAtlasCandidate || useTextureAtlas;
	}

	if (hasTextureAtlasCandidate)
	{
		ResourceManager* resourceManager = engine ? engine->GetResourceManager() : nullptr;
		if (resourceManager && resourceManager->GetResourceContainer())
		{
			resourceManager->GetResourceContainer()->FlushImageTextureAtlas();
		}
	}

	auto AddMaterialTexturesToShader = [&materialTextures](Shader* shader)
	{
		if (!shader)
		{
			return;
		}

		for (Texture* texture : materialTextures)
		{
			if (texture)
			{
				shader->AddTexture(texture);
			}
		}
	};

	RenderPassType mainRenderPassType = engine->GetRenderer()->GetMainRenderType();
	
	Shader* gBufferShader = nullptr;
	if (mainRenderPassType == RenderPassType::Deferred)
	{
		gBufferShader = new Shader();
	}

	Shader* forwardRenderingShader = new Shader();
	AddMaterialTexturesToShader(forwardRenderingShader);
	AddMaterialTexturesToShader(gBufferShader);
	renderPassTypeShaderMap_[RenderPassType::Forward] = forwardRenderingShader;
	
	if(gBufferShader)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer] = gBufferShader;

		std::string vertexShader = isInstancedStaticMesh ?
			ShaderBuilder::GetInstance()->GeometryBufferPass_GetInstancedStaticMeshVertexShaderScript(initializationData_, gBufferShader) :
			ShaderBuilder::GetInstance()->GeometryBufferPass_GetVertexShaderScript(initializationData_, gBufferShader);
		gBufferShader->SetVertexShaderScript(vertexShader);

		std::string fragmentShader = ShaderBuilder::GetInstance()->GeometryBufferPass_GetFragmentShaderScript(initializationData_, gBufferShader);
		gBufferShader->SetFragmentShaderScript(fragmentShader);
	}

	if (forwardRenderingShader->GetShaderType() == ShaderType::Scene)
	{
		std::string vertexShader = isInstancedStaticMesh ?
			ShaderBuilder::GetInstance()->ForwardRenderPass_GetInstancedStaticMeshVertexShaderScript(initializationData_, forwardRenderingShader) :
			ShaderBuilder::GetInstance()->ForwardRenderPass_GetVertexShaderScript(initializationData_, forwardRenderingShader);
		forwardRenderingShader->SetVertexShaderScript(vertexShader);

		std::string fragmentShader = ShaderBuilder::GetInstance()->ForwardRenderPass_GetFragmentShaderScript(initializationData_, forwardRenderingShader);
		forwardRenderingShader->SetFragmentShaderScript(fragmentShader);
	}

	{
		Shader* shadowShader = new Shader();
		AddMaterialTexturesToShader(shadowShader);

		std::string shadowPassVertexShader = isInstancedStaticMesh ?
			ShaderBuilder::GetInstance()->ShadowPass_GetInstancedStaticMeshVertexShaderScript(initializationData_, shadowShader) :
			ShaderBuilder::GetInstance()->ShadowPass_GetVertexShaderScript(initializationData_, shadowShader);
		shadowShader->SetVertexShaderScript(shadowPassVertexShader);

		std::string shadowPassFragmentShader = ShaderBuilder::GetInstance()->ShadowPass_GetFragmentShaderScript(initializationData_, shadowShader);
		shadowShader->SetFragmentShaderScript(shadowPassFragmentShader);

		renderPassTypeShaderMap_[RenderPassType::Shadow] = shadowShader;
	}

	{
		Shader* pointLightShadowShader = new Shader();
		AddMaterialTexturesToShader(pointLightShadowShader);

		std::string pointLightShadowPassVertexShader = isInstancedStaticMesh ?
			ShaderBuilder::GetInstance()->PointShadowPass_GetInstancedStaticMeshVertexShaderScript(initializationData_, pointLightShadowShader) :
			ShaderBuilder::GetInstance()->PointShadowPass_GetVertexShaderScript(initializationData_, pointLightShadowShader);
		pointLightShadowShader->SetVertexShaderScript(pointLightShadowPassVertexShader);
		std::string pointLightShadowPassGeometryShader = ShaderBuilder::GetInstance()->PointShadowPass_GetGeometryShaderScript(initializationData_, pointLightShadowShader);
		pointLightShadowShader->SetGeometryShaderScript(pointLightShadowPassGeometryShader);

		std::string pointLightShadowPassFragmentShader = ShaderBuilder::GetInstance()->PointShadowPass_GetFragmentShaderScript(initializationData_, pointLightShadowShader);
		pointLightShadowShader->SetFragmentShaderScript(pointLightShadowPassFragmentShader);

		renderPassTypeShaderMap_[RenderPassType::PointLightShadow] = pointLightShadowShader;
	}

	{
		Shader* cubemapCaptureShader = new Shader();
		AddMaterialTexturesToShader(cubemapCaptureShader);

		std::string cubemapCaptureVertexShader = isInstancedStaticMesh ?
			ShaderBuilder::GetInstance()->CubemapRenderPass_GetInstancedStaticMeshVertexShaderScript(initializationData_, cubemapCaptureShader) :
			ShaderBuilder::GetInstance()->CubemapRenderPass_GetVertexShaderScript(initializationData_, cubemapCaptureShader);
		cubemapCaptureShader->SetVertexShaderScript(cubemapCaptureVertexShader);

		std::string cubemapCaptureGeometryShader = ShaderBuilder::GetInstance()->CubemapRenderPass_GetGeometryShaderScript(initializationData_, cubemapCaptureShader);
		cubemapCaptureShader->SetGeometryShaderScript(cubemapCaptureGeometryShader);

		std::string cubemapCaptureFragmentShader = ShaderBuilder::GetInstance()->CubemapRenderPass_GetFragmentShaderScript(initializationData_, cubemapCaptureShader);
		cubemapCaptureShader->SetFragmentShaderScript(cubemapCaptureFragmentShader);

		renderPassTypeShaderMap_[RenderPassType::CubemapCapture] = cubemapCaptureShader;
	}
}

void Material::PreInit()
{
	renderPassTypeShaderMap_[RenderPassType::Forward]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::CubemapCapture]->PreInit();

	if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Deferred)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]->PreInit();
	}

	for (auto derivedMaterialInstance : derivedMaterialInstances_)
	{
		derivedMaterialInstance->PreInit();
	}
}

void Material::Init()
{
	Renderer* renderer = engine->GetRenderer();

	renderPassTypeShaderMap_[RenderPassType::Forward]->Init();
	renderer->BindShadowTextures(renderPassTypeShaderMap_[RenderPassType::Forward]);
	
	if (renderer->GetMainRenderType() == RenderPassType::Deferred)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]->Init();
		renderer->BindGeometryBufferTextures(renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]);
	}

	renderPassTypeShaderMap_[RenderPassType::Shadow]->Init();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->Init();
	renderPassTypeShaderMap_[RenderPassType::CubemapCapture]->Init();

	for (auto derivedMaterialInstance : derivedMaterialInstances_)
	{
		derivedMaterialInstance->Init();
	}
}

void Material::PostInit()
{
	renderPassTypeShaderMap_[RenderPassType::Forward]->PostInit();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PostInit();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->PostInit();
	renderPassTypeShaderMap_[RenderPassType::CubemapCapture]->PostInit();

	if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Deferred)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]->PostInit();
	}

	for (auto derivedMaterialInstance : derivedMaterialInstances_)
	{
		derivedMaterialInstance->PostInit();
	}

	engine->GetRenderer()->GetLightManager()->BindLightUniforms(renderPassTypeShaderMap_[RenderPassType::Forward]);

	isInitialized_ = true;

#ifndef GOKNAR_EDITOR
	delete initializationData_;
	initializationData_ = nullptr;
#endif
}

void Material::ResetForRebuild()
{
	for (auto& renderPassShaderPair : renderPassTypeShaderMap_)
	{
		delete renderPassShaderPair.second;
	}

	renderPassTypeShaderMap_.clear();
	ClearMaterialTextureProxies();
	ClearTextureImages();

	delete initializationData_;
	initializationData_ = new MaterialInitializationData(this);

	isInitialized_ = false;
}