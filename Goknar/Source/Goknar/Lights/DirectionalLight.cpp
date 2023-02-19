#include "pch.h"

#include "DirectionalLight.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
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
		shadowMapTexture_->SetName(name_ + LIGHT::SHADOW_MAP_POSTFIX);

		Vector3 lightUpVector = Vector3::Cross(Vector3::Cross(direction_, Vector3::UpVector).GetNormalized(), direction_).GetNormalized();
		shadowMapRenderCamera_ = new Camera(direction_ * -100.f, direction_, lightUpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Orthographic);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetFarDistance(100);
		shadowMapRenderCamera_->Init();
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
	shader->SetInt(shadowMapTexture_->GetName().c_str(), shadowMapTexture_->GetRendererTextureId());

	Matrix biasedViewMatrix = shadowMapRenderCamera_->GetViewingMatrix() * 
		Matrix(	0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0);

	shader->SetMatrix((name_ + LIGHT::VIEW_MATRIX_POSTFIX).c_str(), biasedViewMatrix);
}

void DirectionalLight::RenderShadowMap()
{
}
