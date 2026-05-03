#include "pch.h"

#include "BloomPostProcessingEffect.h"

#include "ComputeShader.h"
#include "Framebuffer.h"
#include "Renderer.h"

namespace
{
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
		mipLevel.texture->SetWidth(width);
		mipLevel.texture->SetHeight(height);
		mipLevel.texture->SetGenerateMipmap(false);
		mipLevel.texture->SetTextureType(TextureType::FLOAT);
		mipLevel.texture->PreInit();
		mipLevel.texture->Init();
		mipLevel.texture->PostInit();

		mipLevel.frameBuffer = new FrameBuffer();
		mipLevel.frameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT0, mipLevel.texture);
		mipLevel.frameBuffer->PreInit();
		mipLevel.frameBuffer->Init();
		mipLevel.frameBuffer->PostInit();
		mipLevel.frameBuffer->Bind();
		mipLevel.frameBuffer->Attach();
		mipLevel.frameBuffer->DrawBuffers();
		mipLevel.frameBuffer->Unbind();

		return mipLevel;
	}

	void DestroyBloomMipChain(std::vector<BloomMipLevel>& mipChain)
	{
		for (BloomMipLevel& mipLevel : mipChain)
		{
			delete mipLevel.texture;
			delete mipLevel.frameBuffer;
			mipLevel.texture = nullptr;
			mipLevel.frameBuffer = nullptr;
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
	if (!GetIsEnabled() || !deferredRenderingData || !inputTexture || width <= 0 || height <= 0)
	{
		return const_cast<Texture*>(inputTexture);
	}

	EnsureBloomResources(width, height);
	EnsureResources(width, height);

	Texture* emissiveTexture = deferredRenderingData->geometryBufferData->emmisiveColorTexture;
	if (!emissiveTexture || bloomDownsampleMipChain_.empty())
	{
		return const_cast<Texture*>(inputTexture);
	}

	const BloomMipLevel& firstMipLevel = bloomDownsampleMipChain_.front();

	bloomPrefilterComputeShader_->Use();
	emissiveTexture->BindToTextureUnit(0);
	bloomPrefilterComputeShader_->SetInt("emissiveInputTexture", 0);
	bloomPrefilterComputeShader_->SetFloat("bloomThreshold", bloomThreshold_);
	bloomPrefilterComputeShader_->SetFloat("bloomSoftKnee", bloomSoftKnee_);
	bloomPrefilterComputeShader_->SetFloat("bloomBrightnessBoost", bloomBrightnessBoost_);
	firstMipLevel.texture->BindAsImage(0, TextureImageAccess::WRITE_ONLY);
	bloomPrefilterComputeShader_->Dispatch2D(firstMipLevel.width, firstMipLevel.height);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	for (size_t mipLevelIndex = 1; mipLevelIndex < bloomDownsampleMipChain_.size(); ++mipLevelIndex)
	{
		const BloomMipLevel& sourceMipLevel = bloomDownsampleMipChain_[mipLevelIndex - 1];
		const BloomMipLevel& targetMipLevel = bloomDownsampleMipChain_[mipLevelIndex];

		bloomDownsampleComputeShader_->Use();
		sourceMipLevel.texture->BindToTextureUnit(0);
		bloomDownsampleComputeShader_->SetInt("bloomDownsampleInputTexture", 0);
		targetMipLevel.texture->BindAsImage(0, TextureImageAccess::WRITE_ONLY);
		bloomDownsampleComputeShader_->Dispatch2D(targetMipLevel.width, targetMipLevel.height);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	Texture* finalBloomTexture = bloomDownsampleMipChain_.front().texture;
	if (1 < bloomDownsampleMipChain_.size())
	{
		for (int mipLevelIndex = (int)bloomDownsampleMipChain_.size() - 2; mipLevelIndex >= 0; --mipLevelIndex)
		{
			const BloomMipLevel& baseMipLevel = bloomDownsampleMipChain_[mipLevelIndex];
			const Texture* lowerMipTexture =
				mipLevelIndex == (int)bloomDownsampleMipChain_.size() - 2 ?
				bloomDownsampleMipChain_[mipLevelIndex + 1].texture :
				bloomUpsampleMipChain_[mipLevelIndex + 1].texture;
			const BloomMipLevel& targetMipLevel = bloomUpsampleMipChain_[mipLevelIndex];

			bloomUpsampleComputeShader_->Use();
			baseMipLevel.texture->BindToTextureUnit(0);
			bloomUpsampleComputeShader_->SetInt("baseBloomInputTexture", 0);
			lowerMipTexture->BindToTextureUnit(1);
			bloomUpsampleComputeShader_->SetInt("nextMipBloomInputTexture", 1);
			bloomUpsampleComputeShader_->SetFloat("bloomScatter", bloomScatter_);
			targetMipLevel.texture->BindAsImage(0, TextureImageAccess::WRITE_ONLY);
			bloomUpsampleComputeShader_->Dispatch2D(targetMipLevel.width, targetMipLevel.height);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		finalBloomTexture = bloomUpsampleMipChain_.front().texture;
	}

	GetComputeShader()->Use();
	inputTexture->BindToTextureUnit(0);
	GetComputeShader()->SetInt("sceneInputTexture", 0);
	finalBloomTexture->BindToTextureUnit(1);
	GetComputeShader()->SetInt("bloomInputTexture", 1);
	GetComputeShader()->SetFloat("bloomIntensity", bloomIntensity_);
	GetComputeShader()->SetFloat("bloomMaxIntensityBoost", bloomMaxIntensityBoost_);
	GetComputeShader()->SetFloat("bloomIntensityCurve", bloomIntensityCurve_);
	outputTexture_->BindAsImage(0, TextureImageAccess::WRITE_ONLY);
	GetComputeShader()->Dispatch2D(width, height);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

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

	int mipWidth = baseWidth;
	int mipHeight = baseHeight;
	for (int mipLevelIndex = 0; mipLevelIndex < maxBloomMipCount_; ++mipLevelIndex)
	{
		bloomDownsampleMipChain_.push_back(CreateBloomMipLevel("bloomDownsampleMip_" + std::to_string(mipLevelIndex), mipWidth, mipHeight));
		bloomUpsampleMipChain_.push_back(CreateBloomMipLevel("bloomUpsampleMip_" + std::to_string(mipLevelIndex), mipWidth, mipHeight));

		if (mipWidth == 1 && mipHeight == 1)
		{
			break;
		}

		mipWidth = GoknarMath::Max(1, mipWidth / 2);
		mipHeight = GoknarMath::Max(1, mipHeight / 2);
	}
}

void BloomPostProcessingEffect::DestroyBloomResources()
{
	DestroyBloomMipChain(bloomDownsampleMipChain_);
	DestroyBloomMipChain(bloomUpsampleMipChain_);
}
