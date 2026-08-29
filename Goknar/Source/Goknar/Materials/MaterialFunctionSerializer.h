#pragma once

#include "Goknar/Core.h"
#include "MaterialFunction.h"

#include <string>

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

class GOKNAR_API MaterialFunctionSerializer
{
public:
	static bool Serialize(const std::string& filepath, const MaterialFunction& materialFunction);
	static bool Deserialize(const std::string& filepath, MaterialFunction& outMaterialFunction);

	static const char* PinTypeToString(MaterialFunctionPinType type);
	static MaterialFunctionPinType StringToPinType(const std::string& type);

private:
	static void SerializePins(
		tinyxml2::XMLDocument& document,
		tinyxml2::XMLElement* parentElement,
		const char* containerName,
		const std::vector<MaterialFunctionPinDefinition>& pins);

	static void DeserializePins(
		tinyxml2::XMLElement* parentElement,
		const char* containerName,
		std::vector<MaterialFunctionPinDefinition>& outPins);
};
