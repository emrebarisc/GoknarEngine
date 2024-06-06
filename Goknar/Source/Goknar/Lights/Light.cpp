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

void Light::SetShaderUniforms(const Shader* shader)
{
}

void Light::PreInit()
{
	if (isShadowEnabled_)
	{
		GOKNAR_CORE_ASSERT(shadowMapTexture_, "Shadow is enabled but shadow map texture is not created!");
		GOKNAR_CORE_ASSERT(shadowMapFramebuffer_, "Shadow is enabled but shadow map framebuffer is not created!");

		shadowMapFramebuffer_->SetFramebufferBindTarget(FramebufferBindTarget::FRAMEBUFFER);
		shadowMapFramebuffer_->AddAttachment(FramebufferAttachment::DEPTH_ATTACHMENT, shadowMapTexture_);
		shadowMapFramebuffer_->PreInit();

		shadowMapTexture_->SetWidth(shadowWidth_);
		shadowMapTexture_->SetHeight(shadowHeight_);
		shadowMapTexture_->SetTextureFormat(TextureFormat::DEPTH);
		shadowMapTexture_->SetTextureInternalFormat(TextureInternalFormat::DEPTH_24);
		shadowMapTexture_->SetTextureType(TextureType::FLOAT);
		shadowMapTexture_->SetTextureDataType(TextureDataType::DYNAMIC);
		shadowMapTexture_->PreInit();
		shadowMapTexture_->Bind();

		shadowMapFramebuffer_->Bind();
		shadowMapFramebuffer_->Attach();

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		shadowMapTexture_->Unbind();
		shadowMapFramebuffer_->Unbind();

		EXIT_ON_GL_ERROR("Light::PreInit");
	}
}

void Light::Init()
{
	if (isShadowEnabled_)
	{
		shadowMapFramebuffer_->Init();
		shadowMapTexture_->Init();
		EXIT_ON_GL_ERROR("Light::Init");
	}
}

void Light::PostInit()
{
	if (isShadowEnabled_)
	{
		shadowMapFramebuffer_->PostInit();
		shadowMapTexture_->PostInit();
		EXIT_ON_GL_ERROR("Light::PostInit");
	}
}
