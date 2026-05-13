#ifndef __TEXTUREATLAS_H__
#define __TEXTUREATLAS_H__

#include "Goknar/Core.h"
#include "Texture.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

class Image;

struct GOKNAR_API TextureAtlasRegion
{
	int x{ 0 };
	int y{ 0 };
	int width{ 0 };
	int height{ 0 };

	float uMin{ 0.0f };
	float vMin{ 0.0f };
	float uMax{ 0.0f };
	float vMax{ 0.0f };

	TextureAtlasCategory category{ TextureAtlasCategory::Opaque };
	int atlasIndex{ -1 };
};

class GOKNAR_API TextureAtlas
{
public:
	TextureAtlas();
	TextureAtlas(const std::string& name, int maxWidth = 4096, int maxHeight = 4096, int padding = 2);
	~TextureAtlas();

	bool AddImage(Image* image);
	bool CanAddImage(Image* image) const;
	bool ContainsImage(const Image* image) const
	{
		return std::find(images_.begin(), images_.end(), image) != images_.end();
	}
	bool Build();

	bool HasImages() const
	{
		return !images_.empty();
	}

	bool GetIsBuilt() const
	{
		return isBuilt_;
	}

	bool GetIsPreInitialized() const
	{
		return isPreInitialized_;
	}

	void PreInit();
	void Init();
	void PostInit();

	Texture* GetTexture() const
	{
		return atlasTexture_;
	}

	const TextureAtlasRegion* GetRegion(const Image* image) const;

	const std::vector<Image*>& GetImages() const
	{
		return images_;
	}

	int GetWidth() const
	{
		return width_;
	}

	int GetHeight() const
	{
		return height_;
	}

	int GetPadding() const
	{
		return padding_;
	}

	TextureAtlasCategory GetCategory() const
	{
		return category_;
	}

	void SetCategory(TextureAtlasCategory category)
	{
		category_ = category;
	}

	int GetAtlasIndex() const
	{
		return atlasIndex_;
	}

	void SetAtlasIndex(int atlasIndex)
	{
		atlasIndex_ = atlasIndex;
	}

	int GetMaxWidth() const
	{
		return maxWidth_;
	}

	int GetMaxHeight() const
	{
		return maxHeight_;
	}

	void SetMaxSize(int maxWidth, int maxHeight)
	{
		maxWidth_ = maxWidth;
		maxHeight_ = maxHeight;
	}

	void SetPadding(int padding)
	{
		padding_ = padding < 0 ? 0 : padding;
	}

private:
	struct PackingItem
	{
		Image* image{ nullptr };
		int width{ 0 };
		int height{ 0 };
		int paddedWidth{ 0 };
		int paddedHeight{ 0 };
	};

	struct PackingPlacement
	{
		Image* image{ nullptr };
		int x{ 0 };
		int y{ 0 };
		int width{ 0 };
		int height{ 0 };
	};

	bool BuildPackingItems(const std::vector<Image*>& images, std::vector<PackingItem>& packableItems, long long& totalArea, int& widestPaddedImage, bool logWarnings) const;
	bool TryFindPlacements(const std::vector<PackingItem>& packableItems, int& atlasWidth, int& atlasHeight, std::vector<PackingPlacement>& placements) const;
	bool TryPack(const std::vector<PackingItem>& items, int atlasWidth, int maxAtlasHeight, std::vector<PackingPlacement>& placements, int& packedHeight) const;
	void CopyImageToAtlas(const Image* image, const TextureAtlasRegion& region, unsigned char* atlasBuffer) const;

	static int NextPowerOfTwo(int value);

	std::string name_{ "textureAtlas" };
	TextureAtlasCategory category_{ TextureAtlasCategory::Opaque };
	int atlasIndex_{ -1 };
	std::vector<Image*> images_;
	std::map<const Image*, TextureAtlasRegion> regions_;

	Texture* atlasTexture_{ nullptr };

	int width_{ 0 };
	int height_{ 0 };
	int maxWidth_{ 8192 };
	int maxHeight_{ 8192 };
	int padding_{ 2 };

	bool isBuilt_{ false };
	bool needsRebuild_{ false };
	bool isPreInitialized_{ false };
	bool isInitialized_{ false };
	bool isPostInitialized_{ false };
};

#endif
