# GG is a GUI for SDL and OpenGL.
#
# Copyright (C) 2009 T. Zachary Laine
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

set(SDL_NO_MAIN true)

find_package(SDL REQUIRED)

include_directories(
    SYSTEM
    ${SDL_INCLUDE_DIRS}
)

set(GiGiSDL_LINK_LIBS GiGi ${SDL_LIBRARIES})
add_library(GiGiSDL SDLGUI.cpp)

if (USE_STATIC_LIBS)
    set_target_properties(GiGiSDL
        PROPERTIES
        LINK_SEARCH_END_STATIC true
    )
endif ()

target_link_libraries(GiGiSDL ${GiGiSDL_LINK_LIBS})

install(
    TARGETS GiGiSDL
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT COMPONENT_GIGISDL
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_GIGISDL
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT COMPONENT_GIGISDL_DEVEL
)

if (UNIX AND BUILD_DEVEL_PACKAGE)
    get_pkg_config_libs(pkg_config_libs ${GiGiSDL_LINK_LIBS})

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/GiGiSDL.pc.in
        ${CMAKE_BINARY_DIR}/GiGiSDL.pc
        @ONLY
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/GiGiSDL.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        COMPONENT COMPONENT_GIGI_DEVEL
        OPTIONAL
    )
endif ()
