//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    void ClickedEcho()
    { std::cerr << "GG SIGNAL : Button::LeftClickedSignal()" << std::endl; }

    void CheckedEcho(bool checked)
    { std::cerr << "GG SIGNAL : StateButton::CheckedSignal(checked=" << checked << ")" << std::endl; }

    void ButtonChangedEcho(std::size_t index)
    { std::cerr << "GG SIGNAL : RadioButtonGroup::ButtonChangedSignal(index=" << index << ")" << std::endl; }

    constexpr GG::Pt SHADOW_OFFSET{GG::X(2), GG::Y(2)};
}

////////////////////////////////////////////////
// GG::Button
////////////////////////////////////////////////
Button::Button(std::string str, const std::shared_ptr<Font>& font, Clr color,
               Clr text_color, Flags<WndFlag> flags) :
    Control(X0, Y0, X1, Y1, flags),
    m_label(Wnd::Create<TextControl>(X0, Y0, X1, Y1, str, font,
                                     text_color, FORMAT_NONE, NO_WND_FLAGS)),
    m_label_shadow(Wnd::Create<TextControl>(SHADOW_OFFSET.x, SHADOW_OFFSET.y, X1, Y1, std::move(str),
                                            font, CLR_SHADOW, FORMAT_NONE, NO_WND_FLAGS))
{
    m_color = color;
    m_label->Hide();
    m_label_shadow->Hide();

    if (INSTRUMENT_ALL_SIGNALS)
        LeftClickedSignal.connect(&ClickedEcho);
}

void Button::CompleteConstruction()
{
    AttachChild(m_label_shadow);
    AttachChild(m_label);
}

Pt Button::MinUsableSize() const
{ return m_label->MinUsableSize(); }

void Button::Show()
{
    Wnd::Show();
    m_label->Hide();
    m_label_shadow->Hide();
}

Button::ButtonState Button::State() const
{ return m_state; }

const std::string& Button::Text() const
{ return m_label->Text(); }

const SubTexture& Button::UnpressedGraphic() const
{ return m_unpressed_graphic; }

const SubTexture& Button::PressedGraphic() const
{ return m_pressed_graphic; }

const SubTexture& Button::RolloverGraphic() const
{ return m_rollover_graphic; }

void Button::Render()
{
    switch (m_state) {
    case ButtonState::BN_PRESSED:   RenderPressed(); break;
    case ButtonState::BN_UNPRESSED: RenderUnpressed(); break;
    case ButtonState::BN_ROLLOVER:  RenderRollover(); break;
    }
}

void Button::SizeMove(Pt ul, Pt lr)
{
    GG::Pt sz = Size();
    Wnd::SizeMove(ul, lr);
    if (sz == Size())
        return;

    m_label->Resize(Size());
    m_label_shadow->Resize(Size());
}

void Button::SetColor(Clr c)
{ Control::SetColor(c); }

void Button::SetState(ButtonState state)
{ m_state = state; }

void Button::SetText(std::string text)
{
    m_label->SetText(text);
    m_label_shadow->SetText(std::move(text));
}

void Button::SetUnpressedGraphic(SubTexture st)
{ m_unpressed_graphic = std::move(st); }

void Button::SetPressedGraphic(SubTexture st)
{ m_pressed_graphic = std::move(st); }

void Button::SetRolloverGraphic(SubTexture st)
{ m_rollover_graphic = std::move(st); }

void Button::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    ButtonState prev_state = m_state;
    m_state = ButtonState::BN_PRESSED;
    if (prev_state == ButtonState::BN_PRESSED && RepeatButtonDown())
        LeftClickedSignal();
    else if (prev_state != ButtonState::BN_PRESSED)
        LeftPressedSignal();
}

void Button::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = ButtonState::BN_PRESSED;
    Wnd::LDrag(pt, move, mod_keys);
}

void Button::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = ButtonState::BN_UNPRESSED;
}

void Button::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_state = ButtonState::BN_ROLLOVER;
        LeftClickedSignal();
    }
}

void Button::RButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    ButtonState prev_state = m_state;
    m_state = ButtonState::BN_PRESSED;
    if (prev_state == ButtonState::BN_PRESSED && RepeatButtonDown())
        RightClickedSignal();
    else if (prev_state != ButtonState::BN_PRESSED)
        RightPressedSignal();
}

void Button::RDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = ButtonState::BN_PRESSED;
    Wnd::LDrag(pt, move, mod_keys);
}

void Button::RButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = ButtonState::BN_UNPRESSED;
}

void Button::RClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_state = ButtonState::BN_ROLLOVER;
        RightClickedSignal();
    }
}

void Button::MouseEnter(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = ButtonState::BN_ROLLOVER;
}

void Button::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = ButtonState::BN_ROLLOVER;
}

void Button::MouseLeave()
{
    if (!Disabled())
        m_state = ButtonState::BN_UNPRESSED;
}

void Button::RenderUnpressed()
{
    if (!m_unpressed_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_unpressed_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    // draw text shadow and text
    m_label_shadow->Render();
    m_label->Render();
}

void Button::RenderPressed()
{
    if (!m_pressed_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_pressed_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    m_label_shadow->OffsetMove(Pt(X1, Y1));
    m_label_shadow->Render();
    m_label_shadow->OffsetMove(Pt(-X1, -Y1));
    m_label->OffsetMove(Pt(X1, Y1));
    m_label->Render();
    m_label->OffsetMove(Pt(-X1, -Y1));
}

void Button::RenderRollover()
{
    if (!m_rollover_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_rollover_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    m_label_shadow->Render();
    m_label->Render();
}

void Button::RenderDefault()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    BeveledRectangle(ul, lr,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     (m_state != ButtonState::BN_PRESSED), 1);
}


////////////////////////////////////////////////
// GG::StateButton
////////////////////////////////////////////////
StateButton::StateButton(std::string str, const std::shared_ptr<Font>& font,
                         Flags<TextFormat> format, Clr color,
                         std::shared_ptr<StateButtonRepresenter> representer,
                         Clr text_color) :
    Control(X0, Y0, X1, Y1, INTERACTIVE),
    m_representer(std::move(representer)),
    m_label(Wnd::Create<TextControl>(X0, Y0, X1, Y1, std::move(str), font,
                                     text_color, format, NO_WND_FLAGS))
{ m_color = color; }

void StateButton::CompleteConstruction()
{
    AttachChild(m_label);
    m_label->Hide();

    if (INSTRUMENT_ALL_SIGNALS)
        CheckedSignal.connect(&CheckedEcho);
}

Pt StateButton::MinUsableSize() const
{
    if (m_representer)
        return m_representer->MinUsableSize(*this);

    return Pt();
}

StateButton::ButtonState StateButton::State() const
{ return m_state; }

const std::string& StateButton::Text() const
{ return m_label->Text(); }

bool StateButton::Checked() const
{ return m_checked; }

void StateButton::Render()
{
    if (m_representer)
        m_representer->Render(*this);
}

void StateButton::Show()
{
    Wnd::Show();
    m_label->Hide();
}

void StateButton::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{ SetState(ButtonState::BN_PRESSED); }

void StateButton::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    SetState(ButtonState::BN_PRESSED);
    Wnd::LDrag(pt, move, mod_keys);
}

void StateButton::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{ SetState(ButtonState::BN_UNPRESSED); }

void StateButton::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        SetCheck(!m_checked);
        if (m_representer)
            m_representer->OnChecked(m_checked);
        CheckedSignal(m_checked);
    }
}

void StateButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ SetState(ButtonState::BN_ROLLOVER); }

void StateButton::MouseLeave()
{ SetState(ButtonState::BN_UNPRESSED); }

void StateButton::SetState(ButtonState next_state) {
    if (!Disabled() && next_state != m_state) {
        ButtonState prev_state = m_state;
        m_state = next_state;
        if (m_representer)
            m_representer->OnChanged(*this, prev_state);
    }
}

void StateButton::SizeMove(Pt ul, Pt lr)
{
    Control::SizeMove(ul, lr);
    m_label->Resize(Size());
}

void StateButton::Reset()
{ SetCheck(false); }

void StateButton::SetCheck(bool b)
{ m_checked = b; }

void StateButton::SetTextColor(Clr c)
{ m_label->SetTextColor(c); }

TextControl* StateButton::GetLabel() const
{ return m_label.get(); }


////////////////////////////////////////////////
// GG::StateButtonRepresenter
////////////////////////////////////////////////
void StateButtonRepresenter::Render(const GG::StateButton& button) const
{}

void StateButtonRepresenter::DoLayout(const GG::StateButton& button, Pt& button_ul, Pt& button_lr, Pt& text_ul) const
{
    X bn_w = X(button.GetLabel()->GetFont()->PointSize()); // set button width and height to text he
    Y bn_h = Y(button.GetLabel()->GetFont()->PointSize());

    button_ul = Pt();
    button_lr = Pt(bn_w, bn_h);

    X w = button.Width();
    Y h = button.Height();
    const X BN_W = button_lr.x - button_ul.x;
    const Y BN_H = button_lr.y - button_ul.y;
    X bn_x = button_ul.x;
    Y bn_y = button_ul.y;
    Flags<TextFormat> format = button.GetLabel()->GetTextFormat();
    Flags<TextFormat> original_format = format;
    static constexpr double SPACING = 0.5;    // the space to leave between the button and text, as a factor of the button's size (width or height)
    if (format & FORMAT_VCENTER)       // center button vertically
        bn_y = (h - BN_H) / 2.0 + 0.5;
    if (format & FORMAT_TOP) {         // put button at top, text just below
        bn_y = Y0;
        text_ul.y = BN_H;
    }
    if (format & FORMAT_BOTTOM) {      // put button at bottom, text just above
        bn_y = (h - BN_H);
        text_ul.y = h - (BN_H * (1 + SPACING)) - (std::max(0, static_cast<int>(button.GetLabel()->GetLineData().size() - 1)) * button.GetLabel()->GetFont()->Lineskip() + button.GetLabel()->GetFont()->Height()) + 0.5;
    }

    if (format & FORMAT_CENTER) {      // center button horizontally
        if (format & FORMAT_VCENTER) { // if both the button and the text are to be centered, bad things happen
            format |= FORMAT_LEFT;     // so go to the default (FORMAT_CENTER|FORMAT_LEFT)
            format &= ~FORMAT_CENTER;
        } else {
            bn_x = (w - bn_x) / 2.0 - BN_W / 2.0 + 0.5;
        }
    }
    if (format & FORMAT_LEFT) {        // put button at left, text just to the right
        bn_x = X0;
        if (format & FORMAT_VCENTER)
            text_ul.x = BN_W * (1 + SPACING) + 0.5;
    }
    if (format & FORMAT_RIGHT) {       // put button at right, text just to the left
        bn_x = (w - BN_W);
        if (format & FORMAT_VCENTER)
            text_ul.x = -BN_W * (1 + SPACING) + 0.5;
    }
    if (format != original_format)
        button.GetLabel()->SetTextFormat(format);
    button_ul = Pt(bn_x, bn_y);
    button_lr = button_ul + Pt(BN_W, BN_H);
}

void StateButtonRepresenter::OnChanged(const StateButton& button, StateButton::ButtonState previous_state) const
{}

void StateButtonRepresenter::OnChecked(bool checked) const
{}

Pt StateButtonRepresenter::MinUsableSize(const StateButton& button) const
{
    Pt bn_ul, bn_lr, tx_ul;

    DoLayout(button, bn_ul, bn_lr, tx_ul);

    Pt text_lr = tx_ul + button.GetLabel()->MinUsableSize();
    return Pt(std::max(bn_lr.x, text_lr.x) - std::min(bn_ul.x, tx_ul.x),
              std::max(bn_lr.y, text_lr.y) - std::min(bn_ul.y, tx_ul.y));
}


////////////////////////////////////////////////
// GG::BeveledCheckBoxRepresenter
////////////////////////////////////////////////
BeveledCheckBoxRepresenter::BeveledCheckBoxRepresenter(Clr interior):
    m_int_color(interior)
{}

void BeveledCheckBoxRepresenter::Render(const GG::StateButton& button) const
{
    // draw button
    Pt cl_ul = button.ClientUpperLeft();
    Pt bn_ul, bn_lr, tx_ul;

    DoLayout(button, bn_ul, bn_lr, tx_ul);

    bn_ul += cl_ul;
    bn_lr += cl_ul;

    static constexpr int BEVEL = 2;
    static constexpr Pt DOUBLE_BEVEL(X(2 * BEVEL), Y(2 * BEVEL));

    BeveledRectangle(bn_ul, bn_lr,
                     button.Disabled() ? DisabledColor(m_int_color) : m_int_color,
                     button.Disabled() ? DisabledColor(button.Color()) : button.Color(),
                     false, BEVEL);
    if (button.Checked())
        BeveledCheck(bn_ul + DOUBLE_BEVEL, bn_lr - DOUBLE_BEVEL,
                     button.Disabled() ? DisabledColor(button.Color()) : button.Color());

    button.GetLabel()->OffsetMove(tx_ul);
    button.GetLabel()->Render();
    button.GetLabel()->OffsetMove(-tx_ul);
}


////////////////////////////////////////////////
// GG::BeveledRadioRepresenter
////////////////////////////////////////////////
BeveledRadioRepresenter::BeveledRadioRepresenter(Clr interior):
    m_int_color(interior)
{}

void BeveledRadioRepresenter::Render(const GG::StateButton& button) const
{
    // draw button
    Pt cl_ul = button.ClientUpperLeft();
    Pt bn_ul, bn_lr, tx_ul;

    DoLayout(button, bn_ul, bn_lr, tx_ul);

    bn_ul += cl_ul;
    bn_lr += cl_ul;

    static constexpr int BEVEL = 2;
    static constexpr Pt DOUBLE_BEVEL(X(2 * BEVEL), Y(2 * BEVEL));

    BeveledCircle(bn_ul, bn_lr,
                  button.Disabled() ? DisabledColor(m_int_color) : m_int_color,
                  button.Disabled() ? DisabledColor(button.Color()) : button.Color(),
                  false, BEVEL);
    if (button.Checked())
        Bubble(bn_ul + DOUBLE_BEVEL, bn_lr - DOUBLE_BEVEL,
               button.Disabled() ? DisabledColor(button.Color()) : button.Color());

    button.GetLabel()->OffsetMove(tx_ul);
    button.GetLabel()->Render();
    button.GetLabel()->OffsetMove(-(tx_ul));
}

Pt BeveledTabRepresenter::MinUsableSize(const StateButton& button) const
{ return button.GetLabel()->MinUsableSize(); }


////////////////////////////////////////////////
// GG::BeveledTabRepresenter
////////////////////////////////////////////////
void BeveledTabRepresenter::Render(const StateButton& button) const
{
    static constexpr int BEVEL = 2;

    // draw button
    Pt cl_ul = button.ClientUpperLeft();
    Pt cl_lr = button.ClientLowerRight();
    Pt tx_ul = Pt();

    Clr color_to_use = button.Checked() ? button.Color() : DarkenClr(button.Color());
    color_to_use = button.Disabled() ? DisabledColor(color_to_use) : color_to_use;
    if (!button.Checked()) {
        cl_ul.y += BEVEL;
        tx_ul.y = Y(BEVEL / 2);
    }
    BeveledRectangle(cl_ul, cl_lr,
                     color_to_use, color_to_use,
                     true, BEVEL,
                     true, true, true, !button.Checked());

    button.GetLabel()->OffsetMove(tx_ul);
    button.GetLabel()->Render();
    button.GetLabel()->OffsetMove(-(tx_ul));
}


////////////////////////////////////////////////
// GG::RadioButtonGroup
////////////////////////////////////////////////
// ButtonSlot
RadioButtonGroup::ButtonSlot::ButtonSlot(std::shared_ptr<StateButton> button_) :
    button(std::move(button_))
{}

RadioButtonGroup::RadioButtonGroup(Orientation orientation) :
    Control(X0, Y0, X1, Y1),
    m_orientation(orientation),
    m_checked_button(NO_BUTTON),
    m_expand_buttons(false),
    m_expand_buttons_proportionally(false),
    m_render_outline(false)
{
    SetColor(CLR_YELLOW);

    if (INSTRUMENT_ALL_SIGNALS)
        ButtonChangedSignal.connect(&ButtonChangedEcho);
}

Pt RadioButtonGroup::MinUsableSize() const
{
    Pt retval;
    for (const ButtonSlot& button_slot : m_button_slots) {
        Pt min_usable_size = button_slot.button->MinUsableSize();
        if (m_orientation == Orientation::VERTICAL) {
            retval.x = std::max(retval.x, min_usable_size.x);
            retval.y += min_usable_size.y;
        } else {
            retval.x += min_usable_size.x;
            retval.y = std::max(retval.y, min_usable_size.y);
        }
    }
    return retval;
}

Orientation RadioButtonGroup::GetOrientation() const
{ return m_orientation; }

bool RadioButtonGroup::Empty() const
{ return m_button_slots.empty(); }

std::size_t RadioButtonGroup::NumButtons() const
{ return m_button_slots.size(); }

std::size_t RadioButtonGroup::CheckedButton() const
{ return m_checked_button; }

bool RadioButtonGroup::ExpandButtons() const
{ return m_expand_buttons; }

bool RadioButtonGroup::ExpandButtonsProportionally() const
{ return m_expand_buttons_proportionally; }

bool RadioButtonGroup::RenderOutline() const
{ return m_render_outline; }

void RadioButtonGroup::RaiseCheckedButton()
{
    if (m_checked_button != NO_BUTTON)
        MoveChildUp(m_button_slots[m_checked_button].button.get());
}

void RadioButtonGroup::Render()
{
    if (m_render_outline) {
        Pt ul = UpperLeft(), lr = LowerRight();
        Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        FlatRectangle(ul, lr, CLR_ZERO, color_to_use, 1);
    }
}

void RadioButtonGroup::SetCheck(std::size_t index)
{
    if (m_button_slots.size() <= index)
        index = NO_BUTTON;
    SetCheckImpl(index, false);
}

void RadioButtonGroup::DisableButton(std::size_t index, bool b)
{
    if (index < m_button_slots.size()) {
        bool was_disabled = m_button_slots[index].button->Disabled();
        m_button_slots[index].button->Disable(b);
        if (b && !was_disabled && index == m_checked_button)
            SetCheck(NO_BUTTON);
    }
}

void RadioButtonGroup::AddButton(std::shared_ptr<StateButton> bn)
{ InsertButton(m_button_slots.size(), std::move(bn)); }

void RadioButtonGroup::InsertButton(std::size_t index, std::shared_ptr<StateButton> bn)
{
    assert(index <= m_button_slots.size());
    if (!m_expand_buttons) {
        Pt min_usable_size = bn->MinUsableSize();
        bn->Resize(Pt(std::max(bn->Width(), min_usable_size.x), std::max(bn->Height(), min_usable_size.y)));
    }
    Pt bn_sz = bn->Size();
    auto&& layout = GetLayout();
    if (!layout) {
        layout = Wnd::Create<Layout>(X0, Y0, ClientWidth(), ClientHeight(), 1, 1);
        SetLayout(layout);
    }
    const int CELLS_PER_BUTTON = m_expand_buttons ? 1 : 2;
    const int X_STRETCH = (m_expand_buttons && m_expand_buttons_proportionally) ? Value(bn_sz.x) : 1;
    const int Y_STRETCH = (m_expand_buttons && m_expand_buttons_proportionally) ? Value(bn_sz.y) : 1;
    if (m_button_slots.empty()) {
        layout->Add(bn, 0, 0);
        if (m_expand_buttons) {
            if (m_orientation == Orientation::VERTICAL)
                layout->SetRowStretch(0, Y_STRETCH);
            else
                layout->SetColumnStretch(0, X_STRETCH);
        }
    } else {
        if (m_orientation == Orientation::VERTICAL) {
            layout->ResizeLayout(layout->Rows() + CELLS_PER_BUTTON, 1);
            layout->SetRowStretch(layout->Rows() - CELLS_PER_BUTTON, Y_STRETCH);
        } else {
            layout->ResizeLayout(1, layout->Columns() + CELLS_PER_BUTTON);
            layout->SetColumnStretch(layout->Columns() - CELLS_PER_BUTTON, X_STRETCH);
        }
        for (std::size_t i = m_button_slots.size() - 1; index <= i; --i) {
            layout->Remove(m_button_slots[i].button.get());
            layout->Add(m_button_slots[i].button,
                        m_orientation == Orientation::VERTICAL ? i * CELLS_PER_BUTTON + CELLS_PER_BUTTON : 0,
                        m_orientation == Orientation::VERTICAL ? 0 : i * CELLS_PER_BUTTON + CELLS_PER_BUTTON);
            if (m_orientation == Orientation::VERTICAL)
                layout->SetMinimumRowHeight(i * CELLS_PER_BUTTON + CELLS_PER_BUTTON, layout->MinimumRowHeight(i * CELLS_PER_BUTTON));
            else
                layout->SetMinimumColumnWidth(i * CELLS_PER_BUTTON + CELLS_PER_BUTTON, layout->MinimumColumnWidth(i * CELLS_PER_BUTTON));
        }
        layout->Add(bn, m_orientation == Orientation::VERTICAL ? index * CELLS_PER_BUTTON : 0, m_orientation == Orientation::VERTICAL ? 0 : index * CELLS_PER_BUTTON);
    }
    if (m_orientation == Orientation::VERTICAL)
        layout->SetMinimumRowHeight(index * CELLS_PER_BUTTON, bn_sz.y);
    else
        layout->SetMinimumColumnWidth(index * CELLS_PER_BUTTON, bn_sz.x);
    m_button_slots.emplace(m_button_slots.begin() + index, ButtonSlot(std::move(bn)));

    if (m_checked_button != NO_BUTTON && index <= m_checked_button)
        ++m_checked_button;
    Reconnect();
}

void RadioButtonGroup::RemoveButton(StateButton* button)
{
    std::size_t index = NO_BUTTON;
    for (std::size_t i = 0; i < m_button_slots.size(); ++i) {
        if (m_button_slots[i].button.get() == button) {
            index = i;
            break;
        }
    }
    assert(index < m_button_slots.size());

    const int CELLS_PER_BUTTON = m_expand_buttons ? 1 : 2;
    auto&& layout = GetLayout();
    layout->Remove(m_button_slots[index].button.get());
    for (std::size_t i = index + 1; i < m_button_slots.size(); ++i) {
        layout->Remove(m_button_slots[i].button.get());
        if (m_orientation == Orientation::VERTICAL) {
            layout->Add(m_button_slots[i].button, i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, 0);
            layout->SetRowStretch(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->RowStretch(i * CELLS_PER_BUTTON));
            layout->SetMinimumRowHeight(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->MinimumRowHeight(i * CELLS_PER_BUTTON));
        } else {
            layout->Add(m_button_slots[i].button, 0, i * CELLS_PER_BUTTON - CELLS_PER_BUTTON);
            layout->SetColumnStretch(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->ColumnStretch(i * CELLS_PER_BUTTON));
            layout->SetMinimumColumnWidth(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->MinimumColumnWidth(i * CELLS_PER_BUTTON));
        }
    }
    m_button_slots[index].connection.disconnect();
    m_button_slots.erase(m_button_slots.begin() + index);
    if (m_button_slots.empty()) {
        layout->ResizeLayout(1, 1);
    } else {
        if (m_orientation == Orientation::VERTICAL)
            layout->ResizeLayout(layout->Rows() - CELLS_PER_BUTTON, 1);
        else
            layout->ResizeLayout(1, layout->Columns() - CELLS_PER_BUTTON);
    }

    if (index == m_checked_button)
        m_checked_button = NO_BUTTON;
    else if (index <= m_checked_button)
        --m_checked_button;
    Reconnect();
}

void RadioButtonGroup::ExpandButtons(bool expand)
{
    if (expand != m_expand_buttons) {
        std::size_t old_checked_button = m_checked_button;
        std::vector<std::shared_ptr<StateButton>> buttons(m_button_slots.size());
        while (!m_button_slots.empty()) {
            auto& button = m_button_slots.back().button;
            buttons[m_button_slots.size() - 1] = button;
            RemoveButton(button.get());
        }
        m_expand_buttons = expand;
        for (auto& button : buttons)
            AddButton(std::move(button));
        SetCheck(old_checked_button);
    }
}

void RadioButtonGroup::ExpandButtonsProportionally(bool proportional)
{
    if (proportional != m_expand_buttons_proportionally) {
        std::size_t old_checked_button = m_checked_button;
        std::vector<std::shared_ptr<StateButton>> buttons(m_button_slots.size());
        while (!m_button_slots.empty()) {
            auto& button = m_button_slots.back().button;
            buttons[m_button_slots.size() - 1] = button;
            RemoveButton(button.get());
        }
        m_expand_buttons_proportionally = proportional;
        for (auto& button : buttons)
            AddButton(std::move(button));
        SetCheck(old_checked_button);
    }
}

void RadioButtonGroup::RenderOutline(bool render_outline)
{ m_render_outline = render_outline; }

const std::vector<RadioButtonGroup::ButtonSlot>& RadioButtonGroup::ButtonSlots() const
{ return m_button_slots; }

void RadioButtonGroup::ConnectSignals()
{
    for (std::size_t i = 0; i < m_button_slots.size(); ++i) {
        m_button_slots[i].connection = m_button_slots[i].button->CheckedSignal.connect([this, i](bool checked) {
            if (checked)
                this->SetCheckImpl(i, true);
            else
                this->m_button_slots[i].button->SetCheck(true);
        });
    }
    SetCheck(m_checked_button);
}

void RadioButtonGroup::SetCheckImpl(std::size_t index, bool signal)
{
    assert(m_checked_button == NO_BUTTON || m_checked_button < m_button_slots.size());
    if (m_checked_button != NO_BUTTON)
        m_button_slots[m_checked_button].button->SetCheck(false);
    if (index != NO_BUTTON)
        m_button_slots[index].button->SetCheck(true);
    m_checked_button = index;
    if (signal)
        ButtonChangedSignal(m_checked_button);
}

void RadioButtonGroup::Reconnect()
{
    for (ButtonSlot& button_slot : m_button_slots)
        button_slot.connection.disconnect();
    ConnectSignals();
}
