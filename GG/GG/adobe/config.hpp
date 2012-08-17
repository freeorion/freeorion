/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_CONFIG_HPP
#define ADOBE_CONFIG_HPP

/*************************************************************************************************/

#include <boost/config.hpp>

/*************************************************************************************************/

/*
    Caution:
        This is the only ASL header that is guarenteed to change with every release. Including
        this header will cause a recompile every time a new ASL version is released.

        ADOBE_VERSION % 100 is the sub-minor version
        ADOBE_VERSION / 100 % 1000 is the minor version
        ADOBE_VERSION / 100000 is the major version
*/

#define ADOBE_VERSION_MAJOR     1
#define ADOBE_VERSION_MINOR     0
#define ADOBE_VERSION_SUBMINOR  41

#define ADOBE_VERSION (ADOBE_VERSION_MAJOR * 100000 + ADOBE_VERSION_MINOR * 100 + ADOBE_VERSION_SUBMINOR)

/*************************************************************************************************/

#define ADOBE_IS_DEPRECATED_ERROR(version) \
    ((ADOBE_VERSION - version) > 0 || defined(ADOBE_NO_DEPRECATED))

/*************************************************************************************************/

// Big thanks to Boost here for doing a majority of the work for us.

#if defined(__CYGWIN__)
    // Cygwin is not Win32
    #define ADOBE_PLATFORM_CYGWIN   1

#elif defined(BOOST_WINDOWS) || defined(__MINGW32__)
    // Win32
    #define ADOBE_PLATFORM_WIN      1

#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    // MacOS
    #define ADOBE_PLATFORM_MAC      1

#elif defined(__BEOS__)
    // BeOS
    #define ADOBE_PLATFORM_BEOS     1

#elif defined(__IBMCPP__)
    // IBM
    #define ADOBE_PLATFORM_AIX      1

#elif defined(__amigaos__)
    // AmigaOS
    #define ADOBE_PLATFORM_AMIGA    1

#elif defined(sun) || defined(__sun)
    // Solaris
    #define ADOBE_PLATFORM_SOLARIS  1

#elif defined(__sgi)
    // SGI Irix
    #define ADOBE_PLATFORM_IRIX     1

#elif defined(__hpux)
    // HP Unix
    #define ADOBE_PLATFORM_HPUX     1

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // BSD
    #define ADOBE_PLATFORM_BSD      1

#elif defined(linux) || defined(__linux) || defined(__linux__)
    // Linux
    #define ADOBE_PLATFORM_LINUX    1

#elif defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE)
    // Generic Unix
    #define ADOBE_PLATFORM_UNIX     1

#else
    // Unknown
    #error "Unknown platform - please configure and report the results to stlab.adobe.com"

#endif

/*************************************************************************************************/

#include <GG/adobe/config/select_compiler.hpp>

/*************************************************************************************************/

#endif

/*************************************************************************************************/
