#include "pch.h"

#include "SpotLight.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

void SpotLight::SetShaderUniforms(const Shader* shader) const
{
	Light::SetShaderUniforms(shader);

	std::string directionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION;
	shader->SetVector3(directionName.c_str(), direction_);

	std::string coverageAngleName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE;
	shader->SetFloat(coverageAngleName.c_str(), coverageAngle_);

	std::string falloffAngleName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE;
	shader->SetFloat(falloffAngleName.c_str(), falloffAngle_);
}
