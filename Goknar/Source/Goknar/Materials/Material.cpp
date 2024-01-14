 #include "pch.h"

#include "Material.h"
#include "MaterialInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

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
}

void Material::Build(MeshUnit* meshUnit)
{
	RenderPassType mainRenderPassType = engine->GetRenderer()->GetMainRenderType();

	bool createForwardRenderingShader = mainRenderPassType == RenderPassType::Forward;

	if (mainRenderPassType == RenderPassType::Deferred)
	{
		Shader* gBufferShader = new Shader();
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer] = gBufferShader;

		std::vector<const Image*>::iterator imageIterator = textureImages_.begin();
		for (; imageIterator != textureImages_.end(); ++imageIterator)
		{
			gBufferShader->AddTexture((*imageIterator)->GetGeneratedTexture());
		}

		gBufferShader->SetVertexShaderScript(ShaderBuilder::GetInstance()->BuildVertexShader_GeometryBufferPass(meshUnit));
		gBufferShader->SetFragmentShaderScript(ShaderBuilder::GetInstance()->GetFragmentShaderScript_GeometryBufferPass(this));
	
		if(blendModel_ == MaterialBlendModel::Transparent)
		{
			createForwardRenderingShader = true;
		}
	}

	if(createForwardRenderingShader)
	{
		Shader* forwardRenderingShader = new Shader();
		renderPassTypeShaderMap_[RenderPassType::Forward] = forwardRenderingShader;

		std::vector<const Image*>::iterator imageIterator = textureImages_.begin();
		for (; imageIterator != textureImages_.end(); ++imageIterator)
		{
			forwardRenderingShader->AddTexture((*imageIterator)->GetGeneratedTexture());
		}

		if (forwardRenderingShader->GetShaderType() == ShaderType::Scene)
		{
			ShaderBuilder::GetInstance()->BuildShader(meshUnit, this);
		}
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
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->PreInit();

	if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Forward)
	{
		renderPassTypeShaderMap_[RenderPassType::Forward]->PreInit();
	}
	else if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Deferred)
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
	renderPassTypeShaderMap_[RenderPassType::Shadow]->Init();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->Init();

	Renderer* renderer = engine->GetRenderer();

	if (renderer->GetMainRenderType() == RenderPassType::Forward)
	{
		renderPassTypeShaderMap_[RenderPassType::Forward]->Init();
		renderer->BindShadowTextures(renderPassTypeShaderMap_[RenderPassType::Forward]);
	}
	else if (renderer->GetMainRenderType() == RenderPassType::Deferred)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]->Init();
		renderer->BindGeometryBufferTextures(renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]);
	}

	for (auto derivedMaterialInstance : derivedMaterialInstances_)
	{
		derivedMaterialInstance->Init();
	}
}

void Material::PostInit()
{
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PostInit();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->PostInit();

	if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Forward)
	{
		renderPassTypeShaderMap_[RenderPassType::Forward]->PostInit();
	}
	else if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Deferred)
	{
		renderPassTypeShaderMap_[RenderPassType::GeometryBuffer]->PostInit();
	}

	for (auto derivedMaterialInstance : derivedMaterialInstances_)
	{
		derivedMaterialInstance->PostInit();
	}
}