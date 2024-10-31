//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/DrawUtil.h>
#include <GG/GroupBox.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


using namespace GG;

namespace {
    Y TopOfFrame(bool label, const std::shared_ptr<Font>& font)
    { return label ? font->Lineskip() / 2 - 1 : Y0; }

    constexpr int FRAME_THICK = 2;
    constexpr int PIXEL_MARGIN = 4;
}

////////////////////////////////////////////////
// GG::GroupBox
////////////////////////////////////////////////
GroupBox::GroupBox()
{}

GroupBox::GroupBox(X x, Y y, X w, Y h, std::string label, const std::shared_ptr<Font>& font,
                   Clr color, Clr text_color, Clr interior,
                   Flags<WndFlag> flags) :
    m_color(color),
    m_text_color(text_color),
    m_int_color(interior),
    m_font(font),
    m_label(label.empty() ? nullptr : GUI::GetGUI()->GetStyleFactory().NewTextControl(
        std::move(label), m_font, m_text_color, FORMAT_LEFT | FORMAT_TOP))
{}

void GroupBox::CompleteConstruction()
{
    if (m_label) {
        m_label->MoveTo(Pt(X0, -m_font->Lineskip()));
        m_label->MoveTo(Pt(X1, m_font->Lineskip()));
        AttachChild(m_label);
    }
}

Pt GroupBox::ClientUpperLeft() const noexcept
{
    Pt retval = UpperLeft();
    if (!m_set_client_corners_equal_to_box_corners)
        retval += Pt(X(FRAME_THICK + PIXEL_MARGIN),
                     Y(FRAME_THICK + PIXEL_MARGIN) + TopOfFrame(m_label != nullptr, m_font));
    return retval;
}

Pt GroupBox::ClientLowerRight() const noexcept
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
    Clr light = LightenClr(m_color);
    Clr dark = DarkenClr(m_color);
    static constexpr int GAP_FROM_TEXT = 2;
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

void GroupBox::SetText(std::string str)
{
    if (!str.empty()) {
        m_label = GUI::GetGUI()->GetStyleFactory().NewTextControl(std::move(str), m_font, m_text_color);
        m_label->MoveTo(Pt(X(FRAME_THICK + PIXEL_MARGIN), Y0));
        m_label->Resize(Pt(X1, m_font->Lineskip()));
    }
}
