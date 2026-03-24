#ifndef __GOKNARASSERT_H__
#define __GOKNARASSERT_H__

#ifdef GOKNAR_ENABLE_ASSERTS
#include "Log.h"
#ifdef GOKNAR_PLATFORM_WINDOWS
#define GOKNAR_CHECK(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("Check Failed!"); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_ERROR(__VA_ARGS__); } DEBUG_BREAK(); } }
#define GOKNAR_CORE_CHECK(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_CORE_ERROR("Check Failed!"); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_CORE_ERROR(__VA_ARGS__); } DEBUG_BREAK(); } }
#define GOKNAR_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("Assertion Failed!"); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_ERROR(__VA_ARGS__); } DEBUG_BREAK(); exit(EXIT_FAILURE); } }
#define GOKNAR_CORE_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_CORE_ERROR("Assertion Failed!"); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_CORE_ERROR(__VA_ARGS__); } DEBUG_BREAK(); exit(EXIT_FAILURE); } }
#else
#define GOKNAR_CHECK(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("%s:%d | Check Failed!", __FILE__, __LINE__); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_ERROR(__VA_ARGS__); } DEBUG_BREAK(); } }
#define GOKNAR_CORE_CHECK(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_CORE_ERROR("%s:%d | Check Failed!", __FILE__, __LINE__); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_CORE_ERROR(__VA_ARGS__); } DEBUG_BREAK(); } }
#define GOKNAR_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_ERROR("%s:%d | Assertion Failed!", __FILE__, __LINE__); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_ERROR(__VA_ARGS__); } DEBUG_BREAK(); exit(EXIT_FAILURE); } }
#define GOKNAR_CORE_ASSERT(condition, ...) { if(UNLIKELY(!(condition))){ GOKNAR_CORE_ERROR("%s:%d | Assertion Failed!", __FILE__, __LINE__); if constexpr (sizeof("" #__VA_ARGS__) > 1) { GOKNAR_CORE_ERROR(__VA_ARGS__); } DEBUG_BREAK(); exit(EXIT_FAILURE); } }
#endif

#else
#define GOKNAR_CHECK(x, ...)
#define GOKNAR_CORE_CHECK(x, ...)
#define GOKNAR_ASSERT(x, ...)
#define GOKNAR_CORE_ASSERT(x, ...)
#endif

#endif