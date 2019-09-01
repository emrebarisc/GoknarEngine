#ifndef __CORE_H__
#define __CORE_H__

#ifdef _WIN32
// Disable warnings on extern before template instantiation
#pragma warning (disable : 4251)
#endif

#ifdef GOKNAR_PLATFORM_WINDOWS
	#ifdef GOKNAR_BUILD_DLL
		#define GOKNAR_API __declspec(dllexport)
	#else
		#define GOKNAR_API __declspec(dllimport)
	#endif

#else
#error Game Engine only supports Windows!
#endif

#ifdef GOKNAR_ENABLE_ASSERTS
	#define GOKNAR_ASSERT(x, ...) \
	{ \
		if(!x) \
		{ \
			GOKNAR_ERROR("Assertion Failed: {0}", __VA_ARGS__); /*__debugbreak();*/ \
		} \
	}
	#define GOKNAR_CORE_ASSERT(x, ...) \
	{ \
		if (!x)\
		{ \
			GOKNAR_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); /*__debugbreak();*/ \
		} \
	}
#else
	#define GOKNAR_ASSERT(x, ...) ENGINE_WARN("Assertion is not enabled!");
	#define GOKNAR_CORE_ASSERT(x, ...) ENGINE_WARN("Assertion is not enabled!");
#endif

#define BUFFER_OFFSET(index) ((const void *) index)

//#define EXIT_ON_GL_ERROR(errorMessage) \
//{ \
//	const GLenum errorValue = glGetError(); \
//	if(errorValue != GL_NO_ERROR) \
//	{ \
//		fprintf(stderr, "%s: %s\n", errorMessage, gluErrorString(errorValue)); \
//		exit(EXIT_FAILURE); \
//	} \
//}

#endif