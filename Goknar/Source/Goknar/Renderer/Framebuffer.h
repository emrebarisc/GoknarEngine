#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "Goknar/Core.h"
#include "Types.h"

class RenderBuffer;
class Texture;

enum class FrameBufferBindTarget
{
	DRAW_FRAMEBUFFER = GL_DRAW_FRAMEBUFFER,
	READ_FRAMEBUFFER = GL_READ_FRAMEBUFFER,
	FRAMEBUFFER = GL_FRAMEBUFFER
};

enum class FrameBufferAttachment
{
	COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
	COLOR_ATTACHMENT1 = GL_COLOR_ATTACHMENT1,
	COLOR_ATTACHMENT2 = GL_COLOR_ATTACHMENT2,
	COLOR_ATTACHMENT3 = GL_COLOR_ATTACHMENT3,
	COLOR_ATTACHMENT4 = GL_COLOR_ATTACHMENT4,
	COLOR_ATTACHMENT5 = GL_COLOR_ATTACHMENT5,
	COLOR_ATTACHMENT6 = GL_COLOR_ATTACHMENT6,
	COLOR_ATTACHMENT7 = GL_COLOR_ATTACHMENT7,
	COLOR_ATTACHMENT8 = GL_COLOR_ATTACHMENT8,
	COLOR_ATTACHMENT9 = GL_COLOR_ATTACHMENT9,

	DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT,
	STENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT
};

class GOKNAR_API FrameBuffer
{
public:
	FrameBuffer();
	virtual ~FrameBuffer();

	void PreInit();
	void Init();
	void PostInit();

	void Bind() const;
	void Bind(FrameBufferBindTarget bindTarget) const;
	void Unbind();

	void DrawBuffers();

	GEuint GetRendererFrameBufferId()
	{
		return rendererFrameBufferId_;
	}

	GEuint GetGUID() const
	{
		return GUID_;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}

	FrameBufferBindTarget GetFrameBufferBindTarget() const
	{
		return frameBufferBindTarget_;
	}

	void SetFrameBufferBindTarget(FrameBufferBindTarget framebufferBindTarget)
	{
		frameBufferBindTarget_ = framebufferBindTarget;
	}

	void AddTextureAttachment(FrameBufferAttachment framebufferAttachment, Texture* textureTarget)
	{
		textureAttachments_.push_back({ framebufferAttachment, textureTarget });
	}

	void AddRenderBufferAttachment(RenderBuffer* renderBufferAttachment)
	{
		renderBufferAttachments_.push_back(renderBufferAttachment);
	}

	void Attach();

protected:

private:
	void AttachTextures();
	void AttachRenderBuffers();
	void CheckStatus();

	std::string name_{ "" };

	GEuint rendererFrameBufferId_{ 0 };

	FrameBufferBindTarget frameBufferBindTarget_{ FrameBufferBindTarget::FRAMEBUFFER };

	std::vector<std::pair<FrameBufferAttachment, Texture*>> textureAttachments_;
	std::vector<RenderBuffer*> renderBufferAttachments_;

	int GUID_{ 0 };

	bool isInitialized_{ false };
};

#endif