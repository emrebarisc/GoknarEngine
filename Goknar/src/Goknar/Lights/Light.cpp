#include "pch.h"

#include "Light.h"

#include "Goknar/Shader.h"

void Light::SetShaderUniforms(const Shader* shader)
{
	char* positionName = "";
	strcpy(positionName, name_);
	strcpy(positionName, "_position");
	shader->SetVector3(positionName, position_);

	char* intensityName = "";
	strcpy(intensityName, name_);
	strcpy(intensityName, "_intensity");
	shader->SetVector3(intensityName, color_ * intensity_);
}
