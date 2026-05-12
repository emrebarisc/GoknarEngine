#include "pch.h"

#include "Framebuffer.h"

#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Renderer/RenderBuffer.h"
#include "Goknar/Log.h"

namespace
{
	bool IsColorAttachment(FrameBufferAttachment attachment)
	{
		return FrameBufferAttachment::COLOR_ATTACHMENT0 <= attachment &&
			attachment <= FrameBufferAttachment::COLOR_ATTACHMENT9;
	}
}

FrameBuffer::FrameBuffer()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseFrameBufferGUID();
	name_ = std::string("framebuffer" + std::to_string(GUID_));
}

FrameBuffer::~FrameBuffer()
{
	engine->GetGraphicsAPI()->DeleteFrameBuffer(rendererFrameBufferId_);
}

void FrameBuffer::PreInit()
{
	// Skip if already initialized
	if (isInitialized_)
	{
		return;
	}

	rendererFrameBufferId_ = engine->GetGraphicsAPI()->CreateFrameBuffer();

	EXIT_ON_GRAPHICS_API_ERROR("FrameBuffer::Init");
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
	engine->GetGraphicsAPI()->BindFrameBuffer(frameBufferBindTarget_, rendererFrameBufferId_);
	EXIT_ON_GRAPHICS_API_ERROR("FrameBuffer::Bind");
}

void FrameBuffer::Bind(FrameBufferBindTarget bindTarget) const
{
	engine->GetGraphicsAPI()->BindFrameBuffer(bindTarget, rendererFrameBufferId_);
	EXIT_ON_GRAPHICS_API_ERROR("FrameBuffer::Bind(FramebufferBindTarget)");
}

void FrameBuffer::Unbind()
{
	engine->GetGraphicsAPI()->BindFrameBuffer(frameBufferBindTarget_, 0);
	EXIT_ON_GRAPHICS_API_ERROR("FrameBuffer::Unbind");
}

void FrameBuffer::DrawBuffers()
{
	std::vector<FrameBufferAttachment> buffers;
	buffers.reserve(textureAttachments_.size());

	for (const auto& textureAttachment : textureAttachments_)
	{
		if (IsColorAttachment(textureAttachment.first))
		{
			buffers.push_back(textureAttachment.first);
		}
	}

	if (buffers.empty())
	{
		engine->GetGraphicsAPI()->DrawBufferNone();
		engine->GetGraphicsAPI()->ReadBufferNone();
		return;
	}

	engine->GetGraphicsAPI()->DrawBuffers(buffers);
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
			engine->GetGraphicsAPI()->AttachTextureToFrameBuffer(frameBufferBindTarget_, framebufferAttachment, textureTarget->GetRendererTextureId(), 0);
		}
		else
		{
			engine->GetGraphicsAPI()->AttachTexture2DToFrameBuffer(frameBufferBindTarget_, framebufferAttachment, textureTarget->GetTextureImageTarget(), textureTarget->GetRendererTextureId(), 0);
		}

		CheckStatus();
	}

	EXIT_ON_GRAPHICS_API_ERROR("FrameBuffer::AttachToTexture");
}

void FrameBuffer::AttachRenderBuffers()
{
	for (decltype(renderBufferAttachments_)::iterator attachmentIterator = renderBufferAttachments_.begin(); attachmentIterator != renderBufferAttachments_.end(); ++attachmentIterator)
	{
		const RenderBuffer* renderBuffer = *attachmentIterator;

		renderBuffer->BindToFrameBuffer();

		CheckStatus();
	}

	EXIT_ON_GRAPHICS_API_ERROR("FrameBuffer::AttachToTexture");
}

void FrameBuffer::CheckStatus()
{
	std::string statusName;
	if (engine->GetGraphicsAPI()->CheckFrameBufferStatus(FrameBufferBindTarget::FRAMEBUFFER, &statusName))
	{
		return;
	}

	GOKNAR_CORE_ERROR("%s", statusName.c_str());
}
