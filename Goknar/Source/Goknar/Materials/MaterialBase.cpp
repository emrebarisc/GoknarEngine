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
	constexpr float MIN_PHONG_EXPONENT = 1.f;

	RenderPassType GetMainRenderTypeSafe()
	{
		if (!engine || !engine->GetRenderer())
		{
			return RenderPassType::Forward;
		}

		return engine->GetRenderer()->GetMainRenderType();
	}

	float SanitizePhongExponent(float phongExponent)
	{
		if (!std::isfinite(phongExponent) || phongExponent < MIN_PHONG_EXPONENT)
		{
			return MIN_PHONG_EXPONENT;
		}

		return phongExponent;
	}
}

IMaterialBase::IMaterialBase() :  
	baseColor_(Vector4{ 1.f }),
	ambientReflectance_(Vector3::ZeroVector),
	specularReflectance_(Vector3::ZeroVector),
	emmisiveColor_(Vector3::ZeroVector),
	blendModel_(MaterialBlendModel::Opaque),
	shadingModel_(MaterialShadingModel::Default),
	phongExponent_(1.f)
{
}

IMaterialBase::IMaterialBase(const IMaterialBase* other)
{
	if(this == other)
	{
		return;
	}

	ambientReflectance_ = other->ambientReflectance_;
	baseColor_ = other->baseColor_;
	specularReflectance_ = other->specularReflectance_;
	emmisiveColor_ = other->emmisiveColor_;
	blendModel_ = other->blendModel_;
	shadingModel_ = other->shadingModel_;
	phongExponent_ = other->phongExponent_;
	textureImages_ = other->textureImages_;
}

IMaterialBase::~IMaterialBase()
{
}

float IMaterialBase::GetPhongExponent() const
{
	float phongExponent = phongExponent_;

	if (GetMainRenderTypeSafe() == RenderPassType::Deferred)
	{
		phongExponent = std::pow(2.f, phongExponent);
	}

	return SanitizePhongExponent(phongExponent);
}

void IMaterialBase::SetPhongExponent(float phongExponent)
{
	phongExponent = SanitizePhongExponent(phongExponent);

	if(GetMainRenderTypeSafe() == RenderPassType::Deferred)
	{
		phongExponent = GoknarMath::Log(phongExponent) / LN_OF_2;
	}

	phongExponent_ = phongExponent;
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
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION, ambientReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::EMMISIVE_COLOR, emmisiveColor_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, phongExponent_);
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
