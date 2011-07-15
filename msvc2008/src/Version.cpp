#include "../../util/Version.h"

namespace {
    static const std::string retval = "v0.3.16 [SVN 4046] MSVC 2008";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
