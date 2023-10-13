# GiGi - A GUI for OpenGL
#
#  Copyright (C) 2009-2011 T. Zachary Laine <whatwasthataddress@gmail.com>
#  Copyright (C) 2013-2020 The FreeOrion Project
#
# Released under the GNU Lesser General Public License 2.1 or later.
# Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
# SPDX-License-Identifier: LGPL-2.1-or-later

add_subdirectory(dialogs)
add_subdirectory(RichText)

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
        ${CMAKE_CURRENT_LIST_DIR}/TabWnd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TextControl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Texture.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Timer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/UnicodeCharsets.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Wnd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/WndEvent.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ZList.cpp
)
