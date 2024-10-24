#ifndef __RENDERTARGET_H__
#define __RENDERTARGET_H__

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"

class Camera;
class Texture;
class FrameBuffer;

class GOKNAR_API RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

	void Init();

	void SetFrameSize(const Vector2& frameSize);

	Camera* GetCamera() const
	{
		return camera_;
	}

	FrameBuffer* GetFrameBuffer() const
	{
		return framebuffer_;
	}

	Texture* GetTexture() const
	{
		return texture_;
	}

protected:
	void GenerateBuffers();

private:
	Camera* camera_{ nullptr };
	FrameBuffer* framebuffer_{ nullptr };
	Texture* texture_{ nullptr };

	Vector2 frameSize_{ 1024, 1024 };
};

#endif