#include "pch.h"

#include "Light.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/ShaderBuilder.h"

Light::Light() :
	id_(0),
	position_(Vector3::ZeroVector),
	color_(Vector3::ZeroVector),
	intensity_(0.f),
	name_(""),
	isShadowEnabled_(true),
	mobility_(LightMobility::Static)
{
	if (isShadowEnabled_)
	{
		//glGenFramebuffers(1, &shadowMappingFBO_);
		//glBindFramebuffer(GL_FRAMEBUFFER, shadowMappingFBO_);
	}
}

void Light::SetShaderUniforms(const Shader* shader) const
{
	std::string positionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION;
	shader->SetVector3(positionName.c_str(), position_);

	std::string intensityName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
	shader->SetVector3(intensityName.c_str(), color_ * intensity_);
}
