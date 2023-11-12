#include "pch.h"

#include "SpotLight.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

SpotLight::SpotLight() : Light(), coverageAngle_(0.f), falloffAngle_(0.f)
{
	id_ = ObjectIDManager::GetInstance()->GetAndIncreaseSpotLightID();
	name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT) + std::to_string(id_);
}

SpotLight::SpotLight(float coverage, float falloff) : SpotLight()
{
	coverageAngle_ = DEGREE_TO_RADIAN(coverage);
	falloffAngle_ = DEGREE_TO_RADIAN(falloff);
}

void SpotLight::Init()
{
	if (isShadowEnabled_)
	{
		shadowMapFramebuffer_ = new Framebuffer();

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
	}

	Light::Init();
}

void SpotLight::SetPosition(const Vector3& position)
{
	Light::SetPosition(position);

	if (isShadowEnabled_)
	{
		shadowMapRenderCamera_->SetPosition(position_);
	}
}

void SpotLight::SetDirection(const Vector3& direction)
{
	direction_ = direction.GetNormalized();

	if (isShadowEnabled_ && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetForwardVector(direction);
	}
}

void SpotLight::SetCoverageAngle(float coverageAngleInDegrees)
{
	coverageAngle_ = DEGREE_TO_RADIAN(coverageAngleInDegrees);

	if (isShadowEnabled_ && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetFOV(RADIAN_TO_DEGREE(coverageAngle_ * 2.f));
	}
}

void SpotLight::SetShaderUniforms(const Shader* shader)
{
	if (mobility_ == LightMobility::Dynamic)
	{
		std::string positionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION;
		shader->SetVector3(positionName.c_str(), position_);

		std::string directionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION;
		shader->SetVector3(directionName.c_str(), direction_);

		std::string intensityName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
		shader->SetVector3(intensityName.c_str(), intensity_ * color_);

		std::string coverageAngleName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE;
		shader->SetFloat(coverageAngleName.c_str(), coverageAngle_);

		std::string falloffAngleName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE;
		shader->SetFloat(falloffAngleName.c_str(), falloffAngle_);
	}

	if (isShadowEnabled_)
	{
		biasedShadowViewProjectionMatrix_ =
			Matrix
		{
			0.5f, 0.f, 0.f, 0.5f,
			0.f, 0.5f, 0.f, 0.5f,
			0.f, 0.f, 0.5f - 0.000001f, 0.5f,
			0.f, 0.f, 0.f, 1.f
		} * shadowMapRenderCamera_->GetViewProjectionMatrix();

		shader->SetMatrix((SHADER_VARIABLE_NAMES::SHADOW::VIEW_MATRIX_PREFIX + name_).c_str(), biasedShadowViewProjectionMatrix_);
	}
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
	shadowMapFramebuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::Shadow);

	shadowMapFramebuffer_->Unbind();

	EXIT_ON_GL_ERROR("SpotLight::RenderShadowMap");
}
