//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/ClrConstants.h
//!
//! Contains some useful constants of type Clr.

#ifndef _GG_ClrConstants_h_
#define _GG_ClrConstants_h_


#include <GG/Clr.h>
#include <GG/Export.h>


namespace GG {

// some useful color constants
inline constexpr Clr CLR_ZERO = Clr(0, 0, 0, 0);
inline constexpr Clr CLR_BLACK = Clr(0, 0, 0, 255);
inline constexpr Clr CLR_WHITE = Clr(255, 255, 255, 255);
inline constexpr Clr CLR_GRAY = Clr(127, 127, 127, 255);
inline constexpr Clr CLR_SHADOW = Clr(127, 127, 127, 127);
inline constexpr Clr CLR_RED = Clr(255, 0, 0, 255);
inline constexpr Clr CLR_GREEN = Clr(0, 255, 0, 255);
inline constexpr Clr CLR_BLUE = Clr(0, 0, 255, 255);
inline constexpr Clr CLR_CYAN = Clr(0, 255, 255, 255);
inline constexpr Clr CLR_YELLOW = Clr(255, 255, 0, 255);
inline constexpr Clr CLR_MAGENTA = Clr(255, 0, 255, 255);

inline constexpr Clr CLR_LIGHT_GRAY = Clr(192, 192, 192, 255);
inline constexpr Clr CLR_DARK_GRAY = Clr(64, 64, 64, 255);
inline constexpr Clr CLR_PINK = Clr(255, 127, 127, 255);
inline constexpr Clr CLR_DARK_RED = Clr(127, 0, 0, 255);
inline constexpr Clr CLR_OLIVE = Clr(127, 127, 0, 255);
inline constexpr Clr CLR_DARK_GREEN = Clr(0, 127, 0, 255);
inline constexpr Clr CLR_TEAL = Clr(0, 127, 127, 255);
inline constexpr Clr CLR_DARK_BLUE = Clr(0, 0, 127, 255);
inline constexpr Clr CLR_PURPLE = Clr(127, 0, 127, 255);
inline constexpr Clr CLR_ORANGE = Clr(255, 127, 0, 255);

}


#endif
