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
	if (generatedTexture_ && !generatedTextureRegisteredToScene_)
	{
		delete generatedTexture_;
	}

	delete[] buffer_;
	buffer_ = nullptr;
}

Texture* Image::GetOrCreateGeneratedTexture()
{
	if (!generatedTexture_)
	{
		generatedTexture_ = hasAtlasRegion_ ? new Texture() : new Texture(this);
		generatedTextureRegisteredToScene_ = false;

		if (!name_.empty())
		{
			generatedTexture_->SetName(name_);
		}

		if (canUseTextureAtlas_ && !hasAtlasRegion_)
		{
			generatedTexture_->SetWaitsForTextureAtlas(true);
		}
	}

	return generatedTexture_;
}

void Image::RegisterTextureAtlasProxy(Texture* texture)
{
	if (!texture)
	{
		return;
	}

	if (std::find(atlasProxyTextures_.begin(), atlasProxyTextures_.end(), texture) == atlasProxyTextures_.end())
	{
		atlasProxyTextures_.push_back(texture);
	}

	if (!hasAtlasRegion_ && canUseTextureAtlas_)
	{
		texture->SetWaitsForTextureAtlas(true);
	}

	if (hasAtlasRegion_ && textureAtlas_)
	{
		ApplyTextureAtlasRegionToTexture(texture, textureAtlas_->GetTexture(), atlasRegion_, false);
	}
}


void Image::UnregisterTextureAtlasProxy(Texture* texture)
{
	if (!texture)
	{
		return;
	}

	atlasProxyTextures_.erase(
		std::remove(atlasProxyTextures_.begin(), atlasProxyTextures_.end(), texture),
		atlasProxyTextures_.end());
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
	texture->SetAtlasTexture(atlasTexture, atlasRegion.uMin, atlasRegion.vMin, atlasRegion.uMax, atlasRegion.vMax);
}

void Image::SetTextureAtlasRegion(TextureAtlas* textureAtlas, Texture* atlasTexture, const TextureAtlasRegion& atlasRegion)
{
	if (!textureAtlas || !atlasTexture)
	{
		return;
	}

	textureAtlas_ = textureAtlas;
	atlasRegion_ = atlasRegion;
	hasAtlasRegion_ = true;

	ApplyTextureAtlasRegionToTexture(GetOrCreateGeneratedTexture(), atlasTexture, atlasRegion, true);

	for (Texture* atlasProxyTexture : atlasProxyTextures_)
	{
		ApplyTextureAtlasRegionToTexture(atlasProxyTexture, atlasTexture, atlasRegion, false);
	}
}

void Image::PreInit()
{
	if (generatedTexture_)
	{
		if (!generatedTextureRegisteredToScene_ &&
			!generatedTexture_->GetUsesAtlasTexture() &&
			!generatedTexture_->GetWaitsForTextureAtlas())
		{
			generatedTexture_->PreInit();
			engine->GetApplication()->GetMainScene()->AddTexture(generatedTexture_);
			generatedTextureRegisteredToScene_ = true;
		}

		return;
	}

	generatedTexture_ = new Texture(this);
	
	if (!name_.empty())
	{
		generatedTexture_->SetName(name_);
	}

	generatedTexture_->PreInit();
	engine->GetApplication()->GetMainScene()->AddTexture(generatedTexture_);
	generatedTextureRegisteredToScene_ = true;
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
