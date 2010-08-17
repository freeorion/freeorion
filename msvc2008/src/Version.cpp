#include "../../util/Version.h"

namespace {
    static const std::string retval = "v0.3.15 [SVN 3736]";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
