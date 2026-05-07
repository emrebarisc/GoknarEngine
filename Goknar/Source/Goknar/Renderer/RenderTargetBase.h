#ifndef __RENDERTARGETBASE_H__
#define __RENDERTARGETBASE_H__

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"

class Camera;
class FrameBuffer;
class Texture;

class GOKNAR_API RenderTargetBase
{
public:
	RenderTargetBase() = default;
	virtual ~RenderTargetBase();

	virtual void Init() = 0;
	virtual void SetFrameSize(const Vector2& frameSize) = 0;

	void SetCamera(Camera* camera)
	{
		camera_ = camera;
		ownsCamera_ = false;
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

	const Vector2& GetFrameSize() const
	{
		return frameSize_;
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

protected:
	Camera* camera_{ nullptr };
	FrameBuffer* framebuffer_{ nullptr };
	Texture* texture_{ nullptr };
	bool ownsCamera_{ false };

	Vector2 frameSize_{ 1024, 1024 };

	bool isActive_{ true };
	bool rerenderShadowMaps_{ true };
};

#endif
