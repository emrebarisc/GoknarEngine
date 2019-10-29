#include "pch.h"

#include "Material.h"

#include "Goknar/Managers/ShaderBuilder.h"
#include "Goknar/Renderer/Shader.h"

Material::Material() : 
	ambientReflectance_(Vector3::ZeroVector), 
	diffuseReflectance_(Vector3::ZeroVector), 
	specularReflectance_(Vector3::ZeroVector),
	textures_(nullptr),
	phongExponent_(0.f)
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
