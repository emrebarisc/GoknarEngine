#pragma once

#include "Goknar/Core.h"

#ifdef GOKNAR_DEBUG

#include <cstdint>
#include <string>
#include <vector>

namespace Goknar
{
namespace Debug
{
	struct ProfileThreadBuffer;

	struct GOKNAR_API ProfileEvent
	{
		const char* name{ nullptr };
		const char* filePath{ nullptr };
		std::uint64_t startTimeNs{ 0 };
		std::uint64_t endTimeNs{ 0 };
		std::uint64_t durationNs{ 0 };
		std::uint64_t threadId{ 0 };
		std::uint64_t frameIndex{ 0 };
		std::uint32_t threadIndex{ 0 };
		std::uint32_t depth{ 0 };
		std::uint32_t lineNumber{ 0 };
	};

	struct GOKNAR_API ProfileThreadInfo
	{
		std::uint32_t index{ 0 };
		std::uint64_t id{ 0 };
		std::uint64_t droppedEventCount{ 0 };
		std::string name{};
	};

	struct GOKNAR_API ProfileSnapshot
	{
		std::vector<ProfileEvent> events{};
		std::vector<ProfileThreadInfo> threads{};
		std::uint64_t firstTimestampNs{ 0 };
		std::uint64_t lastTimestampNs{ 0 };
		std::uint64_t frameIndex{ 0 };
		std::uint64_t droppedEventCount{ 0 };
	};

	class GOKNAR_API Profiler
	{
	public:
		static void SetEnabled(bool isEnabled);
		static bool IsEnabled();
		static bool IsEnabledRelaxed();

		static void SetCurrentThreadName(const char* name);
		static void Clear();
		static void CaptureOneFrame();
		static bool IsCaptureOneFramePending();
		static bool IsCapturingOneFrame();

		static ProfileSnapshot CaptureSnapshot();
		static bool ExportChromeTrace(const std::string& path);
		static bool WriteChromeTrace(const std::string& path, const ProfileSnapshot& snapshot);

		static std::uint64_t BeginFrame(const char* name);
		static void EndFrame(std::uint64_t frameIndex);
		static std::uint64_t GetCurrentFrameIndex();

		static std::uint32_t GetMaxEventsPerThread();

	private:
		friend class ProfileScope;
		friend class ProfileFrameScope;

		static bool ShouldStartFrame();
		static std::uint64_t GetTimestampNs();
		static ProfileThreadBuffer* GetThreadBuffer();
		static void RecordEvent(
			ProfileThreadBuffer* threadBuffer,
			const char* name,
			const char* filePath,
			std::uint32_t lineNumber,
			std::uint64_t startTimeNs,
			std::uint64_t endTimeNs,
			std::uint32_t depth,
			std::uint64_t frameIndex);
	};
}
}

#endif
