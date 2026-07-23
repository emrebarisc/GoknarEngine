#include "pch.h"
#include "AssetParser.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <map>
#include <set>

#include "Goknar/Engine.h"
#include "Goknar/Contents/Audio.h"
#include "Goknar/Contents/Content.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Materials/MaterialSerializer.h"

namespace
{
	using MeshMaterialPathMap = std::map<std::string, std::vector<std::string>>;

	Material* GetMeshMaterial(Content* content, size_t subMeshIndex)
	{
		if (StaticMesh* staticMesh = dynamic_cast<StaticMesh*>(content))
		{
			const auto& subMeshes = staticMesh->GetSubMeshes();
			return subMeshIndex < subMeshes.size() ? subMeshes[subMeshIndex]->GetMaterial() : nullptr;
		}

		if (SkeletalMesh* skeletalMesh = dynamic_cast<SkeletalMesh*>(content))
		{
			const auto& subMeshes = skeletalMesh->GetSubMeshes();
			return subMeshIndex < subMeshes.size() ? subMeshes[subMeshIndex]->GetMaterial() : nullptr;
		}

		return nullptr;
	}

	std::string NormalizePath(const std::string& path)
	{
		return std::filesystem::path(path).lexically_normal().generic_string();
	}

	std::string GetRelativeContentPath(const std::filesystem::path& absolutePath)
	{
		return ContentPathUtils::ToContentRelativePath(absolutePath.generic_string());
	}

	std::string TrimString(const std::string& value)
	{
		const size_t firstNonWhitespaceCharacterIndex = value.find_first_not_of(" \t\n\r");
		if (firstNonWhitespaceCharacterIndex == std::string::npos)
		{
			return "";
		}

		const size_t lastNonWhitespaceCharacterIndex = value.find_last_not_of(" \t\n\r");
		return value.substr(firstNonWhitespaceCharacterIndex, lastNonWhitespaceCharacterIndex - firstNonWhitespaceCharacterIndex + 1);
	}

	std::string GetElementText(const tinyxml2::XMLElement* element)
	{
		if (!element || !element->GetText())
		{
			return "";
		}

		return TrimString(element->GetText());
	}

	bool TryParseBool(const std::string& value, bool& outValue)
	{
		std::string normalizedValue = TrimString(value);
		std::transform(normalizedValue.begin(), normalizedValue.end(), normalizedValue.begin(), [](unsigned char character)
			{
				return static_cast<char>(std::tolower(character));
			});

		if (normalizedValue == "true" || normalizedValue == "1" || normalizedValue == "yes" || normalizedValue == "enabled")
		{
			outValue = true;
			return true;
		}

		if (normalizedValue == "false" || normalizedValue == "0" || normalizedValue == "no" || normalizedValue == "disabled")
		{
			outValue = false;
			return true;
		}

		return false;
	}

	bool ReadBoolAttributeOrElement(const tinyxml2::XMLElement* element, const char* name, bool defaultValue = false)
	{
		if (!element || !name)
		{
			return defaultValue;
		}

		bool parsedValue = defaultValue;
		const char* attributeValue = element->Attribute(name);
		if (attributeValue && TryParseBool(attributeValue, parsedValue))
		{
			return parsedValue;
		}

		const tinyxml2::XMLElement* childElement = element->FirstChildElement(name);
		if (childElement && childElement->GetText() && TryParseBool(childElement->GetText(), parsedValue))
		{
			return parsedValue;
		}

		return defaultValue;
	}

	bool ReadTextureAtlasUsageInternal(const tinyxml2::XMLElement* textureElement, bool defaultValue = false)
	{
		if (!textureElement)
		{
			return defaultValue;
		}

		bool value = defaultValue;
		value = ReadBoolAttributeOrElement(textureElement, "UseTextureAtlas", value);
		value = ReadBoolAttributeOrElement(textureElement, "CanUseTextureAtlas", value);
		value = ReadBoolAttributeOrElement(textureElement, "TextureAtlas", value);
		return value;
	}

	bool ReadTextureUploadToGPUInternal(const tinyxml2::XMLElement* textureElement, bool defaultValue = true)
	{
		if (!textureElement)
		{
			return defaultValue;
		}

		bool value = defaultValue;
		value = ReadBoolAttributeOrElement(textureElement, "UploadToGPU", value);
		value = ReadBoolAttributeOrElement(textureElement, "UploadToGpu", value);
		value = ReadBoolAttributeOrElement(textureElement, "uploadToGPU", value);
		value = ReadBoolAttributeOrElement(textureElement, "UploadTextureToGPU", value);
		return value;
	}

	std::string NormalizeTextureUsageToken(std::string value)
	{
		value = TrimString(value);
		value.erase(
			std::remove_if(
				value.begin(),
				value.end(),
				[](unsigned char character)
				{
					return character == ' ' || character == '_' || character == '-';
				}),
			value.end());

		std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character)
			{
				return static_cast<char>(std::tolower(character));
			});

		return value;
	}

	const char* TextureUsageToStringInternal(TextureUsage textureUsage)
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

	TextureUsage StringToTextureUsageInternal(const std::string& textureUsage)
	{
		const std::string normalizedTextureUsage = NormalizeTextureUsageToken(textureUsage);
		if (normalizedTextureUsage == "diffuse" || normalizedTextureUsage == "basecolor" || normalizedTextureUsage == "albedo") return TextureUsage::Diffuse;
		if (normalizedTextureUsage == "normal") return TextureUsage::Normal;
		if (normalizedTextureUsage == "ambientocclusion" || normalizedTextureUsage == "ao") return TextureUsage::AmbientOcclusion;
		if (normalizedTextureUsage == "metallic" || normalizedTextureUsage == "metalness") return TextureUsage::Metallic;
		if (normalizedTextureUsage == "specular") return TextureUsage::Specular;
		if (normalizedTextureUsage == "emissive") return TextureUsage::Emissive;
		if (normalizedTextureUsage == "roughness") return TextureUsage::Roughness;
		if (normalizedTextureUsage == "height" || normalizedTextureUsage == "displacement") return TextureUsage::Height;
		return TextureUsage::None;
	}

	bool ReadStringAttributeOrElement(const tinyxml2::XMLElement* element, const char* name, std::string& outValue)
	{
		if (!element || !name)
		{
			return false;
		}

		const char* attributeValue = element->Attribute(name);
		if (attributeValue)
		{
			outValue = TrimString(attributeValue);
			return true;
		}

		const tinyxml2::XMLElement* childElement = element->FirstChildElement(name);
		if (childElement && childElement->GetText())
		{
			outValue = GetElementText(childElement);
			return true;
		}

		return false;
	}

	TextureUsage ReadTextureUsageInternal(const tinyxml2::XMLElement* textureElement, TextureUsage defaultValue = TextureUsage::Diffuse)
	{
		if (!textureElement)
		{
			return defaultValue;
		}

		std::string textureUsage;
		if (ReadStringAttributeOrElement(textureElement, "Usage", textureUsage) ||
			ReadStringAttributeOrElement(textureElement, "usage", textureUsage) ||
			ReadStringAttributeOrElement(textureElement, "TextureUsage", textureUsage))
		{
			return StringToTextureUsageInternal(textureUsage);
		}

		return defaultValue;
	}

	std::string GetTextureNameFromPath(const std::string& texturePath)
	{
		const std::string textureName = std::filesystem::path(texturePath).stem().string();
		return textureName.empty() ? "Texture" : textureName;
	}

	std::string GetUniqueTextureName(const std::string& preferredTextureName, std::set<std::string>& usedTextureNames)
	{
		const std::string baseTextureName = preferredTextureName.empty() ? "Texture" : preferredTextureName;
		const auto [baseTextureNameIterator, isBaseTextureNameInserted] = usedTextureNames.insert(baseTextureName);
		if (isBaseTextureNameInserted)
		{
			return baseTextureName;
		}

		int textureNamePostfix = 1;
		std::string uniqueTextureName;
		do
		{
			uniqueTextureName = baseTextureName + "_" + std::to_string(textureNamePostfix++);
		} while (!usedTextureNames.insert(uniqueTextureName).second);

		return uniqueTextureName;
	}

	void TrimTrailingEmptyMaterialPaths(std::vector<std::string>& materialPaths)
	{
		while (!materialPaths.empty() && materialPaths.back().empty())
		{
			materialPaths.pop_back();
		}
	}

	std::vector<std::string> NormalizeMaterialPaths(const std::vector<std::string>& materialPaths)
	{
		std::vector<std::string> normalizedMaterialPaths;
		normalizedMaterialPaths.reserve(materialPaths.size());

		for (const std::string& materialPath : materialPaths)
		{
			normalizedMaterialPaths.push_back(ContentPathUtils::ToContentRelativePath(materialPath));
		}

		TrimTrailingEmptyMaterialPaths(normalizedMaterialPaths);
		return normalizedMaterialPaths;
	}

	std::vector<std::string> GetMaterialPathsFromElement(const tinyxml2::XMLElement* meshElement)
	{
		std::vector<std::string> materialPaths;
		if (!meshElement)
		{
			return materialPaths;
		}

		const tinyxml2::XMLElement* materialPathsElement = meshElement->FirstChildElement("MaterialPaths");
		if (materialPathsElement)
		{
			for (const tinyxml2::XMLElement* materialPathElement = materialPathsElement->FirstChildElement("MaterialPath");
				materialPathElement != nullptr;
				materialPathElement = materialPathElement->NextSiblingElement("MaterialPath"))
			{
				materialPaths.push_back(ContentPathUtils::ToContentRelativePath(GetElementText(materialPathElement)));
			}

			TrimTrailingEmptyMaterialPaths(materialPaths);
			return materialPaths;
		}

		const std::string materialPath = ContentPathUtils::ToContentRelativePath(GetElementText(meshElement->FirstChildElement("MaterialPath")));
		if (!materialPath.empty())
		{
			materialPaths.push_back(materialPath);
		}

		return materialPaths;
	}

	void RemoveMaterialPathElements(tinyxml2::XMLElement* meshElement)
	{
		if (!meshElement)
		{
			return;
		}

		if (tinyxml2::XMLElement* materialPathsElement = meshElement->FirstChildElement("MaterialPaths"))
		{
			meshElement->DeleteChild(materialPathsElement);
		}

		if (tinyxml2::XMLElement* materialPathElement = meshElement->FirstChildElement("MaterialPath"))
		{
			meshElement->DeleteChild(materialPathElement);
		}
	}

	void WriteMaterialPaths(tinyxml2::XMLDocument& document, tinyxml2::XMLElement* meshElement, const std::vector<std::string>& materialPaths)
	{
		if (!meshElement || materialPaths.empty())
		{
			return;
		}

		if (materialPaths.size() == 1)
		{
			tinyxml2::XMLElement* materialPathElement = document.NewElement("MaterialPath");
			materialPathElement->SetText(materialPaths[0].c_str());
			meshElement->InsertEndChild(materialPathElement);
			return;
		}

		tinyxml2::XMLElement* materialPathsElement = document.NewElement("MaterialPaths");
		for (const std::string& materialPath : materialPaths)
		{
			tinyxml2::XMLElement* materialPathElement = document.NewElement("MaterialPath");
			if (!materialPath.empty())
			{
				materialPathElement->SetText(materialPath.c_str());
			}

			materialPathsElement->InsertEndChild(materialPathElement);
		}

		meshElement->InsertEndChild(materialPathsElement);
	}

	MeshMaterialPathMap LoadExistingMeshMaterialPaths(const std::string& assetContainerPath)
	{
		MeshMaterialPathMap materialPathByMeshPath;

		tinyxml2::XMLDocument existingDocument;
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(assetContainerPath, fileContents) ||
			existingDocument.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
		{
			return materialPathByMeshPath;
		}

		tinyxml2::XMLElement* rootElement = existingDocument.FirstChildElement("AssetContainer");
		tinyxml2::XMLElement* assetsElement = rootElement ? rootElement->FirstChildElement("Assets") : nullptr;
		for (tinyxml2::XMLElement* meshElement = assetsElement ? assetsElement->FirstChildElement("Mesh") : nullptr;
			meshElement != nullptr;
			meshElement = meshElement->NextSiblingElement("Mesh"))
		{
			tinyxml2::XMLElement* pathElement = meshElement->FirstChildElement("Path");
			if (!pathElement || !pathElement->GetText())
			{
				continue;
			}

			const std::vector<std::string> materialPaths = GetMaterialPathsFromElement(meshElement);
			if (materialPaths.empty())
			{
				continue;
			}

			materialPathByMeshPath[ContentPathUtils::ToContentRelativePath(pathElement->GetText())] = materialPaths;
		}

		return materialPathByMeshPath;
	}

	std::map<std::string, std::string> LoadExistingTextureNames(const std::string& assetContainerPath)
	{
		std::map<std::string, std::string> textureNameByTexturePath;

		tinyxml2::XMLDocument existingDocument;
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(assetContainerPath, fileContents) ||
			existingDocument.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
		{
			return textureNameByTexturePath;
		}

		tinyxml2::XMLElement* rootElement = existingDocument.FirstChildElement("AssetContainer");
		tinyxml2::XMLElement* assetsElement = rootElement ? rootElement->FirstChildElement("Assets") : nullptr;
		for (tinyxml2::XMLElement* textureElement = assetsElement ? assetsElement->FirstChildElement("Texture") : nullptr;
			textureElement != nullptr;
			textureElement = textureElement->NextSiblingElement("Texture"))
		{
			const std::string texturePath = ContentPathUtils::ToContentRelativePath(GetElementText(textureElement->FirstChildElement("Path")));
			if (texturePath.empty())
			{
				continue;
			}

			const char* textureNameAttribute = textureElement->Attribute("Name");
			textureNameByTexturePath[texturePath] = textureNameAttribute ? TrimString(textureNameAttribute) : GetElementText(textureElement->FirstChildElement("Name"));
		}

		return textureNameByTexturePath;
	}


	std::map<std::string, bool> LoadExistingTextureAtlasUsage(const std::string& assetContainerPath)
	{
		std::map<std::string, bool> textureAtlasUsageByTexturePath;

		tinyxml2::XMLDocument existingDocument;
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(assetContainerPath, fileContents) ||
			existingDocument.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
		{
			return textureAtlasUsageByTexturePath;
		}

		tinyxml2::XMLElement* rootElement = existingDocument.FirstChildElement("AssetContainer");
		tinyxml2::XMLElement* assetsElement = rootElement ? rootElement->FirstChildElement("Assets") : nullptr;
		for (tinyxml2::XMLElement* textureElement = assetsElement ? assetsElement->FirstChildElement("Texture") : nullptr;
			textureElement != nullptr;
			textureElement = textureElement->NextSiblingElement("Texture"))
		{
			const std::string texturePath = ContentPathUtils::ToContentRelativePath(GetElementText(textureElement->FirstChildElement("Path")));
			if (texturePath.empty())
			{
				continue;
			}

			textureAtlasUsageByTexturePath[texturePath] = ReadTextureAtlasUsageInternal(textureElement, false);
		}

		return textureAtlasUsageByTexturePath;
	}

	std::map<std::string, TextureUsage> LoadExistingTextureUsages(const std::string& assetContainerPath)
	{
		std::map<std::string, TextureUsage> textureUsageByTexturePath;

		tinyxml2::XMLDocument existingDocument;
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(assetContainerPath, fileContents) ||
			existingDocument.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
		{
			return textureUsageByTexturePath;
		}

		tinyxml2::XMLElement* rootElement = existingDocument.FirstChildElement("AssetContainer");
		tinyxml2::XMLElement* assetsElement = rootElement ? rootElement->FirstChildElement("Assets") : nullptr;
		for (tinyxml2::XMLElement* textureElement = assetsElement ? assetsElement->FirstChildElement("Texture") : nullptr;
			textureElement != nullptr;
			textureElement = textureElement->NextSiblingElement("Texture"))
		{
			const std::string texturePath = ContentPathUtils::ToContentRelativePath(GetElementText(textureElement->FirstChildElement("Path")));
			if (texturePath.empty())
			{
				continue;
			}

			textureUsageByTexturePath[texturePath] = ReadTextureUsageInternal(textureElement);
		}

		return textureUsageByTexturePath;
	}

	std::map<std::string, bool> LoadExistingTextureUploadToGPU(const std::string& assetContainerPath)
	{
		std::map<std::string, bool> textureUploadToGPUByTexturePath;

		tinyxml2::XMLDocument existingDocument;
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(assetContainerPath, fileContents) ||
			existingDocument.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
		{
			return textureUploadToGPUByTexturePath;
		}

		tinyxml2::XMLElement* rootElement = existingDocument.FirstChildElement("AssetContainer");
		tinyxml2::XMLElement* assetsElement = rootElement ? rootElement->FirstChildElement("Assets") : nullptr;
		for (tinyxml2::XMLElement* textureElement = assetsElement ? assetsElement->FirstChildElement("Texture") : nullptr;
			textureElement != nullptr;
			textureElement = textureElement->NextSiblingElement("Texture"))
		{
			const std::string texturePath = ContentPathUtils::ToContentRelativePath(GetElementText(textureElement->FirstChildElement("Path")));
			if (texturePath.empty())
			{
				continue;
			}

			textureUploadToGPUByTexturePath[texturePath] = ReadTextureUploadToGPUInternal(textureElement, true);
		}

		return textureUploadToGPUByTexturePath;
	}

	std::string TryGetGameAssetFileType(const std::filesystem::path& absolutePath)
	{
		tinyxml2::XMLDocument document;
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(absolutePath.generic_string(), fileContents) ||
			document.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
		{
			return "";
		}

		tinyxml2::XMLElement* root = document.FirstChildElement("GameAsset");
		const char* fileType = root ? root->Attribute("FileType") : nullptr;
		return fileType ? fileType : "";
	}

	TextureAtlasCategory GetTextureAtlasCategory(const Material* material)
	{
		return material && material->GetBlendModel() == MaterialBlendModel::Transparent ?
			TextureAtlasCategory::Transparent :
			TextureAtlasCategory::Opaque;
	}

}


bool AssetParser::ReadTextureAtlasUsage(const tinyxml2::XMLElement* textureElement, bool defaultValue)
{
	return ReadTextureAtlasUsageInternal(textureElement, defaultValue);
}

bool AssetParser::ReadTextureUploadToGPU(const tinyxml2::XMLElement* textureElement, bool defaultValue)
{
	return ReadTextureUploadToGPUInternal(textureElement, defaultValue);
}

bool AssetParser::RegisterTextureToTextureAtlas(Texture* texture, bool useTextureNameForImage, bool flushAtlas, TextureAtlasCategory category)
{
	if (!texture || texture->GetTextureImagePath().empty())
	{
		return false;
	}

	ResourceManager* resourceManager = engine->GetResourceManager();
	if (!resourceManager || !resourceManager->GetResourceContainer())
	{
		return false;
	}

	const std::string relativeTexturePath = ContentPathUtils::ToContentRelativePath(texture->GetTextureImagePath());
	if (relativeTexturePath.empty())
	{
		return false;
	}

	Image* image = resourceManager->GetContent<Image>(relativeTexturePath);
	if (!image)
	{
		return false;
	}

	image->SetCanUseTextureAtlas(true);
	image->AddTextureAtlasCategory(category);
	texture->SetTextureAtlasCategory(category);

	if (useTextureNameForImage && !texture->GetName().empty())
	{
		image->SetName(texture->GetName());
	}

	const bool registered = resourceManager->GetResourceContainer()->RegisterImageToTextureAtlas(image, category);
	if (!registered)
	{
		texture->SetWaitsForTextureAtlas(false);
		return false;
	}

	texture->SetWaitsForTextureAtlas(true);
	image->RegisterTextureAtlasProxy(texture, category);
	if (flushAtlas && registered)
	{
		resourceManager->GetResourceContainer()->FlushImageTextureAtlas();
	}

	return registered;
}

void AssetParser::RegisterMaterialTexturesToTextureAtlas(Material* material, bool flushAtlas)
{
	if (!material)
	{
		return;
	}

	Shader* shader = material->GetShader(RenderPassType::Forward);
	if (!shader)
	{
		return;
	}

	const std::vector<const Texture*>* textures = shader->GetTextures();
	if (!textures)
	{
		return;
	}

	bool registeredAnyTexture = false;
	const TextureAtlasCategory category = GetTextureAtlasCategory(material);
	for (const Texture* constTexture : *textures)
	{
		registeredAnyTexture |= RegisterTextureToTextureAtlas(const_cast<Texture*>(constTexture), false, false, category);
	}

	if (flushAtlas && registeredAnyTexture)
	{
		ResourceManager* resourceManager = engine->GetResourceManager();
		if (resourceManager && resourceManager->GetResourceContainer())
		{
			resourceManager->GetResourceContainer()->FlushImageTextureAtlas();
		}
	}
}

void AssetParser::ParseAssets(const std::string& filePath)
{
	const std::string fullPath = ContentPathUtils::ToAbsoluteContentPath(filePath);

	tinyxml2::XMLDocument xmlFile;
	tinyxml2::XMLError res;

	try
	{
		std::string fileContents;
		if (!DataEncryption::ReadTextFile(fullPath, fileContents))
		{
			res = tinyxml2::XML_ERROR_FILE_NOT_FOUND;
		}
		else
		{
			res = xmlFile.Parse(fileContents.c_str(), fileContents.size());
		}
		if (res)
		{
			throw std::runtime_error("Error: Asset XML file could not be loaded at " + fullPath + ".");
		}
	}
	catch (std::exception & exception)
	{
		std::cerr << exception.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	auto root = xmlFile.FirstChild();
	if (!root)
	{
		throw std::runtime_error("Error: Root could not be found in Asset XML.");
	}

	tinyxml2::XMLElement* assetsElement = root->FirstChildElement("Assets");
	if (assetsElement)
	{
		ParseMeshes(assetsElement);
		ParseTextures(assetsElement);
		ParseMaterials(assetsElement);
		ParseAudio(assetsElement);
	}
}

std::vector<std::string> AssetParser::GetMeshMaterialPaths(const std::string& meshPath, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const MeshMaterialPathMap meshMaterialPaths = LoadExistingMeshMaterialPaths(fullAssetContainerPath);
	const std::string relativeMeshPath = ContentPathUtils::ToContentRelativePath(meshPath);

	const auto meshMaterialPathIterator = meshMaterialPaths.find(relativeMeshPath);
	return meshMaterialPathIterator != meshMaterialPaths.end() ? meshMaterialPathIterator->second : std::vector<std::string>{};
}

std::string AssetParser::GetMeshMaterialPath(const std::string& meshPath, const std::string& assetContainerPath)
{
	const std::vector<std::string> materialPaths = GetMeshMaterialPaths(meshPath, assetContainerPath);
	return materialPaths.empty() ? "" : materialPaths[0];
}

void AssetParser::SetMeshMaterialPaths(const std::string& meshPath, const std::vector<std::string>& materialPaths, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const std::string relativeMeshPath = ContentPathUtils::ToContentRelativePath(meshPath);
	const std::vector<std::string> relativeMaterialPaths = NormalizeMaterialPaths(materialPaths);

	if (relativeMeshPath.empty())
	{
		return;
	}

	tinyxml2::XMLDocument assetContainerDocument;
	tinyxml2::XMLError loadResult = tinyxml2::XML_ERROR_FILE_NOT_FOUND;
	std::string fileContents;
	if (DataEncryption::ReadTextFile(fullAssetContainerPath, fileContents))
	{
		loadResult = assetContainerDocument.Parse(fileContents.c_str(), fileContents.size());
	}
	if (loadResult != tinyxml2::XML_SUCCESS)
	{
		assetContainerDocument.Clear();
		tinyxml2::XMLElement* rootElement = assetContainerDocument.NewElement("AssetContainer");
		assetContainerDocument.InsertFirstChild(rootElement);
		rootElement->InsertEndChild(assetContainerDocument.NewElement("Assets"));
	}

	tinyxml2::XMLElement* rootElement = assetContainerDocument.FirstChildElement("AssetContainer");
	if (!rootElement)
	{
		rootElement = assetContainerDocument.NewElement("AssetContainer");
		assetContainerDocument.InsertFirstChild(rootElement);
	}

	tinyxml2::XMLElement* assetsElement = rootElement->FirstChildElement("Assets");
	if (!assetsElement)
	{
		assetsElement = assetContainerDocument.NewElement("Assets");
		rootElement->InsertEndChild(assetsElement);
	}

	tinyxml2::XMLElement* meshElement = assetsElement->FirstChildElement("Mesh");
	for (; meshElement != nullptr; meshElement = meshElement->NextSiblingElement("Mesh"))
	{
		tinyxml2::XMLElement* pathElement = meshElement->FirstChildElement("Path");
		if (GetElementText(pathElement) == relativeMeshPath)
		{
			break;
		}
	}

	if (!meshElement)
	{
		meshElement = assetContainerDocument.NewElement("Mesh");
		tinyxml2::XMLElement* pathElement = assetContainerDocument.NewElement("Path");
		pathElement->SetText(relativeMeshPath.c_str());
		meshElement->InsertEndChild(pathElement);
		assetsElement->InsertEndChild(meshElement);
	}

	RemoveMaterialPathElements(meshElement);
	WriteMaterialPaths(assetContainerDocument, meshElement, relativeMaterialPaths);

	assetContainerDocument.SaveFile(fullAssetContainerPath.c_str());
}

void AssetParser::SetMeshMaterialPath(const std::string& meshPath, const std::string& materialPath, const std::string& assetContainerPath)
{
	if (materialPath.empty())
	{
		SetMeshMaterialPaths(meshPath, {}, assetContainerPath);
		return;
	}

	SetMeshMaterialPaths(meshPath, { materialPath }, assetContainerPath);
}

TextureUsage AssetParser::GetTextureUsage(const std::string& texturePath, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const std::string relativeTexturePath = ContentPathUtils::ToContentRelativePath(texturePath);

	if (relativeTexturePath.empty())
	{
		return TextureUsage::Diffuse;
	}

	const std::map<std::string, TextureUsage> textureUsages = LoadExistingTextureUsages(fullAssetContainerPath);
	const auto textureUsageIterator = textureUsages.find(relativeTexturePath);
	return textureUsageIterator != textureUsages.end() ? textureUsageIterator->second : TextureUsage::Diffuse;
}

bool AssetParser::SetTextureUsage(const std::string& texturePath, TextureUsage textureUsage, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const std::string relativeTexturePath = ContentPathUtils::ToContentRelativePath(texturePath);

	if (relativeTexturePath.empty())
	{
		return false;
	}

	tinyxml2::XMLDocument assetContainerDocument;
	tinyxml2::XMLError loadResult = tinyxml2::XML_ERROR_FILE_NOT_FOUND;
	std::string fileContents;
	if (DataEncryption::ReadTextFile(fullAssetContainerPath, fileContents))
	{
		loadResult = assetContainerDocument.Parse(fileContents.c_str(), fileContents.size());
	}
	if (loadResult != tinyxml2::XML_SUCCESS)
	{
		assetContainerDocument.Clear();
		tinyxml2::XMLElement* rootElement = assetContainerDocument.NewElement("AssetContainer");
		assetContainerDocument.InsertFirstChild(rootElement);
		rootElement->InsertEndChild(assetContainerDocument.NewElement("Assets"));
	}

	tinyxml2::XMLElement* rootElement = assetContainerDocument.FirstChildElement("AssetContainer");
	if (!rootElement)
	{
		rootElement = assetContainerDocument.NewElement("AssetContainer");
		assetContainerDocument.InsertFirstChild(rootElement);
	}

	tinyxml2::XMLElement* assetsElement = rootElement->FirstChildElement("Assets");
	if (!assetsElement)
	{
		assetsElement = assetContainerDocument.NewElement("Assets");
		rootElement->InsertEndChild(assetsElement);
	}

	tinyxml2::XMLElement* textureElement = assetsElement->FirstChildElement("Texture");
	for (; textureElement != nullptr; textureElement = textureElement->NextSiblingElement("Texture"))
	{
		tinyxml2::XMLElement* pathElement = textureElement->FirstChildElement("Path");
		if (ContentPathUtils::ToContentRelativePath(GetElementText(pathElement)) == relativeTexturePath)
		{
			break;
		}
	}

	if (!textureElement)
	{
		textureElement = assetContainerDocument.NewElement("Texture");
		tinyxml2::XMLElement* pathElement = assetContainerDocument.NewElement("Path");
		pathElement->SetText(relativeTexturePath.c_str());
		textureElement->InsertEndChild(pathElement);
		assetsElement->InsertEndChild(textureElement);
	}

	textureElement->SetAttribute("Usage", TextureUsageToStringInternal(textureUsage));
	return assetContainerDocument.SaveFile(fullAssetContainerPath.c_str()) == tinyxml2::XML_SUCCESS;
}

bool AssetParser::GetTextureUploadToGPU(const std::string& texturePath, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const std::string relativeTexturePath = ContentPathUtils::ToContentRelativePath(texturePath);

	if (relativeTexturePath.empty())
	{
		return true;
	}

	const std::map<std::string, bool> textureUploadToGPU = LoadExistingTextureUploadToGPU(fullAssetContainerPath);
	const auto textureUploadToGPUIterator = textureUploadToGPU.find(relativeTexturePath);
	return textureUploadToGPUIterator != textureUploadToGPU.end() ? textureUploadToGPUIterator->second : true;
}

bool AssetParser::SetTextureUploadToGPU(const std::string& texturePath, bool uploadToGPU, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const std::string relativeTexturePath = ContentPathUtils::ToContentRelativePath(texturePath);

	if (relativeTexturePath.empty())
	{
		return false;
	}

	tinyxml2::XMLDocument assetContainerDocument;
	tinyxml2::XMLError loadResult = tinyxml2::XML_ERROR_FILE_NOT_FOUND;
	std::string fileContents;
	if (DataEncryption::ReadTextFile(fullAssetContainerPath, fileContents))
	{
		loadResult = assetContainerDocument.Parse(fileContents.c_str(), fileContents.size());
	}
	if (loadResult != tinyxml2::XML_SUCCESS)
	{
		assetContainerDocument.Clear();
		tinyxml2::XMLElement* rootElement = assetContainerDocument.NewElement("AssetContainer");
		assetContainerDocument.InsertFirstChild(rootElement);
		rootElement->InsertEndChild(assetContainerDocument.NewElement("Assets"));
	}

	tinyxml2::XMLElement* rootElement = assetContainerDocument.FirstChildElement("AssetContainer");
	if (!rootElement)
	{
		rootElement = assetContainerDocument.NewElement("AssetContainer");
		assetContainerDocument.InsertFirstChild(rootElement);
	}

	tinyxml2::XMLElement* assetsElement = rootElement->FirstChildElement("Assets");
	if (!assetsElement)
	{
		assetsElement = assetContainerDocument.NewElement("Assets");
		rootElement->InsertEndChild(assetsElement);
	}

	tinyxml2::XMLElement* textureElement = assetsElement->FirstChildElement("Texture");
	for (; textureElement != nullptr; textureElement = textureElement->NextSiblingElement("Texture"))
	{
		tinyxml2::XMLElement* pathElement = textureElement->FirstChildElement("Path");
		if (ContentPathUtils::ToContentRelativePath(GetElementText(pathElement)) == relativeTexturePath)
		{
			break;
		}
	}

	if (!textureElement)
	{
		textureElement = assetContainerDocument.NewElement("Texture");
		tinyxml2::XMLElement* pathElement = assetContainerDocument.NewElement("Path");
		pathElement->SetText(relativeTexturePath.c_str());
		textureElement->InsertEndChild(pathElement);
		assetsElement->InsertEndChild(textureElement);
	}

	textureElement->SetAttribute("UploadToGPU", uploadToGPU ? "true" : "false");
	if (!uploadToGPU)
	{
		textureElement->SetAttribute("UseTextureAtlas", "false");
	}

	return assetContainerDocument.SaveFile(fullAssetContainerPath.c_str()) == tinyxml2::XML_SUCCESS;
}

TextureUsage AssetParser::ReadTextureUsage(const tinyxml2::XMLElement* textureElement, TextureUsage defaultValue)
{
	return ReadTextureUsageInternal(textureElement, defaultValue);
}

const char* AssetParser::TextureUsageToString(TextureUsage textureUsage)
{
	return TextureUsageToStringInternal(textureUsage);
}

TextureUsage AssetParser::StringToTextureUsage(const std::string& textureUsage)
{
	return StringToTextureUsageInternal(textureUsage);
}

void AssetParser::ParseMeshes(tinyxml2::XMLElement* assetsElement)
{
	std::stringstream stream;
	tinyxml2::XMLElement* element = assetsElement->FirstChildElement("Mesh");
	ResourceManager* resourceManager = engine->GetResourceManager();
	
	while (element)
	{
		Content* mesh = nullptr;

		tinyxml2::XMLElement* child = element->FirstChildElement("Path");
		if (child)
		{
			std::string path;
			stream << child->GetText() << std::endl;
			stream >> path;

			const std::string relativePath = ContentPathUtils::ToContentRelativePath(path);
			mesh = resourceManager->GetContent<StaticMesh>(relativePath);
			if (!mesh)
			{
				mesh = resourceManager->GetContent<SkeletalMesh>(relativePath);
			}
			stream.clear();
		}

		if (mesh)
		{
			const std::vector<std::string> materialPaths = GetMaterialPathsFromElement(element);
			for (size_t subMeshIndex = 0; subMeshIndex < materialPaths.size(); ++subMeshIndex)
			{
				const std::string relativeMaterialPath = ContentPathUtils::ToContentRelativePath(materialPaths[subMeshIndex]);
				if (relativeMaterialPath.empty())
				{
					continue;
				}

				Material* material = GetMeshMaterial(mesh, subMeshIndex);
				if (material)
				{
					MaterialSerializer::Deserialize(relativeMaterialPath, material);
					AssetParser::RegisterMaterialTexturesToTextureAtlas(material);
				}
			}
		}
		
		element = element->NextSiblingElement("Mesh");
	}
}

void AssetParser::ParseTextures(tinyxml2::XMLElement* assetsElement)
{
	tinyxml2::XMLElement* element = assetsElement->FirstChildElement("Texture");
	ResourceManager* resourceManager = engine->GetResourceManager();
	std::set<std::string> usedTextureNames;

	for (const Image* existingImage : resourceManager->GetResourceContainer()->GetImageArray())
	{
		if (!existingImage)
		{
			continue;
		}

		const std::string& existingTextureName = existingImage->GetName();
		if (!existingTextureName.empty())
		{
			usedTextureNames.insert(existingTextureName);
		}
	}

	while (element)
	{
		const std::string path = ContentPathUtils::ToContentRelativePath(GetElementText(element->FirstChildElement("Path")));
		if (!path.empty())
		{
			Image* image = resourceManager->GetContent<Image>(path);
			if (image)
			{
				// Asset container textures are material-facing by default, so they are
				// eligible for the atlas unless the asset XML opts out with
				// UseTextureAtlas/CanUseTextureAtlas/TextureAtlas = false.
				const bool uploadToGPU = ReadTextureUploadToGPUInternal(element, image->GetUploadToGPU());
				const bool useTextureAtlas = uploadToGPU && ReadTextureAtlasUsageInternal(element, true);
				image->SetTextureUsage(ReadTextureUsageInternal(element, image->GetTextureUsage()));
				image->SetUploadToGPU(uploadToGPU);
				image->SetCanUseTextureAtlas(useTextureAtlas);
				if (!uploadToGPU || image->GetGeneratedTexture())
				{
					image->GetOrCreateGeneratedTexture()->SetUploadToGPU(uploadToGPU);
				}

				if (useTextureAtlas)
				{
					resourceManager->GetResourceContainer()->RegisterImageToTextureAtlas(image);
				}

				if (image->GetName().empty())
				{
					const char* textureNameAttribute = element->Attribute("Name");
					std::string textureName = textureNameAttribute ? TrimString(textureNameAttribute) : GetElementText(element->FirstChildElement("Name"));
					if (textureName.empty())
					{
						textureName = GetTextureNameFromPath(path);
					}

					image->SetName(GetUniqueTextureName(textureName, usedTextureNames));
				}
			}
		}

		element = element->NextSiblingElement("Texture");
	}

	resourceManager->GetResourceContainer()->FlushImageTextureAtlas();
}

void AssetParser::ParseMaterials(tinyxml2::XMLElement* assetsElement)
{
	std::stringstream stream;
	tinyxml2::XMLElement* element = assetsElement->FirstChildElement("Material");
	ResourceManager* resourceManager = engine->GetResourceManager();

	while (element)
	{
		tinyxml2::XMLElement* child = element->FirstChildElement("Path");
		if (child)
		{
			std::string path;
			stream << child->GetText() << std::endl;
			stream >> path;
			path = ContentPathUtils::ToContentRelativePath(path);
			
			//resourceManager->GetContent<Material>(path);
			stream.clear();
		}
		element = element->NextSiblingElement("Material");
	}
}

void AssetParser::ParseAudio(tinyxml2::XMLElement* assetsElement)
{
	std::stringstream stream;
	tinyxml2::XMLElement* element = assetsElement->FirstChildElement("Audio");
	ResourceManager* resourceManager = engine->GetResourceManager();

	while (element)
	{
		tinyxml2::XMLElement* child = element->FirstChildElement("Path");
		if (child)
		{
			std::string path;
			stream << child->GetText() << std::endl;
			stream >> path;

			resourceManager->GetContent<Audio>(ContentPathUtils::ToContentRelativePath(path));

			stream.clear();
		}
		element = element->NextSiblingElement("Audio");
	}
}

void AssetParser::SaveAssets(const std::string& filePath)
{
	tinyxml2::XMLDocument assetXML;
	tinyxml2::XMLNode* rootElement = assetXML.NewElement("AssetContainer");
	assetXML.InsertFirstChild(rootElement);

	tinyxml2::XMLElement* assetsElement = assetXML.NewElement("Assets");

	const std::string relativeFilePath = ContentPathUtils::ToContentRelativePath(filePath);
	const std::string fullPath = ContentPathUtils::ToAbsoluteContentPath(relativeFilePath);
	const MeshMaterialPathMap meshMaterialPaths = LoadExistingMeshMaterialPaths(fullPath);
	const std::map<std::string, std::string> existingTextureNames = LoadExistingTextureNames(fullPath);
	const std::map<std::string, bool> existingTextureAtlasUsage = LoadExistingTextureAtlasUsage(fullPath);
	const std::map<std::string, TextureUsage> existingTextureUsages = LoadExistingTextureUsages(fullPath);
	const std::map<std::string, bool> existingTextureUploadToGPU = LoadExistingTextureUploadToGPU(fullPath);
	const std::filesystem::path contentRoot = std::filesystem::path(ContentDir);
	std::set<std::string> usedTextureNames;

	auto addPathAsset = [&](const char* elementName, const std::string& assetPath, const std::vector<std::string>* materialPaths = nullptr, const std::string* assetName = nullptr, bool useTextureAtlas = true, TextureUsage textureUsage = TextureUsage::Diffuse, bool uploadToGPU = true)
	{
		tinyxml2::XMLElement* assetElement = assetXML.NewElement(elementName);
		tinyxml2::XMLElement* pathElement = assetXML.NewElement("Path");
		pathElement->SetText(assetPath.c_str());

		if (assetName && !assetName->empty())
		{
			assetElement->SetAttribute("Name", assetName->c_str());
		}

		if (std::string(elementName) == "Texture")
		{
			assetElement->SetAttribute("UseTextureAtlas", useTextureAtlas ? "true" : "false");
			assetElement->SetAttribute("Usage", TextureUsageToStringInternal(textureUsage));
			assetElement->SetAttribute("UploadToGPU", uploadToGPU ? "true" : "false");
		}

		assetElement->InsertEndChild(pathElement);

		WriteMaterialPaths(assetXML, assetElement, materialPaths ? *materialPaths : std::vector<std::string>{});

		assetsElement->InsertEndChild(assetElement);
	};

	if (std::filesystem::exists(contentRoot) && std::filesystem::is_directory(contentRoot))
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(contentRoot, std::filesystem::directory_options::skip_permission_denied))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}

			const std::string relativeAssetPath = GetRelativeContentPath(entry.path());
			if (relativeAssetPath == relativeFilePath)
			{
				continue;
			}

			std::string extension = entry.path().extension().generic_string();
			std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char character)
				{
					return static_cast<char>(std::tolower(character));
				});

			if (extension == ".fbx")
			{
				auto existingMaterialPathIterator = meshMaterialPaths.find(relativeAssetPath);
				const std::vector<std::string>* materialPaths = existingMaterialPathIterator != meshMaterialPaths.end() ? &existingMaterialPathIterator->second : nullptr;
				addPathAsset("Mesh", relativeAssetPath, materialPaths);
				continue;
			}

			if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
			{
				const auto existingTextureNameIterator = existingTextureNames.find(relativeAssetPath);
				const std::string preferredTextureName =
					existingTextureNameIterator != existingTextureNames.end() && !existingTextureNameIterator->second.empty() ?
					existingTextureNameIterator->second :
					GetTextureNameFromPath(relativeAssetPath);
				const std::string resolvedTextureName = GetUniqueTextureName(preferredTextureName, usedTextureNames);
				const auto existingTextureAtlasUsageIterator = existingTextureAtlasUsage.find(relativeAssetPath);
				const bool useTextureAtlas = existingTextureAtlasUsageIterator == existingTextureAtlasUsage.end() ? true : existingTextureAtlasUsageIterator->second;
				const auto existingTextureUsageIterator = existingTextureUsages.find(relativeAssetPath);
				TextureUsage textureUsage = existingTextureUsageIterator == existingTextureUsages.end() ? TextureUsage::Diffuse : existingTextureUsageIterator->second;
				const auto existingTextureUploadToGPUIterator = existingTextureUploadToGPU.find(relativeAssetPath);
				bool uploadToGPU = existingTextureUploadToGPUIterator == existingTextureUploadToGPU.end() ? true : existingTextureUploadToGPUIterator->second;
				if ((existingTextureUsageIterator == existingTextureUsages.end() || existingTextureUploadToGPUIterator == existingTextureUploadToGPU.end()) &&
					engine && engine->GetResourceManager() && engine->GetResourceManager()->GetResourceContainer())
				{
					for (Image* image : engine->GetResourceManager()->GetResourceContainer()->GetImageArray())
					{
						if (image && ContentPathUtils::ToContentRelativePath(image->GetPath()) == relativeAssetPath)
						{
							if (existingTextureUsageIterator == existingTextureUsages.end())
							{
								textureUsage = image->GetTextureUsage();
							}

							if (existingTextureUploadToGPUIterator == existingTextureUploadToGPU.end())
							{
								uploadToGPU = image->GetUploadToGPU();
								if (image->GetGeneratedTexture())
								{
									uploadToGPU = image->GetGeneratedTexture()->GetUploadToGPU();
								}
							}
							break;
						}
					}
				}
				addPathAsset("Texture", relativeAssetPath, nullptr, &resolvedTextureName, useTextureAtlas && uploadToGPU, textureUsage, uploadToGPU);
				continue;
			}

			if (extension == ".wav")
			{
				addPathAsset("Audio", relativeAssetPath);
				continue;
			}

			const std::string gameAssetFileType = TryGetGameAssetFileType(entry.path());
			if (gameAssetFileType == "Material")
			{
				addPathAsset("Material", relativeAssetPath);
			}
			else if (gameAssetFileType == "MaterialFunction")
			{
				addPathAsset("MaterialFunction", relativeAssetPath);
			}
			else if (gameAssetFileType == "AnimationGraph")
			{
				addPathAsset("AnimationGraph", relativeAssetPath);
			}
			else if (gameAssetFileType == "NavigationTree")
			{
				addPathAsset("NavigationTree", relativeAssetPath);
			}
		}
	}

	rootElement->InsertEndChild(assetsElement);
	assetXML.SaveFile(fullPath.c_str());
}

void AssetParser::SaveMeshes(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const std::vector<Content*>& meshArray = engine->GetResourceManager()->GetResourceContainer()->GetMeshArray();

	for (Content* mesh : meshArray)
	{
		std::string fullPath = mesh->GetPath();

#ifdef ENGINE_CONTENT_DIR
		if (fullPath.find(EngineContentDir) != std::string::npos)
		{
			continue;
		}
#endif

		tinyxml2::XMLElement* meshElement = xmlDocument.NewElement("Mesh");
		tinyxml2::XMLElement* pathElement = xmlDocument.NewElement("Path");
		
		std::string path = ContentPathUtils::ToContentRelativePath(fullPath);
		pathElement->SetText(path.c_str());
		
		meshElement->InsertEndChild(pathElement);
		parentElement->InsertEndChild(meshElement);
	}
}

void AssetParser::SaveTextures(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const std::vector<Image*>& imageArray = engine->GetResourceManager()->GetResourceContainer()->GetImageArray();

	for (Image* image : imageArray)
	{
		if (!image)
		{
			continue;
		}

		std::string fullPath = image->GetPath();

#ifdef ENGINE_CONTENT_DIR
		if (fullPath.find(EngineContentDir) != std::string::npos)
		{
			continue;
		}
#endif

		const std::string path = ContentPathUtils::ToContentRelativePath(fullPath);
		if (path.empty())
		{
			continue;
		}

		tinyxml2::XMLElement* textureElement = xmlDocument.NewElement("Texture");
		if (!image->GetName().empty())
		{
			textureElement->SetAttribute("Name", image->GetName().c_str());
		}

		textureElement->SetAttribute("Usage", TextureUsageToStringInternal(image->GetTextureUsage()));
		bool uploadToGPU = image->GetUploadToGPU();
		if (image->GetGeneratedTexture())
		{
			uploadToGPU = image->GetGeneratedTexture()->GetUploadToGPU();
		}
		textureElement->SetAttribute("UploadToGPU", uploadToGPU ? "true" : "false");

		if (image->GetCanUseTextureAtlas() && uploadToGPU)
		{
			textureElement->SetAttribute("UseTextureAtlas", "true");
		}

		tinyxml2::XMLElement* pathElement = xmlDocument.NewElement("Path");
		pathElement->SetText(path.c_str());
		textureElement->InsertEndChild(pathElement);

		parentElement->InsertEndChild(textureElement);
	}
}

void AssetParser::SaveMaterials(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{

}

void AssetParser::SaveAudio(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{

}
