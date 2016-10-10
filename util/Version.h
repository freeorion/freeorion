#ifndef _Version_h_
#define _Version_h_

/** @file
 * @brief  Declares free functions to access the application and dependency
 *      versions.
 */

#include <string>
#include <map>

#include "Export.h"

/** @brief  Returns the version string of FreeOrion.
 *
 * The version strings consists of:
 *
 * * the VERSION number composed from @e MAJOR.MINOR.PATCH numbers. An
 *      optional '+' suffix indicates an in between test releases (think
 *      0.4.6 + some more).  Release candidates are suffixed with rcNUMBER.
 * * the BRANCH, which represents the Git branch used to create this build.
 * * the DATE as ISO 8601 date format.
 * * the SHORT_COMMIT_HASH, which is the shortes Git commit hash, which
 *   identifies the commit this build was created from.
 * * the BUILD_SYSTEM, which can be one of "CMake", "MSVC" or "XCode".
 *
 * @return  The version string with the format
 *      <em>VERSION BRANCH_NAME [build DATE.SHORT_COMMIT_HASH] BUILD_SYSTEM</em>
 */
FO_COMMON_API const std::string& FreeOrionVersionString();

/** @brief  Returns a map of dependency versions.
 *
 * @return  A map with the dependency versions.  The key represents the
 *      dependency name, the value corresponding dependency version string.
 *
 * @bug  The current implementation doesn't expose all dependencies of all
 *      application components.  For example the human client has a dependency
 *      on Vorbis but because this function is part of common it isn't added to
 *      the dependency map.
 */
FO_COMMON_API std::map<std::string, std::string> DependencyVersions();

/** @brief  Log the map returned by DependencyVersions() into the @e debug log
 *      channel.
 *
 * @bug  The @e debug log channel isn't the right place to log this but we don't
 *      have any @e info log channel currently.
 */
FO_COMMON_API void LogDependencyVersions();

#endif  // _Version_h_
