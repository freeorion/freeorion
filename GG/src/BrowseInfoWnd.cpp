//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <tuple>
#include <GG/BrowseInfoWnd.h>
#include <GG/DrawUtil.h>
#include <GG/Font.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


using namespace GG;

////////////////////////////////////////////////
// GG::BrowseInfoWnd
////////////////////////////////////////////////
void BrowseInfoWnd::Update(std::size_t mode, const Wnd* target)
{
    UpdateImpl(mode, target);
    Pt new_pos;
    if (PositionWnd) {
        new_pos = PositionWnd(m_cursor_pos, GUI::GetGUI()->GetCursor(), *this, *target);
    } else {
        const Y MARGIN(2);
        new_pos = m_cursor_pos - Pt(Width() / 2, Height() + MARGIN);
    }
    MoveTo(new_pos);
    Pt ul = UpperLeft(), lr = LowerRight();
    if (GUI::GetGUI()->AppWidth() <= lr.x)
        ul.x += GUI::GetGUI()->AppWidth() - lr.x;
    else if (ul.x < 0)
        ul.x = X0;
    if (GUI::GetGUI()->AppHeight() <= lr.y)
        ul.y += GUI::GetGUI()->AppHeight() - lr.y;
    else if (ul.y < 0)
        ul.y = Y0;
    MoveTo(ul);
}


////////////////////////////////////////////////
// GG::TextBoxBrowseInfoWnd
////////////////////////////////////////////////
TextBoxBrowseInfoWnd::TextBoxBrowseInfoWnd(X w, const std::shared_ptr<Font>& font, Clr color,
                                           Clr border_color, Clr text_color, Flags<TextFormat> format,
                                           unsigned int border_width, unsigned int text_margin) :
    BrowseInfoWnd(X0, Y0, w, Y(100)),
    m_text_from_target(true),
    m_font(font),
    m_color(color),
    m_border_color(border_color),
    m_border_width(border_width),
    m_preferred_width(w),
    m_text_control(GetStyleFactory()->NewTextControl("", m_font, text_color, format)),
    m_text_margin(text_margin)
{}

void TextBoxBrowseInfoWnd::CompleteConstruction()
{
    m_text_control->Resize(Pt(Width(), m_text_control->Height()));
    AttachChild(m_text_control);
    GridLayout();
    SetLayoutBorderMargin(m_text_margin);
    InitBuffer();
}

bool TextBoxBrowseInfoWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const
{
    assert(mode <= wnd->BrowseModes().size());
    return !wnd->BrowseInfoText(mode).empty();
}

const std::string& TextBoxBrowseInfoWnd::Text() const
{ return m_text_control->Text(); }

Clr TextBoxBrowseInfoWnd::TextColor() const
{ return m_text_control->TextColor(); }

Flags<TextFormat> TextBoxBrowseInfoWnd::GetTextFormat() const
{ return m_text_control->GetTextFormat(); }

unsigned int TextBoxBrowseInfoWnd::TextMargin() const
{ return GetLayout()->BorderMargin(); }

void TextBoxBrowseInfoWnd::SetText(std::string str)
{
    unsigned int margins = 2 * TextMargin();
    bool str_empty = str.empty();
    Flags<TextFormat> fmt = GetTextFormat();
    auto text_elements = m_font->ExpensiveParseFromTextToTextElements(str, fmt);
    auto lines = m_font->DetermineLines(str, fmt, m_preferred_width - X(margins),
                                        text_elements);
    Pt extent = m_font->TextExtent(lines);
    SetMinSize(extent + Pt(X(margins), Y(margins)));
    m_text_control->SetText(std::move(str));
    Resize(extent + Pt(X(margins), Y0));
    if (str_empty)
        Hide();
    else
        Show();
}

void TextBoxBrowseInfoWnd::InitBuffer()
{
    GG::Pt sz = Size();
    m_buffer.clear();
    m_buffer.store(0.0f,        0.0f);
    m_buffer.store(Value(sz.x), 0.0f);
    m_buffer.store(Value(sz.x), Value(sz.y));
    m_buffer.store(0.0f,        Value(sz.y));
    m_buffer.store(0.0f,        0.0f);
    m_buffer.createServerBuffer();
}

void TextBoxBrowseInfoWnd::SizeMove(const Pt& ul, const Pt& lr)
{
    Pt sz = Size();
    BrowseInfoWnd::SizeMove(ul, lr);
    if (sz != Size())
        InitBuffer();
}

void TextBoxBrowseInfoWnd::Render()
{
    Pt ul = UpperLeft();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(m_border_width);
    glEnableClientState(GL_VERTEX_ARRAY);

    m_buffer.activate();
    glColor(m_color);
    glDrawArrays(GL_TRIANGLE_FAN,   0, m_buffer.size() - 1);
    glColor(m_border_color);
    glDrawArrays(GL_LINE_STRIP,     0, m_buffer.size());


    glLineWidth(1.0f);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}

void TextBoxBrowseInfoWnd::SetTextFromTarget(bool b)
{ m_text_from_target = b; }

void TextBoxBrowseInfoWnd::SetFont(std::shared_ptr<Font> font)
{ m_font = std::move(font); }

void TextBoxBrowseInfoWnd::SetColor(Clr color)
{ m_color = color; }

void TextBoxBrowseInfoWnd::SetBorderColor(Clr border_color)
{ m_border_color = border_color; }

void TextBoxBrowseInfoWnd::SetTextColor(Clr text_color)
{ m_text_control->SetTextColor(text_color); }

void TextBoxBrowseInfoWnd::SetTextFormat(Flags<TextFormat> format)
{ m_text_control->SetTextFormat(format); }

void TextBoxBrowseInfoWnd::SetBorderWidth(unsigned int border_width)
{ m_border_width = border_width; }

void TextBoxBrowseInfoWnd::SetTextMargin(unsigned int text_margin)
{ SetLayoutBorderMargin(text_margin); }

void TextBoxBrowseInfoWnd::UpdateImpl(std::size_t mode, const Wnd* target)
{
    if (m_text_from_target)
        SetText(target->BrowseInfoText(mode));
}
