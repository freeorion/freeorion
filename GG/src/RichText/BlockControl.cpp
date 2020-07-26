//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file RichText/BlockControl.cpp
//!
//! The implementation of BlockControl.

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

