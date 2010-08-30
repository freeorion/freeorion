#include "../../util/Version.h"

namespace {
    static const std::string retval = "post-v0.3.15 [SVN 3783]";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
