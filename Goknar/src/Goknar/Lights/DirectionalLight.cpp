#include "pch.h"

#include "DirectionalLight.h"

#include "Goknar/Shader.h"
#include "Goknar/Managers/ShaderBuilder.h"

int DirectionalLight::currentId_ = 0;

void DirectionalLight::SetShaderUniforms(const Shader* shader)
{
	char* directionName = "";
	strcpy(directionName, name_);
	strcpy(directionName, SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION);
	shader->SetVector3(directionName, direction_);

	char* radianceName = "";
	strcpy(radianceName, name_);
	strcpy(radianceName, SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY);
	shader->SetVector3(radianceName, color_ * intensity_);
}
