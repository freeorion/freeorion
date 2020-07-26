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
#include <GG/Exception.h>
#include <GG/Flags.h>


namespace GG {

/** Alignment flags. */
GG_FLAG_TYPE(Alignment);
extern GG_API const Alignment ALIGN_NONE;     ///< No alignment selected.
extern GG_API const Alignment ALIGN_VCENTER;  ///< Vertically-centered.
extern GG_API const Alignment ALIGN_TOP;      ///< Aligned to top.
extern GG_API const Alignment ALIGN_BOTTOM;   ///< Aligned to bottom.
extern GG_API const Alignment ALIGN_CENTER;   ///< Horizontally-centered.
extern GG_API const Alignment ALIGN_LEFT;     ///< Aligned to left.
extern GG_API const Alignment ALIGN_RIGHT;    ///< Aligned to right.

}

#endif
