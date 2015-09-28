// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2015 Marcel Metz

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
*/

/** \file FontFwd.h \brief Contains forward declaration of all public
    GG control and utility classes. */

#ifndef _GG_GGFwd_h_
#define _GG_GGFwd_h_

namespace GG {
    class Button;
    class DropDownList;
    class DynamicGraphic;
    class Edit;
    class Font;
    class ListBox;
    class MultiEdit;
    class RadioButtonGroup;
    class Scroll;
    class ScrollPanel;
    class StaticGraphic;
    class SubTexture;
    class TabWnd;
    class Texture;
    class TextControl;
    class RichText;
    struct Clr;

    typedef TextControl Label;

    template <class T>
    class Spin;
}

#endif
