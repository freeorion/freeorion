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

#include <GG/TabWnd.h>

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    struct TabChangedEcho
    {
        TabChangedEcho(const std::string& name) : m_name(name) {}
        void operator()(std::size_t index)
            { std::cerr << "GG SIGNAL : " << m_name << "(index=" << index << ")\n"; }
        std::string m_name;
    };

    Y TabHeightFromFont(const boost::shared_ptr<Font>& font)
    { return font->Lineskip() + 10; }
}

////////////////////////////////////////////////
// GG::OverlayWnd
////////////////////////////////////////////////
// static(s)
const std::size_t OverlayWnd::NO_WND = std::numeric_limits<std::size_t>::max();

OverlayWnd::OverlayWnd() :
    m_current_wnd_index(NO_WND)
{}

OverlayWnd::OverlayWnd(X x, Y y, X w, Y h, Flags<WndFlag> flags) :
    Wnd(x, y, w, h, flags),
    m_current_wnd_index(NO_WND)
{ SetLayout(new Layout(X0, Y0, w, h, 1, 1)); }

OverlayWnd::~OverlayWnd()
{
    for (std::size_t i = 0; i < m_wnds.size(); ++i) {
        delete m_wnds[i];
    }
}

Pt OverlayWnd::MinUsableSize() const
{
    Pt retval;
    for (std::size_t i = 0; i < m_wnds.size(); ++i) {
        Pt min_usable_size = m_wnds[i]->MinUsableSize();
        retval.x = std::max(retval.x, min_usable_size.x);
        retval.y = std::max(retval.y, min_usable_size.y);
    }
    return retval;
}

bool OverlayWnd::Empty() const
{ return m_wnds.empty(); }

std::size_t OverlayWnd::NumWnds() const
{ return m_wnds.size(); }

Wnd* OverlayWnd::CurrentWnd() const
{ return m_current_wnd_index == NO_WND ? 0 : m_wnds[m_current_wnd_index]; }

std::size_t OverlayWnd::CurrentWndIndex() const
{ return m_current_wnd_index; }

const std::vector<Wnd*>& OverlayWnd::Wnds() const
{ return m_wnds; }

std::size_t OverlayWnd::AddWnd(Wnd* wnd)
{
    std::size_t retval = m_wnds.size();
    InsertWnd(m_wnds.size(), wnd);
    return retval;
}

void OverlayWnd::InsertWnd(std::size_t index, Wnd* wnd)
{
    m_wnds.insert(m_wnds.begin() + index, wnd);
    if (m_current_wnd_index == NO_WND)
        SetCurrentWnd(0);
}

Wnd* OverlayWnd::RemoveWnd(std::size_t index)
{
    Wnd* retval = 0;
    if (index < m_wnds.size()) {
        std::vector<Wnd*>::iterator it = m_wnds.begin() + index;
        retval = *it;
        m_wnds.erase(it);
        if (index == m_current_wnd_index)
            m_current_wnd_index = NO_WND;
    }
    return retval;
}

Wnd* OverlayWnd::RemoveWnd(Wnd* wnd)
{
    Wnd* retval = 0;
    std::vector<Wnd*>::iterator it = std::find(m_wnds.begin(), m_wnds.end(), wnd);
    if (it != m_wnds.end()) {
        if (it - m_wnds.begin() == static_cast<std::ptrdiff_t>(m_current_wnd_index))
            m_current_wnd_index = NO_WND;
        retval = *it;
        m_wnds.erase(it);
    }
    return retval;
}

void OverlayWnd::SetCurrentWnd(std::size_t index)
{
    assert(index < m_wnds.size());
    Wnd* old_current_wnd = CurrentWnd();
    m_current_wnd_index = index;
    Wnd* current_wnd = CurrentWnd();
    if (current_wnd != old_current_wnd) {
        Layout* layout = GetLayout();
        layout->Remove(old_current_wnd);
        layout->Add(current_wnd, 0, 0);
    }
}


////////////////////////////////////////////////
// GG::TabWnd
////////////////////////////////////////////////
// static(s)
const std::size_t TabWnd::NO_WND = std::numeric_limits<std::size_t>::max();

TabWnd::TabWnd() :
    m_tab_bar(0),
    m_overlay(0)
{}

TabWnd::TabWnd(X x, Y y, X w, Y h, const boost::shared_ptr<Font>& font, Clr color,
               Clr text_color/* = CLR_BLACK*/, TabBarStyle style/* = TAB_BAR_ATTACHED*/,
               Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Wnd(x, y, w, h, flags),
    m_tab_bar(GetStyleFactory()->NewTabBar(X0, Y0, w, font, color, text_color, style, INTERACTIVE)),
    m_overlay(new OverlayWnd(X0, Y0, X1, Y1))
{
    Layout* layout = new Layout(X0, Y0, w, h, 2, 1);
    layout->SetRowStretch(1, 1.0);
    layout->Add(m_tab_bar, 0, 0);
    layout->Add(m_overlay, 1, 0);
    SetLayout(layout);
    Connect(m_tab_bar->TabChangedSignal, boost::bind(&TabWnd::TabChanged, this, _1, true));

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(WndChangedSignal, TabChangedEcho("TabWnd::WndChangedSignal"));
}

Pt TabWnd::MinUsableSize() const
{
    Pt retval = m_tab_bar->MinUsableSize();
    Pt min_usable_size = m_overlay->MinUsableSize();
    retval.x = std::max(retval.x, min_usable_size.x);
    retval.y += min_usable_size.y;
    return retval;
}

bool TabWnd::Empty() const
{ return m_tab_bar->Empty(); }

std::size_t TabWnd::NumWnds() const
{ return m_tab_bar->NumTabs(); }

Wnd* TabWnd::CurrentWnd() const
{ return m_overlay->CurrentWnd(); }

std::size_t TabWnd::CurrentWndIndex() const
{ return m_tab_bar->CurrentTabIndex(); }

std::size_t TabWnd::AddWnd(Wnd* wnd, const std::string& name)
{
    std::size_t retval = m_named_wnds.size();
    InsertWnd(m_named_wnds.size(), wnd, name);
    return retval;
}

void TabWnd::InsertWnd(std::size_t index, Wnd* wnd, const std::string& name)
{
    std::size_t old_tab = m_tab_bar->CurrentTabIndex();
    m_named_wnds[name] = wnd;
    m_overlay->InsertWnd(index, wnd);
    m_tab_bar->InsertTab(index, name);
    GetLayout()->SetMinimumRowHeight(0, m_tab_bar->MinUsableSize().y + 2 * 5);
    if (m_tab_bar->CurrentTabIndex() != old_tab)
        TabChanged(m_tab_bar->CurrentTabIndex(), false);
}

Wnd* TabWnd::RemoveWnd(const std::string& name)
{
    std::size_t old_tab_index = m_tab_bar->CurrentTabIndex();
    Wnd* retval = m_overlay->RemoveWnd(m_named_wnds[name]);
    if (retval) {
        m_named_wnds.erase(name);
        m_tab_bar->RemoveTab(name);
        GetLayout()->SetMinimumRowHeight(0, m_tab_bar->MinUsableSize().y + 2 * 5);
    }
    if (m_tab_bar->CurrentTabIndex() != old_tab_index)
        TabChanged(m_tab_bar->CurrentTabIndex(), false);
    return retval;
}

void TabWnd::SetCurrentWnd(std::size_t index)
{
    m_overlay->SetCurrentWnd(index);
    m_tab_bar->SetCurrentTab(index);
    TabChanged(index, false);
}

const TabBar* TabWnd::GetTabBar() const
{ return m_tab_bar; }

const OverlayWnd* TabWnd::GetOverlayWnd() const
{ return m_overlay; }

const std::map<std::string, Wnd*>& TabWnd::WndNames() const
{ return m_named_wnds; }

void TabWnd::TabChanged(std::size_t index, bool signal)
{
    assert(index < m_named_wnds.size());
    m_overlay->SetCurrentWnd(index);
    if (signal)
        WndChangedSignal(index);
}


////////////////////////////////////////////////
// GG::TabBar
////////////////////////////////////////////////
// static(s)
const std::size_t TabBar::NO_TAB = TabWnd::NO_WND;
const X TabBar::BUTTON_WIDTH(10);

TabBar::TabBar() :
    Control(),
    m_tabs(0),
    m_left_button(0),
    m_right_button(0),
    m_left_right_button_layout(0),
    m_text_color(CLR_BLACK),
    m_style(TAB_BAR_ATTACHED),
    m_first_tab_shown(0)
{}

TabBar::TabBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
               TabBarStyle style/* = TAB_BAR_ATTACHED*/, Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Control(x, y, w, TabHeightFromFont(font), flags),
    m_tabs(0),
    m_font(font),
    m_left_button(0),
    m_right_button(0),
    m_left_right_button_layout(new Layout(X0, Y0, w, TabHeightFromFont(font), 1, 3)),
    m_text_color(text_color),
    m_style(style),
    m_first_tab_shown(0)
{
    SetColor(color);

    SetChildClippingMode(ClipToClient);

    boost::shared_ptr<StyleFactory> style_factory = GetStyleFactory();

    m_tabs = style_factory->NewRadioButtonGroup(X0, Y0, w, TabHeightFromFont(font), HORIZONTAL);
    m_tabs->ExpandButtons(true);
    m_tabs->ExpandButtonsProportionally(true);

    m_left_right_button_layout->SetColumnStretch(0, 1);
    m_left_right_button_layout->SetColumnStretch(1, 0);
    m_left_right_button_layout->SetColumnStretch(2, 0);

    m_left_button = style_factory->NewTabBarLeftButton(X0, Y0, BUTTON_WIDTH, Height(), "-", m_font, Color(), m_text_color);
    m_right_button = style_factory->NewTabBarRightButton(X0, Y0, BUTTON_WIDTH, Height(), "+", m_font, Color(), m_text_color);
    m_left_right_button_layout->SetMinimumColumnWidth(1, m_left_button->Width());
    m_left_right_button_layout->SetMinimumColumnWidth(2, m_right_button->Width());
    m_left_right_button_layout->Add(m_left_button, 0, 1);
    m_left_right_button_layout->Add(m_right_button, 0, 2);
    m_left_right_button_layout->Hide();

    AttachChild(m_tabs);
    AttachChild(m_left_right_button_layout);

    Connect(m_tabs->ButtonChangedSignal, boost::bind(&TabBar::TabChanged, this, _1, true));
    Connect(m_left_button->LeftClickedSignal, &TabBar::LeftClicked, this);
    Connect(m_right_button->LeftClickedSignal, &TabBar::RightClicked, this);

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(TabChangedSignal, TabChangedEcho("TabBar::TabChangedSignal"));
}

Pt TabBar::MinUsableSize() const
{
    Y y(0);
    for (std::size_t i = 0; i < m_tab_buttons.size(); ++i) {
        Y button_min_y = m_tab_buttons[i]->MinUsableSize().y;
        if (y < button_min_y)
            y = button_min_y;
    }
    return Pt(4 * BUTTON_WIDTH, y);
}

bool TabBar::Empty() const
{ return m_tabs->Empty(); }

std::size_t TabBar::NumTabs() const
{ return m_tabs->NumButtons(); }

std::size_t TabBar::CurrentTabIndex() const
{ return m_tabs->CheckedButton(); }

Clr TabBar::TextColor() const
{ return m_text_color; }

void TabBar::SizeMove(const Pt& ul, const Pt& lr)
{
    m_tabs->Resize(Pt(m_tabs->Size().x, lr.y -  ul.y));
    m_left_right_button_layout->SizeMove(Pt(), lr - ul);
    Control::SizeMove(ul, lr);
}

void TabBar::Render()
{}

std::size_t TabBar::AddTab(const std::string& name)
{
    std::size_t retval = m_tab_buttons.size();
    InsertTab(m_tab_buttons.size(), name);
    return retval;
}

void TabBar::InsertTab(std::size_t index, const std::string& name)
{
    assert(index <= m_tab_buttons.size());
    boost::shared_ptr<StyleFactory> style_factory = GetStyleFactory();
    StateButton* button = style_factory->NewTabBarTab(X0, Y0, X1, Y1, name,
                                                      m_font, FORMAT_CENTER, Color(),
                                                      m_text_color, CLR_ZERO,
                                                      m_style == TAB_BAR_ATTACHED ?
                                                      SBSTYLE_3D_TOP_ATTACHED_TAB :
                                                      SBSTYLE_3D_TOP_DETACHED_TAB);
    button->InstallEventFilter(this);
    m_tab_buttons.insert(m_tab_buttons.begin() + index, button);
    m_tabs->InsertButton(index, m_tab_buttons[index]);
    if (Width() < m_tabs->Width()) {
        m_left_right_button_layout->Show();
        m_left_button->Disable(m_first_tab_shown == 0);
        X right_side = m_left_right_button_layout->Visible() ?
            m_left_button->UpperLeft().x :
            LowerRight().x;
        m_right_button->Disable(m_tab_buttons.back()->LowerRight().x <= right_side);
    }
    if (m_tabs->CheckedButton() == RadioButtonGroup::NO_BUTTON)
        SetCurrentTab(0);
}

void TabBar::RemoveTab(const std::string& name)
{
    std::size_t index = NO_TAB;
    for (std::size_t i = 0; i < m_tab_buttons.size(); ++i) {
        if (m_tab_buttons[i]->Text() == name) {
            index = i;
            break;
        }
    }
    assert(index < m_tab_buttons.size());

    m_tab_buttons[index]->RemoveEventFilter(this);
    m_tabs->RemoveButton(m_tab_buttons[index]);
    delete m_tab_buttons[index];
    m_tab_buttons.erase(m_tab_buttons.begin() + index);
    if (m_tabs->Width() <= Width())
        m_left_right_button_layout->Hide();
    if (m_tabs->CheckedButton() == RadioButtonGroup::NO_BUTTON && !m_tab_buttons.empty())
        m_tabs->SetCheck(0);
}

void TabBar::SetCurrentTab(std::size_t index)
{
    m_tabs->SetCheck(index);
    TabChanged(index, false);
}

const Button* TabBar::LeftButton() const
{ return m_left_button; }

const Button* TabBar::RightButton() const
{ return m_right_button; }

void TabBar::DistinguishCurrentTab(const std::vector<StateButton*>& tab_buttons)
{ RaiseCurrentTabButton(); }

void TabBar::TabChanged(std::size_t index, bool signal)
{
    if (index != RadioButtonGroup::NO_BUTTON) {
        BringTabIntoView(index);
        DistinguishCurrentTab(m_tab_buttons);
        if (signal)
            TabChangedSignal(index);
    }
}

void TabBar::LeftClicked()
{
    assert(0 < m_first_tab_shown);
    m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->UpperLeft().x - m_tab_buttons[m_first_tab_shown - 1]->UpperLeft().x, Y0));
    --m_first_tab_shown;
    m_left_button->Disable(m_first_tab_shown == 0);
    m_right_button->Disable(false);
}

void TabBar::RightClicked()
{
    assert(m_first_tab_shown < m_tab_buttons.size() - 1);
    m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->UpperLeft().x - m_tab_buttons[m_first_tab_shown + 1]->UpperLeft().x, Y0));
    ++m_first_tab_shown;
    X right_side = m_left_right_button_layout->Visible() ?
        m_left_button->UpperLeft().x :
        LowerRight().x;
    m_right_button->Disable(m_tab_buttons.back()->LowerRight().x <= right_side);
    m_left_button->Disable(false);
}

void TabBar::BringTabIntoView(std::size_t index)
{
    while (m_tab_buttons[index]->UpperLeft().x < UpperLeft().x) {
        LeftClicked();
    }
    X right_side = m_left_right_button_layout->Visible() ?
        m_left_button->UpperLeft().x :
        LowerRight().x;
    if (m_tab_buttons[index]->Width() < Width()) {
        while (right_side < m_tab_buttons[index]->LowerRight().x && index != m_first_tab_shown) {
            RightClicked();
        }
    } else {
        m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->UpperLeft().x - m_tab_buttons[index]->UpperLeft().x, Y0));
        m_right_button->Disable(m_tab_buttons.back()->LowerRight().x <= right_side);
        m_left_button->Disable(false);
    }
}

bool TabBar::EventFilter(Wnd* w, const WndEvent& event)
{
    if (event.Type() == WndEvent::LButtonDown ||
        event.Type() == WndEvent::RButtonDown)
        MoveChildUp(m_left_right_button_layout);
    return false;
}

void TabBar::RaiseCurrentTabButton()
{ m_tabs->RaiseCheckedButton(); }
