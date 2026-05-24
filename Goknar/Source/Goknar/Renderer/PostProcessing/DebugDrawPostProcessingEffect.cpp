#include "pch.h"

#include "DebugDrawPostProcessingEffect.h"

#include "Goknar/Camera.h"
#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/ComputeShader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Texture.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr GEuint debugLineBufferBindingIndex = 3;
	constexpr size_t maxDebugLineCount = 16384;
	constexpr float debugLinePositionQuantization = 1000.f;
	constexpr float debugLineColorQuantization = 255.f;

	float ClampDebugThickness(float thickness)
	{
		return GoknarMath::Max(1.f, thickness);
	}

	std::int32_t QuantizeDebugValue(float value, float scale)
	{
		return static_cast<std::int32_t>(std::round(value * scale));
	}

	void HashCombine(size_t& seed, std::int32_t value)
	{
		seed ^= std::hash<std::int32_t>{}(value) + 0x9e3779b9u + (seed << 6u) + (seed >> 2u);
	}
}

size_t DebugDrawPostProcessingEffect::DebugLineKeyHash::operator()(const DebugLineKey& key) const
{
	size_t seed = 0;
	HashCombine(seed, key.startX);
	HashCombine(seed, key.startY);
	HashCombine(seed, key.startZ);
	HashCombine(seed, key.endX);
	HashCombine(seed, key.endY);
	HashCombine(seed, key.endZ);
	HashCombine(seed, key.thickness);
	HashCombine(seed, key.colorR);
	HashCombine(seed, key.colorG);
	HashCombine(seed, key.colorB);
	return seed;
}

DebugDrawPostProcessingEffect::DebugDrawPostProcessingEffect()
{
	ComputeShader* debugDrawComputeShader = new ComputeShader();
	const std::string projectShaderPath = ContentDir + "Shaders/PostProcessing/DebugDraw.comp";
	const std::string engineShaderPath = EngineContentDir + "Shaders/PostProcessing/DebugDraw.comp";
	const std::string shaderPath = DataEncryption::FileExists(projectShaderPath) ? projectShaderPath : engineShaderPath;
	debugDrawComputeShader->SetComputeShaderPathAbsolute(shaderPath);
	SetComputeShader(debugDrawComputeShader);
	lines_.reserve(512);
}

DebugDrawPostProcessingEffect::~DebugDrawPostProcessingEffect()
{
	if (lineBufferId_ != 0 && engine && engine->GetGraphicsAPI())
	{
		engine->GetGraphicsAPI()->DeleteBuffer(lineBufferId_);
		lineBufferId_ = 0;
	}
}

void DebugDrawPostProcessingEffect::PrepareFrame(const DeferredRenderingData* deferredRenderingData, Camera* camera)
{
	(void)deferredRenderingData;
	(void)camera;

	currentTime_ = engine ? engine->GetElapsedTime() : 0.f;
	const float deltaTime = engine ? engine->GetDeltaTime() : frameLifetime_;
	frameLifetime_ = GoknarMath::Max(deltaTime * 1.5f, 1.f / 60.f);
	PruneExpiredLines(currentTime_);
	UploadLinesIfNeeded();
}

Texture* DebugDrawPostProcessingEffect::Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height)
{
	if (!GetIsEnabled() || !GetComputeShader() || !deferredRenderingData || !inputTexture || width <= 0 || height <= 0 || lines_.empty())
	{
		return const_cast<Texture*>(inputTexture);
	}

	const Camera* activeCamera = engine && engine->GetCameraManager() ? engine->GetCameraManager()->GetActiveCamera() : nullptr;
	if (!activeCamera)
	{
		return const_cast<Texture*>(inputTexture);
	}

	EnsureResources(width, height);
	UploadLinesIfNeeded();

	GetComputeShader()->Use();

	const int inputTextureUnit = static_cast<int>(inputTexture->GetRendererTextureId());
	inputTexture->BindToTextureUnit(inputTextureUnit);
	GetComputeShader()->SetInt("inputTexture", inputTextureUnit);
	GetComputeShader()->SetMatrix("viewProjectionMatrix", activeCamera->GetViewProjectionMatrix());
	GetComputeShader()->SetInt("debugLineCount", static_cast<int>(lines_.size()));
	GetComputeShader()->SetFloat("currentTime", currentTime_);
	GetComputeShader()->SetFloat("frameLifetime", frameLifetime_);

	engine->GetGraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, debugLineBufferBindingIndex, lineBufferId_);
	outputTexture_->BindAsImage(0, TextureImageAccess::WRITE_ONLY);

	GetComputeShader()->SetInt("renderMode", 0);
	GetComputeShader()->Dispatch2D(width, height);
	engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);

	float maxLineThickness = 1.f;
	for (const DebugLineGpuData& line : lines_)
	{
		maxLineThickness = GoknarMath::Max(maxLineThickness, line.thickness);
	}

	const int maxLinePixelSteps = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(width * width + height * height)) + maxLineThickness + 2.f));
	GetComputeShader()->SetInt("renderMode", 1);
	GetComputeShader()->Dispatch2D(maxLinePixelSteps, static_cast<int>(lines_.size()));
	engine->GetGraphicsAPI()->MemoryBarrier(GraphicsMemoryBarrier::ShaderStorage | GraphicsMemoryBarrier::ShaderImageAccess | GraphicsMemoryBarrier::TextureFetch);
	MarkOneFrameLinesRendered();

	return outputTexture_;
}

void DebugDrawPostProcessingEffect::AddLine(const Vector3& start, const Vector3& end, const Colorf& color, float thickness, float time)
{
	if (lines_.size() >= maxDebugLineCount)
	{
		lines_.erase(lines_.begin());
		if (!oneFrameLineRendered_.empty())
		{
			oneFrameLineRendered_.erase(oneFrameLineRendered_.begin());
		}
		RebuildLineIndexMap();
	}

	const float startTime = engine ? engine->GetElapsedTime() : currentTime_;
	const float safeThickness = ClampDebugThickness(thickness);
	const DebugLineKey lineKey = MakeLineKey(start, end, color, safeThickness);
	const DebugLineGpuData lineGpuData
		{
			start.x, start.y, start.z, safeThickness,
			end.x, end.y, end.z, startTime,
			color.r, color.g, color.b, time
		};

	auto existingLineIterator = lineIndexMap_.find(lineKey);
	if (existingLineIterator != lineIndexMap_.end() && existingLineIterator->second < lines_.size())
	{
		const size_t lineIndex = existingLineIterator->second;
		lines_[lineIndex] = lineGpuData;
		if (lineIndex < oneFrameLineRendered_.size())
		{
			oneFrameLineRendered_[lineIndex] = false;
		}
		lineBufferDirty_ = true;
		return;
	}

	lines_.push_back(lineGpuData);
	oneFrameLineRendered_.push_back(false);
	lineIndexMap_[lineKey] = lines_.size() - 1u;
	lineBufferDirty_ = true;
}

void DebugDrawPostProcessingEffect::ClearLines()
{
	if (lines_.empty())
	{
		return;
	}

	lines_.clear();
	oneFrameLineRendered_.clear();
	lineIndexMap_.clear();
	lineBufferDirty_ = true;
}

void DebugDrawPostProcessingEffect::EnsureLineBuffer()
{
	if (lineBufferId_ != 0 || !engine || !engine->GetGraphicsAPI())
	{
		return;
	}

	lineBufferId_ = engine->GetGraphicsAPI()->CreateBuffer();
}

void DebugDrawPostProcessingEffect::UploadLinesIfNeeded()
{
	if (!lineBufferDirty_ || !engine || !engine->GetGraphicsAPI())
	{
		return;
	}

	EnsureLineBuffer();
	if (lineBufferId_ == 0)
	{
		return;
	}

	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, lineBufferId_);
	const GEsizeiptr bufferSize = static_cast<GEsizeiptr>((std::max)(size_t(1), lines_.size()) * sizeof(DebugLineGpuData));
	graphicsAPI->BufferData(
		GraphicsBufferTarget::ShaderStorageBuffer,
		bufferSize,
		lines_.empty() ? nullptr : lines_.data(),
		GraphicsBufferUsage::DynamicDraw);
	graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0);
	lineBufferDirty_ = false;
}

void DebugDrawPostProcessingEffect::PruneExpiredLines(float currentTime)
{
	const size_t oldLineCount = lines_.size();
	size_t writeIndex = 0;
	for (size_t readIndex = 0; readIndex < lines_.size(); ++readIndex)
	{
		const DebugLineGpuData& line = lines_[readIndex];
		const bool shouldPrune =
			line.duration < 0.f ? false :
			line.duration == 0.f ?
			(readIndex < oneFrameLineRendered_.size() && oneFrameLineRendered_[readIndex]) :
			(currentTime > line.startTime + line.duration);

		if (!shouldPrune)
		{
			if (writeIndex != readIndex)
			{
				lines_[writeIndex] = lines_[readIndex];
				if (writeIndex < oneFrameLineRendered_.size() && readIndex < oneFrameLineRendered_.size())
				{
					oneFrameLineRendered_[writeIndex] = oneFrameLineRendered_[readIndex];
				}
			}
			++writeIndex;
		}
	}

	lines_.resize(writeIndex);
	oneFrameLineRendered_.resize(writeIndex);
	if (oldLineCount != lines_.size())
	{
		RebuildLineIndexMap();
	}

	lineBufferDirty_ = lineBufferDirty_ || oldLineCount != lines_.size();
}

void DebugDrawPostProcessingEffect::MarkOneFrameLinesRendered()
{
	for (size_t lineIndex = 0; lineIndex < lines_.size() && lineIndex < oneFrameLineRendered_.size(); ++lineIndex)
	{
		if (lines_[lineIndex].duration == 0.f)
		{
			oneFrameLineRendered_[lineIndex] = true;
		}
	}
}

void DebugDrawPostProcessingEffect::RebuildLineIndexMap()
{
	lineIndexMap_.clear();
	lineIndexMap_.reserve(lines_.size());
	for (size_t lineIndex = 0; lineIndex < lines_.size(); ++lineIndex)
	{
		const DebugLineGpuData& line = lines_[lineIndex];
		lineIndexMap_[MakeLineKey(
			Vector3{ line.startX, line.startY, line.startZ },
			Vector3{ line.endX, line.endY, line.endZ },
			Colorf{ line.colorR, line.colorG, line.colorB },
			line.thickness)] = lineIndex;
	}
}

DebugDrawPostProcessingEffect::DebugLineKey DebugDrawPostProcessingEffect::MakeLineKey(const Vector3& start, const Vector3& end, const Colorf& color, float thickness) const
{
	return DebugLineKey
		{
			QuantizeDebugValue(start.x, debugLinePositionQuantization),
			QuantizeDebugValue(start.y, debugLinePositionQuantization),
			QuantizeDebugValue(start.z, debugLinePositionQuantization),
			QuantizeDebugValue(end.x, debugLinePositionQuantization),
			QuantizeDebugValue(end.y, debugLinePositionQuantization),
			QuantizeDebugValue(end.z, debugLinePositionQuantization),
			QuantizeDebugValue(thickness, debugLinePositionQuantization),
			QuantizeDebugValue(color.r, debugLineColorQuantization),
			QuantizeDebugValue(color.g, debugLineColorQuantization),
			QuantizeDebugValue(color.b, debugLineColorQuantization)
		};
}
