#include "pch.h"

#include "SpotLight.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/FrameBuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

SpotLight::SpotLight() : Light(), coverageAngle_(0.f), falloffAngle_(0.f)
{
	id_ = ObjectIDManager::GetInstance()->GetAndIncreaseSpotLightID();
	name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT) + std::to_string(id_);

	engine->GetApplication()->GetMainScene()->AddSpotLight(this);
}

SpotLight::SpotLight(float coverage, float falloff) : SpotLight()
{
	coverageAngle_ = DEGREE_TO_RADIAN(coverage);
	falloffAngle_ = DEGREE_TO_RADIAN(falloff);
}

SpotLight::~SpotLight()
{
	engine->GetApplication()->GetMainScene()->RemoveSpotLight(this);
}

void SpotLight::PreInit()
{
	if (isShadowEnabled_)
	{
		shadowMapFrameBuffer_ = new FrameBuffer();

		shadowMapTexture_ = new Texture();
		shadowMapTexture_->SetName(SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + name_);
		shadowMapTexture_->SetTextureBindTarget(TextureBindTarget::TEXTURE_2D);
		shadowMapTexture_->SetTextureImageTarget(TextureImageTarget::TEXTURE_2D);
		shadowMapTexture_->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureFormat(TextureFormat::DEPTH);
		shadowMapTexture_->SetTextureMinFilter(TextureMinFilter::LINEAR);
		shadowMapTexture_->SetTextureMagFilter(TextureMagFilter::LINEAR);
		shadowMapTexture_->SetTextureCompareMode(TextureCompareMode::COMPARE_REF_TO_TEXTURE);
		shadowMapTexture_->SetTextureCompareFunc(TextureCompareFunc::LEQUAL);

		Vector3 lightUpVector = Vector3::Cross(Vector3::Cross(direction_.RotateVectorAroundAxis(direction_.GetOrthonormalBasis(), 1.5708f), direction_).GetNormalized(), direction_).GetNormalized();
		shadowMapRenderCamera_ = new Camera(position_, direction_, lightUpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Perspective);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetImageWidth(shadowWidth_);
		shadowMapRenderCamera_->SetImageHeight(shadowHeight_);
		shadowMapRenderCamera_->SetNearDistance(0.01f);
		shadowMapRenderCamera_->SetFarDistance(radius_);
		shadowMapRenderCamera_->SetFOV(RADIAN_TO_DEGREE(coverageAngle_ * 2.f));

		shadowBiasMatrix_ =
			Matrix
		{
			0.5f, 0.f, 0.f, 0.5f,
			0.f, 0.5f, 0.f, 0.5f,
			0.f, 0.f, 0.5f, 0.5f - 0.000005f,
			0.f, 0.f, 0.f, 1.f
		};
		UpdateBiasedShadowMatrix();
	}

	Light::PreInit();
}

void SpotLight::Init()
{
	Light::Init();
}

void SpotLight::PostInit()
{
	Light::PostInit();
}

void SpotLight::SetPosition(const Vector3& position)
{
	Light::SetPosition(position);

	if (isShadowEnabled_)
	{
		shadowMapRenderCamera_->SetPosition(position_);
		UpdateBiasedShadowMatrix();
	}
}

void SpotLight::SetDirection(const Vector3& direction)
{
	direction_ = direction.GetNormalized();

	if (isShadowEnabled_ && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetForwardVector(direction);
		UpdateBiasedShadowMatrix();
	}
}

void SpotLight::SetCoverageAngle(float coverageAngleInDegrees)
{
	coverageAngle_ = DEGREE_TO_RADIAN(coverageAngleInDegrees);

	if (isShadowEnabled_ && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetFOV(RADIAN_TO_DEGREE(coverageAngle_ * 2.f));
		UpdateBiasedShadowMatrix();
	}
}

void SpotLight::SetShaderUniforms(const Shader* shader)
{
	Light::SetShaderUniforms(shader);
}

void SpotLight::SetShadowRenderPassShaderUniforms(const Shader* shader)
{
}

void SpotLight::SetIsShadowEnabled(bool isShadowEnabled)
{
	Light::SetIsShadowEnabled(isShadowEnabled);
	if (isShadowEnabled && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetPosition(position_);
		shadowMapRenderCamera_->SetForwardVector(direction_);
	}
}

void SpotLight::RenderShadowMap()
{
	engine->GetCameraManager()->SetActiveCamera(shadowMapRenderCamera_);
	shadowMapFrameBuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::Shadow);

	shadowMapFrameBuffer_->Unbind();

	EXIT_ON_GL_ERROR("SpotLight::RenderShadowMap");
}
