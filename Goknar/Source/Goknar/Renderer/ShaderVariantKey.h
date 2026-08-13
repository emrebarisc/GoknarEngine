#ifndef __SHADERVARIANTKEY_H__
#define __SHADERVARIANTKEY_H__

#include "Goknar/Core.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Renderer/Renderer.h"

#include <cstddef>
#include <cstdint>

struct GOKNAR_API ShaderVariantKey
{
	RenderPassType renderPassType{ RenderPassType::None };
	MeshType meshType{ MeshType::None };
	std::uint32_t blendModel{ 0u };
	std::uint32_t shadingModel{ 0u };
	std::uint32_t shadingType{ 0u };
	std::uint32_t materialFeatureFlags{ 0u };
	std::uint32_t textureUsageFlags{ 0u };
	std::int32_t boneCount{ 0 };
	std::uint64_t generationHash{ 0u };

	bool operator==(const ShaderVariantKey& other) const;
	bool operator!=(const ShaderVariantKey& other) const
	{
		return !(*this == other);
	}

	std::uint64_t GetHash() const;
};

struct GOKNAR_API ShaderVariantKeyHasher
{
	std::size_t operator()(const ShaderVariantKey& shaderVariantKey) const;
};

#endif
