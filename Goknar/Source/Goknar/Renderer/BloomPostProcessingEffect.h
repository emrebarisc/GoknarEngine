#ifndef __BLOOMPOSTPROCESSINGEFFECT_H__
#define __BLOOMPOSTPROCESSINGEFFECT_H__

#include "PostProcessing.h"

#include <vector>

class ComputeShader;
class FrameBuffer;
class Texture;

struct BloomMipLevel
{
	Texture* texture{ nullptr };
	FrameBuffer* frameBuffer{ nullptr };
	int width{ 0 };
	int height{ 0 };
};

class GOKNAR_API BloomPostProcessingEffect : public PostProcessingEffect
{
public:
	BloomPostProcessingEffect();
	~BloomPostProcessingEffect() override;

	void PreInit() override;
	void Init() override;
	void PostInit() override;

	Texture* Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height) override;

private:
	void EnsureBloomResources(int width, int height);
	void RecreateBloomResources(int width, int height);
	void DestroyBloomResources();

	ComputeShader* bloomPrefilterComputeShader_{ nullptr };
	ComputeShader* bloomDownsampleComputeShader_{ nullptr };
	ComputeShader* bloomUpsampleComputeShader_{ nullptr };

	std::vector<BloomMipLevel> bloomDownsampleMipChain_{};
	std::vector<BloomMipLevel> bloomUpsampleMipChain_{};

	float bloomThreshold_{ 1.f };
	float bloomIntensity_{ 0.8f };
	float bloomSoftKnee_{ 0.75f };
	float bloomBrightnessBoost_{ 1.5f };
	float bloomScatter_{ 0.7f };
	float bloomMaxIntensityBoost_{ 2.f };
	float bloomIntensityCurve_{ 0.85f };
	int maxBloomMipCount_{ 6 };
	int bloomBaseResolutionDivisor_{ 2 };
};

#endif
