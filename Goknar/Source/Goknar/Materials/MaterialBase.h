#ifndef __MATERIALBASE_H__
#define __MATERIALBASE_H__

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"

class Image;
class Texture;

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

	const Vector3& GetEmissiveColor() const
	{
		return emissiveColor_;
	}

	void SetEmissiveColor(const Vector3& emissiveColor)
	{
		emissiveColor_ = emissiveColor;
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

	bool GetUsesReflectionProbe() const
	{
		return usesReflectionProbe_;
	}

	void SetUsesReflectionProbe(bool usesReflectionProbe)
	{
		usesReflectionProbe_ = usesReflectionProbe;
	}

	inline void SetName(const std::string& name)
	{
		name_ = name;
	}

	inline const std::string& GetName() const
	{
		return name_;
	}

	void AddTextureImage(const Image* image);
	void AddTextureImage(const Image* image, bool useTextureAtlas);

	const std::vector<const Image*>* GetTextureImages() const
	{
		return &textureImages_;
	}

	bool GetTextureImageUsesTextureAtlas(size_t textureImageIndex) const
	{
		return textureImageIndex < textureImageAtlasUsages_.size() ? textureImageAtlasUsages_[textureImageIndex] : useTextureAtlasForTextureImages_;
	}

	void SetUseTextureAtlasForTextureImages(bool useTextureAtlasForTextureImages)
	{
		useTextureAtlasForTextureImages_ = useTextureAtlasForTextureImages;
	}

	bool GetUseTextureAtlasForTextureImages() const
	{
		return useTextureAtlasForTextureImages_;
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

protected:
	void ClearTextureImages();

	std::vector<const Image*> textureImages_;
	std::vector<bool> textureImageAtlasUsages_;

	Vector4 baseColor_{ Vector4::ZeroVector };
	Vector3 emissiveColor_{ Vector3::ZeroVector };

	std::string name_{ "" };

	float ambientOcclusion_{ 1.f };
	float metallic_{ 0.f };
	float roughness_{ 0.5f };
	float translucency_{ 0.f };

	MaterialBlendModel blendModel_{ MaterialBlendModel::Opaque };
	MaterialShadingModel shadingModel_{ MaterialShadingModel::Default };
	bool usesReflectionProbe_{ false };
	bool useTextureAtlasForTextureImages_{ true };

	bool isInitialized_{ false };

private:
};

#endif
