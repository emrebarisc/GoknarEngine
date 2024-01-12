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
	Translucent
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
	~IMaterialBase();

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

	const Vector3& GetDiffuseReflectance() const
	{
		return diffuseReflectance_;
	}

	void SetDiffuseReflectance(const Vector3& diffuseReflectance)
	{
		diffuseReflectance_ = diffuseReflectance;
	}

	const Vector3& GetSpecularReflectance() const
	{
		return specularReflectance_;
	}

	void SetSpecularReflectance(const Vector3& specularReflectance)
	{
		specularReflectance_ = specularReflectance;
	}

	float GetPhongExponent() const
	{
		return phongExponent_;
	}

	void SetPhongExponent(float phongExponent);

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

	Vector3 ambientReflectance_{ Vector3::ZeroVector };
	Vector3 diffuseReflectance_{ Vector3::ZeroVector };
	Vector3 specularReflectance_{ Vector3::ZeroVector };

	std::string name_{ "" };

	float phongExponent_{ 1.f };

	MaterialBlendModel blendModel_{ MaterialBlendModel::Opaque };
	MaterialShadingModel shadingModel_{ MaterialShadingModel::Default };

private:
};

#endif