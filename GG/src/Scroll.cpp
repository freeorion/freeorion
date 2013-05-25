/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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

#include <GG/Scroll.h>

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    struct ScrolledEcho
    {
        ScrolledEcho(const std::string& name) : m_name(name) {}
        void operator()(int tab_min, int tab_max, int scroll_min, int scroll_max)
            {
                std::cerr << "GG SIGNAL : " << m_name
                          << "(tab_min=" << tab_min << " tab_max=" << tab_max
                          << " scroll_min=" << scroll_min << " scroll_max=" << scroll_max
                          << ")\n";
            }
        std::string m_name;
    };

    const unsigned int MIN_TAB_SIZE = 5;
}

////////////////////////////////////////////////
// GG::Scroll
////////////////////////////////////////////////
Scroll::Scroll() :
    Control(),
    m_orientation(VERTICAL),
    m_posn(0),
    m_range_min(0),
    m_range_max(99),
    m_line_sz(5),
    m_page_sz(25),
    m_tab(0),
    m_incr(0),
    m_decr(0),
    m_initial_depressed_region(SBR_NONE),
    m_depressed_region(SBR_NONE),
    m_dragging_tab(false),
    m_tab_dragged(false)
{}

Scroll::Scroll(X x, Y y, X w, Y h, Orientation orientation, Clr color, Clr interior, Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) :
    Control(x, y, w, h, flags),
    m_int_color(interior),
    m_orientation(orientation),
    m_posn(0),
    m_range_min(0),
    m_range_max(99),
    m_line_sz(5),
    m_page_sz(25),
    m_tab(0),
    m_incr(0),
    m_decr(0),
    m_initial_depressed_region(SBR_NONE),
    m_depressed_region(SBR_NONE),
    m_dragging_tab(false),
    m_tab_dragged(false)
{
    Control::SetColor(color);
    boost::shared_ptr<Font> null_font;
    boost::shared_ptr<StyleFactory> style = GetStyleFactory();
    if (m_orientation == VERTICAL) {
        m_decr = style->NewScrollUpButton(X0, Y0, w, Y(Value(w)), "", null_font, color);
        m_incr = style->NewScrollDownButton(X0, h - Value(w), w, Y(Value(w)), "", null_font, color);
        m_tab = style->NewVScrollTabButton(X0, Y(Value(w)), w, Y(TabWidth()), "", null_font, color);
    } else {
        m_decr = style->NewScrollLeftButton(X0, Y0, X(Value(h)), h, "", null_font, color);
        m_incr = style->NewScrollRightButton(w - Value(h), Y0, X(Value(h)), h, "", null_font, color);
        m_tab = style->NewHScrollTabButton(X(Value(h)), Y0, X(TabWidth()), h, "", null_font, color);
    }
    AttachChild(m_decr);
    AttachChild(m_incr);
    AttachChild(m_tab);
    Connect(m_decr->LeftClickedSignal, boost::bind(&Scroll::ScrollLineDecrImpl, this, true));
    Connect(m_incr->LeftClickedSignal, boost::bind(&Scroll::ScrollLineIncrImpl, this, true));
    m_tab->InstallEventFilter(this);

    if (INSTRUMENT_ALL_SIGNALS) {
        Connect(ScrolledSignal, ScrolledEcho("Scroll::ScrolledSignal"));
        Connect(ScrolledAndStoppedSignal, ScrolledEcho("Scroll::ScrolledAndStoppedSignal"));
    }
}

Pt Scroll::MinUsableSize() const
{
    Pt retval;
    const int MIN_DRAGABLE_SIZE = 2;
    if (m_orientation == VERTICAL) {
        retval.x = X(MIN_DRAGABLE_SIZE);
        Y decr_y = m_decr->MinUsableSize().y;
        Y incr_y = m_incr->MinUsableSize().y;
        retval.y = decr_y + incr_y + 3 * std::min(decr_y, incr_y);
    } else {
        X decr_x = m_decr->MinUsableSize().x;
        X incr_x = m_incr->MinUsableSize().x;
        retval.x = decr_x + incr_x + 3 * std::min(decr_x, incr_x);
        retval.y = Y(MIN_DRAGABLE_SIZE);
    }
    return retval;
}

std::pair<int, int> Scroll::PosnRange() const
{ return std::pair<int, int>(m_posn, m_posn + m_page_sz); }

std::pair<int, int> Scroll::ScrollRange() const
{ return std::pair<int, int>(m_range_min, m_range_max); }

unsigned int Scroll::LineSize() const
{ return m_line_sz; }

unsigned int Scroll::PageSize() const
{ return m_page_sz; }
    
Clr Scroll::InteriorColor() const
{ return m_int_color; }

Orientation Scroll::ScrollOrientation() const
{ return m_orientation; }

void Scroll::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    FlatRectangle(ul, lr, int_color_to_use, CLR_ZERO, 0);
}

void Scroll::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    int bn_width = (m_orientation == VERTICAL) ? Value(Size().x) : Value(Size().y);
    m_decr->SizeMove(Pt(), Pt(X(bn_width), Y(bn_width)));
    m_incr->SizeMove(Size() - Pt(X(bn_width), Y(bn_width)), Size());
    m_tab->SizeMove(m_tab->RelativeUpperLeft(),
                    (m_orientation == VERTICAL) ?
                    Pt(X(bn_width), m_tab->RelativeLowerRight().y) :
                    Pt(m_tab->RelativeLowerRight().x, Y(bn_width)));
    SizeScroll(m_range_min, m_range_max, m_line_sz, m_page_sz); // update tab size and position
}

void Scroll::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    m_tab->Disable(b);
    m_incr->Disable(b);
    m_decr->Disable(b);
}

void Scroll::SetColor(Clr c)
{
    Control::SetColor(c);
    m_tab->SetColor(c);
    m_incr->SetColor(c);
    m_decr->SetColor(c);
}

void Scroll::SetInteriorColor(Clr c)
{ m_int_color = c; }

void Scroll::SizeScroll(int min, int max, unsigned int line, unsigned int page)
{
    m_line_sz = line;
    m_range_min = std::min(min, max);
    m_range_max = std::max(min, max);
    m_page_sz = page;

    assert(static_cast<int>(m_page_sz) <= m_range_max - m_range_min + 1);

    if (m_page_sz > static_cast<unsigned int>(m_range_max - m_range_min + 1))
        m_page_sz = (m_range_max - m_range_min + 1);
    if (m_posn > m_range_max - static_cast<int>(m_page_sz - 1))
        m_posn = m_range_max - (m_page_sz - 1);
    if (m_posn < m_range_min)
        m_posn = m_range_min;
    Pt tab_ul = m_tab->RelativeUpperLeft();
    Pt tab_lr = m_orientation == VERTICAL ?
        Pt(m_tab->RelativeLowerRight().x, tab_ul.y + static_cast<int>(TabWidth())):
        Pt(tab_ul.x + static_cast<int>(TabWidth()), m_tab->RelativeLowerRight().y);
    m_tab->SizeMove(tab_ul, tab_lr);
    MoveTabToPosn();
}

void Scroll::SetMax(int max)        
{ SizeScroll(m_range_min, max, m_line_sz, m_page_sz); }

void Scroll::SetMin(int min)        
{ SizeScroll(min, m_range_max, m_line_sz, m_page_sz); }

void Scroll::SetLineSize(unsigned int line)
{ SizeScroll(m_range_min, m_range_max, line, m_page_sz); }

void Scroll::SetPageSize(unsigned int page)  
{ SizeScroll(m_range_min, m_range_max, m_line_sz, page); }

void Scroll::ScrollTo(int p)
{
    if (p < m_range_min)
        m_posn = m_range_min;
    else if (p > static_cast<int>(m_range_max - m_page_sz))
        m_posn = m_range_max - m_page_sz;
    else
        m_posn = p;
    MoveTabToPosn();
}

void Scroll::ScrollLineIncr()
{ ScrollLineIncrImpl(false); }

void Scroll::ScrollLineDecr()
{ ScrollLineDecrImpl(false); }

void Scroll::ScrollPageIncr()
{
    if (static_cast<int>(m_posn + m_page_sz) <= static_cast<int>(m_range_max - m_page_sz))
        m_posn += m_page_sz;
    else
        m_posn = m_range_max - (m_page_sz - 1);
    MoveTabToPosn();
}

void Scroll::ScrollPageDecr()
{
    if (static_cast<int>(m_posn - m_page_sz) >= m_range_min)
        m_posn -= m_page_sz;
    else
        m_posn = m_range_min;
    MoveTabToPosn();
}

unsigned int Scroll::TabSpace() const
{
    // tab_space is the space the tab has to move about in (the control's width less the width of the incr & decr buttons)
    return (m_orientation == VERTICAL ?
            Value(Size().y - m_incr->Size().y - m_decr->Size().y) :
            Value(Size().x - m_incr->Size().x - m_decr->Size().x));
}

unsigned int Scroll::TabWidth() const
{ return std::max(static_cast<unsigned int>(TabSpace() / (m_range_max - m_range_min + 1.0) * m_page_sz + 0.5), MIN_TAB_SIZE); }

Scroll::ScrollRegion Scroll::RegionUnder(const Pt& pt)
{
    ScrollRegion retval;
    Pt ul = ClientUpperLeft();
    if (pt.x - ul.x < m_tab->RelativeUpperLeft().x || pt.y - ul.y <= m_tab->RelativeUpperLeft().y)
        retval = SBR_PAGE_DN;
    else
        retval = SBR_PAGE_UP;
    return retval;
}

Button* Scroll::TabButton() const
{ return m_tab; }

Button* Scroll::IncrButton() const
{ return m_incr; }

Button* Scroll::DecrButton() const
{ return m_decr; }

void Scroll::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        // when a button is pressed, record the region of the control the cursor is over
        ScrollRegion region = RegionUnder(pt);
        if (m_initial_depressed_region == SBR_NONE)
            m_initial_depressed_region = region;
        m_depressed_region = region;
        if (m_depressed_region == m_initial_depressed_region) {
            switch (m_depressed_region)
            {
            case SBR_PAGE_DN: {
                int old_posn = m_posn;
                ScrollPageDecr();
                if (old_posn != m_posn) {
                    ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
                    ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
                }
                break;
            }
            case SBR_PAGE_UP: {
                int old_posn = m_posn;
                ScrollPageIncr();
                if (old_posn != m_posn) {
                    ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
                    ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
                }
                break;
            }
            default: break;
            }
        }
    }
}

void Scroll::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_decr->SetState(Button::BN_UNPRESSED);
        m_incr->SetState(Button::BN_UNPRESSED);
        m_initial_depressed_region = SBR_NONE;
        m_depressed_region = SBR_NONE;
    }
}

void Scroll::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ LButtonUp(pt, mod_keys); }

void Scroll::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{ LButtonUp(pt, mod_keys); }

bool Scroll::EventFilter(Wnd* w, const WndEvent& event)
{
    if (w == m_tab) {
        switch (event.Type()) {
        case WndEvent::LDrag: {
            if (!Disabled()) {
                Pt new_ul = m_tab->RelativeUpperLeft() + event.DragMove();
                if (m_orientation == VERTICAL) {
                    new_ul.x = m_tab->RelativeUpperLeft().x;
                    new_ul.y = std::max(0 + m_decr->Height(),
                                        std::min(new_ul.y, ClientHeight() - m_incr->Height() - m_tab->Height()));
                    m_tab_dragged |= bool(m_tab->RelativeUpperLeft().y - new_ul.y);
                } else {
                    new_ul.x = std::max(0 + m_decr->Width(),
                                        std::min(new_ul.x, ClientWidth() - m_incr->Width() - m_tab->Width()));
                    new_ul.y = m_tab->RelativeUpperLeft().y;
                    m_tab_dragged |= bool(m_tab->RelativeUpperLeft().x - new_ul.x);
                }
                m_tab->MoveTo(new_ul);
                UpdatePosn();
            }
            return true;
        }
        case WndEvent::LButtonDown:
            m_dragging_tab = true;
            break;
        case WndEvent::LButtonUp:
        case WndEvent::LClick:
            if (m_tab_dragged)
                ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
            m_dragging_tab = false;
            m_tab_dragged = false;
            break;
        case WndEvent::MouseLeave:
            return m_dragging_tab;
        default:
            break;
        }
    }
    return false;
}

void Scroll::UpdatePosn()
{
    int old_posn = m_posn;
    int before_tab = (m_orientation == VERTICAL ?   // the tabspace before the tab's lower-value side
                      Value(m_tab->RelativeUpperLeft().y - m_decr->Size().y) :
                      Value(m_tab->RelativeUpperLeft().x - m_decr->Size().x));
    int tab_space = TabSpace() - (m_orientation == VERTICAL ? Value(m_tab->Size().y) : Value(m_tab->Size().x));
    int max_posn = static_cast<int>(m_range_max - m_page_sz + 1);
    m_posn = static_cast<int>(m_range_min + static_cast<double>(before_tab) / tab_space * (max_posn - m_range_min) + 0.5);
    m_posn = std::max(m_range_min, std::min(m_posn, max_posn));
    if (old_posn != m_posn)
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
}

void Scroll::MoveTabToPosn()
{
    int start_tabspace = (m_orientation == VERTICAL ?    // the tab's lowest posible extent
                          Value(m_decr->Size().y) :
                          Value(m_decr->Size().x));
    int tab_space = TabSpace() - (m_orientation == VERTICAL ? Value(m_tab->Size().y) : Value(m_tab->Size().x));
    int max_posn = static_cast<int>(m_range_max - m_page_sz + 1);
    double tab_location =
        (m_posn - m_range_min) / static_cast<double>(max_posn - m_range_min) * tab_space + start_tabspace + 0.5;
    if (m_posn - m_range_min == 0)
        tab_location = m_orientation == VERTICAL ? Value(m_decr->Height()) : Value(m_decr->Width());

    m_tab->MoveTo(m_orientation == VERTICAL ?
                  Pt(m_tab->RelativeUpperLeft().x, Y(static_cast<int>(tab_location))) :
                  Pt(X(static_cast<int>(tab_location)), m_tab->RelativeUpperLeft().y));
}

void Scroll::ScrollLineIncrImpl(bool signal)
{
    int old_posn = m_posn;
    if (static_cast<int>(m_posn + m_line_sz) <= static_cast<int>(m_range_max - m_page_sz))
        m_posn += m_line_sz;
    else
        m_posn = m_range_max - (m_page_sz - 1);
    MoveTabToPosn();
    if (signal && old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}

void Scroll::ScrollLineDecrImpl(bool signal)
{
    int old_posn = m_posn;
    if (static_cast<int>(m_posn - m_line_sz) >= m_range_min)
        m_posn -= m_line_sz;
    else
        m_posn = m_range_min;
    MoveTabToPosn();
    if (signal && old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}


////////////////////////////////////////////////
// free functions
////////////////////////////////////////////////
void GG::SignalScroll(const Scroll& scroll, bool stopped)
{
    std::pair<int, int> pr = scroll.PosnRange();
    std::pair<int, int> sr = scroll.ScrollRange();
    scroll.ScrolledSignal(pr.first, pr.second, sr.first, sr.second);
    if (stopped)
        scroll.ScrolledAndStoppedSignal(pr.first, pr.second, sr.first, sr.second);
}
