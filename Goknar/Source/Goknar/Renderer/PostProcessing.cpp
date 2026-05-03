#include "pch.h"

#include "PostProcessing.h"

#include "ComputeShader.h"
#include "Framebuffer.h"

#include "Goknar/GoknarAssert.h"

#include <glad/glad.h>

PostProcessingEffect::PostProcessingEffect()
{
}

PostProcessingEffect::~PostProcessingEffect()
{
	delete outputTexture_;
	delete outputFrameBuffer_;
	delete computeShader_;
}

void PostProcessingEffect::PreInit()
{
	if (computeShader_)
	{
		computeShader_->PreInit();
	}
}

void PostProcessingEffect::Init()
{
	if (computeShader_)
	{
		computeShader_->Init();
	}
}

void PostProcessingEffect::PostInit()
{
	if (computeShader_)
	{
		computeShader_->PostInit();
	}
}

void PostProcessingEffect::PrepareFrame(const DeferredRenderingData* deferredRenderingData, Camera* camera)
{
	(void)deferredRenderingData;
	(void)camera;
}

Texture* PostProcessingEffect::Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	if (!isEnabled_ || !computeShader_ || !inputTexture || width <= 0 || height <= 0)
	{
		return const_cast<Texture*>(inputTexture);
	}

	EnsureResources(width, height);
	OnRender(deferredRenderingData, inputTexture, width, height);

	return outputTexture_;
}

void PostProcessingEffect::EnsureResources(int width, int height)
{
	if (width_ == width && height_ == height && outputTexture_ && outputFrameBuffer_)
	{
		return;
	}

	RecreateOutputResources(width, height);
}

void PostProcessingEffect::RecreateOutputResources(int width, int height, TextureInternalFormat internalFormat, TextureFormat textureFormat, TextureType textureType)
{
	delete outputTexture_;
	delete outputFrameBuffer_;

	width_ = width;
	height_ = height;

	outputTexture_ = new Texture();
	outputTexture_->SetName("postProcessOutputTexture");
	outputTexture_->SetTextureDataType(TextureDataType::DYNAMIC);
	outputTexture_->SetTextureFormat(textureFormat);
	outputTexture_->SetTextureInternalFormat(internalFormat);
	outputTexture_->SetTextureMinFilter(TextureMinFilter::LINEAR);
	outputTexture_->SetTextureMagFilter(TextureMagFilter::LINEAR);
	outputTexture_->SetWidth(width_);
	outputTexture_->SetHeight(height_);
	outputTexture_->SetGenerateMipmap(false);
	outputTexture_->SetTextureType(textureType);
	outputTexture_->PreInit();
	outputTexture_->Init();
	outputTexture_->PostInit();

	outputFrameBuffer_ = new FrameBuffer();
	outputFrameBuffer_->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT0, outputTexture_);
	outputFrameBuffer_->PreInit();
	outputFrameBuffer_->Init();
	outputFrameBuffer_->PostInit();
	outputFrameBuffer_->Bind();
	outputFrameBuffer_->Attach();
	outputFrameBuffer_->DrawBuffers();
	outputFrameBuffer_->Unbind();
}

void PostProcessingEffect::OnRender(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	GOKNAR_CORE_CHECK(deferredRenderingData != nullptr, "Deferred rendering data is required for post-processing.");

	computeShader_->Use();
	const int inputTextureUnit = static_cast<int>(inputTexture->GetRendererTextureId());
	inputTexture->BindToTextureUnit(inputTextureUnit);
	computeShader_->SetInt("inputTexture", inputTextureUnit);
	outputTexture_->BindAsImage(0, TextureImageAccess::WRITE_ONLY);
	computeShader_->Dispatch2D(width, height);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}
