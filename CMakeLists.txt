cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

cmake_policy(SET CMP0042 NEW)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo Release)
IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "Setting build type to 'Release' as none was specified.")
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
ENDIF(NOT CMAKE_BUILD_TYPE)

message(STATUS "Build type CMAKE_BUILD_TYPE set to ${CMAKE_BUILD_TYPE}")

include(UseCompilerCache)
find_compiler_cache(PROGRAM ccache)


##
## Global project configuration
##

project(FreeOrion)

use_compiler_cache_with_xcode()

include(GNUInstallDirs)
include(UseCodeCoverage)
include(TargetDependencies)

set(FreeOrion_VERSION 0.4.10+)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#set(FreeOrion_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/freeorion")
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${FreeOrion_INSTALL_LIBDIR}")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


add_compile_options(
    # Enable (almost) all warnings
    # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wall-316
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    # http://clang.llvm.org/docs/DiagnosticsReference.html#wall
    $<$<CXX_COMPILER_ID:Clang>:-Wall>
)

set_property(DIRECTORY APPEND
    PROPERTY COMPILE_DEFINITIONS
        $<$<PLATFORM_ID:Linux>:FREEORION_LINUX>
        $<$<PLATFORM_ID:FreeBSD>:FREEORION_FREEBSD>
        $<$<PLATFORM_ID:OpenBSD>:FREEORION_OPENBSD>
        $<$<PLATFORM_ID:Haiku>:FREEORION_HAIKU>
        $<$<PLATFORM_ID:Android>:FREEORION_ANDROID>
)

add_definitions(-DENABLE_BINRELOC -DBINPATH=\"${CMAKE_INSTALL_FULL_BINDIR}\" -DSHAREPATH=\"${CMAKE_INSTALL_FULL_DATADIR}\")

include(TestForBug2795)


##
## Collect project dependencies.
##

set(MINIMUM_PYTHON_VERSION 3.5)
set(MINIMUM_BOOST_VERSION 1.62.0)

find_package(Threads)
find_package(PythonInterp ${MINIMUM_PYTHON_VERSION} REQUIRED)
if(BUILD_AI OR BUILD_SERVER)
    find_package(PythonLibs ${MINIMUM_PYTHON_VERSION} REQUIRED)
endif()
find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        filesystem
        iostreams
        locale
        log
        log_setup
        serialization
        system
        thread
    REQUIRED)

find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} REQUIRED)
set(Boost_PYTHON_SUFFIX "${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")

find_package(ZLIB REQUIRED)
set(SDL_NO_MAIN true)
find_package(SDL REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Ogg REQUIRED)
find_package(Vorbis REQUIRED)

set(BUILD_DEVEL_PACKAGE OFF CACHE INTERNAL "Disables installation of GiGi development files." FORCE)
set(_ORIG_CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_LIBDIR "${FreeOrion_INSTALL_LIBDIR}")
add_subdirectory(GG)
set(CMAKE_INSTALL_LIBDIR ${_ORIG_CMAKE_INSTALL_LIBDIR})
unset(_ORIG_CMAKE_INSTALL_LIBDIR)

set_property(DIRECTORY APPEND
    PROPERTY COMPILE_DEFINITIONS
        # with boost 1.61 some boost::optional internals were changed. However
        # boost::spirit relies on some API the old implementation provided.
        # This define enables the usage of the old boost::optional
        # implementation.  Boost upstream tracks this bug as #12349
        $<$<AND:$<VERSION_GREATER:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.60>,$<VERSION_LESS:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.67>>:BOOST_OPTIONAL_CONFIG_USE_OLD_IMPLEMENTATION_OF_OPTIONAL>

        # We don't need localized output of Boost date_time and not setting
        # the define causes the inclusion of code, which contains std::tolower.
        # This however causes a macro substitutions caused by the libpython
        # headers, which in turn breaks the build.  Python 2.7.13 should have
        # fixed this with python bug #10910
        $<$<PLATFORM_ID:Darwin>:BOOST_DATE_TIME_NO_LOCALE>
        $<$<PLATFORM_ID:FreeBSD>:BOOST_DATE_TIME_NO_LOCALE>
)




##
## Define main project targets.
##

# To run the version generation every compile we need to deferr the
# execution to a separate target and the existing python command
add_custom_target(freeorionversion
    COMMAND
    "${PYTHON_EXECUTABLE}"
    "${CMAKE_SOURCE_DIR}/cmake/make_versioncpp.py"
    "${CMAKE_SOURCE_DIR}"
    "CMake"
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/util/Version.cpp
    PROPERTIES
    GENERATED TRUE
)



add_library(freeorioncommon STATIC "")
target_compile_options(freeorioncommon
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-depth=512>
        $<$<CXX_COMPILER_ID:AppleClang>:-ftemplate-depth=512>
)
target_compile_definitions(freeorioncommon
    PUBLIC
        $<$<NOT:$<PLATFORM_ID:Android>>:BOOST_ALL_DYN_LINK>
    PRIVATE
        -DFREEORION_BUILD_COMMON
        -DFREEORION_PYTHON_VERSION=\"${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}\"
)
target_link_libraries(freeorioncommon
    PUBLIC
        Boost::boost
        Boost::disable_autolinking
        $<$<NOT:$<PLATFORM_ID:Android>>:Boost::dynamic_linking>
    INTERFACE
        Boost::filesystem
        Boost::iostreams
        Boost::locale
        Boost::log_setup
        Boost::log
        Boost::thread
        Boost::serialization
        ZLIB::ZLIB
        ${ICUI18N_LIBRARY}
        ${ICUUC_LIBRARY}
        ${ICUDATA_LIBRARY}
    PRIVATE
        ${CORE_FOUNDATION_LIBRARY}
)
add_dependencies(freeorioncommon freeorionversion)




add_library(freeoriond STATIC "")
target_compile_definitions(freeoriond
    PRIVATE
        -DFREEORION_BUILD_SERVER
)
target_include_directories(freeoriond SYSTEM
    PRIVATE
        ${PYTHON_INCLUDE_PATH}
)
target_link_libraries(freeoriond
    PRIVATE
        freeorioncommon
        Boost::python${Boost_PYTHON_SUFFIX}
        ${PYTHON_LIBRARIES}
)



add_library(freeorionca STATIC "")
target_compile_definitions(freeorionca
    PRIVATE
        -DFREEORION_BUILD_AI
)
target_include_directories(freeorionca SYSTEM
    PRIVATE
        ${PYTHON_INCLUDE_PATH}
)
target_link_libraries(freeorionca
    freeorioncommon
    Boost::python${Boost_PYTHON_SUFFIX}
    ${PYTHON_LIBRARIES}
)
    


add_library(freeorion STATIC "")
target_sources(freeorion
    PRIVATE
        ${FREEORION_ASSETS}
)
set_property(
    SOURCE
        ${FREEORION_ASSETS}
    PROPERTY
        MACOSX_PACKAGE_LOCATION "Resources/"
)
# Workaround for creating the initial Info.plist, which needs to exist
# at configuration time.
execute_process(
    COMMAND
    "${PYTHON_EXECUTABLE}"
    "${CMAKE_SOURCE_DIR}/cmake/make_versioncpp.py"
    "${CMAKE_SOURCE_DIR}"
    "CMake"
)
target_compile_definitions(freeorion
    PRIVATE
        -DFREEORION_BUILD_HUMAN
)
target_include_directories(freeorion SYSTEM
    PRIVATE
        ${SDL_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${OPENAL_INCLUDE_DIR}
        ${OGG_INCLUDE_DIRS}
        ${VORBIS_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
)
target_link_libraries(freeorion
    freeorioncommon
    GiGi::GiGi
    ${SDL_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${OPENAL_LIBRARY}
    ${OGG_LIBRARIES}
    ${VORBIS_LIBRARIES}
    ${ICONV_LIBRARY}
)



##
## Recurse into sources.
##
add_subdirectory(client)
add_subdirectory(combat)
add_subdirectory(Empire)
add_subdirectory(network)
add_subdirectory(python)
add_subdirectory(server)
add_subdirectory(UI)
add_subdirectory(universe)
add_subdirectory(util)
add_subdirectory(check)
add_subdirectory(doc)



