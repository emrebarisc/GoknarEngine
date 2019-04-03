#ifndef __LOG_H__
#define __LOG_H__

#include "Core.h"

#include <memory>

#include "spdlog/spdlog.h"

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

// Core log macros
#define GOKNAR_CORE_TRACE(...)			::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GOKNAR_CORE_INFO(...)			::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GOKNAR_CORE_WARN(...)			::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GOKNAR_CORE_ERROR(...)			::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GOKNAR_CORE_FATAL(...)			::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define GOKNAR_TRACE(...)			::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GOKNAR_INFO(...)			::Log::GetClientLogger()->info(__VA_ARGS__)
#define GOKNAR_WARN(...)			::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GOKNAR_ERROR(...)			::Log::GetClientLogger()->error(__VA_ARGS__)
#define GOKNAR_FATAL(...)			::Log::GetClientLogger()->fatal(__VA_ARGS__)

#endif