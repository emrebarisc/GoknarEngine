#include "pch.h"

#include "DirectionalLight.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

DirectionalLight::DirectionalLight() : Light()
{
	id_ = ObjectIDManager::GetInstance()->GetAndIncreaseDirectionalLightID();
	name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT) + std::to_string(id_);
}

void DirectionalLight::Init()
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

		Vector3 lightUpVector = Vector3::Cross(Vector3::Cross(direction_, direction_.RotateVectorAroundAxis(direction_.GetOrthonormalBasis(), 1.5708f)).GetNormalized(), direction_).GetNormalized();
		shadowMapRenderCamera_ = new Camera(Vector3::ZeroVector, direction_, lightUpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Orthographic);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetImageWidth(shadowWidth_);
		shadowMapRenderCamera_->SetImageHeight(shadowHeight_);
		shadowMapRenderCamera_->SetNearPlane(Vector4{ -50.f, 50.f, -50.f, 50.f });
		shadowMapRenderCamera_->SetNearDistance(-50.f);
		shadowMapRenderCamera_->SetFarDistance(50.f);

		shadowBiasMatrix_ = 
			Matrix
			{
				0.5f, 0.f, 0.f, 0.5f,
				0.f, 0.5f, 0.f, 0.5f,
				0.f, 0.f, 0.5f, 0.5f - 0.001f,
				0.f, 0.f, 0.f, 1.f
			};
		UpdateBiasedShadowMatrix();
	}

	Light::Init();
}

void DirectionalLight::SetShaderUniforms(const Shader* shader)
{
	if (mobility_ == LightMobility::Dynamic)
	{
		std::string directionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION;
		shader->SetVector3(directionName.c_str(), direction_);

		std::string radianceName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
		shader->SetVector3(radianceName.c_str(), color_ * intensity_);

		std::string isCastingShadowName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW;
		shader->SetInt(isCastingShadowName.c_str(), isShadowEnabled_);
	}

	if (isShadowEnabled_)
	{
		shader->SetMatrix((SHADER_VARIABLE_NAMES::SHADOW::VIEW_MATRIX_PREFIX + name_).c_str(), biasedShadowViewProjectionMatrix_);
	}
}

void DirectionalLight::SetShadowRenderPassShaderUniforms(const Shader* shader)
{
}

void DirectionalLight::SetDirection(const Vector3& direction)
{
	direction_ = direction.GetNormalized();

	if (isShadowEnabled_)
	{
		shadowMapRenderCamera_->SetForwardVector(direction);
	}
}

void DirectionalLight::SetIsShadowEnabled(bool isShadowEnabled)
{
	Light::SetIsShadowEnabled(isShadowEnabled);
	if (isShadowEnabled && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetForwardVector(direction_);
	}
}


void DirectionalLight::RenderShadowMap()
{
	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = cameraManager->GetActiveCamera();

	Vector3 shadowCameraForwardVector = shadowMapRenderCamera_->GetForwardVector();
	Vector3 mainCameraForwardVector = mainCamera->GetForwardVector();
	Vector3 mainCameraPosition = mainCamera->GetPosition();
	Vector3 position{ mainCameraPosition.x, mainCameraPosition.y, mainCameraPosition.z };
	shadowMapRenderCamera_->SetPosition(position + 10.f * Vector3{ mainCameraForwardVector.x, mainCameraForwardVector.y, 0.f } - 10.f * Vector3{ shadowCameraForwardVector.x, shadowCameraForwardVector.y, 0.f });
	UpdateBiasedShadowMatrix();
	//shadowMapRenderCamera->SetPosition(Vector3{ 20.f, 0.f, 0.f } - shadowMapRenderCamera_->GetForwardVector() * 25.f);

	cameraManager->SetActiveCamera(shadowMapRenderCamera_);
	shadowMapFramebuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::Shadow);

	// For outputing only!
	//shadowMapTexture_->ReadFromFramebuffer(directionalLight->GetShadowMapFBO());
	//shadowMapTexture_->Save(CONTENT_DIR + directionalLight->GetName() + "FrameBufferTexture.png");

	shadowMapFramebuffer_->Unbind();

	EXIT_ON_GL_ERROR("DirectionalLight::RenderShadowMap");
}
