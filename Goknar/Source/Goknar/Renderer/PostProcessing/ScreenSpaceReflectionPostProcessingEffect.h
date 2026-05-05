#ifndef __SCREENSPACEREFLECTIONPOSTPROCESSINGEFFECT_H__
#define __SCREENSPACEREFLECTIONPOSTPROCESSINGEFFECT_H__

#include "PostProcessing.h"

class GOKNAR_API ScreenSpaceReflectionPostProcessingEffect : public PostProcessingEffect
{
public:
    ScreenSpaceReflectionPostProcessingEffect();
    ~ScreenSpaceReflectionPostProcessingEffect() override;

    void PreInit() override;
    void Init() override;
    void PostInit() override;

    Texture* Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height) override;

    void SetRayStepSize(float rayStepSize) { rayStepSize_ = rayStepSize; }
    void SetMaxSteps(int maxSteps) { maxSteps_ = maxSteps; }
    void SetThickness(float thickness) { thickness_ = thickness; }

private:
    float rayStepSize_{ 0.5f };
    int maxSteps_{ 50 };
    float thickness_{ 0.5f };
};

#endif