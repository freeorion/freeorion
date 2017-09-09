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

#include <GG/BrowseInfoWnd.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/Font.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>

#include <tuple>


using namespace GG;

////////////////////////////////////////////////
// GG::BrowseInfoWnd
////////////////////////////////////////////////
BrowseInfoWnd::BrowseInfoWnd(X x, Y y, X w, Y h) :
    Wnd(x, y, w, h)
{}

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

void BrowseInfoWnd::SetCursorPosition(const Pt& cursor_pos)
{ m_cursor_pos = cursor_pos; }

void BrowseInfoWnd::UpdateImpl(std::size_t mode, const Wnd* target)
{}


////////////////////////////////////////////////
// GG::TextBoxBrowseInfoWnd
////////////////////////////////////////////////
TextBoxBrowseInfoWnd::TextBoxBrowseInfoWnd(X w, const std::shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color,
                                           Flags<TextFormat> format/* = FORMAT_LEFT | FORMAT_WORDBREAK*/,
                                           unsigned int border_width/* = 2*/, unsigned int text_margin/* = 4*/) :
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

bool TextBoxBrowseInfoWnd::TextFromTarget() const
{ return m_text_from_target; }

const std::string& TextBoxBrowseInfoWnd::Text() const
{ return m_text_control->Text(); }

const std::shared_ptr<Font>& TextBoxBrowseInfoWnd::GetFont() const
{ return m_font; }

Clr TextBoxBrowseInfoWnd::Color() const
{ return m_color; }

Clr TextBoxBrowseInfoWnd::TextColor() const
{ return m_text_control->TextColor(); }

Flags<TextFormat> TextBoxBrowseInfoWnd::GetTextFormat() const
{ return m_text_control->GetTextFormat(); }

Clr TextBoxBrowseInfoWnd::BorderColor() const
{ return m_border_color; }

unsigned int TextBoxBrowseInfoWnd::BorderWidth() const
{ return m_border_width; }

unsigned int TextBoxBrowseInfoWnd::TextMargin() const
{ return GetLayout()->BorderMargin(); }

void TextBoxBrowseInfoWnd::SetText(const std::string& str)
{
    unsigned int margins = 2 * TextMargin();

    Flags<TextFormat> fmt = GetTextFormat();
    auto text_elements = m_font->ExpensiveParseFromTextToTextElements(str, fmt);
    auto lines = m_font->DetermineLines(str, fmt, m_preferred_width - X(margins),
                                        text_elements);
    Pt extent = m_font->TextExtent(lines);
    SetMinSize(extent + Pt(X(margins), Y(margins)));
    m_text_control->SetText(str);
    Resize(extent + Pt(X(margins), Y0));
    if (str.empty())
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

void TextBoxBrowseInfoWnd::SetFont(const std::shared_ptr<Font>& font)
{ m_font = font; }

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
