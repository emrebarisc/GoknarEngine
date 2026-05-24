#ifndef __DEBUGDRAWPOSTPROCESSINGEFFECT_H__
#define __DEBUGDRAWPOSTPROCESSINGEFFECT_H__

#include "PostProcessing.h"

#include "Goknar/Color.h"
#include "Goknar/Math/GoknarMath.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

class GOKNAR_API DebugDrawPostProcessingEffect : public PostProcessingEffect
{
public:
	DebugDrawPostProcessingEffect();
	~DebugDrawPostProcessingEffect() override;

	void PrepareFrame(const DeferredRenderingData* deferredRenderingData, Camera* camera) override;
	Texture* Render(const DeferredRenderingData* deferredRenderingData, const Texture* inputTexture, int width, int height) override;

	void AddLine(const Vector3& start, const Vector3& end, const Colorf& color, float thickness, float time);
	void ClearLines();

private:
	struct DebugLineGpuData
	{
		float startX;
		float startY;
		float startZ;
		float thickness;
		float endX;
		float endY;
		float endZ;
		float startTime;
		float colorR;
		float colorG;
		float colorB;
		float duration;
	};

	struct DebugLineKey
	{
		std::int32_t startX;
		std::int32_t startY;
		std::int32_t startZ;
		std::int32_t endX;
		std::int32_t endY;
		std::int32_t endZ;
		std::int32_t thickness;
		std::int32_t colorR;
		std::int32_t colorG;
		std::int32_t colorB;

		bool operator==(const DebugLineKey& other) const
		{
			return
				startX == other.startX &&
				startY == other.startY &&
				startZ == other.startZ &&
				endX == other.endX &&
				endY == other.endY &&
				endZ == other.endZ &&
				thickness == other.thickness &&
				colorR == other.colorR &&
				colorG == other.colorG &&
				colorB == other.colorB;
		}
	};

	struct DebugLineKeyHash
	{
		size_t operator()(const DebugLineKey& key) const;
	};

	void EnsureLineBuffer();
	void UploadLinesIfNeeded();
	void PruneExpiredLines(float currentTime);
	void MarkOneFrameLinesRendered();
	void RebuildLineIndexMap();
	DebugLineKey MakeLineKey(const Vector3& start, const Vector3& end, const Colorf& color, float thickness) const;

	std::vector<DebugLineGpuData> lines_;
	std::vector<bool> oneFrameLineRendered_;
	std::unordered_map<DebugLineKey, size_t, DebugLineKeyHash> lineIndexMap_;
	GEuint lineBufferId_{ 0 };
	bool lineBufferDirty_{ false };
	float currentTime_{ 0.f };
	float frameLifetime_{ 1.f / 60.f };
};

#endif
