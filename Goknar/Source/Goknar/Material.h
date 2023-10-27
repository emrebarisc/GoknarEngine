#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <string>
#include <unordered_map>

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"

class MeshUnit;
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

class GOKNAR_API Material
{
public:
	Material();
	~Material();

	void Build(MeshUnit* meshUnit);

	void PreInit();
	void Init();
	void PostInit();

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

	inline Shader* GetShader(RenderPassType renderPassType) const
	{
		if (renderPassTypeShaderMap_.find(renderPassType) == renderPassTypeShaderMap_.end())
		{
			return nullptr;
		}

		return renderPassTypeShaderMap_.at(renderPassType);
	}

	inline void SetShader(Shader* shader, RenderPassType renderPassType = RenderPassType::Main)
	{
		renderPassTypeShaderMap_[renderPassType] = shader;
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

	void Render(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix, RenderPassType renderPassType = RenderPassType::Main) const;
	void Use(RenderPassType renderPassType = RenderPassType::Main) const;
	void SetShaderVariables(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix, RenderPassType renderPassType = RenderPassType::Main) const;

protected:

private:
	std::vector<const Image*> textureImages_;

	Vector3 ambientReflectance_;
	Vector3 diffuseReflectance_;
	Vector3 specularReflectance_;

	std::string name_;

	std::unordered_map<RenderPassType, Shader*> renderPassTypeShaderMap_;

	float phongExponent_;

	MaterialBlendModel blendModel_;
	MaterialShadingModel shadingModel_;
};

#endif