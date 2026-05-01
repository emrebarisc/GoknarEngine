#include "pch.h"
#include "AssetParser.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <map>
#include <set>

#include "Goknar/Engine.h"
#include "Goknar/Contents/Audio.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Materials/MaterialSerializer.h"

namespace
{
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

	std::map<std::string, std::string> LoadExistingMeshMaterialPaths(const std::string& assetContainerPath)
	{
		std::map<std::string, std::string> materialPathByMeshPath;

		tinyxml2::XMLDocument existingDocument;
		if (existingDocument.LoadFile(assetContainerPath.c_str()) != tinyxml2::XML_SUCCESS)
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
			tinyxml2::XMLElement* materialPathElement = meshElement->FirstChildElement("MaterialPath");
			if (!pathElement || !pathElement->GetText() || !materialPathElement || !materialPathElement->GetText())
			{
				continue;
			}

			materialPathByMeshPath[pathElement->GetText()] = materialPathElement->GetText();
		}

		return materialPathByMeshPath;
	}

	std::map<std::string, std::string> LoadExistingTextureNames(const std::string& assetContainerPath)
	{
		std::map<std::string, std::string> textureNameByTexturePath;

		tinyxml2::XMLDocument existingDocument;
		if (existingDocument.LoadFile(assetContainerPath.c_str()) != tinyxml2::XML_SUCCESS)
		{
			return textureNameByTexturePath;
		}

		tinyxml2::XMLElement* rootElement = existingDocument.FirstChildElement("AssetContainer");
		tinyxml2::XMLElement* assetsElement = rootElement ? rootElement->FirstChildElement("Assets") : nullptr;
		for (tinyxml2::XMLElement* textureElement = assetsElement ? assetsElement->FirstChildElement("Texture") : nullptr;
			textureElement != nullptr;
			textureElement = textureElement->NextSiblingElement("Texture"))
		{
			const std::string texturePath = GetElementText(textureElement->FirstChildElement("Path"));
			if (texturePath.empty())
			{
				continue;
			}

			const char* textureNameAttribute = textureElement->Attribute("Name");
			textureNameByTexturePath[texturePath] = textureNameAttribute ? TrimString(textureNameAttribute) : GetElementText(textureElement->FirstChildElement("Name"));
		}

		return textureNameByTexturePath;
	}

	std::string TryGetGameAssetFileType(const std::filesystem::path& absolutePath)
	{
		tinyxml2::XMLDocument document;
		if (document.LoadFile(absolutePath.generic_string().c_str()) != tinyxml2::XML_SUCCESS)
		{
			return "";
		}

		tinyxml2::XMLElement* root = document.FirstChildElement("GameAsset");
		const char* fileType = root ? root->Attribute("FileType") : nullptr;
		return fileType ? fileType : "";
	}
}


void AssetParser::ParseAssets(const std::string& filePath)
{
	const std::string fullPath = ContentPathUtils::ToAbsoluteContentPath(filePath);

	tinyxml2::XMLDocument xmlFile;
	tinyxml2::XMLError res;

	try
	{
		res = xmlFile.LoadFile(fullPath.c_str());
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

std::string AssetParser::GetMeshMaterialPath(const std::string& meshPath, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const std::map<std::string, std::string> meshMaterialPaths = LoadExistingMeshMaterialPaths(fullAssetContainerPath);
	const std::string relativeMeshPath = ContentPathUtils::ToContentRelativePath(meshPath);

	const auto meshMaterialPathIterator = meshMaterialPaths.find(relativeMeshPath);
	return meshMaterialPathIterator != meshMaterialPaths.end() ? meshMaterialPathIterator->second : "";
}

void AssetParser::SetMeshMaterialPath(const std::string& meshPath, const std::string& materialPath, const std::string& assetContainerPath)
{
	const std::string relativeAssetContainerPath = ContentPathUtils::ToContentRelativePath(assetContainerPath);
	const std::string fullAssetContainerPath = ContentPathUtils::ToAbsoluteContentPath(relativeAssetContainerPath);
	const std::string relativeMeshPath = ContentPathUtils::ToContentRelativePath(meshPath);
	const std::string relativeMaterialPath = ContentPathUtils::ToContentRelativePath(materialPath);

	if (relativeMeshPath.empty())
	{
		return;
	}

	tinyxml2::XMLDocument assetContainerDocument;
	const tinyxml2::XMLError loadResult = assetContainerDocument.LoadFile(fullAssetContainerPath.c_str());
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

	tinyxml2::XMLElement* materialPathElement = meshElement->FirstChildElement("MaterialPath");
	if (relativeMaterialPath.empty())
	{
		if (materialPathElement)
		{
			meshElement->DeleteChild(materialPathElement);
		}
	}
	else
	{
		if (!materialPathElement)
		{
			materialPathElement = assetContainerDocument.NewElement("MaterialPath");
			meshElement->InsertEndChild(materialPathElement);
		}

		materialPathElement->SetText(relativeMaterialPath.c_str());
	}

	assetContainerDocument.SaveFile(fullAssetContainerPath.c_str());
}

void AssetParser::ParseMeshes(tinyxml2::XMLElement* assetsElement)
{
	std::stringstream stream;
	tinyxml2::XMLElement* element = assetsElement->FirstChildElement("Mesh");
	ResourceManager* resourceManager = engine->GetResourceManager();
	
	while (element)
	{
		MeshUnit* mesh = nullptr;

		tinyxml2::XMLElement* child = element->FirstChildElement("Path");
		if (child)
		{
			std::string path;
			stream << child->GetText() << std::endl;
			stream >> path;

			mesh = resourceManager->GetContent<MeshUnit>(ContentPathUtils::ToContentRelativePath(path));
			stream.clear();
		}

		if (mesh)
		{
			child = element->FirstChildElement("MaterialPath");
			if (child)
			{
				std::string materialPath;
				stream << child->GetText() << std::endl;
				stream >> materialPath;
				materialPath = ContentPathUtils::ToContentRelativePath(materialPath);

				if (!materialPath.empty())
				{
					MaterialSerializer::Deserialize(materialPath, mesh->GetMaterial());
				}

				stream.clear();
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
			if (image && image->GetName().empty())
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

		element = element->NextSiblingElement("Texture");
	}
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
	const std::map<std::string, std::string> meshMaterialPaths = LoadExistingMeshMaterialPaths(fullPath);
	const std::map<std::string, std::string> existingTextureNames = LoadExistingTextureNames(fullPath);
	const std::filesystem::path contentRoot = std::filesystem::path(ContentDir);
	std::set<std::string> usedTextureNames;

	auto addPathAsset = [&](const char* elementName, const std::string& assetPath, const std::string* materialPath = nullptr, const std::string* assetName = nullptr)
	{
		tinyxml2::XMLElement* assetElement = assetXML.NewElement(elementName);
		tinyxml2::XMLElement* pathElement = assetXML.NewElement("Path");
		pathElement->SetText(assetPath.c_str());

		if (assetName && !assetName->empty())
		{
			assetElement->SetAttribute("Name", assetName->c_str());
		}

		assetElement->InsertEndChild(pathElement);

		if (materialPath && !materialPath->empty())
		{
			tinyxml2::XMLElement* materialPathElement = assetXML.NewElement("MaterialPath");
			materialPathElement->SetText(materialPath->c_str());
			assetElement->InsertEndChild(materialPathElement);
		}

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
				const std::string* materialPath = existingMaterialPathIterator != meshMaterialPaths.end() ? &existingMaterialPathIterator->second : nullptr;
				addPathAsset("Mesh", relativeAssetPath, materialPath);
				continue;
			}

			if (extension == ".png" || extension == ".jpg")
			{
				const auto existingTextureNameIterator = existingTextureNames.find(relativeAssetPath);
				const std::string preferredTextureName =
					existingTextureNameIterator != existingTextureNames.end() && !existingTextureNameIterator->second.empty() ?
					existingTextureNameIterator->second :
					GetTextureNameFromPath(relativeAssetPath);
				const std::string resolvedTextureName = GetUniqueTextureName(preferredTextureName, usedTextureNames);
				addPathAsset("Texture", relativeAssetPath, nullptr, &resolvedTextureName);
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
		}
	}

	rootElement->InsertEndChild(assetsElement);
	assetXML.SaveFile(fullPath.c_str());
}

void AssetParser::SaveMeshes(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{
	const std::vector<Mesh<MeshUnit>*>& meshArray = engine->GetResourceManager()->GetResourceContainer()->GetMeshArray();

	for (Mesh<MeshUnit>* mesh : meshArray)
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

}

void AssetParser::SaveMaterials(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{

}

void AssetParser::SaveAudio(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement)
{

}
