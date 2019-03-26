#pragma once

#include "Core.h"

#include <memory>

#include "spdlog/spdlog.h"

namespace GameEngine {
	class GAMEENGINE_API Log
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
#define ENGINE_CORE_TRACE(...)			::GameEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ENGINE_CORE_INFO(...)			::GameEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ENGINE_CORE_WARN(...)			::GameEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ENGINE_CORE_ERROR(...)			::GameEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ENGINE_CORE_FATAL(...)			::GameEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define ENGINE_TRACE(...)			::GameEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...)			::GameEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define ENGINE_WARN(...)			::GameEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...)			::GameEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define ENGINE_FATAL(...)			::GameEngine::Log::GetClientLogger()->fatal(__VA_ARGS__)
