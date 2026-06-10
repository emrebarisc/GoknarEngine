#pragma once

#ifdef GOKNAR_DEBUG

#include "Goknar/Profiling/ProfileScope.h"

#if defined(_MSC_VER)
	#define GOKNAR_PROFILE_FUNCTION_NAME __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
	#define GOKNAR_PROFILE_FUNCTION_NAME __PRETTY_FUNCTION__
#else
	#define GOKNAR_PROFILE_FUNCTION_NAME __func__
#endif

#define GOKNAR_PROFILE_CONCAT_IMPL(a, b) a##b
#define GOKNAR_PROFILE_CONCAT(a, b) GOKNAR_PROFILE_CONCAT_IMPL(a, b)

#define GOKNAR_PROFILE_FUNCTION() ::Goknar::Debug::ProfileScope GOKNAR_PROFILE_CONCAT(goknarProfileFunctionScope_, __LINE__)(GOKNAR_PROFILE_FUNCTION_NAME, __FILE__, static_cast<std::uint32_t>(__LINE__))
#define GOKNAR_PROFILE_SCOPE(name) ::Goknar::Debug::ProfileScope GOKNAR_PROFILE_CONCAT(goknarProfileScope_, __LINE__)(name, __FILE__, static_cast<std::uint32_t>(__LINE__))
#define GOKNAR_PROFILE_FRAME(name) ::Goknar::Debug::ProfileFrameScope GOKNAR_PROFILE_CONCAT(goknarProfileFrameScope_, __LINE__)(name, __FILE__, static_cast<std::uint32_t>(__LINE__))

#else

#define GOKNAR_PROFILE_FUNCTION() ((void)0)
#define GOKNAR_PROFILE_SCOPE(name) ((void)0)
#define GOKNAR_PROFILE_FRAME(name) ((void)0)

#endif
