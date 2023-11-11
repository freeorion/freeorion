//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/FontFwd.h
//!
//! Contains forward declaration of Font, the TextFormat flags, StrSize, and
//! CPSize.

#ifndef _GG_FontFwd_h_
#define _GG_FontFwd_h_


#include <GG/Flags.h>
#include <GG/StrongTypedef.h>

namespace GG {

class Font;

/** Text formatting flags. */
GG_FLAG_TYPE(TextFormat);

inline constexpr TextFormat FORMAT_NONE      (0);      ///< Default format selected.
inline constexpr TextFormat FORMAT_VCENTER   (1 << 0); ///< Centers text vertically.
inline constexpr TextFormat FORMAT_TOP       (1 << 1); ///< Top-justifies text.
inline constexpr TextFormat FORMAT_BOTTOM    (1 << 2); ///< Justifies the text to the bottom of the rectangle.
inline constexpr TextFormat FORMAT_CENTER    (1 << 3); ///< Centers text horizontally in the rectangle.
inline constexpr TextFormat FORMAT_LEFT      (1 << 4); ///< Aligns text to the left.
inline constexpr TextFormat FORMAT_RIGHT     (1 << 5); ///< Aligns text to the right.
inline constexpr TextFormat FORMAT_NOWRAP    (1 << 6); ///< Resize control to fit text, don't wrap text. Text only breaks at '\\n'.
inline constexpr TextFormat FORMAT_WORDBREAK (1 << 7); ///< Breaks words. Lines are automatically broken between words if a word would extend past the edge of the control's bounding rectangle.  As always, a '\\n' also breaks the line.
inline constexpr TextFormat FORMAT_LINEWRAP  (1 << 8); ///< Lines are automatically broken when the next glyph would be drawn outside the text rectangle.  As always, a '\\n' also breaks the line.
inline constexpr TextFormat FORMAT_IGNORETAGS(1 << 9); ///< Text formatting tags (e.g. <rgba 0 0 0 255>) are treated as regular text.


#ifdef _MSC_VER
 #pragma warning(push)
 #pragma warning(disable: 4146) // unary minus operator applied to unsigned type, result still unsigned (triggered by defining - on a size_t wrapper)
#endif
/** \class GG::StrSize
    \brief The string size and index value type.

    Such values refer to indices into UTF-8 encoded strings, \a not code
    points.  \see GG_STRONG_SIZE_TYPEDEF */
SIZE_TYPEDEF(StrSize, S);

/** \class GG::CPSize
    \brief The code point size and index value type.

    Such values refer to indices of code points in Unicode strings, \a not
    indices into underlying UTF-8 encoded strings.  \see
    GG_STRONG_SIZE_TYPEDEF */
SIZE_TYPEDEF(CPSize, CP);
#ifdef _MSC_VER
 #pragma warning(pop)
#endif

}

#endif
