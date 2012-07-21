##########################################################################
# Boost Configuration Support                                            #
##########################################################################
# Copyright (C) 2007 Douglas Gregor <doug.gregor@gmail.com>              #
# Copyright (C) 2007 Troy Straszheim                                     #
#                                                                        #
# Distributed under the Boost Software License, Version 1.0.             #
# See accompanying file LICENSE_1_0.txt or copy at                       #
#   http://www.boost.org/LICENSE_1_0.txt                                 #
##########################################################################

include(CheckCXXSourceCompiles)

set(BUILD_PLATFORM "unknown")

# Multi-threading support
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  set(MULTI_THREADED_COMPILE_FLAGS "-pthreads")
  set(MULTI_THREADED_LINK_LIBS rt)
  set(BUILD_PLATFORM "sunos")
elseif(CMAKE_SYSTEM_NAME STREQUAL "BeOS")
  # No threading options necessary for BeOS
  set(BUILD_PLATFORM "beos")
elseif(CMAKE_SYSTEM_NAME MATCHES ".*BSD")
  set(MULTI_THREADED_COMPILE_FLAGS "-pthread")
  set(MULTI_THREADED_LINK_LIBS pthread)
  set(BUILD_PLATFORM "bsd")
elseif(CMAKE_SYSTEM_NAME STREQUAL "DragonFly")
  # DragonFly is a FreeBSD bariant
  set(MULTI_THREADED_COMPILE_FLAGS "-pthread")
  set(BUILD_PLATFORM "dragonfly")
elseif(CMAKE_SYSTEM_NAME STREQUAL "IRIX")
  # TODO: GCC on Irix doesn't support multi-threading?
  set(BUILD_PLATFORM "irix")
elseif(CMAKE_SYSTEM_NAME STREQUAL "HP-UX")
  # TODO: gcc on HP-UX does not support multi-threading?
  set(BUILD_PLATFORM "hpux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # No threading options necessary for Mac OS X
  set(BUILD_PLATFORM "macos")
elseif(UNIX)
  # Assume -pthread and -lrt on all other variants
  set(MULTI_THREADED_COMPILE_FLAGS "-pthread -D_REENTRANT")
  set(MULTI_THREADED_LINK_FLAGS "")  
  set(MULTI_THREADED_LINK_LIBS pthread rt)

  if (MINGW)
    set(BUILD_PLATFORM "mingw")
  elseif(CYGWIN)
    set(BUILD_PLATFORM "cygwin")
  elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BUILD_PLATFORM "linux")
  else()
    set(BUILD_PLATFORM "unix")
  endif()
elseif(WIN32)
  set(BUILD_PLATFORM "windows")
else()
  set(BUILD_PLATFORM "unknown")
endif()

# create cache entry
set(BUILD_PLATFORM ${BUILD_PLATFORM} CACHE STRING "Platform name")

message(STATUS "Build platform: ${BUILD_PLATFORM}")

# Setup DEBUG_COMPILE_FLAGS, RELEASE_COMPILE_FLAGS, DEBUG_LINK_FLAGS and
# and RELEASE_LINK_FLAGS based on the CMake equivalents
if(CMAKE_CXX_FLAGS_DEBUG)
  if(MSVC)
    # Eliminate the /MDd flag; we'll add it back when we need it
    string(REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG 
           "${CMAKE_CXX_FLAGS_DEBUG}") 
  endif(MSVC)
  set(DEBUG_COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "Compilation flags for debug libraries")
endif(CMAKE_CXX_FLAGS_DEBUG)
if(CMAKE_CXX_FLAGS_RELEASE)
  if(MSVC)
    # Eliminate the /MD flag; we'll add it back when we need it
    string(REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE
           "${CMAKE_CXX_FLAGS_RELEASE}") 
  endif(MSVC)
  set(RELEASE_COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "Compilation flags for release libraries")
endif(CMAKE_CXX_FLAGS_RELEASE)
if(CMAKE_SHARED_LINKER_FLAGS_DEBUG)
  set(DEBUG_LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING "Linker flags for debug libraries")
endif(CMAKE_SHARED_LINKER_FLAGS_DEBUG)
if(CMAKE_SHARED_LINKER_FLAGS_RELEASE)
  set(RELEASE_LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "Link flags for release libraries")
endif(CMAKE_SHARED_LINKER_FLAGS_RELEASE)

# Set DEBUG_EXE_LINK_FLAGS, RELEASE_EXE_LINK_FLAGS
if (CMAKE_EXE_LINKER_FLAGS_DEBUG)
  set(DEBUG_EXE_LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
endif (CMAKE_EXE_LINKER_FLAGS_DEBUG)
if (CMAKE_EXE_LINKER_FLAGS_RELEASE)
  set(RELEASE_EXE_LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
endif (CMAKE_EXE_LINKER_FLAGS_RELEASE)

# Tweak the configuration and build types appropriately.
if(CMAKE_CONFIGURATION_TYPES)
  # Limit CMAKE_CONFIGURATION_TYPES to Debug and Release
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Semicolon-separate list of supported configuration types" FORCE)
else(CMAKE_CONFIGURATION_TYPES)
  # Build in release mode by default
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are Release or Debug" FORCE)
  endif (NOT CMAKE_BUILD_TYPE)
endif(CMAKE_CONFIGURATION_TYPES)

# Clear out the built-in C++ compiler and link flags for each of the 
# configurations.
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE INTERNAL "Unused by GG")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Unused by GG")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Unused by GG")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Unused by GG")
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE INTERNAL "Unused by GG")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "Unused by GG")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "Unused by GG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "Unused by GG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "" CACHE INTERNAL "Unused by GG")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "Unused by GG")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "Unused by GG")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "Unused by GG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "Unused by GG")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "Unused by GG")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "Unused by GG")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "Unused by GG")
