#include "pch.h"

#include "Light.h"

#include "Goknar/Shader.h"
#include "Goknar/Managers/ShaderBuilder.h"

void Light::SetShaderUniforms(const Shader* shader) const
{
	std::string positionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION;
	shader->SetVector3(positionName.c_str(), position_);

	std::string intensityName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
	shader->SetVector3(intensityName.c_str(), color_ * intensity_);
}