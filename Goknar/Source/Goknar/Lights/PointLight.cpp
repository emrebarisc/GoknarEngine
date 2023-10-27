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

void PointLight::Init()
{
	if (isShadowEnabled_)
	{
		shadowMapFramebuffer_ = new Framebuffer();

		shadowMapTexture_ = new Texture();
		shadowMapTexture_->SetTextureBindTarget(TextureBindTarget::TEXTURE_CUBE_MAP);
		shadowMapTexture_->SetTextureImageTarget(TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X);
		shadowMapTexture_->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureWrappingR(TextureWrapping::CLAMP_TO_EDGE);

		shadowMapTexture_->SetName(LIGHT::SHADOW_MAP_PREFIX + name_);

		shadowMapRenderCamera_ = new Camera(position_, Vector3::ForwardVector, Vector3::UpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Perspective);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetImageWidth(shadowWidth_);
		shadowMapRenderCamera_->SetImageHeight(shadowHeight_);
		shadowMapRenderCamera_->SetNearDistance(1.f);
		shadowMapRenderCamera_->SetFarDistance(radius_);
	}

	Light::Init();
}

void PointLight::RenderShadowMap()
{
	Camera* shadowMapRenderCamera = GetShadowMapRenderCamera();

	CameraManager* cameraManager = engine->GetCameraManager();
	cameraManager->SetActiveCamera(shadowMapRenderCamera);

	glViewport(0, 0, shadowMapTexture_->GetWidth(), shadowMapTexture_->GetHeight());
	shadowMapFramebuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::Shadow);

	shadowMapFramebuffer_->Unbind();
}

void PointLight::SetShaderUniforms(const Shader* shader) const
{
	if (GetLightMobility() == LightMobility::Movable)
	{
		std::string positionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION;
		shader->SetVector3(positionName.c_str(), position_);

		std::string intensityName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
		shader->SetVector3(intensityName.c_str(), intensity_);

		std::string radiusName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS;
		shader->SetVector3(radiusName.c_str(), radius_);
	}
}
