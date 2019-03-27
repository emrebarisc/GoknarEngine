#pragma once

#ifdef GAMEENGINE_PLATFORM_WINDOWS
	#ifdef GAMEENGINE_BUILD_DLL
		#define GAMEENGINE_API __declspec(dllexport)
	#else
		#define GAMEENGINE_API __declspec(dllimport)
	#endif

#else
#error Game Engine only supports Windows!
#endif

#ifdef ENGINE_ENABLE_ASSERTS
	#define ENGINE_ASSERT(x, ...) \
	{ \
		if(!x) \
		{ \
			ENGINE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); \
		} \
	}
	#define ENGINE_CORE_ASSERT(x, ...) \
	{ \
		if (!x)\
		{ \
			ENGINE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); \
		} \
	}
#else
	#define ENGINE_ASSERT(x, ...) ENGINE_WARN("Assertion is not enabled!");
	#define ENGINE_CORE_ASSERT(x, ...) ENGINE_WARN("Assertion is not enabled!");
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