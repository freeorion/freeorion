# GG is a GUI for SDL and OpenGL.
#
# Copyright (C) 2009-2011 T. Zachary Laine
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

add_subdirectory(dialogs)
add_subdirectory(RichText)
add_subdirectory(nanovg)

if(BUILD_SDL_DRIVER)
    add_subdirectory(SDL)
endif()

target_sources(GiGi
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/AlignmentFlags.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Base.cpp
        ${CMAKE_CURRENT_LIST_DIR}/BrowseInfoWnd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Button.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Control.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Cursor.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DeferredLayout.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DrawUtil.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DropDownList.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DynamicGraphic.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Edit.cpp
        ${CMAKE_CURRENT_LIST_DIR}/EventPump.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Font.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GLClientAndServerBuffer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GroupBox.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GUI.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Layout.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ListBox.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Menu.cpp
        ${CMAKE_CURRENT_LIST_DIR}/MultiEdit.cpp
        ${CMAKE_CURRENT_LIST_DIR}/PtRect.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Scroll.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ScrollPanel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/StaticGraphic.cpp
        ${CMAKE_CURRENT_LIST_DIR}/StyleFactory.cpp
        ${CMAKE_CURRENT_LIST_DIR}/svg.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TabWnd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TextControl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Texture.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Timer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/UnicodeCharsets.cpp
        ${CMAKE_CURRENT_LIST_DIR}/VectorTexture.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Wnd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/WndEvent.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ZList.cpp
)
