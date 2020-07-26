//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _GG_RichText_BlockControl_h_
#define _GG_RichText_BlockControl_h_


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


#endif
