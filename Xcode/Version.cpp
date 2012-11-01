#include "../util/Version.h"

namespace {
    static const std::string retval = "v0.4.1+ [SVN 5368] Xcode 3";
}

const std::string& FreeOrionVersionString()
{ return retval; }
