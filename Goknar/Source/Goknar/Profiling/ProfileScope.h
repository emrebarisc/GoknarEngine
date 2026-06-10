#pragma once

#ifdef GOKNAR_DEBUG

#include "Goknar/Profiling/Profiler.h"

namespace Goknar
{
namespace Debug
{
	class GOKNAR_API ProfileScope
	{
	public:
		ProfileScope(const char* name, const char* filePath, std::uint32_t lineNumber) noexcept;
		~ProfileScope() noexcept;

		ProfileScope(const ProfileScope&) = delete;
		ProfileScope(ProfileScope&&) = delete;
		ProfileScope& operator=(const ProfileScope&) = delete;
		ProfileScope& operator=(ProfileScope&&) = delete;

	private:
		const char* name_{ nullptr };
		const char* filePath_{ nullptr };
		ProfileThreadBuffer* threadBuffer_{ nullptr };
		std::uint64_t startTimeNs_{ 0 };
		std::uint64_t frameIndex_{ 0 };
		std::uint32_t depth_{ 0 };
		std::uint32_t lineNumber_{ 0 };
		bool isActive_{ false };
	};

	class GOKNAR_API ProfileFrameScope
	{
	public:
		ProfileFrameScope(const char* name, const char* filePath, std::uint32_t lineNumber) noexcept;
		~ProfileFrameScope() noexcept;

		ProfileFrameScope(const ProfileFrameScope&) = delete;
		ProfileFrameScope(ProfileFrameScope&&) = delete;
		ProfileFrameScope& operator=(const ProfileFrameScope&) = delete;
		ProfileFrameScope& operator=(ProfileFrameScope&&) = delete;

	private:
		const char* name_{ nullptr };
		const char* filePath_{ nullptr };
		ProfileThreadBuffer* threadBuffer_{ nullptr };
		std::uint64_t startTimeNs_{ 0 };
		std::uint64_t frameIndex_{ 0 };
		std::uint32_t depth_{ 0 };
		std::uint32_t lineNumber_{ 0 };
		bool isActive_{ false };
	};
}
}

#endif
