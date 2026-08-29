#include "MaterialFunctionSerializer.h"

#include "tinyxml2.h"

#include "Goknar/Core.h"
#include "Goknar/Helpers/ContentPathUtils.h"

namespace
{
	constexpr const char* kMaterialFunctionFileType = "MaterialFunction";
}

bool MaterialFunctionSerializer::Serialize(const std::string& filepath, const MaterialFunction& materialFunction)
{
	const std::string relativeFilePath = ContentPathUtils::ToContentRelativePath(filepath);
	const std::string absolutePath = ContentPathUtils::ToAbsoluteContentPath(relativeFilePath);
	tinyxml2::XMLDocument document;

	tinyxml2::XMLElement* root = document.NewElement("GameAsset");
	root->SetAttribute("FileType", kMaterialFunctionFileType);
	document.InsertFirstChild(root);

	auto AddTextElement = [&](const char* name, const std::string& content, bool useCData = false)
	{
		tinyxml2::XMLElement* element = document.NewElement(name);
		if (useCData)
		{
			tinyxml2::XMLText* text = document.NewText(content.c_str());
			text->SetCData(true);
			element->InsertEndChild(text);
		}
		else
		{
			element->SetText(content.c_str());
		}
		root->InsertEndChild(element);
	};

	AddTextElement("Name", materialFunction.GetName());
	AddTextElement("GeneratedFunctionName", materialFunction.GetGeneratedFunctionName());
	AddTextElement("GeneratedFunctionDefinitions", materialFunction.GetGeneratedFunctionDefinitions(), true);

	SerializePins(document, root, "Inputs", materialFunction.GetInputs());
	SerializePins(document, root, "Outputs", materialFunction.GetOutputs());

	return document.SaveFile(absolutePath.c_str()) == tinyxml2::XML_SUCCESS;
}

bool MaterialFunctionSerializer::Deserialize(const std::string& filepath, MaterialFunction& outMaterialFunction)
{
	tinyxml2::XMLDocument document;
	const std::string relativeFilePath = ContentPathUtils::ToContentRelativePath(filepath);
	const std::string absolutePath = ContentPathUtils::ToAbsoluteContentPath(relativeFilePath);
	if (document.LoadFile(absolutePath.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return false;
	}

	tinyxml2::XMLElement* root = document.FirstChildElement("GameAsset");
	const char* fileType = root ? root->Attribute("FileType") : nullptr;
	if (!root || !fileType || std::string(fileType) != kMaterialFunctionFileType)
	{
		return false;
	}

	outMaterialFunction.SetAssetPath(relativeFilePath);

	if (tinyxml2::XMLElement* nameElement = root->FirstChildElement("Name"); nameElement && nameElement->GetText())
	{
		outMaterialFunction.SetName(nameElement->GetText());
	}
	else
	{
		outMaterialFunction.SetName("");
	}

	if (tinyxml2::XMLElement* generatedFunctionNameElement = root->FirstChildElement("GeneratedFunctionName");
		generatedFunctionNameElement && generatedFunctionNameElement->GetText())
	{
		outMaterialFunction.SetGeneratedFunctionName(generatedFunctionNameElement->GetText());
	}
	else
	{
		outMaterialFunction.SetGeneratedFunctionName("");
	}

	if (tinyxml2::XMLElement* generatedFunctionDefinitionsElement = root->FirstChildElement("GeneratedFunctionDefinitions");
		generatedFunctionDefinitionsElement && generatedFunctionDefinitionsElement->GetText())
	{
		outMaterialFunction.SetGeneratedFunctionDefinitions(generatedFunctionDefinitionsElement->GetText());
	}
	else
	{
		outMaterialFunction.SetGeneratedFunctionDefinitions("");
	}

	std::vector<MaterialFunctionPinDefinition> inputs;
	DeserializePins(root, "Inputs", inputs);
	outMaterialFunction.SetInputs(inputs);

	std::vector<MaterialFunctionPinDefinition> outputDefinitions;
	DeserializePins(root, "Outputs", outputDefinitions);
	if (outputDefinitions.empty())
	{
		MaterialFunctionPinDefinition outputDefinition{ "Result", MaterialFunctionPinType::Float };
		if (tinyxml2::XMLElement* outputElement = root->FirstChildElement("Output"))
		{
			if (const char* outputName = outputElement->Attribute("Name"))
			{
				outputDefinition.name = outputName;
			}

			if (const char* outputType = outputElement->Attribute("Type"))
			{
				outputDefinition.type = StringToPinType(outputType);
			}
		}
		outputDefinitions.push_back(outputDefinition);
	}
	outMaterialFunction.SetOutputs(outputDefinitions);

	return true;
}

const char* MaterialFunctionSerializer::PinTypeToString(MaterialFunctionPinType type)
{
	switch (type)
	{
	case MaterialFunctionPinType::None: return "None";
	case MaterialFunctionPinType::Float: return "Float";
	case MaterialFunctionPinType::Vector2: return "Vector2";
	case MaterialFunctionPinType::Vector3: return "Vector3";
	case MaterialFunctionPinType::Vector4: return "Vector4";
	case MaterialFunctionPinType::Vector4i: return "Vector4i";
	case MaterialFunctionPinType::Matrix4x4: return "Matrix4x4";
	case MaterialFunctionPinType::Texture: return "Texture";
	case MaterialFunctionPinType::Any: return "Any";
	default: return "None";
	}
}

MaterialFunctionPinType MaterialFunctionSerializer::StringToPinType(const std::string& type)
{
	if (type == "Float") return MaterialFunctionPinType::Float;
	if (type == "Vector2") return MaterialFunctionPinType::Vector2;
	if (type == "Vector3") return MaterialFunctionPinType::Vector3;
	if (type == "Vector4") return MaterialFunctionPinType::Vector4;
	if (type == "Vector4i") return MaterialFunctionPinType::Vector4i;
	if (type == "Matrix4x4") return MaterialFunctionPinType::Matrix4x4;
	if (type == "Texture") return MaterialFunctionPinType::Texture;
	if (type == "Any") return MaterialFunctionPinType::Any;
	return MaterialFunctionPinType::None;
}

void MaterialFunctionSerializer::SerializePins(
	tinyxml2::XMLDocument& document,
	tinyxml2::XMLElement* parentElement,
	const char* containerName,
	const std::vector<MaterialFunctionPinDefinition>& pins)
{
	tinyxml2::XMLElement* containerElement = document.NewElement(containerName);
	for (const MaterialFunctionPinDefinition& pin : pins)
	{
		tinyxml2::XMLElement* pinElement = document.NewElement("Pin");
		pinElement->SetAttribute("Name", pin.name.c_str());
		pinElement->SetAttribute("Type", PinTypeToString(pin.type));
		containerElement->InsertEndChild(pinElement);
	}

	parentElement->InsertEndChild(containerElement);
}

void MaterialFunctionSerializer::DeserializePins(
	tinyxml2::XMLElement* parentElement,
	const char* containerName,
	std::vector<MaterialFunctionPinDefinition>& outPins)
{
	outPins.clear();

	tinyxml2::XMLElement* containerElement = parentElement ? parentElement->FirstChildElement(containerName) : nullptr;
	for (tinyxml2::XMLElement* pinElement = containerElement ? containerElement->FirstChildElement("Pin") : nullptr;
		pinElement != nullptr;
		pinElement = pinElement->NextSiblingElement("Pin"))
	{
		MaterialFunctionPinDefinition pinDefinition;
		if (const char* pinName = pinElement->Attribute("Name"))
		{
			pinDefinition.name = pinName;
		}
		if (const char* pinType = pinElement->Attribute("Type"))
		{
			pinDefinition.type = StringToPinType(pinType);
		}
		outPins.push_back(pinDefinition);
	}
}
