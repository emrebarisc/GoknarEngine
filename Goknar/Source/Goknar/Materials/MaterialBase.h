#ifndef __MATERIALBASE_H__
#define __MATERIALBASE_H__

#include <string>
#include <unordered_map>

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"

class Image;

enum class MaterialBlendModel
{
	Opaque = 0,
	Masked,
	Transparent
};

enum class MaterialShadingModel
{
	Default,
	TwoSided
};


class GOKNAR_API IMaterialBase
{
public:
	IMaterialBase();
	IMaterialBase(const IMaterialBase* other);
	virtual ~IMaterialBase();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void Render(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix) const;
	virtual void Use(RenderPassType renderPassType) const;
	virtual void SetShaderVariables(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix) const;

	virtual Shader* GetShader(RenderPassType renderPassType) const = 0;

	const Vector3& GetAmbientReflectance() const
	{
		return ambientReflectance_;
	}

	void SetAmbientReflectance(const Vector3& ambientReflectance)
	{
		ambientReflectance_ = ambientReflectance;
	}

	const Vector4& GetBaseColor() const
	{
		return baseColor_;
	}

	void SetBaseColor(const Vector3& diffuseReflectance)
	{
		baseColor_ = Vector4(diffuseReflectance, 1.f);
	}

	void SetBaseColor(const Vector4& diffuseReflectance)
	{
		baseColor_ = diffuseReflectance;
	}

	const Vector3& GetSpecularReflectance() const
	{
		return specularReflectance_;
	}

	void SetSpecularReflectance(const Vector3& specularReflectance)
	{
		specularReflectance_ = specularReflectance;
	}

	const Vector3& GetEmmisiveColor() const
	{
		return emmisiveColor_;
	}

	void SetEmmisiveColor(const Vector3& emmisiveColor)
	{
		emmisiveColor_ = emmisiveColor;
	}

	float GetPhongExponent() const
	{
		return phongExponent_;
	}

	void SetPhongExponent(float phongExponent);

	float GetTranslucency() const
	{
		return translucency_;
	}

	void SetTranslucency(float translucency)
	{
		translucency_ = translucency;
	}

	MaterialBlendModel GetBlendModel() const
	{
		return blendModel_;
	}

	void SetBlendModel(MaterialBlendModel blendModel)
	{
		blendModel_ = blendModel;
	}

	MaterialShadingModel GetShadingModel() const
	{
		return shadingModel_;
	}

	void SetShadingModel(MaterialShadingModel shadingModel)
	{
		shadingModel_ = shadingModel;
	}

	inline void SetName(const std::string& name)
	{
		name_ = name;
	}

	inline const std::string& GetName() const
	{
		return name_;
	}

	void AddTextureImage(const Image* image)
	{
		textureImages_.push_back(image);
	}

	const std::vector<const Image*>* GetTextureImages() const
	{
		return &textureImages_;
	}

protected:
	std::vector<const Image*> textureImages_;

	Vector4 baseColor_{ Vector4::ZeroVector };
	Vector3 ambientReflectance_{ Vector3::ZeroVector };
	Vector3 specularReflectance_{ Vector3::ZeroVector };
	Vector3 emmisiveColor_{ Vector3::ZeroVector };

	std::string name_{ "" };

	float translucency_{ 0.f };
	float phongExponent_{ 1.f };

	MaterialBlendModel blendModel_{ MaterialBlendModel::Opaque };
	MaterialShadingModel shadingModel_{ MaterialShadingModel::Default };

private:
};

#endif