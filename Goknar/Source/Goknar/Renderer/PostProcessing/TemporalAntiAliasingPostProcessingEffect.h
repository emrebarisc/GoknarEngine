#ifndef __TEMPORALANTIALIASINGPOSTPROCESSINGEFFECT_H__
#define __TEMPORALANTIALIASINGPOSTPROCESSINGEFFECT_H__

#include "PostProcessing.h"

#include "Goknar/Math/Matrix.h"

#include <cstdint>
#include <unordered_map>

class Camera;
class FrameBuffer;
class Texture;

struct TemporalAntiAliasingRenderContext
{
	Texture* outputTexture{ nullptr };
	FrameBuffer* outputFrameBuffer{ nullptr };

	Texture* historyTextures[2]{ nullptr, nullptr };
	FrameBuffer* historyFrameBuffers[2]{ nullptr, nullptr };

	Matrix previousViewProjectionMatrix{ Matrix::IdentityMatrix };
	Vector2 previousJitterUv{ Vector2::ZeroVector };

	const Camera* lastCamera{ nullptr };

	int width{ 0 };
	int height{ 0 };
	int readHistoryIndex{ 0 };

	std::uint64_t frameIndex{ 0 };

	bool hasValidHistory{ false };
};

class GOKNAR_API TemporalAntiAliasingPostProcessingEffect : public PostProcessingEffect
{
public:
	TemporalAntiAliasingPostProcessingEffect();
	~TemporalAntiAliasingPostProcessingEffect() override;

	void PrepareFrame(const DeferredRenderingData* deferredRenderingData, Camera* camera) override;
	Texture* Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height) override;

private:
	void EnsureContextResources(TemporalAntiAliasingRenderContext& renderContext, int width, int height);
	void RecreateContextResources(TemporalAntiAliasingRenderContext& renderContext, int width, int height);
	void DestroyContextResources(TemporalAntiAliasingRenderContext& renderContext);
	void DestroyAllContexts();

	Vector2 GetHaltonJitter(std::uint64_t frameIndex) const;

	std::unordered_map<const DeferredRenderingData*, TemporalAntiAliasingRenderContext> renderContexts_{};

	float jitterScale_{ 0.01f };
	float maxHistoryWeight_{ 0.96f };
	float minHistoryWeight_{ 0.40f };
	float motionWeightScale_{ 0.05f };
};

#endif
