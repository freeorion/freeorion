//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/MultiEditFwd.h
//!
//! Contains forward declaration of the MultiEdit class, and the MultiEditStyle
//! flags.

#ifndef _GG_MultiEditFwd_h_
#define _GG_MultiEditFwd_h_


#include <GG/Flags.h>


namespace GG {

class MultiEdit;

/** The styles of display and interaction for a MultiEdit. */
GG_FLAG_TYPE(MultiEditStyle);

inline constexpr MultiEditStyle MULTI_NONE             (0);        ///< Default style selected.
inline constexpr MultiEditStyle MULTI_WORDBREAK        (1 << 0);   ///< Breaks words. Lines are automatically broken between words if a word would extend past the edge of the control's bounding rectangle. (As always, a '\\n' also breaks the line.)
inline constexpr MultiEditStyle MULTI_LINEWRAP         (1 << 1);   ///< Lines are automatically broken when the next character (or space) would be drawn outside the text rectangle.
inline constexpr MultiEditStyle MULTI_VCENTER          (1 << 2);   ///< Vertically centers text.
inline constexpr MultiEditStyle MULTI_TOP              (1 << 3);   ///< Aligns text to the top.
inline constexpr MultiEditStyle MULTI_BOTTOM           (1 << 4);   ///< Aligns text to the bottom.
inline constexpr MultiEditStyle MULTI_CENTER           (1 << 5);   ///< Centers text.
inline constexpr MultiEditStyle MULTI_LEFT             (1 << 6);   ///< Aligns text to the left.
inline constexpr MultiEditStyle MULTI_RIGHT            (1 << 7);   ///< Aligns text to the right.
inline constexpr MultiEditStyle MULTI_READ_ONLY        (1 << 8);   ///< The control is not user-interactive, only used to display text.  Text can still be programmatically altered and selected.
inline constexpr MultiEditStyle MULTI_TERMINAL_STYLE   (1 << 9);   ///< The text in the control is displayed so that the bottom is visible, instead of the top.
inline constexpr MultiEditStyle MULTI_INTEGRAL_HEIGHT  (1 << 10);  ///< The height of the control will always be a multiple of the height of one row (fractions rounded down).
inline constexpr MultiEditStyle MULTI_NO_VSCROLL       (1 << 11);  ///< Vertical scrolling is not available, and there is no vertical scroll bar.
inline constexpr MultiEditStyle MULTI_NO_HSCROLL       (1 << 12);  ///< Horizontal scrolling is not available, and there is no horizontal scroll bar.

inline constexpr Flags<MultiEditStyle> MULTI_NO_SCROLL{MULTI_NO_VSCROLL | MULTI_NO_HSCROLL}; ///< Scrolls are not used for this control.

}


#endif
