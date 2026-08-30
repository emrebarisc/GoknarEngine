#include "MaterialSerializer.h"

#include <cmath>
#include <sstream>

#include "tinyxml2.h"

#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Engine.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"

using namespace tinyxml2;

namespace
{
    constexpr float DEFAULT_AMBIENT_OCCLUSION = 1.f;
    constexpr float DEFAULT_METALLIC = 0.f;
    constexpr float DEFAULT_ROUGHNESS = 0.5f;

    bool LoadXmlDocumentFromPath(const std::string& filePath, tinyxml2::XMLDocument& document)
    {
        std::string fileContents;
        if (!DataEncryption::ReadTextFile(filePath, fileContents))
        {
            return false;
        }

        return document.Parse(fileContents.c_str(), fileContents.size()) == XML_SUCCESS;
    }

    float ClampNormalizedScalar(float value, float defaultValue)
    {
        if (!std::isfinite(value))
        {
            return defaultValue;
        }

        return GoknarMath::Clamp(value, 0.f, 1.f);
    }

    float ConvertLegacyPhongExponentToRoughness(float phongExponent)
    {
        if (!std::isfinite(phongExponent) || phongExponent < 1.f)
        {
            return DEFAULT_ROUGHNESS;
        }

        return ClampNormalizedScalar(std::sqrt(2.f / (phongExponent + 2.f)), DEFAULT_ROUGHNESS);
    }

    const char* TextureUsageToString(TextureUsage textureUsage)
    {
        switch (textureUsage)
        {
        case TextureUsage::Diffuse: return "Diffuse";
        case TextureUsage::Normal: return "Normal";
        case TextureUsage::AmbientOcclusion: return "AmbientOcclusion";
        case TextureUsage::Metallic: return "Metallic";
        case TextureUsage::Specular: return "Specular";
        case TextureUsage::Emissive: return "Emissive";
        case TextureUsage::Roughness: return "Roughness";
        case TextureUsage::Height: return "Height";
        case TextureUsage::None:
        default:
            return "None";
        }
    }

    TextureUsage StringToTextureUsage(const std::string& textureUsage)
    {
        if (textureUsage == "Diffuse") return TextureUsage::Diffuse;
        if (textureUsage == "Normal") return TextureUsage::Normal;
        if (textureUsage == "AmbientOcclusion") return TextureUsage::AmbientOcclusion;
        if (textureUsage == "Metallic") return TextureUsage::Metallic;
        if (textureUsage == "Specular") return TextureUsage::Specular;
        if (textureUsage == "Emissive") return TextureUsage::Emissive;
        if (textureUsage == "Roughness") return TextureUsage::Roughness;
        if (textureUsage == "Height") return TextureUsage::Height;
        return TextureUsage::None;
    }

    bool StringToBool(const std::string& value, bool defaultValue)
    {
        if (value == "1" || value == "true" || value == "True" || value == "TRUE" || value == "yes" || value == "Yes")
        {
            return true;
        }

        if (value == "0" || value == "false" || value == "False" || value == "FALSE" || value == "no" || value == "No")
        {
            return false;
        }

        return defaultValue;
    }

    bool ReadBoolAttributeOrElement(const tinyxml2::XMLElement* element, const char* name, bool defaultValue)
    {
        if (!element || !name)
        {
            return defaultValue;
        }

        if (const char* attributeValue = element->Attribute(name))
        {
            return StringToBool(attributeValue, defaultValue);
        }

        const tinyxml2::XMLElement* childElement = element->FirstChildElement(name);
        if (childElement && childElement->GetText())
        {
            return StringToBool(childElement->GetText(), defaultValue);
        }

        return defaultValue;
    }

    bool ReadTextureAtlasUsage(const tinyxml2::XMLElement* element, bool defaultValue)
    {
        bool value = defaultValue;
        value = ReadBoolAttributeOrElement(element, "UseTextureAtlas", value);
        value = ReadBoolAttributeOrElement(element, "CanUseTextureAtlas", value);
        value = ReadBoolAttributeOrElement(element, "TextureAtlas", value);
        return value;
    }
}

void MaterialSerializer::Serialize(const std::string& filepath, const Material* material)
{
    const MaterialInitializationData* materialInitializationData = material->GetInitializationData();
    const std::string relativeFilePath = ContentPathUtils::ToContentRelativePath(filepath);
    const std::string contentPath = ContentPathUtils::ToAbsoluteContentPath(relativeFilePath);

    tinyxml2::XMLDocument doc;

    XMLElement* root = doc.NewElement("GameAsset");

    root->SetAttribute("FileType", "Material");
    root->SetAttribute("UseTextureAtlas", material->GetUseTextureAtlasForTextureImages() ? "true" : "false");
    doc.InsertFirstChild(root);

    auto AddPropertyElement = [&](const char* name, const std::string& content)
        {
            XMLElement* el = doc.NewElement(name);
            el->SetText(content.c_str());
            root->InsertEndChild(el);
        };

    AddPropertyElement("Name", material->GetName());
    AddPropertyElement("BoneCount", std::to_string(materialInitializationData->boneCount));

    MaterialBlendModel blendModel = material->GetBlendModel();
    AddPropertyElement("BlendModel", blendModel == MaterialBlendModel::Masked ? "Masked" :
        blendModel == MaterialBlendModel::Transparent ? "Transparent" : "Opaque");

    MaterialShadingModel shadingModel = material->GetShadingModel();
    AddPropertyElement("ShadingModel", shadingModel == MaterialShadingModel::TwoSided ? "TwoSided" : "Default");

    MaterialShadingType shadingType = material->GetShadingType();
    AddPropertyElement("ShadingType", shadingType == MaterialShadingType::Unlit ? "Unlit" : "Default");

    AddPropertyElement("UsesReflectionProbe", material->GetUsesReflectionProbe() ? "1" : "0");

    const std::vector<const Image*>* textureImages = material->GetTextureImages();
    if (textureImages)
    {
        for (size_t textureImageIndex = 0; textureImageIndex < textureImages->size(); ++textureImageIndex)
        {
            const Image* image = (*textureImages)[textureImageIndex];
            if (image)
            {
                XMLElement* texElement = doc.NewElement("Texture");
                const std::string texturePath = ContentPathUtils::ToContentRelativePath(image->GetPath());
                texElement->SetAttribute("path", texturePath.c_str());
                texElement->SetAttribute("usage", TextureUsageToString(image->GetTextureUsage()));
                texElement->SetAttribute("UseTextureAtlas", material->GetTextureImageUsesTextureAtlas(textureImageIndex) ? "true" : "false");
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

    auto SerializeVector4 = [&](const char* name, const Vector4& vec)
        {
            std::stringstream ss;
            ss << vec.x << " " << vec.y << " " << vec.z << " " << vec.w;
            AddPropertyElement(name, ss.str());
        };

    SerializeVector4("BaseColorValue", material->GetBaseColor());
    SerializeVector3("EmissiveColorValue", material->GetEmissiveColor());
    AddPropertyElement("AmbientOcclusionValue", std::to_string(material->GetAmbientOcclusion()));
    AddPropertyElement("MetallicValue", std::to_string(material->GetMetallic()));
    AddPropertyElement("RoughnessValue", std::to_string(material->GetRoughness()));

    AddPropertyElement("Translucency", std::to_string(material->GetTranslucency()));

    SerializeShaderFunction(doc, root, "BaseColor", materialInitializationData->baseColor);
    SerializeShaderFunction(doc, root, "EmissiveColor", materialInitializationData->emissiveColor);
    SerializeShaderFunction(doc, root, "AmbientOcclusion", materialInitializationData->ambientOcclusion);
    SerializeShaderFunction(doc, root, "Metallic", materialInitializationData->metallic);
    SerializeShaderFunction(doc, root, "Roughness", materialInitializationData->roughness);
    SerializeShaderFunction(doc, root, "FragmentNormal", materialInitializationData->fragmentNormal);
    AddPropertyElement("FragmentNormalIsTangentSpace", materialInitializationData->fragmentNormalIsTangentSpace ? "1" : "0");
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

    doc.SaveFile(contentPath.c_str());
}

void MaterialSerializer::Deserialize(const std::string& filepath, Material* owner)
{
    const std::string relativeFilePath = ContentPathUtils::ToContentRelativePath(filepath);
    const std::string contentPath = ContentPathUtils::ToAbsoluteContentPath(relativeFilePath);

    tinyxml2::XMLDocument doc;
    if (!LoadXmlDocumentFromPath(contentPath, doc))
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
    if (!fileTypeAttr || std::string(fileTypeAttr) != "Material") return;

    const bool materialDefaultUseTextureAtlas = ReadTextureAtlasUsage(root, true);
    owner->SetUseTextureAtlasForTextureImages(materialDefaultUseTextureAtlas);

    XMLElement* child = root->FirstChildElement("Name");
    if (child && child->GetText())
    {
        owner->SetName(child->GetText());
    }

    child = root->FirstChildElement("BoneCount");
    if (child && child->GetText())
    {
        materialInitializationData->boneCount = std::stoi(child->GetText());
    }
    else
    {
        materialInitializationData->boneCount = 0;
    }

    child = root->FirstChildElement("BlendModel");
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

    child = root->FirstChildElement("ShadingType");
    if (child && child->GetText())
    {
        std::string shadingType = child->GetText();
        owner->SetShadingType(shadingType == "Default" ? MaterialShadingType::Default :
            shadingType == "Unlit" ? MaterialShadingType::Unlit :
            owner->GetShadingType());
    }

    child = root->FirstChildElement("UsesReflectionProbe");
    if (child && child->GetText())
    {
        owner->SetUsesReflectionProbe(std::string(child->GetText()) == "1");
    }
    else
    {
        owner->SetUsesReflectionProbe(false);
    }

    bool registeredTextureAtlasImage = false;
    child = root->FirstChildElement("Texture");
    while (child)
    {
        if (child->Attribute("path"))
        {
            std::string texturePath = ContentPathUtils::ToContentRelativePath(child->Attribute("path"));

            Image* image = engine->GetResourceManager()->GetContent<Image>(texturePath);

            if (image)
            {
                if (child->Attribute("usage"))
                {
                    image->SetTextureUsage(StringToTextureUsage(child->Attribute("usage")));
                }

                const bool useTextureAtlas = ReadTextureAtlasUsage(child, materialDefaultUseTextureAtlas);
                owner->AddTextureImage(image, useTextureAtlas);
                registeredTextureAtlasImage = registeredTextureAtlasImage || useTextureAtlas;
            }
        }
        child = child->NextSiblingElement("Texture");
    }

    if (registeredTextureAtlasImage)
    {
        ResourceManager* resourceManager = engine ? engine->GetResourceManager() : nullptr;
        if (resourceManager && resourceManager->GetResourceContainer())
        {
            resourceManager->GetResourceContainer()->FlushImageTextureAtlas();
        }
    }

    child = root->FirstChildElement("AmbientOcclusionValue");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        float ambientOcclusion = DEFAULT_AMBIENT_OCCLUSION;
        stream >> ambientOcclusion;
        owner->SetAmbientOcclusion(ClampNormalizedScalar(ambientOcclusion, DEFAULT_AMBIENT_OCCLUSION));
    }
    else
    {
        owner->SetAmbientOcclusion(DEFAULT_AMBIENT_OCCLUSION);
    }

    child = root->FirstChildElement("BaseColorValue");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        Vector4 baseColor;
        stream >> baseColor.x >> baseColor.y >> baseColor.z >> baseColor.w;
        owner->SetBaseColor(baseColor);
    }

    child = root->FirstChildElement("MetallicValue");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        float metallic = DEFAULT_METALLIC;
        stream >> metallic;
        owner->SetMetallic(ClampNormalizedScalar(metallic, DEFAULT_METALLIC));
    }
    else
    {
        owner->SetMetallic(DEFAULT_METALLIC);
    }

    child = root->FirstChildElement("EmissiveColorValue");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        Vector3 emissiveColor;
        stream >> emissiveColor.x >> emissiveColor.y >> emissiveColor.z;
        owner->SetEmissiveColor(emissiveColor);
    }

    child = root->FirstChildElement("RoughnessValue");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        float roughness = DEFAULT_ROUGHNESS;
        stream >> roughness;
        owner->SetRoughness(ClampNormalizedScalar(roughness, DEFAULT_ROUGHNESS));
    }
    else
    {
        child = root->FirstChildElement("PhongExponent");
        if (child && child->GetText())
        {
            std::stringstream stream(child->GetText());
            float phongExponent = 1.f;
            stream >> phongExponent;
            owner->SetRoughness(ConvertLegacyPhongExponentToRoughness(phongExponent));
        }
        else
        {
            owner->SetRoughness(DEFAULT_ROUGHNESS);
        }
    }

    child = root->FirstChildElement("Translucency");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        float translucency;
        stream >> translucency;
        owner->SetTranslucency(translucency);
    }

    DeserializeShaderFunction(root, "BaseColor", materialInitializationData->baseColor);
    DeserializeShaderFunction(root, "EmissiveColor", materialInitializationData->emissiveColor);
    DeserializeShaderFunction(root, "AmbientOcclusion", materialInitializationData->ambientOcclusion);
    DeserializeShaderFunction(root, "Metallic", materialInitializationData->metallic);
    DeserializeShaderFunction(root, "Roughness", materialInitializationData->roughness);
    DeserializeShaderFunction(root, "FragmentNormal", materialInitializationData->fragmentNormal);
    child = root->FirstChildElement("FragmentNormalIsTangentSpace");
    materialInitializationData->fragmentNormalIsTangentSpace = child && child->GetText() ? std::string(child->GetText()) == "1" || std::string(child->GetText()) == "true" : false;
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
