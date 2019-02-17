# GG is a GUI for SDL and OpenGL.
#
# Copyright (C) 2009-2011 T. Zach Laine
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1
# of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
#
# If you do not wish to comply with the terms of the LGPL please
# contact the author as other terms are available for a fee.
#
# Zach Laine
# whatwasthataddress@gmail.com

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

if(POLICY CMP0044)
    cmake_policy(SET CMP0044 OLD)
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
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_PNG_TEXTURES "Enable PNG texture support." ON)
option(ENABLE_TIFF_TEXTURES "Enable TIFF texture support." OFF)
option(BUILD_SDL_DRIVER "Builds GG SDL support (the GiGiSDL library)." ON)
option(BUILD_DOCUMENTATION "Builds HTML documentation." OFF)
option(BUILD_TESTING "Build the testing tree." OFF)
option(BUILD_DEVEL_PACKAGE "Install development files." ON)

cmake_dependent_option(USE_STATIC_LIBS "Favour static linking of depencendies." OFF BUILD_SHARED_LIBS OFF)

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

set(MINIMUM_BOOST_VERSION 1.58.0)

if(NOT USE_STATIC_LIBS)
    add_definitions(
        -DBOOST_ALL_NO_LIB
        -DBOOST_ALL_DYN_LINK
        -DBOOST_LOG_NO_LIB
        -DBOOST_LOG_DYN_LINK
    )
endif()

set(Boost_USE_STATIC_LIBS ${USE_STATIC_LIBS})
find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS date_time filesystem regex system thread log REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)

if(NOT OPENGL_GLU_FOUND)
    message(FATAL_ERROR "OpenGL GLU library not found.")
endif()

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
set(SDL_NO_MAIN true)

find_package(SDL REQUIRED)


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

add_library(GiGi "")

if(USE_STATIC_LIBS)
    set_target_properties(GiGi
        PROPERTIES
        LINK_SEARCH_END_STATIC true
    )
endif()

target_compile_definitions(GiGi
    PRIVATE
        -DFONS_USE_FREETYPE
        # Starting with boost 1.68 boost::gil integrates support for
        # grayscale-alpha png images, so prefer their implementation
        # instead of our hacky gilext code.
        $<$<VERSION_LESS:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.68>:GIGI_CONFIG_USE_OLD_IMPLEMENTATION_OF_GIL_PNG_IO>
        $<$<VERSION_GREATER:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.67>:BOOST_GIL_IO_ENABLE_GRAY_ALPHA>
)

target_include_directories(GiGi SYSTEM
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
        ${PNG_INCLUDE_DIRS}
        ${TIFF_INCLUDE_DIRS}
)

target_link_libraries(GiGi
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
)

if(PNG_FOUND)
    target_link_libraries(GiGi ${PNG_LIBRARIES})
endif()

if(TIFF_FOUND)
    target_link_libraries(GiGi ${TIFF_LIBRARIES})
endif()


add_library(GiGiSDL "")

if(USE_STATIC_LIBS)
    set_target_properties(GiGiSDL
        PROPERTIES
        LINK_SEARCH_END_STATIC true
    )
endif()

target_include_directories(GiGiSDL SYSTEM
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        ${SDL_INCLUDE_DIR}
)

target_link_libraries(GiGiSDL
    GiGi
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${SDL_LIBRARIES}
)


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


if(UNIX AND NOT APPLE)
    install(
        TARGETS GiGiSDL
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT COMPONENT_GIGISDL
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT COMPONENT_GIGISDL
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT COMPONENT_GIGISDL_DEVEL
    )
endif()

if(UNIX AND BUILD_DEVEL_PACKAGE)
    get_pkg_config_libs(pkg_config_libs ${GiGiSDL_LINK_LIBS})

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GiGiSDL.pc.in
        ${CMAKE_BINARY_DIR}/GiGiSDL.pc
        @ONLY
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/GiGiSDL.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
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

if(BUILD_SDL_DRIVER)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES SDL>=1.2)
endif()

if(BUILD_DEVEL_PACKAGE)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES boost-devel>=1.44.0 Mesa-devel freetype2-devel)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES gigi-devel=${GIGI_VERSION})
endif()

if(BUILD_DEVEL_PACKAGE AND BUILD_SDL_DRIVER)
    list(APPEND CPACK_RPM_PACKAGE_REQUIRES SDL-devel>=1.2)
endif()

# Deb-specific settings
set(CPACK_DEBIAN_PACKAGE_SECTION libs)

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1-mesa-glx | libgl1, libglu1-mesa | libglu, libfreetype6")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, gigi (= ${GIGI_VERSION})")

if(BUILD_SDL_DRIVER)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsdl1.2debian (>= 1.2)")
endif()

if(BUILD_DEVEL_PACKAGE)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "boost-devel (>= 1.44.0), libgl1-mesa-dev | libgl-dev, libglu1-mesa-dev | libglu-dev, libfreetype6-dev")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, gigi-dev (= ${GIGI_VERSION})")
endif()

if(BUILD_DEVEL_PACKAGE AND BUILD_SDL_DRIVER)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsdl1.2-dev (>= 1.2)")
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
    COMPONENT_GIGISDL
    DISPLAY_NAME GiGiSDL
    DESCRIPTION "An SDL input driver (http://www.libsdl.org)."
    GROUP COMPONENT_GROUP_RUNTIME
    DEPENDS COMPONENT_GIGI
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
