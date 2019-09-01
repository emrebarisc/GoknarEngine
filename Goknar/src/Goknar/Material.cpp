#include "pch.h"

#include "Material.h"

Material::Material() : ambientReflectance_(Vector3::ZeroVector), diffuseReflectance_(Vector3::ZeroVector), specularReflectance_(Vector3::ZeroVector), phongExponent_(0.f)
{
}

Material::~Material()
{
}
