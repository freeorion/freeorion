#include "Version.h"

#include "Logger.h"

#include <SDL2/SDL_version.h>
#include <zlib.h>
#include <png.h>
#include <python2.7/patchlevel.h>
//#include <vorbis/codec.h>

#include <ft2build.h>
#include FT_FREETYPE_H

//#include <boost/lexical_cast.hpp>
#include <boost/version.hpp>

#include <sstream>

namespace {
    std::string BoostVersionString()
    { return BOOST_LIB_VERSION; }

    std::string SDLVersionString() {
        std::stringstream ss;
        ss << SDL_MAJOR_VERSION << "." << SDL_MINOR_VERSION << "." << SDL_PATCHLEVEL;
        return ss.str();
    }

    std::string ZLibVersionString()
    { return ZLIB_VERSION; }

    std::string FreeTypeVersionString() {
        std::stringstream ss;
        ss << FREETYPE_MAJOR << "." << FREETYPE_MINOR << "." << FREETYPE_PATCH;
        return ss.str();
    }

    // Ogg does not supply a version string in the header or from a function as far as I can tell
    //std::string OggVersionString()
    //{ return ""; }

    // Vorbis requires calling a function in the shared library to get the
    // version string, but I don't want to add the vorbis dependency to anything
    // that uses the Common library.
    //std::string VorbisVersionString() {
    //    const char* retval = vorbis_version_string();
    //    return retval;
    //}

    std::string PythonVersionString()
    { return PY_VERSION; }

    std::string PNGVersionString()
    { return PNG_LIBPNG_VER_STRING; }
}

std::map<std::string, std::string> DependencyVersions() {
    std::map<std::string, std::string> retval;
    // fill with all findable version strings...
    retval["Boost"] =       BoostVersionString();
    retval["SDL"] =         SDLVersionString();
    retval["zlib"] =        ZLibVersionString();
    retval["FreeType"] =    FreeTypeVersionString();
    retval["Python"] =      PythonVersionString();
    retval["PNG"] =         PNGVersionString();

    return retval;
}

void LogDependencyVersions() {
    std::map<std::string, std::string> vers = DependencyVersions();
    DebugLogger() << "Dependency versions from headers:";
    for (std::map<std::string, std::string>::const_iterator it = vers.begin();
         it != vers.end(); ++it)
    { DebugLogger() << it->first << ": " << it->second; }
}
