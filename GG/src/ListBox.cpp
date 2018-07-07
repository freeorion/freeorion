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

#include <GG/DeferredLayout.h>
#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/Scroll.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>

#include <boost/cast.hpp>

#include <iterator>
#include <numeric>


using namespace GG;

// static(s)
const int ListBox::DEFAULT_MARGIN(2);
const X ListBox::DEFAULT_ROW_WIDTH(50);
const Y ListBox::DEFAULT_ROW_HEIGHT(22);
const unsigned int ListBox::BORDER_THICK = 2;

namespace {
    struct ListSignalEcho
    {
        ListSignalEcho(const ListBox& lb, const std::string& name) :
            m_LB(lb),
            m_name(name)
        {}

        void operator()()
        { std::cerr << "GG SIGNAL : " << m_name << "()" << std::endl; }

        void operator()(const ListBox::SelectionSet& sels)
        {
            std::cerr << "GG SIGNAL : " << m_name << "(sels=[ ";

            for (const auto& sel : sels)
            { std::cerr << RowIndex(sel) << ' '; }

            std::cerr << "])" << std::endl;
        }

        void operator()(ListBox::const_iterator it)
        { std::cerr << "GG SIGNAL : " << m_name << "(row=" << RowIndex(it) << ")" << std::endl; }

        void operator()(ListBox::const_iterator it, const Pt& pt, const Flags<ModKey>& mod_keys)
        { std::cerr << "GG SIGNAL : " << m_name << "(row=" << RowIndex(it) << " pt=" << pt << ")" << std::endl; }

        std::size_t RowIndex(ListBox::const_iterator it)
        { return std::distance(m_LB.begin(), it); }

        const ListBox& m_LB;
        std::string m_name;
    };

    const int SCROLL_WIDTH = 14;

    class RowSorter // used to sort rows by a certain column (which may contain some empty cells)
    {
    public:
        RowSorter(const boost::function<bool (const ListBox::Row&, const ListBox::Row&, std::size_t)>& cmp,
                  std::size_t col, bool invert) :
            m_cmp(cmp),
            m_sort_col(col),
            m_invert(invert)
        {}

        bool operator()(const std::shared_ptr<ListBox::Row>& l, const std::shared_ptr<ListBox::Row>& r)
        { return m_invert ? m_cmp(*r, *l, m_sort_col) : m_cmp(*l, *r, m_sort_col); }
        bool operator()(const ListBox::Row* l, const ListBox::Row* r)
        { return m_invert ? m_cmp(*r, *l, m_sort_col) : m_cmp(*l, *r, m_sort_col); }

    private:
        boost::function<bool (const ListBox::Row&, const ListBox::Row&, std::size_t)> m_cmp;
        std::size_t m_sort_col;
        bool m_invert;
    };

    ListBox::Row* SafeDeref(const ListBox::iterator& it, const ListBox::iterator& end)
    { return it == end ? nullptr : it->get(); }

    std::shared_ptr<ListBox::Row> IteratorToShared(const ListBox::iterator& it, const ListBox::iterator& end)
    { return it == end ? std::shared_ptr<ListBox::Row>() : std::shared_ptr<ListBox::Row>(*it); }

    bool RowAboveOrIsRow(ListBox::iterator lhs, ListBox::iterator rhs, ListBox::iterator end)
    {
        if (rhs == end)
            return true;
        if (lhs == end)
            return false;
        if (lhs == rhs)
            return true;
        const ListBox::Row* lhs_row = SafeDeref(lhs, end);
        const ListBox::Row* rhs_row = SafeDeref(rhs, end);
        if (!rhs_row)
            return true;
        if (!lhs_row)
            return false;
        return lhs_row->Top() < rhs_row->Top();
    }

    void ResetIfEqual(ListBox::iterator& val, ListBox::iterator other, ListBox::iterator end)
    {
        if (val == other)
            val = end;
    }

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


namespace {
    /** Make \p layout at least \p size large*/
    void ValidateLayoutSize(GG::Layout* layout, std::size_t size)
    {
        if (layout->Columns() < size)
            layout->ResizeLayout(1, size);
    }
}

////////////////////////////////////////////////
// GG::ListBox::Row
////////////////////////////////////////////////
ListBox::Row::Row() :
    Control(X0, Y0, ListBox::DEFAULT_ROW_WIDTH, ListBox::DEFAULT_ROW_HEIGHT),
    m_cells(),
    m_row_alignment(ALIGN_VCENTER),
    m_col_alignments(),
    m_col_widths(),
    m_col_stretches(),
    m_margin(ListBox::DEFAULT_MARGIN),
    m_ignore_adjust_layout(false),
    m_is_normalized(false)
{}

ListBox::Row::Row(X w, Y h, const std::string& drag_drop_data_type,
                  Alignment align/* = ALIGN_VCENTER*/, unsigned int margin/* = 2*/) : 
    Control(X0, Y0, w, h),
    m_cells(),
    m_row_alignment(align),
    m_col_alignments(),
    m_col_widths(),
    m_col_stretches(),
    m_margin(margin),
    m_ignore_adjust_layout(false),
    m_is_normalized(false)
{ SetDragDropDataType(drag_drop_data_type); }

void ListBox::Row::CompleteConstruction()
{ SetLayout(Wnd::Create<DeferredLayout>(X0, Y0, Width(), Height(), 1, 1, m_margin, m_margin)); }

ListBox::Row::~Row()
{}

std::string ListBox::Row::SortKey(std::size_t col) const
{
    if (col >= m_cells.size()) {
        std::cout << "ListBox::Row::SortKey out of range column = " << col << " > num cols = " << m_cells.size();
        return "";
    }

    const TextControl* text_control = dynamic_cast<const TextControl*>(at(col));
    return text_control ? text_control->Text() : "";
}

std::size_t ListBox::Row::size() const
{ return m_cells.size(); }

bool ListBox::Row::empty() const
{ return m_cells.empty(); }

Control* ListBox::Row::at(std::size_t n) const
{ return m_cells.at(n).get(); }

Alignment ListBox::Row::RowAlignment() const
{ return m_row_alignment; }

Alignment ListBox::Row::ColAlignment(std::size_t n) const
{ return m_col_alignments[n]; }

X ListBox::Row::ColWidth(std::size_t n) const
{ return m_col_widths[n]; }

unsigned int ListBox::Row::Margin() const
{ return m_margin; }

bool ListBox::Row::IsNormalized() const
{ return m_is_normalized; }

void ListBox::Row::Render()
{}

void ListBox::Row::GrowWidthsStretchesAlignmentsTo(std::size_t nn) {
    if (m_col_widths.size() < nn) {
        m_col_widths.resize(nn, X(5));
        m_col_alignments.resize(nn, ALIGN_NONE);
        m_col_stretches.resize(nn, 0.0);
    }
}

void ListBox::Row::push_back(std::shared_ptr<Control> c)
{
    m_cells.push_back(c);
    GrowWidthsStretchesAlignmentsTo(m_cells.size());
    auto ii = m_cells.size() - 1;
    auto&& layout = GetLayout();
    if (c) {
        layout->Add(std::forward<std::shared_ptr<Control>>(c), 0, ii, m_row_alignment | m_col_alignments[ii]);
        layout->SetMinimumColumnWidth(ii, m_col_widths.back());
        layout->SetColumnStretch(ii, m_col_stretches.back());
    }
}

void ListBox::Row::clear()
{
    m_cells.clear();
    RemoveLayout();
    DetachChildren();
    SetLayout(Wnd::Create<DeferredLayout>(X0, Y0, Width(), Height(), 1, 1, m_margin, m_margin));
}

void ListBox::Row::resize(std::size_t n)
{
    if (n == m_cells.size())
        return;

    auto&& layout = GetLayout();
    for (auto& cell : m_cells) {
        layout->Remove(cell.get());
    }

    std::size_t old_size = m_cells.size();

    for (std::size_t ii = n; ii < old_size; ++ii) {
        m_cells[ii].reset();
    }
    m_cells.resize(n, nullptr);
    m_col_widths.resize(n);
    m_col_alignments.resize(n);
    m_col_stretches.resize(n);
    for (std::size_t ii = old_size; ii < n; ++ii) {
        m_col_widths[ii] = old_size ? m_col_widths[old_size - 1] : X(5); // assign new cells reasonable default widths
        m_col_alignments[ii] = ALIGN_NONE;
        m_col_stretches[ii] = 0.0;
    }

    DetachChildren();
    SetLayout(layout);

    bool nonempty_cell_found = false;
    for (auto& control : m_cells) {
        if (control) {
            nonempty_cell_found = true;
            break;
        }
    }

    if (!nonempty_cell_found)
        return;

    layout->ResizeLayout(1, m_cells.size());
    for (std::size_t ii = 0; ii < m_cells.size(); ++ii) {
        if (!m_col_widths.empty())
            layout->SetMinimumColumnWidth(ii, m_col_widths[ii]);
        if (!m_col_stretches.empty())
            layout->SetColumnStretch(ii, m_col_stretches[ii]);
        if (m_cells[ii]) {
            if (m_col_alignments.empty())
                layout->Add(m_cells[ii], 0, ii, m_row_alignment);
            else
                layout->Add(m_cells[ii], 0, ii, m_row_alignment | m_col_alignments[ii]);
        }
    }
}

void ListBox::Row::SetCell(std::size_t n, const std::shared_ptr<Control>& c)
{
    if (c == m_cells[n])
        return;

    auto&& layout = GetLayout();

    if (m_cells.size() > n && m_cells[n]) {
        layout->Remove(m_cells[n].get());
        m_cells[n].reset();
    }

    m_cells[n] = c;

    if (!c)
        return;
    if (layout->Columns() <= n)
        layout->ResizeLayout(1, n + 1);
    layout->Add(c, 0, n, m_row_alignment | m_col_alignments[n]);
}

Control* ListBox::Row::RemoveCell(std::size_t n)
{
    if (m_cells.size() <= n)
        return nullptr;
    auto&& layout = GetLayout();
    auto& retval = m_cells[n];
    layout->Remove(retval.get());
    m_cells[n].reset();
    return retval.get();
}

void ListBox::Row::SetRowAlignment(Alignment align)
{
    if (align == m_row_alignment)
        return;

    m_row_alignment = align;

    auto&& layout = GetLayout();
    for (std::size_t ii = 0; ii < m_cells.size(); ++ii) {
        if (m_cells[ii])
            layout->Add(m_cells[ii], 0, ii,
                        (m_row_alignment
                         | (m_col_alignments.empty() ? ALIGN_NONE : m_col_alignments[ii])));
    }
}

void ListBox::Row::SetColAlignment(std::size_t n, Alignment align)
{
    GrowWidthsStretchesAlignmentsTo(n + 1);
    if (align == m_col_alignments[n])
        return;

    m_col_alignments[n] = align;
    auto&& layout = GetLayout();
    ValidateLayoutSize(layout.get(), n + 1);
    if (m_cells[n])
        layout->SetChildAlignment(m_cells[n].get(), m_row_alignment | align);
}

void ListBox::Row::SetColWidth(std::size_t n, X width)
{
    GrowWidthsStretchesAlignmentsTo(n + 1);
    if (width == m_col_widths[n])
        return;

    m_col_widths[n] = width;

    auto&& layout = GetLayout();
    ValidateLayoutSize(layout.get(), n + 1);
    layout->SetMinimumColumnWidth(n, width);
}

void ListBox::Row::SetColAlignments(const std::vector<Alignment>& aligns)
{
    if (aligns == m_col_alignments)
        return;

    m_col_alignments = aligns;
    m_col_alignments.resize(m_cells.size(), ALIGN_NONE);
    auto&& layout = GetLayout();
    ValidateLayoutSize(layout.get(), aligns.size());
    for (std::size_t ii = 0; ii < m_cells.size(); ++ii) {
        if (m_cells[ii])
            layout->SetChildAlignment(m_cells[ii].get(), m_row_alignment | m_col_alignments[ii]);
    }
}

void ListBox::Row::ClearColAlignments()
{
    if (m_col_alignments.empty())
        return;

    m_col_alignments.clear();
    auto&& layout = GetLayout();
    for (auto& control : m_cells) {
        if (control)
            layout->SetChildAlignment(control.get(), m_row_alignment);
    }
}

void ListBox::Row::SetColWidths(const std::vector<X>& widths)
{
    if (widths == m_col_widths)
        return;

    m_col_widths = widths;
    m_col_widths.resize(m_cells.size(), GG::X(5));
    auto&& layout = GetLayout();
    ValidateLayoutSize(layout.get(), widths.size());
    for (std::size_t ii = 0; ii < m_cells.size(); ++ii) {
        layout->SetMinimumColumnWidth(ii, m_col_widths[ii]);
    }
}

void ListBox::Row::ClearColWidths()
{
    if (m_col_widths.empty())
        return;

    m_col_widths.clear();
    auto&& layout = GetLayout();
    ValidateLayoutSize(layout.get(), m_cells.size());
    for (std::size_t ii = 0; ii < m_cells.size(); ++ii) {
        layout->SetMinimumColumnWidth(ii, GG::X0);
    }
}

void ListBox::Row::SetColStretches(const std::vector<double>& stretches)
{
    if (stretches == m_col_stretches)
        return;

    m_col_stretches = stretches;
    m_col_stretches.resize(m_cells.size(), 0.0);
    auto&& layout = GetLayout();
    ValidateLayoutSize(layout.get(), m_col_stretches.size());
    for (std::size_t ii = 0; ii < m_cells.size(); ++ii) {
        layout->SetColumnStretch(ii, m_col_stretches[ii]);
    }
}

void ListBox::Row::SetMargin(unsigned int margin)
{
    if (margin == m_margin)
        return;

    m_margin = margin;
    auto&& layout = GetLayout();
    layout->SetBorderMargin(margin);
    layout->SetCellMargin(margin);
}

void ListBox::Row::SetNormalized(bool normalized)
{ m_is_normalized = normalized; }

void ListBox::Row::RClick(const Pt& pt, GG::Flags<GG::ModKey> mod) {
     RightClickedSignal(pt, mod);
}

////////////////////////////////////////////////
// GG::ListBox::RowPtrIteratorLess
////////////////////////////////////////////////
bool ListBox::RowPtrIteratorLess::operator()(const ListBox::iterator& lhs, const ListBox::iterator& rhs) const
{ return (*lhs)->Top() < (*rhs)->Top(); }


////////////////////////////////////////////////
// GG::ListBox
////////////////////////////////////////////////

ListBox::ListBox(Clr color, Clr interior/* = CLR_ZERO*/) :
    Control(X0, Y0, X1, Y1, INTERACTIVE),
    m_caret(m_rows.end()),
    m_old_sel_row(m_rows.end()),
    m_old_rdown_row(m_rows.end()),
    m_lclick_row(m_rows.end()),
    m_rclick_row(m_rows.end()),
    m_last_row_browsed(m_rows.end()),
    m_first_row_shown(m_rows.end()),
    m_cell_margin(DEFAULT_MARGIN),
    m_int_color(interior),
    m_header_row(Wnd::Create<Row>()),
    m_sort_cmp(DefaultRowCmp<Row>())
{
    Control::SetColor(color);
}

void ListBox::CompleteConstruction()
{
    ValidateStyle();
    SetChildClippingMode(ClipToClient);
    m_auto_scroll_timer.Stop();
    m_auto_scroll_timer.Connect(this);

    InstallEventFilter(shared_from_this());

    if (INSTRUMENT_ALL_SIGNALS) {
        ClearedRowsSignal.connect(ListSignalEcho(*this, "ListBox::ClearedRowsSignal"));
        BeforeInsertRowSignal.connect(ListSignalEcho(*this, "ListBox::BeforeInsertRowSignal"));
        AfterInsertRowSignal.connect(ListSignalEcho(*this, "ListBox::AfterInsertRowSignal"));
        SelRowsChangedSignal.connect(ListSignalEcho(*this, "ListBox::SelRowsChangedSignal"));
        DroppedRowSignal.connect(ListSignalEcho(*this, "ListBox::DroppedRowSignal"));
        LeftClickedRowSignal.connect(ListSignalEcho(*this, "ListBox::LeftClickedRowSignal"));
        RightClickedRowSignal.connect(ListSignalEcho(*this, "ListBox::RightClickedRowSignal"));
        DoubleClickedRowSignal.connect(ListSignalEcho(*this, "ListBox::DoubleClickedRowSignal"));
        BeforeEraseRowSignal.connect(ListSignalEcho(*this, "ListBox::BeforeEraseRowSignal"));
        AfterEraseRowSignal.connect(ListSignalEcho(*this, "ListBox::AfterEraseRowSignal"));
        BrowsedRowSignal.connect(ListSignalEcho(*this, "ListBox::BrowsedRowSignal"));
    }
}

ListBox::~ListBox()
{}

void ListBox::AllowDrops(bool allow)
{ m_allow_drops = allow; }

bool ListBox::AllowingDrops()
{ return m_allow_drops; }

void ListBox::AllowAllDropTypes(bool allow) {
    // If all types are allow use boost::none as a sentinel
    if (allow)
        m_allowed_drop_types = boost::none;

    // Otherwise hold each allowed type in a set.
    else if (!m_allowed_drop_types)
        m_allowed_drop_types = std::unordered_set<std::string>();
}

void ListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                              const Pt& pt, Flags<ModKey> mod_keys) const
{
    for (auto& it = first; it != last; ++it) {
        const auto& row = dynamic_cast<const Row* const>(it->first);

        bool allowed = (m_allow_drops
                        && row
                        && AllowedDropType(row->DragDropDataType()));

        it->second = allowed;
    }
}

void ListBox::HandleRowRightClicked(const Pt& pt, GG::Flags<GG::ModKey> mod) {
    iterator row_it = RowUnderPt(pt);
    if (row_it != m_rows.end()) {
        m_rclick_row = row_it;
        RightClickedRowSignal(row_it, pt, mod);
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

const ListBox::Row& ListBox::GetRow(std::size_t n) const
{
    assert(n < m_rows.size());
    return **std::next(m_rows.begin(), n);
}

ListBox::iterator ListBox::Caret() const
{ return m_caret; }

const ListBox::SelectionSet& ListBox::Selections() const
{ return m_selections; }

bool ListBox::Selected(iterator it) const
{ return it != m_rows.end() && m_selections.count(it); }

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
    if (m_first_row_shown == m_rows.end())
        return 0;

    // Find the last column that is entirely left of the rightmost pixel.
    X rightmost_pixel = ClientLowerRight().x;
    std::size_t ii_last_visible(0);
    for (auto& cell : (*m_first_row_shown)->GetLayout()->Children()) {
        if (cell->UpperLeft().x >= rightmost_pixel)
            break;
        if ((cell->UpperLeft().x < rightmost_pixel) && (cell->LowerRight().x >= rightmost_pixel))
            return ii_last_visible;
        ++ii_last_visible;
    }

    return (ii_last_visible ? (ii_last_visible - 1) : 0);
}

std::size_t ListBox::NumRows() const
{ return m_rows.size(); }

std::size_t ListBox::NumCols() const
{ return m_num_cols; }

bool ListBox::KeepColWidths() const
{ return m_keep_col_widths; }

bool ListBox::ManuallyManagingColProps() const
{ return !m_manage_column_props; }

std::size_t ListBox::SortCol() const
{ return m_sort_col; }

X ListBox::ColWidth(std::size_t n) const
{ return m_col_widths[n]; }

Alignment ListBox::ColAlignment(std::size_t n) const
{ return m_col_alignments[n]; }

Alignment ListBox::RowAlignment(iterator it) const
{ return (*it)->RowAlignment(); }

double ListBox::ColStretch(std::size_t n) const
{ return m_col_stretches[n]; }

bool ListBox::AllowedDropType(const std::string& type) const
{
    return (!m_allowed_drop_types                 // all types allowed
            || m_allowed_drop_types->count(type)); //this type allowed;
}

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

    iterator wnd_it = std::find_if(m_rows.begin(), m_rows.end(),
                                   [&wnd](const std::shared_ptr<Row>& x){ return x.get() == wnd; });
    if (wnd_it == m_rows.end())
        return;

    if (!m_selections.count(wnd_it))
        return;

    // Preserve the displayed row order in the dragged selections by finding the y offset of wnd
    // and adjusting all the dragged rows relative to wnd.
    std::map<GG::Y, SelectionSet::iterator> selections_Y_sorted;
    for (SelectionSet::iterator sel_it = m_selections.begin(); sel_it != m_selections.end(); ++sel_it) {
        selections_Y_sorted.insert({(**sel_it)->Top(), sel_it});
    }

    Y vertical_offset = offset.y;
    for (const auto& sorted_sel : selections_Y_sorted) {
        auto row_wnd = **(sorted_sel.second);
        if (row_wnd.get() == wnd)
            break;
        vertical_offset += row_wnd->Height();
    }

    for (const auto& sorted_sel : selections_Y_sorted) {
        auto row_wnd = **(sorted_sel.second);
        if (row_wnd.get() != wnd) {
            GUI::GetGUI()->RegisterDragDropWnd(row_wnd, Pt(offset.x, vertical_offset), shared_from_this());
            vertical_offset -= row_wnd->Height();
        } else {
            vertical_offset -= wnd->Height();
        }
    }
}

void ListBox::AcceptDrops(const Pt& pt, std::vector<std::shared_ptr<Wnd>> wnds, Flags<ModKey> mod_keys)
{
    iterator insertion_it = RowUnderPt(pt);
    bool inserting_at_first_row = insertion_it == m_first_row_shown;
    for (auto& wnd : wnds) {
        if (const auto& row = std::dynamic_pointer_cast<Row>(wnd))
            Insert(row, insertion_it, true);
    }

    // Adjust to show rows inserted before the first row.
    if (inserting_at_first_row)
        SetFirstRowShown(std::prev(m_first_row_shown, wnds.size()));
}

void ListBox::ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination)
{
    if (MatchesOrContains(this, destination))
        return;

    std::vector<std::shared_ptr<Row>> initially_selected_rows;
    if (!(m_style & LIST_NOSEL) && !m_selections.empty()) {
        // save selections...
        for (const auto& sel : m_selections)
            initially_selected_rows.push_back(*sel);
        m_selections.clear();
    }

    // remove dragged-away row from this ListBox
    for (auto& wnd : wnds) {
        auto row = boost::polymorphic_downcast<Row*>(wnd);
        iterator row_it = std::find_if(m_rows.begin(), m_rows.end(),
                                       [&row](const std::shared_ptr<Row>& x){ return x.get() == row; });


        if (row_it == m_rows.end())
            continue;

        Erase(row_it, false, true);
    }

    if (!(m_style & LIST_NOSEL) && !initially_selected_rows.empty()) {
        // reselect any remaining from old selections
        SelectionSet new_selections;
        for (auto& row : initially_selected_rows) {
            iterator sel_it = std::find(m_rows.begin(), m_rows.end(), row);
            if (sel_it != m_rows.end())
                new_selections.insert(sel_it);
        }

        m_selections = new_selections;

        if (m_selections.size() != initially_selected_rows.size()) {
            SelRowsChangedSignal(m_selections);
        }
    }
}

void ListBox::PreRender()
{
    // Use the first row to define the column properties
    if (!m_rows.empty()
        && m_manage_column_props
        && (m_col_widths.empty() || !m_keep_col_widths))
    {
        DefineColWidths(*(*m_rows.begin()));
        DefineColAlignments(*(*m_rows.begin()));
        DefineColStretches(*(*m_rows.begin()));
    }

    if (m_normalize_rows_on_insert) {
        if (!m_header_row->empty() && !m_header_row->IsNormalized())
            NormalizeRow(m_header_row.get());
        for (auto& row : m_rows)
            if (!row->IsNormalized())
                NormalizeRow(row.get());
    }

    // Adding/removing scrolls and prerendering rows may change the row sizes and require a change
    // in added/removed scrolls. This may not be stable. Perform two cycles and if it is not
    // stable then force the scrollbar to be added if either cycle had a scroll bar.

    // Perform a cycle of adjust scrolls and prerendering rows and return if sizes changed.
    auto check_adjust_scroll_size_change = [this](std::pair<bool, bool> force_scrolls = {false, false}) {
        // This adjust scrolls may add or remove scrolls
        AdjustScrolls(true);

        bool visible_row_size_change = ShowVisibleRows(true);

        bool header_size_change = false;
        if (!m_header_row->empty()) {
            auto old_size = m_header_row->Size();
            GUI::PreRenderWindow(m_header_row.get());
            header_size_change |= (old_size != m_header_row->Size());
        }
        return visible_row_size_change | header_size_change;
    };

    // Try adjusting scroll twice and then force the scrolls on.
    if (check_adjust_scroll_size_change()) {
        bool any_vscroll = (m_vscroll != nullptr);
        bool any_hscroll = (m_hscroll != nullptr);

        if (check_adjust_scroll_size_change()) {
            any_vscroll |= (m_vscroll != nullptr);
            any_hscroll |= (m_hscroll != nullptr);
            check_adjust_scroll_size_change({any_hscroll, any_vscroll});
        }
    }

    // Reset require prerender after call to adjust scrolls
    Control::PreRender();

    // Position rows
    Pt pt(m_first_row_offset);
    for (auto& row : m_rows) {
        row->MoveTo(pt);
        pt.y += row->Height();
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

    // HACK! This gets around the issue of how to render headers and scrolls,
    // which do not fall within the client area.
    if (!m_header_row->empty()) {
        Rect header_area(Pt(ul.x + static_cast<int>(BORDER_THICK), m_header_row->Top()),
                         Pt(lr.x - static_cast<int>(BORDER_THICK), m_header_row->Bottom()));
        BeginScissorClipping(header_area.ul, header_area.lr);
        GUI::GetGUI()->RenderWindow(m_header_row.get());
        EndScissorClipping();
    }

    if (m_first_row_shown == m_rows.end())
        return;

    iterator last_visible_row = LastVisibleRow();

    BeginClipping();

    // draw selection hiliting
    Y top(0);
    Y bottom = (*m_first_row_shown)->Height();
    for (iterator curr_sel : m_selections) {
        if (RowAboveOrIsRow(m_first_row_shown, curr_sel, m_rows.end()) &&
            RowAboveOrIsRow(curr_sel, last_visible_row, m_rows.end()))
        {
            top = std::max((*curr_sel)->Top(), cl_ul.y);
            bottom = std::min((*curr_sel)->Top() + (*curr_sel)->Height(), cl_lr.y);
            FlatRectangle(Pt(cl_ul.x, top), Pt(cl_lr.x, bottom),
                          hilite_color_to_use, CLR_ZERO, 0);
        }
    }

    // draw caret
    if (m_caret != m_rows.end() &&
        RowAboveOrIsRow(m_first_row_shown, m_caret, m_rows.end()) &&
        RowAboveOrIsRow(m_caret, last_visible_row, m_rows.end()) &&
        MatchesOrContains(this, GUI::GetGUI()->FocusWnd().get()))
    {
        Pt row_ul = (*m_caret)->UpperLeft();
        Pt row_lr = (*m_caret)->LowerRight();
        row_lr.x = ClientLowerRight().x;
        FlatRectangle(row_ul, row_lr, CLR_ZERO, CLR_SHADOW, 2);
    }

    EndClipping();

    if (m_vscroll)
        GUI::GetGUI()->RenderWindow(m_vscroll.get());
    if (m_hscroll)
        GUI::GetGUI()->RenderWindow(m_hscroll.get());
}

void ListBox::SizeMove(const Pt& ul, const Pt& lr)
{
    const GG::Pt old_size = Size();
    Wnd::SizeMove(ul, lr);
    AdjustScrolls(old_size != Size());
    if (old_size != Size())
        RequirePreRender();
}

bool ListBox::ShowVisibleRows(bool do_prerender)
{
    bool a_row_size_changed = false;
    // Ensure that data in occluded cells is not rendered
    // and that any re-layout during prerender is immediate.
    Y visible_height(BORDER_THICK);
    Y max_visible_height = ClientSize().y;
    bool hide = true;
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        if (it == m_first_row_shown)
            hide = false;

        if (hide) {
            (*it)->Hide();
        } else {
            (*it)->Show();
            if (do_prerender) {
                auto old_size = (*it)->Size();
                GUI::PreRenderWindow(it->get());
                a_row_size_changed |= (old_size != (*it)->Size());
            }

            visible_height += (*it)->Height();
            if (visible_height >= max_visible_height)
                hide = true;
        }
    }

    return a_row_size_changed;
}

void ListBox::Show()
{
    Control::Show();

    // Deal with the header row and non row children normally
    for (auto& wnd : Children()) {
        const Row* row(dynamic_cast<Row*>(wnd.get()));
        bool is_regular_row(row && row != m_header_row.get());
        if (!is_regular_row)
            wnd->Show();
    }

    // Show rows that will be visible when rendered but don't prerender them.
    ShowVisibleRows(false);
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

ListBox::iterator ListBox::Insert(std::shared_ptr<Row> row, iterator it)
{ return Insert(std::forward<std::shared_ptr<Row>>(row), it, false); }

ListBox::iterator ListBox::Insert(std::shared_ptr<Row> row)
{ return Insert(std::forward<std::shared_ptr<Row>>(row), m_rows.end(), false); }

void ListBox::Insert(const std::vector<std::shared_ptr<Row>>& rows, iterator it)
{ Insert(rows, it, false); }

void ListBox::Insert(const std::vector<std::shared_ptr<Row>>& rows)
{ Insert(rows, m_rows.end(), false); }

std::shared_ptr<ListBox::Row> ListBox::Erase(iterator it, bool signal/* = false*/)
{ return Erase(it, false, signal); }

void ListBox::Clear()
{
    m_rows.clear();
    m_caret = m_rows.end();
    DetachChild(m_header_row.get());
    DetachChildren();
    AttachChild(m_header_row);
    m_first_row_offset = Pt(X(BORDER_THICK), Y(BORDER_THICK));
    m_first_row_shown = m_rows.end();
    m_first_col_shown = 0;
    m_selections.clear();
    m_old_sel_row = m_rows.end();
    m_old_rdown_row = m_rows.end();
    m_lclick_row = m_rows.end();
    m_rclick_row = m_rows.end();
    m_last_row_browsed = m_rows.end();

    if (!m_keep_col_widths) { // remove column widths and alignments, if needed
        m_col_widths.clear();
        m_col_alignments.clear();
        m_col_stretches.clear();

        if (m_manage_column_props)
            m_num_cols = 1;
    }

    DetachChildAndReset(m_vscroll);
    DetachChildAndReset(m_hscroll);

    RequirePreRender();
    ClearedRowsSignal();
}

void ListBox::SelectRow(iterator it, bool signal/* = false*/)
{
    if (m_style & LIST_NOSEL)
        return;
    if (it == m_rows.end())
        return;
    if (m_selections.count(it))
        return;

    SelectionSet previous_selections = m_selections;

    if (m_style & LIST_SINGLESEL)
        m_selections.clear();

    m_selections.insert(it);

    if (signal && previous_selections != m_selections)
        SelRowsChangedSignal(m_selections);
}

void ListBox::DeselectRow(iterator it, bool signal/* = false*/)
{
    SelectionSet previous_selections = m_selections;

    if (it == m_rows.end())  // always check that an iterator is valid before attempting a search for it
        return;
    if (m_selections.count(it))
        m_selections.erase(it);

    if (signal && previous_selections != m_selections)
        SelRowsChangedSignal(m_selections);
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
        SelRowsChangedSignal(m_selections);
}

void ListBox::DeselectAll(bool signal/* = false*/)
{
    SelectionSet previous_selections = m_selections;

    if (!m_selections.empty()) {
        m_selections.clear();
        m_caret = m_rows.end();
    }

    if (signal && previous_selections != m_selections)
        SelRowsChangedSignal(m_selections);
}

ListBox::iterator ListBox::begin()
{ return m_rows.begin(); }

ListBox::iterator ListBox::end()
{ return m_rows.end(); }

ListBox::Row& ListBox::GetRow(std::size_t n)
{
    assert(n < m_rows.size());
    return **std::next(m_rows.begin(), n);
}

void ListBox::SetSelections(const SelectionSet& s, bool signal/* = false*/)
{
    if (m_style & LIST_NOSEL)
        return;

    SelectionSet previous_selections = m_selections;

    m_selections = s;

    if (signal && previous_selections != m_selections)
        SelRowsChangedSignal(m_selections);
}

void ListBox::SetCaret(iterator it)
{ m_caret = it; }

void ListBox::BringRowIntoView(iterator target)
{
    if (target == m_rows.end())
        return;

    // m_first_row_shown only equals end() if the list is empty, hence 'it' is invalid.
    if (m_first_row_shown == m_rows.end())
        return;

    // Find the y offsets of the first and last shown rows and target.
    auto first_row_found(false);
    auto last_row_found(false);
    auto target_found(false);

    auto y_offset_top(Y0);
    auto y_offset_bot(Y0);
    auto target_y_offset(Y0);

    auto first_row_y_offset(Y0);
    auto last_row_y_offset(Y0);

    auto final_row = --m_rows.end();
    auto it = m_rows.begin();

    while ((it != m_rows.end()) && (!first_row_found || !last_row_found || !target_found)) {
        y_offset_bot += (*it)->Height();

        if (it == m_first_row_shown) {
            first_row_y_offset = y_offset_top;
            first_row_found = true;
        }

        if (it == target) {
            target_y_offset = y_offset_top;
            target_found = true;
        }

        if (first_row_found && !last_row_found
            && (((y_offset_bot - first_row_y_offset) >= ClientHeight())
                || it == final_row))
        {
            last_row_found = true;
            last_row_y_offset = y_offset_top;
        }

        y_offset_top = y_offset_bot;
        ++it;
    }

    if (!target_found)
        return;

    if (y_offset_bot <= ClientHeight())
        SetFirstRowShown(begin());

    // Shift the view if target is outside of [first_row .. last_row]
    if (target_y_offset < first_row_y_offset)
        SetFirstRowShown(target);
    else if (target_y_offset >= last_row_y_offset)
        SetFirstRowShown(FirstRowShownWhenBottomIs(target));
}

void ListBox::SetFirstRowShown(iterator it)
{
    if (it == m_rows.end() && !m_rows.empty())
        return;

    RequirePreRender();

    m_first_row_shown = it;

    AdjustScrolls(false);
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

void ListBox::SetColHeaders(const std::shared_ptr<Row>& r)
{
    Y client_height = ClientHeight();
    DetachChildAndReset(m_header_row);
    if (r) {
        m_header_row = r;
        // if this column header is being added to an empty listbox, the listbox takes on some of the
        // attributes of the header, similar to the insertion of a row into an empty listbox; see Insert()
        if (m_manage_column_props && m_rows.empty() && m_col_widths.empty()) {
            m_num_cols = m_header_row->size();
            m_col_widths.resize(m_header_row->size(),
                                ClientWidth() / static_cast<int>(m_header_row->size()));
            // put the remainder in the last column, so the total width == ClientWidth()
            m_col_widths.back() += ClientWidth() % static_cast<int>(m_header_row->size());
            m_col_alignments.resize(m_header_row->size(), AlignmentFromStyle(m_style));
            m_col_stretches.resize(m_header_row->size(), 0.0);
        }
        m_header_row->MoveTo(Pt(X0, -m_header_row->Height()));
        AttachChild(m_header_row);
    } else {
        m_header_row = Wnd::Create<Row>();
    }
    if (client_height != ClientHeight())
        AdjustScrolls(true);
}

void ListBox::RemoveColHeaders()
{ SetColHeaders(nullptr); }

void ListBox::SetNumCols(std::size_t n)
{
    assert(n);
    m_num_cols = n;
    if (m_manage_column_props) {
        if (m_col_widths.size()) {
            m_col_widths.resize(n);
            m_col_alignments.resize(n, ALIGN_NONE);
            m_col_stretches.resize(n, 0.0);
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
            m_col_stretches.resize(n, 0.0);
        }
    }

    if (n <= m_sort_col)
        m_sort_col = 0;

    RequirePreRender();
}

void ListBox::SetColWidth(std::size_t n, X w)
{
    if (m_num_cols < n + 1)
        m_num_cols = n + 1;
    if (m_col_widths.size() < n + 1)
        m_col_widths.resize(n + 1);

    m_col_widths[n] = w;
    for (auto& row : m_rows) {
        row->SetColWidth(n, w);
    }
    AdjustScrolls(false);
}

void ListBox::SetSortCol(std::size_t n)
{
    bool needs_resort = m_sort_col != n && !(m_style & LIST_NOSORT);
    if (m_num_cols < n + 1)
        m_num_cols = n + 1;

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
{
    m_manage_column_props = true;
    m_keep_col_widths = true;
}

void ListBox::UnLockColWidths()
{
    m_manage_column_props = true;
    m_keep_col_widths = false;
}

void ListBox::ManuallyManageColProps()
{ m_manage_column_props = false; }

void ListBox::SetColAlignment(std::size_t n, Alignment align)
{
    if (m_num_cols < n + 1)
        m_num_cols = n + 1;
    if (m_col_alignments.size() < n + 1)
        m_col_alignments.resize(n + 1);

    m_col_alignments[n] = align;
    for (auto& row : m_rows) {
        row->SetColAlignment(n, align);
    }
}

void ListBox::SetColStretch(std::size_t n, double x)
{
    if (m_num_cols < n + 1)
        m_num_cols = n + 1;
    if (m_col_stretches.size() < n + 1)
        m_col_stretches.resize(n + 1);

    m_col_stretches[n] = x;
    for (auto& row : m_rows) {
        auto&& layout = row->GetLayout();
        if (!layout)
            return;
        layout->SetColumnStretch(n, x);
    }
}

void ListBox::SetRowAlignment(iterator it, Alignment align)
{ (*it)->SetRowAlignment(align); }

void ListBox::NormalizeRowsOnInsert(bool enable)
{ m_normalize_rows_on_insert = enable; }

void ListBox::AddPaddingAtEnd(bool enable)
{ m_add_padding_at_end = enable; }

void ListBox::AllowDropType(const std::string& str)
{
    // Create the set if necessary
    if (!m_allowed_drop_types)
        m_allowed_drop_types = std::unordered_set<std::string>();
    m_allowed_drop_types->insert(str);
}

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
    if (!InClient(pt))
        return m_rows.end();

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

void ListBox::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        bool bring_caret_into_view = true;
        switch (key) {
        case GGK_SPACE: // space bar (selects item under caret like a mouse click)
            if (m_caret != m_rows.end()) {
                m_old_sel_row_selected = m_selections.count(m_caret);
                ClickAtRow(m_caret, mod_keys);
            }
            break;
        case GGK_DELETE: // delete key
            if (m_style & LIST_USERDELETE) {
                if (m_style & LIST_NOSEL) {
                    if (m_caret != m_rows.end())
                        Erase(m_caret, false, true);
                } else {
                    std::vector<iterator> prev_selections(m_selections.size());
                    std::copy(m_selections.begin(), m_selections.end(), prev_selections.begin());
                    m_selections.clear();
                    for (iterator it : prev_selections) {
                        Erase(it, false, true);
                    }
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
                while (m_caret != m_rows.begin() && 0 < (space -= (*std::prev(m_caret))->Height())) {
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
        case GGK_LEFT:{ // left key (not numpad key)
            if (m_first_col_shown == 0)
                break;

            --m_first_col_shown;
            auto&& first_row_first_child((*m_first_row_shown)->GetLayout()->Children().begin());
            auto first_shown_cell = *std::next(first_row_first_child, m_first_col_shown);
            GG::X new_scroll_offset(first_shown_cell->UpperLeft().x - UpperLeft().x - GG::X(BORDER_THICK));
            m_hscroll->ScrollTo(Value(new_scroll_offset));
            SignalScroll(*m_hscroll, true);
            break;}
        case GGK_RIGHT:{ // right key (not numpad)
            std::size_t num_cols((*m_first_row_shown)->GetLayout()->Children().size());
            if (num_cols <= 1)
                break;
            if (LastVisibleCol() >= (num_cols - 1))
                break;

            ++m_first_col_shown;
            auto&& first_row_first_child((*m_first_row_shown)->GetLayout()->Children().begin());
            auto first_shown_cell = *std::next(first_row_first_child, m_first_col_shown);
            GG::X new_scroll_offset(first_shown_cell->UpperLeft().x - UpperLeft().x - GG::X(BORDER_THICK));
            m_hscroll->ScrollTo(Value(new_scroll_offset));
            SignalScroll(*m_hscroll, true);
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

void ListBox::DragDropEnter(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys)
{
    ResetAutoScrollVars();
    DragDropHere(pt, drop_wnds_acceptable, mod_keys);
}

void ListBox::DragDropHere(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys)
{
    this->DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);

    if (m_rows.empty() || !m_auto_scroll_during_drag_drops || !InClient(pt))
        return;

    const Pt MARGIN_OFFSET = Pt(X(m_auto_scroll_margin), Y(m_auto_scroll_margin));
    Rect client_no_scroll_hole(ClientUpperLeft() + MARGIN_OFFSET, ClientLowerRight() - MARGIN_OFFSET);
    m_auto_scrolling_up = pt.y < client_no_scroll_hole.ul.y;
    m_auto_scrolling_down = client_no_scroll_hole.lr.y < pt.y;
    m_auto_scrolling_left = pt.x < client_no_scroll_hole.ul.x;
    m_auto_scrolling_right = client_no_scroll_hole.lr.x < pt.x;
    if (m_auto_scrolling_up || m_auto_scrolling_down || m_auto_scrolling_left || m_auto_scrolling_right) {
        bool acceptable_drop = false;
        for (auto& acceptable_wnd : drop_wnds_acceptable) {
            if (AllowedDropType(acceptable_wnd.first->DragDropDataType())) {
                acceptable_drop = true;
                break;
            }
        }
        if (acceptable_drop) {
            if (!m_auto_scroll_timer.Running()) {
                m_auto_scroll_timer.Reset(GUI::GetGUI()->Ticks());
                m_auto_scroll_timer.Start();
            }
        } else {
            DragDropLeave();
        }
    }
}

void ListBox::DragDropLeave()
{ ResetAutoScrollVars(); }

void ListBox::CancellingChildDragDrop(const std::vector<const Wnd*>& wnds)
{ ResetAutoScrollVars(); }

void ListBox::TimerFiring(unsigned int ticks, Timer* timer)
{
    if (timer == &m_auto_scroll_timer && !m_rows.empty()) {
        if (m_vscroll) {
            if (m_auto_scrolling_up &&
                m_first_row_shown != m_rows.end() &&
                m_first_row_shown != m_rows.begin()) {
                m_vscroll->ScrollTo(m_vscroll->PosnRange().first -
                                    Value((*std::prev(m_first_row_shown))->Height()));
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
        if (m_old_sel_row != m_rows.end()) {
            m_old_sel_row_selected = m_selections.count(m_old_sel_row);
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
        if (m_old_sel_row != m_rows.end()) {
            iterator sel_row = RowUnderPt(pt);
            if (sel_row == m_old_sel_row) {
                if (m_style & LIST_NOSEL)
                    m_caret = sel_row;
                else
                    ClickAtRow(sel_row, mod_keys);
                m_lclick_row = sel_row;
                LeftClickedRowSignal(sel_row, pt, mod_keys);
            }
        }
        break;
    }

    case WndEvent::LDoubleClick: {
        iterator row = RowUnderPt(pt);
        if (row != m_rows.end() && row == m_lclick_row) {
            DoubleClickedRowSignal(row, pt, mod_keys);
            m_old_sel_row = m_rows.end();
        } else {
            LClick(pt, mod_keys);
        }
        break;
    }

    case WndEvent::RButtonDown: {
        iterator row = RowUnderPt(pt);
        if (row != m_rows.end())
            m_old_rdown_row = row;
        else
            m_old_rdown_row = m_rows.end();
        break;
    }

    case WndEvent::RClick: {
        iterator row = RowUnderPt(pt);
        if (row != m_rows.end() && row == m_old_rdown_row) {
            m_rclick_row = row;
            RightClickedRowSignal(row, pt, mod_keys);
        }
        m_old_rdown_row = m_rows.end();
        break;
    }

    case WndEvent::MouseEnter: {
        if (m_style & LIST_BROWSEUPDATES) {
            iterator sel_row = RowUnderPt(pt);
            if (sel_row != m_rows.end() && m_last_row_browsed != sel_row)
                BrowsedRowSignal(m_last_row_browsed = sel_row);
        }
        break;
    }

    case WndEvent::MouseHere:
        break;

    case WndEvent::MouseLeave: {
        if (m_style & LIST_BROWSEUPDATES) {
            if (m_last_row_browsed != m_rows.end())
                BrowsedRowSignal(m_last_row_browsed = m_rows.end());
        }
        break;
    }

    case WndEvent::GainingFocus: {
        if (w == this)
            return false;
        GUI::GetGUI()->SetFocusWnd(shared_from_this());
        break;
    }

    case WndEvent::MouseWheel:
        return false;

    case WndEvent::DragDropEnter:
    case WndEvent::DragDropHere:
    case WndEvent::CheckDrops:
    case WndEvent::DragDropLeave:
    case WndEvent::DragDroppedOn:
        if (w == this)
            return false;
        //std::cout << "ListBox::EventFilter of type: " << EventTypeName(event) << std::endl << std::flush;
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

void ListBox::DefineColWidths(const Row& row)
{
    const GG::X WIDTH = ClientSize().x - SCROLL_WIDTH;

    m_col_widths.resize(row.size());
    GG::X total_width = GG::X0;
    for (std::size_t i = 0; i < row.size(); ++i) {
        // use the column width from the Row
        total_width += row.ColWidth(i);
    }

    const GG::X_d SCALE_FACTOR = 1.0 * WIDTH / total_width;

    GG::X total_scaled_width = GG::X0;
    for (std::size_t i = 0; i < row.size(); ++i) {
        total_scaled_width += (m_col_widths[i] = row.ColWidth(i) * SCALE_FACTOR);
    }
    m_col_widths.back() += total_scaled_width - WIDTH;
}

void ListBox::DefineColAlignments(const Row& row)
{
    m_col_alignments.resize(row.size());
    for (std::size_t i = 0; i < row.size(); ++i) {
        // use the column alignment from the Row, if it has been set;
        // otherwise, use the one dictated by the ListBoxStyle flags
        Alignment a = row.ColAlignment(i);
        if (a == ALIGN_NONE)
            a = AlignmentFromStyle(m_style);
        m_col_alignments[i] = a;
    }
}

void ListBox::DefineColStretches(const Row& row)
{
    auto&& layout = GetLayout();
    if (!layout)
        return;

    m_col_stretches.resize(row.size());
    for (std::size_t i = 0; i < row.size(); ++i) {
        m_col_stretches[i] = layout->ColumnStretch (i);
    }
}

ListBox::iterator ListBox::Insert(std::shared_ptr<Row> row, iterator it, bool dropped)
{
    if(!row)
        return m_rows.end();

    // Track the originating row in case this is an intra-ListBox
    // drag-and-drop.
    iterator original_dropped_position = m_rows.end();
    if (dropped)
        original_dropped_position = std::find(m_rows.begin(), m_rows.end(), row);

    bool moved = (original_dropped_position != m_rows.end());
    std::size_t original_dropped_offset = moved ? std::distance(begin(), original_dropped_position) :0;

    iterator retval = it;

    row->InstallEventFilter(shared_from_this());

    BeforeInsertRowSignal(it);

    if (m_rows.empty()) {
        m_rows.push_back(row);
        retval = m_rows.begin();
    } else {
        if (!(m_style & LIST_NOSORT)) {
            retval = m_rows.begin();
            RowSorter cmp(m_sort_cmp, m_sort_col, m_style & LIST_SORTDESCENDING);
            while (retval != m_rows.end() && !cmp(row.get(), retval->get())) {
                ++retval;
            }
        }
        retval = m_rows.insert(retval, row);
    }

    AttachChild(row);

    if (m_first_row_shown == m_rows.end())
        m_first_row_shown = m_rows.begin();

    if (dropped && moved) {
        Erase(original_dropped_position, true, false);
        // keep pointing at the same offset as original location after the erase
        original_dropped_position = begin();
        std::advance(original_dropped_position, original_dropped_offset);
    }

    row->Hide();

    row->Resize(Pt(std::max(ClientWidth(), X(1)), row->Height()));

    row->RightClickedSignal.connect(boost::bind(&ListBox::HandleRowRightClicked, this, _1, _2));

    AfterInsertRowSignal(it);
    if (dropped)
        DroppedRowSignal(retval);
    if (moved)
        MovedRowSignal(retval, original_dropped_position);

    RequirePreRender();
    return retval;
}

void ListBox::Insert(const std::vector<std::shared_ptr<Row>>& rows, iterator it, bool dropped)
{
    for (auto& row : rows)
    { Insert(row, it, dropped); }
}

std::shared_ptr<ListBox::Row> ListBox::Erase(iterator it, bool removing_duplicate, bool signal)
{
    if (it == m_rows.end())
        return nullptr;

    RequirePreRender();

    auto row = *it;
    if (!removing_duplicate) {
        DetachChild(row.get());
        row->RemoveEventFilter(shared_from_this());
    }

    ResetIfEqual(m_old_sel_row,     it, m_rows.end());
    ResetIfEqual(m_old_rdown_row,   it, m_rows.end());
    ResetIfEqual(m_lclick_row,      it, m_rows.end());
    ResetIfEqual(m_rclick_row,      it, m_rows.end());
    ResetIfEqual(m_last_row_browsed,it, m_rows.end());

    bool check_first_row_and_caret_for_end = false;
    if (m_first_row_shown == it) {
        ++m_first_row_shown;
        check_first_row_and_caret_for_end = true;
    }
    if (m_caret == it) {
        ++m_caret;
        check_first_row_and_caret_for_end = true;
    }

    // remove row from selections and contained rows.
    m_selections.erase(it);
    m_rows.erase(it);

    if (check_first_row_and_caret_for_end && m_first_row_shown == m_rows.end() && !m_rows.empty())
        --m_first_row_shown;
    if (check_first_row_and_caret_for_end && m_caret == m_rows.end() && !m_rows.empty())
        --m_caret;

    return row;
}

void ListBox::BringCaretIntoView()
{ BringRowIntoView(m_caret); }

void ListBox::ResetAutoScrollVars()
{
    m_auto_scrolling_up = false;
    m_auto_scrolling_down = false;
    m_auto_scrolling_left = false;
    m_auto_scrolling_right = false;
    m_auto_scroll_timer.Stop();
}

struct ListBox::SelectionCache
{
    std::set<std::shared_ptr<Row>> selections;
    std::shared_ptr<Row> caret;
    std::shared_ptr<Row> old_sel_row;
    std::shared_ptr<Row> old_rdown_row;
    std::shared_ptr<Row> lclick_row;
    std::shared_ptr<Row> rclick_row;
    std::shared_ptr<Row> last_row_browsed;
};

// TODO: change to unique_ptr with move mechanics or more the entire definition into the cpp file.
std::shared_ptr<ListBox::SelectionCache> ListBox::CacheSelections()
{
    auto cache = std::make_shared<ListBox::SelectionCache>();
    cache->caret = IteratorToShared(m_caret, m_rows.end());
    for (const auto& sel : m_selections) {
        cache->selections.insert(*sel);
    }
    cache->old_sel_row =      IteratorToShared(m_old_sel_row, m_rows.end());
    cache->old_rdown_row =    IteratorToShared(m_old_rdown_row, m_rows.end());
    cache->lclick_row =       IteratorToShared(m_lclick_row, m_rows.end());
    cache->rclick_row =       IteratorToShared(m_rclick_row, m_rows.end());
    cache->last_row_browsed = IteratorToShared(m_last_row_browsed, m_rows.end());

    m_selections.clear();

    return cache;
}

void ListBox::RestoreCachedSelections(const ListBox::SelectionCache& cache)
{
    m_selections.clear();

    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        auto row = *it;
        if (cache.caret == row)
            m_caret = it;
        if (cache.selections.count(row))
            m_selections.insert(it);
        if (cache.old_sel_row == row)
            m_old_sel_row = it;
        if (cache.old_rdown_row == row)
            m_old_rdown_row = it;
        if (cache.lclick_row == row)
            m_lclick_row = it;
        if (cache.rclick_row == row)
            m_rclick_row = it;
        if (cache.last_row_browsed == row)
            m_last_row_browsed = it;
    }
}

void ListBox::Resort()
{
    std::shared_ptr<ListBox::SelectionCache> cached_selections = CacheSelections();

    std::vector<std::shared_ptr<Row>> rows_vec(m_rows.size());
    std::copy(m_rows.begin(), m_rows.end(), rows_vec.begin());
    std::stable_sort(rows_vec.begin(), rows_vec.end(),
                     RowSorter(m_sort_cmp, m_sort_col, m_style & LIST_SORTDESCENDING));
    m_rows.clear();
    m_rows.insert(m_rows.begin(), rows_vec.begin(), rows_vec.end());

    RequirePreRender();

    RestoreCachedSelections(*cached_selections);

    m_first_row_shown = m_rows.empty() ? m_rows.end() : m_rows.begin();
    SetFirstRowShown(m_first_row_shown);
}

ListBox::Row& ListBox::ColHeaders()
{ return *m_header_row; }

void ListBox::ConnectSignals()
{
    if (m_vscroll)
        m_vscroll->ScrolledSignal.connect(boost::bind(&ListBox::VScrolled, this, _1, _2, _3, _4));
    if (m_hscroll)
        m_hscroll->ScrolledSignal.connect(boost::bind(&ListBox::HScrolled, this, _1, _2, _3, _4));
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

Pt ListBox::ClientSizeExcludingScrolls() const
{
    // This client area calculation is used to determine if scroll should/should not be added, so
    // it does not include the thickness of scrolls.
    Pt cl_sz = (LowerRight()
                - Pt(X(BORDER_THICK), Y(BORDER_THICK))
                - UpperLeft()
                - Pt(X(BORDER_THICK),
                     static_cast<int>(BORDER_THICK)
                     + (m_header_row->empty() ? Y0 : m_header_row->Height())));
    return cl_sz;
}

std::pair<boost::optional<X>, boost::optional<Y>> ListBox::CheckIfScrollsRequired(
    const std::pair<bool, bool>& force_scrolls,
    const boost::optional<Pt>& maybe_client_size) const
{
    // Use the precalculated client size if possible.
    auto cl_sz = maybe_client_size ? *maybe_client_size : ClientSizeExcludingScrolls();

    X total_x_extent = std::accumulate(m_col_widths.begin(), m_col_widths.end(), X0);
    Y total_y_extent(0);
    for (auto& row : m_rows)
        total_y_extent += row->Height();

    bool vertical_needed =
        force_scrolls.second ||
        m_first_row_shown != m_rows.begin() ||
        (m_rows.size() && (cl_sz.y < total_y_extent ||
                           (cl_sz.y < total_y_extent - SCROLL_WIDTH &&
                            cl_sz.x < total_x_extent - SCROLL_WIDTH)));
    bool horizontal_needed =
        force_scrolls.first ||
        m_first_col_shown ||
        (m_rows.size() && (cl_sz.x < total_x_extent ||
                           (cl_sz.x < total_x_extent - SCROLL_WIDTH &&
                            cl_sz.y < total_y_extent - SCROLL_WIDTH)));

    if (m_add_padding_at_end) {
        // This probably looks a little odd. We only want to show scrolls if they
        // are needed, that is if the data shown exceed the bounds of the client
        // area. However, if we are going to show scrolls, we want to allow them
        // to range such that the first row/column shown can be any of the N
        // rows/columns. This is necessary since otherwise the bottom row may get
        // cut off. Dead space after the last row/column is the result, even if it
        // may look slightly ugly.
        if (!m_col_widths.empty() && m_col_widths.back() < cl_sz.x)
            total_x_extent += cl_sz.x - m_col_widths.back();
        if (!m_rows.empty() && m_rows.back()->Height() < cl_sz.y)
            total_y_extent += cl_sz.y - m_rows.back()->Height();
    }

    boost::optional<X> x_retval = horizontal_needed ? boost::optional<X>(total_x_extent) : boost::none;
    boost::optional<Y> y_retval = vertical_needed   ? boost::optional<Y>(total_y_extent) : boost::none;

    return {x_retval, y_retval};
}

std::pair<bool, bool> ListBox::AddOrRemoveScrolls(
    const std::pair<boost::optional<X>, boost::optional<Y>>& required_total_extents,
    const boost::optional<Pt>& maybe_client_size)
{
    // Use the precalculated client size if possible.
    auto cl_sz = maybe_client_size ? *maybe_client_size : ClientSizeExcludingScrolls();

    const auto& style = GetStyleFactory();

    bool horizontal_needed = (required_total_extents.first ? true : false);
    bool vertical_needed = (required_total_extents.second ? true : false);

    bool vscroll_added_or_removed = false;

    // Remove unecessary vscroll
    if (m_vscroll && !vertical_needed) {
        vscroll_added_or_removed = true;

        //Scroll back to zero
        m_vscroll->ScrollTo(0);
        SignalScroll(*m_vscroll, true);

        DetachChildAndReset(m_vscroll);
    }

    // Add necessary vscroll
    if (!m_vscroll && vertical_needed) {
        vscroll_added_or_removed = true;
        m_vscroll = style->NewListBoxVScroll(m_color, CLR_SHADOW);
        m_vscroll->NonClientChild(true);
        m_vscroll->MoveTo(Pt(cl_sz.x - SCROLL_WIDTH, Y0));
        m_vscroll->Resize(Pt(X(SCROLL_WIDTH), cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0)));

        AttachChild(m_vscroll);
        m_vscroll->ScrolledSignal.connect(boost::bind(&ListBox::VScrolled, this, _1, _2, _3, _4));
    }

    if (vertical_needed) {
        unsigned int line_size = m_vscroll_wheel_scroll_increment;
        if (line_size == 0 && !this->Empty()) {
            const auto& row = *begin();
            line_size = Value(row->Height());
        }

        unsigned int page_size = std::abs(Value(cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0)));

        m_vscroll->SizeScroll(0, Value(*required_total_extents.second - 1),
                              line_size, std::max(line_size, page_size));

        MoveChildUp(m_vscroll.get());

        // Scroll to the correct location
        Y acc(0);
        for (iterator it2 = m_rows.begin(); it2 != m_first_row_shown; ++it2)
            acc += (*it2)->Height();
        m_vscroll->ScrollTo(Value(acc));
        SignalScroll(*m_vscroll, true);
    }

    bool hscroll_added_or_removed = false;

    // Remove unecessary hscroll
    if (m_hscroll && !horizontal_needed) {
        hscroll_added_or_removed = true;

        //Scroll back to zero
        m_hscroll->ScrollTo(0);
        SignalScroll(*m_hscroll, true);

        DetachChild(m_hscroll.get());
        m_hscroll = nullptr;
    }

    // Add necessary hscroll
    if (!m_hscroll && horizontal_needed) {
        hscroll_added_or_removed = true;
        m_hscroll = style->NewListBoxHScroll(m_color, CLR_SHADOW);
        m_hscroll->NonClientChild(true);
        m_hscroll->MoveTo(Pt(X0, cl_sz.y - SCROLL_WIDTH));
        m_hscroll->Resize(Pt(cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0), Y(SCROLL_WIDTH)));

        AttachChild(m_hscroll);
        m_hscroll->ScrolledSignal.connect(boost::bind(&ListBox::HScrolled, this, _1, _2, _3, _4));
    }

    if (horizontal_needed) {
        unsigned int line_size = m_hscroll_wheel_scroll_increment;
        if (line_size == 0 && !this->Empty()) {
            const auto& row = *begin();
            line_size = Value(row->Height());
        }

        unsigned int page_size = std::abs(Value(cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0)));

        m_hscroll->SizeScroll(0, Value(*required_total_extents.first - 1),
                              line_size, std::max(line_size, page_size));
        MoveChildUp(m_hscroll.get());
    }

    return {hscroll_added_or_removed, vscroll_added_or_removed};
}

void ListBox::AdjustScrolls(bool adjust_for_resize, const std::pair<bool, bool>& force_scrolls)
{
    // The client size before scrolls are/are not added.
    const Pt cl_sz = ClientSizeExcludingScrolls();

    // The size of the underlying list box, indicating if scrolls are required.
    const auto required_total_extents = CheckIfScrollsRequired(force_scrolls, cl_sz);

    bool vscroll_added_or_removed;
    std::tie(std::ignore,  vscroll_added_or_removed) = AddOrRemoveScrolls(required_total_extents, cl_sz);

    if (!adjust_for_resize)
        return;

    if (m_vscroll) {
        X scroll_x = cl_sz.x - SCROLL_WIDTH;
        Y scroll_y(0);
        m_vscroll->SizeMove(Pt(scroll_x, scroll_y),
                            Pt(scroll_x + SCROLL_WIDTH,
                               scroll_y + cl_sz.y - (m_hscroll ? SCROLL_WIDTH : 0)));
    }

    if (m_hscroll) {
        X scroll_x(0);
        Y scroll_y = cl_sz.y - SCROLL_WIDTH;
        m_hscroll->SizeMove(Pt(scroll_x, scroll_y),
                            Pt(scroll_x + cl_sz.x - (m_vscroll ? SCROLL_WIDTH : 0),
                               scroll_y + SCROLL_WIDTH));
    }

    // Resize rows to fit client area.
    if (vscroll_added_or_removed || adjust_for_resize) {
        RequirePreRender();
        X row_width(std::max(ClientWidth(), X(1)));
        for (auto& row : m_rows) {
            row->Resize(Pt(row_width, row->Height()));
        }
    }
}

void ListBox::VScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_row_shown = m_rows.empty() ? m_rows.end() : m_rows.begin();
    Y position(BORDER_THICK);

    // scan through list of rows until the tab position is less than one of the rows' centres
    for (iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        // first shown row is at least the current row, and position is at least the top of the current row
        m_first_row_shown = it;
        Y row_height = (*it)->Height();

        // if this is the last row, abort
        iterator next_it = it;  ++next_it;
        if (next_it == m_rows.end())
            break;

        // current row is too far for the tab position to be moved to its end. current row remains the first one shown.
        if (tab_low < (-position) + row_height / 2)
            break;

        // position is at least at the bottom of the current row
        position = position - row_height;
    }

    if (position != m_first_row_offset.y)
        RequirePreRender();

    m_first_row_offset.y = position;

}

void ListBox::HScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_col_shown = 0;
    X accum(BORDER_THICK);
    X position(BORDER_THICK);
    for (std::size_t i = 0; i < m_col_widths.size(); ++i) {
        X col_width = m_col_widths[i];
        if (tab_low < accum + col_width / 2) {
            m_first_col_shown = i;
            position = -accum;
            break;
        }
        accum += col_width;
    }

    m_first_row_offset.x = position;
    RequirePreRender();
}

void ListBox::ClickAtRow(iterator it, Flags<ModKey> mod_keys)
{
    if (it == m_rows.end())
        return;
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
                iterator low  = RowPtrIteratorLess()(m_caret, it) ? m_caret : it;
                iterator high = RowPtrIteratorLess()(m_caret, it) ? it : m_caret;

                bool erase = !m_selections.count(m_caret);
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
            bool erase = m_caret != m_rows.end() && !m_selections.count(m_caret);
            if (!(m_style & LIST_QUICKSEL))
                m_selections.clear();
            if (m_caret == m_rows.end()) {
                // No previous caret exists; mark the first row as the caret.
                m_caret = m_rows.begin();
            } 
            // select all rows between the caret and this row (inclusive), don't move the caret
            iterator low  = RowPtrIteratorLess()(m_caret, it) ? m_caret : it;
            iterator high = RowPtrIteratorLess()(m_caret, it) ? it : m_caret;
            if (high != m_rows.end())
                ++high;
            for (iterator it2 = low; it2 != high; ++it2) {
                if (erase)
                    m_selections.erase(it2);
                else
                    m_selections.insert(it2);
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
        SelRowsChangedSignal(m_selections);
}

void ListBox::NormalizeRow(Row* row)
{
    assert(m_num_cols);
    row->SetMargin(m_cell_margin);
    row->resize(m_num_cols);
    row->SetColWidths(m_col_widths);
    row->SetColAlignments(m_col_alignments);
    row->SetColStretches(m_col_stretches);
    row->Resize(Pt(ClientWidth(), row->Height()));

    // Normalize row is only called during prerender.
    GUI::PreRenderWindow(row);
}

ListBox::iterator ListBox::FirstRowShownWhenBottomIs(iterator bottom_row)
{
    Y available_space = ClientHeight() - (*bottom_row)->Height();
    iterator it = bottom_row;
    while (it != m_rows.begin() && (*std::prev(it))->Height() <= available_space) {
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
