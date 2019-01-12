// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.

   Zach Laine
   whatwasthataddress@gmail.com */

/** \file ClrConstants.h \brief Contains some useful constants of type Clr. */

#ifndef _GG_ClrConstants_h_
#define _GG_ClrConstants_h_

#include <GG/Clr.h>
#include <GG/Export.h>

namespace GG {

// some useful color constants
constexpr Clr CLR_ZERO = Clr(0, 0, 0, 0);
constexpr Clr CLR_BLACK = Clr(0, 0, 0, 255);
constexpr Clr CLR_WHITE = Clr(255, 255, 255, 255);
constexpr Clr CLR_GRAY = Clr(127, 127, 127, 255);
constexpr Clr CLR_SHADOW = Clr(127, 127, 127, 127);
constexpr Clr CLR_RED = Clr(255, 0, 0, 255);
constexpr Clr CLR_GREEN = Clr(0, 255, 0, 255);
constexpr Clr CLR_BLUE = Clr(0, 0, 255, 255);
constexpr Clr CLR_CYAN = Clr(0, 255, 255, 255);
constexpr Clr CLR_YELLOW = Clr(255, 255, 0, 255);
constexpr Clr CLR_MAGENTA = Clr(255, 0, 255, 255);

constexpr Clr CLR_LIGHT_GRAY = Clr(192, 192, 192, 255);
constexpr Clr CLR_DARK_GRAY = Clr(64, 64, 64, 255);
constexpr Clr CLR_PINK = Clr(255, 127, 127, 255);
constexpr Clr CLR_DARK_RED = Clr(127, 0, 0, 255);
constexpr Clr CLR_OLIVE = Clr(127, 127, 0, 255);
constexpr Clr CLR_DARK_GREEN = Clr(0, 127, 0, 255);
constexpr Clr CLR_TEAL = Clr(0, 127, 127, 255);
constexpr Clr CLR_DARK_BLUE = Clr(0, 0, 127, 255);
constexpr Clr CLR_PURPLE = Clr(127, 0, 127, 255);
constexpr Clr CLR_ORANGE = Clr(255, 127, 0, 255);

} // namespace GG

#endif

