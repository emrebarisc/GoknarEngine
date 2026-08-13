#include "pch.h"

#include "ShaderTextureSlots.h"

namespace ShaderTextureSlots
{
	bool IsSemanticTextureUsage(TextureUsage textureUsage)
	{
		return GetSemanticTextureUniformName(textureUsage) != nullptr;
	}

	int GetFixedTextureUnit(TextureUsage textureUsage)
	{
		switch (textureUsage)
		{
		case TextureUsage::Diffuse:
			return static_cast<int>(BASE_COLOR_TEXTURE_UNIT);
		case TextureUsage::Normal:
			return static_cast<int>(NORMAL_TEXTURE_UNIT);
		case TextureUsage::ORM:
			return static_cast<int>(ORM_TEXTURE_UNIT);
		case TextureUsage::Emissive:
			return static_cast<int>(EMISSIVE_TEXTURE_UNIT);
		case TextureUsage::AmbientOcclusion:
			return static_cast<int>(AMBIENT_OCCLUSION_TEXTURE_UNIT);
		case TextureUsage::Metallic:
			return static_cast<int>(METALLIC_TEXTURE_UNIT);
		case TextureUsage::Roughness:
			return static_cast<int>(ROUGHNESS_TEXTURE_UNIT);
		default:
			return -1;
		}
	}

	const char* GetSemanticTextureUniformName(TextureUsage textureUsage)
	{
		switch (textureUsage)
		{
		case TextureUsage::Diffuse:
			return "baseColorTexture";
		case TextureUsage::Normal:
			return "normalTexture";
		case TextureUsage::ORM:
			return "ormTexture";
		case TextureUsage::Emissive:
			return "emissiveTexture";
		case TextureUsage::AmbientOcclusion:
			return "ambientOcclusionTexture";
		case TextureUsage::Metallic:
			return "metallicTexture";
		case TextureUsage::Roughness:
			return "roughnessTexture";
		default:
			return nullptr;
		}
	}

	std::string GetSemanticAtlasUVTransformUniformName(TextureUsage textureUsage)
	{
		const char* textureUniformName = GetSemanticTextureUniformName(textureUsage);
		return textureUniformName ? std::string(textureUniformName) + "_UVTransform" : "";
	}
}
