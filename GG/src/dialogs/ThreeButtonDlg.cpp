//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Button.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>


using namespace GG;

ThreeButtonDlg::ThreeButtonDlg(X w, Y h, std::string msg, const std::shared_ptr<Font>& font,
                               Clr color, Clr border_color, Clr button_color, Clr text_color,
                               std::size_t buttons, std::string zero, std::string one, std::string two) :
    Wnd((GUI::GetGUI()->AppWidth() - w) / 2, (GUI::GetGUI()->AppHeight() - h) / 2,
        w, h, INTERACTIVE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_button_color(button_color),
    m_escape(buttons - 1),
    m_button_layout(Wnd::Create<Layout>(X0, Y0, X1, Y1, 2, 1, 10))
{
    if (buttons < 1)
        buttons = 1;
    else if (3 < buttons)
        buttons = 3;

    constexpr int SPACING = 10;
    const Y BUTTON_HEIGHT = font->Height() + 10;

    auto button_layout = Wnd::Create<Layout>(X0, Y0, X1, Y1, 1, buttons, 0, 10);

    const auto& style = GetStyleFactory();

    auto message_text =
        style.NewTextControl(std::move(msg), font, m_text_color,
                             FORMAT_CENTER | FORMAT_VCENTER | FORMAT_WORDBREAK);
    message_text->Resize(Pt(ClientWidth() - 2 * SPACING, Height()));
    message_text->SetResetMinSize(true);
    m_button_layout->Add(std::move(message_text), 0, 0);
    m_button_layout->SetRowStretch(0, 1);
    m_button_layout->SetMinimumRowHeight(1, BUTTON_HEIGHT);

    m_button_0 = style.NewButton((zero.empty() ? (buttons < 3 ? "Ok" : "Yes") : std::move(zero)),
                                 font, m_button_color, m_text_color);
    button_layout->Add(m_button_0, 0, 0);

    if (2 <= buttons) {
        m_button_1 = style.NewButton((one.empty() ? (buttons < 3 ? "Cancel" : "No") : std::move(one)),
                                     font, m_button_color, m_text_color);
        button_layout->Add(m_button_1, 0, 1);
    }
    if (3 <= buttons) {
        m_button_2 = style.NewButton((two.empty() ? "Cancel" : std::move(two)),
                                     font, m_button_color, m_text_color);
        button_layout->Add(m_button_2, 0, 2);
    }
    m_button_layout->Add(std::move(button_layout), 1, 0);
}

void ThreeButtonDlg::CompleteConstruction()
{
    Wnd::CompleteConstruction();

    SetLayout(m_button_layout);

    m_button_0->LeftClickedSignal.connect(
        boost::bind(&ThreeButtonDlg::Button0Clicked, this));
    if (m_button_1)
        m_button_1->LeftClickedSignal.connect(
            boost::bind(&ThreeButtonDlg::Button1Clicked, this));
    if (m_button_2)
        m_button_2->LeftClickedSignal.connect(
            boost::bind(&ThreeButtonDlg::Button2Clicked, this));
}

Clr ThreeButtonDlg::ButtonColor() const
{ return m_button_color; }

std::size_t ThreeButtonDlg::Result() const
{ return m_result; }

std::size_t ThreeButtonDlg::DefaultButton() const
{ return m_default; }

std::size_t ThreeButtonDlg::EscapeButton() const
{ return m_escape; }

void ThreeButtonDlg::Render()
{ FlatRectangle(UpperLeft(), LowerRight(), m_color, m_border_color, 1); }

void ThreeButtonDlg::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (key == Key::GGK_RETURN || key == Key::GGK_KP_ENTER) {
        if (m_default == 0)
            Button0Clicked();
        else if (m_default == 1)
            Button1Clicked();
        else if (m_default == 2)
            Button2Clicked();
    } else if (key == Key::GGK_ESCAPE) {
        if (m_escape == 0)
            Button0Clicked();
        else if (m_escape == 1)
            Button1Clicked();
        else if (m_escape == 2)
            Button2Clicked();
    }
}

void ThreeButtonDlg::SetButtonColor(Clr color)
{
    m_button_color = color;
    if (m_button_0)
        m_button_0->SetColor(color);
    if (m_button_1)
        m_button_1->SetColor(color);
    if (m_button_2)
        m_button_2->SetColor(color);
}

void ThreeButtonDlg::SetDefaultButton(std::size_t i)
{
    if (NumButtons() <= i)
        m_default = NO_BUTTON;
    else
        m_default = i;
}

void ThreeButtonDlg::SetEscapeButton(std::size_t i)
{
    if (NumButtons() <= i)
        m_escape = NO_BUTTON;
    else
        m_escape = i;
}

std::size_t ThreeButtonDlg::NumButtons() const
{
    std::size_t retval = 1;
    if (m_button_2)
        retval = 3;
    else if (m_button_1)
        retval = 2;
    return retval;
}

void ThreeButtonDlg::Button0Clicked()
{
    m_modal_done.store(true);
    m_result = 0;
}

void ThreeButtonDlg::Button1Clicked()
{
    m_modal_done.store(true);
    m_result = 1;
}

void ThreeButtonDlg::Button2Clicked()
{
    m_modal_done.store(true);
    m_result = 2;
}
