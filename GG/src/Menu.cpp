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
    struct MenuSignalEcho
    {
        MenuSignalEcho(const std::string& name) : m_name(name) {}
        void operator()()
        { std::cerr << "GG SIGNAL : " << m_name << "()" << std::endl; }
        void operator()(int id)
        { std::cerr << "GG SIGNAL : " << m_name << "(id=" << id << ")" << std::endl; }
        std::string m_name;
    };

    const int BORDER_THICKNESS = 1; // thickness with which to draw menu borders
    const int MENU_SEPARATION = 10; // distance between menu texts in a MenuBar, in pixels
}


////////////////////////////////////////////////
// GG::MenuItem
////////////////////////////////////////////////
MenuItem::MenuItem() :
    SelectedIDSignal(new SelectedIDSignalType()),
    SelectedSignal(new SelectedSignalType()),
    item_ID(0),
    disabled(false),
    checked(false),
    separator(false)
{}

MenuItem::MenuItem(bool separator) :
    SelectedIDSignal(),
    SelectedSignal(),
    item_ID(0),
    disabled(true),
    checked(false),
    separator(true)
{}

MenuItem::MenuItem(const std::string& str, int id, bool disable, bool check) :
    SelectedIDSignal(new SelectedIDSignalType()),
    SelectedSignal(new SelectedSignalType()),
    label(str),
    item_ID(id),
    disabled(disable),
    checked(check),
    separator(false)
{
    if (INSTRUMENT_ALL_SIGNALS) {
        Connect(*SelectedIDSignal, MenuSignalEcho("MenuItem::SelectedIDSignal"));
        Connect(*SelectedSignal, MenuSignalEcho("MenuItem::SelectedSignal"));
    }
}

MenuItem::MenuItem(const std::string& str, int id, bool disable, bool check, const SelectedIDSlotType& slot) :
    SelectedIDSignal(new SelectedIDSignalType()),
    SelectedSignal(new SelectedSignalType()),
    label(str),
    item_ID(id),
    disabled(disable),
    checked(check),
    separator(false)
{
    SelectedIDSignal->connect(slot);

    if (INSTRUMENT_ALL_SIGNALS) {
        Connect(*SelectedIDSignal, MenuSignalEcho("MenuItem::SelectedIDSignal"));
        Connect(*SelectedSignal, MenuSignalEcho("MenuItem::SelectedSignal"));
    }
}

MenuItem::MenuItem(const std::string& str, int id, bool disable, bool check, const SelectedSlotType& slot) :
    SelectedIDSignal(new SelectedIDSignalType()),
    SelectedSignal(new SelectedSignalType()),
    label(str),
    item_ID(id),
    disabled(disable),
    checked(check),
    separator(false)
{
    SelectedSignal->connect(slot);

    if (INSTRUMENT_ALL_SIGNALS) {
        Connect(*SelectedIDSignal, MenuSignalEcho("MenuItem::SelectedIDSignal"));
        Connect(*SelectedSignal, MenuSignalEcho("MenuItem::SelectedSignal"));
    }
}

MenuItem::~MenuItem()
{}


////////////////////////////////////////////////
// GG::MenuBar
////////////////////////////////////////////////
const std::size_t MenuBar::INVALID_CARET = std::numeric_limits<std::size_t>::max();

MenuBar::MenuBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, Clr text_color/* = CLR_WHITE*/,
                 Clr color/* = CLR_BLACK*/, Clr interior/* = CLR_SHADOW*/) :
    Control(x, y, w, font->Lineskip()),
    m_font(font),
    m_border_color(color),
    m_int_color(interior),
    m_text_color(text_color),
    m_sel_text_color(text_color),
    m_caret(INVALID_CARET)
{
    // use opaque interior color as hilite color
    interior.a = 255;
    m_hilite_color = interior;
    AdjustLayout();

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(BrowsedSignal, MenuSignalEcho("MenuBar::BrowsedSignal"));
}

MenuBar::MenuBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, const MenuItem& m,
                 Clr text_color/* = CLR_WHITE*/, Clr color/* = CLR_BLACK*/, Clr interior/* = CLR_SHADOW*/) :
    Control(x, y, w, font->Lineskip()),
    m_font(font),
    m_border_color(color),
    m_int_color(interior),
    m_text_color(text_color),
    m_sel_text_color(text_color),
    m_menu_data(m),
    m_caret(INVALID_CARET)
{
    // use opaque interior color as hilite color
    interior.a = 255;
    m_hilite_color = interior;
    AdjustLayout();

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(BrowsedSignal, MenuSignalEcho("MenuBar::BrowsedSignal"));
}

Pt MenuBar::MinUsableSize() const
{
    Pt retval;
    for (TextControl* menu_label : m_menu_labels) {
        Pt min_size = menu_label->MinUsableSize();
        retval.y = std::max(retval.y, min_size.y);
        retval.x += min_size.x;
    }
    return retval;
}

const MenuItem& MenuBar::AllMenus() const
{ return m_menu_data; }

bool MenuBar::ContainsMenu(const std::string& str) const
{
    bool retval = false;
    for (const MenuItem& submenu : m_menu_data.next_level) {
        if (submenu.label == str) {
            retval = true;
            break;
        }
    }
    return retval;
}

std::size_t MenuBar::NumMenus() const
{ return m_menu_data.next_level.size(); }

const MenuItem& MenuBar::GetMenu(const std::string& str) const
{
    for (const MenuItem& submenu : m_menu_data.next_level) {
        if (submenu.label == str)
            return submenu;
    }
    return m_menu_data.next_level.front();
}

const MenuItem& MenuBar::GetMenu(std::size_t n) const
{ return *(m_menu_data.next_level.begin() + n); }

Clr MenuBar::BorderColor() const        
{ return m_border_color; }

Clr MenuBar::InteriorColor() const
{ return m_int_color; }

Clr MenuBar::TextColor() const
{ return m_text_color; }

Clr MenuBar::HiliteColor() const
{ return m_hilite_color; }

Clr MenuBar::SelectedTextColor() const
{ return m_sel_text_color; }

void MenuBar::Render()
{
    Pt ul = UpperLeft();
    Pt lr = LowerRight();
    FlatRectangle(ul, lr, m_int_color, m_border_color, BORDER_THICKNESS);

    // paint caret, if any
    if (m_caret != INVALID_CARET) {
        Pt caret_ul = m_menu_labels[m_caret]->UpperLeft() + Pt(X(!m_caret ? BORDER_THICKNESS : 0), Y(BORDER_THICKNESS));
        Pt caret_lr = m_menu_labels[m_caret]->LowerRight() - Pt(X(m_caret == m_menu_labels.size() - 1 ? BORDER_THICKNESS : 0), Y(BORDER_THICKNESS));
        FlatRectangle(caret_ul, caret_lr, m_hilite_color, CLR_ZERO, 0);
    }
}

void MenuBar::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        for (std::size_t i = 0; i < m_menu_labels.size(); ++i) {
            if (m_menu_labels[i]->InWindow(pt)) {
                m_caret = INVALID_CARET;
                BrowsedSignal(0);
                // since a MenuBar is usually modeless, but becomes modal when a menu is opened, we do something kludgey here:
                // we launch a PopupMenu whenever a menu item is selected, then use the ID returned from it to find the
                // menu item that was chosen; we then emit a signal from that item
                if (m_menu_data.next_level[i].next_level.empty()) {
                    (*m_menu_data.next_level[i].SelectedIDSignal)(m_menu_data.next_level[i].item_ID);
                    (*m_menu_data.next_level[i].SelectedSignal)();
                } else {
                    MenuItem popup_data;
                    PopupMenu menu(m_menu_labels[i]->Left(), m_menu_labels[i]->Bottom(), m_font, m_menu_data.next_level[i], m_text_color, m_border_color, m_int_color);
                    menu.SetHiliteColor(m_hilite_color);
                    menu.SetSelectedTextColor(m_sel_text_color);
                    Connect(menu.BrowsedSignal, boost::ref(BrowsedSignal));
                    menu.Run();
                }
            }
        }
    }
}

void MenuBar::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_caret = INVALID_CARET;
        for (std::size_t i = 0; i < m_menu_data.next_level.size(); ++i) {
            if (m_menu_labels[i]->InWindow(pt)) {
                m_caret = i;
                break;
            }
        }
    }
}

void MenuBar::MouseLeave()
{ m_caret = INVALID_CARET; }

void MenuBar::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    AdjustLayout();
}

MenuItem& MenuBar::AllMenus()
{ return m_menu_data; }

MenuItem& MenuBar::GetMenu(const std::string& str)
{
    for (MenuItem& submenu : m_menu_data.next_level) {
        if (submenu.label == str)
            return submenu;
    }
    return m_menu_data.next_level.front();
}

MenuItem& MenuBar::GetMenu(int n)
{ return m_menu_data.next_level[n]; }

void MenuBar::AddMenu(const MenuItem& menu)
{
    m_menu_data.next_level.push_back(menu);
    AdjustLayout();
}

void MenuBar::SetBorderColor(Clr clr)
{ m_border_color = clr; }

void MenuBar::SetInteriorColor(Clr clr)
{ m_int_color = clr; }

void MenuBar::SetTextColor(Clr clr)
{ m_text_color = clr; }

void MenuBar::SetHiliteColor(Clr clr)
{ m_hilite_color = clr; }

void MenuBar::SetSelectedTextColor(Clr clr)
{ m_sel_text_color = clr; }

const boost::shared_ptr<Font>& MenuBar::GetFont() const
{ return m_font; }

const std::vector<TextControl*>& MenuBar::MenuLabels() const
{ return m_menu_labels; }

std::size_t MenuBar::Caret() const
{ return m_caret; }

void MenuBar::AdjustLayout(bool reset/* = false*/)
{
    if (reset) {
        DeleteChildren();
        m_menu_labels.clear();
    }

    // create any needed labels
    for (std::size_t i = m_menu_labels.size(); i < m_menu_data.next_level.size(); ++i) {
        m_menu_labels.push_back(GetStyleFactory()->NewTextControl(m_menu_data.next_level[i].label, m_font, m_text_color, FORMAT_NOWRAP));
        m_menu_labels.back()->Resize(Pt(m_menu_labels.back()->Width() + 2 * MENU_SEPARATION, m_font->Lineskip()));
        AttachChild(m_menu_labels.back());
    }

    // determine rows layout
    std::vector<int> menu_rows; // each element is the last + 1 index displayable on that row
    X space = Width();
    for (std::size_t i = 0; i < m_menu_labels.size(); ++i) {
        space -= m_menu_labels[i]->Width();
        if (space < 0) { // if this menu's text won't fit in the available space
            space = Width();
            // if moving this menu to the next row would leave an empty row, leave it here even though it won't quite fit
            if (!menu_rows.empty() && menu_rows.back() == static_cast<int>(i) - 1) {
                menu_rows.push_back(i + 1);
            } else {
                menu_rows.push_back(i);
                space -= m_menu_labels[i]->Width();
            }
        }
    }
    if (menu_rows.empty() || menu_rows.back() < static_cast<int>(m_menu_labels.size()))
        menu_rows.push_back(m_menu_labels.size());

    // place labels
    int label_i = 0;
    for (std::size_t row = 0; row < menu_rows.size(); ++row) {
        X x(0);
        for (; label_i < menu_rows[row]; ++label_i) {
            m_menu_labels[label_i]->MoveTo(Pt(x, static_cast<int>(row) * m_font->Lineskip()));
            x += m_menu_labels[label_i]->Width();
        }
    }

    // resize MenuBar if needed
    Y desired_ht = static_cast<int>(std::max(static_cast<std::size_t>(1), menu_rows.size())) * m_font->Lineskip();
    if (Height() != desired_ht)
        Resize(Pt(Width(), desired_ht));
}


////////////////////////////////////////////////
// GG::PopupMenu
////////////////////////////////////////////////
namespace {
    // distance to leave between edge of PopupMenu contents and the control's border
    const X HORIZONTAL_MARGIN(3);
}

const std::size_t PopupMenu::INVALID_CARET = std::numeric_limits<std::size_t>::max();

PopupMenu::PopupMenu(X x, Y y, const boost::shared_ptr<Font>& font, const MenuItem& m, Clr text_color/* = CLR_WHITE*/,
                     Clr border_color/* = CLR_BLACK*/, Clr interior_color/* = CLR_SHADOW*/, Clr hilite_color/* = CLR_GRAY*/) :
    Wnd(X0, Y0, GUI::GetGUI()->AppWidth() - 1, GUI::GetGUI()->AppHeight() - 1, INTERACTIVE | MODAL),
    m_font(font),
    m_border_color(border_color),
    m_int_color(interior_color),
    m_text_color(text_color),
    m_hilite_color(hilite_color),
    m_sel_text_color(text_color),
    m_menu_data(m),
    m_open_levels(),
    m_caret(1, INVALID_CARET),
    m_origin(x, y),
    m_item_selected(0)
{
    m_open_levels.resize(1);

    if (INSTRUMENT_ALL_SIGNALS)
        Connect(BrowsedSignal, MenuSignalEcho("PopupMenu::BrowsedSignal"));
}

Pt PopupMenu::ClientUpperLeft() const
{ return m_origin; }

int PopupMenu::MenuID() const
{ return (m_item_selected ? m_item_selected->item_ID : 0); }

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
            std::vector<boost::shared_ptr<Font::TextElement> > text_elements
                = m_font->ExpensiveParseFromTextToTextElements(str, fmt);
            std::vector<Font::LineData> lines = m_font->DetermineLines(str, fmt, X0, text_elements);
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
                    std::vector<boost::shared_ptr<Font::TextElement> > text_elements =
                        m_font->ExpensiveParseFromTextToTextElements(menu.next_level[j].label, fmt);
                    std::vector<Font::LineData> lines =
                        m_font->DetermineLines(menu.next_level[j].label, fmt, X0, text_elements);

                    m_font->RenderText(line_rect.ul, line_rect.lr, menu.next_level[j].label, fmt, lines);

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
    BrowsedSignal(0);
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
    int update_ID = 0;
    if (m_caret[0] != INVALID_CARET) {
        MenuItem* menu_ptr = &m_menu_data;
        for (std::size_t caret : m_caret) {
            if (caret == INVALID_CARET)
                break;
            menu_ptr = &menu_ptr->next_level[caret];
        }
        update_ID = menu_ptr->item_ID;
    }
    BrowsedSignal(update_ID);
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
    if (m_item_selected) {
        (*m_item_selected->SelectedIDSignal)(m_item_selected->item_ID);
        (*m_item_selected->SelectedSignal)();
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

const boost::shared_ptr<Font>& PopupMenu::GetFont() const
{ return m_font; }

const MenuItem& PopupMenu::MenuData() const
{ return m_menu_data; }

const std::vector<Rect>& PopupMenu::OpenLevels() const
{ return m_open_levels; }

const std::vector<std::size_t>& PopupMenu::Caret() const
{ return m_caret; }

const MenuItem* PopupMenu::ItemSelected() const
{ return m_item_selected; }
