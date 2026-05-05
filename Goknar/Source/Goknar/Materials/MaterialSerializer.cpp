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
		case TextureUsage::Emmisive: return "Emmisive";
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
		if (textureUsage == "Emmisive") return TextureUsage::Emmisive;
		if (textureUsage == "Roughness") return TextureUsage::Roughness;
		if (textureUsage == "Height") return TextureUsage::Height;
		return TextureUsage::None;
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

    const std::vector<const Image*>* textureImages = material->GetTextureImages();
    if (textureImages)
    {
        for (const Image* image : *textureImages)
        {
            if (image)
            {
                XMLElement* texElement = doc.NewElement("Texture");
                const std::string texturePath = ContentPathUtils::ToContentRelativePath(image->GetPath());
                texElement->SetAttribute("path", texturePath.c_str());
                texElement->SetAttribute("usage", TextureUsageToString(image->GetTextureUsage()));
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
    SerializeVector3("EmmisiveColorValue", material->GetEmisiveColor());
    AddPropertyElement("AmbientOcclusionValue", std::to_string(material->GetAmbientOcclusion()));
    AddPropertyElement("MetallicValue", std::to_string(material->GetMetallic()));
    AddPropertyElement("RoughnessValue", std::to_string(material->GetRoughness()));

    AddPropertyElement("Translucency", std::to_string(material->GetTranslucency()));

    SerializeShaderFunction(doc, root, "BaseColor", materialInitializationData->baseColor);
    SerializeShaderFunction(doc, root, "EmissiveColor", materialInitializationData->emisiveColor);
    SerializeShaderFunction(doc, root, "AmbientOcclusion", materialInitializationData->ambientOcclusion);
    SerializeShaderFunction(doc, root, "Metallic", materialInitializationData->metallic);
    SerializeShaderFunction(doc, root, "Roughness", materialInitializationData->roughness);
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
                owner->AddTextureImage(image);
            }
        }
        child = child->NextSiblingElement("Texture");
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

    child = root->FirstChildElement("DiffuseReflectance");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        Vector3 diffuseReflectance;
        stream >> diffuseReflectance.x >> diffuseReflectance.y >> diffuseReflectance.z;
        owner->SetBaseColor(diffuseReflectance);
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

    child = root->FirstChildElement("EmisiveColorValue");
    if (child && child->GetText())
    {
        std::stringstream stream(child->GetText());
        Vector3 emmisiveColor;
        stream >> emmisiveColor.x >> emmisiveColor.y >> emmisiveColor.z;
        owner->SetEmisiveColor(emmisiveColor);
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
    DeserializeShaderFunction(root, "EmissiveColor", materialInitializationData->emisiveColor);
    DeserializeShaderFunction(root, "AmbientOcclusion", materialInitializationData->ambientOcclusion);
    DeserializeShaderFunction(root, "Metallic", materialInitializationData->metallic);
    DeserializeShaderFunction(root, "Roughness", materialInitializationData->roughness);
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
