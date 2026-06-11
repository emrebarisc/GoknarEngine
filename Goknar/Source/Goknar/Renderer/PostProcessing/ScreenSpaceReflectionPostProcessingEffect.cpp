#include "pch.h"
#include "ScreenSpaceReflectionPostProcessingEffect.h"
#include "Goknar/Renderer/ComputeShader.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Camera.h"
#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Managers/CameraManager.h"

#include "Goknar/Profiling/ProfileMacros.h"

ScreenSpaceReflectionPostProcessingEffect::ScreenSpaceReflectionPostProcessingEffect()
{
	ComputeShader* ssrComputeShader = new ComputeShader();
	const std::string projectShaderPath = ContentDir + "Shaders/PostProcessing/ScreenSpaceReflection.comp";
	std::string shaderPath = projectShaderPath;

#if defined(ENGINE_CONTENT_DIR)
	if (!DataEncryption::FileExists(projectShaderPath))
	{
		const std::string engineShaderPath = EngineContentDir + "Shaders/PostProcessing/ScreenSpaceReflection.comp";

		if (DataEncryption::FileExists(engineShaderPath))
		{
			shaderPath = engineShaderPath;
		}
	}
#endif
	ssrComputeShader->SetComputeShaderPathAbsolute(shaderPath);
	SetComputeShader(ssrComputeShader);
}

ScreenSpaceReflectionPostProcessingEffect::~ScreenSpaceReflectionPostProcessingEffect()
{
}

void ScreenSpaceReflectionPostProcessingEffect::PreInit()
{
	PostProcessingEffect::PreInit();
}

void ScreenSpaceReflectionPostProcessingEffect::Init()
{
	PostProcessingEffect::Init();
}

void ScreenSpaceReflectionPostProcessingEffect::PostInit()
{
	PostProcessingEffect::PostInit();
}

Texture* ScreenSpaceReflectionPostProcessingEffect::Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	GOKNAR_PROFILE_FUNCTION();

	if (!GetIsEnabled() || !GetComputeShader() || !deferredRenderingData || !inputTexture || width <= 0 || height <= 0)
	{
		return const_cast<Texture*>(inputTexture);
	}

	{
		GOKNAR_PROFILE_SCOPE("ScreenSpaceReflectionPostProcessingEffect::EnsureResources");
		EnsureResources(width, height);
	}

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	if (!activeCamera || !deferredRenderingData->geometryBufferData)
	{
		return const_cast<Texture*>(inputTexture);
	}

	Texture* worldPositionTexture = deferredRenderingData->geometryBufferData->worldPositionTexture;
	Texture* worldNormalTexture = deferredRenderingData->geometryBufferData->worldNormalTexture;
	Texture* aoMetallicRoughnessTexture = deferredRenderingData->geometryBufferData->ambientOcclusionMetallicRoughnessTexture;

	if (!worldPositionTexture || !worldNormalTexture || !aoMetallicRoughnessTexture)
	{
		return const_cast<Texture*>(inputTexture);
	}

	constexpr int inputTextureUnit = 0;
	constexpr int worldPositionTextureUnit = 1;
	constexpr int worldNormalTextureUnit = 2;
	constexpr int aoMetallicRoughnessTextureUnit = 3;

	GetComputeShader()->Use();

	inputTexture->BindToTextureUnit(inputTextureUnit);
	GetComputeShader()->SetInt("inputTexture", inputTextureUnit);

	worldPositionTexture->BindToTextureUnit(worldPositionTextureUnit);
	GetComputeShader()->SetInt("position_GBuffer", worldPositionTextureUnit);

	worldNormalTexture->BindToTextureUnit(worldNormalTextureUnit);
	GetComputeShader()->SetInt("normal_GBuffer", worldNormalTextureUnit);

	aoMetallicRoughnessTexture->BindToTextureUnit(aoMetallicRoughnessTextureUnit);
	GetComputeShader()->SetInt("aoMetallicRoughness_GBuffer", aoMetallicRoughnessTextureUnit);

	GetComputeShader()->SetMatrix("viewProjectionMatrix", activeCamera->GetViewProjectionMatrix());
	GetComputeShader()->SetVector3("viewPosition", activeCamera->GetPosition());

	GetComputeShader()->SetFloat("rayStepSize", rayStepSize_);
	GetComputeShader()->SetInt("maxSteps", maxSteps_);
	GetComputeShader()->SetFloat("thickness", thickness_);

	outputTexture_->BindAsImage(0, TextureImageAccess::WRITE_ONLY);
	GetComputeShader()->Dispatch2D(width, height);

	engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);

	return outputTexture_;
}
