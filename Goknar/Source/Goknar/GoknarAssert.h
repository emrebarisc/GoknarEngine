#ifndef __GOKNARASSERT_H__
#define __GOKNARASSERT_H__

#include "Log.h"

#ifdef GOKNAR_ENABLE_ASSERTS
	#include "Log.h"
	#define GOKNAR_CHECK(x, ...) { if(UNLIKELY(!(x))){ GOKNAR_ERROR("Assertion Failed: {0}", TUPLE_TAIL(__VA_ARGS__)); DEBUG_BREAK(); } }
	#define GOKNAR_ASSERT(x, ...) { if(UNLIKELY(!(x))){ GOKNAR_ERROR("Assertion Failed: {0}", TUPLE_TAIL(__VA_ARGS__)); DEBUG_BREAK(); exit(EXIT_FAILURE); } }
	#define GOKNAR_CORE_ASSERT(x, ...) { if(UNLIKELY(!(x))){ GOKNAR_CORE_ERROR("Assertion Failed: {0}", TUPLE_TAIL(__VA_ARGS__)); DEBUG_BREAK(); exit(EXIT_FAILURE); } }
#else
	#define GOKNAR_ASSERT(x, ...)
	#define GOKNAR_CORE_ASSERT(x, ...)
#endif

#endif