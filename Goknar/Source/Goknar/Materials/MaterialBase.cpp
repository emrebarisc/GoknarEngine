#include "pch.h"

#include "MaterialBase.h"

#include <cmath>

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Lights/LightManager/LightManager.h"

#include "Goknar/Managers/WindowManager.h"

namespace
{
	float SanitizeNormalizedMaterialScalar(float value, float defaultValue)
	{
		if (!std::isfinite(value))
		{
			return defaultValue;
		}

		return GoknarMath::Clamp(value, 0.f, 1.f);
	}
}

IMaterialBase::IMaterialBase() :  
	baseColor_(Vector4{ 1.f }),
	emisiveColor_(Vector3::ZeroVector),
	ambientOcclusion_(1.f),
	metallic_(0.f),
	roughness_(0.5f),
	blendModel_(MaterialBlendModel::Opaque),
	shadingModel_(MaterialShadingModel::Default)
{
}

IMaterialBase::IMaterialBase(const IMaterialBase* other)
{
	if(this == other)
	{
		return;
	}

	baseColor_ = other->baseColor_;
	emisiveColor_ = other->emisiveColor_;
	ambientOcclusion_ = other->ambientOcclusion_;
	metallic_ = other->metallic_;
	roughness_ = other->roughness_;
	blendModel_ = other->blendModel_;
	shadingModel_ = other->shadingModel_;
	textureImages_ = other->textureImages_;
}

IMaterialBase::~IMaterialBase()
{
}

void IMaterialBase::SetAmbientOcclusion(float ambientOcclusion)
{
	ambientOcclusion_ = SanitizeNormalizedMaterialScalar(ambientOcclusion, 1.f);
}

void IMaterialBase::SetMetallic(float metallic)
{
	metallic_ = SanitizeNormalizedMaterialScalar(metallic, 0.f);
}

void IMaterialBase::SetRoughness(float roughness)
{
	roughness_ = SanitizeNormalizedMaterialScalar(roughness, 0.5f);
}

void IMaterialBase::PreInit()
{
}

void IMaterialBase::Init()
{
}

void IMaterialBase::PostInit()
{
	isInitialized_ = true;
}

void IMaterialBase::Render(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix) const
{
	Use(renderPassType);
	SetShaderVariables(renderPassType, worldAndRelativeTransformationMatrix);
}

void IMaterialBase::Use(RenderPassType renderPassType) const
{
	Shader* shader = GetShader(renderPassType);
	if (shader)
	{
		shader->Use();
	}
}

void IMaterialBase::SetShaderVariables(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix) const
{
	Shader* shader = GetShader(renderPassType);

	if (!shader)
	{
		return;
	}

	bool cullBackFaces = 
		shadingModel_ == MaterialShadingModel::Default && 
		(	renderPassType == RenderPassType::Forward ||
			renderPassType == RenderPassType::GeometryBuffer);

	if (cullBackFaces)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	if (renderPassType == RenderPassType::Forward || renderPassType == RenderPassType::GeometryBuffer)
	{
		shader->SetVector4(SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR, baseColor_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION, ambientOcclusion_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::METALLIC, metallic_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::ROUGHNESS, roughness_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR, emisiveColor_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY, translucency_);
	}
	else if (renderPassType == RenderPassType::Shadow || renderPassType == RenderPassType::PointLightShadow)
	{
		engine->GetRenderer()->GetLightManager()->SetShadowRenderPassShaderUniforms(shader);
	}

	if (renderPassType == RenderPassType::Forward || renderPassType == RenderPassType::Deferred)
	{
		engine->GetRenderer()->SetLightUniforms(shader);
	}

	shader->SetMVP(worldAndRelativeTransformationMatrix);
	engine->SetShaderEngineVariables(shader);
}
