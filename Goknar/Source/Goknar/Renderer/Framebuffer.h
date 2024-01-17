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

class GOKNAR_API Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void PreInit();
	void Init();
	void PostInit();

	void Bind() const;
	void Unbind();

	void DrawBuffers();

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

	FramebufferBindTarget GetFrameBufferBindTarget() const
	{
		return framebufferBindTarget_;
	}

	void SetFramebufferBindTarget(FramebufferBindTarget framebufferBindTarget)
	{
		framebufferBindTarget_ = framebufferBindTarget;
	}

	void AddAttachment(FramebufferAttachment framebufferAttachment, Texture* textureTarget)
	{
		attachments.push_back({ framebufferAttachment, textureTarget });
	}

	void Attach();

protected:

private:
	std::string name_{ "" };

	GEuint rendererFramebufferId_{ 0 };

	FramebufferBindTarget framebufferBindTarget_{ FramebufferBindTarget::FRAMEBUFFER };

	std::vector<std::pair<FramebufferAttachment, Texture*>> attachments;

	int objectId_{ 0 };

	bool isInitialized_{ false };
};

#endif