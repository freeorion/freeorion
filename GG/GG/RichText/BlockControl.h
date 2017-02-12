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

#ifndef BLOCKCONTROL_H
#define BLOCKCONTROL_H

#include <GG/Control.h>

namespace GG {

/** \brief BlockControl is an abstract base class for controls that can determine their height
 * when you set their width.
 *
 * BlockControls are used for embedding controls in text.
 */
class GG_API BlockControl : public Control
{
public:
    //! Create a block control.
    BlockControl(X x, Y y, X w, GG::Flags<GG::WndFlag> flags);

    //! Set the maximum width of the block control. Returns the size based on the width.
    virtual Pt SetMaxWidth(X width) = 0;

    //! Redirect size move to setmaxwidth.
    void SizeMove(const Pt& ul, const Pt& lr) override;
};

}

#endif // BLOCKCONTROL_H
