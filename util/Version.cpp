#include "Version.h"

namespace {
    const std::string version_string = "v0.3.1-RC4" // no semicolon here
#ifdef FREEORION_REVISION
    FREEORION_REVISION
#endif
    ;
}

const std::string& FreeOrionVersionString()
{
    return version_string;
}
