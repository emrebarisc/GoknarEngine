 #include "pch.h"

#include "Material.h"
#include "MaterialInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderBuilderNew.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Lights/LightManager/LightManager.h"

Material::Material() :
	IMaterialBase()
{
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
		gBufferShader->SetVertexShaderScript(ShaderBuilder::GetInstance()->BuildVertexShader_GeometryBufferPass(meshUnit));
		gBufferShader->SetFragmentShaderScript(ShaderBuilder::GetInstance()->GetFragmentShaderScript_GeometryBufferPass(this));
	}

	if (forwardRenderingShader->GetShaderType() == ShaderType::Scene)
	{
		//ShaderBuilder::GetInstance()->BuildShader(meshUnit, this);

		std::string vertexShader = ShaderBuilderNew::GetInstance()->ForwardRender_GetVertexShaderScript(initializationData_, forwardRenderingShader);
		forwardRenderingShader->SetVertexShaderScript(vertexShader);

		std::string fragmentShader = ShaderBuilderNew::GetInstance()->ForwardRender_GetFragmentShaderScript(initializationData_, forwardRenderingShader);
		forwardRenderingShader->SetFragmentShaderScript(fragmentShader);
	}

	Shader* shadowShader = new Shader();
	shadowShader->SetVertexShaderScript(ShaderBuilder::GetInstance()->BuildVertexShader_ShadowPass(meshUnit));
	shadowShader->SetFragmentShaderScript(ShaderBuilder::GetInstance()->GetFragmentShaderScript_ShadowPass());
	renderPassTypeShaderMap_[RenderPassType::Shadow] = shadowShader;

	Shader* pointLightShadowShader = new Shader();
	pointLightShadowShader->SetVertexShaderScript(ShaderBuilder::GetInstance()->BuildVertexShader_PointLightShadowPass(meshUnit));
	pointLightShadowShader->SetGeometryShaderScript(ShaderBuilder::GetInstance()->GetGeometryShaderScript_PointLightShadowPass());
	pointLightShadowShader->SetFragmentShaderScript(ShaderBuilder::GetInstance()->GetFragmentShaderScript_PointLightShadowPass());
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow] = pointLightShadowShader;
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