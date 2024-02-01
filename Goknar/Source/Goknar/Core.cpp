#include "Core.h"

#ifdef CONTENT_DIR
const std::string ContentDir = CONTENT_DIR;
#else
const std::string ContentDir = "";
#endif

#ifdef GOKNAR_DEBUG
    #ifdef ENGINE_CONTENT_DIR
    const std::string EngineContentDir = ENGINE_CONTENT_DIR;
    #else
    const std::string EngineContentDir = "";
    #endif
#endif