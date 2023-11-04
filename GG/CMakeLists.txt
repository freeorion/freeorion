# GiGi - A GUI for OpenGL
#
#  Copyright (C) 2009-2011 T. Zachary Laine <whatwasthataddress@gmail.com>
#  Copyright (C) 2013-2020 The FreeOrion Project
#
# Released under the GNU Lesser General Public License 2.1 or later.
# Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
# SPDX-License-Identifier: LGPL-2.1-or-later

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake)


##
## Global project configuration
##

project(GiGi)

include(CMakeDependentOption)
include(GNUInstallDirs)
include(UseCodeCoverage)
include(Util)

set(GIGI_VERSION 0.8.0)
if(WIN32)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_PNG_TEXTURES "Enable PNG texture support." ON)
option(ENABLE_TIFF_TEXTURES "Enable TIFF texture support." OFF)
option(BUILD_DOCUMENTATION "Builds HTML documentation." OFF)
option(BUILD_TESTING "Build the testing tree." OFF)
option(BUILD_DEVEL_PACKAGE "Install development files." ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


if(MSVC)
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")

    add_definitions(/wd4146 /wd4099 /wd4251 /wd4800 /wd4267 /wd4275
                    /wd4244 /wd4101 /wd4258 /wd4351 /wd4996)
    add_definitions(
        /EHsc
        -D_UNICODE
    )
endif()


##
## Collect project dependencies.
##

set(MINIMUM_BOOST_VERSION 1.69.0)

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS filesystem regex system REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)

if(ENABLE_PNG_TEXTURES)
    find_package(PNG REQUIRED)
    set(int_have_png 1)
else()
    set(int_have_png 0)
endif()

if(ENABLE_TIFF_TEXTURES)
    find_package(TIFF REQUIRED)
    set(int_have_tiff 1)
else()
    set(int_have_tiff 0)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/GG/Config.h
    @ONLY
)


##
## Define main project targets.
##

add_library(GiGiHeader INTERFACE)
add_library(GiGi::header ALIAS GiGiHeader)

target_include_directories(GiGiHeader
    INTERFACE
        Boost::boost
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_library(GiGi "")
add_library(GiGi::GiGi ALIAS GiGi)

target_compile_definitions(GiGi
    PUBLIC
        GiGiHeader
    PRIVATE
        -DFONS_USE_FREETYPE
        -DBOOST_GIL_IO_ENABLE_GRAY_ALPHA
        $<$<VERSION_GREATER:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.71>:BOOST_GIL_USES_MP11>
)

target_include_directories(GiGi
    INTERFACE
        ${OPENGL_INCLUDE_DIR}
    PRIVATE
        ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(GiGi
    PUBLIC
        GiGiHeader
        Boost::disable_autolinking
        Boost::dynamic_linking
        GLEW::GLEW
        $<IF:$<BOOL:${OPENGL_opengl_LIBRARY}>,${OPENGL_opengl_LIBRARY},${OPENGL_gl_LIBRARY}>
    PRIVATE
        Boost::filesystem
        Boost::regex
        ${FREETYPE_LIBRARIES}
)

if(TARGET PNG::PNG)
    target_link_libraries(GiGi PRIVATE PNG::PNG)
endif()

if(TARGET TIFF::TIFF)
    target_link_libraries(GiGi PRIVATE TIFF::TIFF)
endif()


##
## Recurse into sources.
##

add_subdirectory(GG)
add_subdirectory(src)

if(BUILD_TESTING)
    enable_testing()
    enable_coverage()

    if(NOT TARGET unittest)
        add_custom_target(unittest
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMENT "Run tests for ${CMAKE_PROJECT_NAME}"
        )
    endif()

    add_subdirectory(test)
endif()

if(BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()


##
## Install targets and package project.
##

if(BUILD_DEVEL_PACKAGE)
    install(
        DIRECTORY GG
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT COMPONENT_GIGI_DEVEL
    )
endif()

if(UNIX AND NOT APPLE)
    install(
        TARGETS GiGi
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT COMPONENT_GIGI
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT COMPONENT_GIGI
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT COMPONENT_GIGI_DEVEL
    )
endif()

if(UNIX AND BUILD_DEVEL_PACKAGE)
    get_pkg_config_libs(pkg_config_libs ${GiGi_LINK_LIBS})

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GiGi.pc.in
        ${CMAKE_BINARY_DIR}/GiGi.pc
        @ONLY
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/GiGi.pc
        DESTINATION  ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        COMPONENT COMPONENT_GIGI_DEVEL
        OPTIONAL
    )
endif()


set(CPACK_PACKAGE_VERSION ${GIGI_VERSION})
set(CPACK_PACKAGE_VENDOR "T. Zachary Laine")
set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A C++ OpenGL GUI library.")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src)

# RPM-specific settings
set(CPACK_RPM_PACKAGE_LICENSE LGPL)
set(CPACK_RPM_PACKAGE_REQUIRES)

list(APPEND CPACK_RPM_PACKAGE_REQUIRES Mesa freetype2)
list(APPEND CPACK_RPM_PACKAGE_REQUIRES gigi=${GIGI_VERSION})

if(BUILD_DEVEL_PACKAGE)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES boost-devel>=1.44.0 Mesa-devel freetype2-devel)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES gigi-devel=${GIGI_VERSION})
endif()

# Deb-specific settings
set(CPACK_DEBIAN_PACKAGE_SECTION libs)

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1-mesa-glx | libgl1, libfreetype6")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, gigi (= ${GIGI_VERSION})")

if(BUILD_DEVEL_PACKAGE)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "boost-devel (>= 1.44.0), libgl1-mesa-dev | libgl-dev, libfreetype6-dev")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, gigi-dev (= ${GIGI_VERSION})")
endif()

if(UNIX)
    set(CPACK_GENERATOR "TGZ;TBZ2")
    find_program(RPMBUILD rpmbuild)
    if(RPMBUILD)
        list(APPEND CPACK_GENERATOR RPM)
    endif()
    set(RPMBUILD ${RPMBUILD} CACHE INTERNAL "")
    find_program(DPKG dpkg)
    if(DPKG)
        list(APPEND CPACK_GENERATOR DEB)
    endif()
    set(DPKG ${DPKG} CACHE INTERNAL "")
    set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif()

include(CPack)

cpack_add_component(
    COMPONENT_GIGI
    DISPLAY_NAME GiGi
    DESCRIPTION "The core GG library, including the GUI state machine and all the controls.  Not usable without an input driver."
    REQUIRED
    GROUP COMPONENT_GROUP_RUNTIME
)

cpack_add_component(
    COMPONENT_GIGI_DEVEL
    DISPLAY_NAME "GiGi"
    DESCRIPTION "The files necessary to do development with GiGi."
    GROUP COMPONENT_GROUP_DEVEL
    DEPENDS COMPONENT_GIGI
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
