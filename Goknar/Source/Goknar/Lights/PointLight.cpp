#include "pch.h"

#include "PointLight.h"

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

PointLight::PointLight() : Light()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreasePointLightGUID();
	name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT);

	radius_ = 25.f;

	shadowIntensity_ = 0.f;

	engine->GetApplication()->GetMainScene()->AddPointLight(this);
}

PointLight::~PointLight()
{
	DeallocateViewMatrices();

	engine->GetApplication()->GetMainScene()->RemovePointLight(this);
}

void PointLight::PreInit()
{
	if (isShadowEnabled_)
	{
		shadowMapFrameBuffer_ = new FrameBuffer();

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

		AllocateViewMatrices();
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

	shadowMapFrameBuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::PointLightShadow);

	shadowMapFrameBuffer_->Unbind();
}

void PointLight::SetShaderUniforms(const Shader* shader)
{
	Light::SetShaderUniforms(shader);
}

void PointLight::SetShadowRenderPassShaderUniforms(const Shader* shader)
{
	shader->SetVector3(SHADER_VARIABLE_NAMES::SHADOW::LIGHT_POSITION, position_);
	shader->SetFloat(SHADER_VARIABLE_NAMES::SHADOW::LIGHT_RADIUS, radius_);
	shader->SetMatrixArray(SHADER_VARIABLE_NAMES::SHADOW::POINT_LIGHT_VIEW_MATRICES_ARRAY, viewMatrices_, 6);
}

void PointLight::SetPosition(const Vector3& position)
{
	Light::SetPosition(position);

	if (isShadowEnabled_)
	{
		shadowMapRenderCamera_->SetPosition(position);
		UpdateShadowViewProjectionMatrices();
	}
}

void PointLight::SetIsShadowEnabled(bool isShadowEnabled)
{
	if (isShadowEnabled_ == isShadowEnabled)
	{
		return;
	}

	Light::SetIsShadowEnabled(isShadowEnabled);
	if (isShadowEnabled)
	{
		AllocateViewMatrices();

		if (shadowMapRenderCamera_)
		{
			shadowMapRenderCamera_->SetPosition(position_);
			UpdateShadowViewProjectionMatrices();
		}
	}
	else
	{
		DeallocateViewMatrices();
	}
}

void PointLight::AllocateViewMatrices()
{
	if (!viewMatrices_)
	{
		viewMatrices_ = new Matrix[6];
	}
}

void PointLight::DeallocateViewMatrices()
{
	delete[] viewMatrices_;
	viewMatrices_ = nullptr;
}

void PointLight::UpdateShadowViewProjectionMatrices()
{
	shadowMapRenderCamera_->SetVectors(
		Vector3{ 1.f, 0.f, 0.f }, 
		Vector3{ 0.f, 0.f, -1.f }, 
		Vector3{ 0.f, -1.f, 0.f },
		true);
	viewMatrices_[0] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(
		Vector3{ -1.f, 0.f, 0.f },
		Vector3{ 0.f, 0.f, 1.f },
		Vector3{ 0.f, -1.f, 0.f },
		true);
	viewMatrices_[1] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(
		Vector3{ 0.f, 1.f, 0.f },
		Vector3{ 1.f, 0.f, 0.f },
		Vector3{ 0.f, 0.f, 1.f },
		true);
	viewMatrices_[2] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(
		Vector3{ 0.f,-1.f, 0.f },
		Vector3{ 1.f, 0.f, 0.f },
		Vector3{ 0.f, 0.f,-1.f },
		true);
	viewMatrices_[3] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(
		Vector3{ 0.f, 0.f, 1.f },
		Vector3{ 1.f, 0.f, 0.f },
		Vector3{ 0.f,-1.f, 0.f },
		true);
	viewMatrices_[4] = shadowMapRenderCamera_->GetViewProjectionMatrix();

	shadowMapRenderCamera_->SetVectors(
		Vector3{ 0.f, 0.f,-1.f },
		Vector3{ -1.f, 0.f, 0.f },
		Vector3{ 0.f,-1.f, 0.f },
		true);
	viewMatrices_[5] = shadowMapRenderCamera_->GetViewProjectionMatrix();
}
