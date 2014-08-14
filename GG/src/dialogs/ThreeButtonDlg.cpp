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

#include <GG/dialogs/ThreeButtonDlg.h>

#include <GG/GUI.h>
#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>


using namespace GG;

const std::size_t ThreeButtonDlg::NO_BUTTON = std::numeric_limits<std::size_t>::max();

ThreeButtonDlg::ThreeButtonDlg(X x, Y y, X w, Y h, const std::string& msg, const boost::shared_ptr<Font>& font,
                               Clr color, Clr border_color, Clr button_color, Clr text_color, std::size_t buttons,
                               const std::string& zero/* = ""*/, const std::string& one/* = ""*/,
                               const std::string& two/* = ""*/) :
    Wnd(x, y, w, h, INTERACTIVE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_button_color(button_color),
    m_default(0),
    m_escape(buttons - 1),
    m_result(0),
    m_button_0(0),
    m_button_1(0),
    m_button_2(0)
{ Init(msg, font, buttons, zero, one, two); }

ThreeButtonDlg::ThreeButtonDlg(X w, Y h, const std::string& msg, const boost::shared_ptr<Font>& font,
                               Clr color, Clr border_color, Clr button_color, Clr text_color, std::size_t buttons,
                               const std::string& zero/* = ""*/, const std::string& one/* = ""*/, const std::string& two/* = ""*/) :
    Wnd((GUI::GetGUI()->AppWidth() - w) / 2, (GUI::GetGUI()->AppHeight() - h) / 2, w, h, INTERACTIVE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_button_color(button_color),
    m_default(0),
    m_escape(buttons - 1),
    m_result(0),
    m_button_0(0),
    m_button_1(0),
    m_button_2(0)
{ Init(msg, font, buttons, zero, one, two); }

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

void ThreeButtonDlg::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (key == GGK_RETURN || key == GGK_KP_ENTER) {
        if (m_default == 0)
            Button0Clicked();
        else if (m_default == 1)
            Button1Clicked();
        else if (m_default == 2)
            Button2Clicked();
    } else if (key == GGK_ESCAPE) {
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

void ThreeButtonDlg::Init(const std::string& msg, const boost::shared_ptr<Font>& font, std::size_t buttons,
                          const std::string& zero/* = ""*/, const std::string& one/* = ""*/,
                          const std::string& two/* = ""*/)
{
    if (buttons < 1)
        buttons = 1;
    else if (3 < buttons)
        buttons = 3;

    const int SPACING = 10;
    const Y BUTTON_HEIGHT = font->Height() + 10;

    Layout* layout = new Layout(X0, Y0, X1, Y1, 2, 1, 10);
    Layout* button_layout = new Layout(X0, Y0, X1, Y1, 1, buttons, 0, 10);

    boost::shared_ptr<StyleFactory> style = GetStyleFactory();

    TextControl* message_text = style->NewTextControl(X0, Y0, ClientWidth() - 2 * SPACING, Height(), msg, font, m_text_color,
                                                      FORMAT_CENTER | FORMAT_VCENTER | FORMAT_WORDBREAK);
    message_text->SetMinSize(true);
    layout->Add(message_text, 0, 0);
    layout->SetRowStretch(0, 1);
    layout->SetMinimumRowHeight(1, BUTTON_HEIGHT);

    m_button_0 = style->NewButton(X0, Y0, X1, Y1, (zero == "" ? (buttons < 3 ? "Ok" : "Yes") : zero),
                                  font, m_button_color, m_text_color);
    button_layout->Add(m_button_0, 0, 0);

    if (2 <= buttons) {
        m_button_1 = style->NewButton(X0, Y0, X1, Y1, (one == "" ? (buttons < 3 ? "Cancel" : "No") : one),
                                      font, m_button_color, m_text_color);
        button_layout->Add(m_button_1, 0, 1);
    }
    if (3 <= buttons) {
        m_button_2 = style->NewButton(X0, Y0, X1, Y1, (two == "" ? "Cancel" : two),
                                      font, m_button_color, m_text_color);
        button_layout->Add(m_button_2, 0, 2);
    }
    layout->Add(button_layout, 1, 0);

    SetLayout(layout);

    ConnectSignals();
}

void ThreeButtonDlg::ConnectSignals()
{
    Connect(m_button_0->LeftClickedSignal, &ThreeButtonDlg::Button0Clicked, this);
    if (m_button_1)
        Connect(m_button_1->LeftClickedSignal, &ThreeButtonDlg::Button1Clicked, this);
    if (m_button_2)
        Connect(m_button_2->LeftClickedSignal, &ThreeButtonDlg::Button2Clicked, this);
}

void ThreeButtonDlg::Button0Clicked()
{
    m_done = true;
    m_result = 0;
}

void ThreeButtonDlg::Button1Clicked()
{
    m_done = true;
    m_result = 1;
}

void ThreeButtonDlg::Button2Clicked()
{
    m_done = true;
    m_result = 2;
}
