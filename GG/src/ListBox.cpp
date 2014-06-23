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

#include <GG/ListBox.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/Scroll.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>

#include <cmath>
#include <numeric>


using namespace GG;

namespace {
    struct ListSignalEcho
    {
        ListSignalEcho(const ListBox& lb, const std::string& name) :
            m_LB(lb),
            m_name(name)
            {}
        void operator()()
            { std::cerr << "GG SIGNAL : " << m_name << "()\n"; }
        void operator()(const ListBox::SelectionSet& sels)
            {
                std::cerr << "GG SIGNAL : " << m_name
                          << "(sels=[ ";
                for (ListBox::SelectionSet::const_iterator it = sels.begin();
                     it != sels.end();
                     ++it) {
                    std::cerr << RowIndex(*it) << ' ';
                }
                std::cerr << "])\n";
            }
        void operator()(ListBox::const_iterator it)
            { std::cerr << "GG SIGNAL : " << m_name << "(row=" << RowIndex(it) << ")\n"; }
        void operator()(ListBox::const_iterator it, const Pt& pt)
            {
                std::cerr << "GG SIGNAL : " << m_name
                          << "(row=" << RowIndex(it) << " pt=" << pt << ")\n";
            }
        std::size_t RowIndex(ListBox::const_iterator it)
            { return std::distance(m_LB.begin(), it); }
        const ListBox& m_LB;
        std::string m_name;
    };

    const int SCROLL_WIDTH = 14;
    const X DEFAULT_ROW_WIDTH(50);
    const Y DEFAULT_ROW_HEIGHT(22);

    class RowSorter // used to sort rows by a certain column (which may contain some empty cells)
    {
    public:
        RowSorter(const boost::function<bool (const ListBox::Row&, const ListBox::Row&, std::size_t)>& cmp,
                  std::size_t col, bool invert) :
            m_cmp(cmp),
            m_sort_col(col),
            m_invert(invert)
        {}

        bool operator()(const ListBox::Row* l, const ListBox::Row* r)
        {
            bool retval = m_cmp(*l, *r, m_sort_col);
            return m_invert ? !retval : retval;
        }

    private:
        boost::function<bool (const ListBox::Row&, const ListBox::Row&, std::size_t)> m_cmp;
        std::size_t m_sort_col;
        bool m_invert;
    };

    bool LessThan(ListBox::iterator lhs, ListBox::iterator rhs, ListBox::iterator end)
    { return ListBox::RowPtrIteratorLess<std::list<ListBox::Row*> >::LessThan(lhs, rhs, end); }

    bool LessThanEqual(ListBox::iterator lhs, ListBox::iterator rhs, ListBox::iterator end)
    {
        return lhs == rhs ||
            ListBox::RowPtrIteratorLess<std::list<ListBox::Row*> >::LessThan(lhs, rhs, end);
    }

    void ResetIfEqual(ListBox::iterator& val, ListBox::iterator other, ListBox::iterator end)
    {
        if (val == other)
            val = end;
    }

    ListBox::Row* SafeDeref(const ListBox::iterator& it, const ListBox::iterator& end)
    { return it == end ? 0 : *it; }

    struct ScopedSet
    {
        ScopedSet(ListBox::iterator*& var, ListBox::iterator* value) :
            m_var(var = value)
        {}

        ~ScopedSet()
        { m_var = 0; }

        ListBox::iterator*& m_var;
    };

    Alignment AlignmentFromStyle(Flags<ListBoxStyle> style)
    {
        Alignment retval = ALIGN_NONE;
        if (style & LIST_LEFT)
            retval = ALIGN_LEFT;
        if (style & LIST_CENTER)
            retval = ALIGN_CENTER;
        if (style & LIST_RIGHT)
            retval = ALIGN_RIGHT;
        return retval;
    }
}

///////////////////////////////////////
// ListBoxStyle
///////////////////////////////////////
const ListBoxStyle GG::LIST_NONE            (0);
const ListBoxStyle GG::LIST_VCENTER         (1 << 0);
const ListBoxStyle GG::LIST_TOP             (1 << 1);
const ListBoxStyle GG::LIST_BOTTOM          (1 << 2);
const ListBoxStyle GG::LIST_CENTER          (1 << 3);
const ListBoxStyle GG::LIST_LEFT            (1 << 4);
const ListBoxStyle GG::LIST_RIGHT           (1 << 5);
const ListBoxStyle GG::LIST_NOSORT          (1 << 6);
const ListBoxStyle GG::LIST_SORTDESCENDING  (1 << 7);
const ListBoxStyle GG::LIST_NOSEL           (1 << 8);
const ListBoxStyle GG::LIST_SINGLESEL       (1 << 9);
const ListBoxStyle GG::LIST_QUICKSEL        (1 << 10);
const ListBoxStyle GG::LIST_USERDELETE      (1 << 11);
const ListBoxStyle GG::LIST_BROWSEUPDATES   (1 << 12);

GG_FLAGSPEC_IMPL(ListBoxStyle);

namespace {
    bool RegisterListBoxStyles()
    {
        FlagSpec<ListBoxStyle>& spec = FlagSpec<ListBoxStyle>::instance();
        spec.insert(LIST_NONE,          "LIST_NONE",            true);
        spec.insert(LIST_VCENTER,       "LIST_VCENTER",         true);
        spec.insert(LIST_TOP,           "LIST_TOP",             true);
        spec.insert(LIST_BOTTOM,        "LIST_BOTTOM",          true);
        spec.insert(LIST_CENTER,        "LIST_CENTER",          true);
        spec.insert(LIST_LEFT,          "LIST_LEFT",            true);
        spec.insert(LIST_RIGHT,         "LIST_RIGHT",           true);
        spec.insert(LIST_NOSORT,        "LIST_NOSORT",          true);
        spec.insert(LIST_SORTDESCENDING,"LIST_SORTDESCENDING",  true);
        spec.insert(LIST_NOSEL,         "LIST_NOSEL",           true);
        spec.insert(LIST_SINGLESEL,     "LIST_SINGLESEL",       true);
        spec.insert(LIST_QUICKSEL,      "LIST_QUICKSEL",        true);
        spec.insert(LIST_USERDELETE,    "LIST_USERDELETE",      true);
        spec.insert(LIST_BROWSEUPDATES, "LIST_BROWSEUPDATES",   true);
        return true;
    }
    bool dummy = RegisterListBoxStyles();
}


////////////////////////////////////////////////
// GG::ListBox::Row::DeferAdjustLayout
////////////////////////////////////////////////
ListBox::Row::DeferAdjustLayout::DeferAdjustLayout(Row* row) :
    m_row(row)
{ m_row->m_ignore_adjust_layout = true; }

ListBox::Row::DeferAdjustLayout::~DeferAdjustLayout()
{
    m_row->m_ignore_adjust_layout = false;
    m_row->AdjustLayout();
}


////////////////////////////////////////////////
// GG::ListBox::Row
////////////////////////////////////////////////
ListBox::Row::Row() :
    Control(X0, Y0, DEFAULT_ROW_WIDTH, DEFAULT_ROW_HEIGHT),
    m_row_alignment(ALIGN_VCENTER),
    m_margin(2),
    m_ignore_adjust_layout(false)
{}

ListBox::Row::Row(X w, Y h, const std::string& drag_drop_data_type,
                  Alignment align/* = ALIGN_VCENTER*/, unsigned int margin/* = 2*/) : 
    Control(X0, Y0, w, h),
    m_row_alignment(align),
    m_margin(margin),
    m_ignore_adjust_layout(false)
{ SetDragDropDataType(drag_drop_data_type); }

ListBox::Row::~Row()
{}

std::string ListBox::Row::SortKey(std::size_t column) const
{
    const TextControl* text_control = dynamic_cast<const TextControl*>(at(column));
    return text_control ? text_control->Text() : "";
}

std::size_t ListBox::Row::size() const
{ return m_cells.size(); }

bool ListBox::Row::empty() const
{ return m_cells.empty(); }

Control* ListBox::Row::operator[](std::size_t n) const
{ return m_cells[n]; }

Control* ListBox::Row::at(std::size_t n) const
{ return m_cells.at(n); }

Alignment ListBox::Row::RowAlignment() const
{ return m_row_alignment; }

Alignment ListBox::Row::ColAlignment(std::size_t n) const
{ return m_col_alignments[n]; }

X ListBox::Row::ColWidth(std::size_t n) const
{ return m_col_widths[n]; }

unsigned int ListBox::Row::Margin() const
{ return m_margin; }

Control* ListBox::Row::CreateControl(const std::string& str, const boost::shared_ptr<Font>& font, Clr color) const
{ return GetStyleFactory()->NewTextControl(X0, Y0, str, font, color); }

Control* ListBox::Row::CreateControl(const SubTexture& st) const
{ return new StaticGraphic(X0, Y0, st.Width(), st.Height(), st, GRAPHIC_SHRINKFIT); }

void ListBox::Row::Render()
{}

void ListBox::Row::push_back(Control* c)
{
    m_cells.push_back(c);
    m_col_widths.push_back(X(5));
    m_col_alignments.push_back(ALIGN_NONE);
    if (1 < m_cells.size())
        m_col_widths.back() = m_col_widths[m_cells.size() - 1];
    AdjustLayout();
}

void ListBox::Row::push_back(const std::string& str, const boost::shared_ptr<Font>& font,
                             Clr color/* = CLR_BLACK*/)
{ push_back(CreateControl(str, font, color)); }

void ListBox::Row::push_back(const std::string& str, const std::string& font_filename, unsigned int pts,
                             Clr color/* = CLR_BLACK*/)
{ push_back(CreateControl(str, GUI::GetGUI()->GetFont(font_filename, pts), color)); }

void ListBox::Row::push_back(const SubTexture& st)
{ push_back(CreateControl(st)); }

void ListBox::Row::clear()
{
    m_cells.clear();
    RemoveLayout();
    DeleteChildren();
}

void ListBox::Row::resize(std::size_t n)
{
    if (n == m_cells.size())
        return;

    std::size_t old_size = m_cells.size();
    for (std::size_t i = n; i < old_size; ++i) {
        delete m_cells[i];
    }
    m_cells.resize(n);
    m_col_widths.resize(n);
    m_col_alignments.resize(n);
    for (std::size_t i = old_size; i < n; ++i) {
        m_col_widths[i] = old_size ? m_col_widths[old_size - 1] : X(5); // assign new cells reasonable default widths
        m_col_alignments[i] = ALIGN_NONE;
    }
    AdjustLayout();
}

void ListBox::Row::SetCell(std::size_t n, Control* c)
{
    //assert(c != m_cells[n]);  // replaced with following test and return to avoid crashes
    if (c == m_cells[n])
        return;

    delete m_cells[n];
    m_cells[n] = c;
    AdjustLayout();
}

Control* ListBox::Row::RemoveCell(std::size_t n)
{
    Control* retval = m_cells[n];
    m_cells[n] = 0;
    AdjustLayout();
    return retval;
}

void ListBox::Row::SetRowAlignment(Alignment align)
{
    if (align == m_row_alignment)
        return;

    m_row_alignment = align;
    AdjustLayout();
}

void ListBox::Row::SetColAlignment(std::size_t n, Alignment align)
{
    if (align == m_col_alignments[n])
        return;

    m_col_alignments[n] = align;
    AdjustLayout();
}

void ListBox::Row::SetColWidth(std::size_t n, X width)
{
    if (width == m_col_widths[n])
        return;

    m_col_widths[n] = width;
    AdjustLayout();
}

void ListBox::Row::SetColAlignments(const std::vector<Alignment>& aligns)
{
    if (aligns == m_col_alignments)
        return;

    m_col_alignments = aligns;
    AdjustLayout();
}

void ListBox::Row::SetColWidths(const std::vector<X>& widths)
{
    if (widths == m_col_widths)
        return;

    m_col_widths = widths;
    AdjustLayout();
}

void ListBox::Row::SetMargin(unsigned int margin)
{
    if (margin == m_margin)
        return;

    m_margin = margin;
    AdjustLayout();
}

void ListBox::Row::AdjustLayout(bool adjust_for_push_back/* = false*/)
{
    if (m_ignore_adjust_layout)
        return;

    RemoveLayout();
    DetachChildren();

    bool nonempty_cell_found = false;
    for (std::size_t i = 0; i < m_cells.size(); ++i) {
        if (m_cells[i]) {
            nonempty_cell_found = true;
            break;
        }
    }

    if (!nonempty_cell_found)
        return;

    SetLayout(new Layout(X0, Y0, Width(), Height(), 1, m_cells.size(), m_margin, m_margin));
    Layout* layout = GetLayout();
    for (std::size_t i = 0; i < m_cells.size(); ++i) {
        layout->SetMinimumColumnWidth(i, m_col_widths[i]);
        if (m_cells[i])
            layout->Add(m_cells[i], 0, i, m_row_alignment | m_col_alignments[i]);
    }
}

////////////////////////////////////////////////
// GG::ListBox
////////////////////////////////////////////////
// static(s)
const unsigned int ListBox::BORDER_THICK = 2;

ListBox::ListBox() :
    Control(),
    m_rows(),
    m_vscroll(0),
    m_hscroll(0),
    m_vscroll_wheel_scroll_increment(0),
    m_hscroll_wheel_scroll_increment(0),
    m_caret(m_rows.end()),
    m_selections(RowPtrIteratorLess<std::list<Row*> >(&m_rows)),
    m_old_sel_row(m_rows.end()),
    m_old_sel_row_selected(false),
    m_old_rdown_row(m_rows.end()),
    m_lclick_row(m_rows.end()),
    m_rclick_row(m_rows.end()),
    m_last_row_browsed(m_rows.end()),
    m_first_row_shown(m_rows.end()),
    m_first_col_shown(0),
    m_cell_margin(2),
    m_style(LIST_NONE),
    m_header_row(0),
    m_keep_col_widths(false),
    m_clip_cells(false),
    m_sort_col(0),
    m_sort_cmp(DefaultRowCmp<Row>()),
    m_auto_scroll_during_drag_drops(true),
    m_auto_scroll_margin(8),
    m_auto_scrolling_up(false),
    m_auto_scrolling_down(false),
    m_auto_scrolling_left(false),
    m_auto_scrolling_right(false),
    m_auto_scroll_timer(250),
    m_iterator_being_erased(0)
{
    m_auto_scroll_timer.Stop();
    m_auto_scroll_timer.Connect(this);
}

ListBox::ListBox(X x, Y y, X w, Y h, Clr color, Clr interior/* = CLR_ZERO*/,
                 Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Control(x, y, w, h, flags),
    m_rows(),
    m_vscroll(0),
    m_hscroll(0),
    m_vscroll_wheel_scroll_increment(0),
    m_hscroll_wheel_scroll_increment(0),
    m_caret(m_rows.end()),
    m_selections(RowPtrIteratorLess<std::list<Row*> >(&m_rows)),
    m_old_sel_row(m_rows.end()),
    m_old_sel_row_selected(false),
    m_old_rdown_row(m_rows.end()),
    m_lclick_row(m_rows.end()),
    m_rclick_row(m_rows.end()),
    m_last_row_browsed(m_rows.end()),
    m_first_row_shown(m_rows.end()),
    m_first_col_shown(0),
    m_cell_margin(2),
    m_int_color(interior),
    m_hilite_color(CLR_SHADOW),
    m_style(LIST_NONE),
    m_header_row(new Row()),
    m_keep_col_widths(false),
    m_clip_cells(false),
    m_sort_col(0),
    m_sort_cmp(DefaultRowCmp<Row>()),
    m_auto_scroll_during_drag_drops(true),
    m_auto_scroll_margin(8),
    m_auto_scrolling_up(false),
    m_auto_scrolling_down(false),
    m_auto_scrolling_left(false),
    m_auto_scrolling_right(false),
    m_auto_scroll_timer(250),
    m_iterator_being_erased(0)
{
    Control::SetColor(color);
    ValidateStyle();
    SetChildClippingMode(ClipToClient);
    m_auto_scroll_timer.Stop();
    m_auto_scroll_timer.Connect(this);

    InstallEventFilter(this);

    if (INSTRUMENT_ALL_SIGNALS) {
        Connect(ClearedSignal,          ListSignalEcho(*this, "ListBox::ClearedSignal"));
        Connect(BeforeInsertSignal,     ListSignalEcho(*this, "ListBox::BeforeInsertSignal"));
        Connect(AfterInsertSignal,      ListSignalEcho(*this, "ListBox::AfterinsertSignal"));
        Connect(SelChangedSignal,       ListSignalEcho(*this, "ListBox::SelChangedSignal"));
        Connect(DroppedSignal,          ListSignalEcho(*this, "ListBox::DroppedSignal"));
        Connect(DropAcceptableSignal,   ListSignalEcho(*this, "ListBox::DropAcceptableSignal"));
        Connect(LeftClickedSignal,      ListSignalEcho(*this, "ListBox::LeftClickedSignal"));
        Connect(RightClickedSignal,     ListSignalEcho(*this, "ListBox::RightClickedSignal"));
        Connect(DoubleClickedSignal,    ListSignalEcho(*this, "ListBox::DoubleClickedSignal"));
        Connect(BeforeEraseSignal,      ListSignalEcho(*this, "ListBox::BeforeEraseSignal"));
        Connect(AfterEraseSignal,       ListSignalEcho(*this, "ListBox::AfterEraseSignal"));
        Connect(BrowsedSignal,          ListSignalEcho(*this, "ListBox::BrowsedSignal"));
    }
}

ListBox::~ListBox()
{ delete m_header_row; }

void ListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const Pt& pt) const
{
    for (std::map<const Wnd*, bool>::iterator it = first; it != last; ++it) {
        it->second = false;
        const Row* row = dynamic_cast<const Row*>(it->first);
        if (row &&
            (m_allowed_drop_types.find("") != m_allowed_drop_types.end() ||
             m_allowed_drop_types.find(row->DragDropDataType()) != m_allowed_drop_types.end())) {
            iterator insertion_it = RowUnderPt(pt);
            try {
                DropAcceptableSignal(insertion_it);
                it->second = true;
            } catch (const DontAcceptDrop&) {}
        }
    }
}

Pt ListBox::MinUsableSize() const
{
    return Pt(X(5 * SCROLL_WIDTH + 2 * BORDER_THICK),
              Y(5 * SCROLL_WIDTH + 2 * BORDER_THICK));
}

Pt ListBox::ClientUpperLeft() const
{
    return UpperLeft() +
        Pt(X(BORDER_THICK), static_cast<int>(BORDER_THICK) + (m_header_row->empty() ? Y0 : m_header_row->Height()));
}

Pt ListBox::ClientLowerRight() const
{ return LowerRight() - Pt(static_cast<int>(BORDER_THICK) + RightMargin(), static_cast<int>(BORDER_THICK) + BottomMargin()); }

bool ListBox::Empty() const
{ return m_rows.empty(); }

ListBox::const_iterator ListBox::begin() const
{ return m_rows.begin(); }

ListBox::const_iterator ListBox::end() const
{ return m_rows.end(); }

ListBox::const_reverse_iterator ListBox::rbegin() const
{ return m_rows.rbegin(); }

ListBox::const_reverse_iterator ListBox::rend() const
{ return m_rows.rend(); }

const ListBox::Row& ListBox::GetRow(std::size_t n) const
{
    assert(n < m_rows.size());
    return **boost::next(m_rows.begin(), n);
}

ListBox::iterator ListBox::Caret() const
{ return m_caret; }

const ListBox::SelectionSet& ListBox::Selections() const
{ return m_selections; }

bool ListBox::Selected(iterator it) const
{ return m_selections.find(it) != m_selections.end(); }

Clr ListBox::InteriorColor() const
{ return m_int_color; }

Clr ListBox::HiliteColor() const
{ return m_hilite_color; }

Flags<ListBoxStyle> ListBox::Style() const
{ return m_style; }

const ListBox::Row& ListBox::ColHeaders() const
{ return *m_header_row; }

ListBox::iterator ListBox::FirstRowShown() const
{ return m_first_row_shown; }

std::size_t ListBox::FirstColShown() const
{ return m_first_col_shown; }

ListBox::iterator ListBox::LastVisibleRow() const
{
    Y visible_pixels = ClientSize().y;
    Y acc(0);
    iterator it = m_first_row_shown;
    for (; it != m_rows.end(); ) {
        acc += (*it)->Height();
        iterator next_it = it;
        ++next_it;
        if (visible_pixels <= acc || next_it == m_rows.end())
            break;
        it = next_it;
    }
    return it;
}

std::size_t ListBox::LastVisibleCol() const
{
    X visible_pixels = ClientSize().x;
    X acc(0);
    std::size_t i = m_first_col_shown;
    for (; i < m_col_widths.size(); ++i) {
        acc += m_col_widths[i];
        if (visible_pixels <= acc)
            break;
    }
    if (m_col_widths.size() <= i)
        i = m_col_widths.size() - 1;
    return i;
}

std::size_t ListBox::NumRows() const
{ return m_rows.size(); }

std::size_t ListBox::NumCols() const
{ return m_col_widths.size(); }

bool ListBox::KeepColWidths() const
{ return m_keep_col_widths; }

std::size_t ListBox::SortCol() const
{ return m_sort_col; }

X ListBox::ColWidth(std::size_t n) const
{ return m_col_widths[n]; }

Alignment ListBox::ColAlignment(std::size_t n) const
{ return m_col_alignments[n]; }

Alignment ListBox::RowAlignment(iterator it) const
{ return (*it)->RowAlignment(); }

const std::set<std::string>& ListBox::AllowedDropTypes() const
{ return m_allowed_drop_types; }

bool ListBox::AutoScrollDuringDragDrops() const
{ return m_auto_scroll_during_drag_drops; }

unsigned int ListBox::AutoScrollMargin() const
{ return m_auto_scroll_margin; }

unsigned int ListBox::AutoScrollInterval() const
{ return m_auto_scroll_timer.Interval(); }

void ListBox::StartingChildDragDrop(const Wnd* wnd, const Pt& offset)
{
    if (m_selections.empty())
        return;
    if (m_rows.empty())
        return;

    iterator wnd_it = std::find(m_rows.begin(), m_rows.end(), wnd);
    //assert(wnd_it != m_rows.end());   // replaced with following test and return to avoid crashes
    if (wnd_it == m_rows.end())
        return;

    SelectionSet::iterator wnd_sel_it = m_selections.find(wnd_it);
    //assert(wnd_sel_it != m_selections.end()); // replaced with following test and return to avoid crashes
    if (wnd_sel_it == m_selections.end())
        return;

    Y vertical_offset = offset.y;

    for (SelectionSet::iterator sel_it = m_selections.begin(); sel_it != wnd_sel_it; ++sel_it) {
        vertical_offset += (**sel_it)->Height();
    }
    for (SelectionSet::iterator sel_it = m_selections.begin(); sel_it != m_selections.end(); ++sel_it) {
        Wnd* row_wnd = **sel_it;
        if (row_wnd != wnd) {
            GUI::GetGUI()->RegisterDragDropWnd(row_wnd, Pt(offset.x, vertical_offset), this);
            vertical_offset -= row_wnd->Height();
        } else {
            vertical_offset -= wnd->Height();
        }
    }
}

void ListBox::AcceptDrops(const std::vector<Wnd*>& wnds, const Pt& pt)
{
    // TODO: Pull the call to RowUnderPt() out and reuse the value in each loop iteration.
    for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
        Row* row = boost::polymorphic_downcast<Row*>(*it);
        iterator insertion_it = RowUnderPt(pt);
        Insert(row, insertion_it, true, true);
    }
}

void ListBox::ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination)
{
    if (!MatchesOrContains(this, destination)) {
        for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
            Row* row = boost::polymorphic_downcast<Row*>(*it);
            iterator row_it = std::find(m_rows.begin(), m_rows.end(), row);
            //assert(row_it != m_rows.end());   // replaced with following test and continue to avoid crashes
            if (row_it == m_rows.end())
                continue;

            Erase(row_it, false, true);
        }
    }
}

void ListBox::Render()
{
    // draw beveled rectangle around client area
    Pt ul = UpperLeft(), lr = LowerRight();
    Pt cl_ul = ClientUpperLeft(), cl_lr = ClientLowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    Clr hilite_color_to_use = Disabled() ? DisabledColor(m_hilite_color) : m_hilite_color;

    BeveledRectangle(ul, lr, int_color_to_use, color_to_use, false, BORDER_THICK);

    if (m_first_row_shown == m_rows.end())
        return;

    iterator last_visible_row = LastVisibleRow();

    BeginClipping();

    // draw selection hiliting

    // Note that inside the for loop below, prev_sel is guaranteed to be valid
    // (i.e. != m_rows.end()), since a non-empty m_selections implies a
    // non-empty m_rows, and a non-empty m_rows implies a valid
    // m_first_row_shown.
    iterator prev_sel = m_first_row_shown;
    Y top(0);
    Y bottom = (*m_first_row_shown)->Height();
    for (SelectionSet::iterator sel_it = m_selections.begin(); sel_it != m_selections.end(); ++sel_it) {
        iterator curr_sel = *sel_it;
        if (LessThanEqual(m_first_row_shown, curr_sel, m_rows.end()) &&
            LessThanEqual(curr_sel, last_visible_row, m_rows.end())) {
            // No need to look for the current selection's top, if it is the
            // same as the bottom of the last iteration.
            if (boost::next(prev_sel) == curr_sel) {
                top = bottom;
            } else {
                for (iterator it = prev_sel; it != curr_sel; ++it) {
                    top += (*it)->Height();
                }
            }
            bottom = top + (*curr_sel)->Height();
            if (cl_lr.y < bottom)
                bottom = cl_lr.y;
            FlatRectangle(Pt(cl_ul.x, cl_ul.y + top), Pt(cl_lr.x, cl_ul.y + bottom),
                          hilite_color_to_use, CLR_ZERO, 0);
            prev_sel = curr_sel;
        }
    }

    // draw caret
    if (m_caret != m_rows.end() &&
        LessThanEqual(m_first_row_shown, m_caret, m_rows.end()) &&
        LessThanEqual(m_caret, last_visible_row, m_rows.end()) &&
        MatchesOrContains(this, GUI::GetGUI()->FocusWnd())) {
        Pt row_ul = (*m_caret)->UpperLeft();
        Pt row_lr = (*m_caret)->LowerRight();
        row_lr.x = ClientLowerRight().x;
        FlatRectangle(row_ul, row_lr, CLR_ZERO, CLR_SHADOW, 2);
    }

    EndClipping();

    // HACK! This gets around the issue of how to render headers and scrolls,
    // which do not fall within the client area.
    if (!m_header_row->empty()) {
        Rect header_area(Pt(ul.x + static_cast<int>(BORDER_THICK), m_header_row->Top()),
                         Pt(lr.x - static_cast<int>(BORDER_THICK), m_header_row->Bottom()));
        BeginScissorClipping(header_area.ul, header_area.lr);
        GUI::GetGUI()->RenderWindow(m_header_row);
        EndScissorClipping();
    }
    if (m_vscroll)
        GUI::GetGUI()->RenderWindow(m_vscroll);
    if (m_hscroll)
        GUI::GetGUI()->RenderWindow(m_hscroll);

    // ensure that data in occluded cells is not rendered
    bool hide = true;
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        if (it == m_first_row_shown)
            hide = false;

        if (hide)
            (*it)->Hide();
        else
            (*it)->Show();

        if (it == last_visible_row)
            hide = true;
    }
}

void ListBox::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    if (!m_header_row->empty())
        NormalizeRow(m_header_row);
    AdjustScrolls(true);
}

void ListBox::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    if (m_vscroll)
        m_vscroll->Disable(b);
    if (m_hscroll)
        m_hscroll->Disable(b);
}

void ListBox::SetColor(Clr c)
{
    Control::SetColor(c);
    if (m_vscroll)
        m_vscroll->SetColor(c);
    if (m_hscroll)
        m_hscroll->SetColor(c);
}

ListBox::iterator ListBox::Insert(Row* row, iterator it, bool signal/* = true*/)
{ return Insert(row, it, false, signal); }

ListBox::iterator ListBox::Insert(Row* row, bool signal/* = true*/)
{ return Insert(row, m_rows.end(), false, signal); }

void ListBox::Insert(const std::vector<Row*>& rows, iterator it, bool signal/* = true*/)
{ Insert(rows, it, false, signal); }

void ListBox::Insert(const std::vector<Row*>& rows, bool signal/* = true*/)
{ Insert(rows, m_rows.end(), false, signal); }

ListBox::Row* ListBox::Erase(iterator it, bool signal/* = false*/)
{ return Erase(it, false, signal); }

void ListBox::Clear()
{
    m_rows.clear();
    m_caret = m_rows.end();
    DetachChild(m_header_row);
    DeleteChildren();
    AttachChild(m_header_row);
    m_vscroll = 0;
    m_hscroll = 0;
    m_first_row_shown = m_rows.end();
    m_first_col_shown = 0;
    m_selections.clear();
    m_old_sel_row = m_rows.end();
    m_lclick_row = m_rows.end();

    if (!m_keep_col_widths) { // remove column widths and alignments, if needed
        m_col_widths.clear();
        m_col_alignments.clear();
    }

    AdjustScrolls(false);

    if (m_iterator_being_erased)
        *m_iterator_being_erased = m_rows.end();

    ClearedSignal();
}

void ListBox::SelectRow(iterator it, bool signal/* = false*/)
{
    if (m_style & LIST_NOSEL)
        return;
    if (it == m_rows.end())
        return;
    if (m_selections.find(it) != m_selections.end())
        return;

    SelectionSet previous_selections = m_selections;

    if (m_style & LIST_SINGLESEL)
        m_selections.clear();

    m_selections.insert(it);

    if (signal && previous_selections != m_selections)
        SelChangedSignal(m_selections);
}

void ListBox::DeselectRow(iterator it, bool signal/* = false*/)
{
    SelectionSet previous_selections = m_selections;

    if (m_selections.find(it) != m_selections.end())
        m_selections.erase(it);

    if (signal && previous_selections != m_selections)
        SelChangedSignal(m_selections);
}

void ListBox::SelectAll(bool signal/* = false*/)
{
    if (m_style & LIST_NOSEL)
        return;

    SelectionSet previous_selections = m_selections;

    if (m_style & LIST_SINGLESEL) {
        if (m_selections.empty() && !m_rows.empty()) {
            m_selections.insert(m_rows.begin());
        }
    } else {
        if (m_selections.size() != m_rows.size()) {
            m_selections.clear();
            for (iterator it = m_rows.begin(); it != m_rows.end(); ++it)
                m_selections.insert(it);
        }
    }

    if (signal && previous_selections != m_selections)
        SelChangedSignal(m_selections);
}

void ListBox::DeselectAll(bool signal/* = false*/)
{
    SelectionSet previous_selections = m_selections;

    if (!m_selections.empty()) {
        m_selections.clear();
        m_caret = m_rows.end();
    }

    if (signal && previous_selections != m_selections)
        SelChangedSignal(m_selections);
}

ListBox::iterator ListBox::begin()
{ return m_rows.begin(); }

ListBox::iterator ListBox::end()
{ return m_rows.end(); }

ListBox::reverse_iterator ListBox::rbegin()
{ return m_rows.rbegin(); }

ListBox::reverse_iterator ListBox::rend()
{ return m_rows.rend(); }

ListBox::Row& ListBox::GetRow(std::size_t n)
{
    assert(n < m_rows.size());
    return **boost::next(m_rows.begin(), n);
}

void ListBox::SetSelections(const SelectionSet& s, bool signal/* = false*/)
{
    if (m_style & LIST_NOSEL)
        return;

    SelectionSet previous_selections = m_selections;

    m_selections = s;

    if (signal && previous_selections != m_selections)
        SelChangedSignal(m_selections);
}

void ListBox::SetCaret(iterator it)
{ m_caret = it; }

void ListBox::BringRowIntoView(iterator it)
{
    if (it != m_rows.end()) {
        if (LessThan(it, m_first_row_shown, m_rows.end())) {
            m_first_row_shown = it;
        } else if (LessThanEqual(LastVisibleRow(), it, m_rows.end())) {
            // Find the row that preceeds the target row by about ClientSize().y
            // pixels, and make it the first row shown.
            m_first_row_shown = FirstRowShownWhenBottomIs(it, ClientHeight());
        }
        if (m_vscroll) {
            Y acc(0);
            for (iterator it2 = m_rows.begin(); it2 != m_first_row_shown; ++it2)
                acc += (*it)->Height();
            m_vscroll->ScrollTo(Value(acc));
            SignalScroll(*m_vscroll, true);
        }
    }
}

void ListBox::SetFirstRowShown(iterator it)
{
    if (it != m_rows.end()) {
        m_first_row_shown = it;
        if (m_vscroll) {
            Y acc(0);
            for (iterator it2 = m_rows.begin(); it2 != m_first_row_shown; ++it2)
                acc += (*it)->Height();
            m_vscroll->ScrollTo(Value(acc));
            SignalScroll(*m_vscroll, true);
        } else {
            std::size_t row_num = std::distance(m_rows.begin(), m_first_row_shown);
            VScrolled(row_num, 0, 0, 0);
        }
    }
}

void ListBox::SetVScrollWheelIncrement(unsigned int increment)
{
    m_vscroll_wheel_scroll_increment = increment;
    AdjustScrolls(false);
}

void ListBox::SetHScrollWheelIncrement(unsigned int increment)
{
    m_hscroll_wheel_scroll_increment = increment;
    AdjustScrolls(false);
}

void ListBox::SetInteriorColor(Clr c)
{ m_int_color = c; }

void ListBox::SetHiliteColor(Clr c)
{ m_hilite_color = c; }

void ListBox::SetStyle(Flags<ListBoxStyle> s)
{
    Flags<ListBoxStyle> old_style = m_style;
    m_style = s;
    ValidateStyle();

    // if we're going from an unsorted style to a sorted one, do the sorting now
    if (old_style & LIST_NOSORT) {
        if (!(m_style & LIST_NOSORT))
            Resort();
    // if we're changing the sorting order of a sorted list, reverse the contents
    } else if (static_cast<bool>(old_style & LIST_SORTDESCENDING) !=
               static_cast<bool>(m_style & LIST_SORTDESCENDING)) {
        Resort();
    }
}

void ListBox::SetColHeaders(Row* r)
{
    Y client_height = ClientHeight();
    delete m_header_row;
    if (r) {
        m_header_row = r;
        // if this column header is being added to an empty listbox, the listbox takes on some of the
        // attributes of the header, similar to the insertion of a row into an empty listbox; see Insert()
        if (m_rows.empty() && m_col_widths.empty()) {
            m_col_widths.resize(m_header_row->size(),
                                (ClientSize().x - SCROLL_WIDTH) / static_cast<int>(m_header_row->size()));
            // put the remainder in the last column, so the total width == ClientSize().x - SCROLL_WIDTH
            m_col_widths.back() += (ClientSize().x - SCROLL_WIDTH) % static_cast<int>(m_header_row->size());
            m_col_alignments.resize(m_header_row->size(), AlignmentFromStyle(m_style));
        }
        NormalizeRow(m_header_row);
        m_header_row->MoveTo(Pt(X0, -m_header_row->Height()));
        AttachChild(m_header_row);
    } else {
        m_header_row = new Row();
    }
    if (client_height != ClientHeight())
        AdjustScrolls(true);
}

void ListBox::RemoveColHeaders()
{ SetColHeaders(0); }

void ListBox::SetNumCols(std::size_t n)
{
    if (m_col_widths.size()) {
        m_col_widths.resize(n);
        m_col_alignments.resize(n);
    } else {
        m_col_widths.resize(n, ClientSize().x / static_cast<int>(n));
        m_col_widths.back() += ClientSize().x % static_cast<int>(n);
        Alignment alignment = ALIGN_NONE;
        if (m_style & LIST_LEFT)
            alignment = ALIGN_LEFT;
        if (m_style & LIST_CENTER)
            alignment = ALIGN_CENTER;
        if (m_style & LIST_RIGHT)
            alignment = ALIGN_RIGHT;
        m_col_alignments.resize(n, alignment);
    }
    if (n <= m_sort_col)
        m_sort_col = 0;
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        NormalizeRow(*it);
    }
    AdjustScrolls(false);
}

void ListBox::SetColWidth(std::size_t n, X w)
{
    m_col_widths[n] = w;
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        (*it)->SetColWidth(n, w);
    }
    AdjustScrolls(false);
}

void ListBox::SetSortCol(std::size_t n)
{
    bool needs_resort = m_sort_col != n && !(m_style & LIST_NOSORT);
    m_sort_col = n;
    if (needs_resort)
        Resort();
}

void ListBox::SetSortCmp(const boost::function<bool (const Row&, const Row&, std::size_t)>& sort_cmp)
{
    m_sort_cmp = sort_cmp;
    if (!(m_style & LIST_NOSORT))
        Resort();
}

void ListBox::LockColWidths()
{ m_keep_col_widths = true; }

void ListBox::UnLockColWidths()
{ m_keep_col_widths = false; }

void ListBox::SetColAlignment(std::size_t n, Alignment align)
{
    m_col_alignments[n] = align;
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        (*it)->SetColAlignment(n, align);
    }
}

void ListBox::SetRowAlignment(iterator it, Alignment align)
{ (*it)->SetRowAlignment(align); }

void ListBox::AllowDropType(const std::string& str)
{ m_allowed_drop_types.insert(str); }

void ListBox::DisallowDropType(const std::string& str)
{ m_allowed_drop_types.erase(str); }

void ListBox::AutoScrollDuringDragDrops(bool auto_scroll)
{ m_auto_scroll_during_drag_drops = auto_scroll; }

void ListBox::SetAutoScrollMargin(unsigned int margin)
{ m_auto_scroll_margin = margin; }

void ListBox::SetAutoScrollInterval(unsigned int interval)
{ m_auto_scroll_timer.SetInterval(interval); }

X ListBox::RightMargin() const
{ return X(m_vscroll ? SCROLL_WIDTH : 0); }

Y ListBox::BottomMargin() const
{ return Y(m_hscroll ? SCROLL_WIDTH : 0); }

unsigned int ListBox::CellMargin() const
{ return m_cell_margin; }

ListBox::iterator ListBox::RowUnderPt(const Pt& pt) const
{
    iterator retval = m_first_row_shown;
    Y acc = ClientUpperLeft().y;
    for ( ; retval != m_rows.end(); ++retval) {
        acc += (*retval)->Height();
        if (pt.y <= acc)
            break;
    }
    return retval;
}

ListBox::iterator ListBox::OldSelRow() const
{ return m_old_sel_row; }

ListBox::iterator ListBox::OldRDownRow() const
{ return m_old_rdown_row; }

ListBox::iterator ListBox::LClickRow() const
{ return m_lclick_row; }

ListBox::iterator ListBox::RClickRow() const
{ return m_rclick_row; }

bool ListBox::AutoScrollingUp() const
{ return m_auto_scrolling_up; }

bool ListBox::AutoScrollingDown() const
{ return m_auto_scrolling_down; }

bool ListBox::AutoScrollingLeft() const
{ return m_auto_scrolling_left; }

bool ListBox::AutoScrollingRight() const
{ return m_auto_scrolling_right; }

void ListBox::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        bool bring_caret_into_view = true;
        switch (key) {
        case GGK_SPACE: // space bar (selects item under caret like a mouse click)
            if (m_caret != m_rows.end()) {
                m_old_sel_row_selected = m_selections.find(m_caret) != m_selections.end();
                ClickAtRow(m_caret, mod_keys);
            }
            break;
        case GGK_DELETE: // delete key
            if (m_style & LIST_USERDELETE) {
                if (m_style & LIST_NOSEL) {
                    if (m_caret != m_rows.end())
                        delete Erase(m_caret, false, true);
                } else {
                    for (SelectionSet::iterator it = m_selections.begin(); it != m_selections.end(); ++it) {
                        delete Erase(*it, false, true);
                    }
                    m_selections.clear();
                }
            } else {
                // Pass delete on if we ignored it
                Control::KeyPress(key, key_code_point, mod_keys);
            }
            break;

        // vertical scrolling keys
        case GGK_UP: // arrow-up (not numpad arrow)
            if (m_caret != m_rows.end() && m_caret != m_rows.begin())
                --m_caret;
            break;
        case GGK_DOWN: // arrow-down (not numpad arrow)
            if (m_caret != m_rows.end() && m_caret != --m_rows.end())
                ++m_caret;
            break;
        case GGK_PAGEUP: // page up key (not numpad key)
            if (m_caret != m_rows.end()) {
                Y space = ClientSize().y;
                while (m_caret != m_rows.begin() && 0 < (space -= (*boost::prior(m_caret))->Height())) {
                    --m_caret;
                }
            }
            break;
        case GGK_PAGEDOWN: // page down key (not numpad key)
            if (m_caret != m_rows.end()) {
                Y space = ClientSize().y;
                while (m_caret != --m_rows.end() && 0 < (space -= (*m_caret)->Height())) {
                    ++m_caret;
                }
            }
            break;
        case GGK_HOME: // home key (not numpad)
            if (m_caret != m_rows.end())
                m_caret = m_rows.begin();
            break;
        case GGK_END: // end key (not numpad)
            if (m_caret != m_rows.end())
                m_caret = --m_rows.end();
            break;

        // horizontal scrolling keys
        case GGK_LEFT: // left key (not numpad key)
            if (m_first_col_shown) {
                --m_first_col_shown;
                m_hscroll->ScrollTo(
                    Value(std::accumulate(m_col_widths.begin(), m_col_widths.begin() + m_first_col_shown, X0)));
                SignalScroll(*m_hscroll, true);
            }
            break;
        case GGK_RIGHT:{ // right key (not numpad)
            std::size_t last_fully_visible_col = LastVisibleCol();
            if (std::accumulate(m_col_widths.begin(), m_col_widths.begin() + last_fully_visible_col, X0) >
                ClientSize().x) {
                --last_fully_visible_col;
            }
            if (last_fully_visible_col < m_col_widths.size() - 1) {
                ++m_first_col_shown;
                m_hscroll->ScrollTo(
                    Value(std::accumulate(m_col_widths.begin(), m_col_widths.begin() + m_first_col_shown, X0)));
                SignalScroll(*m_hscroll, true);
            }
            break;}

        // any other key gets passed along to the parent
        default:
            Control::KeyPress(key, key_code_point, mod_keys);
            bring_caret_into_view = false;
        }

        if (bring_caret_into_view &&
            key != GGK_SPACE && key != GGK_DELETE && key != GGK_LEFT && key != GGK_RIGHT) {
            BringCaretIntoView();
        }
    } else {
        Control::KeyPress(key, key_code_point, mod_keys);
    }
}

void ListBox::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    if (Disabled() || !m_vscroll)
        return;
    m_vscroll->ScrollLineIncr(-move);
    SignalScroll(*m_vscroll, true);
}

void ListBox::DragDropEnter(const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys)
{
    ResetAutoScrollVars();
    DragDropHere(pt, drag_drop_wnds, mod_keys);
}

void ListBox::DragDropHere(const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys)
{
    if (!m_rows.empty() && m_auto_scroll_during_drag_drops && InClient(pt)) {
        const Pt MARGIN_OFFSET = Pt(X(m_auto_scroll_margin), Y(m_auto_scroll_margin));
        Rect client_no_scroll_hole(ClientUpperLeft() + MARGIN_OFFSET, ClientLowerRight() - MARGIN_OFFSET);
        m_auto_scrolling_up = pt.y < client_no_scroll_hole.ul.y;
        m_auto_scrolling_down = client_no_scroll_hole.lr.y < pt.y;
        m_auto_scrolling_left = pt.x < client_no_scroll_hole.ul.x;
        m_auto_scrolling_right = client_no_scroll_hole.lr.x < pt.x;
        if (m_auto_scrolling_up || m_auto_scrolling_down || m_auto_scrolling_left || m_auto_scrolling_right) {
            bool acceptible_drop = false;
            for (std::map<Wnd*, GG::Pt>::const_iterator it = drag_drop_wnds.begin(); it != drag_drop_wnds.end(); ++it) {
                if (m_allowed_drop_types.find("") != m_allowed_drop_types.end() ||
                    m_allowed_drop_types.find(it->first->DragDropDataType()) != m_allowed_drop_types.end()) {
                    acceptible_drop = true;
                    break;
                }
            }
            if (acceptible_drop) {
                if (!m_auto_scroll_timer.Running()) {
                    m_auto_scroll_timer.Reset(GUI::GetGUI()->Ticks());
                    m_auto_scroll_timer.Start();
                }
            } else {
                DragDropLeave();
            }
        }
    }
}

void ListBox::DragDropLeave()
{ ResetAutoScrollVars(); }

void ListBox::TimerFiring(unsigned int ticks, Timer* timer)
{
    if (timer == &m_auto_scroll_timer && !m_rows.empty()) {
        if (m_vscroll) {
            if (m_auto_scrolling_up &&
                m_first_row_shown != m_rows.end() &&
                m_first_row_shown != m_rows.begin()) {
                m_vscroll->ScrollTo(m_vscroll->PosnRange().first -
                                    Value((*boost::prior(m_first_row_shown))->Height()));
                SignalScroll(*m_vscroll, true);
            }
            if (m_auto_scrolling_down) {
                iterator last_visible_row = LastVisibleRow();
                if (last_visible_row != m_rows.end() &&
                    (last_visible_row != --m_rows.end() ||
                     ClientLowerRight().y < (*last_visible_row)->Bottom()))
                {
                    m_vscroll->ScrollTo(m_vscroll->PosnRange().first +
                                        Value((*m_first_row_shown)->Height()));
                    SignalScroll(*m_vscroll, true);
                }
            }
        }
        if (m_hscroll) {
            if (m_auto_scrolling_left && 0 < m_first_col_shown) {
                m_hscroll->ScrollTo(m_hscroll->PosnRange().first -
                                    Value(m_col_widths[m_first_col_shown - 1]));
                SignalScroll(*m_hscroll, true);
            }
            if (m_auto_scrolling_right) {
                std::size_t last_visible_col = LastVisibleCol();
                if (last_visible_col < m_col_widths.size() - 1 ||
                    ClientLowerRight().x < m_rows.front()->Right()) {
                    m_hscroll->ScrollTo(m_hscroll->PosnRange().first +
                                        Value(m_col_widths[m_first_col_shown]));
                    SignalScroll(*m_hscroll, true);
                }
            }
        }
    }
}

bool ListBox::EventFilter(Wnd* w, const WndEvent& event)
{
    assert(w == this || dynamic_cast<Row*>(w));

    if (Disabled())
        return true;

    Pt pt = event.Point();
    Flags<ModKey> mod_keys = event.ModKeys();

    switch (event.Type()) {
    case WndEvent::LButtonDown: {
        m_old_sel_row = RowUnderPt(pt);
        if (!InClient(pt)) {
            m_old_sel_row = m_rows.end();
        } else if (m_old_sel_row != m_rows.end()) {
            m_old_sel_row_selected = m_selections.find(m_old_sel_row) != m_selections.end();
            if (!(m_style & LIST_NOSEL) && !m_old_sel_row_selected)
                ClickAtRow(m_old_sel_row, mod_keys);
        }
        break;
    }

    case WndEvent::LButtonUp: {
        m_old_sel_row = m_rows.end();
        break;
    }

    case WndEvent::LClick: {
        if (m_old_sel_row != m_rows.end() && InClient(pt)) {
            iterator sel_row = RowUnderPt(pt);
            if (sel_row == m_old_sel_row) {
                if (m_style & LIST_NOSEL)
                    m_caret = sel_row;
                else
                    ClickAtRow(sel_row, mod_keys);
                m_lclick_row = sel_row;
                LeftClickedSignal(sel_row, pt);
            }
        }
        break;
    }

    case WndEvent::LDoubleClick: {
        iterator row = RowUnderPt(pt);
        if (row != m_rows.end() && row == m_lclick_row && InClient(pt)) {
            DoubleClickedSignal(row);
            m_old_sel_row = m_rows.end();
        } else {
            LClick(pt, mod_keys);
        }
        break;
    }

    case WndEvent::RButtonDown: {
        iterator row = RowUnderPt(pt);
        if (row != m_rows.end() && InClient(pt))
            m_old_rdown_row = row;
        else
            m_old_rdown_row = m_rows.end();
        break;
    }

    case WndEvent::RClick: {
        iterator row = RowUnderPt(pt);
        if (row != m_rows.end() && row == m_old_rdown_row && InClient(pt)) {
            m_rclick_row = row;
            RightClickedSignal(row, pt);
        }
        m_old_rdown_row = m_rows.end();
        break;
    }

    case WndEvent::MouseEnter: {
        if (m_style & LIST_BROWSEUPDATES) {
            iterator sel_row = RowUnderPt(pt);
            if (m_last_row_browsed != sel_row)
                BrowsedSignal(m_last_row_browsed = sel_row);
        }
        break;
    }

    case WndEvent::MouseHere:
        break;

    case WndEvent::MouseLeave: {
        if (m_style & LIST_BROWSEUPDATES) {
            if (m_last_row_browsed != m_rows.end())
                BrowsedSignal(m_last_row_browsed = m_rows.end());
        }
        break;
    }

    case WndEvent::GainingFocus: {
        if (w == this)
            return false;
        GUI::GetGUI()->SetFocusWnd(this);
        break;
    }

    case WndEvent::MouseWheel:
        return false;

    case WndEvent::DragDropEnter:
    case WndEvent::DragDropHere:
    case WndEvent::DragDropLeave:
        if (w == this)
            return false;
        HandleEvent(event);
        break;

    case WndEvent::KeyPress:
    case WndEvent::KeyRelease:
    case WndEvent::TimerFiring:
        return false;

    default:
        break;
    }

    return true;
}

ListBox::iterator ListBox::Insert(Row* row, iterator it, bool dropped, bool signal)
{
    // Track the originating row in case this is an intra-ListBox
    // drag-and-drop.
    iterator original_dropped_position = m_rows.end();
    if (dropped)
        original_dropped_position = std::find(m_rows.begin(), m_rows.end(), row);

    iterator retval = it;

    // The first row inserted into an empty list box defines the number of
    // columns, and initializes the column widths and alignments.
    if (m_rows.empty() && (m_col_widths.empty() || !m_keep_col_widths)) {
        const GG::X WIDTH = ClientSize().x - SCROLL_WIDTH;

        m_col_widths.resize(row->size());
        m_col_alignments.resize(row->size());
        GG::X total = GG::X0;
        for (std::size_t i = 0; i < row->size(); ++i) {
            // use the column width from the Row
            total += row->ColWidth(i);

            // use the column alignment from the Row, if it has been set;
            // otherwise, use the one dictated by the ListBoxStyle flags
            Alignment a = row->ColAlignment(i);
            if (a == ALIGN_NONE)
                a = AlignmentFromStyle(m_style);
            m_col_alignments[i] = a;
        }

        const GG::X_d SCALE_FACTOR = 1.0 * WIDTH / total;

        total = GG::X0;
        for (std::size_t i = 0; i < row->size(); ++i) {
            total += (m_col_widths[i] = row->ColWidth(i) * SCALE_FACTOR);
        }
        m_col_widths.back() += total - WIDTH;

        if (!m_header_row->empty())
            NormalizeRow(m_header_row);
    }

    row->InstallEventFilter(this);
    NormalizeRow(row);

    if (signal)
        BeforeInsertSignal(it);

    Pt insertion_pt;
    if (m_rows.empty()) {
        m_rows.push_back(row);
        retval = m_rows.begin();
    } else {
        if (!(m_style & LIST_NOSORT)) {
            retval = m_rows.begin();
            RowSorter cmp(m_sort_cmp, m_sort_col, m_style & LIST_SORTDESCENDING);
            while (retval != m_rows.end() && !cmp(row, *retval)) {
                ++retval;
            }
        }
        Y y = retval == m_rows.end() ?
            m_rows.back()->RelativeLowerRight().y : (*retval)->RelativeUpperLeft().y;
        insertion_pt = Pt(X0, y);
        retval = m_rows.insert(retval, row);
    }

    Y row_height = row->Height();
    for (iterator it2 = boost::next(retval); it2 != m_rows.end(); ++it2) {
        (*it2)->OffsetMove(Pt(X0, row_height));
    }

    AttachChild(row);
    row->MoveTo(insertion_pt);

    if (m_first_row_shown == m_rows.end())
        m_first_row_shown = m_rows.begin();

    AdjustScrolls(false);

    if (dropped) {
        // TODO: Can these be inverted without breaking anything?  It would be
        // semantically clearer if they were.
        DroppedSignal(retval);
        if (original_dropped_position != m_rows.end())
            Erase(original_dropped_position, true, false);
    }

    if (signal)
        AfterInsertSignal(it);

    return retval;
}

void ListBox::Insert(const std::vector<Row*>& rows, iterator it, bool dropped, bool signal)
{
    if (rows.empty())
        return;

    if (signal || dropped) {
        // need to signal or handle dropping for each row, so add individually
        for (std::vector<Row*>::const_iterator row_it = rows.begin();
             row_it != rows.end(); ++row_it)
        { Insert(*row_it, it, dropped, signal); }
        return;
    }

    // don't need to signal or handle dropping issues, so can add rows at once
    // without externally handling after each


    // The first row inserted into an empty list box defines the number of
    // columns, and initializes the column widths and alignments.
    if (m_col_widths.empty() || !m_keep_col_widths) {
        Row* row = *rows.begin();

        const GG::X WIDTH = ClientSize().x - SCROLL_WIDTH;

        m_col_widths.resize(row->size());
        m_col_alignments.resize(row->size());
        GG::X total = GG::X0;
        for (std::size_t i = 0; i < row->size(); ++i) {
            // use the column width from the Row
            total += row->ColWidth(i);

            // use the column alignment from the Row, if it has been set;
            // otherwise, use the one dictated by the ListBoxStyle flags
            Alignment a = row->ColAlignment(i);
            if (a == ALIGN_NONE)
                a = AlignmentFromStyle(m_style);
            m_col_alignments[i] = a;
        }

        const GG::X_d SCALE_FACTOR = 1.0 * WIDTH / total;

        total = GG::X0;
        for (std::size_t i = 0; i < row->size(); ++i) {
            total += (m_col_widths[i] = row->ColWidth(i) * SCALE_FACTOR);
        }
        m_col_widths.back() += total - WIDTH;

        if (!m_header_row->empty())
            NormalizeRow(m_header_row);
    }

    // housekeeping of rows...
    for (std::vector<Row*>::const_iterator row_it = rows.begin();
         row_it != rows.end(); ++row_it)
    {
        Row* row = *row_it;
        row->InstallEventFilter(this);
        NormalizeRow(row);
    }

    // add row at requested location (or default end position)
    m_rows.insert(it, rows.begin(), rows.end());

    // more housekeeping of rows...
    for (std::vector<Row*>::const_iterator row_it = rows.begin();
         row_it != rows.end(); ++row_it)
    {
        Row* row = *row_it;
        AttachChild(row);
    }

    // sort?
    if (!(m_style & LIST_NOSORT)) {
        Resort();
    } else {
        // reposition rows to account for insertion
        Y y(0);
        for (it = m_rows.begin(); it != m_rows.end(); ++it) {
            Row* row = *it;
            row->MoveTo(Pt(X0, y));
            y += row->Height();
        }
    }

    if (m_first_row_shown == m_rows.end())
        m_first_row_shown = m_rows.begin();

    AdjustScrolls(false);
}

ListBox::Row* ListBox::Erase(iterator it, bool removing_duplicate, bool signal)
{
    if (it != m_rows.end()) {
        if (m_iterator_being_erased) {
            *m_iterator_being_erased = m_rows.end();
            return 0;
        }

        Row* row = *it;
        Y row_height = row->Height();
        if (!removing_duplicate) {
            DetachChild(row);
            row->RemoveEventFilter(this);
        }

        for (iterator it2 = boost::next(it); it2 != m_rows.end(); ++it2) {
            (*it2)->OffsetMove(Pt(X0, -row_height));
        }

        ResetIfEqual(m_old_sel_row, it, m_rows.end());
        ResetIfEqual(m_old_rdown_row, it, m_rows.end());
        ResetIfEqual(m_lclick_row, it, m_rows.end());
        ResetIfEqual(m_rclick_row, it, m_rows.end());
        ResetIfEqual(m_last_row_browsed, it, m_rows.end());

        bool check_first_row_and_caret_for_end = false;
        if (m_first_row_shown == it) {
            ++m_first_row_shown;
            check_first_row_and_caret_for_end = true;
        }
        if (m_caret == it) {
            ++m_caret;
            check_first_row_and_caret_for_end = true;
        }

        // Tracking this iterator is necessary because the signal may indirectly
        // cause the iterator to be invalidated.
        ScopedSet scoped_set(m_iterator_being_erased, &it);

        if (signal && !removing_duplicate)
            BeforeEraseSignal(it);

        if (it != m_rows.end()) {
            m_selections.erase(it);
            m_rows.erase(it);
        }

        if (check_first_row_and_caret_for_end && m_first_row_shown == m_rows.end() && !m_rows.empty())
            --m_first_row_shown;
        if (check_first_row_and_caret_for_end && m_caret == m_rows.end() && !m_rows.empty())
            --m_caret;

        AdjustScrolls(false);

        if (signal && !removing_duplicate)
            AfterEraseSignal(it);

        return row;
    } else {
        return 0;
    }
}

void ListBox::BringCaretIntoView()
{ BringRowIntoView(m_caret); }

void ListBox::RecreateScrolls()
{
    delete m_vscroll;
    delete m_hscroll;
    m_vscroll = m_hscroll = 0;
    AdjustScrolls(false);
}

void ListBox::ResetAutoScrollVars()
{
    m_auto_scrolling_up = false;
    m_auto_scrolling_down = false;
    m_auto_scrolling_left = false;
    m_auto_scrolling_right = false;
    m_auto_scroll_timer.Stop();
}

void ListBox::Resort()
{
    Row* caret = SafeDeref(m_caret, m_rows.end());
    std::set<Row*> selections;
    for (SelectionSet::const_iterator it = m_selections.begin(); it != m_selections.end(); ++it) {
        selections.insert(**it);
    }
    m_selections.clear();
    Row* old_sel_row =      SafeDeref(m_old_sel_row, m_rows.end());
    Row* old_rdown_row =    SafeDeref(m_old_rdown_row, m_rows.end());
    Row* lclick_row =       SafeDeref(m_lclick_row, m_rows.end());
    Row* rclick_row =       SafeDeref(m_rclick_row, m_rows.end());
    Row* last_row_browsed = SafeDeref(m_last_row_browsed, m_rows.end());

    std::vector<Row*> rows_vec(m_rows.size());
    std::copy(m_rows.begin(), m_rows.end(), rows_vec.begin());
    std::stable_sort(rows_vec.begin(), rows_vec.end(),
                     RowSorter(m_sort_cmp, m_sort_col, m_style & LIST_SORTDESCENDING));
    m_rows.clear();
    m_rows.insert(m_rows.begin(), rows_vec.begin(), rows_vec.end());

    if (m_iterator_being_erased)
        *m_iterator_being_erased = m_rows.end();

    Y y(0);
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        Row* row = *it;
        if (caret == row)
            m_caret = it;
        if (selections.find(row) != selections.end())
            m_selections.insert(it);
        if (old_sel_row == row)
            m_old_sel_row = it;
        if (old_rdown_row == row)
            m_old_rdown_row = it;
        if (lclick_row == row)
            m_lclick_row = it;
        if (rclick_row == row)
            m_rclick_row = it;
        if (last_row_browsed == row)
            m_last_row_browsed = it;

        row->MoveTo(Pt(X0, y));
        y += row->Height();
    }

    m_first_row_shown = m_rows.empty() ? m_rows.end() : m_rows.begin();
}

ListBox::Row& ListBox::ColHeaders()
{ return *m_header_row; }

void ListBox::ConnectSignals()
{
    if (m_vscroll)
        Connect(m_vscroll->ScrolledSignal, &ListBox::VScrolled, this);
    if (m_hscroll)
        Connect(m_hscroll->ScrolledSignal, &ListBox::HScrolled, this);
}

void ListBox::ValidateStyle()
{
    int dup_ct = 0;   // duplication count
    if (m_style & LIST_LEFT) ++dup_ct;
    if (m_style & LIST_RIGHT) ++dup_ct;
    if (m_style & LIST_CENTER) ++dup_ct;
    if (dup_ct != 1) {  // exactly one must be picked; when none or multiples are picked, use LIST_LEFT by default
        m_style &= ~(LIST_RIGHT | LIST_CENTER);
        m_style |= LIST_LEFT;
    }
    dup_ct = 0;
    if (m_style & LIST_TOP) ++dup_ct;
    if (m_style & LIST_BOTTOM) ++dup_ct;
    if (m_style & LIST_VCENTER) ++dup_ct;
    if (dup_ct != 1) {  // exactly one must be picked; when none or multiples are picked, use LIST_VCENTER by default
        m_style &= ~(LIST_TOP | LIST_BOTTOM);
        m_style |= LIST_VCENTER;
    }
    dup_ct = 0;
    if (m_style & LIST_NOSEL) ++dup_ct;
    if (m_style & LIST_SINGLESEL) ++dup_ct;
    if (m_style & LIST_QUICKSEL) ++dup_ct;
    if (1 < dup_ct)  // at most one of these may be picked; when multiples are picked, disable all of them
        m_style &= ~(LIST_NOSEL | LIST_SINGLESEL | LIST_QUICKSEL);
}

void ListBox::AdjustScrolls(bool adjust_for_resize)
{
    // this client area calculation disregards the thickness of scrolls
    Pt cl_sz = (LowerRight() - Pt(X(BORDER_THICK), Y(BORDER_THICK))) -
        (UpperLeft() + Pt(X(BORDER_THICK), static_cast<int>(BORDER_THICK)
            + (m_header_row->empty()
               ? Y0
               : m_header_row->Height())));

    X total_x_extent = std::accumulate(m_col_widths.begin(), m_col_widths.end(), X0);
    Y total_y_extent(0);
    if (!m_rows.empty())
        total_y_extent = m_rows.back()->Bottom() - m_rows.front()->Top();

    bool vertical_needed =
        m_first_row_shown != m_rows.begin() ||
        m_rows.size() && (cl_sz.y < total_y_extent ||
                          (cl_sz.y < total_y_extent - SCROLL_WIDTH &&
                           cl_sz.x < total_x_extent - SCROLL_WIDTH));
    bool horizontal_needed =
        m_first_col_shown ||
        m_rows.size() && (cl_sz.x < total_x_extent ||
                          (cl_sz.x < total_x_extent - SCROLL_WIDTH &&
                           cl_sz.y < total_y_extent - SCROLL_WIDTH));

    // This probably looks a little odd.  We only want to show scrolls if they
    // are needed, that is if the data shown exceed the bounds of the client
    // area.  However, if we are going to show scrolls, we want to allow them
    // to range such that the first row/column shown can be any of the N
    // rows/columns.  Dead space after the last row/column is fine.
    if (!m_col_widths.empty() && m_col_widths.back() < cl_sz.x)
        total_x_extent += cl_sz.x - m_col_widths.back();
    if (!m_rows.empty() && m_rows.back()->Height() < cl_sz.y)
        total_y_extent += cl_sz.y - m_rows.back()->Height();

    boost::shared_ptr<StyleFactory> style = GetStyleFactory();

    if (m_vscroll) { // if scroll already exists...
        if (!vertical_needed) { // remove scroll
            DeleteChild(m_vscroll);
            m_vscroll = 0;

        } else { // ensure vertical scroll has the right logical and physical dimensions
            X scroll_x = cl_sz.x - SCROLL_WIDTH;
            Y scroll_y(0);
            if (adjust_for_resize) {
                m_vscroll->SizeMove(Pt(scroll_x, scroll_y),
                                    Pt(scroll_x + SCROLL_WIDTH,
                                       scroll_y + cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0)));
            }

            unsigned int line_size = m_vscroll_wheel_scroll_increment;
            if (line_size == 0 && !this->Empty()) {
                const Row* row = *begin();
                line_size = Value(row->Height());
            }

            unsigned int page_size = std::abs(Value(cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0)));

            m_vscroll->SizeScroll(0, Value(total_y_extent - 1),
                                  line_size, std::max(line_size, page_size));

            MoveChildUp(m_vscroll);
        }
    } else if (!m_vscroll && vertical_needed) { // if scroll doesn't exist but is needed
        m_vscroll =
            style->NewListBoxVScroll(
                cl_sz.x - SCROLL_WIDTH, Y0,
                X(SCROLL_WIDTH), cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0),
                m_color, CLR_SHADOW);

        unsigned int line_size = m_vscroll_wheel_scroll_increment;
        if (line_size == 0 && !this->Empty()) {
            const Row* row = *begin();
            line_size = Value(row->Height());
        }

        unsigned int page_size = std::abs(Value(cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0)));

        m_vscroll->SizeScroll(0, Value(total_y_extent - 1),
                              line_size, std::max(line_size, page_size));
        AttachChild(m_vscroll);
        Connect(m_vscroll->ScrolledSignal, &ListBox::VScrolled, this);
    }

    if (m_hscroll) { // if scroll already exists...
        if (!horizontal_needed) { // remove scroll
            DeleteChild(m_hscroll);
            m_hscroll = 0;
        } else { // ensure horizontal scroll has the right logical and physical dimensions
            X scroll_x(0);
            Y scroll_y = cl_sz.y - SCROLL_WIDTH;
            if (adjust_for_resize) {
                m_hscroll->SizeMove(Pt(scroll_x, scroll_y),
                                    Pt(scroll_x + cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0),
                                       scroll_y + SCROLL_WIDTH));
            }

            unsigned int line_size = m_hscroll_wheel_scroll_increment;
            if (line_size == 0 && !this->Empty()) {
                const Row* row = *begin();
                line_size = Value(row->Height());
            }

            unsigned int page_size = std::abs(Value(cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0)));

            m_hscroll->SizeScroll(0, Value(total_x_extent - 1),
                                  line_size, std::max(line_size, page_size));
            MoveChildUp(m_hscroll);
        }
    } else if (!m_hscroll && horizontal_needed) { // if scroll doesn't exist but is needed
        m_hscroll =
            style->NewListBoxHScroll(
                X0, cl_sz.y - SCROLL_WIDTH,
                cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0), Y(SCROLL_WIDTH),
                m_color, CLR_SHADOW);

        unsigned int line_size = m_hscroll_wheel_scroll_increment;
        if (line_size == 0 && !this->Empty()) {
            const Row* row = *begin();
            line_size = Value(row->Height());
        }

        unsigned int page_size = std::abs(Value(cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0)));

        m_hscroll->SizeScroll(0, Value(total_x_extent - 1),
                              line_size, std::max(line_size, page_size));
        AttachChild(m_hscroll);
        Connect(m_hscroll->ScrolledSignal, &ListBox::HScrolled, this);
    }
}

void ListBox::VScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_row_shown = m_rows.empty() ? m_rows.end() : m_rows.begin();
    Y accum(0);
    Y position(0);
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        Y row_height = (*it)->Height();
        if (tab_low < accum + row_height / 2) {
            m_first_row_shown = it;
            position = -accum;
            break;
        }
        accum += row_height;
    }
    X initial_x = m_rows.empty() ? X0 : (*m_rows.begin())->RelativeUpperLeft().x;
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        (*it)->MoveTo(Pt(initial_x, position));
        position += (*it)->Height();
    }
}

void ListBox::HScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_col_shown = 0;
    X accum(0);
    X position(0);
    for (std::size_t i = 0; i < m_col_widths.size(); ++i) {
        X col_width = m_col_widths[i];
        if (tab_low < accum + col_width / 2) {
            m_first_col_shown = i;
            position = -accum;
            break;
        }
        accum += col_width;
    }
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        (*it)->MoveTo(Pt(position, (*it)->RelativeUpperLeft().y));
    }
    m_header_row->MoveTo(Pt(position, m_header_row->RelativeUpperLeft().y));
}

void ListBox::ClickAtRow(iterator it, Flags<ModKey> mod_keys)
{
    //assert(it != m_rows.end());   // replaced with following test and return to avoid crashes
    if (it == m_rows.end())
        return;
    //assert(!m_rows.empty());      // replaced with following test and return to avoid crashes
    if (m_rows.empty())
        return;

    SelectionSet previous_selections = m_selections;

    if (m_style & LIST_SINGLESEL) {
        // No special keys are being used; just clear all previous selections,
        // select this row, set the caret here.
        m_selections.clear();
        m_selections.insert(it);
        m_caret = it;
    } else {
        if (mod_keys & MOD_KEY_CTRL) { // control key depressed
            if (mod_keys & MOD_KEY_SHIFT && m_caret != m_rows.end()) {
                // Both shift and control keys are depressed.
                iterator low  = LessThan(m_caret, it, m_rows.end()) ? m_caret : it;
                iterator high = LessThan(m_caret, it, m_rows.end()) ? it : m_caret;
                bool erase = m_selections.find(m_caret) == m_selections.end();
                if (high != m_rows.end())
                    ++high;
                for (iterator it2 = low; it2 != high; ++it2) {
                    if (erase)
                        m_selections.erase(it2);
                    else
                        m_selections.insert(it2);
                }
            } else { // just the control key is depressed: toggle the item selected, adjust the caret
                if (m_old_sel_row_selected)
                    m_selections.erase(it);
                else
                    m_selections.insert(it);
                m_caret = it;
            }
        } else if (mod_keys & MOD_KEY_SHIFT) { // shift key depressed
            bool erase = m_selections.find(m_caret) == m_selections.end();
            if (!(m_style & LIST_QUICKSEL))
                m_selections.clear();
            if (m_caret == m_rows.end()) {
                // No previous caret exists; select this row, mark it as the caret.
                m_selections.insert(it);
                m_caret = it;
            } else { // select all rows between the caret and this row (inclusive), don't move the caret
                iterator low  = LessThan(m_caret, it, m_rows.end()) ? m_caret : it;
                iterator high = LessThan(m_caret, it, m_rows.end()) ? it : m_caret;
                if (high != m_rows.end())
                    ++high;
                for (iterator it2 = low; it2 != high; ++it2) {
                    if (erase)
                        m_selections.erase(it2);
                    else
                        m_selections.insert(it2);
                }
            }
        } else { // unless LIST_QUICKSEL is used, this is treated just like LIST_SINGLESEL above
            if (m_style & LIST_QUICKSEL) {
                if (m_old_sel_row_selected)
                    m_selections.erase(it);
                else
                    m_selections.insert(it);
                m_caret = it;
            } else {
                m_selections.clear();
                m_selections.insert(it);
                m_caret = it;
            }
        }
    }

    if (previous_selections != m_selections)
        SelChangedSignal(m_selections);
}

void ListBox::NormalizeRow(Row* row)
{
    Row::DeferAdjustLayout defer_adjust_layout(row);
    row->resize(m_col_widths.size());
    row->SetColWidths(m_col_widths);
    row->SetColAlignments(m_col_alignments);
    row->SetMargin(m_cell_margin);
    row->Resize(Pt(std::accumulate(m_col_widths.begin(), m_col_widths.end(), X0), row->Height()));
}

ListBox::iterator ListBox::FirstRowShownWhenBottomIs(iterator bottom_row, Y client_height)
{
    if (bottom_row == m_rows.end())
        return m_rows.begin();
    Y available_space = client_height - (*bottom_row)->Height();
    iterator it = bottom_row;
    while (it != m_rows.begin() && (*boost::prior(it))->Height() <= available_space) {
        available_space -= (*--it)->Height();
    }
    return it;
}

std::size_t ListBox::FirstColShownWhenRightIs(std::size_t right_col, X client_width)
{
    if (right_col == static_cast<std::size_t>(-1))
        return 0;
    X available_space = client_width - m_col_widths[right_col];
    std::size_t i = right_col;
    while (0 < i && m_col_widths[i - 1] <= available_space) {
        available_space -= m_col_widths[--i];
    }
    return i;
}
