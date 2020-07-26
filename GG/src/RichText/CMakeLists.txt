# GiGi - A GUI for OpenGL
#
#  Copyright (C) 2017 Marcel Metz
#  Copyright (C) 2018-2020 The FreeOrion Project
#
# Released under the GNU Lesser General Public License 2.1 or later.
# Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
# SPDX-License-Identifier: LGPL-2.1-or-later

target_sources(GiGi
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/BlockControl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ImageBlock.cpp
        ${CMAKE_CURRENT_LIST_DIR}/RichText.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TagParser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TextBlock.cpp
)
