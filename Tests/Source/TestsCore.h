#include "Goknar/GoknarAssert.h"
#include "Goknar/Log.h"

#define TEST_ASSERT(condition, message) \
    if (!(condition)) \
    { \
        GOKNAR_CORE_CHECK(false, message); \
        throw("Test failed."); \
    } \
    else \
    {}
