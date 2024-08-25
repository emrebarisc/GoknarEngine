#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <vector>

#include "MaterialBase.h"

class MeshUnit;
class Material;
class MaterialInstance;

struct GOKNAR_API ShaderFunctionAndResult
{
	std::string calculation{ "" };
	std::string result{ "" };
};

struct GOKNAR_API MaterialInitializationData
{
	MaterialInitializationData() = delete;
	MaterialInitializationData(const Material* ownerMaterial) : owner(ownerMaterial) {}

	const Material* owner;
	ShaderFunctionAndResult baseColor;
	ShaderFunctionAndResult emmisiveColor;
	ShaderFunctionAndResult fragmentNormal;
	ShaderFunctionAndResult vertexNormal;
	ShaderFunctionAndResult uv;
	ShaderFunctionAndResult vertexPositionOffset;
	std::string vertexShaderFunctions{ "" };
	std::string fragmentShaderFunctions{ "" };
	std::string vertexShaderUniforms{ "" };
	std::string fragmentShaderUniforms{ "" };

	void AddVertexShaderFunction(const std::string& function)
	{
		vertexShaderFunctions += "\n";
		vertexShaderFunctions += function;
		vertexShaderFunctions += "\n";
	}

	void AddFragmentShaderFunction(const std::string& function)
	{
		fragmentShaderFunctions += "\n";
		fragmentShaderFunctions += function;
		fragmentShaderFunctions += "\n";
	}

	void AddVertexShaderUniform(const std::string& uniform)
	{
		vertexShaderUniforms += uniform;
		vertexShaderUniforms += ";\n";
	}

	void AddFragmentShaderUniform(const std::string& uniform)
	{
		fragmentShaderUniforms += uniform;
		fragmentShaderUniforms += ";\n";
	}

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

	MaterialInitializationData* initializationData_{ new MaterialInitializationData( this ) };
};

#endif