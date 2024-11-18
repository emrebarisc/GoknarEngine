#ifndef __RENDERBUFFER_H__
#define __RENDERBUFFER_H__

#include "Goknar/Core.h"
#include "Types.h"

class Texture;

enum class RenderBufferBindTarget
{
	RENDERBUFFER = GL_RENDERBUFFER
};

enum class RenderBufferAttachment
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

enum class RenderBufferInternalType
{
	DEPTH = GL_DEPTH_COMPONENT,
	DEPTH_16 = GL_DEPTH_COMPONENT16,
	DEPTH_24 = GL_DEPTH_COMPONENT24,
	DEPTH_32 = GL_DEPTH_COMPONENT32,
	DEPTH_32F = GL_DEPTH_COMPONENT32F,
	DEPTH_STENCIL = GL_DEPTH_STENCIL,
	DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
	RED = GL_RED,
	RG = GL_RG,
	RGB = GL_RGB,
	RGB16F = GL_RGB16F,
	RGB32F = GL_RGB32F,
	RGBA = GL_RGBA,
	RGBA16F = GL_RGBA16F,
	RGBA32F = GL_RGBA32F,
};

class GOKNAR_API RenderBuffer
{
public:
	RenderBuffer();
	virtual ~RenderBuffer();

	void PreInit();
	void Init();
	void PostInit();

	void Bind() const;
	void BindToFrameBuffer() const;
	void Bind(RenderBufferBindTarget bindTarget) const;
	void Unbind() const;

	GEuint GetRendererRenderBufferId()
	{
		return rendererRenderBufferId_;
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

	RenderBufferBindTarget GetFrameBufferBindTarget() const
	{
		return renderBufferBindTarget_;
	}

	void SetRenderBufferBindTarget(RenderBufferBindTarget renderBindTarget)
	{
		renderBufferBindTarget_ = renderBindTarget;
	}
	
	RenderBufferAttachment GetRenderBufferAttachment() const
	{
		return renderBufferAttachment_;
	}

	void SetRenderBufferAttachment(RenderBufferAttachment renderBufferAttachment)
	{
		renderBufferAttachment_ = renderBufferAttachment;
	}

	RenderBufferInternalType GetRenderBufferInternalType() const
	{
		return renderBufferInternalType_;
	}

	void SetRenderBufferInternalType(RenderBufferInternalType renderBufferInternalType)
	{
		renderBufferInternalType_ = renderBufferInternalType;
	}

	void SetWidth(int width)
	{
		width_ = width;
	}

	int GetWidth() const
	{
		return width_;
	}

	void SetHeight(int height)
	{
		height_ = height;
	}

	int GetHeight() const
	{
		return height_;
	}

protected:

private:
	std::string name_{ "" };

	GEuint rendererRenderBufferId_{ 0 };

	RenderBufferBindTarget renderBufferBindTarget_{ RenderBufferBindTarget::RENDERBUFFER };
	RenderBufferAttachment renderBufferAttachment_{ RenderBufferAttachment::COLOR_ATTACHMENT0 };
	RenderBufferInternalType renderBufferInternalType_{ RenderBufferInternalType::RGB };

	int GUID_{ 0 };

	int width_{ 1920 };
	int height_{ 1080 };

	bool isInitialized_{ false };
};

#endif