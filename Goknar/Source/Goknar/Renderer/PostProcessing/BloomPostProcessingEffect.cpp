#include "pch.h"

#include "BloomPostProcessingEffect.h"

#include "Renderer/ComputeShader.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Renderer.h"

#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"

namespace
{
	constexpr int kBloomOutputImageUnit = 0;

	int BindTextureUsingEngineTextureUnit(const Texture* texture)
	{
		const int textureUnit = static_cast<int>(texture->GetRendererTextureId());
		texture->BindToTextureUnit(textureUnit);
		return textureUnit;
	}

	BloomMipLevel CreateBloomMipLevel(const std::string& textureName, int width, int height)
	{
		BloomMipLevel mipLevel{};
		mipLevel.width = width;
		mipLevel.height = height;

		mipLevel.texture = new Texture();
		mipLevel.texture->SetName(textureName);
		mipLevel.texture->SetTextureDataType(TextureDataType::DYNAMIC);
		mipLevel.texture->SetTextureFormat(TextureFormat::RGBA);
		mipLevel.texture->SetTextureInternalFormat(TextureInternalFormat::RGBA16F);
		mipLevel.texture->SetTextureMinFilter(TextureMinFilter::LINEAR);
		mipLevel.texture->SetTextureMagFilter(TextureMagFilter::LINEAR);
		mipLevel.texture->SetTextureWrappingR(TextureWrapping::CLAMP_TO_EDGE);
		mipLevel.texture->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
		mipLevel.texture->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
		mipLevel.texture->SetWidth(width);
		mipLevel.texture->SetHeight(height);
		mipLevel.texture->SetGenerateMipmap(false);
		mipLevel.texture->SetTextureType(TextureType::FLOAT);
		mipLevel.texture->PreInit();
		mipLevel.texture->Init();
		mipLevel.texture->PostInit();

		return mipLevel;
	}

	void DestroyBloomMipChain(std::vector<BloomMipLevel>& mipChain)
	{
		for (BloomMipLevel& mipLevel : mipChain)
		{
			delete mipLevel.texture;
			mipLevel.texture = nullptr;
		}

		mipChain.clear();
	}
}

BloomPostProcessingEffect::BloomPostProcessingEffect()
{
	bloomPrefilterComputeShader_ = new ComputeShader();
	bloomDownsampleComputeShader_ = new ComputeShader();
	bloomUpsampleComputeShader_ = new ComputeShader();

	ComputeShader* bloomCombineComputeShader = new ComputeShader();

#if defined(GOKNAR_BUILD_DEBUG)
	const std::string bloomShaderDirectory = EngineContentDir + "Shaders/PostProcessing/";
#else
	const std::string bloomShaderDirectory = ContentDir + "Shaders/PostProcessing/";
#endif

	bloomPrefilterComputeShader_->SetComputeShaderPathAbsolute(bloomShaderDirectory + "BloomPrefilter.comp");
	bloomDownsampleComputeShader_->SetComputeShaderPathAbsolute(bloomShaderDirectory + "BloomDownsample.comp");
	bloomUpsampleComputeShader_->SetComputeShaderPathAbsolute(bloomShaderDirectory + "BloomUpsample.comp");
	bloomCombineComputeShader->SetComputeShaderPathAbsolute(bloomShaderDirectory + "BloomCombine.comp");

	SetComputeShader(bloomCombineComputeShader);
}

BloomPostProcessingEffect::~BloomPostProcessingEffect()
{
	delete bloomPrefilterComputeShader_;
	delete bloomDownsampleComputeShader_;
	delete bloomUpsampleComputeShader_;

	DestroyBloomResources();
}

void BloomPostProcessingEffect::PreInit()
{
	if (bloomPrefilterComputeShader_)
	{
		bloomPrefilterComputeShader_->PreInit();
	}

	if (bloomDownsampleComputeShader_)
	{
		bloomDownsampleComputeShader_->PreInit();
	}

	if (bloomUpsampleComputeShader_)
	{
		bloomUpsampleComputeShader_->PreInit();
	}

	PostProcessingEffect::PreInit();
}

void BloomPostProcessingEffect::Init()
{
	if (bloomPrefilterComputeShader_)
	{
		bloomPrefilterComputeShader_->Init();
	}

	if (bloomDownsampleComputeShader_)
	{
		bloomDownsampleComputeShader_->Init();
	}

	if (bloomUpsampleComputeShader_)
	{
		bloomUpsampleComputeShader_->Init();
	}

	PostProcessingEffect::Init();
}

void BloomPostProcessingEffect::PostInit()
{
	if (bloomPrefilterComputeShader_)
	{
		bloomPrefilterComputeShader_->PostInit();
	}

	if (bloomDownsampleComputeShader_)
	{
		bloomDownsampleComputeShader_->PostInit();
	}

	if (bloomUpsampleComputeShader_)
	{
		bloomUpsampleComputeShader_->PostInit();
	}

	PostProcessingEffect::PostInit();
}

Texture* BloomPostProcessingEffect::Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	if (!GetIsEnabled() || !deferredRenderingData || !inputTexture || width <= 0 || height <= 0 || bloomIntensity_ <= 0.f)
	{
		return const_cast<Texture*>(inputTexture);
	}

	Texture* emissiveTexture = deferredRenderingData->geometryBufferData ?
		deferredRenderingData->geometryBufferData->emissiveColorTexture : nullptr;
	if (!emissiveTexture)
	{
		return const_cast<Texture*>(inputTexture);
	}

	EnsureBloomResources(width, height);
	EnsureResources(width, height);

	if (bloomDownsampleMipChain_.empty())
	{
		return const_cast<Texture*>(inputTexture);
	}

	const BloomMipLevel& firstMipLevel = bloomDownsampleMipChain_.front();

	bloomPrefilterComputeShader_->Use();
	bloomPrefilterComputeShader_->SetInt("emissiveInputTexture", BindTextureUsingEngineTextureUnit(emissiveTexture));
	bloomPrefilterComputeShader_->SetFloat("bloomThreshold", bloomThreshold_);
	bloomPrefilterComputeShader_->SetFloat("bloomSoftKnee", bloomSoftKnee_);
	bloomPrefilterComputeShader_->SetFloat("bloomBrightnessBoost", bloomBrightnessBoost_);
	firstMipLevel.texture->BindAsImage(kBloomOutputImageUnit, TextureImageAccess::WRITE_ONLY);
	bloomPrefilterComputeShader_->Dispatch2D(firstMipLevel.width, firstMipLevel.height);
	engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);

	if (1 < bloomDownsampleMipChain_.size())
	{
		bloomDownsampleComputeShader_->Use();

		for (size_t mipLevelIndex = 1; mipLevelIndex < bloomDownsampleMipChain_.size(); ++mipLevelIndex)
		{
			const BloomMipLevel& sourceMipLevel = bloomDownsampleMipChain_[mipLevelIndex - 1];
			const BloomMipLevel& targetMipLevel = bloomDownsampleMipChain_[mipLevelIndex];

			bloomDownsampleComputeShader_->SetInt("bloomDownsampleInputTexture", BindTextureUsingEngineTextureUnit(sourceMipLevel.texture));
			targetMipLevel.texture->BindAsImage(kBloomOutputImageUnit, TextureImageAccess::WRITE_ONLY);
			bloomDownsampleComputeShader_->Dispatch2D(targetMipLevel.width, targetMipLevel.height);
			engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);
		}
	}

	Texture* finalBloomTexture = bloomDownsampleMipChain_.front().texture;
	if (!bloomUpsampleMipChain_.empty())
	{
		bloomUpsampleComputeShader_->Use();
		bloomUpsampleComputeShader_->SetFloat("bloomScatter", bloomScatter_);

		for (int mipLevelIndex = static_cast<int>(bloomUpsampleMipChain_.size()) - 1; mipLevelIndex >= 0; --mipLevelIndex)
		{
			const BloomMipLevel& baseMipLevel = bloomDownsampleMipChain_[mipLevelIndex];
			const Texture* lowerMipTexture =
				mipLevelIndex == static_cast<int>(bloomUpsampleMipChain_.size()) - 1 ?
				bloomDownsampleMipChain_[mipLevelIndex + 1].texture :
				bloomUpsampleMipChain_[mipLevelIndex + 1].texture;
			const BloomMipLevel& targetMipLevel = bloomUpsampleMipChain_[mipLevelIndex];

			bloomUpsampleComputeShader_->SetInt("baseBloomInputTexture", BindTextureUsingEngineTextureUnit(baseMipLevel.texture));
			bloomUpsampleComputeShader_->SetInt("nextMipBloomInputTexture", BindTextureUsingEngineTextureUnit(lowerMipTexture));
			targetMipLevel.texture->BindAsImage(kBloomOutputImageUnit, TextureImageAccess::WRITE_ONLY);
			bloomUpsampleComputeShader_->Dispatch2D(targetMipLevel.width, targetMipLevel.height);
			engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);
		}

		finalBloomTexture = bloomUpsampleMipChain_.front().texture;
	}

	GetComputeShader()->Use();
	GetComputeShader()->SetInt("sceneInputTexture", BindTextureUsingEngineTextureUnit(inputTexture));
	GetComputeShader()->SetInt("bloomInputTexture", BindTextureUsingEngineTextureUnit(finalBloomTexture));
	GetComputeShader()->SetFloat("bloomIntensity", bloomIntensity_);
	GetComputeShader()->SetFloat("bloomMaxIntensityBoost", bloomMaxIntensityBoost_);
	GetComputeShader()->SetFloat("bloomIntensityCurve", bloomIntensityCurve_);
	outputTexture_->BindAsImage(kBloomOutputImageUnit, TextureImageAccess::WRITE_ONLY);
	GetComputeShader()->Dispatch2D(width, height);
	engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);

	return outputTexture_;
}

void BloomPostProcessingEffect::EnsureBloomResources(int width, int height)
{
	if (width_ != width || height_ != height || bloomDownsampleMipChain_.empty())
	{
		RecreateBloomResources(width, height);
	}
}

void BloomPostProcessingEffect::RecreateBloomResources(int width, int height)
{
	DestroyBloomResources();

	const int baseWidth = GoknarMath::Max(1, width / bloomBaseResolutionDivisor_);
	const int baseHeight = GoknarMath::Max(1, height / bloomBaseResolutionDivisor_);

	bloomDownsampleMipChain_.reserve(maxBloomMipCount_);
	bloomUpsampleMipChain_.reserve(GoknarMath::Max(0, maxBloomMipCount_ - 1));

	int mipWidth = baseWidth;
	int mipHeight = baseHeight;
	for (int mipLevelIndex = 0; mipLevelIndex < maxBloomMipCount_; ++mipLevelIndex)
	{
		bloomDownsampleMipChain_.push_back(CreateBloomMipLevel("bloomDownsampleMip_" + std::to_string(mipLevelIndex), mipWidth, mipHeight));

		if (mipWidth == 1 && mipHeight == 1)
		{
			break;
		}

		mipWidth = GoknarMath::Max(1, mipWidth / 2);
		mipHeight = GoknarMath::Max(1, mipHeight / 2);
	}

	for (size_t mipLevelIndex = 0; mipLevelIndex + 1 < bloomDownsampleMipChain_.size(); ++mipLevelIndex)
	{
		const BloomMipLevel& downsampleMipLevel = bloomDownsampleMipChain_[mipLevelIndex];
		bloomUpsampleMipChain_.push_back(CreateBloomMipLevel(
			"bloomUpsampleMip_" + std::to_string(mipLevelIndex),
			downsampleMipLevel.width,
			downsampleMipLevel.height));
	}
}

void BloomPostProcessingEffect::DestroyBloomResources()
{
	DestroyBloomMipChain(bloomDownsampleMipChain_);
	DestroyBloomMipChain(bloomUpsampleMipChain_);
}
