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

#include <GG/Menu.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    const int BORDER_THICKNESS = 1; // thickness with which to draw menu borders
}


////////////////////////////////////////////////
// GG::MenuItem
////////////////////////////////////////////////
MenuItem::MenuItem() :
    MenuItem("", false, false)
{}

MenuItem::MenuItem(bool separator_) :
    disabled(true),
    checked(false),
    separator(true),
    next_level(),
    m_selected_on_close_callback{}
{}

MenuItem::MenuItem(const std::string& str, bool disable, bool check,
                   std::function<void()> selected_on_close_callback) :
    label(str),
    disabled(disable),
    checked(check),
    separator(false),
    next_level(),
    m_selected_on_close_callback{selected_on_close_callback}
{}

MenuItem::~MenuItem()
{}


////////////////////////////////////////////////
// GG::PopupMenu
////////////////////////////////////////////////
namespace {
    // distance to leave between edge of PopupMenuClassic contents and the control's border
    const X HORIZONTAL_MARGIN(3);
}

const std::size_t PopupMenu::INVALID_CARET = std::numeric_limits<std::size_t>::max();

PopupMenu::PopupMenu(X x, Y y, const std::shared_ptr<Font>& font, Clr text_color/* = CLR_WHITE*/,
                     Clr border_color/* = CLR_BLACK*/, Clr interior_color/* = CLR_SHADOW*/, Clr hilite_color/* = CLR_GRAY*/) :
    Wnd(X0, Y0, GUI::GetGUI()->AppWidth() - 1, GUI::GetGUI()->AppHeight() - 1, INTERACTIVE | MODAL),
    m_font(font),
    m_border_color(border_color),
    m_int_color(interior_color),
    m_text_color(text_color),
    m_hilite_color(hilite_color),
    m_sel_text_color(text_color),
    m_menu_data(MenuItem()),
    m_open_levels(),
    m_caret(1, INVALID_CARET),
    m_origin(x, y),
    m_item_selected(nullptr)
{
    m_open_levels.resize(1);
}

void PopupMenu::AddMenuItem(MenuItem&& menu_item)
{
    m_menu_data.next_level.push_back(std::forward<MenuItem>(menu_item));
}

Pt PopupMenu::ClientUpperLeft() const
{ return m_origin; }

Clr PopupMenu::BorderColor() const
{ return m_border_color; }

Clr PopupMenu::InteriorColor() const
{ return m_int_color; }

Clr PopupMenu::TextColor() const
{ return m_text_color; }

Clr PopupMenu::HiliteColor() const
{ return m_hilite_color; }

Clr PopupMenu::SelectedTextColor() const
{ return m_sel_text_color; }

void PopupMenu::Render()
{
    if (m_menu_data.next_level.size())
    {
        Pt ul = ClientUpperLeft();

        const Y INDICATOR_VERTICAL_MARGIN(3);
        const Y INDICATOR_HEIGHT = m_font->Lineskip() - 2 * INDICATOR_VERTICAL_MARGIN;
        const Y CHECK_HEIGHT = INDICATOR_HEIGHT;
        const X CHECK_WIDTH(Value(CHECK_HEIGHT));

        X next_menu_x_offset(0);
        Y next_menu_y_offset(0);
        for (std::size_t i = 0; i < m_caret.size(); ++i) {
            bool needs_indicator = false;

            // get the correct submenu
            MenuItem* menu_ptr = &m_menu_data;
            for (std::size_t j = 0; j < i; ++j)
                menu_ptr = &menu_ptr->next_level[m_caret[j]];
            MenuItem& menu = *menu_ptr;

            // determine the total size of the menu, render it, and record its bounding rect
            std::string str;
            for (std::size_t j = 0; j < menu.next_level.size(); ++j) {
                str += menu.next_level[j].label + (static_cast<int>(j) < static_cast<int>(menu.next_level.size()) - 1 ? "\n" : "");
                if (menu.next_level[j].next_level.size() || menu.next_level[j].checked)
                    needs_indicator = true;
            }
            Flags<TextFormat> fmt = FORMAT_LEFT | FORMAT_TOP;
            auto text_elements = m_font->ExpensiveParseFromTextToTextElements(str, fmt);
            auto lines = m_font->DetermineLines(str, fmt, X0, text_elements);
            Pt menu_sz = m_font->TextExtent(lines); // get dimensions of text in menu
            menu_sz.x += 2 * HORIZONTAL_MARGIN;
            if (needs_indicator)
                menu_sz.x += CHECK_WIDTH + 2 * HORIZONTAL_MARGIN; // make room for the little arrow
            Rect r(ul.x + next_menu_x_offset, ul.y + next_menu_y_offset,
                   ul.x + next_menu_x_offset + menu_sz.x, ul.y + next_menu_y_offset + menu_sz.y);

            if (r.lr.x > GUI::GetGUI()->AppWidth()) {
                X offset = r.lr.x - GUI::GetGUI()->AppWidth();
                r.ul.x -= offset;
                r.lr.x -= offset;
            }
            if (r.lr.y > GUI::GetGUI()->AppHeight()) {
                Y offset = r.lr.y - GUI::GetGUI()->AppHeight();
                r.ul.y -= offset;
                r.lr.y -= offset;
            }
            next_menu_x_offset = menu_sz.x;
            next_menu_y_offset = static_cast<int>(m_caret[i]) * m_font->Lineskip();
            FlatRectangle(r.ul, r.lr, m_int_color, m_border_color, BORDER_THICKNESS);
            m_open_levels[i] = r;

            // paint caret, if any
            if (m_caret[i] != INVALID_CARET &&
                !menu.next_level[m_caret[i]].separator &&
                !menu.next_level[m_caret[i]].disabled)
            {
                Rect tmp_r = r;
                tmp_r.ul.y += static_cast<int>(m_caret[i]) * m_font->Lineskip();
                tmp_r.lr.y = tmp_r.ul.y + m_font->Lineskip() + 3;
                tmp_r.ul.x += BORDER_THICKNESS;
                tmp_r.lr.x -= BORDER_THICKNESS;
                if (m_caret[i] == 0)
                    tmp_r.ul.y += BORDER_THICKNESS;
                if (m_caret[i] == menu.next_level.size() - 1)
                    tmp_r.lr.y -= BORDER_THICKNESS;
                FlatRectangle(tmp_r.ul, tmp_r.lr, m_hilite_color, CLR_ZERO, 0);
            }

            // paint menu text and submenu indicator arrows
            Rect line_rect = r;
            line_rect.ul.x += HORIZONTAL_MARGIN;
            line_rect.lr.x -= HORIZONTAL_MARGIN;
            for (std::size_t j = 0; j < menu.next_level.size(); ++j) {
                Clr clr =   (m_caret[i] == j)
                                ? (menu.next_level[j].disabled
                                    ? DisabledColor(m_sel_text_color)
                                    : m_sel_text_color)
                                : (menu.next_level[j].disabled
                                    ? DisabledColor(m_text_color)
                                    : m_text_color);
                glColor3ub(clr.r, clr.g, clr.b);

                if (!menu.next_level[j].separator) {
                    // TODO cache line data v expensive calculation
                    auto element_data = m_font->ExpensiveParseFromTextToTextElements(menu.next_level[j].label, fmt);
                    auto line_data = m_font->DetermineLines(menu.next_level[j].label, fmt, X0, element_data);

                    m_font->RenderText(line_rect.ul, line_rect.lr, menu.next_level[j].label, fmt, line_data);

                } else {
                    Line(line_rect.ul.x + HORIZONTAL_MARGIN, line_rect.ul.y + INDICATOR_HEIGHT/2 + INDICATOR_VERTICAL_MARGIN,
                         line_rect.lr.x - HORIZONTAL_MARGIN, line_rect.ul.y + INDICATOR_HEIGHT/2 + INDICATOR_VERTICAL_MARGIN);
                }

                if (menu.next_level[j].checked) {
                    FlatCheck(Pt(line_rect.lr.x - CHECK_WIDTH - HORIZONTAL_MARGIN, line_rect.ul.y + INDICATOR_VERTICAL_MARGIN),
                              Pt(line_rect.lr.x - HORIZONTAL_MARGIN, line_rect.ul.y + INDICATOR_VERTICAL_MARGIN + CHECK_HEIGHT),
                              clr);
                }

                // submenu indicator arrow
                if (menu.next_level[j].next_level.size() > 0u) {
                    Triangle(line_rect.lr.x - Value(INDICATOR_HEIGHT/2) - HORIZONTAL_MARGIN,
                             line_rect.ul.y + INDICATOR_VERTICAL_MARGIN,
                             line_rect.lr.x - Value(INDICATOR_HEIGHT/2) - HORIZONTAL_MARGIN,
                             line_rect.ul.y + m_font->Lineskip() - INDICATOR_VERTICAL_MARGIN,
                             line_rect.lr.x - HORIZONTAL_MARGIN,
                             line_rect.ul.y + m_font->Lineskip()/2);
                    glEnd();
                    glEnable(GL_TEXTURE_2D);
                }
                line_rect.ul.y += m_font->Lineskip();
            }
        }
    }
}

void PopupMenu::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (m_caret[0] != INVALID_CARET) {
        MenuItem* menu_ptr = &m_menu_data;
        for (std::size_t caret : m_caret) {
            if (caret != INVALID_CARET) {
                menu_ptr = &menu_ptr->next_level[caret];
            }
        }
        if (!menu_ptr->disabled && !menu_ptr->separator) {
            m_item_selected = menu_ptr;
            m_done = true;
        }
    } else {
        m_done = true;
    }
}

void PopupMenu::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ LButtonUp(pt, mod_keys); }

void PopupMenu::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    bool cursor_is_in_menu = false;
    for (int i = static_cast<int>(m_open_levels.size()) - 1; i >= 0; --i) {
        // get the correct submenu
        MenuItem* menu_ptr = &m_menu_data;
        for (int j = 0; j < i; ++j)
            menu_ptr = &menu_ptr->next_level[m_caret[j]];
        MenuItem& menu = *menu_ptr;

        if (pt.x >= m_open_levels[i].ul.x && pt.x <= m_open_levels[i].lr.x &&
            pt.y >= m_open_levels[i].ul.y && pt.y <= m_open_levels[i].lr.y)
        {
            std::size_t row_selected = Value((pt.y - m_open_levels[i].ul.y) / m_font->Lineskip());
            if (row_selected == m_caret[i]) {
                cursor_is_in_menu = true;
            } else if (row_selected < menu.next_level.size()) {
                m_caret[i] = row_selected;
                m_open_levels.resize(i + 1);
                m_caret.resize(i + 1);
                if (!menu.next_level[row_selected].disabled && menu.next_level[row_selected].next_level.size()) {
                    m_caret.push_back(INVALID_CARET);
                    m_open_levels.push_back(Rect());
                }
                cursor_is_in_menu = true;
            }
        }
    }
    if (!cursor_is_in_menu) {
        m_open_levels.resize(1);
        m_caret.resize(1);
        m_caret[0] = INVALID_CARET;
    }
}

void PopupMenu::RButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{ LButtonUp(pt, mod_keys); }

void PopupMenu::RClick(const Pt& pt, Flags<ModKey> mod_keys)
{ LButtonUp(pt, mod_keys); }

void PopupMenu::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{ LDrag(pt, Pt(), mod_keys); }

bool PopupMenu::Run()
{
    bool retval = Wnd::Run();
    if (retval
        && m_item_selected
        && m_item_selected->m_selected_on_close_callback)
    {
        m_item_selected->m_selected_on_close_callback();
    }

    return retval;
}

void PopupMenu::SetBorderColor(Clr clr)
{ m_border_color = clr; }

void PopupMenu::SetInteriorColor(Clr clr)
{ m_int_color = clr; }

void PopupMenu::SetTextColor(Clr clr)
{ m_text_color = clr; }

void PopupMenu::SetHiliteColor(Clr clr)
{ m_hilite_color = clr; }

void PopupMenu::SetSelectedTextColor(Clr clr)
{ m_sel_text_color = clr; }

const std::shared_ptr<Font>& PopupMenu::GetFont() const
{ return m_font; }

const MenuItem& PopupMenu::MenuData() const
{ return m_menu_data; }

const std::vector<Rect>& PopupMenu::OpenLevels() const
{ return m_open_levels; }

const std::vector<std::size_t>& PopupMenu::Caret() const
{ return m_caret; }

const MenuItem* PopupMenu::ItemSelected() const
{ return m_item_selected; }
