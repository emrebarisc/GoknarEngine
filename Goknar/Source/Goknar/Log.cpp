#include "pch.h"
#include "Log.h"

std::mutex Log::logMutex_;
std::ofstream Log::fileSink_;

void Log::Init()
{
#ifndef GOKNAR_ENABLE_LOGGING
	return;
#else
	std::lock_guard<std::mutex> lock(logMutex_);

	fileSink_.open("Goknar.log", std::ios::out | std::ios::app);

	if (fileSink_.is_open()) {
		fileSink_ << "\n---------- Log Session Started: " << GetTimestamp() << " ----------\n";
	}
#endif
}

void Log::Shutdown()
{
#ifndef GOKNAR_ENABLE_LOGGING
	return;
#else
	std::lock_guard<std::mutex> lock(logMutex_);
	if (fileSink_.is_open()) {
		fileSink_ << "---------- Log Session Ended ----------\n";
		fileSink_.close();
	}
#endif
}

void Log::OutputLogImpl(const std::string& name, LogType type, const std::string& message)
{
#ifndef GOKNAR_ENABLE_LOGGING
	(void)name;
	(void)type;
	(void)message;
	return;
#else
	if (type == LogType::None) return;

	std::string timestamp = GetTimestamp();
	const char* levelStr = GetLevelString(type);

	std::lock_guard<std::mutex> lock(logMutex_);

	std::cout << GetColorCode(type)
		<< "[" << timestamp << "] "
		<< name << ": "
		<< message
		<< ResetColorCode() << std::endl;

	if (fileSink_.is_open()) {
		fileSink_ << "[" << timestamp << "] [" << levelStr << "] "
			<< name << ": " << message << std::endl;
	}
#endif
}

std::string Log::GetTimestamp()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
	return ss.str();
}

const char* Log::GetLevelString(LogType type)
{
	switch (type)
	{
	case LogType::Trace: return "TRACE";
	case LogType::Info:  return "INFO";
	case LogType::Warn:  return "WARN";
	case LogType::Error: return "ERROR";
	case LogType::Fatal: return "FATAL";
	default:             return "UNKNOWN";
	}
}

const char* Log::GetColorCode(LogType type)
{
	switch (type)
	{
	case LogType::Trace: return "\033[94m";
	case LogType::Info:  return "\033[92m";
	case LogType::Warn:  return "\033[93m";
	case LogType::Error: return "\033[91m";
	case LogType::Fatal: return "\033[41m";
	default:             return "\033[0m";
	}
}

const char* Log::ResetColorCode() { return "\033[0m"; }
