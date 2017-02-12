/* GG is a GUI for SDL and OpenGL.

   Copyright (C) 2015 Mitten-O

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

/** \file \brief The implementation of BlockControl.
 */

#include <GG/RichText/BlockControl.h>

namespace GG {

    BlockControl::BlockControl(X x, Y y, X w, GG::Flags<GG::WndFlag> flags):
        Control(x, y, w, Y0, flags)
    {}

    void BlockControl::SizeMove(const Pt& ul, const Pt& lr)
    {
        Pt previous_ul = UpperLeft();
        Pt previous_lr = LowerRight();

        X previous_width = previous_lr.x - previous_ul.x;
        X new_width = lr.x - ul.x;

        Control::SizeMove(ul, lr);

        // Recalculate size if width changed.
        // Block controls only listen to width setting from outside
        // and decide their own height.
        if (new_width != previous_width) {
            SetMaxWidth(lr.x - ul.x);
        }
    }

}

