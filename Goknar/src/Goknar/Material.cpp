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
	shadingModel_(MaterialShadingModel::Opaque)
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
	shader_->Use();
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, ambientReflectance_);
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, diffuseReflectance_);
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
	shader_->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, phongExponent_);

	shader_->SetMVP(modelMatrix);

	engine->GetApplication()->GetMainScene()->SetShaderDynamicLightUniforms(shader_);
}
