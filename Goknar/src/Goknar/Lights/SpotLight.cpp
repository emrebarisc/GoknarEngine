#include "pch.h"

#include "SpotLight.h"

#include "Goknar/Shader.h"
#include "Goknar/Managers/ShaderBuilder.h"

int SpotLight::currentId_ = 0;

void SpotLight::SetShaderUniforms(const Shader* shader)
{
	Light::SetShaderUniforms(shader);

	char* directionName = "";
	strcpy(directionName, name_);
	strcpy(directionName, SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION);
	shader->SetVector3(directionName, direction_);

	char* coverageAngleName = "";
	strcpy(coverageAngleName, name_);
	strcpy(coverageAngleName, SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE);
	shader->SetFloat(coverageAngleName, coverageAngle_);

	char* falloffAngleName = "";
	strcpy(falloffAngleName, name_);
	strcpy(falloffAngleName, SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE);
	shader->SetFloat(falloffAngleName, falloffAngle_);
}
