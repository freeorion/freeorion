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
extern GG_API const TextFormat FORMAT_NONE;        ///< Default format selected.
extern GG_API const TextFormat FORMAT_VCENTER;     ///< Centers text vertically.
extern GG_API const TextFormat FORMAT_TOP;         ///< Top-justifies text.
extern GG_API const TextFormat FORMAT_BOTTOM;      ///< Justifies the text to the bottom of the rectangle.
extern GG_API const TextFormat FORMAT_CENTER;      ///< Centers text horizontally in the rectangle.
extern GG_API const TextFormat FORMAT_LEFT;        ///< Aligns text to the left.
extern GG_API const TextFormat FORMAT_RIGHT;       ///< Aligns text to the right.
extern GG_API const TextFormat FORMAT_NOWRAP;      ///< Resize control to fit text, don't wrap text. Text only breaks at '\\n'.
extern GG_API const TextFormat FORMAT_WORDBREAK;   ///< Breaks words. Lines are automatically broken between words if a word would extend past the edge of the control's bounding rectangle.  As always, a '\\n' also breaks the line.
extern GG_API const TextFormat FORMAT_LINEWRAP;    ///< Lines are automatically broken when the next glyph would be drawn outside the the text rectangle.  As always, a '\\n' also breaks the line.
extern GG_API const TextFormat FORMAT_IGNORETAGS;  ///< Text formatting tags (e.g. <rgba 0 0 0 255>) are treated as regular text.

#ifdef _MSC_VER
 #pragma warning(push)
 #pragma warning(disable: 4146) // unary minus operator applied to unsigned type, result still unsigned (triggered by defining - on a size_t wrapper)
#endif
/** \class GG::StrSize
    \brief The string size and index value type.

    Such values refer to indices into UTF-8 encoded strings, \a not code
    points.  \see GG_STRONG_SIZE_TYPEDEF */
GG_STRONG_SIZE_TYPEDEF(StrSize);

/** \class GG::CPSize
    \brief The code point size and index value type.

    Such values refer to indices of code points in Unicode strings, \a not
    indices into underlying UTF-8 encoded strings.  \see
    GG_STRONG_SIZE_TYPEDEF */
GG_STRONG_SIZE_TYPEDEF(CPSize);
#ifdef _MSC_VER
 #pragma warning(pop)
#endif

// some useful size constants
extern GG_API const StrSize S0;
extern GG_API const StrSize S1;
extern GG_API const StrSize INVALID_STR_SIZE;
extern GG_API const CPSize CP0;
extern GG_API const CPSize CP1;
extern GG_API const CPSize INVALID_CP_SIZE;

}

#endif
