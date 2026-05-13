#ifndef __TEXTUREATLASETYPES_H__
#define __TEXTUREATLASETYPES_H__

#include "Goknar/Core.h"

enum class GOKNAR_API TextureAtlasCategory : unsigned char
{
	Opaque = 0,
	Transparent
};

inline const char* TextureAtlasCategoryToString(TextureAtlasCategory category)
{
	switch (category)
	{
	case TextureAtlasCategory::Opaque:
		return "Opaque";
	case TextureAtlasCategory::Transparent:
		return "Transparent";
	default:
		return "Unknown";
	}
}

#endif
