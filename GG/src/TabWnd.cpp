//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/TabWnd.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {

struct TabChangedEcho
{
    TabChangedEcho(std::string name) : m_name(std::move(name)) {}
    void operator()(std::size_t index)
        { std::cerr << "GG SIGNAL : " << m_name << "(index=" << index << ")\n"; }
    std::string m_name;
};

Y TabHeightFromFont(const std::shared_ptr<Font>& font)
{ return font->Lineskip() + 10; }

}

////////////////////////////////////////////////
// GG::OverlayWnd
////////////////////////////////////////////////
OverlayWnd::OverlayWnd(X x, Y y, X w, Y h, Flags<WndFlag> flags) :
    Wnd(x, y, w, h, flags)
{}

void OverlayWnd::CompleteConstruction()
{ SetLayout(Wnd::Create<Layout>(X0, Y0, Width(), Height(), 1, 1)); }

Pt OverlayWnd::MinUsableSize() const
{
    Pt retval;
    for (auto& wnd : m_wnds) {
        Pt min_usable_size = wnd->MinUsableSize();
        retval.x = std::max(retval.x, min_usable_size.x);
        retval.y = std::max(retval.y, min_usable_size.y);
    }
    return retval;
}

bool OverlayWnd::Empty() const
{ return m_wnds.empty(); }

std::size_t OverlayWnd::NumWnds() const
{ return m_wnds.size(); }

std::shared_ptr<Wnd> OverlayWnd::CurrentWnd() const
{ return m_current_wnd_index == NO_WND ? nullptr : m_wnds[m_current_wnd_index]; }

std::size_t OverlayWnd::CurrentWndIndex() const
{ return m_current_wnd_index; }

std::size_t OverlayWnd::AddWnd(std::shared_ptr<Wnd> wnd)
{
    std::size_t retval = m_wnds.size();
    InsertWnd(m_wnds.size(), std::move(wnd));
    return retval;
}

void OverlayWnd::InsertWnd(std::size_t index, std::shared_ptr<Wnd> wnd)
{
    m_wnds.insert(m_wnds.begin() + index, std::move(wnd));
    if (m_current_wnd_index == NO_WND)
        SetCurrentWnd(0);
}

Wnd* OverlayWnd::RemoveWnd(std::size_t index)
{
    Wnd* retval = nullptr;
    if (index < m_wnds.size()) {
        auto it = m_wnds.begin() + index;
        retval = it->get();
        m_wnds.erase(it);
        if (index == m_current_wnd_index)
            m_current_wnd_index = NO_WND;
    }
    return retval;
}

Wnd* OverlayWnd::RemoveWnd(Wnd* wnd)
{
    Wnd* retval = nullptr;
    auto it = std::find_if(m_wnds.begin(), m_wnds.end(),
                           [&wnd](const std::shared_ptr<Wnd>& x){ return (x.get() == wnd); });
    if (it != m_wnds.end()) {
        if (it - m_wnds.begin() == static_cast<std::ptrdiff_t>(m_current_wnd_index))
            m_current_wnd_index = NO_WND;
        retval = it->get();
        m_wnds.erase(it);
    }
    return retval;
}

void OverlayWnd::SetCurrentWnd(std::size_t index)
{
    assert(index < m_wnds.size());
    const auto& old_current_wnd = CurrentWnd();
    m_current_wnd_index = index;
    const auto& current_wnd = CurrentWnd();
    assert(current_wnd);
    if (current_wnd != old_current_wnd) {
        GG::Pt ul = old_current_wnd ? old_current_wnd->UpperLeft() : current_wnd->UpperLeft();
        GG::Pt lr = old_current_wnd ? old_current_wnd->LowerRight() : current_wnd->LowerRight();
        current_wnd->SizeMove(ul, lr);

        auto&& layout = GetLayout();
        layout->Remove(old_current_wnd.get());
        layout->Add(current_wnd, 0, 0);

        if (old_current_wnd)
            old_current_wnd->SizeMove(ul, lr);

        // Toggle the size to force layout to relayout even though size
        // has not changed.
        SizeMove(UpperLeft(), LowerRight() - GG::Pt(GG::X1, GG::Y1));
        SizeMove(UpperLeft(), LowerRight() + GG::Pt(GG::X1, GG::Y1));
    }
}


////////////////////////////////////////////////
// GG::TabWnd
////////////////////////////////////////////////
TabWnd::TabWnd(X x, Y y, X w, Y h, const std::shared_ptr<Font>& font, Clr color,
               Clr text_color) :
    Wnd(x, y, w, h, INTERACTIVE),
    m_tab_bar(GetStyleFactory().NewTabBar(font, color, text_color)),
    m_overlay(Wnd::Create<OverlayWnd>(X0, Y0, X1, Y1))
{}

void TabWnd::CompleteConstruction()
{
    auto layout = Wnd::Create<Layout>(X0, Y0, Width(), Height(), 2, 1);
    layout->SetRowStretch(1, 1.0);
    layout->Add(m_tab_bar, 0, 0);
    layout->Add(m_overlay, 1, 0);
    SetLayout(std::move(layout));
    m_tab_bar->TabChangedSignal.connect(
        boost::bind(&TabWnd::TabChanged, this, boost::placeholders::_1, true));

    if (INSTRUMENT_ALL_SIGNALS)
        TabChangedSignal.connect(TabChangedEcho("TabWnd::TabChangedSignal"));
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
{ return m_overlay->CurrentWnd().get(); }

std::size_t TabWnd::CurrentWndIndex() const
{ return m_tab_bar->CurrentTabIndex(); }

std::size_t TabWnd::AddWnd(std::shared_ptr<Wnd> wnd, std::string name)
{
    std::size_t retval = m_named_wnds.size();
    InsertWnd(m_named_wnds.size(), std::move(wnd), std::move(name));
    return retval;
}

void TabWnd::InsertWnd(std::size_t index, std::shared_ptr<Wnd> wnd, std::string name)
{
    std::size_t old_tab = m_tab_bar->CurrentTabIndex();
    m_named_wnds[name] = wnd.get();
    m_overlay->InsertWnd(index, std::move(wnd));
    m_tab_bar->InsertTab(index, std::move(name));
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

const std::map<std::string, Wnd*>& TabWnd::WndNames() const
{ return m_named_wnds; }

void TabWnd::TabChanged(std::size_t index, bool signal)
{
    assert(index < m_named_wnds.size());
    m_overlay->SetCurrentWnd(index);
    if (signal)
        TabChangedSignal(index);
}


////////////////////////////////////////////////
// GG::TabBar
////////////////////////////////////////////////
TabBar::TabBar(const std::shared_ptr<Font>& font, Clr color, Clr text_color,
               Flags<WndFlag> flags) :
    Control(X0, Y0, X1, TabHeightFromFont(font), flags),
    m_font(font),
    m_left_right_button_layout(Wnd::Create<Layout>(X0, Y0, X1, TabHeightFromFont(font), 1, 3)),
    m_text_color(text_color)
{
    SetColor(color);
}

void TabBar::CompleteConstruction()
{
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    const auto& style_factory = GetStyleFactory();

    m_tabs = style_factory.NewRadioButtonGroup(Orientation::HORIZONTAL);
    m_tabs->ExpandButtons(true);
    m_tabs->ExpandButtonsProportionally(true);

    m_left_right_button_layout->SetColumnStretch(0, 1);
    m_left_right_button_layout->SetColumnStretch(1, 0);
    m_left_right_button_layout->SetColumnStretch(2, 0);

    m_left_button = style_factory.NewTabBarLeftButton(m_font, Color(), m_text_color);
    m_right_button = style_factory.NewTabBarRightButton(m_font, Color(), m_text_color);
    m_left_button->Resize(Pt(ButtonWidth(), Height()));
    m_right_button->Resize(Pt(ButtonWidth(), Height()));
    m_left_right_button_layout->SetMinimumColumnWidth(1, m_left_button->Width());
    m_left_right_button_layout->SetMinimumColumnWidth(2, m_right_button->Width());
    m_left_right_button_layout->Add(m_left_button, 0, 1);
    m_left_right_button_layout->Add(m_right_button, 0, 2);
    m_left_right_button_layout->Hide();

    AttachChild(m_tabs);
    AttachChild(m_left_right_button_layout);

    m_tabs->ButtonChangedSignal.connect(boost::bind(&TabBar::TabChanged, this, boost::placeholders::_1, true));
    m_left_button->LeftPressedSignal.connect(boost::bind(&TabBar::LeftClicked, this));
    m_right_button->LeftPressedSignal.connect(boost::bind(&TabBar::RightClicked, this));

    if (INSTRUMENT_ALL_SIGNALS)
        TabChangedSignal.connect(TabChangedEcho("TabBar::TabChangedSignal"));

    DoLayout();
}

Pt TabBar::MinUsableSize() const
{
    Y y(Y0);
    for (auto& button : m_tab_buttons) {
        Y button_min_y = button->MinUsableSize().y;
        if (y < button_min_y)
            y = button_min_y;
    }
    return Pt(4 * ButtonWidth(), y);
}

bool TabBar::Empty() const
{ return m_tabs->Empty(); }

std::size_t TabBar::NumTabs() const
{ return m_tabs->NumButtons(); }

std::size_t TabBar::CurrentTabIndex() const
{ return m_tabs->CheckedButton(); }

Clr TabBar::TextColor() const
{ return m_text_color; }

void TabBar::MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys)
{
    if (move < 0 && m_right_button && !m_right_button->Disabled()) {
        RightClicked();
        return;
    }
    if (move > 0 && m_left_button && !m_left_button->Disabled()) {
        LeftClicked();
        return;
    }
}

void TabBar::SizeMove(Pt ul, Pt lr)
{
    const auto old_size = Size();
    Control::SizeMove(ul, lr);
    if(old_size != Size())
        DoLayout();
}

void TabBar::DoLayout()
{
    m_tabs->Resize(Pt(m_tabs->Size().x, LowerRight().y - UpperLeft().y));
    m_left_right_button_layout->SizeMove(Pt(), LowerRight() - UpperLeft());
    RecalcLeftRightButton();
}

void TabBar::Render()
{}

std::size_t TabBar::AddTab(std::string name)
{
    std::size_t retval = m_tab_buttons.size();
    InsertTab(m_tab_buttons.size(), std::move(name));
    return retval;
}

void TabBar::InsertTab(std::size_t index, std::string name)
{
    assert(index <= m_tab_buttons.size());
    auto button = GetStyleFactory().NewTabBarTab(
        std::move(name), m_font, FORMAT_CENTER, Color(), m_text_color);
    button->InstallEventFilter(shared_from_this());
    m_tab_buttons.insert(m_tab_buttons.begin() + index, button);
    m_tabs->InsertButton(index, m_tab_buttons[index]);
    RecalcLeftRightButton();
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

    m_tab_buttons[index]->RemoveEventFilter(shared_from_this());
    m_tabs->RemoveButton(m_tab_buttons[index].get());
    m_tab_buttons.erase(m_tab_buttons.begin() + index);
    RecalcLeftRightButton();
    if (m_tabs->CheckedButton() == RadioButtonGroup::NO_BUTTON && !m_tab_buttons.empty())
        m_tabs->SetCheck(0);
}

void TabBar::RecalcLeftRightButton()
{
    if (m_left_button)
        m_left_button->Disable(m_first_tab_shown == 0);
    if (m_left_button && m_right_button && m_tab_buttons.size())
        m_right_button->Disable(m_tab_buttons.back()->Right() <= m_left_button->Left());
    if (Width() < m_tabs->Width() && !m_left_right_button_layout->Visible()) {
        m_left_right_button_layout->Show();
    }
    if (m_tabs->Width() <= Width() && m_left_right_button_layout->Visible())
        m_left_right_button_layout->Hide();
}

void TabBar::SetCurrentTab(std::size_t index)
{
    m_tabs->SetCheck(index);
    TabChanged(index, false);
}

X TabBar::ButtonWidth() const
{ return ToX(m_font->SpaceWidth() * 2.5); }

const Button* TabBar::LeftButton() const
{ return m_left_button.get(); }

const Button* TabBar::RightButton() const
{ return m_right_button.get(); }

void TabBar::DistinguishCurrentTab(const std::vector<StateButton*>& tab_buttons)
{ RaiseCurrentTabButton(); }

void TabBar::TabChanged(std::size_t index, bool signal)
{
    if (index != RadioButtonGroup::NO_BUTTON) {
        BringTabIntoView(index);
        std::vector<StateButton*> tab_buttons(m_tab_buttons.size());
        std::transform(m_tab_buttons.begin(), m_tab_buttons.end(), tab_buttons.begin(),
                       [](const std::shared_ptr<StateButton>& x){ return x.get(); });
        DistinguishCurrentTab(tab_buttons);
        if (signal)
            TabChangedSignal(index);
    }
}

void TabBar::LeftClicked()
{
    if (m_first_tab_shown == 0 || m_first_tab_shown >= m_tab_buttons.size())
        return;
    const auto& first_shown_tab = m_tab_buttons[m_first_tab_shown];
    const auto& prev_tab = m_tab_buttons[m_first_tab_shown - 1u];
    if (!first_shown_tab || !prev_tab)
        return;

    m_tabs->OffsetMove(Pt(first_shown_tab->Left() - prev_tab->Left(), Y0));
    --m_first_tab_shown;
    m_left_button->Disable(m_first_tab_shown == 0);
    m_right_button->Disable(false);
}

void TabBar::RightClicked()
{
    assert(m_first_tab_shown < m_tab_buttons.size() - 1);
    m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->Left() -
                            m_tab_buttons[m_first_tab_shown + 1]->Left(),
                          Y0));
    ++m_first_tab_shown;
    X right_side = m_left_right_button_layout->Visible() ?
        m_left_button->Left() :
        Right();
    // Is there anything to the right the user may want to see?
    bool more_to_show = m_tab_buttons.back()->Right() > right_side;
    // Are there any tabs left to hide to the left?
    bool more_to_hide = m_first_tab_shown < m_tab_buttons.size() - 1;
    m_right_button->Disable( !(more_to_show && more_to_hide) );
    m_left_button->Disable(false);
}

void TabBar::BringTabIntoView(std::size_t index)
{
    if (index >= m_tab_buttons.size())
        return;
    const auto& target_tab = m_tab_buttons[index];
    if (!target_tab)
        return;

    const auto left = Left();
    std::size_t count = m_tab_buttons.size();
    while (target_tab->Left() < left && count-->0)
        LeftClicked();

    const X right_side = m_left_right_button_layout->Visible() ?
        m_left_button->Left() : Right();

    if (target_tab->Width() < Width()) {
        while (right_side < target_tab->Right() && index != m_first_tab_shown)
            RightClicked();

    } else {
        if (m_first_tab_shown >= m_tab_buttons.size())
            return;
        const auto& first_shown_tab = m_tab_buttons[m_first_tab_shown];
        if (!first_shown_tab || !m_tab_buttons.back())
            return;

        m_tabs->OffsetMove(Pt(first_shown_tab->Left() - target_tab->Left(), Y0));
        m_right_button->Disable(m_tab_buttons.back()->Right() <= right_side);
        m_left_button->Disable(false);
    }
}

bool TabBar::EventFilter(Wnd* w, const WndEvent& event)
{
    if (event.Type() == WndEvent::EventType::LButtonDown ||
        event.Type() == WndEvent::EventType::RButtonDown)
    { MoveChildUp(m_left_right_button_layout.get()); }
    return false;
}

void TabBar::RaiseCurrentTabButton()
{ m_tabs->RaiseCheckedButton(); }
