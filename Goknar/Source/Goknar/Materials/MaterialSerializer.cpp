#include "MaterialSerializer.h"

#include <sstream>

#include "tinyxml2.h"

#include "Goknar/Engine.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"

using namespace tinyxml2;

void MaterialSerializer::Serialize(const std::string& filepath, const Material* material)
{
    const MaterialInitializationData* materialInitializationData = material->GetInitializationData();
    std::string contentDir = ContentDir + filepath;

    tinyxml2::XMLDocument doc;

    XMLElement* root = doc.NewElement("GameAsset");

    root->SetAttribute("FileType", "Material");
    doc.InsertFirstChild(root);
    
    auto AddPropertyElement = [&](const char* name, const std::string& content)
        {
            XMLElement* el = doc.NewElement(name);
            el->SetText(content.c_str());
            root->InsertEndChild(el);
        };

    AddPropertyElement("BoneCount", std::to_string(materialInitializationData->boneCount));

    MaterialBlendModel blendModel = material->GetBlendModel();
    AddPropertyElement("BlendModel", blendModel == MaterialBlendModel::Masked ? "Masked" :
        blendModel == MaterialBlendModel::Transparent ? "Transparent" : "Opaque");

    MaterialShadingModel shadingModel = material->GetShadingModel();
    AddPropertyElement("ShadingModel", shadingModel == MaterialShadingModel::TwoSided ? "TwoSided" : "Default");

    const std::vector<const Image*>* textureImages = material->GetTextureImages();
    if (textureImages)
    {
        for (const Image* image : *textureImages)
        {
            if (image)
            {
                XMLElement* texElement = doc.NewElement("Texture");
                texElement->SetAttribute("path", image->GetPath().c_str());
                root->InsertEndChild(texElement);
            }
        }
    }

    auto SerializeVector3 = [&](const char* name, const Vector3& vec)
        {
            std::stringstream ss;
            ss << vec.x << " " << vec.y << " " << vec.z;
            AddPropertyElement(name, ss.str());
        };

    SerializeVector3("AmbientReflectance", material->GetAmbientReflectance());

    Vector4 baseColor = material->GetBaseColor();
    SerializeVector3("DiffuseReflectance", Vector3(baseColor.x, baseColor.y, baseColor.z));

    SerializeVector3("SpecularReflectance", material->GetSpecularReflectance());

    AddPropertyElement("PhongExponent", std::to_string(material->GetPhongExponent()));

    SerializeShaderFunction(doc, root, "BaseColor", materialInitializationData->baseColor);
    SerializeShaderFunction(doc, root, "EmissiveColor", materialInitializationData->emmisiveColor);
    SerializeShaderFunction(doc, root, "FragmentNormal", materialInitializationData->fragmentNormal);
    SerializeShaderFunction(doc, root, "VertexNormal", materialInitializationData->vertexNormal);
    SerializeShaderFunction(doc, root, "UV", materialInitializationData->uv);
    SerializeShaderFunction(doc, root, "VertexPositionOffset", materialInitializationData->vertexPositionOffset);

    auto AddTextElement =
        [&](const char* name, const std::string& content)
        {
            XMLElement* el = doc.NewElement(name);
            XMLText* text = doc.NewText(content.c_str());
            text->SetCData(true);
            el->InsertEndChild(text);
            root->InsertEndChild(el);
        };

    AddTextElement("VertexShaderFunctions", materialInitializationData->vertexShaderFunctions);
    AddTextElement("FragmentShaderFunctions", materialInitializationData->fragmentShaderFunctions);
    AddTextElement("VertexShaderUniforms", materialInitializationData->vertexShaderUniforms);
    AddTextElement("FragmentShaderUniforms", materialInitializationData->fragmentShaderUniforms);

    doc.SaveFile(contentDir.c_str());
}

void MaterialSerializer::Deserialize(const std::string& filepath, Material* owner)
{
    std::string contentDir = ContentDir + filepath;

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(contentDir.c_str()) != XML_SUCCESS)
    {
        return;
    }

    XMLElement* root = doc.FirstChildElement("GameAsset");
    if (!root)
    {
        return;
    }

    MaterialInitializationData* materialInitializationData = owner->GetInitializationData();

    const char* fileTypeAttr = root->Attribute("FileType");
    if (!fileTypeAttr || std::string(fileTypeAttr) != "MaterialData") return;

    XMLElement* child = root->FirstChildElement("BoneCount");
    if (child && child->GetText())
    {
        materialInitializationData->boneCount = std::stoi(child->GetText());
    }
    else
    {
        materialInitializationData->boneCount = 0;
    }

    XMLElement* child = root->FirstChildElement("BlendModel");
    if (child && child->GetText())
    {
        std::string blendModel = child->GetText();
        owner->SetBlendModel(blendModel == "Masked" ? MaterialBlendModel::Masked :
            blendModel == "Transparent" ? MaterialBlendModel::Transparent :
            MaterialBlendModel::Opaque);
    }

    child = root->FirstChildElement("ShadingModel");
    if (child && child->GetText())
    {
        std::string shadingModel = child->GetText();
        owner->SetShadingModel(shadingModel == "Default" ? MaterialShadingModel::Default :
            shadingModel == "TwoSided" ? MaterialShadingModel::TwoSided :
            owner->GetShadingModel());
    }

    child = root->FirstChildElement("Texture");
    while (child)
    {
        if (child->Attribute("path"))
        {
            std::string texturePath = child->Attribute("path");
            owner->AddTextureImage(engine->GetResourceManager()->GetContent<Image>(texturePath));
        }
        child = child->NextSiblingElement("Texture");
    }

    child = root->FirstChildElement("AmbientReflectance");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        Vector3 ambientReflectance;
        stream >> ambientReflectance.x >> ambientReflectance.y >> ambientReflectance.z;
        owner->SetAmbientReflectance(ambientReflectance);
    }

    child = root->FirstChildElement("DiffuseReflectance");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        Vector3 diffuseReflectance;
        stream >> diffuseReflectance.x >> diffuseReflectance.y >> diffuseReflectance.z;
        owner->SetBaseColor(diffuseReflectance);
    }

    child = root->FirstChildElement("SpecularReflectance");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        Vector3 specularReflectance;
        stream >> specularReflectance.x >> specularReflectance.y >> specularReflectance.z;
        owner->SetSpecularReflectance(specularReflectance);
    }

    child = root->FirstChildElement("PhongExponent");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        float phongExponent;
        stream >> phongExponent;
        owner->SetPhongExponent(phongExponent);
    }
    else
    {
        owner->SetPhongExponent(1.f);
    }

    DeserializeShaderFunction(root, "BaseColor", materialInitializationData->baseColor);
    DeserializeShaderFunction(root, "EmissiveColor", materialInitializationData->emmisiveColor);
    DeserializeShaderFunction(root, "FragmentNormal", materialInitializationData->fragmentNormal);
    DeserializeShaderFunction(root, "VertexNormal", materialInitializationData->vertexNormal);
    DeserializeShaderFunction(root, "UV", materialInitializationData->uv);
    DeserializeShaderFunction(root, "VertexPositionOffset", materialInitializationData->vertexPositionOffset);

    auto GetTextContent =
        [&](const char* name) -> std::string
        {
            XMLElement* el = root->FirstChildElement(name);
            return el && el->GetText() ? el->GetText() : "";
        };

    materialInitializationData->vertexShaderFunctions = GetTextContent("VertexShaderFunctions");
    materialInitializationData->fragmentShaderFunctions = GetTextContent("FragmentShaderFunctions");
    materialInitializationData->vertexShaderUniforms = GetTextContent("VertexShaderUniforms");
    materialInitializationData->fragmentShaderUniforms = GetTextContent("FragmentShaderUniforms");
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