#include "pch.h"

#include "MaterialBase.h"

#include <cmath>

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Lights/LightManager/LightManager.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Managers/ResourceManager.h"

#include "Goknar/Managers/WindowManager.h"

namespace
{
<<<<<<< HEAD
	float SanitizeNormalizedMaterialScalar(float value, float defaultValue)
	{
		if (!std::isfinite(value))
		{
			return defaultValue;
		}

		return GoknarMath::Clamp(value, 0.f, 1.f);
	}

	TextureAtlasCategory GetTextureAtlasCategory(MaterialBlendModel blendModel)
	{
		return blendModel == MaterialBlendModel::Transparent ?
			TextureAtlasCategory::Transparent :
			TextureAtlasCategory::Opaque;
=======
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
>>>>>>> master
	}
}

IMaterialBase::IMaterialBase() :  
	baseColor_(Vector4{ 1.f }),
	emissiveColor_(Vector3::ZeroVector),
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
	emissiveColor_ = other->emissiveColor_;
	ambientOcclusion_ = other->ambientOcclusion_;
	metallic_ = other->metallic_;
	roughness_ = other->roughness_;
	blendModel_ = other->blendModel_;
	shadingModel_ = other->shadingModel_;
	usesReflectionProbe_ = other->usesReflectionProbe_;
	textureImages_ = other->textureImages_;
	textureImageAtlasUsages_ = other->textureImageAtlasUsages_;
	useTextureAtlasForTextureImages_ = other->useTextureAtlasForTextureImages_;
}

IMaterialBase::~IMaterialBase()
{
}

<<<<<<< HEAD
void IMaterialBase::AddTextureImage(const Image* image)
{
	AddTextureImage(image, useTextureAtlasForTextureImages_);
}

void IMaterialBase::AddTextureImage(const Image* image, bool useTextureAtlas)
{
	if (!image)
=======
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
>>>>>>> master
	{
		return;
	}

	textureImages_.push_back(image);
	textureImageAtlasUsages_.push_back(useTextureAtlas);

	if (useTextureAtlas)
	{
		Image* mutableImage = const_cast<Image*>(image);
		mutableImage->SetCanUseTextureAtlas(true);

		ResourceManager* resourceManager = engine ? engine->GetResourceManager() : nullptr;
		if (resourceManager && resourceManager->GetResourceContainer())
		{
			resourceManager->GetResourceContainer()->RegisterImageToTextureAtlas(mutableImage, GetTextureAtlasCategory(blendModel_));
		}
	}
}

void IMaterialBase::ClearTextureImages()
{
	textureImages_.clear();
	textureImageAtlasUsages_.clear();
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

<<<<<<< HEAD
	if (!shader)
	{
		return;
	}

=======
>>>>>>> master
	bool cullBackFaces = 
		shadingModel_ == MaterialShadingModel::Default && 
		(	renderPassType == RenderPassType::Forward ||
			renderPassType == RenderPassType::GeometryBuffer ||
			renderPassType == RenderPassType::CubemapCapture);

	if (cullBackFaces)
	{
		engine->GetGraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, true);
	}
	else
	{
		engine->GetGraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, false);
	}

<<<<<<< HEAD
	if (renderPassType == RenderPassType::Forward ||
		renderPassType == RenderPassType::GeometryBuffer ||
		renderPassType == RenderPassType::CubemapCapture)
=======
	if (renderPassType == RenderPassType::Forward || renderPassType == RenderPassType::GeometryBuffer)
>>>>>>> master
	{
		shader->SetVector4(SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR, baseColor_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION, ambientOcclusion_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::METALLIC, metallic_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::ROUGHNESS, roughness_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR, emissiveColor_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY, translucency_);
		shader->SetInt(SHADER_VARIABLE_NAMES::MATERIAL::SHADING_TYPE_ID, (int)shadingType_);
	}
	else if (renderPassType == RenderPassType::Shadow || renderPassType == RenderPassType::PointLightShadow)
	{
		engine->GetRenderer()->GetLightManager()->SetShadowRenderPassShaderUniforms(shader);
	}
	if (renderPassType == RenderPassType::CubemapCapture)
	{
		engine->GetRenderer()->SetCubemapRenderPassShaderUniforms(shader);
	}

	if (renderPassType == RenderPassType::Forward)
	{
		engine->GetRenderer()->SetLightUniforms(shader);
		if (usesReflectionProbe_)
		{
			engine->GetRenderer()->SetReflectionProbeUniforms(shader);
		}
	}
	else if (renderPassType == RenderPassType::CubemapCapture)
	{
		engine->GetRenderer()->SetLightUniforms(shader);
		shader->SetBool(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::HAS_REFLECTION_PROBE, false);
		shader->SetInt(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP, 0);
	}

	shader->SetMVP(worldAndRelativeTransformationMatrix);
	engine->SetShaderEngineVariables(shader);
}
