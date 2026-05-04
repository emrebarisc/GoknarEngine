#include "pch.h"

#include "TemporalAntiAliasingPostProcessingEffect.h"

#include "ComputeShader.h"
#include "Framebuffer.h"
#include "Renderer.h"
#include "Texture.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/CameraManager.h"

namespace
{
	float GetHaltonValue(std::uint64_t index, int base)
	{
		float result = 0.f;
		float inverseBase = 1.f / static_cast<float>(base);
		float fraction = inverseBase;

		while (index > 0)
		{
			result += static_cast<float>(index % base) * fraction;
			index /= base;
			fraction *= inverseBase;
		}

		return result;
	}

	Texture* CreateTemporalTexture(const std::string& textureName, int width, int height)
	{
		Texture* texture = new Texture();
		texture->SetName(textureName);
		texture->SetTextureDataType(TextureDataType::DYNAMIC);
		texture->SetTextureFormat(TextureFormat::RGBA);
		texture->SetTextureInternalFormat(TextureInternalFormat::RGBA16F);
		texture->SetTextureMinFilter(TextureMinFilter::LINEAR);
		texture->SetTextureMagFilter(TextureMagFilter::LINEAR);
		texture->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
		texture->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
		texture->SetWidth(width);
		texture->SetHeight(height);
		texture->SetGenerateMipmap(false);
		texture->SetTextureType(TextureType::FLOAT);
		texture->PreInit();
		texture->Init();
		texture->PostInit();

		return texture;
	}

	FrameBuffer* CreateTemporalFrameBuffer(Texture* texture)
	{
		FrameBuffer* frameBuffer = new FrameBuffer();
		frameBuffer->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT0, texture);
		frameBuffer->PreInit();
		frameBuffer->Init();
		frameBuffer->PostInit();
		frameBuffer->Bind();
		frameBuffer->Attach();
		frameBuffer->DrawBuffers();
		frameBuffer->Unbind();

		return frameBuffer;
	}

	void DestroyTemporalTextureAndFrameBuffer(Texture*& texture, FrameBuffer*& frameBuffer)
	{
		delete texture;
		texture = nullptr;

		delete frameBuffer;
		frameBuffer = nullptr;
	}

	void BlitTemporalColor(FrameBuffer* sourceFrameBuffer, FrameBuffer* destinationFrameBuffer, int width, int height)
	{
		if (!sourceFrameBuffer || !destinationFrameBuffer || width <= 0 || height <= 0)
		{
			return;
		}

		sourceFrameBuffer->Bind(FrameBufferBindTarget::READ_FRAMEBUFFER);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		destinationFrameBuffer->Bind(FrameBufferBindTarget::DRAW_FRAMEBUFFER);

		glBlitFramebuffer(
			0, 0, width, height,
			0, 0, width, height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

TemporalAntiAliasingPostProcessingEffect::TemporalAntiAliasingPostProcessingEffect()
{
	ComputeShader* temporalAntiAliasingComputeShader = new ComputeShader();

#if defined(GOKNAR_BUILD_DEBUG)
	const std::string shaderDirectory = EngineContentDir + "Shaders/PostProcessing/";
#else
	const std::string shaderDirectory = ContentDir + "Shaders/PostProcessing/";
#endif

	temporalAntiAliasingComputeShader->SetComputeShaderPathAbsolute(shaderDirectory + "TemporalAntiAliasing.comp");
	SetComputeShader(temporalAntiAliasingComputeShader);
}

TemporalAntiAliasingPostProcessingEffect::~TemporalAntiAliasingPostProcessingEffect()
{
	DestroyAllContexts();
	outputTexture_ = nullptr;
	outputFrameBuffer_ = nullptr;
}

void TemporalAntiAliasingPostProcessingEffect::PrepareFrame(const DeferredRenderingData* deferredRenderingData, Camera* camera)
{
	if (!camera)
	{
		return;
	}

	if (!deferredRenderingData || !GetIsEnabled())
	{
		camera->SetTemporalJitter(Vector2::ZeroVector);

		if (deferredRenderingData)
		{
			TemporalAntiAliasingRenderContext& renderContext = renderContexts_[deferredRenderingData];
			renderContext.hasValidHistory = false;
			renderContext.frameIndex = 0;
			renderContext.readHistoryIndex = 0;
			renderContext.lastCamera = nullptr;
		}

		return;
	}

	TemporalAntiAliasingRenderContext& renderContext = renderContexts_[deferredRenderingData];
	if (renderContext.lastCamera != camera)
	{
		renderContext.lastCamera = camera;
		renderContext.previousViewProjectionMatrix = Matrix::IdentityMatrix;
		renderContext.previousJitterUv = Vector2::ZeroVector;
		renderContext.hasValidHistory = false;
		renderContext.frameIndex = 0;
		renderContext.readHistoryIndex = 0;
	}

	camera->SetTemporalJitter(GetHaltonJitter(renderContext.frameIndex));
}

Texture* TemporalAntiAliasingPostProcessingEffect::Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	if (!GetIsEnabled() || !GetComputeShader() || !deferredRenderingData || !inputTexture || width <= 0 || height <= 0)
	{
		return const_cast<Texture*>(inputTexture);
	}

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	if (!activeCamera || !deferredRenderingData->geometryBufferData)
	{
		return const_cast<Texture*>(inputTexture);
	}

	Texture* worldPositionTexture = deferredRenderingData->geometryBufferData->worldPositionTexture;
	Texture* worldNormalTexture = deferredRenderingData->geometryBufferData->worldNormalTexture;
	if (!worldPositionTexture || !worldNormalTexture)
	{
		return const_cast<Texture*>(inputTexture);
	}

	TemporalAntiAliasingRenderContext& renderContext = renderContexts_[deferredRenderingData];
	EnsureContextResources(renderContext, width, height);

	outputTexture_ = renderContext.outputTexture;
	outputFrameBuffer_ = renderContext.outputFrameBuffer;

	const bool canUseHistory =
		renderContext.hasValidHistory &&
		renderContext.historyTextures[renderContext.readHistoryIndex] != nullptr;

	const Texture* historyTexture = canUseHistory ? renderContext.historyTextures[renderContext.readHistoryIndex] : inputTexture;
	const int inputTextureUnit = static_cast<int>(inputTexture->GetRendererTextureId());
	const int historyTextureUnit = static_cast<int>(historyTexture->GetRendererTextureId());
	const int worldPositionTextureUnit = static_cast<int>(worldPositionTexture->GetRendererTextureId());
	const int worldNormalTextureUnit = static_cast<int>(worldNormalTexture->GetRendererTextureId());

	Vector2 currentJitter = activeCamera->GetTemporalJitter();
	Vector2 currentJitterUv(-currentJitter.x / static_cast<float>(width), -currentJitter.y / static_cast<float>(height));

	GetComputeShader()->Use();
	inputTexture->BindToTextureUnit(inputTextureUnit);
	GetComputeShader()->SetInt("inputTexture", inputTextureUnit);
	historyTexture->BindToTextureUnit(historyTextureUnit);
	GetComputeShader()->SetInt("historyTexture", historyTextureUnit);
	worldPositionTexture->BindToTextureUnit(worldPositionTextureUnit);
	GetComputeShader()->SetInt("worldPositionTexture", worldPositionTextureUnit);
	worldNormalTexture->BindToTextureUnit(worldNormalTextureUnit);
	GetComputeShader()->SetInt("normalTexture", worldNormalTextureUnit);
	GetComputeShader()->SetBool("hasHistory", canUseHistory);
	GetComputeShader()->SetFloat("maxHistoryWeight", maxHistoryWeight_);
	GetComputeShader()->SetFloat("minHistoryWeight", minHistoryWeight_);
	GetComputeShader()->SetFloat("motionWeightScale", motionWeightScale_);
	GetComputeShader()->SetVector2("inverseTextureSize", Vector2(1.f / static_cast<float>(width), 1.f / static_cast<float>(height)));
	GetComputeShader()->SetMatrix("previousViewProjectionMatrix", canUseHistory ? renderContext.previousViewProjectionMatrix : activeCamera->GetViewProjectionMatrix());
	GetComputeShader()->SetVector2("currentJitterUv", currentJitterUv);
	GetComputeShader()->SetVector2("previousJitterUv", canUseHistory ? renderContext.previousJitterUv : currentJitterUv);
	outputTexture_->BindAsImage(0, TextureImageAccess::WRITE_ONLY);
	GetComputeShader()->Dispatch2D(width, height);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	const int writeHistoryIndex = 1 - renderContext.readHistoryIndex;
	BlitTemporalColor(renderContext.outputFrameBuffer, renderContext.historyFrameBuffers[writeHistoryIndex], width, height);

	renderContext.readHistoryIndex = writeHistoryIndex;
	renderContext.previousViewProjectionMatrix = activeCamera->GetViewProjectionMatrix();
	renderContext.previousJitterUv = currentJitterUv;
	renderContext.hasValidHistory = true;
	++renderContext.frameIndex;

	return outputTexture_;
}

void TemporalAntiAliasingPostProcessingEffect::EnsureContextResources(TemporalAntiAliasingRenderContext& renderContext, int width, int height)
{
	const bool hasAllResources =
		renderContext.outputTexture &&
		renderContext.outputFrameBuffer &&
		renderContext.historyTextures[0] &&
		renderContext.historyTextures[1] &&
		renderContext.historyFrameBuffers[0] &&
		renderContext.historyFrameBuffers[1];

	if (hasAllResources && renderContext.width == width && renderContext.height == height)
	{
		return;
	}

	RecreateContextResources(renderContext, width, height);
}

void TemporalAntiAliasingPostProcessingEffect::RecreateContextResources(TemporalAntiAliasingRenderContext& renderContext, int width, int height)
{
	DestroyContextResources(renderContext);

	renderContext.width = width;
	renderContext.height = height;
	renderContext.readHistoryIndex = 0;
	renderContext.frameIndex = 0;
	renderContext.hasValidHistory = false;
	renderContext.previousViewProjectionMatrix = Matrix::IdentityMatrix;
	renderContext.previousJitterUv = Vector2::ZeroVector;

	renderContext.outputTexture = CreateTemporalTexture("taaOutputTexture", width, height);
	renderContext.outputFrameBuffer = CreateTemporalFrameBuffer(renderContext.outputTexture);

	renderContext.historyTextures[0] = CreateTemporalTexture("taaHistoryTexture0", width, height);
	renderContext.historyFrameBuffers[0] = CreateTemporalFrameBuffer(renderContext.historyTextures[0]);
	renderContext.historyTextures[1] = CreateTemporalTexture("taaHistoryTexture1", width, height);
	renderContext.historyFrameBuffers[1] = CreateTemporalFrameBuffer(renderContext.historyTextures[1]);
}

void TemporalAntiAliasingPostProcessingEffect::DestroyContextResources(TemporalAntiAliasingRenderContext& renderContext)
{
	DestroyTemporalTextureAndFrameBuffer(renderContext.outputTexture, renderContext.outputFrameBuffer);
	DestroyTemporalTextureAndFrameBuffer(renderContext.historyTextures[0], renderContext.historyFrameBuffers[0]);
	DestroyTemporalTextureAndFrameBuffer(renderContext.historyTextures[1], renderContext.historyFrameBuffers[1]);

	renderContext.width = 0;
	renderContext.height = 0;
	renderContext.readHistoryIndex = 0;
	renderContext.hasValidHistory = false;
	renderContext.lastCamera = nullptr;
	renderContext.previousViewProjectionMatrix = Matrix::IdentityMatrix;
	renderContext.previousJitterUv = Vector2::ZeroVector;
}

void TemporalAntiAliasingPostProcessingEffect::DestroyAllContexts()
{
	for (auto& renderContextPair : renderContexts_)
	{
		DestroyContextResources(renderContextPair.second);
	}

	renderContexts_.clear();
}

Vector2 TemporalAntiAliasingPostProcessingEffect::GetHaltonJitter(std::uint64_t frameIndex) const
{
	const std::uint64_t haltonIndex = frameIndex + 1;
	return Vector2(
		(GetHaltonValue(haltonIndex, 2) - 0.5f) * jitterScale_,
		(GetHaltonValue(haltonIndex, 3) - 0.5f) * jitterScale_);
}
