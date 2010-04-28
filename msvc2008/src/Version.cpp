#include "../../util/Version.h"

namespace {
    static const std::string retval = "v0.3.14 [Rev 3559]";
}

const std::string& FreeOrionVersionString()
{
    return retval;
}
