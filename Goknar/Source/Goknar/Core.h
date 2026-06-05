#ifndef __CORE_H__
#define __CORE_H__

#include <string>

#ifdef _WIN32
// Disable warnings on extern before template instantiation
#pragma warning (disable : 4251)
#endif

#if defined(ENGINE_CONTENT_DIR)
	extern const std::string EngineContentDir;
#endif

extern std::string ProjectDir;
extern std::string ContentDir;

#ifdef GOKNAR_PLATFORM_WINDOWS
	#define DEBUG_BREAK() __debugbreak()
	#ifdef GOKNAR_BUILD_DLL
		#define GOKNAR_API __declspec(dllexport)
	#else
		#define GOKNAR_API __declspec(dllimport)
	#endif

#elif defined(GOKNAR_PLATFORM_UNIX)
	#include <csignal>
	#define DEBUG_BREAK() std::raise(SIGINT)
	#ifndef GOKNAR_BUILD_DLL
		#define GOKNAR_API __declspec(dllexport)
	#else
		#define GOKNAR_API __attribute__((visibility("default")))
	#endif

#else
	#error Game Engine only supports Windows and Unix!
#endif

#define TUPLE_TAIL(...) __VA_ARGS__

#define UNLIKELY(x) (x)

#define BUFFER_OFFSET(index) ((const void *) index)

#define VA_ARGS(...) , ##__VA_ARGS__

GOKNAR_API void GoknarCheckGraphicsAPIError(const char* errorMessage);

#define EXIT_ON_GRAPHICS_API_ERROR(errorMessage) GoknarCheckGraphicsAPIError(errorMessage)

#define REGISTER_CLASS(className) \
    DynamicObjectFactory::GetInstance()->RegisterObjectClass<className>(#className); \

#endif
