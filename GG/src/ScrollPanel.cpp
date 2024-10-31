//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>
#include <GG/ClrConstants.h>
#include <GG/Clr.h>
#include <GG/DrawUtil.h>
#include <GG/Flags.h>
#include <GG/Scroll.h>
#include <GG/ScrollPanel.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

using std::max;
using std::min;

namespace {
    // Minimum acceptable width for a scroll.
    constexpr X MIN_SCROLL_WIDTH{14};

    // Space to leave left of the vertical scroll.
    constexpr X SCROLL_MARGIN_X{2};
}

void ScrollPanel::SizeMove(Pt ul, Pt lr)
{
    // Store the new size for input to the layout.
    Wnd::SizeMove(ul, lr);

    // Lay out the children.
    DoLayout();
}

void ScrollPanel::Render()
{ FlatRectangle(UpperLeft(), LowerRight(), m_background_color, CLR_ZERO, 0); }

void ScrollPanel::CompleteConstruction()
{
    // Very important to clip the content of this panel,
    // to actually only show the currently viewed part.
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    // Get the scroll bar from the current style factory.
    const auto& style = GetStyleFactory();
    m_vscroll = style.NewMultiEditVScroll(CLR_WHITE, CLR_BLACK);

    // Don't accept less than MIN_SCROLL_WIDTH pixels wide scrolls.
    if (m_vscroll->Width() < MIN_SCROLL_WIDTH) {
        m_vscroll->Resize(Pt(MIN_SCROLL_WIDTH, m_vscroll->Height()));
    }

    AttachChild(m_vscroll);
    AttachChild(m_content);

    namespace ph = boost::placeholders;

    m_vscroll->ScrolledSignal.connect(boost::bind(&ScrollPanel::OnScrolled, this, ph::_1, ph::_2, ph::_3, ph::_4));

    DoLayout();
}

void ScrollPanel::ScrollTo(Y pos)
{
    m_vscroll->ScrollTo(Value(pos));
    SignalScroll(*m_vscroll, true);
}

void ScrollPanel::MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys)
{
    m_vscroll->ScrollLineIncr(-move);
    SignalScroll(*m_vscroll, true);
}

void ScrollPanel::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    // unused variable bool shift_down = mod_keys & (MOD_KEY_LSHIFT | MOD_KEY_RSHIFT);
    bool ctrl_down = mod_keys & (MOD_KEY_CTRL | MOD_KEY_RCTRL);
    bool numlock_on = mod_keys & MOD_KEY_NUM;

    if (!numlock_on) {
        // convert keypad keys into corresponding non-number keys
        switch (key) {
        case Key::GGK_KP0:       key = Key::GGK_INSERT;   break;
        case Key::GGK_KP1:       key = Key::GGK_END;      break;
        case Key::GGK_KP2:       key = Key::GGK_DOWN;     break;
        case Key::GGK_KP3:       key = Key::GGK_PAGEDOWN; break;
        case Key::GGK_KP4:       key = Key::GGK_LEFT;     break;
        case Key::GGK_KP5:                                break;
        case Key::GGK_KP6:       key = Key::GGK_RIGHT;    break;
        case Key::GGK_KP7:       key = Key::GGK_HOME;     break;
        case Key::GGK_KP8:       key = Key::GGK_UP;       break;
        case Key::GGK_KP9:       key = Key::GGK_PAGEUP;   break;
        case Key::GGK_KP_PERIOD: key = Key::GGK_DELETE;   break;
        default:                                          break;
        }
    }

    switch (key) {
    case Key::GGK_UP: {
        m_vscroll->ScrollLineIncr(-1);
        SignalScroll(*m_vscroll, true);
        break;
    }

    case Key::GGK_DOWN: {
        m_vscroll->ScrollLineIncr(1);
        SignalScroll(*m_vscroll, true);
        break;
    }

    case Key::GGK_HOME: {
        if (ctrl_down) {
            m_vscroll->ScrollTo(m_vscroll->ScrollRange().first);
            SignalScroll(*m_vscroll, true);
        }
        break;
    }

    case Key::GGK_END: {
        if (ctrl_down) {
            m_vscroll->ScrollTo(m_vscroll->ScrollRange().second);
            SignalScroll(*m_vscroll, true);
        }
        break;
    }

    case Key::GGK_PAGEUP: {
        m_vscroll->ScrollPageDecr();
        SignalScroll(*m_vscroll, true);
        break;
    }

    case Key::GGK_PAGEDOWN: {
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
