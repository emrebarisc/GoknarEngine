#include "Core.h"

#ifdef CONTENT_DIR
const std::string ContentDir = CONTENT_DIR;
#else
const std::string ContentDir = "";
#endif