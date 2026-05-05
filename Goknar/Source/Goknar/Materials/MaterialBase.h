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

	float GetAmbientOcclusion() const
	{
		return ambientOcclusion_;
	}

	void SetAmbientOcclusion(float ambientOcclusion);

	float GetMetallic() const
	{
		return metallic_;
	}

	void SetMetallic(float metallic);

	float GetRoughness() const
	{
		return roughness_;
	}

	void SetRoughness(float roughness);

	const Vector3& GetEmisiveColor() const
	{
		return emisiveColor_;
	}

	void SetEmisiveColor(const Vector3& emmisiveColor)
	{
		emisiveColor_ = emmisiveColor;
	}

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

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

protected:
	std::vector<const Image*> textureImages_;

	Vector4 baseColor_{ Vector4::ZeroVector };
	Vector3 emisiveColor_{ Vector3::ZeroVector };

	std::string name_{ "" };

	float ambientOcclusion_{ 1.f };
	float metallic_{ 0.f };
	float roughness_{ 0.5f };
	float translucency_{ 0.f };

	MaterialBlendModel blendModel_{ MaterialBlendModel::Opaque };
	MaterialShadingModel shadingModel_{ MaterialShadingModel::Default };

	bool isInitialized_{ false };

private:
};

#endif
