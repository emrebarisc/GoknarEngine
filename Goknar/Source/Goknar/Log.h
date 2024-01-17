#ifndef __LOG_H__
#define __LOG_H__

#include "Core.h"

#include <memory>

#include "spdlog/spdlog.h"

enum class GOKNAR_API LogType : unsigned char 
{
	None = 0,
	Trace,
	Info,
	Warn,
	Error,
	Fatal
};

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

	template<typename... Arguments>
	inline static void LogCore(LogType type, const Arguments&... args)
	{
		coreLogger_->log(LogTypeToSpdlogType(type), args...);
	}

	template<typename... Arguments>
	inline static void LogClient(LogType type, const Arguments&... args)
	{
		clientLogger_->log(LogTypeToSpdlogType(type), args...);
	}

private:
	static spdlog::level::level_enum LogTypeToSpdlogType(LogType type)
	{
		spdlog::level::level_enum level = spdlog::level::level_enum::off;

		switch (type)
		{
		case LogType::None:
			break;
		case LogType::Trace:
			level = spdlog::level::level_enum::trace;
			break;
		case LogType::Info:
			level = spdlog::level::level_enum::info;
			break;
		case LogType::Warn:
			level = spdlog::level::level_enum::warn;
			break;
		case LogType::Error:
			level = spdlog::level::level_enum::err;
			break;
		case LogType::Fatal:
			level = spdlog::level::level_enum::critical;
			break;
		default:
			break;
		}

		return level;
	}

	static std::shared_ptr<spdlog::logger> coreLogger_;
	static std::shared_ptr<spdlog::logger> clientLogger_;
};

// Core log macros
#define GOKNAR_CORE_TRACE(...)			::Log::LogCore(LogType::Trace, ##__VA_ARGS__)
#define GOKNAR_CORE_INFO(...)			::Log::LogCore(LogType::Info, ##__VA_ARGS__)
#define GOKNAR_CORE_WARN(...)			::Log::LogCore(LogType::Warn, ##__VA_ARGS__)
#define GOKNAR_CORE_ERROR(...)			::Log::LogCore(LogType::Error, ##__VA_ARGS__)
#define GOKNAR_CORE_FATAL(...)			::Log::LogCore(LogType::Fatal, ##__VA_ARGS__)

// Client log macros
#define GOKNAR_TRACE(...)			::Log::LogClient(LogType::Trace, ##__VA_ARGS__)
#define GOKNAR_INFO(...)			::Log::LogClient(LogType::Info, ##__VA_ARGS__)
#define GOKNAR_WARN(...)			::Log::LogClient(LogType::Warn, ##__VA_ARGS__)
#define GOKNAR_ERROR(...)			::Log::LogClient(LogType::Error, ##__VA_ARGS__)
#define GOKNAR_FATAL(...)			::Log::LogClient(LogType::Fatal, ##__VA_ARGS__)

#endif