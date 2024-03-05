#include "Core.h"

#ifdef CONTENT_DIR
const std::string ContentDir = CONTENT_DIR;
#else
const std::string ContentDir = "";
#endif

#if defined(GOKNAR_BUILD_DEBUG)
    #ifdef ENGINE_CONTENT_DIR
    const std::string EngineContentDir = ENGINE_CONTENT_DIR;
    #else
    const std::string EngineContentDir = "";
    #endif
#endif