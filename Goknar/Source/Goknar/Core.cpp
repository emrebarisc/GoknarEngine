#include "Core.h"

#ifdef PROJECT_DIR
std::string ProjectDir = PROJECT_DIR;
#else
std::string ProjectDir = "";
#endif

std::string ContentDir = "";

#if defined(GOKNAR_BUILD_DEBUG)
    #ifdef ENGINE_CONTENT_DIR
    const std::string EngineContentDir = ENGINE_CONTENT_DIR;
    #endif
#endif