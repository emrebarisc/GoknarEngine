#include "pch.h"

#include "RenderBuffer.h"

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

RenderBuffer::RenderBuffer()
{
	objectId_ = ObjectIDManager::GetInstance()->GetAndIncreaseRenderBufferID();
	name_ = std::string("renderbuffer" + std::to_string(objectId_));
}

RenderBuffer::~RenderBuffer()
{
	glDeleteRenderbuffers(1, &rendererRenderBufferId_);
}

void RenderBuffer::PreInit()
{
	if (isInitialized_)
	{
		return;
	}

	glGenRenderbuffers(1, &rendererRenderBufferId_);
	glBindRenderbuffer((int)renderBufferBindTarget_, rendererRenderBufferId_);
	glRenderbufferStorage((int)renderBufferBindTarget_, (int)renderBufferInternalType_, width_, height_);

	EXIT_ON_GL_ERROR("RenderBuffer::Init");
}

void RenderBuffer::Init()
{
}

void RenderBuffer::PostInit()
{
	isInitialized_ = true;
}

void RenderBuffer::Bind() const
{
	glBindRenderbuffer((int)renderBufferBindTarget_, rendererRenderBufferId_);

	EXIT_ON_GL_ERROR("RenderBuffer::Bind");
}

void RenderBuffer::BindToFrameBuffer() const
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, (int)renderBufferAttachment_, (int)renderBufferBindTarget_, rendererRenderBufferId_);

	EXIT_ON_GL_ERROR("RenderBuffer::BindToFrameBuffer");
}

void RenderBuffer::Bind(RenderBufferBindTarget bindTarget) const
{
	glBindRenderbuffer((int)bindTarget, rendererRenderBufferId_);
	EXIT_ON_GL_ERROR("RenderBuffer::Bind(RenderBufferBindTarget)");
}

void RenderBuffer::Unbind() const
{
	glBindRenderbuffer((int)renderBufferBindTarget_, 0);
	EXIT_ON_GL_ERROR("RenderBuffer::Unbind");
}