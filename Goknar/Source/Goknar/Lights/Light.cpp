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
	delete shadowMapFrameBuffer_;

	if (shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->Destroy();
	}
}

void Light::SetShaderUniforms(const Shader* shader)
{
}

void Light::PreInit()
{
	if (isShadowEnabled_)
	{
		GOKNAR_CORE_ASSERT(shadowMapTexture_, "Shadow is enabled but shadow map texture is not created!");
		GOKNAR_CORE_ASSERT(shadowMapFrameBuffer_, "Shadow is enabled but shadow map framebuffer is not created!");

		shadowMapFrameBuffer_->SetFrameBufferBindTarget(FrameBufferBindTarget::FRAMEBUFFER);
		shadowMapFrameBuffer_->AddTextureAttachment(FrameBufferAttachment::DEPTH_ATTACHMENT, shadowMapTexture_);
		shadowMapFrameBuffer_->PreInit();
		shadowMapFrameBuffer_->Init();
		shadowMapFrameBuffer_->PostInit();

		shadowMapTexture_->SetWidth(shadowWidth_);
		shadowMapTexture_->SetHeight(shadowHeight_);
		shadowMapTexture_->SetTextureFormat(TextureFormat::DEPTH);
		shadowMapTexture_->SetTextureInternalFormat(TextureInternalFormat::DEPTH_24);
		shadowMapTexture_->SetTextureType(TextureType::FLOAT);
		shadowMapTexture_->SetTextureDataType(TextureDataType::DYNAMIC);
		shadowMapTexture_->PreInit();
		shadowMapTexture_->Init();
		shadowMapTexture_->PostInit();
		shadowMapTexture_->Bind();

		shadowMapFrameBuffer_->Bind();
		shadowMapFrameBuffer_->Attach();

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		shadowMapTexture_->Unbind();
		shadowMapFrameBuffer_->Unbind();

		EXIT_ON_GL_ERROR("Light::PreInit");
	}
}

void Light::Init()
{
	if (isShadowEnabled_)
	{
		shadowMapFrameBuffer_->Init();
		shadowMapTexture_->Init();
		EXIT_ON_GL_ERROR("Light::Init");
	}
}

void Light::PostInit()
{
	if (isShadowEnabled_)
	{
		shadowMapFrameBuffer_->PostInit();
		shadowMapTexture_->PostInit();
		EXIT_ON_GL_ERROR("Light::PostInit");
	}
}
