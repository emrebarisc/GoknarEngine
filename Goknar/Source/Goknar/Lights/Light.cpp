#include "pch.h"

#include "Light.h"

#include "Goknar/Camera.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

Light::~Light()
{
	delete shadowMapTexture_;
	delete shadowMapFramebuffer_;
}

void Light::SetShaderUniforms(const Shader* shader) const
{
	std::string positionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION;
	shader->SetVector3(positionName.c_str(), position_);

	std::string intensityName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
	shader->SetVector3(intensityName.c_str(), color_ * intensity_);
}

void Light::Init()
{
	if (isShadowEnabled_)
	{
		GOKNAR_CORE_ASSERT(shadowMapTexture_, "Shadow is enabled but shadow map texture is not created!");
		GOKNAR_CORE_ASSERT(shadowMapFramebuffer_, "Shadow is enabled but shadow map framebuffer is not created!");

		shadowMapFramebuffer_->SetFramebufferBindTarget(FramebufferBindTarget::FRAMEBUFFER);
		shadowMapFramebuffer_->SetFramebufferAttachment(FramebufferAttachment::DEPTH_ATTACHMENT);

		shadowMapFramebuffer_->SetTextureTarget(shadowMapTexture_);
		shadowMapFramebuffer_->Init();

		shadowMapTexture_->SetWidth(shadowWidth_);
		shadowMapTexture_->SetHeight(shadowHeight_);
		shadowMapTexture_->SetTextureMinFilter(TextureMinFilter::LINEAR);
		shadowMapTexture_->SetTextureMagFilter(TextureMagFilter::LINEAR);
		shadowMapTexture_->SetTextureFormat(TextureFormat::DEPTH);
		shadowMapTexture_->SetTextureInternalFormat(TextureInternalFormat::DEPTH_24);
		shadowMapTexture_->SetTextureType(TextureType::FLOAT);
		shadowMapTexture_->SetTextureDataType(TextureDataType::DYNAMIC);
		shadowMapTexture_->SetTextureCompareMode(TextureCompareMode::CompareRefToTexture);
		shadowMapTexture_->SetTextureCompareFunc(TextureCompareFunc::LEQUAL);
		shadowMapTexture_->Init();
		shadowMapTexture_->Bind();

		shadowMapFramebuffer_->Bind();
		shadowMapFramebuffer_->Attach();

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		shadowMapTexture_->Unbind();
		shadowMapFramebuffer_->Unbind();

		EXIT_ON_GL_ERROR("Light::Init");
	}
}
