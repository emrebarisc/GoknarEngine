#include "SunGlarePostProcessingEffect.h"

#include <cmath>

#include "Goknar/Camera.h"
#include "Goknar/Application.h"
#include "Goknar/Core.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Renderer/ComputeShader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"

namespace
{
	constexpr int kSunGlareOutputImageUnit = 0;
	constexpr float kSunProjectionDistanceScale = 0.85f;
	constexpr float kOffscreenFadeRange = 0.18f;

	int BindTextureUsingEngineTextureUnit(const Texture* texture)
	{
		const int textureUnit = static_cast<int>(texture->GetRendererTextureId());
		texture->BindToTextureUnit(textureUnit);
		return textureUnit;
	}

	float Saturate(float value)
	{
		return GoknarMath::Clamp(value, 0.f, 1.f);
	}

	bool IsFinite(const Vector2& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y);
	}

	float GetOffscreenFade(float coordinate)
	{
		const float distanceOutsideScreen = GoknarMath::Max(GoknarMath::Abs(coordinate - 0.5f) - 0.5f, 0.f);
		return 1.f - Saturate(distanceOutsideScreen / kOffscreenFadeRange);
	}

	bool GetFirstDirectionalLightDirection(Vector3& outDirection)
	{
		if (!engine || !engine->GetApplication() || !engine->GetApplication()->GetMainScene())
		{
			return false;
		}

		const std::vector<DirectionalLight*>& directionalLights = engine->GetApplication()->GetMainScene()->GetDirectionalLights();
		if (directionalLights.empty() || !directionalLights[0])
		{
			return false;
		}

		const Vector3 direction = directionalLights[0]->GetDirection().GetNormalized();
		if (direction.SquareLength() <= SMALLER_EPSILON)
		{
			return false;
		}

		outDirection = direction;
		return true;
	}
}

SunGlarePostProcessingEffect::SunGlarePostProcessingEffect()
{
	ComputeShader* sunGlareComputeShader = new ComputeShader();
	sunGlareComputeShader->SetComputeShaderPathAbsolute(ContentDir + "Shaders/PostProcessing/SunGlare.comp");
	SetComputeShader(sunGlareComputeShader);
}

void SunGlarePostProcessingEffect::PrepareFrame(const DeferredRenderingData* deferredRenderingData, Camera* camera)
{
	(void)deferredRenderingData;

	sunVisibility_ = 0.f;
	if (!camera)
	{
		return;
	}

	Vector3 firstDirectionalLightDirection;
	if (GetFirstDirectionalLightDirection(firstDirectionalLightDirection))
	{
		sunDirection_ = firstDirectionalLightDirection;
	}

	const Vector3 sunWorldDirection = (-sunDirection_).GetNormalized();
	if (sunWorldDirection.SquareLength() <= SMALLER_EPSILON)
	{
		return;
	}

	const float facing = Vector3::Dot(camera->GetForwardVector().GetNormalized(), sunWorldDirection);
	if (facing <= SMALLER_EPSILON)
	{
		return;
	}

	const float projectionDistance = GoknarMath::Max(
		camera->GetNearDistance() + 1.f,
		camera->GetFarDistance() * kSunProjectionDistanceScale);
	const Vector3 sunWorldPosition = camera->GetPosition() + sunWorldDirection * projectionDistance;
	const Vector4 clipPosition = camera->GetViewProjectionMatrix() * Vector4(sunWorldPosition, 1.f);
	if (clipPosition.w <= SMALLER_EPSILON)
	{
		return;
	}

	const Vector2 normalizedDevicePosition(clipPosition.x / clipPosition.w, clipPosition.y / clipPosition.w);
	if (!IsFinite(normalizedDevicePosition))
	{
		return;
	}

	sunScreenPosition_ = normalizedDevicePosition * 0.5f + Vector2(0.5f);

	const float facingFade = Saturate((facing - 0.05f) / 0.45f);
	sunVisibility_ = facingFade * GetOffscreenFade(sunScreenPosition_.x) * GetOffscreenFade(sunScreenPosition_.y);
}

Texture* SunGlarePostProcessingEffect::Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	if (!GetIsEnabled() || !deferredRenderingData || !inputTexture || width <= 0 || height <= 0 || sunVisibility_ <= SMALLER_EPSILON || glareIntensity_ <= 0.f)
	{
		return const_cast<Texture*>(inputTexture);
	}

	EnsureResources(width, height);
	OnRender(deferredRenderingData, inputTexture, width, height);

	return outputTexture_;
}

void SunGlarePostProcessingEffect::SetSunDirection(const Vector3& sunDirection)
{
	const Vector3 normalizedSunDirection = sunDirection.GetNormalized();
	if (normalizedSunDirection.SquareLength() <= SMALLER_EPSILON)
	{
		return;
	}

	sunDirection_ = normalizedSunDirection;
}

void SunGlarePostProcessingEffect::OnRender(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	(void)deferredRenderingData;

	GetComputeShader()->Use();
	GetComputeShader()->SetInt("inputTexture", BindTextureUsingEngineTextureUnit(inputTexture));
	GetComputeShader()->SetVector2("sunScreenPosition", sunScreenPosition_);
	GetComputeShader()->SetVector3("sunColor", sunColor_);
	GetComputeShader()->SetFloat("sunVisibility", sunVisibility_);
	GetComputeShader()->SetFloat("glareIntensity", glareIntensity_);
	GetComputeShader()->SetFloat("haloRadius", haloRadius_);
	GetComputeShader()->SetFloat("coreRadius", coreRadius_);
	GetComputeShader()->SetFloat("streakStrength", streakStrength_);
	GetComputeShader()->SetFloat("sourceThreshold", sourceThreshold_);
	GetComputeShader()->SetFloat("rayIntensity", rayIntensity_);
	GetComputeShader()->SetFloat("ghostIntensity", ghostIntensity_);
	GetComputeShader()->SetFloat("ghostFade", ghostFade_);
	outputTexture_->BindAsImage(kSunGlareOutputImageUnit, TextureImageAccess::WRITE_ONLY);
	GetComputeShader()->Dispatch2D(width, height);
	engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);
}
