#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <string>

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"
#include "Renderer/Shader.h"

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

class GOKNAR_API Material
{
public:
	Material();
	~Material();

	void Init();

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

	void SetPhongExponent(float phongExponent)
	{
		phongExponent_ = phongExponent;
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

	inline Shader* GetShader() const
	{
		return shader_;
	}

	inline void SetShader(Shader* shader)
	{
		shader_ = shader;
	}

	inline void SetName(const std::string& name)
	{
		name_ = name;
	}

	inline const std::string& GetName() const
	{
		return name_;
	}

	void Render(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix) const;
	void Use() const;
	void SetShaderVariables(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix) const;

protected:

private:
	Vector3 ambientReflectance_;
	Vector3 diffuseReflectance_;
	Vector3 specularReflectance_;

	std::string name_;

	Shader* shader_;

	float phongExponent_;

	MaterialBlendModel blendModel_;
	MaterialShadingModel shadingModel_;
};

#endif