 #include "pch.h"

#include "Material.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"

#include "Goknar/Contents/Image.h"
#include "Goknar/Lights/ShadowManager/ShadowManager.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

#include "Goknar/Managers/WindowManager.h"

Material::Material() : 
	ambientReflectance_(Vector3::ZeroVector), 
	diffuseReflectance_(Vector3::ZeroVector), 
	specularReflectance_(Vector3::ZeroVector),
	phongExponent_(1.f),
	blendModel_(MaterialBlendModel::Opaque),
	shadingModel_(MaterialShadingModel::Default)
{
}

Material::~Material()
{
	delete renderPassTypeShaderMap_[RenderPassType::Forward];
	delete renderPassTypeShaderMap_[RenderPassType::GeometryBuffer];
	delete renderPassTypeShaderMap_[RenderPassType::Shadow];
	delete renderPassTypeShaderMap_[RenderPassType::PointLightShadow];
}

void Material::Build(MeshUnit* meshUnit)
{
	if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Forward)
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
	else if (engine->GetRenderer()->GetMainRenderType() == RenderPassType::Deferred)
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
}

void Material::Render(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix) const
{
	Use(renderPassType);
	SetShaderVariables(renderPassType, worldAndRelativeTransformationMatrix);
}

void Material::Use(RenderPassType renderPassType) const
{
	GOKNAR_CORE_ASSERT(renderPassTypeShaderMap_.find(renderPassType) != renderPassTypeShaderMap_.end());
	
	renderPassTypeShaderMap_.at(renderPassType)->Use();
}

void Material::SetShaderVariables(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix) const
{
	GOKNAR_CORE_ASSERT(renderPassTypeShaderMap_.find(renderPassType) != renderPassTypeShaderMap_.end());

	if (shadingModel_ == MaterialShadingModel::Default)
	{
		glEnable(GL_CULL_FACE);
	}
	else if (shadingModel_ == MaterialShadingModel::TwoSided)
	{
		glDisable(GL_CULL_FACE);
	}

	Shader* shader = renderPassTypeShaderMap_.at(renderPassType);

	if (renderPassType == RenderPassType::Forward || renderPassType == RenderPassType::GeometryBuffer)
	{
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, ambientReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, diffuseReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, phongExponent_);
	}
	else if (renderPassType == RenderPassType::Shadow || renderPassType == RenderPassType::PointLightShadow)
	{
		engine->GetRenderer()->GetShadowManager()->SetShadowRenderPassShaderUniforms(shader);
	}

	if (renderPassType == RenderPassType::Forward)
	{
		engine->GetRenderer()->SetLightUniforms(shader);
	}

	shader->SetMVP(worldAndRelativeTransformationMatrix);
	engine->SetShaderEngineVariables(shader);
}
