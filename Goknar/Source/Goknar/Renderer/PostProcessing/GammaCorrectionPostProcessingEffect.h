#ifndef __GAMMACORRECTIONPOSTPROCESSINGEFFECT_H__
#define __GAMMACORRECTIONPOSTPROCESSINGEFFECT_H__

#include "PostProcessing.h"

class GOKNAR_API GammaCorrectionPostProcessingEffect : public PostProcessingEffect
{
public:
	GammaCorrectionPostProcessingEffect();

	void SetGamma(float gamma);

	float GetGamma() const
	{
		return gamma_;
	}

protected:
	void OnRender(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height) override;

private:
	float gamma_{ 2.2f };
};

#endif
