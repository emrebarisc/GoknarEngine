#include "pch.h"

#include "RenderBuffer.h"

#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Log.h"

RenderBuffer::RenderBuffer()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseRenderBufferGUID();
	name_ = std::string("renderbuffer" + std::to_string(GUID_));
}

RenderBuffer::~RenderBuffer()
{
	engine->GetGraphicsAPI()->DeleteRenderBuffer(rendererRenderBufferId_);
}

void RenderBuffer::PreInit()
{
	if (isInitialized_)
	{
		return;
	}

	rendererRenderBufferId_ = engine->GetGraphicsAPI()->CreateRenderBuffer();
	engine->GetGraphicsAPI()->BindRenderBuffer(renderBufferBindTarget_, rendererRenderBufferId_);
	engine->GetGraphicsAPI()->RenderBufferStorage(renderBufferBindTarget_, renderBufferInternalType_, width_, height_);

	EXIT_ON_GRAPHICS_API_ERROR("RenderBuffer::Init");
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
	engine->GetGraphicsAPI()->BindRenderBuffer(renderBufferBindTarget_, rendererRenderBufferId_);

	EXIT_ON_GRAPHICS_API_ERROR("RenderBuffer::Bind");
}

void RenderBuffer::BindToFrameBuffer() const
{
	engine->GetGraphicsAPI()->AttachRenderBufferToFrameBuffer(FrameBufferBindTarget::FRAMEBUFFER, renderBufferAttachment_, renderBufferBindTarget_, rendererRenderBufferId_);

	EXIT_ON_GRAPHICS_API_ERROR("RenderBuffer::BindToFrameBuffer");
}

void RenderBuffer::Bind(RenderBufferBindTarget bindTarget) const
{
	engine->GetGraphicsAPI()->BindRenderBuffer(bindTarget, rendererRenderBufferId_);
	EXIT_ON_GRAPHICS_API_ERROR("RenderBuffer::Bind(RenderBufferBindTarget)");
}

void RenderBuffer::Unbind() const
{
	engine->GetGraphicsAPI()->BindRenderBuffer(renderBufferBindTarget_, 0);
	EXIT_ON_GRAPHICS_API_ERROR("RenderBuffer::Unbind");
}
