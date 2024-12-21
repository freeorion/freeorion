//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2016 LGM Doyle
//!  Copyright (C) 2017-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/DeferredLayout.h>


using namespace GG;

DeferredLayout::DeferredLayout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
                               unsigned int border_margin,
                               unsigned int cell_margin) :
    Layout(x, y, w, h, rows, columns, border_margin, cell_margin),
    m_ul_prerender(Pt(x, y)),
    m_lr_prerender(Pt(x + w, y + h))
{}

void DeferredLayout::SizeMove(Pt ul, Pt lr)
{
    if (m_make_resize_immediate_during_prerender) {
        if (ul != m_ul_prerender || lr != m_lr_prerender)
            DoLayout(ul, lr);
        return;
    }

    if ((ul != RelativeUpperLeft()) || (lr != RelativeLowerRight())) {
        RequirePreRender();

        m_ul_prerender = ul;
        m_lr_prerender = lr;
    }

    // Note: m_upperleft and m_lowerright will be updated when DoLayout() is called in PreRender().
}

void DeferredLayout::PreRender()
{
    Layout::PreRender();
    ScopedAssign assignment(m_make_resize_immediate_during_prerender, true);
    DoLayout(m_ul_prerender, m_lr_prerender);
    m_ul_prerender = RelativeUpperLeft();
    m_lr_prerender = RelativeLowerRight();
}

void DeferredLayout::RedoLayout()
{ RequirePreRender(); }
