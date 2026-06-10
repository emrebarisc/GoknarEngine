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

    void SetRayStepSize(float rayStepSize) { rayStepSize_ = rayStepSize > 0.001f ? rayStepSize : 0.001f; }
    void SetMaxSteps(int maxSteps) { maxSteps_ = maxSteps > 0 ? maxSteps : 1; }
    void SetThickness(float thickness) { thickness_ = thickness > 0.001f ? thickness : 0.001f; }

private:
    float rayStepSize_{ 0.5f };
    int maxSteps_{ 1 };
    float thickness_{ 0.5f };
};

#endif