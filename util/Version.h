#ifndef _Version_h_
#define _Version_h_

//! @file
//!     Declares free functions to access the application and dependency
//!     versions.

#include <string>
#include <map>

#include "Export.h"

//! Returns the version string of FreeOrion.
//!
//! The version strings consists of:
//!
//! * the VERSION number composed from @e MAJOR.MINOR.PATCH numbers. An
//!      optional '+' suffix indicates an in between test releases (think
//!      0.4.6 + some more).  Release candidates are suffixed with rcNUMBER.
//! * the BRANCH, which represents the Git branch used to create this build.
//! * the DATE as ISO 8601 date format.
//! * the SHORT_COMMIT_HASH, which is the shortes Git commit hash, which
//!   identifies the commit this build was created from.
//! * the BUILD_SYSTEM, which can be one of "CMake" or "MSVC"
//!
//! @return
//!     The version string with the format
//!     <em>VERSION BRANCH_NAME [build DATE.SHORT_COMMIT_HASH] BUILD_SYSTEM</em>
FO_COMMON_API const std::string& FreeOrionVersionString();

//! Returns a map of dependency versions.
//!
//! @return
//!     A map with the dependency versions.  The key represents the dependency
//!     name, the value corresponding dependency version string.
//!
//! @note
//!     This function returns different dependencies, depending on which game
//!     executable it was called from.
FO_COMMON_API std::map<std::string, std::string> DependencyVersions();

//! Log the map returned by DependencyVersions() into the @e info log channel.
void LogDependencyVersions();

#endif  // _Version_h_
