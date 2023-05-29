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
	#define DEBUG_BREAK() 
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

#define EXIT_ON_GL_ERROR(errorMessage) \
{ \
	const GLenum errorValue = glGetError(); \
	if(errorValue != GL_NO_ERROR) \
	{ \
		GOKNAR_CORE_FATAL("{0}: Error Value: {1}\n", errorMessage, errorValue/*gluErrorString(errorValue));*/); \
		DEBUG_BREAK(); \
		exit(EXIT_FAILURE); \
	} \
}

#endif