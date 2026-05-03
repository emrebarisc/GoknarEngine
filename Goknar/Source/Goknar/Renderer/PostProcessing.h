#ifndef __POSTPROCESSING_H__
#define __POSTPROCESSING_H__

#include "Goknar/Core.h"

#include "Texture.h"

class ComputeShader;
class DeferredRenderingData;
class FrameBuffer;
class Texture;

class GOKNAR_API PostProcessingEffect
{
public:
	PostProcessingEffect();
	virtual ~PostProcessingEffect();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	void SetComputeShader(ComputeShader* computeShader)
	{
		computeShader_ = computeShader;
	}

	ComputeShader* GetComputeShader() const
	{
		return computeShader_;
	}

	FrameBuffer* GetOutputFrameBuffer() const
	{
		return outputFrameBuffer_;
	}

	Texture* GetOutputTexture() const
	{
		return outputTexture_;
	}

	void SetIsEnabled(bool isEnabled)
	{
		isEnabled_ = isEnabled;
	}

	bool GetIsEnabled() const
	{
		return isEnabled_;
	}

	virtual Texture* Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height);

protected:
	void EnsureResources(int width, int height);
	void RecreateOutputResources(int width, int height, TextureInternalFormat internalFormat = TextureInternalFormat::RGBA16F, TextureFormat textureFormat = TextureFormat::RGBA, TextureType textureType = TextureType::FLOAT);
	virtual void OnRender(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height);

	ComputeShader* computeShader_{ nullptr };

	Texture* outputTexture_{ nullptr };
	FrameBuffer* outputFrameBuffer_{ nullptr };

	int width_{ 0 };
	int height_{ 0 };

private:
	bool isEnabled_{ true };
};

#endif
