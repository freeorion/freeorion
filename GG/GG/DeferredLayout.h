// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.

   Copyright (C) 2016 LGM-Doyle

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

/** \file DeferredLayout.h \brief Contains the DefferedLayout class, which is used to size and
    align GG windows in the PreRender() phase. */

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
    /** \name Structors */ ///@{
    /** Ctor. */
    DeferredLayout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
                   unsigned int border_margin = 0, unsigned int cell_margin = INVALID_CELL_MARGIN);
    //@}

    /** \name Accessors */ ///@{
    //@}

    /** \name Mutators */ ///@{
    void SizeMove(const Pt& ul, const Pt& lr) override;
    void PreRender() override;
    //@}

protected:
    /** \name Mutators */ ///@{
    void RedoLayout() override;
    //@}

private:
    Pt   m_ul_prerender;
    Pt   m_lr_prerender;
    bool m_make_resize_immediate_during_prerender;
};

} // namespace GG

#endif
