#include "../../util/Version.h"

namespace {
    static const std::string retval = "Pre-v0.3.13 [Development Build]";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
