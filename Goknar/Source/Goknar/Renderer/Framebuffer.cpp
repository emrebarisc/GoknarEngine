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
#include "Goknar/Log.h"

Framebuffer::Framebuffer()
{
	objectId_ = ObjectIDManager::GetInstance()->GetAndIncreaseFramebufferID();
	name_ = std::string("framebuffer" + std::to_string(objectId_));
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &rendererFramebufferId_);
}

void Framebuffer::PreInit()
{
	// Skip if already initialized
	if (isInitialized_)
	{
		return;
	}

	glGenFramebuffers(1, &rendererFramebufferId_);

	EXIT_ON_GL_ERROR("Framebuffer::Init");
	isInitialized_ = true;
}

void Framebuffer::Init()
{
}

void Framebuffer::PostInit()
{
}

void Framebuffer::Bind() const
{
	glBindFramebuffer((int)framebufferBindTarget_, rendererFramebufferId_);
	EXIT_ON_GL_ERROR("Framebuffer::Bind");
}

void Framebuffer::Bind(FramebufferBindTarget bindTarget) const
{
	glBindFramebuffer((int)bindTarget, rendererFramebufferId_);
	EXIT_ON_GL_ERROR("Framebuffer::Bind(FramebufferBindTarget)");
}

void Framebuffer::Unbind()
{
	glBindFramebuffer((int)framebufferBindTarget_, 0);
	EXIT_ON_GL_ERROR("Framebuffer::Unbind");
}

void Framebuffer::DrawBuffers()
{
	int attachmentsSize = attachments.size();
	unsigned int* buffers = new unsigned int[attachmentsSize];

	for (int attachmentIndex = 0; attachmentIndex < attachmentsSize; ++attachmentIndex)
	{
		buffers[attachmentIndex] = (unsigned int)attachments[attachmentIndex].first;
	}

	glDrawBuffers(attachmentsSize, buffers);
	delete[] buffers;
}

void Framebuffer::Attach()
{
	GOKNAR_CORE_ASSERT(0 < attachments.size(), "Framebuffer texture target is null to attach!");

	for (decltype(attachments)::iterator attachmentIterator = attachments.begin(); attachmentIterator != attachments.end(); ++attachmentIterator)
	{
		FramebufferAttachment framebufferAttachment = attachmentIterator->first;
		Texture* textureTarget = attachmentIterator->second;

		if (textureTarget->GetTextureBindTarget() == TextureBindTarget::TEXTURE_CUBE_MAP)
		{
			glFramebufferTexture((int)framebufferBindTarget_, (int)framebufferAttachment, textureTarget->GetRendererTextureId(), 0);
		}
		else
		{
			glFramebufferTexture2D((int)framebufferBindTarget_, (int)framebufferAttachment, (int)textureTarget->GetTextureImageTarget(), textureTarget->GetRendererTextureId(), 0);
		}

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
			GOKNAR_CORE_INFO("GL_FRAMEBUFFER_COMPLETE");
			break;
		default:
			break;
		}
	}

	EXIT_ON_GL_ERROR("Framebuffer::AttachToTexture");
}
