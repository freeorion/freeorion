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

/** \file FontFwd.h \brief Contains forward declaration of Font, the
    TextFormat flags, StrSize, and CPSize. */

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
