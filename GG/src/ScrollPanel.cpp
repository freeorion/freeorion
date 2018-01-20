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

#include "../GG/ScrollPanel.h"

#include <GG/ClrConstants.h>
#include <GG/Clr.h>
#include <GG/DrawUtil.h>
#include <GG/Flags.h>
#include <GG/Scroll.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>

#include <algorithm>

namespace GG {

    using std::max;
    using std::min;

    namespace {
        // Minimum acceptable width for a scroll.
        X MIN_SCROLL_WIDTH(14);

        // Space to leave left of the vertical scroll.
        X SCROLL_MARGIN_X(2);
    }

    void ScrollPanel::SizeMove(const Pt& ul, const Pt& lr)
    {
        // Store the new size for input to the layout.
        Wnd::SizeMove(ul, lr);

        // Lay out the children.
        DoLayout();
    }

    void ScrollPanel::Render()
    {
        FlatRectangle(UpperLeft(), LowerRight(), m_background_color, CLR_ZERO, 0);
    }

    ScrollPanel::ScrollPanel():
        m_vscroll(nullptr),
        m_content(nullptr),
        m_background_color(CLR_ZERO)
    {}

    ScrollPanel::ScrollPanel(X x, Y y, X w, Y h, std::shared_ptr<Wnd> content):
        Wnd(x, y, w, h, INTERACTIVE),
        m_vscroll(nullptr),
        m_content(content),
        m_background_color(CLR_ZERO)
    {}

    void ScrollPanel::CompleteConstruction()
    {
        // Very important to clip the content of this panel,
        // to actually only show the currently viewed part.
        SetChildClippingMode(ClipToClient);

        // Get the scroll bar from the current style factory.
        const auto& style = GetStyleFactory();
        m_vscroll = style->NewMultiEditVScroll(CLR_WHITE, CLR_BLACK);

        // Don't accept less than MIN_SCROLL_WIDTH pixels wide scrolls.
        if (m_vscroll->Width() < MIN_SCROLL_WIDTH) {
            m_vscroll->Resize(Pt(MIN_SCROLL_WIDTH, m_vscroll->Height()));
        }

        AttachChild(m_vscroll);
        AttachChild(m_content);

        m_vscroll->ScrolledSignal.connect(boost::bind(&ScrollPanel::OnScrolled, this, _1, _2, _3, _4));

        DoLayout();
    }

    ScrollPanel::~ScrollPanel()
    {}

    void ScrollPanel::ScrollTo(Y pos)
    {
        m_vscroll->ScrollTo(Value(pos));
        SignalScroll(*m_vscroll, true);
    }

    void ScrollPanel::SetBackgroundColor(const Clr& color)
    {
        m_background_color = color;
    }

    void ScrollPanel::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
    {
        m_vscroll->ScrollLineIncr(-move);
        SignalScroll(*m_vscroll, true);
    }

    void ScrollPanel::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
    {
        // unused variable bool shift_down = mod_keys & (MOD_KEY_LSHIFT | MOD_KEY_RSHIFT);
        bool ctrl_down = mod_keys & (MOD_KEY_CTRL | MOD_KEY_RCTRL);
        bool numlock_on = mod_keys & MOD_KEY_NUM;

        if (!numlock_on) {
            // convert keypad keys into corresponding non-number keys
            switch (key) {
            case GGK_KP0:       key = GGK_INSERT;   break;
            case GGK_KP1:       key = GGK_END;      break;
            case GGK_KP2:       key = GGK_DOWN;     break;
            case GGK_KP3:       key = GGK_PAGEDOWN; break;
            case GGK_KP4:       key = GGK_LEFT;     break;
            case GGK_KP5:                           break;
            case GGK_KP6:       key = GGK_RIGHT;    break;
            case GGK_KP7:       key = GGK_HOME;     break;
            case GGK_KP8:       key = GGK_UP;       break;
            case GGK_KP9:       key = GGK_PAGEUP;   break;
            case GGK_KP_PERIOD: key = GGK_DELETE;   break;
            default:                                break;
            }
        }

        switch (key) {
        case GGK_UP: {
            m_vscroll->ScrollLineIncr(-1);
            SignalScroll(*m_vscroll, true);
            break;
        }

        case GGK_DOWN: {
            m_vscroll->ScrollLineIncr(1);
            SignalScroll(*m_vscroll, true);
            break;
        }

        case GGK_HOME: {
            if (ctrl_down) {
                m_vscroll->ScrollTo(m_vscroll->ScrollRange().first);
                SignalScroll(*m_vscroll, true);
            }
            break;
        }

        case GGK_END: {
            if (ctrl_down) {
                m_vscroll->ScrollTo(m_vscroll->ScrollRange().second);
                SignalScroll(*m_vscroll, true);
            }
            break;
        }

        case GGK_PAGEUP: {
            m_vscroll->ScrollPageDecr();
            SignalScroll(*m_vscroll, true);
            break;
        }

        case GGK_PAGEDOWN: {
            m_vscroll->ScrollPageIncr();
            SignalScroll(*m_vscroll, true);
            break;
        }

        default:
            break;
        }
    }

    void ScrollPanel::DoLayout()
    {
        // Position the scroll bar to the right edge of the panel.
        Pt scroll_ul(Width() - m_vscroll->Width(), Y0);
        Pt scroll_lr(Width(), Height() - 1);
        m_vscroll->SizeMove(scroll_ul, scroll_lr);

        // Let the content fill all room left from the scroll.
        Pt content_lr(ClientSize().x - m_vscroll->Width() - SCROLL_MARGIN_X,
                      m_content_pos.y + m_content->Height());
        m_content->SizeMove(m_content_pos, content_lr);

        // The scroll position is directly the offset of the
        // content in pixels.

        // Start from no offset.
        m_vscroll->SetMin(0);

        // Allow the bottom of the content to be scrolled to the middle of the viewport.
        m_vscroll->SetMax(Value(m_content->Height() + Height() / 2));

        // Move a viewportfull at a time with page up and down.
        m_vscroll->SetPageSize(Value(Height()));

        // Make scrolling fairly speedy.
        m_vscroll->SetLineSize(Value(Height() / 10));
    }

    void ScrollPanel::OnScrolled(int tab_min, int tab_max, int min, int max)
    {
        // Move the content up by the offset given by the top of the tab.
        m_content_pos.y = -Y(tab_min);

        // Immediately move the content.
        m_content->MoveTo(m_content_pos);
    }
}
