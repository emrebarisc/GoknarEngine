#include "pch.h"

#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Log::coreLogger_;
std::shared_ptr<spdlog::logger> Log::clientLogger_;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	coreLogger_ = spdlog::stdout_color_mt("ENGINE");
	coreLogger_->set_level(spdlog::level::trace);

	clientLogger_ = spdlog::stdout_color_mt("APPLICATION");
	clientLogger_->set_level(spdlog::level::trace);

	GOKNAR_CORE_WARN("Initialized Game Engine Log.");
}