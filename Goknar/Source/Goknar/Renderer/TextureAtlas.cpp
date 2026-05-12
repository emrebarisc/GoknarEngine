#include "pch.h"

#include "TextureAtlas.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "Goknar/Application.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Scene.h"

TextureAtlas::TextureAtlas() :
	TextureAtlas("textureAtlas")
{
}

TextureAtlas::TextureAtlas(const std::string& name, int maxWidth, int maxHeight, int padding) :
	name_(name),
	maxWidth_(maxWidth),
	maxHeight_(maxHeight),
	padding_(padding < 0 ? 0 : padding)
{
}

TextureAtlas::~TextureAtlas()
{
	delete atlasTexture_;
}

bool TextureAtlas::AddImage(Image* image)
{
	if (!image)
	{
		return false;
	}

	if (std::find(images_.begin(), images_.end(), image) != images_.end())
	{
		return true;
	}

	images_.push_back(image);

	// Scene/material textures may be discovered after ResourceContainer::PreInit().
	// Do not reject those late registrations; rebuild the atlas before the next
	// PreInit/flush so they become atlas proxies instead of standalone GPU textures.
	if (isBuilt_)
	{
		isBuilt_ = false;
		needsRebuild_ = true;
	}

	return true;
}

bool TextureAtlas::Build()
{
	if (isBuilt_ && !needsRebuild_)
	{
		return atlasTexture_ != nullptr;
	}

	std::vector<PackingItem> packableItems;
	packableItems.reserve(images_.size());

	long long totalArea = 0;
	int widestPaddedImage = 0;

	for (Image* image : images_)
	{
		if (!image || !image->GetBuffer() || image->GetWidth() <= 0 || image->GetHeight() <= 0)
		{
			continue;
		}

		if (image->GetChannels() < 1 || image->GetChannels() > 4)
		{
			GOKNAR_CORE_WARN("Image {0} has unsupported channel count {1} and will not be packed into an atlas.", image->GetPath(), image->GetChannels());
			continue;
		}

		PackingItem item;
		item.image = image;
		item.width = image->GetWidth();
		item.height = image->GetHeight();
		item.paddedWidth = item.width + padding_ * 2;
		item.paddedHeight = item.height + padding_ * 2;

		if (item.paddedWidth > maxWidth_ || item.paddedHeight > maxHeight_)
		{
			GOKNAR_CORE_WARN("Image {0} is too large for TextureAtlas max size and will use a standalone texture.", image->GetPath());
			continue;
		}

		packableItems.push_back(item);
		totalArea += static_cast<long long>(item.paddedWidth) * static_cast<long long>(item.paddedHeight);
		widestPaddedImage = (std::max)(widestPaddedImage, item.paddedWidth);
	}

	if (packableItems.empty())
	{
		isBuilt_ = true;
		needsRebuild_ = false;
		return false;
	}

	std::sort(
		packableItems.begin(),
		packableItems.end(),
		[](const PackingItem& left, const PackingItem& right)
		{
			if (left.paddedHeight == right.paddedHeight)
			{
				return left.paddedWidth > right.paddedWidth;
			}

			return left.paddedHeight > right.paddedHeight;
		});

	const int estimatedSquareSize = NextPowerOfTwo(static_cast<int>(std::ceil(std::sqrt(static_cast<double>(totalArea)))));
	int candidateWidth = NextPowerOfTwo((std::max)(widestPaddedImage, estimatedSquareSize));
	candidateWidth = (std::max)(1, (std::min)(candidateWidth, maxWidth_));

	std::vector<PackingPlacement> placements;
	int packedHeight = 0;
	bool packed = false;

	while (candidateWidth <= maxWidth_)
	{
		placements.clear();
		packedHeight = 0;

		if (TryPack(packableItems, candidateWidth, maxHeight_, placements, packedHeight))
		{
			width_ = candidateWidth;
			height_ = NextPowerOfTwo(packedHeight);
			if (height_ <= maxHeight_)
			{
				packed = true;
				break;
			}
		}

		if (candidateWidth == maxWidth_)
		{
			break;
		}

		candidateWidth = (std::min)(candidateWidth * 2, maxWidth_);
	}

	if (!packed)
	{
		GOKNAR_CORE_WARN("TextureAtlas {0} could not pack {1} images. Images will use standalone textures.", name_, static_cast<int>(packableItems.size()));
		isBuilt_ = true;
		needsRebuild_ = false;
		return false;
	}

	unsigned char* atlasBuffer = new unsigned char[static_cast<size_t>(width_) * static_cast<size_t>(height_) * 4];
	std::memset(atlasBuffer, 0, static_cast<size_t>(width_) * static_cast<size_t>(height_) * 4);

	if (!atlasTexture_)
	{
		atlasTexture_ = new Texture();
	}

	atlasTexture_->SetName(name_);
	atlasTexture_->SetBuffer(atlasBuffer);
	atlasTexture_->SetSize(width_, height_);
	atlasTexture_->SetChannels(4);
	atlasTexture_->SetTextureFormat(TextureFormat::RGBA);
	atlasTexture_->SetTextureInternalFormat(TextureInternalFormat::RGBA);
	atlasTexture_->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
	atlasTexture_->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
	atlasTexture_->SetTextureWrappingR(TextureWrapping::CLAMP_TO_EDGE);
	atlasTexture_->SetTextureMinFilter(TextureMinFilter::LINEAR);
	atlasTexture_->SetTextureMagFilter(TextureMagFilter::LINEAR);
	atlasTexture_->SetGenerateMipmap(false);

	regions_.clear();

	for (const PackingPlacement& placement : placements)
	{
		TextureAtlasRegion region;
		region.x = placement.x + padding_;
		region.y = placement.y + padding_;
		region.width = placement.width;
		region.height = placement.height;
		region.uMin = static_cast<float>(region.x) / static_cast<float>(width_);
		region.vMin = static_cast<float>(region.y) / static_cast<float>(height_);
		region.uMax = static_cast<float>(region.x + region.width) / static_cast<float>(width_);
		region.vMax = static_cast<float>(region.y + region.height) / static_cast<float>(height_);

		CopyImageToAtlas(placement.image, region, atlasBuffer);

		regions_[placement.image] = region;
		placement.image->SetTextureAtlasRegion(this, atlasTexture_, region);
		// Keep the source buffer available so the atlas can be rebuilt if more
		// scene/material textures are registered later in the initialization phase.
	}

	if (atlasTexture_->GetOwnedRendererTextureId() != 0)
	{
		atlasTexture_->UpdateBufferOnGPU();
		atlasTexture_->ClearBuffer();
	}

	isBuilt_ = true;
	needsRebuild_ = false;
	return true;
}

void TextureAtlas::PreInit()
{
	if ((needsRebuild_ || !isBuilt_) && !Build())
	{
		return;
	}

	if (!atlasTexture_)
	{
		return;
	}

	if (atlasTexture_->GetOwnedRendererTextureId() == 0 && !atlasTexture_->GetIsInitialized())
	{
		atlasTexture_->PreInit();
	}

	isPreInitialized_ = true;
}

void TextureAtlas::Init()
{
	if (isInitialized_ || !atlasTexture_)
	{
		return;
	}

	atlasTexture_->Init();
	isInitialized_ = true;
}

void TextureAtlas::PostInit()
{
	if (isPostInitialized_ || !atlasTexture_)
	{
		return;
	}

	atlasTexture_->PostInit();
	isPostInitialized_ = true;
}

const TextureAtlasRegion* TextureAtlas::GetRegion(const Image* image) const
{
	auto regionIterator = regions_.find(image);
	if (regionIterator == regions_.end())
	{
		return nullptr;
	}

	return &regionIterator->second;
}

bool TextureAtlas::TryPack(const std::vector<PackingItem>& items, int atlasWidth, int maxAtlasHeight, std::vector<PackingPlacement>& placements, int& packedHeight) const
{
	int shelfX = 0;
	int shelfY = 0;
	int shelfHeight = 0;

	for (const PackingItem& item : items)
	{
		if (item.paddedWidth > atlasWidth || item.paddedHeight > maxAtlasHeight)
		{
			return false;
		}

		if (shelfX + item.paddedWidth > atlasWidth)
		{
			shelfY += shelfHeight;
			shelfX = 0;
			shelfHeight = 0;
		}

		if (shelfY + item.paddedHeight > maxAtlasHeight)
		{
			return false;
		}

		PackingPlacement placement;
		placement.image = item.image;
		placement.x = shelfX;
		placement.y = shelfY;
		placement.width = item.width;
		placement.height = item.height;

		placements.push_back(placement);

		shelfX += item.paddedWidth;
		shelfHeight = (std::max)(shelfHeight, item.paddedHeight);
	}

	packedHeight = shelfY + shelfHeight;
	return true;
}

void TextureAtlas::CopyImageToAtlas(const Image* image, const TextureAtlasRegion& region, unsigned char* atlasBuffer) const
{
	const unsigned char* source = image->GetBuffer();
	const int sourceChannels = image->GetChannels();
	const int sourceWidth = image->GetWidth();
	const int sourceHeight = image->GetHeight();

	for (int y = -padding_; y < sourceHeight + padding_; ++y)
	{
		const int sourceY = (std::max)(0, (std::min)(y, sourceHeight - 1));
		const int destinationY = region.y + y;

		if (destinationY < 0 || destinationY >= height_)
		{
			continue;
		}

		for (int x = -padding_; x < sourceWidth + padding_; ++x)
		{
			const int sourceX = (std::max)(0, (std::min)(x, sourceWidth - 1));
			const int destinationX = region.x + x;

			if (destinationX < 0 || destinationX >= width_)
			{
				continue;
			}

			const unsigned char* sourcePixel = source + (static_cast<size_t>(sourceY) * static_cast<size_t>(sourceWidth) + static_cast<size_t>(sourceX)) * static_cast<size_t>(sourceChannels);
			unsigned char* destinationPixel = atlasBuffer + (static_cast<size_t>(destinationY) * static_cast<size_t>(width_) + static_cast<size_t>(destinationX)) * 4;

			switch (sourceChannels)
			{
			case 1:
				destinationPixel[0] = sourcePixel[0];
				destinationPixel[1] = sourcePixel[0];
				destinationPixel[2] = sourcePixel[0];
				destinationPixel[3] = 255;
				break;
			case 2:
				destinationPixel[0] = sourcePixel[0];
				destinationPixel[1] = sourcePixel[0];
				destinationPixel[2] = sourcePixel[0];
				destinationPixel[3] = sourcePixel[1];
				break;
			case 3:
				destinationPixel[0] = sourcePixel[0];
				destinationPixel[1] = sourcePixel[1];
				destinationPixel[2] = sourcePixel[2];
				destinationPixel[3] = 255;
				break;
			case 4:
				destinationPixel[0] = sourcePixel[0];
				destinationPixel[1] = sourcePixel[1];
				destinationPixel[2] = sourcePixel[2];
				destinationPixel[3] = sourcePixel[3];
				break;
			default:
				break;
			}
		}
	}
}

int TextureAtlas::NextPowerOfTwo(int value)
{
	if (value <= 1)
	{
		return 1;
	}

	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	return value + 1;
}
