#include "pch.h"
#include "AssetParser.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <map>

#include "Goknar/Engine.h"
#include "Goknar/Contents/Audio.h"
#include "Goknar/Contents/Image.h"
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
		std::string normalizedAbsolutePath = NormalizePath(absolutePath.generic_string());
		std::string normalizedContentDir = NormalizePath(ContentDir);
		if (!normalizedContentDir.empty() && normalizedContentDir.back() != '/')
		{
			normalizedContentDir += '/';
		}

		if (normalizedAbsolutePath.rfind(normalizedContentDir, 0) == 0)
		{
			return normalizedAbsolutePath.substr(normalizedContentDir.size());
		}

		return normalizedAbsolutePath;
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
	std::string fullPath = ContentDir + filePath;

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

			mesh = resourceManager->GetContent<MeshUnit>(path);
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
	std::stringstream stream;
	tinyxml2::XMLElement* element = assetsElement->FirstChildElement("Texture");
	ResourceManager* resourceManager = engine->GetResourceManager();

	while (element)
	{
		tinyxml2::XMLElement* child = element->FirstChildElement("Path");
		if (child)
		{
			std::string path;
			stream << child->GetText() << std::endl;
			stream >> path;
			
			//resourceManager->GetContent<Texture>(path);
			stream.clear();
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

			resourceManager->GetContent<Audio>(path);

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

	const std::string fullPath = ContentDir + filePath;
	const std::map<std::string, std::string> meshMaterialPaths = LoadExistingMeshMaterialPaths(fullPath);
	const std::filesystem::path contentRoot = std::filesystem::path(ContentDir);

	auto addPathAsset = [&](const char* elementName, const std::string& assetPath, const std::string* materialPath = nullptr)
	{
		tinyxml2::XMLElement* assetElement = assetXML.NewElement(elementName);
		tinyxml2::XMLElement* pathElement = assetXML.NewElement("Path");
		pathElement->SetText(assetPath.c_str());
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
			if (relativeAssetPath == filePath)
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
				addPathAsset("Texture", relativeAssetPath);
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
	const std::vector<MeshUnit*>& meshArray = engine->GetResourceManager()->GetResourceContainer()->GetMeshArray();

	for (MeshUnit* mesh : meshArray)
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
		
		std::string path = fullPath.substr(ContentDir.size());
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
