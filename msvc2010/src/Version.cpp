#include "../../util/Version.h"

namespace {
    static const std::string retval = "v0.4.1+ Non-Latin Path Test [SVN 5103] MSVC 2010";
}

const std::string& FreeOrionVersionString()
{ return retval; }
