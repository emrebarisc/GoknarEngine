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
	delete renderPassTypeShaderMap_[RenderPassType::GBuffer];
	delete renderPassTypeShaderMap_[RenderPassType::Shadow];
	delete renderPassTypeShaderMap_[RenderPassType::PointLightShadow];
}

void Material::Build(MeshUnit* meshUnit)
{
	Shader* mainShader = new Shader();
	std::vector<const Image*>::iterator imageIterator = textureImages_.begin();
	for (; imageIterator != textureImages_.end(); ++imageIterator)
	{
		mainShader->AddTexture((*imageIterator)->GetGeneratedTexture());
	}

	renderPassTypeShaderMap_[RenderPassType::Forward] = mainShader;
	if (mainShader->GetShaderType() == ShaderType::Scene)
	{
		ShaderBuilder::GetInstance()->BuildShader(meshUnit, this);
	}

	const std::string& mainShaderVertexShaderScript = mainShader->GetVertexShaderScript();

	Shader* shadowShader = new Shader();
	shadowShader->SetVertexShaderScript(ShaderBuilder::GetInstance()->BuildVertexShader_ShadowPass(meshUnit));
	shadowShader->SetFragmentShaderScript(ShaderBuilder::GetFragmentShaderScript_ShadowPass());
	renderPassTypeShaderMap_[RenderPassType::Shadow] = shadowShader;

	Shader* pointLightShadowShader = new Shader();
	pointLightShadowShader->SetVertexShaderScript(ShaderBuilder::GetInstance()->BuildVertexShader_PointLightShadowPass(meshUnit));
	pointLightShadowShader->SetGeometryShaderScript(ShaderBuilder::GetGeometryShaderScript_PointLightShadowPass());
	pointLightShadowShader->SetFragmentShaderScript(ShaderBuilder::GetFragmentShaderScript_PointLightShadowPass());
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow] = pointLightShadowShader;
}

void Material::PreInit()
{
	renderPassTypeShaderMap_[RenderPassType::Forward]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->PreInit();
}

void Material::Init()
{
	renderPassTypeShaderMap_[RenderPassType::Forward]->Init();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->Init();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->Init();
}

void Material::PostInit()
{
	renderPassTypeShaderMap_[RenderPassType::Forward]->PostInit();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PostInit();
	renderPassTypeShaderMap_[RenderPassType::PointLightShadow]->PostInit();
}

void Material::Render(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix, RenderPassType renderPassType) const
{
	Use(renderPassType);
	SetShaderVariables(worldTransformationMatrix, relativeTransformationMatrix, renderPassType);
}

void Material::Use(RenderPassType renderPassType) const
{
	GOKNAR_CORE_ASSERT(renderPassTypeShaderMap_.find(renderPassType) != renderPassTypeShaderMap_.end());
	
	renderPassTypeShaderMap_.at(renderPassType)->Use();
}

void Material::SetShaderVariables(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix, RenderPassType renderPassType) const
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

	if (renderPassType == RenderPassType::Forward)
	{
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, ambientReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, diffuseReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, phongExponent_);

		engine->GetRenderer()->SetLightUniforms(shader);
	}
	else if (renderPassType == RenderPassType::Shadow || renderPassType == RenderPassType::PointLightShadow)
	{
		engine->GetRenderer()->GetShadowManager()->SetShadowRenderPassShaderUniforms(shader);
	}

	shader->SetMVP(worldTransformationMatrix, relativeTransformationMatrix);
	engine->SetShaderEngineVariables(shader);
}
