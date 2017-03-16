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

target_sources(GiGi
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/BlockControl.h
        ${CMAKE_CURRENT_LIST_DIR}/ImageBlock.h
        ${CMAKE_CURRENT_LIST_DIR}/RichText.h
)
