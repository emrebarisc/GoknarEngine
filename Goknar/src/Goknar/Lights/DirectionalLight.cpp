#include "pch.h"

#include "DirectionalLight.h"

#include "Goknar/Shader.h"
#include "Goknar/Managers/ShaderBuilder.h"

int DirectionalLight::currentId_ = 0;

void DirectionalLight::SetShaderUniforms(const Shader* shader) const
{
	std::string directionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION;
	shader->SetVector3(directionName.c_str(), direction_);

	std::string radianceName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
	shader->SetVector3(radianceName.c_str(), color_ * intensity_);
}
