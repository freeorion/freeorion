/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2016-2016 LGM Doyle

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
   */

#include <GG/DeferredLayout.h>

using namespace GG;

DeferredLayout::DeferredLayout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
                               unsigned int border_margin/* = 0*/,
                               unsigned int cell_margin/* = INVALID_CELL_MARGIN*/) :
    Layout(x, y, w, h, rows, columns, border_margin, cell_margin),
    m_ul_prerender(Pt(x, y)),
    m_lr_prerender(Pt(x + w, y + h)),
    m_make_resize_immediate_during_prerender(false)
{}

void DeferredLayout::SizeMove(const Pt& ul, const Pt& lr)
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
    ScopedAssign<bool> assignment(m_make_resize_immediate_during_prerender, true);
    DoLayout(m_ul_prerender, m_lr_prerender);
    m_ul_prerender = RelativeUpperLeft();
    m_lr_prerender = RelativeLowerRight();
}

void DeferredLayout::RedoLayout()
{ RequirePreRender(); }
