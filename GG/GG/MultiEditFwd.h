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

/** \file MultiEditFwd.h \brief Contains forward declaration of the MultiEdit
    class, and the MultiEditStyle flags. */

#ifndef _GG_MultiEditFwd_h_
#define _GG_MultiEditFwd_h_

#include <GG/Flags.h>


namespace GG {

class MultiEdit;

/** The styles of display and interaction for a MultiEdit. */
GG_FLAG_TYPE(MultiEditStyle);
extern GG_API const MultiEditStyle MULTI_NONE;             ///< Default style selected.
extern GG_API const MultiEditStyle MULTI_WORDBREAK;        ///< Breaks words. Lines are automatically broken between words if a word would extend past the edge of the control's bounding rectangle. (As always, a '\\n' also breaks the line.)
extern GG_API const MultiEditStyle MULTI_LINEWRAP;         ///< Lines are automatically broken when the next character (or space) would be drawn outside the the text rectangle.
extern GG_API const MultiEditStyle MULTI_VCENTER;          ///< Vertically centers text. 
extern GG_API const MultiEditStyle MULTI_TOP;              ///< Aligns text to the top. 
extern GG_API const MultiEditStyle MULTI_BOTTOM;           ///< Aligns text to the bottom. 
extern GG_API const MultiEditStyle MULTI_CENTER;           ///< Centers text. 
extern GG_API const MultiEditStyle MULTI_LEFT;             ///< Aligns text to the left. 
extern GG_API const MultiEditStyle MULTI_RIGHT;            ///< Aligns text to the right. 
extern GG_API const MultiEditStyle MULTI_READ_ONLY;        ///< The control is not user-interactive, only used to display text.  Text can still be programmatically altered and selected.
extern GG_API const MultiEditStyle MULTI_TERMINAL_STYLE;   ///< The text in the control is displayed so that the bottom is visible, instead of the top.
extern GG_API const MultiEditStyle MULTI_INTEGRAL_HEIGHT;  ///< The height of the control will always be a multiple of the height of one row (fractions rounded down).
extern GG_API const MultiEditStyle MULTI_NO_VSCROLL;       ///< Vertical scrolling is not available, and there is no vertical scroll bar.
extern GG_API const MultiEditStyle MULTI_NO_HSCROLL;       ///< Horizontal scrolling is not available, and there is no horizontal scroll bar.
extern GG_API const Flags<MultiEditStyle> MULTI_NO_SCROLL; ///< Scrolls are not used for this control.

}

#endif
