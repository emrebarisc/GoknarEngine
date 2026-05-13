#ifndef __TEXTUREATLASMANAGER_H__
#define __TEXTUREATLASMANAGER_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/TextureAtlas.h"

#include <memory>
#include <vector>

class Image;

class GOKNAR_API TextureAtlasManager
{
public:
	TextureAtlasManager() = default;
	TextureAtlasManager(const TextureAtlasManager&) = delete;
	TextureAtlasManager& operator=(const TextureAtlasManager&) = delete;
	~TextureAtlasManager() = default;

	bool AddImage(Image* image, TextureAtlasCategory category);

	void PreInit();
	void Init();
	void PostInit();

	bool HasImages() const;

	TextureAtlas* GetAtlas(int atlasIndex) const;
	TextureAtlas* GetFirstAtlas(TextureAtlasCategory category) const;

	const std::vector<std::unique_ptr<TextureAtlas>>& GetAtlases() const
	{
		return atlases_;
	}

private:
	std::unique_ptr<TextureAtlas> CreateAtlas(TextureAtlasCategory category);

	std::vector<std::unique_ptr<TextureAtlas>> atlases_;
	int nextAtlasIndex_{ 0 };
};

#endif
