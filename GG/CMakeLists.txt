cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

enable_testing()

project(GiGi)

set(CMAKE_VERBOSE_MAKEFILE false)

########################################
# Configuration                        #
########################################
set(GIGI_VERSION 0.8.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake)
include(Config)
include(Util)

set(MINIMUM_BOOST_VERSION 1.47.0)
set(MINIMUM_BOOST_COMPONENTS date_time filesystem signals system thread)

option(BUILD_STATIC "Enables building of static libraries." OFF)
option(BUILD_SHARED "Enables building of shared libraries." ON)
option(BUILD_DEBUG "Controls generation of debug code; OFF implies release code." OFF)
option(BUILD_MULTI_THREADED "Controls generation of multi-threaded code; OFF implies single-threaded code." ON)

option(BUILD_EXPERIMENTAL_EVE_SUPPORT "Enables building of experimental Adobe Eve bindings." OFF)

if (BUILD_DEBUG)
    set(BUILD_RELEASE OFF)
else ()
    set(BUILD_RELEASE ON)
endif ()

if (BUILD_MULTI_THREADED)
    set(BUILD_SINGLE_THREADED OFF)
else ()
    set(BUILD_SINGLE_THREADED ON)
endif ()

set(BOOST_SIGNALS_NAMESPACE "signals" CACHE STRING
    "Specifies alternate namespace used for boost::signals (only needed if you changed it using the BOOST_SIGNALS_NAMESPACE define when you built boost).")

option(USE_DEVIL
       "Enables optional use of the DevIL image-loading library.  This should only be enabled if you need to load image files other than PNG, JPEG, and TIFF."
       OFF)

option(BUILD_SDL_DRIVER
       "Builds GG SDL support (the GiGiSDL library)."
       ON)
option(BUILD_OGRE_DRIVER
       "Builds GG Ogre support (the GiGiOgre library)."
       ON)
option(BUILD_OGRE_OIS_PLUGIN
       "Builds OIS input plugin for the GiGiOgre library."
       ON)
option(BUILD_DOCUMENTATION
       "Builds HTML documentation (requires Doxygen)."
       OFF)

set(RUNTIME_ONLY_PACKAGE false)
set(DEVEL_ONLY_PACKAGE false)
set(NO_DOCS_PACKAGE false)
set(NO_HEADERS_PACKAGE false)
set(GG_PACKAGE_DEPENDENCY false)
set(GG_DEVEL_PACKAGE_DEPENDENCY false)
set(SDL_PACKAGE_DEPENDENCY false)
set(SDL_DEVEL_PACKAGE_DEPENDENCY false)
set(OGRE_OIS_PACKAGE_DEPENDENCY false)
set(OGRE_OIS_DEVEL_PACKAGE_DEPENDENCY false)
if (UNIX)
    set(PACKAGE_CONFIGURATIONS
        NoPackage GiGi GiGiSDL GiGiOgre Full GiGiDevel GiGiSDLDevel GiGiOgreDevel FullDevel
        CACHE INTERNAL
        "The types of packages available for build.  NoPackage means leave everything in.  They follow the pattern {GiGi|GiGiSDL|GiGiOgre|Full}[Devel]."
    )
    set(PACKAGE_TYPE
        NoPackage
        CACHE STRING
        "The type of package chosen for this build.  These allow you to build Debs and RPMs that contain the sublibraries within GG, or all of GG, binary-only or \"-devel\".  NoPackage means leave everything in.  They follow the pattern {GiGi|GiGiSDL|GiGiOgre|Full}[Devel]."
    )
    list_contains(package_type_valid ${PACKAGE_TYPE} ${PACKAGE_CONFIGURATIONS})
    if (NOT package_type_valid)
        message(FATAL_ERROR "Your choice for PACKAGE_TYPE \"${PACKAGE_TYPE}\" is not a valid string.  Choose one of [${PACKAGE_CONFIGURATIONS}].")
    endif ()
    if (NOT ${PACKAGE_TYPE} STREQUAL NoPackage)
        if (${PACKAGE_TYPE} MATCHES Devel)
            set(RUNTIME_ONLY_PACKAGE false)
            set(DEVEL_ONLY_PACKAGE true)
        else ()
            set(RUNTIME_ONLY_PACKAGE true)
            set(DEVEL_ONLY_PACKAGE false)
        endif ()
        if (NOT ${PACKAGE_TYPE} STREQUAL FullDevel AND NOT ${PACKAGE_TYPE} STREQUAL GiGiDevel)
            set(NO_DOCS_PACKAGE true)
            set(NO_HEADERS_PACKAGE true)
            if (DEVEL_ONLY_PACKAGE)
                set(GG_DEVEL_PACKAGE_DEPENDENCY true)
            endif ()
        endif ()
        if (RUNTIME_ONLY_PACKAGE AND NOT ${PACKAGE_TYPE} STREQUAL Full AND NOT ${PACKAGE_TYPE} STREQUAL GiGi)
            set(GG_PACKAGE_DEPENDENCY true)
        endif ()
        if (${PACKAGE_TYPE} STREQUAL Full OR ${PACKAGE_TYPE} STREQUAL GiGiSDL)
            set(SDL_PACKAGE_DEPENDENCY true)
        endif ()
        if (${PACKAGE_TYPE} STREQUAL FullDevel OR ${PACKAGE_TYPE} STREQUAL GiGiSDLDevel)
            set(SDL_DEVEL_PACKAGE_DEPENDENCY true)
        endif ()
        if (${PACKAGE_TYPE} STREQUAL Full OR ${PACKAGE_TYPE} STREQUAL GiGiOgre)
            set(OGRE_OIS_PACKAGE_DEPENDENCY true)
        endif ()
        if (${PACKAGE_TYPE} STREQUAL FullDevel OR ${PACKAGE_TYPE} STREQUAL GiGiOgreDevel)
            set(OGRE_OIS_DEVEL_PACKAGE_DEPENDENCY true)
        endif ()
    endif ()
else ()
    set(PACKAGE_CONFIGURATIONS NoPackage)
    set(PACKAGE_TYPE NoPackage)
endif ()

# Determines whether ${component} is included in the package being built (if
# any), and returns the result in ${var}.  Ignores the distinction between
# Devel and non-Devel package types.
macro (package_compatible var component)
    if (${component} MATCHES GiGiSDL)
        if (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} MATCHES GiGiSDL)
            set(${var} true)
        else ()
            set(${var} false)
        endif ()
    elseif (${component} MATCHES GiGiOgre)
        if (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} MATCHES GiGiOgre)
            set(${var} true)
        else ()
            set(${var} false)
        endif ()
    elseif (${component} MATCHES GiGi)
        if (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} STREQUAL GiGi OR ${PACKAGE_TYPE} STREQUAL GiGiDevel)
            set(${var} true)
        else ()
            set(${var} false)
        endif ()
    else ()
        message(FATAL_ERROR "Invalid component \"${component}\" passed to package_compatible().")
    endif ()
endmacro ()

include(CMakeDependentOption)
cmake_dependent_option(BUILD_TUTORIALS
                       "Build tutorial apps (requires SDL driver)."
                       OFF
                       BUILD_SDL_DRIVER
                       OFF)

cmake_dependent_option(BUILD_TESTS
                       "Build test apps (requires SDL driver)."
                       OFF
                       BUILD_SDL_DRIVER
                       OFF)

add_definitions(-DADOBE_STD_SERIALIZATION)

include_directories(${CMAKE_HOME_DIRECTORY})


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

if (NOT BOOST_SIGNALS_NAMESPACE STREQUAL "signals")
    add_definitions(
        -DBOOST_SIGNALS_NAMESPACE=${BOOST_SIGNALS_NAMESPACE}
        -Dsignals=${BOOST_SIGNALS_NAMESPACE}
    )
endif ()

set(int_have_devil 0)
set(int_have_jpeg 0)
set(int_have_png 0)
set(int_have_tiff 0)
if (USE_DEVIL)
    find_package(DevIL)
    if (IL_FOUND)
        include_directories(${IL_INCLUDE_DIR})
        set(int_have_devil 1)
    else ()
        message(FATAL_ERROR "DevIL libraries not found.")
    endif ()
else ()
    find_package(JPEG)
    if (JPEG_FOUND)
        include_directories(${JPEG_INCLUDE_DIR})
        set(int_have_jpeg 1)
    endif ()
    find_package(PNG)
    if (PNG_FOUND)
        include_directories(${PNG_INCLUDE_DIR})
        set(int_have_png 1)
    endif ()
    find_package(TIFF)
    if (TIFF_FOUND)
        include_directories(${TIFF_INCLUDE_DIR})
        set(int_have_tiff 1)
    endif ()
endif ()

find_package(OpenGL)
if (OPENGL_FOUND)
    if (NOT OPENGL_GLU_FOUND)
        message(FATAL_ERROR "OpenGL GLU library not found.")
    endif ()
    include_directories(${OPENGL_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "OpenGL library not found.")
endif ()

find_package(Freetype)
if (FREETYPE_FOUND)
    include_directories(${FREETYPE_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Freetype library not found.")
endif ()


########################################
# Header Generation                    #
########################################
if (NOT EXISTS ${CMAKE_HOME_DIRECTORY}/libltdl/config.h)
    if (UNIX)
        message("-- Configuring libltdl using \"libltdl/configure\" ...")
        execute_process(
            COMMAND ${CMAKE_HOME_DIRECTORY}/libltdl/configure
            WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/libltdl
            OUTPUT_QUIET
        )
    else ()
        file(WRITE ${CMAKE_HOME_DIRECTORY}/libltdl/config.h
             "/* WARNING: Generated by GG's build system.  All local changes will be lost! */
#define error_t int
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_CTYPE_H 1
#define HAVE_MEMORY_H 1
#define HAVE_ERRNO_H 1
#define __WIN32__
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define LTDL_OBJDIR \".libs\"
#define LTDL_DLOPEN_DEPLIBS 1
#define LTDL_SHLIBPATH_VAR \"PATH\"
#define LTDL_SHLIB_EXT \".dll\"
"
        )
    endif ()
endif ()

configure_file(
    ${CMAKE_HOME_DIRECTORY}/cmake/Config.h.in
    ${CMAKE_HOME_DIRECTORY}/GG/Config.h
    @ONLY
)


########################################
# Installed Files                      #
########################################
if (NOT NO_HEADERS_PACKAGE)
    install(
        DIRECTORY GG
        DESTINATION include
        COMPONENT COMPONENT_GIGI_DEVEL
        PATTERN ".svn" EXCLUDE
        PATTERN "*~" EXCLUDE
    )
endif ()

macro (install_pc_file component)
    package_compatible(ok_to_install ${component})
    if (ok_to_install)
        install(
            FILES ${CMAKE_BINARY_DIR}/${component}.pc
            DESTINATION lib${LIB_SUFFIX}/pkgconfig
            COMPONENT COMPONENT_GIGI_DEVEL
            OPTIONAL
        )
    endif ()
endmacro ()

if (UNIX AND NOT RUNTIME_ONLY_PACKAGE)
    install_pc_file(GiGi)
    install_pc_file(GiGiSDL)
    install_pc_file(GiGiOgre)
endif ()


########################################
# Recurse Into Sources                 #
########################################
add_subdirectory(src)

if (BUILD_TESTS)
    add_subdirectory(test)
endif ()

if (BUILD_TUTORIALS)
    add_subdirectory(tutorial)
endif ()


########################################
# Documentation                        #
########################################
if (BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
endif ()

if (DOXYGEN_FOUND)
    set(GG_DOC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doc")
    set(GG_DOC_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc")
    set(GG_DOC_STAMP_DIR "${GG_DOC_OUTPUT_DIR}/GG")
    configure_file(
        ${GG_DOC_SOURCE_DIR}/Doxyfile.in
        ${GG_DOC_OUTPUT_DIR}/Doxyfile
        @ONLY
        IMMEDIATE
    )
    add_custom_command (
        OUTPUT ${GG_DOC_STAMP_DIR}/index.html
        COMMAND ${DOXYGEN_EXECUTABLE} ${GG_DOC_OUTPUT_DIR}/Doxyfile
        MAIN_DEPENDENCY ${GG_DOC_OUTPUT_DIR}/Doxyfile ${GG_DOC_SOURCE_DIR}/Doxyfile.in
        DEPENDS ${GG_DOC_SOURCE_DIR}/GGDoc.txt
        COMMENT "Generating documentation with Doxygen" VERBATIM
    )
    add_custom_target (doc ALL DEPENDS ${GG_DOC_STAMP_DIR}/index.html)
    if (NOT NO_DOCS_PACKAGE)
        install(
            DIRECTORY ${GG_DOC_STAMP_DIR}
            DESTINATION doc
        )
    endif ()
endif ()


########################################
# Packaging                            #
########################################
if (RUNTIME_ONLY_PACKAGE AND DEVEL_ONLY_PACKAGE)
    message(FATAL_ERROR "Both RUNTIME_ONLY_PACKAGE and DEVEL_ONLY_PACKAGE are turned on.  This means that nothing will be installed and generated packages will be empty!")
endif ()

set(CPACK_PACKAGE_VERSION ${GIGI_VERSION})
set(CPACK_PACKAGE_VENDOR "T. Zachary Laine")
set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A C++ OpenGL GUI library.")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_HOME_DIRECTORY}/COPYING)
if (NOT ${PACKAGE_TYPE} STREQUAL NoPackage)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
    if (${PACKAGE_TYPE} STREQUAL Full)
        set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
        string(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)
    else ()
        if (NOT ${PACKAGE_TYPE} STREQUAL FullDevel)
            set(CPACK_PACKAGE_NAME ${PACKAGE_TYPE})
            string(REPLACE Devel "" CPACK_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
            if (${PACKAGE_TYPE} MATCHES Ogre OR ${PACKAGE_TYPE} MATCHES SDL)
                string(REPLACE GiGi GiGi- CPACK_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
            endif ()
        else ()
            set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
        endif ()
        set(version_devel_substring ${CPACK_PACKAGE_VERSION})
        if (DEVEL_ONLY_PACKAGE)
            set(version_devel_substring devel-${version_devel_substring})
        endif ()
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${version_devel_substring}-${CPACK_SYSTEM_NAME}")
        string(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)
    endif ()
endif ()
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src)

# RPM-specific settings
set(CPACK_RPM_PACKAGE_LICENSE LGPL)
set(CPACK_RPM_PACKAGE_REQUIRES)
if (NOT RUNTIME_ONLY_PACKAGE)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES boost-devel>=1.44.0 Mesa-devel freetype2-devel)
    if (GG_DEVEL_PACKAGE_DEPENDENCY)
        list(APPEND CPACK_RPM_PACKAGE_REQUIRES gigi-devel=${GIGI_VERSION})
    endif ()
    if (SDL_DEVEL_PACKAGE_DEPENDENCY)
        list(APPEND CPACK_RPM_PACKAGE_REQUIRES SDL-devel>=1.2)
    endif ()
    if (OGRE_OIS_DEVEL_PACKAGE_DEPENDENCY)
        list(APPEND CPACK_RPM_PACKAGE_REQUIRES ogre-devel>=1.6 libois-devel>=1.2)
    endif ()
endif ()
if (NOT DEVEL_ONLY_PACKAGE)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES Mesa freetype2)
    if (GG_PACKAGE_DEPENDENCY)
        list(APPEND CPACK_RPM_PACKAGE_REQUIRES gigi=${GIGI_VERSION})
    endif ()
    if (SDL_PACKAGE_DEPENDENCY)
        list(APPEND CPACK_RPM_PACKAGE_REQUIRES SDL>=1.2)
    endif ()
    if (OGRE_OIS_PACKAGE_DEPENDENCY)
        list(APPEND CPACK_RPM_PACKAGE_REQUIRES ogre>=1.6 libois1>=1.2)
    endif ()
endif ()

# Deb-specific settings
set(CPACK_DEBIAN_PACKAGE_SECTION libs)
if (NOT RUNTIME_ONLY_PACKAGE)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "boost-devel (>= 1.44.0), libgl1-mesa-dev | libgl-dev, libglu1-mesa-dev | libglu-dev, libfreetype6-dev")
    if (GG_DEVEL_PACKAGE_DEPENDENCY)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, gigi-dev (= ${GIGI_VERSION})")
    endif ()
    if (SDL_DEVEL_PACKAGE_DEPENDENCY)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsdl1.2-dev (>= 1.2)")
    endif ()
    if (OGRE_OIS_DEVEL_PACKAGE_DEPENDENCY)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS},  libogre-dev (>= 1.4.9), libois-dev")
    endif ()
endif ()
if (NOT DEVEL_ONLY_PACKAGE)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1-mesa-glx | libgl1, libglu1-mesa | libglu, libfreetype6")
    if (GG_PACKAGE_DEPENDENCY)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, gigi (= ${GIGI_VERSION})")
    endif ()
    if (SDL_PACKAGE_DEPENDENCY)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsdl1.2debian (>= 1.2)")
    endif ()
    if (OGRE_OIS_PACKAGE_DEPENDENCY)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS},  libogremain-1.4.9 (>= 1.4.9), libois1")
    endif ()
endif ()

if (UNIX)
    set(CPACK_GENERATOR "TGZ;TBZ2")
    find_program(RPMBUILD rpmbuild)
    if (RPMBUILD)
        list(APPEND CPACK_GENERATOR RPM)
    endif ()
    set(RPMBUILD ${RPMBUILD} CACHE INTERNAL "")
    find_program(DPKG dpkg)
    if (DPKG)
        list(APPEND CPACK_GENERATOR DEB)
    endif ()
    set(DPKG ${DPKG} CACHE INTERNAL "")
    set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
elseif (WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif ()

include(CPack)

cpack_add_component(
    COMPONENT_GIGI
    DISPLAY_NAME GiGi
    DESCRIPTION "The core GG library, including the GUI state machine and all the controls.  Not usable without an input driver."
    REQUIRED
    GROUP COMPONENT_GROUP_RUNTIME
)

cpack_add_component(
    COMPONENT_GIGISDL
    DISPLAY_NAME GiGiSDL
    DESCRIPTION "An SDL input driver (http://www.libsdl.org)."
    GROUP COMPONENT_GROUP_RUNTIME
    DEPENDS COMPONENT_GIGI
)

cpack_add_component(
    COMPONENT_GIGIOGRE
    DISPLAY_NAME GiGiOgre
    DESCRIPTION "An Ogre input driver (http://www.ogre3d.org).  Note that Ogre itself does not include proper input support.  The OIS input plugin is recommended."
    GROUP COMPONENT_GROUP_RUNTIME
    DEPENDS COMPONENT_GIGI
)

cpack_add_component(
    COMPONENT_GIGIOGREPLUGIN_OIS
    DISPLAY_NAME "GiGiOgre OIS Plugin"
    DESCRIPTION "An Ogre plugin using OIS (http://sourceforge.net/projects/wgois)."
    GROUP COMPONENT_GROUP_RUNTIME
    DEPENDS COMPONENT_GIGIOGRE
)

cpack_add_component(
    COMPONENT_GIGI_DEVEL
    DISPLAY_NAME "GiGi"
    DESCRIPTION "The files necessary to do development with GiGi."
    GROUP COMPONENT_GROUP_DEVEL
    DEPENDS COMPONENT_GIGI
)

cpack_add_component(
    COMPONENT_GIGISDL_DEVEL
    DISPLAY_NAME "GiGiSDL"
    DESCRIPTION "The files necessary to do development with GiGiSDL."
    GROUP COMPONENT_GROUP_DEVEL
    DEPENDS COMPONENT_GIGI_DEVEL COMPONENT_GIGISDL
)

cpack_add_component(
    COMPONENT_GIGIOGRE_DEVEL
    DISPLAY_NAME "GiGiOgre"
    DESCRIPTION "The files necessary to do development with GiGiOgre."
    GROUP COMPONENT_GROUP_DEVEL
    DEPENDS COMPONENT_GIGI_DEVEL COMPONENT_GIGIOGRE
)

cpack_add_component(
    COMPONENT_GIGIOGREPLUGIN_OIS_DEVEL
    DISPLAY_NAME "GiGiOgre OIS Plugin"
    DESCRIPTION "The files necessary to do development with GiGiOgre's OIS Plugin.  Only required for users who are statically linking against Ogre, and want to link to the OIS plugin at link-time."
    GROUP COMPONENT_GROUP_DEVEL
    DEPENDS COMPONENT_GIGIOGRE_DEVEL COMPONENT_GIGIOGREPLUGIN_OIS
)

cpack_add_component_group(
    COMPONENT_GROUP_RUNTIME
    DISPLAY_NAME "Runtime"
    DESCRIPTION "The runtime libs needed to run an application that uses GG."
    EXPANDED
)

cpack_add_component_group(
    COMPONENT_GROUP_DEVEL
    DISPLAY_NAME "Development"
    DESCRIPTION "The development libs and headers needed to build an application that uses GG."
    EXPANDED
)
