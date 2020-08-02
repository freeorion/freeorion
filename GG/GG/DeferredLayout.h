//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2016 LGM-Doyle
//!  Copyright (C) 2017-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/DeferredLayout.h
//!
//! Contains the DeferredLayout class, which is used to size and align GG
//! windows in the PreRender() phase.

#ifndef _GG_DeferredLayout_h_
#define _GG_DeferredLayout_h_


#include <GG/Layout.h>


namespace GG {

/** \brief An invisible Wnd subclass that arranges its child Wnds during PreRender.

    A DeferredLayout is a layout that does all of the expensive layout operations once per frame
    during PreRender().
*/
class GG_API DeferredLayout : public Layout
{
public:
    /** Ctor. */
    DeferredLayout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
                   unsigned int border_margin = 0, unsigned int cell_margin = INVALID_CELL_MARGIN);

    void SizeMove(const Pt& ul, const Pt& lr) override;
    void PreRender() override;

protected:
    void RedoLayout() override;

private:
    Pt   m_ul_prerender;
    Pt   m_lr_prerender;
    bool m_make_resize_immediate_during_prerender;
};

}


#endif
