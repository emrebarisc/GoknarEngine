#include "pch.h"

#include "TextureAtlasManager.h"

#include "Goknar/Contents/Image.h"
#include "Goknar/Log.h"

#include <string>

namespace
{
	std::string MakeAtlasName(TextureAtlasCategory category, int atlasIndex)
	{
		return std::string(TextureAtlasCategoryToString(category)) + "TextureAtlas_" + std::to_string(atlasIndex);
	}
}

bool TextureAtlasManager::AddImage(Image* image, TextureAtlasCategory category)
{
	if (!image)
	{
		return false;
	}

	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas && atlas->GetCategory() == category && atlas->ContainsImage(image))
		{
			return true;
		}
	}

	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas && atlas->GetCategory() == category && atlas->CanAddImage(image))
		{
			return atlas->AddImage(image);
		}
	}

	std::unique_ptr<TextureAtlas> newAtlas = CreateAtlas(category);
	if (!newAtlas->CanAddImage(image))
	{
		GOKNAR_CORE_WARN(
			"Image %s could not fit in a %s texture atlas and will use a standalone texture.",
			image->GetPath(),
			TextureAtlasCategoryToString(category));
		return false;
	}

	TextureAtlas* atlas = newAtlas.get();
	atlases_.push_back(std::move(newAtlas));
	return atlas->AddImage(image);
}

void TextureAtlasManager::PreInit()
{
	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas && atlas->HasImages())
		{
			atlas->PreInit();
		}
	}
}

void TextureAtlasManager::Init()
{
	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas)
		{
			atlas->Init();
		}
	}
}

void TextureAtlasManager::PostInit()
{
	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas)
		{
			atlas->PostInit();
		}
	}
}

bool TextureAtlasManager::HasImages() const
{
	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas && atlas->HasImages())
		{
			return true;
		}
	}

	return false;
}

TextureAtlas* TextureAtlasManager::GetAtlas(int atlasIndex) const
{
	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas && atlas->GetAtlasIndex() == atlasIndex)
		{
			return atlas.get();
		}
	}

	return nullptr;
}

TextureAtlas* TextureAtlasManager::GetFirstAtlas(TextureAtlasCategory category) const
{
	for (const std::unique_ptr<TextureAtlas>& atlas : atlases_)
	{
		if (atlas && atlas->GetCategory() == category)
		{
			return atlas.get();
		}
	}

	return nullptr;
}

std::unique_ptr<TextureAtlas> TextureAtlasManager::CreateAtlas(TextureAtlasCategory category)
{
	const int atlasIndex = nextAtlasIndex_++;
	std::unique_ptr<TextureAtlas> atlas(new TextureAtlas(MakeAtlasName(category, atlasIndex)));
	atlas->SetCategory(category);
	atlas->SetAtlasIndex(atlasIndex);
	return atlas;
}
