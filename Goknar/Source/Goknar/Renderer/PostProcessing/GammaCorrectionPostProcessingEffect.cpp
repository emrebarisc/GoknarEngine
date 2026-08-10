#include "pch.h"

#include "GammaCorrectionPostProcessingEffect.h"

#include "Renderer/ComputeShader.h"

GammaCorrectionPostProcessingEffect::GammaCorrectionPostProcessingEffect()
{
	ComputeShader* gammaCorrectionComputeShader = new ComputeShader();

#if defined(GOKNAR_BUILD_DEBUG)
	const std::string shaderDirectory = EngineContentDir + "Shaders/PostProcessing/";
#else
	const std::string shaderDirectory = ContentDir + "Shaders/PostProcessing/";
#endif

	gammaCorrectionComputeShader->SetComputeShaderPathAbsolute(shaderDirectory + "GammaCorrection.comp");
	SetComputeShader(gammaCorrectionComputeShader);
}

void GammaCorrectionPostProcessingEffect::SetGamma(float gamma)
{
	if (0.f < gamma)
	{
		gamma_ = gamma;
	}
}

void GammaCorrectionPostProcessingEffect::OnRender(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	GetComputeShader()->SetFloat("inverseGamma", 1.f / gamma_);

	PostProcessingEffect::OnRender(deferredRenderingData, inputTexture, width, height);
}
