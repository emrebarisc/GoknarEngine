#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "Goknar/Core.h"
#include "Types.h"

class Texture;

enum class FramebufferBindTarget
{
	DRAW_FRAMEBUFFER = GL_DRAW_FRAMEBUFFER,
	READ_FRAMEBUFFER = GL_READ_FRAMEBUFFER,
	FRAMEBUFFER = GL_FRAMEBUFFER
};

enum class FramebufferAttachment
{
	COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
	DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT,
	STENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT
};

class GOKNAR_API Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void Init();
	void Bind() const;
	void Unbind();

	GEuint GetRendererFramebufferId()
	{
		return rendererFramebufferId_;
	}

	GEuint GetFramebufferObjectId() const
	{
		return objectId_;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}

	Texture* GetTextureTarget() const
	{
		return textureTarget_;
	}

	void SetTextureTarget(Texture* textureTarget)
	{
		textureTarget_ = textureTarget;
	}

	FramebufferBindTarget GetFrameBufferBindTarget() const
	{
		return framebufferBindTarget_;
	}

	void SetFramebufferBindTarget(FramebufferBindTarget framebufferBindTarget)
	{
		framebufferBindTarget_ = framebufferBindTarget;
	}

	FramebufferAttachment GetFrameBufferAttachment() const
	{
		return framebufferAttachment_;
	}

	void SetFramebufferAttachment(FramebufferAttachment framebufferAttachment)
	{
		framebufferAttachment_ = framebufferAttachment;
	}

	void Attach();

protected:

private:
	std::string name_{ "" };

	Texture* textureTarget_{ nullptr };

	GEuint rendererFramebufferId_{ 0 };

	FramebufferBindTarget framebufferBindTarget_{ FramebufferBindTarget::FRAMEBUFFER };
	FramebufferAttachment framebufferAttachment_{ FramebufferAttachment::COLOR_ATTACHMENT0 };

	int objectId_{ 0 };

	bool isInitialized_{ false };
};

#endif