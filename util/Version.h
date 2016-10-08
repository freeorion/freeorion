#ifndef _Version_h_
#define _Version_h_

/** @file
 * @brief  Declares free functions to access the application and dependencies
 *      versions.
 */

#include <string>
#include <map>

#include "Export.h"

/** @brief  Returns the version string of FreeOrion.
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
