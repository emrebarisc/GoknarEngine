#include "pch.h"

#include "Contents/Image.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Renderer/Texture.h"
#include "Renderer/TextureAtlas.h"

#include <algorithm>

Image::Image() :
	Content(),
	width_(0),
	height_(0),
	channels_(0),
	textureUsage_(TextureUsage::None),
	buffer_(nullptr),
	generatedTexture_(nullptr)
{

}

Image::Image(const std::string& path) :
	Content(path),
	width_(0),
	height_(0),
	channels_(0),
	textureUsage_(TextureUsage::Diffuse),
	buffer_(nullptr),
	generatedTexture_(nullptr)
{
}

Image::Image(const std::string& path, int width, int height, int channels, unsigned char* buffer) :
	Content(path),
	width_(width),
	height_(height),
	channels_(channels),
	textureUsage_(TextureUsage::Diffuse),
	buffer_(buffer),
	generatedTexture_(nullptr)
{
}


Image::~Image()
{
	delete generatedTexture_;

	delete[] buffer_;
	buffer_ = nullptr;
}

Texture* Image::GetOrCreateGeneratedTexture()
{
	if (!generatedTexture_)
	{
		generatedTexture_ = hasAtlasRegion_ ? new Texture() : new Texture(this);

		if (!name_.empty())
		{
			generatedTexture_->SetName(name_);
		}

		if (canUseTextureAtlas_ && !hasAtlasRegion_)
		{
			generatedTexture_->SetWaitsForTextureAtlas(true);
		}

		if (!textureAtlasCategories_.empty())
		{
			generatedTexture_->SetTextureAtlasCategory(textureAtlasCategories_.front());
		}
	}

	return generatedTexture_;
}

void Image::RegisterTextureAtlasProxy(Texture* texture, TextureAtlasCategory category)
{
	if (!texture)
	{
		return;
	}

	AddTextureAtlasCategory(category);

	auto proxyIterator = std::find_if(
		atlasProxyTextures_.begin(),
		atlasProxyTextures_.end(),
		[texture](const TextureAtlasProxy& proxy)
		{
			return proxy.texture == texture;
		});

	if (proxyIterator == atlasProxyTextures_.end())
	{
		atlasProxyTextures_.push_back({ texture, category });
	}
	else
	{
		proxyIterator->category = category;
	}

	texture->SetTextureAtlasProxySourceImage(this);
	texture->SetTextureAtlasCategory(category);
	texture->SetUploadToGPU(uploadToGPU_);

	if (!FindTextureAtlasBinding(category) && canUseTextureAtlas_)
	{
		texture->SetWaitsForTextureAtlas(true);
	}

	if (const TextureAtlasBinding* binding = FindTextureAtlasBinding(category))
	{
		ApplyTextureAtlasRegionToTexture(texture, binding->atlasTexture, binding->atlasRegion, false);
	}
}


void Image::UnregisterTextureAtlasProxy(Texture* texture)
{
	if (!texture)
	{
		return;
	}

	atlasProxyTextures_.erase(
		std::remove_if(
			atlasProxyTextures_.begin(),
			atlasProxyTextures_.end(),
			[texture](const TextureAtlasProxy& proxy)
			{
				return proxy.texture == texture;
			}),
		atlasProxyTextures_.end());

	if (texture->GetTextureAtlasProxySourceImage() == this)
	{
		texture->SetTextureAtlasProxySourceImage(nullptr);
	}
}

void Image::ApplyTextureAtlasRegionToTexture(Texture* texture, Texture* atlasTexture, const TextureAtlasRegion& atlasRegion, bool applyImageTextureProperties)
{
	if (!texture || !atlasTexture)
	{
		return;
	}

	if (applyImageTextureProperties)
	{
		if (!name_.empty())
		{
			texture->SetName(name_);
		}

		texture->SetTextureUsage(textureUsage_);
		texture->SetTextureWrappingR(textureWrappingR_);
		texture->SetTextureWrappingT(textureWrappingT_);
		texture->SetTextureWrappingS(textureWrappingS_);
	}

	texture->SetSize(width_, height_);
	texture->SetChannels(channels_);
	texture->SetTextureMinFilter(atlasTexture->GetTextureMinFilter());
	texture->SetTextureMagFilter(atlasTexture->GetTextureMagFilter());
	texture->SetAtlasTexture(
		atlasTexture,
		atlasRegion.uMin,
		atlasRegion.vMin,
		atlasRegion.uMax,
		atlasRegion.vMax,
		atlasRegion.category,
		atlasRegion.atlasIndex);
}

void Image::SetTextureAtlasRegion(
	TextureAtlas* textureAtlas,
	Texture* atlasTexture,
	const TextureAtlasRegion& atlasRegion,
	TextureAtlasCategory category,
	int atlasIndex)
{
	if (!textureAtlas || !atlasTexture)
	{
		return;
	}

	AddTextureAtlasCategory(category);

	TextureAtlasRegion resolvedRegion = atlasRegion;
	resolvedRegion.category = category;
	resolvedRegion.atlasIndex = atlasIndex;

	TextureAtlasBinding* binding = FindMutableTextureAtlasBinding(category);
	if (!binding)
	{
		textureAtlasBindings_.push_back(TextureAtlasBinding());
		binding = &textureAtlasBindings_.back();
	}

	binding->textureAtlas = textureAtlas;
	binding->atlasTexture = atlasTexture;
	binding->atlasRegion = resolvedRegion;
	binding->atlasIndex = atlasIndex;
	binding->hasAtlasRegion = true;

	textureAtlas_ = textureAtlas;
	atlasRegion_ = resolvedRegion;
	hasAtlasRegion_ = true;

	if (textureAtlasCategories_.empty() || textureAtlasCategories_.front() == category)
	{
		ApplyTextureAtlasRegionToTexture(GetOrCreateGeneratedTexture(), atlasTexture, resolvedRegion, true);
	}

	for (const TextureAtlasProxy& atlasProxyTexture : atlasProxyTextures_)
	{
		if (atlasProxyTexture.category == category)
		{
			ApplyTextureAtlasRegionToTexture(atlasProxyTexture.texture, atlasTexture, resolvedRegion, false);
		}
	}
}

TextureAtlas* Image::GetTextureAtlas(TextureAtlasCategory category) const
{
	if (const TextureAtlasBinding* binding = FindTextureAtlasBinding(category))
	{
		return binding->textureAtlas;
	}

	return nullptr;
}

void Image::AddTextureAtlasCategory(TextureAtlasCategory category)
{
	if (std::find(textureAtlasCategories_.begin(), textureAtlasCategories_.end(), category) == textureAtlasCategories_.end())
	{
		textureAtlasCategories_.push_back(category);
	}
}

const Image::TextureAtlasBinding* Image::FindTextureAtlasBinding(TextureAtlasCategory category) const
{
	for (const TextureAtlasBinding& binding : textureAtlasBindings_)
	{
		if (binding.hasAtlasRegion && binding.atlasRegion.category == category)
		{
			return &binding;
		}
	}

	return nullptr;
}

Image::TextureAtlasBinding* Image::FindMutableTextureAtlasBinding(TextureAtlasCategory category)
{
	for (TextureAtlasBinding& binding : textureAtlasBindings_)
	{
		if (binding.hasAtlasRegion && binding.atlasRegion.category == category)
		{
			return &binding;
		}
	}

	return nullptr;
}

void Image::PreInit()
{
	if (generatedTexture_)
	{
		if (!generatedTexture_->GetUsesAtlasTexture() &&
			!generatedTexture_->GetWaitsForTextureAtlas())
		{
			generatedTexture_->PreInit();
		}

		return;
	}

	generatedTexture_ = new Texture(this);
	
	if (!name_.empty())
	{
		generatedTexture_->SetName(name_);
	}

	generatedTexture_->PreInit();
}

void Image::Init()
{
	if (!generatedTexture_ || textureAtlas_)
	{
		return;
	}

	generatedTexture_->Init();
}

void Image::PostInit()
{
	if (!generatedTexture_ || textureAtlas_)
	{
		return;
	}

	generatedTexture_->PostInit();
}
