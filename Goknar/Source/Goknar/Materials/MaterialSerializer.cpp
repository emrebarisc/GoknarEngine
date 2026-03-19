#include "MaterialSerializer.h"
#include "Goknar/Materials/Material.h"
#include "tinyxml2.h"

using namespace tinyxml2;

void MaterialSerializer::Serialize(const std::string& filepath, const MaterialInitializationData* data)
{
    std::string contentDir = ContentDir + filepath;

    tinyxml2::XMLDocument doc;
    
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

    doc.SaveFile(contentDir.c_str());
}

void MaterialSerializer::Deserialize(const std::string& filepath, Material* owner, MaterialInitializationData* outData)
{
    std::string contentDir = ContentDir + filepath;

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(contentDir.c_str()) != XML_SUCCESS)
    {
        return;
    }

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

void MaterialSerializer::SerializeShaderFunction(tinyxml2::XMLDocument& doc, XMLElement* parent, const std::string& name, const ShaderFunctionAndResult& func)
{
    XMLElement* funcElement = doc.NewElement(name.c_str());

    XMLElement* calc = doc.NewElement("Calculation");
    calc->SetText(func.calculation.c_str());
    funcElement->InsertEndChild(calc);

    XMLElement* res = doc.NewElement("Result");
    res->SetText(func.result.c_str());
    funcElement->InsertEndChild(res);

    parent->InsertEndChild(funcElement);
}

void MaterialSerializer::DeserializeShaderFunction(tinyxml2::XMLElement* parent, const std::string& name, ShaderFunctionAndResult& outFunc)
{
    XMLElement* funcElement = parent->FirstChildElement(name.c_str());
    if (funcElement)
    {
        XMLElement* calc = funcElement->FirstChildElement("Calculation");
        if (calc && calc->GetText()) outFunc.calculation = calc->GetText();

        XMLElement* res = funcElement->FirstChildElement("Result");
        if (res && res->GetText()) outFunc.result = res->GetText();
    }
}