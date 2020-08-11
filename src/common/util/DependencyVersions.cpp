#include "Version.h"

#include "Logger.h"

#include <sstream>

#include <zlib.h>
#include <boost/version.hpp>

#if defined(FREEORION_BUILD_SERVER) || defined(FREEORION_BUILD_AI)
#   include <patchlevel.h>
#endif

#if defined(FREEORION_BUILD_HUMAN)
#   include <SDL2/SDL_version.h>
#   include <png.h>
#   include <ft2build.h>
#   include FT_FREETYPE_H
#   include <vorbis/codec.h>
#endif


/** @file
 * @brief  Implement free functions to access the dependency versions.
 */

namespace {
#if defined(FREEORION_BUILD_HUMAN)
    std::string SDLVersionString() {
        std::stringstream ss;
        SDL_version version;

        SDL_GetVersion(&version);
        ss << static_cast<int>(version.major) << "."
           << static_cast<int>(version.minor) << "."
           << static_cast<int>(version.patch);
        return ss.str();
    }

    std::string FreeTypeVersionString() {
        std::stringstream ss;
        ss << FREETYPE_MAJOR << "." << FREETYPE_MINOR << "." << FREETYPE_PATCH;
        return ss.str();
    }

    // Ogg does not supply a version string in the header or from a function as far as I can tell
    //std::string OggVersionString()
    //{ return ""; }

    std::string VorbisVersionString() {
        const char* retval = vorbis_version_string();
        return retval;
    }

    std::string PNGVersionString()
    { return PNG_LIBPNG_VER_STRING; }
#endif
}

std::map<std::string, std::string> DependencyVersions() {
    std::map<std::string, std::string> retval;
    // fill with all findable version strings...
    retval["zlib"] =        ZLIB_VERSION;
    retval["Boost"] =       BOOST_LIB_VERSION;

#if defined(FREEORION_BUILD_SERVER) || defined(FREEORION_BUILD_AI)
    retval["Python"] =      PY_VERSION;
#endif

#if defined(FREEORION_BUILD_HUMAN)
    retval["SDL"] =         SDLVersionString();
    retval["FreeType"] =    FreeTypeVersionString();
    retval["PNG"] =         PNGVersionString();
    retval["libvorbis"] =   VorbisVersionString();
#endif

    return retval;
}

void LogDependencyVersions() {
    InfoLogger() << "Dependency versions from headers:";
    for (const auto& version : DependencyVersions()) {
        if (!version.second.empty())
            InfoLogger() << version.first << ": " << version.second;
    }
}
