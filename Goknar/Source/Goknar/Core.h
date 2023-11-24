#ifndef __CORE_H__
#define __CORE_H__

#include <string>

#ifdef _WIN32
// Disable warnings on extern before template instantiation
#pragma warning (disable : 4251)
#endif

#ifdef CONTENT_DIR
	extern const std::string ContentDir;
#else
	extern const std::string ContentDir;
#endif

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

#define EXIT_ON_GL_ERROR(errorMessage) \
{ \
	const GLenum errorValue = glGetError(); \
	if(errorValue != GL_NO_ERROR) \
	{ \
		std::string errorString = "NO_ERROR";\
		switch (errorValue)\
		{\
		case GL_INVALID_ENUM:					errorString = "INVALID_ENUM"; break;\
		case GL_INVALID_VALUE:					errorString = "INVALID_VALUE"; break;\
		case GL_INVALID_OPERATION:				errorString = "INVALID_OPERATION"; break;\
		case GL_STACK_OVERFLOW:					errorString = "STACK_OVERFLOW"; break;\
		case GL_STACK_UNDERFLOW:				errorString = "STACK_UNDERFLOW"; break;\
		case GL_OUT_OF_MEMORY:					errorString = "OUT_OF_MEMORY"; break;\
		case GL_INVALID_FRAMEBUFFER_OPERATION:	errorString = "INVALID_FRAMEBUFFER_OPERATION"; break;\
		default:								errorString = "UNKNOWN";\
		}\
		GOKNAR_CORE_FATAL("{0}: Error Value: {1}\n", errorMessage, errorString); \
		DEBUG_BREAK(); \
		exit(EXIT_FAILURE); \
	} \
}

#endif