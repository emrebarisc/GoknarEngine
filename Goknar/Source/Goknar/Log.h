#ifndef __LOG_H__
#define __LOG_H__

#include "Core.h"
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <mutex>
#include <cstdio>

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
	static void Shutdown();

	template<typename... Arguments>
	inline static void LogCore(LogType type, const char* format, Arguments&&... args)
	{
		OutputLog("ENGINE", type, format, std::forward<Arguments>(args)...);
	}

	inline static void LogCore(LogType type, const std::string& message)
	{
		OutputLogImpl("ENGINE", type, message);
	}

	inline static void LogCore(LogType type)
	{
		OutputLogImpl("ENGINE", type, "");
	}

	template<typename... Arguments>
	inline static void LogClient(LogType type, const char* format, Arguments&&... args)
	{
		OutputLog("APPLICATION", type, format, std::forward<Arguments>(args)...);
	}

	inline static void LogClient(LogType type, const std::string& message)
	{
		OutputLogImpl("APPLICATION", type, message);
	}

	inline static void LogClient(LogType type)
	{
		OutputLogImpl("APPLICATION", type, "");
	}

private:
	template<typename T>
	static auto FormatArg(T&& arg)
	{
		if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
			return arg.c_str();
		}
		else {
			return std::forward<T>(arg);
		}
	}

	inline static const char* FormatArg(const std::string& arg)
	{
		return arg.c_str();
	}

	static void OutputLogImpl(const std::string& name, LogType type, const std::string& message);

	template<typename... Args>
	static void OutputLog(const std::string& name, LogType type, const char* format, Args&&... args)
	{
		char stackBuffer[1024];

		int result = std::snprintf(stackBuffer, sizeof(stackBuffer), format, FormatArg(std::forward<Args>(args))...);

		if (result >= 0 && result < sizeof(stackBuffer))
		{
			OutputLogImpl(name, type, std::string(stackBuffer, result));
		}
		else if (result >= sizeof(stackBuffer))
		{
			size_t requiredSize = static_cast<size_t>(result) + 1;
			std::string heapBuffer(requiredSize, '\0');

			// We must format again if we had to allocate heap memory
			std::snprintf(&heapBuffer[0], requiredSize, format, FormatArg(std::forward<Args>(args))...);
			heapBuffer.pop_back();

			OutputLogImpl(name, type, heapBuffer);
		}
		else
		{
			OutputLogImpl(name, type, "Log formatting error!");
		}
	}

	static std::string GetTimestamp();
	static const char* GetLevelString(LogType type);
	static const char* GetColorCode(LogType type);
	static const char* ResetColorCode();

	static std::mutex logMutex_;
	static std::ofstream fileSink_;
};

#define GOKNAR_CORE_TRACE(...)			::Log::LogCore(LogType::Trace, ##__VA_ARGS__)
#define GOKNAR_CORE_INFO(...)			::Log::LogCore(LogType::Info, ##__VA_ARGS__)
#define GOKNAR_CORE_WARN(...)			::Log::LogCore(LogType::Warn, ##__VA_ARGS__)
#define GOKNAR_CORE_ERROR(...)			::Log::LogCore(LogType::Error, ##__VA_ARGS__)
#define GOKNAR_CORE_FATAL(...)			::Log::LogCore(LogType::Fatal, ##__VA_ARGS__)

#define GOKNAR_TRACE(...)			::Log::LogClient(LogType::Trace, ##__VA_ARGS__)
#define GOKNAR_INFO(...)			::Log::LogClient(LogType::Info, ##__VA_ARGS__)
#define GOKNAR_WARN(...)			::Log::LogClient(LogType::Warn, ##__VA_ARGS__)
#define GOKNAR_ERROR(...)			::Log::LogClient(LogType::Error, ##__VA_ARGS__)
#define GOKNAR_FATAL(...)			::Log::LogClient(LogType::Fatal, ##__VA_ARGS__)

#endif