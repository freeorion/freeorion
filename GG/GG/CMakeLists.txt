# GG is a GUI for SDL and OpenGL.
#
# Copyright (C) 2017 Marcel Metz
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

if(BUILD_SDL_DRIVER)
    add_subdirectory(SDL)
endif()

target_sources(GiGi
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/AlignmentFlags.h
        ${CMAKE_CURRENT_LIST_DIR}/Base.h
        ${CMAKE_CURRENT_LIST_DIR}/BrowseInfoWnd.h
        ${CMAKE_CURRENT_LIST_DIR}/Button.h
        ${CMAKE_CURRENT_LIST_DIR}/ClrConstants.h
        ${CMAKE_CURRENT_LIST_DIR}/Clr.h
        ${CMAKE_CURRENT_LIST_DIR}/Config.h
        ${CMAKE_CURRENT_LIST_DIR}/Control.h
        ${CMAKE_CURRENT_LIST_DIR}/Cursor.h
        ${CMAKE_CURRENT_LIST_DIR}/DeferredLayout.h
        ${CMAKE_CURRENT_LIST_DIR}/DrawUtil.h
        ${CMAKE_CURRENT_LIST_DIR}/DropDownList.h
        ${CMAKE_CURRENT_LIST_DIR}/DynamicGraphic.h
        ${CMAKE_CURRENT_LIST_DIR}/Edit.h
        ${CMAKE_CURRENT_LIST_DIR}/Enum.h
        ${CMAKE_CURRENT_LIST_DIR}/EventPump.h
        ${CMAKE_CURRENT_LIST_DIR}/Exception.h
        ${CMAKE_CURRENT_LIST_DIR}/Export.h
        ${CMAKE_CURRENT_LIST_DIR}/Flags.h
        ${CMAKE_CURRENT_LIST_DIR}/FontFwd.h
        ${CMAKE_CURRENT_LIST_DIR}/Font.h
        ${CMAKE_CURRENT_LIST_DIR}/GGFwd.h
        ${CMAKE_CURRENT_LIST_DIR}/GLClientAndServerBuffer.h
        ${CMAKE_CURRENT_LIST_DIR}/GroupBox.h
        ${CMAKE_CURRENT_LIST_DIR}/GUI.h
        ${CMAKE_CURRENT_LIST_DIR}/Layout.h
        ${CMAKE_CURRENT_LIST_DIR}/ListBox.h
        ${CMAKE_CURRENT_LIST_DIR}/Menu.h
        ${CMAKE_CURRENT_LIST_DIR}/MultiEditFwd.h
        ${CMAKE_CURRENT_LIST_DIR}/MultiEdit.h
        ${CMAKE_CURRENT_LIST_DIR}/PtRect.h
        ${CMAKE_CURRENT_LIST_DIR}/Scroll.h
        ${CMAKE_CURRENT_LIST_DIR}/ScrollPanel.h
        ${CMAKE_CURRENT_LIST_DIR}/Slider.h
        ${CMAKE_CURRENT_LIST_DIR}/Spin.h
        ${CMAKE_CURRENT_LIST_DIR}/StaticGraphic.h
        ${CMAKE_CURRENT_LIST_DIR}/StrongTypedef.h
        ${CMAKE_CURRENT_LIST_DIR}/StyleFactory.h
        ${CMAKE_CURRENT_LIST_DIR}/TabWnd.h
        ${CMAKE_CURRENT_LIST_DIR}/TextControl.h
        ${CMAKE_CURRENT_LIST_DIR}/Texture.h
        ${CMAKE_CURRENT_LIST_DIR}/Timer.h
        ${CMAKE_CURRENT_LIST_DIR}/UnicodeCharsets.h
        ${CMAKE_CURRENT_LIST_DIR}/WndEvent.h
        ${CMAKE_CURRENT_LIST_DIR}/Wnd.h
        ${CMAKE_CURRENT_LIST_DIR}/ZList.h
)
