#include "pch.h"

#include "DirectionalLight.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Lights/LightManager/LightManager.h"

DirectionalLight::DirectionalLight() : Light()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseDirectionalLightGUID();
	name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT);

	engine->GetApplication()->GetMainScene()->AddDirectionalLight(this);
}

DirectionalLight::~DirectionalLight()
{
	engine->GetApplication()->GetMainScene()->RemoveDirectionalLight(this);
}

void DirectionalLight::PreInit()
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

		Vector3 lightUpVector = Vector3::Cross(Vector3::Cross(direction_, direction_.RotateVectorAroundAxis(direction_.GetOrthonormalBasis(), 1.5708f)).GetNormalized(), direction_).GetNormalized();
		shadowMapRenderCamera_ = new Camera(Vector3::ZeroVector, direction_, lightUpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Orthographic);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetImageWidth(shadowWidth_);
		shadowMapRenderCamera_->SetImageHeight(shadowHeight_);
		shadowMapRenderCamera_->SetNearPlane(Vector4{ -20.f, 20.f, -20.f, 20.f });
		shadowMapRenderCamera_->SetNearDistance(-50.f);
		shadowMapRenderCamera_->SetFarDistance(50.f);

		shadowBiasMatrix_ =
			Matrix
		{
			0.5f, 0.f, 0.f, 0.5f,
			0.f, 0.5f, 0.f, 0.5f,
			0.f, 0.f, 0.5f, 0.5f - 0.00001f,
			0.f, 0.f, 0.f, 1.f
		};
		UpdateBiasedShadowMatrix();
	}

	Light::PreInit();
}

void DirectionalLight::Init()
{
	Light::Init();
}

void DirectionalLight::PostInit()
{
	Light::PostInit();
}

void DirectionalLight::SetShaderUniforms(const Shader* shader)
{
	Light::SetShaderUniforms(shader);
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
	shadowMapRenderCamera_->SetPosition(position + 15.f * mainCameraForwardVector - 10.f * shadowCameraForwardVector);
	UpdateBiasedShadowMatrix();
	//shadowMapRenderCamera->SetPosition(Vector3{ 20.f, 0.f, 0.f } - shadowMapRenderCamera_->GetForwardVector() * 25.f);

	cameraManager->SetActiveCamera(shadowMapRenderCamera_);
	shadowMapFrameBuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::Shadow);

	// For outputing only!
	//shadowMapTexture_->ReadFromFrameBuffer(directionalLight->GetShadowMapFBO());
	//shadowMapTexture_->Save(CONTENT_DIR + directionalLight->GetName() + "FrameBufferTexture.png");

	shadowMapFrameBuffer_->Unbind();

	EXIT_ON_GL_ERROR("DirectionalLight::RenderShadowMap");
}