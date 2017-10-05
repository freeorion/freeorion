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

#include <GG/GroupBox.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


using namespace GG;

namespace {
    Y TopOfFrame(bool label, const std::shared_ptr<Font>& font)
    { return label ? font->Lineskip() / 2 - 1 : Y0; }
}

////////////////////////////////////////////////
// GG::GroupBox
////////////////////////////////////////////////
// static(s)
const int GroupBox::FRAME_THICK = 2;
const int GroupBox::PIXEL_MARGIN = 4;

GroupBox::GroupBox() :
    m_label(nullptr),
    m_set_client_corners_equal_to_box_corners(false)
{}

GroupBox::GroupBox(X x, Y y, X w, Y h, const std::string& label, const std::shared_ptr<Font>& font,
                   Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                   Flags<WndFlag> flags/* = NO_WND_FLAGS*/) :
    m_color(color),
    m_text_color(text_color),
    m_int_color(interior),
    m_font(font),
    m_label(label.empty() ? nullptr : GUI::GetGUI()->GetStyleFactory()->NewTextControl(label, m_font, m_text_color, FORMAT_LEFT | FORMAT_TOP)),
    m_set_client_corners_equal_to_box_corners(false)
{}

void GroupBox::CompleteConstruction()
{
    if (m_label) {
        m_label->MoveTo(Pt(X0, -m_font->Lineskip()));
        m_label->MoveTo(Pt(X1, m_font->Lineskip()));
        AttachChild(m_label);
    }
}

Pt GroupBox::ClientUpperLeft() const
{
    Pt retval = UpperLeft();
    if (!m_set_client_corners_equal_to_box_corners)
        retval += Pt(X(FRAME_THICK + PIXEL_MARGIN),
                     Y(FRAME_THICK + PIXEL_MARGIN) + TopOfFrame(m_label != nullptr, m_font));
    return retval;
}

Pt GroupBox::ClientLowerRight() const
{
    Pt retval = LowerRight();
    if (!m_set_client_corners_equal_to_box_corners)
        retval -= Pt(X(FRAME_THICK + PIXEL_MARGIN), Y(FRAME_THICK + PIXEL_MARGIN));
    return retval;
}

void GroupBox::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight() - Pt(X1, Y1);
    ul.y += TopOfFrame(m_label != nullptr, m_font);
    Clr light = LightColor(m_color);
    Clr dark = DarkColor(m_color);
    const int GAP_FROM_TEXT = 2;
    int vertices[24] = {
        Value(ul.x) + FRAME_THICK + PIXEL_MARGIN - GAP_FROM_TEXT, Value(ul.y),
        Value(ul.x), Value(ul.y),
        Value(ul.x), Value(lr.y),
        Value(lr.x), Value(lr.y),
        Value(lr.x), Value(ul.y),
        Value(ul.x) + FRAME_THICK + PIXEL_MARGIN - GAP_FROM_TEXT, Value(ul.y)
    };
    if (m_label) {
        vertices[0] = Value(m_label->TextUpperLeft().x - GAP_FROM_TEXT);
        vertices[10] = Value(m_label->TextLowerRight().x + GAP_FROM_TEXT);
    }

    for (std::size_t i = 0; i < 12; i += 2) {
        vertices[12 + i + 0] = vertices[i + 0] + 1;
        vertices[12 + i + 1] = vertices[i + 1] + 1;
    }
    --vertices[12];
    --vertices[22];

    glDisable(GL_TEXTURE_2D);

    glColor(light);
    glBegin(GL_LINE_STRIP);
    for (std::size_t i = 12; i < 24; i += 2) {
        glVertex2i(vertices[i + 0], vertices[i + 1]);
    }
    glEnd();

    glColor(dark);
    glBegin(GL_LINE_STRIP);
    for (std::size_t i = 0; i < 12; i += 2) {
        glVertex2i(vertices[i + 0], vertices[i + 1]);
    }
    glEnd();

    glColor(m_int_color);
    glBegin(GL_QUADS);
    glVertex2i(vertices[14] + 1, vertices[5] - 1);
    glVertex2i(vertices[14] + 1, vertices[13] + 1);
    glVertex2i(vertices[6] - 1,  vertices[13] + 1);
    glVertex2i(vertices[6] - 1,  vertices[5] - 1);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

void GroupBox::SetColor(Clr c)
{ m_color = c; }

void GroupBox::SetTextColor(Clr c)
{ m_text_color = c; }

void GroupBox::SetInteriorColor(Clr c)
{ m_int_color = c; }

void GroupBox::SetClientCornersEqualToBoxCorners(bool b)
{
    if (b != m_set_client_corners_equal_to_box_corners) {
        m_set_client_corners_equal_to_box_corners = b;
        if (m_label) {
            if (m_set_client_corners_equal_to_box_corners)
                m_label->MoveTo(Pt(X(FRAME_THICK + PIXEL_MARGIN), Y0));
            else
                m_label->MoveTo(Pt(X0, -m_font->Lineskip()));
        }
    }
}

void GroupBox::SetText(const std::string& str)
{
    if (!str.empty()) {
        m_label = GUI::GetGUI()->GetStyleFactory()->NewTextControl(str, m_font, m_text_color);
        m_label->MoveTo(Pt(X(FRAME_THICK + PIXEL_MARGIN), Y0));
        m_label->Resize(Pt(X1, m_font->Lineskip()));
    }
}
