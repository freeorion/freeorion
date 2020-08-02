# GiGi - A GUI for OpenGL
#
#  Copyright (C) 2017 Marcel Metz
#  Copyright (C) 2018-2020 The FreeOrion Project
#
# Released under the GNU Lesser General Public License 2.1 or later.
# Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
# SPDX-License-Identifier: LGPL-2.1-or-later

add_subdirectory(dialogs)
add_subdirectory(RichText)

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
