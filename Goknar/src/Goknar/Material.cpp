#include "pch.h"

#include "Material.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

Material::Material() : 
	ambientReflectance_(Vector3::ZeroVector), 
	diffuseReflectance_(Vector3::ZeroVector), 
	specularReflectance_(Vector3::ZeroVector),
	phongExponent_(0.f),
	shader_(nullptr),
	blendModel_(MaterialBlendModel::Opaque),
	shadingModel_(MaterialShadingModel::Default)
{
}

Material::~Material()
{
}

void Material::Init()
{
	if (shader_ == nullptr)
	{
		shader_ = new Shader();
	}

	shader_->Init();
}

void Material::Render(const Matrix& modelMatrix) const
{
	if (shadingModel_ == MaterialShadingModel::Default)
	{
		glEnable(GL_CULL_FACE);
	}
	else if (shadingModel_ == MaterialShadingModel::TwoSided)
	{
		glDisable(GL_CULL_FACE);
	}

	shader_->Use();
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, ambientReflectance_);
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, diffuseReflectance_);
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
	shader_->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, phongExponent_);

	shader_->SetMVP(modelMatrix);

	engine->GetApplication()->GetMainScene()->SetShaderDynamicLightUniforms(shader_);
	engine->SetShaderEngineVariables(shader_);
}
