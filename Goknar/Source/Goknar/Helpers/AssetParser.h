#ifndef __ASSETPARSER_H__
#define __ASSETPARSER_H__

#include "Goknar/Core.h"
#include <string>
#include <vector>

#include "TinyXML/include/tinyxml2.h"
#include "Goknar/Renderer/TextureAtlasTypes.h"

struct Vector3;
class Material;
class Texture;

class GOKNAR_API AssetParser
{
public:
	static void ParseAssets(const std::string& filePath);
	static void SaveAssets(const std::string& filePath);
	static std::vector<std::string> GetMeshMaterialPaths(const std::string& meshPath, const std::string& assetContainerPath = "AssetContainer");
	static std::string GetMeshMaterialPath(const std::string& meshPath, const std::string& assetContainerPath = "AssetContainer");
	static void SetMeshMaterialPaths(const std::string& meshPath, const std::vector<std::string>& materialPaths, const std::string& assetContainerPath = "AssetContainer");
	static void SetMeshMaterialPath(const std::string& meshPath, const std::string& materialPath, const std::string& assetContainerPath = "AssetContainer");

	static bool ReadTextureAtlasUsage(const tinyxml2::XMLElement* textureElement, bool defaultValue = true);
	static bool RegisterTextureToTextureAtlas(
		Texture* texture,
		bool useTextureNameForImage = false,
		bool flushAtlas = false,
		TextureAtlasCategory category = TextureAtlasCategory::Opaque);
	static void RegisterMaterialTexturesToTextureAtlas(Material* material, bool flushAtlas = true);

private:
	static void ParseMeshes(tinyxml2::XMLElement* assetsElement);
	static void ParseTextures(tinyxml2::XMLElement* assetsElement);
	static void ParseMaterials(tinyxml2::XMLElement* assetsElement);
	static void ParseAudio(tinyxml2::XMLElement* assetsElement);

	static void SaveMeshes(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void SaveTextures(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void SaveMaterials(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
	static void SaveAudio(tinyxml2::XMLDocument& xmlDocument, tinyxml2::XMLElement* parentElement);
    
	static std::string Serialize(const Vector3& vector);
};

#endif
