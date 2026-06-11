#ifndef __RENDERTARGET_H__
#define __RENDERTARGET_H__

#include "RenderTargetBase.h"

class Camera;
class DeferredRenderingData;
class FrameBuffer;
class RenderBuffer;
class Texture;

class GOKNAR_API RenderTarget : public RenderTargetBase
{
public:
	RenderTarget();
	~RenderTarget() override;

	void Init() override;

	void SetFrameSize(const Vector2& frameSize) override;

	DeferredRenderingData* GetDeferredRenderingData() const
	{
		return deferredRenderingData_;
	}

	RenderBuffer* GetDepthRenderBuffer() const
	{
		return depthRenderbuffer_;
	}

	void SetRequirePostProcessingEffects(bool requirePostProcessingEffect)
	{
		requirePostProcessingEffects_ = requirePostProcessingEffect;
	}

	bool GetRequirePostProcessingEffects() const
	{
		return requirePostProcessingEffects_;
	}

protected:
	virtual void GenerateBuffers();

	DeferredRenderingData* deferredRenderingData_{ nullptr };
	RenderBuffer* depthRenderbuffer_{ nullptr };

	bool requirePostProcessingEffects_{ false };
};

#endif
