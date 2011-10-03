#include "../../util/Version.h"

namespace {
    static const std::string retval = "v0.3.17+ [SVN 4342] MSVC 2010";
}

const std::string& FreeOrionVersionString()
{ return retval; }
