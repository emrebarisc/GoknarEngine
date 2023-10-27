#include "pch.h"

#include "DirectionalLight.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/CameraManager.h"
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
	Light::Init();

	if (isShadowEnabled_)
	{
		shadowMapTexture_->SetName(LIGHT::SHADOW_MAP_PREFIX + name_);

		Vector3 lightUpVector = Vector3::Cross(Vector3::Cross(direction_, Vector3::UpVector).GetNormalized(), direction_).GetNormalized();
		shadowMapRenderCamera_ = new Camera(Vector3::ZeroVector, direction_, lightUpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Orthographic);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetImageWidth(shadowWidth_);
		shadowMapRenderCamera_->SetImageHeight(shadowHeight_);
		shadowMapRenderCamera_->SetNearDistance(0.01f);
		shadowMapRenderCamera_->SetFarDistance(1000.f);
	}
}

void DirectionalLight::SetShaderUniforms(const Shader* shader) const
{
	std::string directionName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION;
	shader->SetVector3(directionName.c_str(), direction_);

	std::string radianceName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY;
	shader->SetVector3(radianceName.c_str(), color_ * intensity_);

	std::string isCastingShadowName = name_ + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW;
	shader->SetInt(isCastingShadowName.c_str(), isShadowEnabled_);

	Matrix biasedViewMatrix = shadowMapRenderCamera_->GetViewingMatrix(); 
	biasedViewMatrix *= shadowMapRenderCamera_->GetProjectionMatrix();
	biasedViewMatrix *=
		Matrix
	{
		0.5f, 0.f, 0.f, 0.f,
		0.f, 0.5f, 0.f, 0.f,
		0.f, 0.f, 0.5f - 0.000025f, 0.f,
		0.5f, 0.5f, 0.5f, 1.f
	};

	shader->SetMatrix((SHADER_VARIABLE_NAMES::LIGHT::VIEW_MATRIX_PREFIX + name_).c_str(), biasedViewMatrix);
}

void DirectionalLight::RenderShadowMap()
{
	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();

	Camera* shadowMapRenderCamera = GetShadowMapRenderCamera();
	Vector3 mainCameraForwardVector = mainCamera->GetForwardVector();
	Vector3 mainCameraPosition = mainCamera->GetPosition();
	Vector3 position{ mainCameraPosition.x, mainCameraPosition.y, 0.f };
	shadowMapRenderCamera->SetPosition(position + 10.f * mainCameraForwardVector - 25.f * shadowMapRenderCamera->GetForwardVector());
	//shadowMapRenderCamera->SetPosition(Vector3{ 20.f, 0.f, 0.f } - shadowMapRenderCamera->GetForwardVector() * 25.f);

	cameraManager->SetActiveCamera(shadowMapRenderCamera);
	Texture* shadowMapTexture = GetShadowMapTexture();
	glViewport(0, 0, shadowMapTexture->GetWidth(), shadowMapTexture->GetHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, GetShadowMapFBO());

	Renderer* renderer = engine->GetRenderer();
	renderer->Render(RenderPassType::Depth);

	// For outputing only!
	//shadowMapTexture->ReadFromFramebuffer(directionalLight->GetShadowMapFBO());
	//shadowMapTexture->Save(CONTENT_DIR + directionalLight->GetName() + "FrameBufferTexture.png");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
