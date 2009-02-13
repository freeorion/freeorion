#include "../../util/Version.h"

const std::string& FreeOrionVersionString()
{
    static const std::string retval = "Post-v0.3.12 [Development Build]";
    return retval;
}
