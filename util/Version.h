// -*- C++ -*-
#ifndef _Version_h_
#define _Version_h_

#include <string>
#include <map>

#include "Export.h"

FO_COMMON_API const std::string& FreeOrionVersionString();

FO_COMMON_API std::map<std::string, std::string> DependencyVersions();

FO_COMMON_API void LogDependencyVersions();

#endif
