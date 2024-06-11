 #include "pch.h"

#include "MaterialBase.h"

#include "Goknar/Application.h"
#include "Goknar/Scene.h"
#include "Goknar/Engine.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Lights/LightManager/LightManager.h"

#include "Goknar/Managers/WindowManager.h"

IMaterialBase::IMaterialBase() :  
	diffuseReflectance_(Vector4::ZeroVector), 
	ambientReflectance_(Vector3::ZeroVector),
	specularReflectance_(Vector3::ZeroVector),
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
	diffuseReflectance_ = other->diffuseReflectance_;
	specularReflectance_ = other->specularReflectance_;
	blendModel_ = other->blendModel_;
	shadingModel_ = other->shadingModel_;
	phongExponent_ = other->phongExponent_;
	textureImages_ = other->textureImages_;
}

IMaterialBase::~IMaterialBase()
{
}

void IMaterialBase::SetPhongExponent(float phongExponent)
{
	if(engine->GetRenderer()->GetMainRenderType() == RenderPassType::Deferred)
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
	if (shadingModel_ == MaterialShadingModel::Default)
	{
		glEnable(GL_CULL_FACE);
	}
	else if (	shadingModel_ == MaterialShadingModel::TwoSided && 
				(renderPassType == RenderPassType::Forward || renderPassType == RenderPassType::GeometryBuffer))
	{
		glDisable(GL_CULL_FACE);
	}

	Shader* shader = GetShader(renderPassType);

	if (renderPassType == RenderPassType::Forward || renderPassType == RenderPassType::GeometryBuffer)
	{
		shader->SetVector4(SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR, diffuseReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION, ambientReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
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
