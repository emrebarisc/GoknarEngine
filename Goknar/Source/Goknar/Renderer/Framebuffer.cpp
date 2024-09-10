#include "pch.h"

#include "Framebuffer.h"

#include "glad/glad.h"

#ifdef GOKNAR_PLATFORM_WINDOWS
#include <GL/GLU.h>
#elif defined(GOKNAR_PLATFORM_UNIX)
#include <GL/gl.h>
#endif

#include "Goknar/GoknarAssert.h"
#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Renderer/RenderBuffer.h"
#include "Goknar/Log.h"

FrameBuffer::FrameBuffer()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseFrameBufferGUID();
	name_ = std::string("framebuffer" + std::to_string(GUID_));
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &rendererFrameBufferId_);
}

void FrameBuffer::PreInit()
{
	// Skip if already initialized
	if (isInitialized_)
	{
		return;
	}

	glGenFramebuffers(1, &rendererFrameBufferId_);

	EXIT_ON_GL_ERROR("FrameBuffer::Init");
}

void FrameBuffer::Init()
{
}

void FrameBuffer::PostInit()
{
	isInitialized_ = true;
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer((int)frameBufferBindTarget_, rendererFrameBufferId_);
	EXIT_ON_GL_ERROR("FrameBuffer::Bind");
}

void FrameBuffer::Bind(FrameBufferBindTarget bindTarget) const
{
	glBindFramebuffer((int)bindTarget, rendererFrameBufferId_);
	EXIT_ON_GL_ERROR("FrameBuffer::Bind(FramebufferBindTarget)");
}

void FrameBuffer::Unbind()
{
	glBindFramebuffer((int)frameBufferBindTarget_, 0);
	EXIT_ON_GL_ERROR("FrameBuffer::Unbind");
}

void FrameBuffer::DrawBuffers()
{
	int attachmentsSize = textureAttachments_.size();
	unsigned int* buffers = new unsigned int[attachmentsSize];

	for (int attachmentIndex = 0; attachmentIndex < attachmentsSize; ++attachmentIndex)
	{
		buffers[attachmentIndex] = (unsigned int)textureAttachments_[attachmentIndex].first;
	}

	glDrawBuffers(attachmentsSize, buffers);
	delete[] buffers;
}

void FrameBuffer::Attach()
{
	GOKNAR_CORE_ASSERT(0 < textureAttachments_.size() || 0 < renderBufferAttachments_.size(), "FrameBuffer has no attachments!");

	AttachTextures();
	AttachRenderBuffers();
}

void FrameBuffer::AttachTextures()
{
	for (decltype(textureAttachments_)::iterator attachmentIterator = textureAttachments_.begin(); attachmentIterator != textureAttachments_.end(); ++attachmentIterator)
	{
		FrameBufferAttachment framebufferAttachment = attachmentIterator->first;
		Texture* textureTarget = attachmentIterator->second;

		if (textureTarget->GetTextureBindTarget() == TextureBindTarget::TEXTURE_CUBE_MAP)
		{
			glFramebufferTexture((int)frameBufferBindTarget_, (int)framebufferAttachment, textureTarget->GetRendererTextureId(), 0);
		}
		else
		{
			glFramebufferTexture2D((int)frameBufferBindTarget_, (int)framebufferAttachment, (int)textureTarget->GetTextureImageTarget(), textureTarget->GetRendererTextureId(), 0);
		}

		CheckStatus();
	}

	EXIT_ON_GL_ERROR("FrameBuffer::AttachToTexture");
}

void FrameBuffer::AttachRenderBuffers()
{
	for (decltype(renderBufferAttachments_)::iterator attachmentIterator = renderBufferAttachments_.begin(); attachmentIterator != renderBufferAttachments_.end(); ++attachmentIterator)
	{
		const RenderBuffer* renderBuffer = *attachmentIterator;

		renderBuffer->BindToFrameBuffer();

		CheckStatus();
	}

	EXIT_ON_GL_ERROR("FrameBuffer::AttachToTexture");
}

void FrameBuffer::CheckStatus()
{
	switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
	case GL_FRAMEBUFFER_UNDEFINED:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_UNDEFINED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_UNSUPPORTED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		GOKNAR_CORE_ERROR("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
		break;
	case GL_FRAMEBUFFER_COMPLETE:
		//GOKNAR_CORE_INFO("GL_FRAMEBUFFER_COMPLETE");
		break;
	default:
		break;
	}
}
