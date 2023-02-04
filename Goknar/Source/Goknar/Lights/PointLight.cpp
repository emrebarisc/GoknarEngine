#include "pch.h"

#include "PointLight.h"

#include "Renderer/Shader.h"

void PointLight::Init()
{
	Light::Init();
}

void PointLight::RenderShadowMap()
{
}

void PointLight::SetShaderUniforms(const Shader* shader) const
{
	std::string positionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION;
	shader->SetVector3(positionName.c_str(), position_);

	std::string intensityName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
	shader->SetVector3(intensityName.c_str(), intensity_);
}
