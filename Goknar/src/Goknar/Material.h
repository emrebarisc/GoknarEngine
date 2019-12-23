#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "Goknar/Core.h"

#include "Math.h"

class Shader;
class Texture;

enum class MaterialShadingModel
{
	Opaque = 0,
	Masked,
	Translucent
};

class GOKNAR_API Material
{
public:
	Material();
	~Material();

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

	void AddTexture(const Texture* texture)
	{
		textures_.push_back(texture);
	}

	const std::vector<const Texture*>& GetTextures() const
	{
		return textures_;
	}

	MaterialShadingModel GetShadingModel() const
	{
		return shadingModel_;
	}

	void SetShadingModel(MaterialShadingModel shadingModel)
	{
		shadingModel_ = shadingModel;
	}

	void Render(const Shader* shader) const;

protected:

private:
	Vector3 ambientReflectance_;
	Vector3 diffuseReflectance_;
	Vector3 specularReflectance_;

	MaterialShadingModel shadingModel_;

	std::vector<const Texture*> textures_;

	float phongExponent_;
};

#endif