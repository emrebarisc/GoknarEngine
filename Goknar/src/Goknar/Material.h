#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "Goknar/Core.h"

#include "Math.h"

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

protected:

private:
	Vector3 ambientReflectance_;
	Vector3 diffuseReflectance_;
	Vector3 specularReflectance_;
	float phongExponent_;
};

#endif