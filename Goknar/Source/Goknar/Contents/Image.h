#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Core.h"
#include "Contents/Content.h"
#include "Renderer/Texture.h"
#include "Renderer/TextureAtlas.h"

#include <vector>

class GOKNAR_API Image : public Content
{
public:
	Image();
	Image(const std::string& path);
	Image(const std::string& path, int width, int height, int channels, unsigned char* buffer);
	virtual ~Image();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	const unsigned char* GetBuffer() const
	{
		return buffer_;
	}

	void SetTextureUsage(TextureUsage textureUsage)
	{
		textureUsage_ = textureUsage;
	}

	TextureUsage GetTextureUsage() const
	{
		return textureUsage_;
	}

	void SetTextureWrappingR(TextureWrapping textureWrappingR)
	{
		textureWrappingR_ = textureWrappingR;
	}

	TextureWrapping GetTextureWrappingR() const
	{
		return textureWrappingR_;
	}

	void SetTextureWrappingT(TextureWrapping textureWrappingT)
	{
		textureWrappingT_ = textureWrappingT;
	}

	TextureWrapping GetTextureWrappingT() const
	{
		return textureWrappingT_;
	}

	void SetTextureWrappingS(TextureWrapping textureWrappingS)
	{
		textureWrappingS_ = textureWrappingS;
	}

	TextureWrapping GetTextureWrappingS() const
	{
		return textureWrappingS_;
	}

	Texture* GetGeneratedTexture() const
	{
		return generatedTexture_;
	}

	Texture* GetOrCreateGeneratedTexture();
	void RegisterTextureAtlasProxy(Texture* texture, TextureAtlasCategory category = TextureAtlasCategory::Opaque);
	void UnregisterTextureAtlasProxy(Texture* texture);

	void SetTextureAtlasRegion(
		TextureAtlas* textureAtlas,
		Texture* atlasTexture,
		const TextureAtlasRegion& atlasRegion,
		TextureAtlasCategory category = TextureAtlasCategory::Opaque,
		int atlasIndex = -1);

	TextureAtlas* GetTextureAtlas() const
	{
		return textureAtlas_;
	}

	TextureAtlas* GetTextureAtlas(TextureAtlasCategory category) const;

	const TextureAtlasRegion* GetTextureAtlasRegion() const
	{
		return hasAtlasRegion_ ? &atlasRegion_ : nullptr;
	}

	bool GetIsInTextureAtlas() const
	{
		return hasAtlasRegion_ && textureAtlas_ && generatedTexture_;
	}

	void SetCanUseTextureAtlas(bool canUseTextureAtlas)
	{
		canUseTextureAtlas_ = canUseTextureAtlas;
	}

	bool GetCanUseTextureAtlas() const
	{
		return canUseTextureAtlas_;
	}

	void AddTextureAtlasCategory(TextureAtlasCategory category);
	bool HasTextureAtlasCategories() const
	{
		return !textureAtlasCategories_.empty();
	}

	const std::vector<TextureAtlasCategory>& GetTextureAtlasCategories() const
	{
		return textureAtlasCategories_;
	}

	void ClearBuffer()
	{
		delete[] buffer_;
		buffer_ = nullptr;
	}

	int GetWidth() const
	{
		return width_;
	}

	int GetHeight() const
	{
		return height_;
	}

	int GetChannels() const
	{
		return channels_;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}

private:
	void ApplyTextureAtlasRegionToTexture(Texture* texture, Texture* atlasTexture, const TextureAtlasRegion& atlasRegion, bool applyImageTextureProperties);

	struct TextureAtlasProxy
	{
		Texture* texture{ nullptr };
		TextureAtlasCategory category{ TextureAtlasCategory::Opaque };
	};

	struct TextureAtlasBinding
	{
		TextureAtlas* textureAtlas{ nullptr };
		Texture* atlasTexture{ nullptr };
		TextureAtlasRegion atlasRegion;
		int atlasIndex{ -1 };
		bool hasAtlasRegion{ false };
	};

	const TextureAtlasBinding* FindTextureAtlasBinding(TextureAtlasCategory category) const;
	TextureAtlasBinding* FindMutableTextureAtlasBinding(TextureAtlasCategory category);

	Texture* generatedTexture_;
	std::vector<TextureAtlasProxy> atlasProxyTextures_;
	std::vector<TextureAtlasCategory> textureAtlasCategories_;
	std::vector<TextureAtlasBinding> textureAtlasBindings_;
	TextureAtlas* textureAtlas_{ nullptr };
	TextureAtlasRegion atlasRegion_;
	bool hasAtlasRegion_{ false };
	bool canUseTextureAtlas_{ false };

	unsigned char* buffer_;

	std::string name_;

	int width_;
	int height_;
	int channels_;

	TextureUsage textureUsage_{ TextureUsage::Diffuse };
	TextureWrapping textureWrappingR_{ TextureWrapping::REPEAT };
	TextureWrapping textureWrappingT_{ TextureWrapping::REPEAT };
	TextureWrapping textureWrappingS_{ TextureWrapping::REPEAT };
};

#endif
