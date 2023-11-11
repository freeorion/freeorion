//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2007 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/AlignmentFlags.h
//!
//! Contains the Alignment flag type and the global alignment flag constants.

#ifndef _GG_AlignmentFlags_h_
#define _GG_AlignmentFlags_h_


#include <GG/Base.h>
#include <GG/Flags.h>


namespace GG {

/** Alignment flags. */
GG_FLAG_TYPE(Alignment);

inline constexpr Alignment ALIGN_NONE      (0);        ///< No alignment selected.
inline constexpr Alignment ALIGN_VCENTER   (1 << 0);   ///< Vertically-centered.
inline constexpr Alignment ALIGN_TOP       (1 << 1);   ///< Aligned to top.
inline constexpr Alignment ALIGN_BOTTOM    (1 << 2);   ///< Aligned to bottom.
inline constexpr Alignment ALIGN_CENTER    (1 << 3);   ///< Horizontally-centered.
inline constexpr Alignment ALIGN_LEFT      (1 << 4);   ///< Aligned to left.
inline constexpr Alignment ALIGN_RIGHT     (1 << 5);   ///< Aligned to right.

}


#endif
