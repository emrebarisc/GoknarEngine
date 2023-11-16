#ifndef __GOKNARASSERT_H__
#define __GOKNARASSERT_H__

#ifdef GOKNAR_ENABLE_ASSERTS
	#include "Log.h"

	#ifdef GOKNAR_PLATFORM_WINDOWS
		#define GOKNAR_CHECK(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("Assertion Failed: {0}", TUPLE_TAIL(__VA_ARGS__)); DEBUG_BREAK(); } }
		#define GOKNAR_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("Assertion Failed: {0}", TUPLE_TAIL(__VA_ARGS__)); DEBUG_BREAK(); exit(EXIT_FAILURE); } }
		#define GOKNAR_CORE_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_CORE_ERROR("Assertion Failed: {0}", TUPLE_TAIL(__VA_ARGS__)); DEBUG_BREAK(); exit(EXIT_FAILURE); } }
	#else
		#define GOKNAR_CHECK(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("Assertion Failed: {0}", ##__VA_ARGS__); DEBUG_BREAK(); } }
		#define GOKNAR_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("Assertion Failed: {0}", ##__VA_ARGS__); DEBUG_BREAK(); exit(EXIT_FAILURE); } }
		#define GOKNAR_CORE_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_CORE_ERROR("Assertion Failed: {0}", ##__VA_ARGS__); DEBUG_BREAK(); exit(EXIT_FAILURE); } }
	#endif

#else
	#define GOKNAR_CHECK(x, ...)
	#define GOKNAR_ASSERT(x, ...)
	#define GOKNAR_CORE_ASSERT(x, ...)
#endif

#endif