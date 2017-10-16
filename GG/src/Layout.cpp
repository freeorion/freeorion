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

#include <GG/Layout.h>

#include <GG/ClrConstants.h>
#include <GG/DrawUtil.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>

#include <cassert>
#include <cmath>

using namespace GG;

namespace {
    unsigned int MinDueToMargin(unsigned int cell_margin, std::size_t num_rows_or_columns, std::size_t row_or_column)
    {
        return (row_or_column == 0 || row_or_column == num_rows_or_columns - 1) ?
            static_cast<unsigned int>(std::ceil(cell_margin / 2.0)) :
            cell_margin;
    }
}

// RowColParams
Layout::RowColParams::RowColParams() :
    stretch(0),
    min(0),
    effective_min(0),
    current_origin(0),
    current_width(0)
{}

// WndPosition
Layout::WndPosition::WndPosition() :
    first_row(0),
    first_column(0),
    last_row(0),
    last_column(0),
    alignment(ALIGN_NONE)
{}

Layout::WndPosition::WndPosition(std::size_t first_row_, std::size_t first_column_,
                                 std::size_t last_row_, std::size_t last_column_,
                                 Flags<Alignment> alignment_, const Pt& original_ul_, const Pt& original_size_) :
    first_row(first_row_),
    first_column(first_column_),
    last_row(last_row_),
    last_column(last_column_),
    alignment(alignment_),
    original_ul(original_ul_),
    original_size(original_size_)
{}

// Layout
const unsigned int Layout::INVALID_CELL_MARGIN = std::numeric_limits<unsigned int>::max();

Layout::Layout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
               unsigned int border_margin/* = 0*/, unsigned int cell_margin/* = INVALID_CELL_MARGIN*/) :
    Wnd(x, y, w, h, NO_WND_FLAGS),
    m_cells(rows, std::vector<std::weak_ptr<Wnd>>(columns)),
    m_border_margin(border_margin),
    m_cell_margin(cell_margin == INVALID_CELL_MARGIN ? border_margin : cell_margin),
    m_row_params(rows),
    m_column_params(columns),
    m_ignore_child_resize(false),
    m_stop_resize_recursion(false),
    m_render_outline(false),
    m_outline_color(CLR_MAGENTA)
{
    assert(rows);
    assert(columns);
}

Pt Layout::MinUsableSize() const
{ return m_min_usable_size; }

std::size_t Layout::Rows() const
{ return m_cells.size(); }

std::size_t Layout::Columns() const
{ return m_cells.empty() ? 0 : m_cells[0].size(); }

Flags<Alignment> Layout::ChildAlignment(const Wnd* wnd) const
{
    auto it = m_wnd_positions.find(const_cast<Wnd*>(wnd));
    if (it == m_wnd_positions.end())
        throw NoSuchChild("Layout::ChildAlignment() : Alignment of a nonexistent child was requested");
    return it->second.alignment;
}

unsigned int Layout::BorderMargin() const
{ return m_border_margin; }

unsigned int Layout::CellMargin() const
{ return m_cell_margin; }

double Layout::RowStretch(std::size_t row) const
{ return m_row_params[row].stretch; }

double Layout::ColumnStretch(std::size_t column) const
{ return m_column_params[column].stretch; }

Y Layout::MinimumRowHeight(std::size_t row) const
{ return Y(m_row_params[row].min); }

X Layout::MinimumColumnWidth(std::size_t column) const
{ return X(m_column_params[column].min); }

std::vector<std::vector<const Wnd*>> Layout::Cells() const
{
    std::vector<std::vector<const Wnd*>> retval(m_cells.size());
    for (std::size_t i = 0; i < m_cells.size(); ++i) {
        retval[i].resize(m_cells[i].size());
        for (std::size_t j = 0; j < m_cells[i].size(); ++j) {
            retval[i][j] = m_cells[i][j].lock().get();
        }
    }    
    return retval;
}

std::vector<std::vector<Rect>> Layout::CellRects() const
{
    std::vector<std::vector<Rect>> retval = RelativeCellRects();
    for (std::vector<Rect>& column : retval) {
        for (Rect& cell : column) {
            cell += ClientUpperLeft();
        }
    }
    return retval;
}

std::vector<std::vector<Rect>> Layout::RelativeCellRects() const
{
    std::vector<std::vector<Rect>> retval(m_cells.size());
    for (std::size_t i = 0; i < m_cells.size(); ++i) {
        retval[i].resize(m_cells[i].size());
        for (std::size_t j = 0; j < m_cells[i].size(); ++j) {
            Pt ul(X(m_column_params[j].current_origin),
                  Y(m_row_params[i].current_origin));
            Pt lr = ul + Pt(X(m_column_params[j].current_width),
                            Y(m_row_params[i].current_width));
            Rect rect(ul, lr);
            if (j)
                rect.ul.x += static_cast<int>(m_cell_margin / 2);
            if (j != m_cells[i].size() - 1)
                rect.lr.x -= static_cast<int>(m_cell_margin - m_cell_margin / 2);
            if (i)
                rect.ul.y += static_cast<int>(m_cell_margin / 2);
            if (i != m_cells.size() - 1)
                rect.lr.y -= static_cast<int>(m_cell_margin - m_cell_margin / 2);
            retval[i][j] = rect;
        }
    }
    return retval;
}

bool Layout::RenderOutline() const
{ return m_render_outline; }

Clr Layout::OutlineColor() const
{ return m_outline_color; }

void Layout::StartingChildDragDrop(const Wnd* wnd, const Pt& offset)
{
    if (auto&& parent = Parent())
        parent->StartingChildDragDrop(wnd, offset);
}

void Layout::CancellingChildDragDrop(const std::vector<const Wnd*>& wnds)
{
    if (auto&& parent = Parent())
        parent->CancellingChildDragDrop(wnds);
}

void Layout::ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination)
{
    if (auto&& parent = Parent())
        parent->ChildrenDraggedAway(wnds, destination);
}

void Layout::SizeMove(const Pt& ul, const Pt& lr)
{ DoLayout(ul, lr); }

void Layout::DoLayout(Pt ul, Pt lr)
{
    if (m_stop_resize_recursion)
        return;

    // these hold values used to calculate m_min_usable_size
    std::vector<unsigned int> row_effective_min_usable_sizes(m_row_params.size());
    std::vector<unsigned int> column_effective_min_usable_sizes(m_column_params.size());

    // reset effective_min values
    for (std::size_t i = 0; i < m_row_params.size(); ++i) {
        unsigned int min_due_to_margin = MinDueToMargin(m_cell_margin, m_row_params.size(), i);
        m_row_params[i].effective_min = std::max(m_row_params[i].min, min_due_to_margin);
        row_effective_min_usable_sizes[i] = min_due_to_margin;
    }

    for (std::size_t i = 0; i < m_column_params.size(); ++i) {
        unsigned int min_due_to_margin = MinDueToMargin(m_cell_margin, m_column_params.size(), i);
        m_column_params[i].effective_min = std::max(m_column_params[i].min, min_due_to_margin);
        column_effective_min_usable_sizes[i] = min_due_to_margin;
    }

    // adjust effective minimums based on cell contents
    for (const auto& wnd_position : m_wnd_positions) {
        Pt margin;
        if (0 < wnd_position.second.first_row && wnd_position.second.last_row < m_row_params.size())
            margin.y = Y(m_cell_margin);
        else if (0 < wnd_position.second.first_row || wnd_position.second.last_row < m_row_params.size())
            margin.y = Y(static_cast<int>(std::ceil(m_cell_margin / 2.0)));
        if (0 < wnd_position.second.first_column && wnd_position.second.last_column < m_column_params.size())
            margin.x = X(m_cell_margin);
        else if (0 < wnd_position.second.first_column || wnd_position.second.last_column < m_column_params.size())
            margin.x = X(static_cast<int>(std::ceil(m_cell_margin / 2.0)));

        Pt min_space_needed = wnd_position.first->MinSize() + margin;
        Pt min_usable_size = wnd_position.first->MinUsableSize() + margin;

        // HACK! This is put here so that TextControl, which is currently GG's
        // only height-for-width Wnd type, doesn't get vertically squashed
        // down to 0-height cells.  Note that they can still get horizontally
        // squashed.
        if (TextControl* text_control = dynamic_cast<TextControl*>(wnd_position.first)) {
            min_space_needed.y = (text_control->MinUsableSize(Width()) + margin).y;
            min_usable_size.y = min_space_needed.y;
        }

        // adjust row minimums
        double total_stretch = 0.0;
        for (std::size_t i = wnd_position.second.first_row; i < wnd_position.second.last_row; ++i) {
            total_stretch += m_row_params[i].stretch;
        }
        if (total_stretch) {
            for (std::size_t i = wnd_position.second.first_row; i < wnd_position.second.last_row; ++i) {
                m_row_params[i].effective_min = std::max(m_row_params[i].effective_min, static_cast<unsigned int>(Value(min_space_needed.y / total_stretch * m_row_params[i].stretch)));
                row_effective_min_usable_sizes[i] = std::max(row_effective_min_usable_sizes[i], static_cast<unsigned int>(Value(min_usable_size.y / total_stretch * m_row_params[i].stretch)));
            }
        } else { // if all rows have 0.0 stretch, distribute height evenly
            double per_row_min = Value(min_space_needed.y / static_cast<double>(wnd_position.second.last_row - wnd_position.second.first_row));
            double per_row_usable_min = Value(min_usable_size.y / static_cast<double>(wnd_position.second.last_row - wnd_position.second.first_row));
            for (std::size_t i = wnd_position.second.first_row; i < wnd_position.second.last_row; ++i) {
                m_row_params[i].effective_min = std::max(m_row_params[i].effective_min, static_cast<unsigned int>(per_row_min + 0.5));
                row_effective_min_usable_sizes[i] = std::max(row_effective_min_usable_sizes[i], static_cast<unsigned int>(per_row_usable_min + 0.5));
            }
        }

        // adjust column minimums
        total_stretch = 0.0;
        for (std::size_t i = wnd_position.second.first_column; i < wnd_position.second.last_column; ++i) {
            total_stretch += m_column_params[i].stretch;
        }
        if (total_stretch) {
            for (std::size_t i = wnd_position.second.first_column; i < wnd_position.second.last_column; ++i) {
                m_column_params[i].effective_min = std::max(m_column_params[i].effective_min, static_cast<unsigned int>(Value(min_space_needed.x / total_stretch * m_column_params[i].stretch)));
                column_effective_min_usable_sizes[i] = std::max(column_effective_min_usable_sizes[i], static_cast<unsigned int>(Value(min_usable_size.x / total_stretch * m_column_params[i].stretch)));
            }
        } else { // if all columns have 0.0 stretch, distribute width evenly
            double per_column_min = Value(min_space_needed.x / static_cast<double>(wnd_position.second.last_column - wnd_position.second.first_column));
            double per_column_usable_min = Value(min_usable_size.x / static_cast<double>(wnd_position.second.last_column - wnd_position.second.first_column));
            for (std::size_t i = wnd_position.second.first_column; i < wnd_position.second.last_column; ++i) {
                m_column_params[i].effective_min = std::max(m_column_params[i].effective_min, static_cast<unsigned int>(per_column_min + 0.5));
                column_effective_min_usable_sizes[i] = std::max(column_effective_min_usable_sizes[i], static_cast<unsigned int>(per_column_usable_min + 0.5));
            }
        }
    }

    // determine final effective minimums, preserving stretch ratios
    double greatest_min_over_stretch_ratio = 0.0;
    double greatest_usable_min_over_stretch_ratio = 0.0;
    bool is_zero_total_stretch = true;
    for (std::size_t i = 0; i < m_row_params.size(); ++i) {
        if (m_row_params[i].stretch) {
            is_zero_total_stretch = false;
            greatest_min_over_stretch_ratio = std::max(greatest_min_over_stretch_ratio, m_row_params[i].effective_min / m_row_params[i].stretch);
            greatest_usable_min_over_stretch_ratio = std::max(greatest_usable_min_over_stretch_ratio, row_effective_min_usable_sizes[i] / m_row_params[i].stretch);
        }
    }
    for (std::size_t i = 0; i < m_row_params.size(); ++i) {
        if (is_zero_total_stretch || m_row_params[i].stretch) {
            m_row_params[i].effective_min = std::max(
                m_row_params[i].effective_min,
                static_cast<unsigned int>(m_row_params[i].stretch * greatest_min_over_stretch_ratio));
            row_effective_min_usable_sizes[i] = std::max(
                row_effective_min_usable_sizes[i],
                static_cast<unsigned int>(m_row_params[i].stretch * greatest_usable_min_over_stretch_ratio));
        }
    }
    greatest_min_over_stretch_ratio = 0.0;
    greatest_usable_min_over_stretch_ratio = 0.0;
    is_zero_total_stretch = true;
    for (std::size_t i = 0; i < m_column_params.size(); ++i) {
        if (m_column_params[i].stretch) {
            greatest_min_over_stretch_ratio = std::max(greatest_min_over_stretch_ratio, m_column_params[i].effective_min / m_column_params[i].stretch);
            greatest_usable_min_over_stretch_ratio = std::max(greatest_usable_min_over_stretch_ratio, column_effective_min_usable_sizes[i] / m_column_params[i].stretch);
        }
    }
    for (std::size_t i = 0; i < m_column_params.size(); ++i) {
        if (is_zero_total_stretch || m_column_params[i].stretch) {
            m_column_params[i].effective_min = std::max(
                m_column_params[i].effective_min,
                static_cast<unsigned int>(m_column_params[i].stretch * greatest_min_over_stretch_ratio));
            column_effective_min_usable_sizes[i] = std::max(
                column_effective_min_usable_sizes[i],
                static_cast<unsigned int>(m_column_params[i].stretch * greatest_usable_min_over_stretch_ratio));
        }
    }

    //TODO Determine min usable size before stretching to fit space requested
    m_min_usable_size.x = X(2 * m_border_margin);
    for (unsigned int column_size : column_effective_min_usable_sizes) {
        m_min_usable_size.x += static_cast<int>(column_size);
    }
    m_min_usable_size.y = Y(2 * m_border_margin);
    for (unsigned int row_size : row_effective_min_usable_sizes) {
        m_min_usable_size.y += static_cast<int>(row_size);
    }

    bool size_or_min_size_changed = false;
    Pt new_min_size(TotalMinWidth(), TotalMinHeight());
    if (new_min_size != MinSize()) {
        ScopedAssign<bool> assignment(m_stop_resize_recursion, true);
        SetMinSize(new_min_size);
        ClampRectWithMinAndMaxSize(ul, lr);
        size_or_min_size_changed = true;
    }
    Pt original_size = Size();
    Wnd::SizeMove(ul, lr);
    if (Size() != original_size)
        size_or_min_size_changed = true;

    // if this is the layout object for some Wnd, propogate the minimum size up to the owning Wnd
    if (auto&& parent = Parent()) {
        if (parent->GetLayout().get() == this) {
            Pt new_parent_min_size = MinSize() + parent->Size() - parent->ClientSize();
            ScopedAssign<bool> assignment(m_stop_resize_recursion, true);
            parent->SetMinSize(Pt(new_parent_min_size.x, new_parent_min_size.y));
        }
    }

    // determine row and column positions
    double total_stretch = TotalStretch(m_row_params);
    int total_stretch_space = Value(Size().y - MinSize().y);
    double space_per_unit_stretch = total_stretch ? total_stretch_space / total_stretch : 0.0;
    bool larger_than_min = 0 < total_stretch_space;
    double remainder = 0.0;
    int current_origin = m_border_margin;
    for (std::size_t i = 0; i < m_row_params.size(); ++i) {
        if (larger_than_min) {
            if (i < m_row_params.size() - 1) {
                double raw_width =
                    m_row_params[i].effective_min +
                    (total_stretch ?
                     space_per_unit_stretch * m_row_params[i].stretch :
                     total_stretch_space / static_cast<double>(m_row_params.size()));
                int int_raw_width = static_cast<int>(raw_width);
                m_row_params[i].current_width = int_raw_width;
                remainder += raw_width - int_raw_width;
                if (1.0 < remainder) {
                    --remainder;
                    ++m_row_params[i].current_width;
                }
            } else {
                m_row_params[i].current_width = Value(Height()) - m_border_margin - current_origin;
            }
        } else {
            m_row_params[i].current_width = m_row_params[i].effective_min;
        }
        m_row_params[i].current_origin = current_origin;
        current_origin += m_row_params[i].current_width;
    }

    total_stretch = TotalStretch(m_column_params);
    total_stretch_space = Value(Size().x - MinSize().x);
    space_per_unit_stretch = total_stretch ? total_stretch_space / total_stretch : 0.0;
    larger_than_min = 0 < total_stretch_space;
    remainder = 0.0;
    current_origin = m_border_margin;
    for (std::size_t i = 0; i < m_column_params.size(); ++i) {
        if (larger_than_min) {
            if (i < m_column_params.size() - 1) {
                double raw_width =
                    m_column_params[i].effective_min +
                    (total_stretch ?
                     space_per_unit_stretch * m_column_params[i].stretch :
                     total_stretch_space / static_cast<double>(m_column_params.size()));
                int int_raw_width = static_cast<int>(raw_width);
                m_column_params[i].current_width = int_raw_width;
                remainder += raw_width - int_raw_width;
                if (1.0 < remainder) {
                    --remainder;
                    ++m_column_params[i].current_width;
                }
            } else {
                m_column_params[i].current_width = Value(Width()) - m_border_margin - current_origin;
            }
        } else {
            m_column_params[i].current_width = m_column_params[i].effective_min;
        }
        m_column_params[i].current_origin = current_origin;
        current_origin += m_column_params[i].current_width;
    }

    if (m_row_params.back().current_origin + m_row_params.back().current_width != Value(Height()) - m_border_margin)
        throw FailedCalculationCheck("Layout::DoLayout() : calculated row positions do not sum to the height of the layout");

    if (m_column_params.back().current_origin + m_column_params.back().current_width != Value(Width()) - m_border_margin)
        throw FailedCalculationCheck("Layout::DoLayout() : calculated column positions do not sum to the width of the layout");

    // resize cells and their contents
    m_ignore_child_resize = true;
    for (auto& wnd_position : m_wnd_positions) {
        Pt wnd_ul(X(m_column_params[wnd_position.second.first_column].current_origin),
                  Y(m_row_params[wnd_position.second.first_row].current_origin));
        Pt wnd_lr(X(m_column_params[wnd_position.second.last_column - 1].current_origin +
                    m_column_params[wnd_position.second.last_column - 1].current_width),
                  Y(m_row_params[wnd_position.second.last_row - 1].current_origin +
                    m_row_params[wnd_position.second.last_row - 1].current_width));
        Pt ul_margin(X0, Y0);
        Pt lr_margin(X0, Y0);
        if (0 < wnd_position.second.first_row)
            ul_margin.y += static_cast<int>(m_cell_margin / 2);
        if (0 < wnd_position.second.first_column)
            ul_margin.x += static_cast<int>(m_cell_margin / 2);
        if (wnd_position.second.last_row < m_row_params.size())
            lr_margin.y += static_cast<int>(m_cell_margin / 2.0 + 0.5);
        if (wnd_position.second.last_column < m_column_params.size())
            lr_margin.x += static_cast<int>(m_cell_margin / 2.0 + 0.5);

        wnd_ul += ul_margin;
        wnd_lr -= lr_margin;

        if (wnd_position.second.alignment == ALIGN_NONE) { // expand to fill available space
            wnd_position.first->SizeMove(wnd_ul, wnd_lr);
        } else { // align as appropriate
            Pt available_space = wnd_lr - wnd_ul;
            Pt min_usable_size = wnd_position.first->MinUsableSize();
            Pt min_size = wnd_position.first->MinSize();

            // HACK! This is put here so that TextControl, which is currently GG's
            // only height-for-width Wnd type, doesn't get vertically squashed
            // down to 0-height cells.  Note that they can still get horizontally
            // squashed.
            if (TextControl* text_control = dynamic_cast<TextControl*>(wnd_position.first)) {
                X text_width = (wnd_lr.x - wnd_ul.x);
                min_usable_size = text_control->MinUsableSize(text_width);
                min_size = min_usable_size;
            }

            Pt window_size(std::min(available_space.x, std::max(wnd_position.second.original_size.x, std::max(min_size.x, min_usable_size.x))),
                           std::min(available_space.y, std::max(wnd_position.second.original_size.y, std::max(min_size.y, min_usable_size.y))));
            Pt resize_ul, resize_lr;
            if (wnd_position.second.alignment & ALIGN_LEFT) {
                resize_ul.x = wnd_ul.x;
                resize_lr.x = resize_ul.x + window_size.x;
            } else if (wnd_position.second.alignment & ALIGN_CENTER) {
                resize_ul.x = wnd_ul.x + (available_space.x - window_size.x) / 2;
                resize_lr.x = resize_ul.x + window_size.x;
            } else if (wnd_position.second.alignment & ALIGN_RIGHT) {
                resize_lr.x = wnd_lr.x;
                resize_ul.x = resize_lr.x - window_size.x;
            } else {
                resize_ul.x = wnd_ul.x;
                resize_lr.x = wnd_lr.x;
            }
            if (wnd_position.second.alignment & ALIGN_TOP) {
                resize_ul.y = wnd_ul.y;
                resize_lr.y = resize_ul.y + window_size.y;
            } else if (wnd_position.second.alignment & ALIGN_VCENTER) {
                resize_ul.y = wnd_ul.y + (available_space.y - window_size.y) / 2;
                resize_lr.y = resize_ul.y + window_size.y;
            } else if (wnd_position.second.alignment & ALIGN_BOTTOM) {
                resize_lr.y = wnd_lr.y;
                resize_ul.y = resize_lr.y - window_size.y;
            } else {
                resize_ul.y = wnd_ul.y;
                resize_lr.y = wnd_lr.y;
            }
            wnd_position.first->SizeMove(resize_ul, resize_lr);
        }
    }
    m_ignore_child_resize = false;

    if (ContainingLayout() && size_or_min_size_changed)
        ContainingLayout()->ChildSizeOrMinSizeChanged();
}

void Layout::Render()
{
    if (m_render_outline) {
        Pt ul = UpperLeft(), lr = LowerRight();
        FlatRectangle(ul, lr, CLR_ZERO, m_outline_color, 1);
        for (const std::vector<Rect>& columns : CellRects()) {
            for (const Rect& cell : columns) {
                FlatRectangle(cell.ul, cell.lr, CLR_ZERO, m_outline_color, 1);
            }
        }
    }
}

void Layout::Add(std::shared_ptr<Wnd> wnd, std::size_t row, std::size_t column, Flags<Alignment> alignment/* = ALIGN_NONE*/)
{ Add(std::forward<std::shared_ptr<Wnd>>(wnd), row, column, 1, 1, alignment); }

void Layout::Add(std::shared_ptr<Wnd> wnd, std::size_t row, std::size_t column, std::size_t num_rows, std::size_t num_columns,
                 Flags<Alignment> alignment/* = ALIGN_NONE*/)
{
    std::size_t last_row = row + num_rows;
    std::size_t last_column = column + num_columns;
    assert(row < last_row);
    assert(column < last_column);
    ValidateAlignment(alignment);
    if (m_cells.size() < last_row || m_cells[0].size() < last_column) {
        ResizeLayout(std::max(last_row, Rows()), std::max(last_column, Columns()));
    }
    for (std::size_t i = row; i < last_row; ++i) {
        for (std::size_t j = column; j < last_column; ++j) {
            if (m_cells[i][j].lock())
                throw AttemptedOverwrite("Layout::Add() : Attempted to add a Wnd to a layout cell that is already occupied");
            m_cells[i][j] = wnd;
        }
    }
    if (wnd) {
        m_wnd_positions[wnd.get()] = WndPosition(row, column, last_row, last_column, alignment, wnd->RelativeUpperLeft(), wnd->Size());
        AttachChild(std::forward<std::shared_ptr<Wnd>>(wnd));
    }
    RedoLayout();
}

void Layout::Remove(Wnd* wnd)
{
    auto it = m_wnd_positions.find(wnd);
    if (it == m_wnd_positions.end())
        return;

    const WndPosition& wnd_position = it->second;
    for (std::size_t i = wnd_position.first_row; i < wnd_position.last_row; ++i) {
        for (std::size_t j = wnd_position.first_column; j < wnd_position.last_column; ++j) {
            m_cells[i][j] = std::weak_ptr<Wnd>();
        }
    }
    Pt original_ul = it->second.original_ul;
    Pt original_size = it->second.original_size;
    m_wnd_positions.erase(wnd);
    RedoLayout();
    wnd->SizeMove(original_ul, original_ul + original_size);
    DetachChild(wnd);
}

void Layout::DetachAndResetChildren()
{
    std::map<Wnd*, WndPosition> wnd_positions = m_wnd_positions;
    DetachChildren();
    for (auto& wnd_position : wnd_positions) {
        wnd_position.first->SizeMove(wnd_position.second.original_ul, wnd_position.second.original_ul + wnd_position.second.original_size);
    }
    m_wnd_positions.clear();
}

void Layout::ResizeLayout(std::size_t rows, std::size_t columns)
{
    assert(0 < rows);
    assert(0 < columns);
    if (static_cast<std::size_t>(rows) < m_cells.size()) {
        for (std::size_t i = static_cast<std::size_t>(rows); i < m_cells.size(); ++i) {
            for (auto& cell : m_cells[i]) {
                auto locked = cell.lock();
                cell.reset();
                DetachChild(locked.get());
                m_wnd_positions.erase(locked.get());
            }
        }
    }
    m_cells.resize(rows);
    for (auto& row : m_cells) {
        if (static_cast<std::size_t>(columns) < row.size()) {
            for (std::size_t j = static_cast<std::size_t>(columns); j < row.size(); ++j) {
                auto locked = row[j].lock();
                row[j].reset();
                DetachChild(locked.get());
                m_wnd_positions.erase(locked.get());
            }
        }
        row.resize(columns);
    }
    m_row_params.resize(rows);
    m_column_params.resize(columns);
    RedoLayout();
}

void Layout::SetChildAlignment(const Wnd* wnd, Flags<Alignment> alignment)
{
    std::map<Wnd*, WndPosition>::iterator it = m_wnd_positions.find(const_cast<Wnd*>(wnd));
    if (it != m_wnd_positions.end()) {
        ValidateAlignment(alignment);
        it->second.alignment = alignment;
        RedoLayout();
    }
}

void Layout::SetBorderMargin(unsigned int margin)
{
    m_border_margin = margin;
    RedoLayout();
}

void Layout::SetCellMargin(unsigned int margin)
{
    m_cell_margin = margin;
    RedoLayout();
}

void Layout::SetRowStretch(std::size_t row, double stretch)
{
    assert(row < m_row_params.size());
    m_row_params[row].stretch = stretch;
    RedoLayout();
}

void Layout::SetColumnStretch(std::size_t column, double stretch)
{
    assert(column < m_column_params.size());
    m_column_params[column].stretch = stretch;
    RedoLayout();
}

void Layout::SetMinimumRowHeight(std::size_t row, Y height)
{
    assert(row < m_row_params.size());
    m_row_params[row].min = Value(height);
    RedoLayout();
}

void Layout::SetMinimumColumnWidth(std::size_t column, X width)
{
    assert(column < m_column_params.size());
    m_column_params[column].min = Value(width);
    RedoLayout();
}

void Layout::RenderOutline(bool render_outline)
{ m_render_outline = render_outline; }

void Layout::SetOutlineColor(Clr color)
{ m_outline_color = color; }

void Layout::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }

void Layout::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }

void Layout::KeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
{ ForwardEventToParent(); }

double Layout::TotalStretch(const std::vector<RowColParams>& params_vec) const
{
    double retval = 0.0;
    for (const RowColParams& param : params_vec) {
        retval += param.stretch;
    }
    return retval;
}

X Layout::TotalMinWidth() const
{
    X retval = X(2 * m_border_margin);
    for (const RowColParams& column_param : m_column_params) {
        retval += static_cast<int>(column_param.effective_min);
    }
    return retval;
}

Y Layout::TotalMinHeight() const
{
    Y retval = Y(2 * m_border_margin);
    for (const RowColParams& row_param : m_row_params) {
        retval += static_cast<int>(row_param.effective_min);
    }
    return retval;
}

void Layout::ValidateAlignment(Flags<Alignment>& alignment)
{
    int dup_ct = 0;   // duplication count
    if (alignment & ALIGN_LEFT) ++dup_ct;
    if (alignment & ALIGN_RIGHT) ++dup_ct;
    if (alignment & ALIGN_CENTER) ++dup_ct;
    if (1 < dup_ct) {   // when multiples are picked, use ALIGN_CENTER by default
        alignment &= ~(ALIGN_RIGHT | ALIGN_LEFT);
        alignment |= ALIGN_CENTER;
    }
    dup_ct = 0;
    if (alignment & ALIGN_TOP) ++dup_ct;
    if (alignment & ALIGN_BOTTOM) ++dup_ct;
    if (alignment & ALIGN_VCENTER) ++dup_ct;
    if (1 < dup_ct) {   // when multiples are picked, use ALIGN_VCENTER by default
        alignment &= ~(ALIGN_TOP | ALIGN_BOTTOM);
        alignment |= ALIGN_VCENTER;
    }

    // get rid of any irrelevant bits
    if (!(alignment & (ALIGN_LEFT | ALIGN_RIGHT | ALIGN_CENTER | ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER)))
        alignment = ALIGN_NONE;
}

void Layout::RedoLayout()
{
    //Bug:  This does nothing if the size has not changed.  Fixing it to
    //use Layout::SizeMove breaks all text boxes.
    Resize(Size());
}

void Layout::ChildSizeOrMinSizeChanged()
{
    if (!m_ignore_child_resize)
        RedoLayout();
}
