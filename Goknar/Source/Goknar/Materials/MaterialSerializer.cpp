#include "MaterialSerializer.h"
#include "Goknar/Materials/Material.h"
#include "tinyxml2.h"

using namespace tinyxml2;

void MaterialSerializer::Serialize(const std::string& filepath, const MaterialInitializationData* data)
{
    XMLDocument doc;
    
    // Create the root element with the generic name "FileType"
    XMLElement* root = doc.NewElement("FileType");
    // Set the attribute as requested
    root->SetAttribute("FileType", "MaterialData");
    root->SetAttribute("BoneCount", data->boneCount);
    doc.InsertFirstChild(root);

    // Serialize ShaderFunctionAndResults
    SerializeShaderFunction(doc, root, "BaseColor", data->baseColor);
    SerializeShaderFunction(doc, root, "EmissiveColor", data->emmisiveColor);
    SerializeShaderFunction(doc, root, "FragmentNormal", data->fragmentNormal);
    SerializeShaderFunction(doc, root, "VertexNormal", data->vertexNormal);
    SerializeShaderFunction(doc, root, "UV", data->uv);
    SerializeShaderFunction(doc, root, "VertexPositionOffset", data->vertexPositionOffset);

    // Helper for long text strings
    auto AddTextElement = [&](const char* name, const std::string& content) {
        XMLElement* el = doc.NewElement(name);
        // Use CData for shaders to prevent XML parsing errors with symbols like < or &
        XMLText* text = doc.NewText(content.c_str());
        text->SetCData(true);
        el->InsertEndChild(text);
        root->InsertEndChild(el);
    };

    AddTextElement("VertexShaderFunctions", data->vertexShaderFunctions);
    AddTextElement("FragmentShaderFunctions", data->fragmentShaderFunctions);
    AddTextElement("VertexShaderUniforms", data->vertexShaderUniforms);
    AddTextElement("FragmentShaderUniforms", data->fragmentShaderUniforms);

    doc.SaveFile(filepath.c_str());
}

void MaterialSerializer::Deserialize(const std::string& filepath, Material* owner, MaterialInitializationData* outData)
{
    XMLDocument doc;
    if (doc.LoadFile(filepath.c_str()) != XML_SUCCESS) return;

    // Find the generic "FileType" element
    XMLElement* root = doc.FirstChildElement("FileType");
    if (!root) return;

    // Verify this is actually MaterialData
    const char* fileTypeAttr = root->Attribute("FileType");
    if (!fileTypeAttr || std::string(fileTypeAttr) != "MaterialData") return;

    outData->boneCount = root->IntAttribute("BoneCount");

    // Deserialize ShaderFunctionAndResults
    DeserializeShaderFunction(root, "BaseColor", outData->baseColor);
    DeserializeShaderFunction(root, "EmissiveColor", outData->emmisiveColor);
    DeserializeShaderFunction(root, "FragmentNormal", outData->fragmentNormal);
    DeserializeShaderFunction(root, "VertexNormal", outData->vertexNormal);
    DeserializeShaderFunction(root, "UV", outData->uv);
    DeserializeShaderFunction(root, "VertexPositionOffset", outData->vertexPositionOffset);

    auto GetTextContent = [&](const char* name) -> std::string {
        XMLElement* el = root->FirstChildElement(name);
        return el && el->GetText() ? el->GetText() : "";
    };

    outData->vertexShaderFunctions = GetTextContent("VertexShaderFunctions");
    outData->fragmentShaderFunctions = GetTextContent("FragmentShaderFunctions");
    outData->vertexShaderUniforms = GetTextContent("VertexShaderUniforms");
    outData->fragmentShaderUniforms = GetTextContent("FragmentShaderUniforms");
}