#include "pch.h"

#include "Material.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"

Material::Material() : 
	ambientReflectance_(Vector3::ZeroVector), 
	diffuseReflectance_(Vector3::ZeroVector), 
	specularReflectance_(Vector3::ZeroVector),
	phongExponent_(0.f),
	shadingModel_(MaterialShadingModel::Opaque)
{
}

Material::~Material()
{
}

void Material::Render(const Shader* shader) const
{
	shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, ambientReflectance_);
	shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, diffuseReflectance_);
	shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, phongExponent_);
}
