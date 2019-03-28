#pragma once

#include "Core.h"

#include <memory>

#include "spdlog/spdlog.h"

namespace Goknar {
	class GOKNAR_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
		{
			return coreLogger_;
		}

		inline static std::shared_ptr<spdlog::logger>& GetClientLogger()
		{
			return clientLogger_;
		}

	private:
		static std::shared_ptr<spdlog::logger> coreLogger_;
		static std::shared_ptr<spdlog::logger> clientLogger_;
	};
}

// Core log macros
#define GOKNAR_CORE_TRACE(...)			::Goknar::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GOKNAR_CORE_INFO(...)			::Goknar::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GOKNAR_CORE_WARN(...)			::Goknar::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GOKNAR_CORE_ERROR(...)			::Goknar::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GOKNAR_CORE_FATAL(...)			::Goknar::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define GOKNAR_TRACE(...)			::Goknar::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GOKNAR_INFO(...)			::Goknar::Log::GetClientLogger()->info(__VA_ARGS__)
#define GOKNAR_WARN(...)			::Goknar::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GOKNAR_ERROR(...)			::Goknar::Log::GetClientLogger()->error(__VA_ARGS__)
#define GOKNAR_FATAL(...)			::Goknar::Log::GetClientLogger()->fatal(__VA_ARGS__)
