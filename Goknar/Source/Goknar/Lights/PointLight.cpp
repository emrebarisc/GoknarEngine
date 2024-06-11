#include "pch.h"

#include "PointLight.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

void PointLight::PreInit()
{
	if (isShadowEnabled_)
	{
		shadowMapFramebuffer_ = new Framebuffer();

		shadowMapTexture_ = new Texture();
		shadowMapTexture_->SetName(SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + name_);
		shadowMapTexture_->SetTextureBindTarget(TextureBindTarget::TEXTURE_CUBE_MAP);
		shadowMapTexture_->SetTextureImageTarget(TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X);
		shadowMapTexture_->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureWrappingR(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureFormat(TextureFormat::DEPTH);
		shadowMapTexture_->SetTextureInternalFormat(TextureInternalFormat::DEPTH);
		shadowMapTexture_->SetTextureCompareMode(TextureCompareMode::NONE);
		shadowMapTexture_->SetTextureMinFilter(TextureMinFilter::NEAREST);
		shadowMapTexture_->SetTextureMagFilter(TextureMagFilter::NEAREST);

		//shadowMapTexture_->SetTextureCompareMode(TextureCompareMode::COMPARE_REF_TO_TEXTURE);
		//shadowMapTexture_->SetTextureCompareFunc(TextureCompareFunc::LEQUAL);
		//shadowMapTexture_->SetTextureMinFilter(TextureMinFilter::LINEAR);
		//shadowMapTexture_->SetTextureMagFilter(TextureMagFilter::LINEAR);

		shadowMapRenderCamera_ = new Camera(position_, Vector3::ForwardVector, Vector3::UpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Perspective);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetImageWidth(shadowWidth_);
		shadowMapRenderCamera_->SetImageHeight(shadowHeight_);
		shadowMapRenderCamera_->SetNearDistance(0.1f);
		shadowMapRenderCamera_->SetFarDistance(radius_);
		shadowMapRenderCamera_->SetFOV(90.f);
		shadowMapRenderCamera_->SetPosition(position_);

		UpdateShadowViewProjectionMatrices();
	}

	Light::PreInit();
}

void PointLight::Init()
{
	Light::Init();
}

void PointLight::PostInit()
{
	Light::PostInit();
}

void PointLight::RenderShadowMap()
{
	CameraManager* cameraManager = engine->GetCameraManager();
	cameraManager->SetActiveCamera(shadowMapRenderCamera_);

	shadowMapFramebuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::PointLightShadow);

	shadowMapFramebuffer_->Unbind();
}

void PointLight::SetShaderUniforms(const Shader* shader)
{
	Light::SetShaderUniforms(shader);
}

void PointLight::SetShadowRenderPassShaderUniforms(const Shader* shader)
{
	shader->SetVector3(SHADER_VARIABLE_NAMES::SHADOW::LIGHT_POSITION, position_);
	shader->SetFloat(SHADER_VARIABLE_NAMES::SHADOW::LIGHT_RADIUS, radius_);
	shader->SetMatrixArray(SHADER_VARIABLE_NAMES::SHADOW::POINT_LIGHT_VIEW_MATRICES_ARRAY, &viewMatrices_[0], 6);
}

void PointLight::SetPosition(const Vector3& position)
{
	Light::SetPosition(position);

	shadowMapRenderCamera_->SetPosition(position);

	if (isShadowEnabled_)
	{
		UpdateShadowViewProjectionMatrices();
	}
}

void PointLight::SetIsShadowEnabled(bool isShadowEnabled)
{
	Light::SetIsShadowEnabled(isShadowEnabled);
	if (isShadowEnabled && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetPosition(position_);
	}
}

void PointLight::UpdateShadowViewProjectionMatrices()
{
	if (!shadowMapRenderCamera_)
	{
		return;
	}

	shadowMapRenderCamera_->SetVectors(Vector3{ 1.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, -1.0f }, Vector3{ 0.0f, -1.0f, 0.0f });
	viewMatrices_[0] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(Vector3{ -1.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 1.0f }, Vector3{ 0.0f,-1.0f, 0.0f });
	viewMatrices_[1] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(Vector3{ 0.0f, 1.0f, 0.0f }, Vector3{ 1.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 1.0f });
	viewMatrices_[2] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(Vector3{ 0.0f,-1.0f, 0.0f }, Vector3{ 1.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f,-1.0f });
	viewMatrices_[3] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(Vector3{ 0.0f, 0.0f, 1.0f }, Vector3{ 1.0f, 0.0f, 0.0f }, Vector3{ 0.0f,-1.0f, 0.0f });
	viewMatrices_[4] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(Vector3{ 0.0f, 0.0f,-1.0f }, Vector3{ -1.0f, 0.0f, 0.0f }, Vector3{ 0.0f,-1.0f, 0.0f });
	viewMatrices_[5] = shadowMapRenderCamera_->GetViewProjectionMatrix();
}
