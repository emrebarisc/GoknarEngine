#include "pch.h"
#include "AssetParser.h"

#include <sstream>
#include <stdexcept>
#include <iostream>

#include "Goknar/Engine.h"
#include "Goknar/Contents/Audio.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Materials/MaterialBase.h"


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
		tinyxml2::XMLElement* child = element->FirstChildElement("Path");
		if (child)
		{
			std::string path;
			stream << child->GetText() << std::endl;
			stream >> path;

			resourceManager->GetContent<MeshUnit>(path);
			stream.clear();
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
	
	SaveMeshes(assetXML, assetsElement);
	SaveTextures(assetXML, assetsElement);
	SaveMaterials(assetXML, assetsElement);
	SaveAudio(assetXML, assetsElement);

	rootElement->InsertEndChild(assetsElement);

	std::string fullPath = ContentDir + filePath;
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