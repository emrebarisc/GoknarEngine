#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <vector>

#include "MaterialBase.h"

class MeshUnit;
class MaterialInstance;

struct GOKNAR_API MaterialInitializationData
{
	std::string baseColor{ "" };
	std::string normal{ "" };
	std::string uv{ "" };
	std::string positionOffset{ "" };
	int boneCount{ 0 };
};

class GOKNAR_API Material : public IMaterialBase
{
public:
	Material();
	Material(const Material* parent);
	virtual ~Material();

	void Build(MeshUnit* meshUnit);

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	inline virtual Shader* GetShader(RenderPassType renderPassType) const override
	{
		if (renderPassTypeShaderMap_.find(renderPassType) == renderPassTypeShaderMap_.end())
		{
			return nullptr;
		}

		return renderPassTypeShaderMap_.at(renderPassType);
	}

	inline void SetShader(Shader* shader, RenderPassType renderPassType = RenderPassType::Forward)
	{
		renderPassTypeShaderMap_[renderPassType] = shader;
	}

	void AddDerivedMaterialInstance(MaterialInstance* materialInstance)
	{
		derivedMaterialInstances_.push_back(materialInstance);
	}

	void RemoveDerivedMaterialInstance(MaterialInstance* materialInstance)
	{
		decltype(derivedMaterialInstances_.cbegin()) derivedMaterialInstancesIterator = derivedMaterialInstances_.cbegin();
		while (derivedMaterialInstancesIterator != derivedMaterialInstances_.end())
		{
			if (*derivedMaterialInstancesIterator == materialInstance)
			{
				derivedMaterialInstances_.erase(derivedMaterialInstancesIterator);
				break;
			}
			std::advance(derivedMaterialInstancesIterator, 1);
		}
	}

	MaterialInitializationData* GetInitializationData() const
	{
		return initializationData_;
	}

protected:


private:
	std::vector<MaterialInstance*> derivedMaterialInstances_;

	std::unordered_map<RenderPassType, Shader*> renderPassTypeShaderMap_;

	MaterialInitializationData* initializationData_{ new MaterialInitializationData() };
};

#endif