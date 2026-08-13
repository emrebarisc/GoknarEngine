#ifndef __SHADERTEXTURESLOTS_H__
#define __SHADERTEXTURESLOTS_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Renderer/Types.h"

#include <string>

namespace ShaderTextureSlots
{
	inline constexpr GEuint BASE_COLOR_TEXTURE_UNIT = 0u;
	inline constexpr GEuint NORMAL_TEXTURE_UNIT = 1u;
	inline constexpr GEuint ORM_TEXTURE_UNIT = 2u;
	inline constexpr GEuint EMISSIVE_TEXTURE_UNIT = 3u;
	inline constexpr GEuint AMBIENT_OCCLUSION_TEXTURE_UNIT = 4u;
	inline constexpr GEuint METALLIC_TEXTURE_UNIT = 5u;
	inline constexpr GEuint ROUGHNESS_TEXTURE_UNIT = 6u;
	inline constexpr GEuint FIRST_GENERIC_TEXTURE_UNIT = 8u;

	GOKNAR_API bool IsSemanticTextureUsage(TextureUsage textureUsage);
	GOKNAR_API int GetFixedTextureUnit(TextureUsage textureUsage);
	GOKNAR_API const char* GetSemanticTextureUniformName(TextureUsage textureUsage);
	GOKNAR_API std::string GetSemanticAtlasUVTransformUniformName(TextureUsage textureUsage);
}

#endif
