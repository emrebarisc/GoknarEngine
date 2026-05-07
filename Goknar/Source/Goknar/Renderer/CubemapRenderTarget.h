#ifndef __CUBEMAPRENDERTARGET_H__
#define __CUBEMAPRENDERTARGET_H__

#include "RenderTargetBase.h"

class Texture;

class GOKNAR_API CubemapRenderTarget : public RenderTargetBase
{
public:
	CubemapRenderTarget() = default;
	~CubemapRenderTarget() override;

	void Init() override;
	void SetFrameSize(const Vector2& frameSize) override;

	Texture* GetDepthTexture() const
	{
		return depthTexture_;
	}

	void GenerateMipmaps() const;

protected:
	void GenerateBuffers();

private:
	Texture* depthTexture_{ nullptr };
};

#endif
