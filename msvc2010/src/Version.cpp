#include "../../util/Version.h"

namespace {
    static const std::string retval = "post-v0.3.15 [SVN 4022] MSVC 2010";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
