#pragma once

#include "Goknar/Core.h"

#include <string>
#include <vector>

enum class GOKNAR_API MaterialFunctionPinType
{
	None = 0,
	Float,
	Vector2,
	Vector3,
	Vector4,
	Vector4i,
	Matrix4x4,
	Texture,
	Any
};

struct GOKNAR_API MaterialFunctionPinDefinition
{
	std::string name{ "" };
	MaterialFunctionPinType type{ MaterialFunctionPinType::None };
};

class GOKNAR_API MaterialFunction
{
public:
	void SetName(const std::string& name)
	{
		name_ = name;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetAssetPath(const std::string& assetPath)
	{
		assetPath_ = assetPath;
	}

	const std::string& GetAssetPath() const
	{
		return assetPath_;
	}

	std::vector<MaterialFunctionPinDefinition>& GetInputs()
	{
		return inputs_;
	}

	const std::vector<MaterialFunctionPinDefinition>& GetInputs() const
	{
		return inputs_;
	}

	void SetInputs(const std::vector<MaterialFunctionPinDefinition>& inputs)
	{
		inputs_ = inputs;
	}

	std::vector<MaterialFunctionPinDefinition>& GetOutputs()
	{
		return outputs_;
	}

	const std::vector<MaterialFunctionPinDefinition>& GetOutputs() const
	{
		return outputs_;
	}

	void SetOutputs(const std::vector<MaterialFunctionPinDefinition>& outputs)
	{
		outputs_ = outputs;
	}

	void SetGeneratedFunctionName(const std::string& generatedFunctionName)
	{
		generatedFunctionName_ = generatedFunctionName;
	}

	const std::string& GetGeneratedFunctionName() const
	{
		return generatedFunctionName_;
	}

	void SetGeneratedFunctionDefinitions(const std::string& generatedFunctionDefinitions)
	{
		generatedFunctionDefinitions_ = generatedFunctionDefinitions;
	}

	const std::string& GetGeneratedFunctionDefinitions() const
	{
		return generatedFunctionDefinitions_;
	}

private:
	std::string name_{ "" };
	std::string assetPath_{ "" };
	std::vector<MaterialFunctionPinDefinition> inputs_{};
	std::vector<MaterialFunctionPinDefinition> outputs_{ { "Result", MaterialFunctionPinType::Float } };
	std::string generatedFunctionName_{ "" };
	std::string generatedFunctionDefinitions_{ "" };
};
