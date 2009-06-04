cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

set(CMAKE_VERBOSE_MAKEFILE true)

########################################
# Configuration                        #
########################################
set(FREEORION_VERSION v0.3.13)
set(FREEORION_RELEASE 0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)
include(Config)
include(Util)

set(MINIMUM_BOOST_VERSION 1.36.0)
set(MINIMUM_BOOST_COMPONENTS date_time filesystem serialization signals system thread)

option(BUILD_DEBUG "Controls generation of debug code; OFF implies release code." OFF)

if (BUILD_DEBUG)
    set(BUILD_RELEASE OFF)
else ()
    set(BUILD_RELEASE ON)
endif ()

set(BUILD_STATIC OFF)
set(BUILD_SHARED ON)
set(BUILD_SINGLE_THREADED OFF)

if (WIN32)
    add_definitions(-DFREEORION_WIN32)
elseif (APPLE)
    add_definitions(-DFREEORION_MACOSX)
elseif (CYGWIN)
    message(FATAL_ERROR "Sorry, Cygwin is not a supported platform for FreeOrion.")
elseif (UNIX)
    add_definitions(-DFREEORION_LINUX -DENABLE_BINRELOC -Wall -Wno-parentheses -Wno-deprecated)
else ()
    message(FATAL_ERROR "Unknown platform type! Your setup is not a supported platform for FreeOrion.")
endif ()


########################################
# Dependencies                         #
########################################
set(Boost_USE_STATIC_LIBS ${BUILD_STATIC})
set(Boost_USE_MULTITHREADED ${BUILD_MULTI_THREADED})
find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS ${MINIMUM_BOOST_COMPONENTS})
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost libraries not found.")
endif ()

find_package(ZLIB)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "ZLib library not found.")
endif ()

find_package(Subversion)
set(svn_revision_number ???)
if (Subversion_FOUND)
   execute_process(
       COMMAND ${Subversion_SVN_EXECUTABLE} info
       WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
       RESULT_VARIABLE result
       OUTPUT_VARIABLE output
   )
   if (NOT result)
       string(REGEX REPLACE ".*Last Changed Rev: ([0-9]*).*" "\\1" svn_revision_number ${output})
   endif ()
endif ()
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/Version.cpp.in
    ${CMAKE_BINARY_DIR}/util/Version.cpp
    @ONLY
)


########################################
# Recurse Into Sources                 #
########################################
include_directories(${CMAKE_SOURCE_DIR} log4cpp)

add_subdirectory(server)
#add_subdirectory(client/AI)
#add_subdirectory(client/human)
