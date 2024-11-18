#ifndef __RENDERTARGET_H__
#define __RENDERTARGET_H__

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"

class Camera;
class DeferredRenderingData;
class FrameBuffer;
class RenderBuffer;
class Texture;

class GOKNAR_API RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

	void Init();

	void SetFrameSize(const Vector2& frameSize);

	void SetCamera(Camera* camera)
	{
		camera_ = camera;
	}

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

	bool GetIsActive() const
	{
		return isActive_;
	}

	void SetIsActive(bool isActive)
	{
		isActive_ = isActive;
	}

	bool GetRerenderShadowMaps() const
	{
		return rerenderShadowMaps_;
	}

	void SetRerenderShadowMaps(bool renderShadows)
	{
		rerenderShadowMaps_ = renderShadows;
	}

	DeferredRenderingData* GetDeferredRenderingData() const
	{
		return deferredRenderingData_;
	}

	RenderBuffer* GetDepthRenderBuffer() const
	{
		return depthRenderbuffer_;
	}

protected:
	void GenerateBuffers();

private:
	DeferredRenderingData* deferredRenderingData_{ nullptr };
	Camera* camera_{ nullptr };
	FrameBuffer* framebuffer_{ nullptr };
	RenderBuffer* depthRenderbuffer_{ nullptr };
	Texture* texture_{ nullptr };

	Vector2 frameSize_{ 1024, 1024 };

	bool isActive_{ true };
	bool rerenderShadowMaps_{ true };
};

#endif