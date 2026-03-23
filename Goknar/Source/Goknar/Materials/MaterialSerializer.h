#pragma once

#include "Goknar/Core.h"
#include <string>

struct MaterialInitializationData;
class Material;

namespace tinyxml2 
{ 
    class XMLDocument; 
    class XMLElement; 
}

class GOKNAR_API MaterialSerializer
{
public:
    static void Serialize(const std::string& filepath, const Material* material);
    static void Deserialize(const std::string& filepath, Material* owner);

private:
    static void SerializeShaderFunction(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const std::string& name, const struct ShaderFunctionAndResult& func);
    static void DeserializeShaderFunction(tinyxml2::XMLElement* parent, const std::string& name, struct ShaderFunctionAndResult& outFunc);
};