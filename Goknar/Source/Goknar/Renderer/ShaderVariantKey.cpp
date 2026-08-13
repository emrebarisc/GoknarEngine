#include "pch.h"

#include "ShaderVariantKey.h"

namespace
{
	void HashCombine(std::uint64_t& hash, std::uint64_t value)
	{
		hash ^= value + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
	}
}

bool ShaderVariantKey::operator==(const ShaderVariantKey& other) const
{
	return renderPassType == other.renderPassType &&
		meshType == other.meshType &&
		blendModel == other.blendModel &&
		shadingModel == other.shadingModel &&
		shadingType == other.shadingType &&
		materialFeatureFlags == other.materialFeatureFlags &&
		textureUsageFlags == other.textureUsageFlags &&
		boneCount == other.boneCount &&
		generationHash == other.generationHash;
}

std::uint64_t ShaderVariantKey::GetHash() const
{
	std::uint64_t hash = 14695981039346656037ull;
	HashCombine(hash, static_cast<std::uint64_t>(renderPassType));
	HashCombine(hash, static_cast<std::uint64_t>(meshType));
	HashCombine(hash, blendModel);
	HashCombine(hash, shadingModel);
	HashCombine(hash, shadingType);
	HashCombine(hash, materialFeatureFlags);
	HashCombine(hash, textureUsageFlags);
	HashCombine(hash, static_cast<std::uint64_t>(boneCount));
	HashCombine(hash, generationHash);
	return hash;
}

std::size_t ShaderVariantKeyHasher::operator()(const ShaderVariantKey& shaderVariantKey) const
{
	return static_cast<std::size_t>(shaderVariantKey.GetHash());
}
