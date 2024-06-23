 #include "pch.h"

#include "Material.h"
#include "MaterialInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderBuilderNew.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Lights/LightManager/LightManager.h"

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

	delete renderPassTypeShaderMap_[RenderPassType::Forward];
	delete renderPassTypeShaderMap_[RenderPassType::GeometryBuffer];
	delete renderPassTypeShaderMap_[RenderPassType::Shadow];
	delete renderPassTypeShaderMap_[RenderPassType::PointLightShadow];

	delete initializationData_;
}

void Material::Build(MeshUnit* meshUnit)
{
	int ownerMeshBoneCount = 0;
	if (SkeletalMesh* skeletalMesh = dynamic_cast<SkeletalMesh*>(meshUnit))
	{
		ownerMeshBoneCount = skeletalMesh->GetBoneSize();
	}

	initializationData_->boneCount = ownerMeshBoneCount;

	RenderPassType mainRenderPassType = engine->GetRenderer()->GetMainRenderType();
	
	Shader* gBufferShader = nullptr;
	if (mainRenderPassType == RenderPassType::Deferred)
	{
		gBufferShader = new Shader();
	}

	Shader* forwardRenderingShader = new Shader();

	std::vector<const Image*>::iterator imageIterator = textureImages_.begin();
	for (; imageIterator != textureImages_.end(); ++imageIterator)
	{
		forwardRenderingShader->AddTexture((*imageIterator)->GetGeneratedTexture());
		if(gBufferShader)
		{
			gBufferShader->AddTexture((*imageIterator)->GetGeneratedTexture());
		}
	}
	renderPassTypeShaderMap_[RenderPassType::Forward] = forwardRenderingShader;
	
	if(gBufferShader)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer] = gBufferShader;

		std::string vertexShader = ShaderBuilderNew::GetInstance()->GeometryBufferPass_GetVertexShaderScript(initializationData_, gBufferShader);
		gBufferShader->SetVertexShaderScript(vertexShader);

		std::string fragmentShader = ShaderBuilderNew::GetInstance()->GeometryBufferPass_GetFragmentShaderScript(initializationData_, gBufferShader);
		gBufferShader->SetFragmentShaderScript(fragmentShader);
	}

	if (forwardRenderingShader->GetShaderType() == ShaderType::Scene)
	{
		std::string vertexShader = ShaderBuilderNew::GetInstance()->ForwardRenderPass_GetVertexShaderScript(initializationData_, forwardRenderingShader);
		forwardRenderingShader->SetVertexShaderScript(vertexShader);

		std::string fragmentShader = ShaderBuilderNew::GetInstance()->ForwardRenderPass_GetFragmentShaderScript(initializationData_, forwardRenderingShader);
		forwardRenderingShader->SetFragmentShaderScript(fragmentShader);
	}

	{
		Shader* shadowShader = new Shader();

		std::string shadowPassVertexShader = ShaderBuilderNew::GetInstance()->ShadowPass_GetVertexShaderScript(initializationData_, shadowShader);
		shadowShader->SetVertexShaderScript(shadowPassVertexShader);

		std::string shadowPassFragmentShader = ShaderBuilderNew::GetInstance()->ShadowPass_GetFragmentShaderScript(initializationData_, shadowShader);
		shadowShader->SetFragmentShaderScript(shadowPassFragmentShader);

		renderPassTypeShaderMap_[RenderPassType::Shadow] = shadowShader;
	}

	{
		Shader* pointLightShadowShader = new Shader();

		std::string pointLightShadowPassVertexShader = ShaderBuilderNew::GetInstance()->PointShadowPass_GetVertexShaderScript(initializationData_, pointLightShadowShader);
		pointLightShadowShader->SetVertexShaderScript(pointLightShadowPassVertexShader);

		std::string pointLightShadowPassGeometryShader = ShaderBuilderNew::GetInstance()->PointShadowPass_GetGeometryShaderScript(initializationData_, pointLightShadowShader);
		pointLightShadowShader->SetGeometryShaderScript(pointLightShadowPassGeometryShader);

		std::string pointLightShadowPassFragmentShader = ShaderBuilderNew::GetInstance()->PointShadowPass_GetFragmentShaderScript(initializationData_, pointLightShadowShader);
		pointLightShadowShader->SetFragmentShaderScript(pointLightShadowPassFragmentShader);

		renderPassTypeShaderMap_[RenderPassType::PointLightShadow] = pointLightShadowShader;
	}

}

void Material::PreInit()
{
	renderPassTypeShaderMap_[RenderPassType::Forward]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->PreInit();

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

	if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Deferred)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]->PostInit();
	}

	for (auto derivedMaterialInstance : derivedMaterialInstances_)
	{
		derivedMaterialInstance->PostInit();
	}

	engine->GetRenderer()->GetLightManager()->BindLightUniforms(renderPassTypeShaderMap_[RenderPassType::Forward]);

	delete initializationData_;
	initializationData_ = nullptr;
}