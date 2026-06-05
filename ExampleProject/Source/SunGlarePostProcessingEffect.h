#pragma once

#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Renderer/PostProcessing/PostProcessing.h"

class GOKNAR_API SunGlarePostProcessingEffect : public PostProcessingEffect
{
public:
	SunGlarePostProcessingEffect();
	~SunGlarePostProcessingEffect() override = default;

	void PrepareFrame(const DeferredRenderingData* deferredRenderingData, Camera* camera) override;
	Texture* Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height) override;

	void SetSunDirection(const Vector3& sunDirection);

private:
	void OnRender(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height) override;

	Vector2 sunScreenPosition_{ 0.5f, 0.5f };
	Vector3 sunDirection_{ 0.577350f, 0.577350f, -0.577350f };
	Vector3 sunColor_{ 1.f, 0.86f, 0.55f };

	float sunVisibility_{ 0.f };
	float glareIntensity_{ 0.52f };
	float haloRadius_{ 0.2f };
	float coreRadius_{ 0.03f };
	float streakStrength_{ 0.34f };
	float sourceThreshold_{ 0.85f };
	float rayIntensity_{ 0.85f };
	float ghostIntensity_{ 0.72f };
	float ghostFade_{ 0.9f };
};
