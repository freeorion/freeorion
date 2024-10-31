//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/StyleFactory.h>
#include <GG/utf8/checked.h>
#include <GG/WndEvent.h>

using namespace GG;

namespace {
    CONSTEXPR_FONT bool LineEndsWithEndlineCharacter(const Font::LineData& line, std::string_view original_string)
    {
        if (line.Empty())
            return false;
        const auto str_idx = Value(line.char_data.back().string_index);
        if (str_idx < original_string.size())
            return original_string[str_idx] == '\n';
        return false;
    }

    CONSTEXPR_FONT bool LineEndsWithEndlineCharacter(const Font::LineVec& lines,
                                                     std::size_t line_idx, std::string_view original_string)
    {
        if (line_idx < lines.size())
            return false;
        return LineEndsWithEndlineCharacter(lines.at(line_idx), original_string);
    }
}

///////////////////////////////////////
// MultiEditStyle
///////////////////////////////////////
GG_FLAGSPEC_IMPL(MultiEditStyle);

namespace {
    bool RegisterMultiEditStyles()
    {
        FlagSpec<MultiEditStyle>& spec = FlagSpec<MultiEditStyle>::instance();
        spec.insert(MULTI_NONE,             "MULTI_NONE");
        spec.insert(MULTI_WORDBREAK,        "MULTI_WORDBREAK");
        spec.insert(MULTI_LINEWRAP,         "MULTI_LINEWRAP");
        spec.insert(MULTI_VCENTER,          "MULTI_VCENTER");
        spec.insert(MULTI_TOP,              "MULTI_TOP");
        spec.insert(MULTI_BOTTOM,           "MULTI_BOTTOM");
        spec.insert(MULTI_CENTER,           "MULTI_CENTER");
        spec.insert(MULTI_LEFT,             "MULTI_LEFT");
        spec.insert(MULTI_RIGHT,            "MULTI_RIGHT");
        spec.insert(MULTI_READ_ONLY,        "MULTI_READ_ONLY");
        spec.insert(MULTI_TERMINAL_STYLE,   "MULTI_TERMINAL_STYLE");
        spec.insert(MULTI_INTEGRAL_HEIGHT,  "MULTI_INTEGRAL_HEIGHT");
        spec.insert(MULTI_NO_VSCROLL,       "MULTI_NO_VSCROLL");
        spec.insert(MULTI_NO_HSCROLL,       "MULTI_NO_HSCROLL");
        return true;
    }
    bool dummy = RegisterMultiEditStyles();

    constexpr unsigned int SCROLL_WIDTH = 14;
    constexpr std::size_t ALL_LINES = std::numeric_limits<std::size_t>::max();
    constexpr unsigned int BORDER_THICK = 2;
}


////////////////////////////////////////////////
// GG::MultiEdit
////////////////////////////////////////////////
MultiEdit::MultiEdit(std::string str, const std::shared_ptr<Font>& font, Clr color,
                     Flags<MultiEditStyle> style, Clr text_color, Clr interior) :
    Edit("", font, color, text_color, interior),
    m_style(style),
    m_cursor_begin(0, CP0),
    m_cursor_end(0, CP0),
    m_max_lines_history(ALL_LINES)
{
    SetColor(color);
    Edit::SetText(std::move(str));
}

void MultiEdit::CompleteConstruction()
{
    SetStyle(m_style);
    SizeMove(UpperLeft(), LowerRight()); // do this to set up the scrolls, and in case MULTI_INTEGRAL_HEIGHT is in effect
    SetName("MultiEdit (" + std::to_string(Text().size()) + "): " + Text().substr(0, 16));
}

Pt MultiEdit::MinUsableSize() const noexcept
{
    return Pt(X(4 * SCROLL_WIDTH + 2 * BORDER_THICK),
              Y(4 * SCROLL_WIDTH + 2 * BORDER_THICK));
}

void MultiEdit::Render()
{
    const Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    const Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
    const Clr hilite_color_to_use = Disabled() ? DisabledColor(HiliteColor()) : HiliteColor();
    const Clr text_color_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();

    const Pt ul = UpperLeft(), lr = LowerRight();
    const Pt cl_ul = ClientUpperLeft();
    const Pt cl_lr = ClientLowerRight();

    BeveledRectangle(ul, lr, int_color_to_use, color_to_use, false, BORDER_THICK);

    const auto& lines = GetLineData();
    if (lines.empty()) {
        // no text to render
        return;
    }

    // clip text to client area
    BeginScissorClipping(Pt(cl_ul.x - 1, cl_ul.y), cl_lr);

    const std::size_t first_visible_row = FirstVisibleRow();
    const std::size_t last_visible_row = LastVisibleRow();

    // safety checks
    if (first_visible_row > last_visible_row || (last_visible_row > lines.size())) {
        EndScissorClipping();
        return;
    }

    const auto& font = GetFont();
    const auto LINESKIP = font->Lineskip();
    const auto HEIGHT = font->Height();
    const auto low_cursor_pos  = LowCursorPos();
    const auto high_cursor_pos = HighCursorPos();
    const auto& text = Text();
    const auto focus_wnd{GUI::GetGUI()->FocusWnd().get()};
    const bool multiselected = MultiSelected();
    const std::size_t caret_row = (!multiselected && focus_wnd == this && !(m_style & MULTI_READ_ONLY))
        ? m_cursor_begin.first : std::numeric_limits<std::size_t>::max();

    // process tags
    Font::RenderState rs(text_color_to_use);
    font->ProcessTagsBefore(lines, rs, first_visible_row, CP0);

    auto text_format = (TextFormat() & ~(FORMAT_TOP | FORMAT_BOTTOM)) | FORMAT_VCENTER;
    for (std::size_t row = first_visible_row; row <= last_visible_row && row < lines.size(); ++row) {
        const bool is_caret_row = (caret_row == row);

        const Y row_y_pos = ((m_style & MULTI_TOP) || m_contents_sz.y - ClientSize().y < Y0) ?
            cl_ul.y + static_cast<int>(row) * LINESKIP - m_first_row_shown_y_from_top_of_text :
            cl_lr.y - static_cast<int>(lines.size() - row) * LINESKIP -
                m_first_row_shown_y_from_top_of_text + (m_vscroll && m_hscroll ? BottomMargin() : Y0);

        Pt text_pos(cl_ul.x + RowStartX(row), row_y_pos);
        const X initial_text_x_pos = text_pos.x;

        const auto& line = lines.at(row);
        if (!line.Empty()) {
            const auto& line_char_data = line.char_data;
            const CPSize cd_size{line_char_data.size()};

            if (!multiselected || low_cursor_pos.first > row || row > high_cursor_pos.first) {
                // just draw normal text on this line
                Pt text_lr = text_pos + Pt(line_char_data.back().extent, HEIGHT);
                font->RenderText(text_pos, text_lr, text, text_format, lines, rs, row, CP0, row + 1, cd_size);

            } else {
                // one or more chars of this row are selected, so highlight, then draw the range in the selected-text color

                // idx0 to idx1 is unhighlighted, idx1 to idx2 is hilited, and
                // idx2 to idx3 is unhighlighted; each range may be empty
                const CPSize idx0{0};
                const CPSize idx1 = low_cursor_pos.first == row ? std::max(idx0, low_cursor_pos.second) : idx0;
                const bool ends_with_newline = LineEndsWithEndlineCharacter(lines, row, text);
                // TODO: review if the following adjustment to idx3 is truly necessary; tests suggest it is not. if it is determined necessary, please comment why
                const CPSize idx3{cd_size - (ends_with_newline ? CP1 : CP0)};
                const CPSize idx2 = high_cursor_pos.first == row ? std::min(high_cursor_pos.second, idx3) : idx3;

                // draw text
                const X text_l = (idx0 == idx1) ? text_pos.x :
                                                  initial_text_x_pos + line_char_data.at(Value(idx1 - CP1)).extent;
                Pt text_lr{text_l, text_pos.y + HEIGHT};
                font->RenderText(text_pos, text_lr, text, text_format, lines, rs, row, idx0, row + 1, idx1);
                text_pos.x = text_lr.x;

                // draw highlighting
                if (idx1 != idx2)
                    text_lr.x = initial_text_x_pos + line_char_data.at(Value(idx2 - CP1)).extent;
                FlatRectangle(text_pos, Pt(text_lr.x, text_pos.y + LINESKIP), hilite_color_to_use, CLR_ZERO, 0);

                // draw highlighted text
                font->RenderText(text_pos, text_lr, text, text_format, lines, rs, row, idx1, row + 1, idx2);
                text_pos.x = text_lr.x;

                if (idx2 != idx3) {
                    text_lr.x = initial_text_x_pos + line.char_data.at(Value(idx3 - CP1)).extent;

                    // render the text after the highlighted text, all the way through to the end
                    // of the line, even if ends with newline, so that any tags associated with that
                    // final character will be processed.
                    font->RenderText(text_pos, text_lr, text, text_format, lines, rs,
                                     row, idx2, row + 1, cd_size);
                }
            }
        }

        // if there's no selected text, but this row contains the caret (and
        // MULTI_READ_ONLY is not in effect)
        if (is_caret_row) {
            X caret_x = initial_text_x_pos;
            if (!line.Empty() && m_cursor_begin.second > CP0) {
                const auto caret_char_idx = Value(m_cursor_begin.second - CP1);
                caret_x += line.char_data.at(caret_char_idx).extent;
            }
            glColor(text_color_to_use);
            Line(caret_x, row_y_pos, caret_x, row_y_pos + LINESKIP);
        }
    }

    EndScissorClipping();
}

void MultiEdit::SizeMove(Pt ul, Pt lr)
{
    Pt lower_right = lr;
    if (m_style & MULTI_INTEGRAL_HEIGHT)
        lower_right.y -= Value((lr.y - ul.y) - (2 * PIXEL_MARGIN)) % Value(GetFont()->Lineskip());
    const bool resized = (lower_right - ul) != Size();

    // need to restore scroll position after SetText call below, so that
    // resizing this control doesn't reset the scroll position to the top.
    // just calling PreserveTextPositionOnNextSetText() before the SetText
    // call doesn't work as that leaves the scrollbars unadjusted for the resize
    Pt initial_scroll_pos = ScrollPosition();

    Edit::SizeMove(ul, lower_right);

    if (resized) {
        SetText(Text());                        // resets scroll position to (0, 0)
        SetScrollPosition(initial_scroll_pos);  // restores scroll position
    }
}

void MultiEdit::SelectAll()
{
    const auto& ld = GetLineData();
    m_cursor_begin = std::pair<std::size_t, CPSize>(0, CP0);
    m_cursor_end = ld.empty() ? m_cursor_begin :                            // if empty, null range with begin = end
        std::pair<std::size_t, CPSize>(ld.size() - 1,                       // otherwise, end is past last glyph
                                       CPSize(ld.back().char_data.size()));

    CPSize begin_cursor_cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_begin.first, m_cursor_begin.second, ld);
    CPSize end_cursor_cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, ld);
    this->m_cursor_pos = {begin_cursor_cp_idx, end_cursor_cp_idx};
}

void MultiEdit::DeselectAll()
{
    m_cursor_begin = std::pair<std::size_t, CPSize>(0, CP0);
    m_cursor_end = m_cursor_begin;

    const CPSize cursor_pos = CodePointIndexOfLineAndGlyph(m_cursor_begin.first, m_cursor_begin.second, GetLineData());
    this->m_cursor_pos = {cursor_pos, cursor_pos};
}

void MultiEdit::SetText(std::string str)
{
    if (!utf8::is_valid(str.begin(), str.end()))
        return;

    SetName("MultiEdit (" + std::to_string(str.size()) + "): " + str.substr(0, 16));

    if (m_preserve_text_position_on_next_set_text) {
        TextControl::SetText(std::move(str));
        m_preserve_text_position_on_next_set_text = false;
        return;
    }

    const bool scroll_to_end = (m_style & MULTI_TERMINAL_STYLE) &&
        (!m_vscroll || m_vscroll->ScrollRange().second - m_vscroll->PosnRange().second <= 1);

    // trim the rows, if required by m_max_lines_history
    const Pt cl_sz = ClientSize();
    Flags<TextFormat> format = GetTextFormat();
    auto text_elements = GetFont()->ExpensiveParseFromTextToTextElements(str, format);
    const auto new_lines = GetFont()->DetermineLines(str, format, cl_sz.x, text_elements);
    const auto& line_data{GetLineData()};

    if (m_max_lines_history == ALL_LINES || m_max_lines_history >= new_lines.size()) {
        TextControl::SetText(std::move(str), std::move(text_elements));

    } else {
        std::size_t first_line = 0;
        std::size_t last_line = m_max_lines_history - 1;
        CPSize cursor_begin_idx = INVALID_CP_SIZE; // used to correct the cursor range when lines get chopped
        CPSize cursor_end_idx = INVALID_CP_SIZE;
        if (m_style & MULTI_TERMINAL_STYLE) {
            first_line = new_lines.size() - 1 - m_max_lines_history;
            last_line = new_lines.size() - 1;
        }
        const CPSize first_line_first_char_idx = CodePointIndexOfLineAndGlyph(first_line, CP0, new_lines);
        if (m_style & MULTI_TERMINAL_STYLE) {
            // chopping these lines off the front will invalidate the cursor range unless we do this
            const CPSize cursor_begin_index = CodePointIndexOfLineAndGlyph(m_cursor_begin.first, m_cursor_begin.second, new_lines);
            cursor_begin_idx = first_line_first_char_idx < cursor_begin_index ? CP0 : cursor_begin_index - first_line_first_char_idx;
            const CPSize cursor_end_index = CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, new_lines);
            cursor_end_idx = first_line_first_char_idx < cursor_end_index ? CP0 : cursor_end_index - first_line_first_char_idx;
        }
        const StrSize first_line_first_string_idx = StringIndexOfLineAndGlyph(first_line, CP0, new_lines);
        const StrSize last_line_last_string_idx = (last_line < new_lines.size() - 1) ?
            StringIndexOfLineAndGlyph(last_line + 1, CP0, new_lines) :
            StringIndexOfLineAndGlyph(new_lines.size() - 1, CP0, new_lines);

        // set text to a substring of visible
        TextControl::SetText(str.substr(Value(first_line_first_string_idx),
                                        Value(last_line_last_string_idx - first_line_first_string_idx)));

        if (cursor_begin_idx != INVALID_CP_SIZE && cursor_end_idx != INVALID_CP_SIZE) {
            bool found_cursor_begin = false;
            bool found_cursor_end = false;
            for (std::size_t i = 0; i < line_data.size(); ++i) {
                const auto& ldi{line_data.at(i)};
                if (ldi.Empty())
                    continue;
                const auto char_back_cp{ldi.char_data.back().code_point_index};

                if (!found_cursor_begin && cursor_begin_idx <= char_back_cp) {
                    m_cursor_begin.first = i;
                    m_cursor_begin.second = cursor_begin_idx - GlyphIndexOfLineAndGlyph(i, CP0, line_data);
                    found_cursor_begin = true;
                }
                if (!found_cursor_end && cursor_end_idx <= char_back_cp) {
                    m_cursor_end.first = i;
                    m_cursor_end.second = cursor_end_idx - GlyphIndexOfLineAndGlyph(i, CP0, line_data);
                    found_cursor_end = true;
                }
            }
        }
    }

    // make sure the change in text did not make the cursor position invalid
    if (line_data.empty()) {
        m_cursor_end.first = 0;
        m_cursor_end.second = CP0;
    } else if (line_data.size() <= m_cursor_end.first) {
        m_cursor_end.first = line_data.size() - 1;
        m_cursor_end.second = CPSize(line_data.at(m_cursor_end.first).char_data.size());
    } else if (line_data.at(m_cursor_end.first).char_data.size() < Value(m_cursor_end.second)) {
        m_cursor_end.second = CPSize(line_data.at(m_cursor_end.first).char_data.size());
    }
    m_cursor_begin = m_cursor_end; // eliminate any hiliting

    const CPSize cursor_pos = CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, line_data);
    this->m_cursor_pos = {cursor_pos, cursor_pos};

    m_contents_sz = GetFont()->TextExtent(line_data);

    AdjustScrolls();
    AdjustView();
    if (scroll_to_end && m_vscroll) {
        m_vscroll->ScrollTo(m_vscroll->ScrollRange().second - m_vscroll->PageSize());
        SignalScroll(*m_vscroll, true);
    }

    m_preserve_text_position_on_next_set_text = false;
}

void MultiEdit::SetStyle(Flags<MultiEditStyle> style)
{
    m_style = style;
    ValidateStyle();
    Flags<TextFormat> format;
    if (m_style & MULTI_WORDBREAK)
        format |= FORMAT_WORDBREAK;
    if (m_style & MULTI_LINEWRAP)
        format |= FORMAT_LINEWRAP;
    if (m_style & MULTI_VCENTER)
        format |= FORMAT_VCENTER;
    if (m_style & MULTI_TOP)
        format |= FORMAT_TOP;
    if (m_style & MULTI_BOTTOM)
        format |= FORMAT_BOTTOM;
    if (m_style & MULTI_CENTER)
        format |= FORMAT_CENTER;
    if (m_style & MULTI_LEFT)
        format |= FORMAT_LEFT;
    if (m_style & MULTI_RIGHT)
        format |= FORMAT_RIGHT;
    SetTextFormat(format);
    SetText(Text());
}

void MultiEdit::SetMaxLinesOfHistory(std::size_t max)
{
    m_max_lines_history = max;
    SetText(Text());
}

void MultiEdit::SetScrollPosition(Pt pt)
{
    if (m_hscroll) {
        const auto [low, high] = m_hscroll->ScrollRange();
        const X xlow{low}, xhigh{high};
        if (pt.x < xlow)
            pt.x = xlow;
        if (pt.x > xhigh)
            pt.x = xhigh;
        const X posn_low_x{m_hscroll->PosnRange().first};
        if (pt.x != posn_low_x) {
            m_hscroll->ScrollTo(Value(pt.x));
            SignalScroll(*m_hscroll, true);
        }
    }
    if (m_vscroll) {
        const auto [low, high] = m_vscroll->ScrollRange();
        const Y ylow{low}, yhigh{high};
        if (pt.y < ylow)
            pt.y = ylow;
        if (pt.y > yhigh)
            pt.y = yhigh;
        const Y posn_low_y{m_vscroll->PosnRange().first};
        if (pt.y != posn_low_y) {
            m_vscroll->ScrollTo(Value(pt.y));
            SignalScroll(*m_vscroll, true);
        }
    }
}

void MultiEdit::SetVScrollWheelIncrement(unsigned int increment)
{
    m_vscroll_wheel_scroll_increment = increment;
    AdjustScrolls();
}

void MultiEdit::SetHScrollWheelIncrement(unsigned int increment)
{
    m_hscroll_wheel_scroll_increment = increment;
    AdjustScrolls();
}

X MultiEdit::RightMargin() const noexcept
{ return X(m_vscroll ? SCROLL_WIDTH : 0); }

Y MultiEdit::BottomMargin() const noexcept
{ return Y(m_hscroll ? SCROLL_WIDTH : 0); }

std::pair<std::size_t, CPSize> MultiEdit::GlyphAt(Pt pt) const
{
    const auto& line_data = GetLineData();
    if (line_data.empty())
        return {0, CP0};

    const auto row = RowAt(pt.y);
    const auto constrained_row = std::min(row, line_data.size() - 1u);
    const CPSize line_sz{line_data.at(constrained_row).char_data.size()};

    const auto char_idx = (row > constrained_row) ? line_sz : std::min(GlyphAt(row, pt.x), line_sz);

    //std::cout << "glyph at " << pt << ": line: " << constrained_row << "  glyph idx: " << Value(char_idx) << std::endl;

    return {constrained_row, char_idx};
}

std::pair<std::size_t, CPSize> MultiEdit::GlyphAt(CPSize idx) const
{
    const auto& lines = GetLineData();

    std::pair<std::size_t, CPSize> retval(0, CP0);
    if (lines.empty() || Value(idx) > Text().size())
        return retval;

    retval = LinePositionOfGlyph(idx, lines);

    if (retval.second == INVALID_CP_SIZE) {
        retval.first = lines.size() - 1;
        retval.second = CPSize(lines.back().char_data.size());
    }

    return retval;
}

Pt MultiEdit::ScrollPosition() const
{
    return {m_hscroll ? X{m_hscroll->PosnRange().first} : X0,
            m_vscroll ? Y{m_vscroll->PosnRange().first} : Y0};
}

X MultiEdit::RowStartX(std::size_t row) const
{
    X retval = -m_first_col_shown_x_from_left_of_text;

    const Pt cl_sz = ClientSize();
    const X excess_width = m_contents_sz.x - cl_sz.x;
    if (m_style & MULTI_RIGHT)
        retval -= excess_width;
    else if (m_style & MULTI_CENTER)
        retval -= excess_width / 2;

    const auto& line_data = GetLineData();
    if (line_data.empty() || line_data.size() <= row)
        return retval;
    const auto& row_data = line_data.at(row);
    if (row_data.Empty())
        return retval;
    const X line_width = row_data.char_data.back().extent;
    const X right_margin = m_vscroll && m_hscroll ? RightMargin() : X0;

    if (row_data.justification == ALIGN_LEFT)
        retval += right_margin;
    else if (row_data.justification == ALIGN_RIGHT)
        retval += m_contents_sz.x - line_width + right_margin;
    else if (row_data.justification == ALIGN_CENTER)
        retval += (m_contents_sz.x - line_width + right_margin) / 2;

    return retval;
}

X MultiEdit::CharXOffset(std::size_t row, CPSize idx) const
{
    if (idx == CP0)
        return X0; // first char starts at position 0 on line (assuming ALIGN_LEFT ?)
    const auto& ld{GetLineData()};
    if (ld.empty() || row >= ld.size())
        return X0;
    const auto& cd = ld.at(row).char_data;
    if (cd.empty())
        return X0;
    if (Value(idx) >= cd.size())
        return cd.back().extent; // right side of last char
    return cd.at(Value(idx - CP1)).extent; // right side of previous char is left side of requested char
}

std::size_t MultiEdit::RowAt(Y y) const
{
    const auto format = GetTextFormat();
    y += m_first_row_shown_y_from_top_of_text;

    const auto lineskip = std::max(GG::Y1, GetFont()->Lineskip());

    if ((format & FORMAT_TOP) || m_contents_sz.y - ClientSize().y < Y0) {
        return y / lineskip;

    } else {
        return NumLines() -
            (ClientSize().y + (m_vscroll && m_hscroll ? BottomMargin() : Y0) - y - 1) / lineskip;
    }
}

CPSize MultiEdit::GlyphAt(std::size_t row, X x) const
{
    const auto& line_data = GetLineData();
    if (line_data.empty())
        return CP0;
    // out of range of rows?
    if (row >= line_data.size())
        return CPSize(line_data.back().char_data.size());

    //std::cout << "GlyphAt row: " << row << " X: " << x << std::endl;
    const auto& line = line_data.at(row);
    // empty line?
    if (line.char_data.empty())
        return CP0;

    x -= RowStartX(row);

    // past end of line?
    if (x > line.char_data.back().extent) {
        //std::cout << "past line right edge at: " << line.char_data.back().extent << "\n";
        //std::cout << "row: " << "  last row: " << line_data.size() - 1 << std::endl;
        if (row < line_data.size() - 1) {
            // know this row is not empty due to above check
            return CPSize(line.char_data.size()-1); // last glyph should be a newline if this is not the last row
        } else {
            return CPSize(line.char_data.size());   // one past the last glyph
        }
    }

    // in middle of line. advance characters until within or left of position x
    CPSize retval(CP0);
    while (Value(retval) < line.char_data.size() &&
           line.char_data.at(Value(retval)).extent < x)
    { ++retval; }

    // pick between previous and partially-past character
    if (Value(retval) < line.char_data.size()) {
        X prev_extent = (retval != CP0) ? line.char_data.at(Value(retval - CP1)).extent : X0;
        X half_way = (prev_extent + line.char_data.at(Value(retval)).extent) / 2;
        if (half_way < x) // if the point is more than halfway across the character, put the cursor *after* the character
            ++retval;
    }

    return retval;
}

std::size_t MultiEdit::FirstVisibleRow() const
{ return std::min(RowAt(Y0), NumLines()); }

std::size_t MultiEdit::LastVisibleRow() const
{ return std::min(RowAt(ClientSize().y), NumLines()); }

std::size_t MultiEdit::FirstFullyVisibleRow() const
{
    std::size_t retval = RowAt(Y0);
    if (Value(m_first_row_shown_y_from_top_of_text) % Value(GetFont()->Lineskip()))
        ++retval;
    return std::min(retval, NumLines());
}

std::size_t MultiEdit::LastFullyVisibleRow() const
{
    std::size_t retval = RowAt(ClientSize().y);
    if (Value(m_first_row_shown_y_from_top_of_text + ClientSize().y + BottomMargin()) % Value(GetFont()->Lineskip()))
        --retval;
    return std::min(retval, NumLines());
}

CPSize MultiEdit::FirstVisibleChar(std::size_t row) const
{
    const auto& line_data = GetLineData();
    if (line_data.empty())
        return CP0;
    const auto& line = line_data.at(row);
    if (line.Empty())
        return GlyphAt(row, X0);
    else
        return std::min(GlyphAt(row, X0), CPSize(line.char_data.size()) - CP1);
}

CPSize MultiEdit::LastVisibleChar(std::size_t row) const
{
    const auto& line_data = GetLineData();
    if (line_data.empty())
        return CP0;
    const auto& line = line_data.at(row);
    if (line.Empty())
        return GlyphAt(row, ClientSize().x);
    else
        return std::min(GlyphAt(row, ClientSize().x), CPSize(line.char_data.size()) - CP1);
}

std::pair<std::size_t, CPSize> MultiEdit::HighCursorPos() const
{
    if (m_cursor_begin.first < m_cursor_end.first ||
        (m_cursor_begin.first == m_cursor_end.first &&
         m_cursor_begin.second < m_cursor_end.second))
    {
        return m_cursor_end;
    } else {
        return m_cursor_begin;
    }
}

std::pair<std::size_t, CPSize> MultiEdit::LowCursorPos() const
{
    if (m_cursor_begin.first < m_cursor_end.first ||
        (m_cursor_begin.first == m_cursor_end.first &&
         m_cursor_begin.second < m_cursor_end.second))
    {
        return m_cursor_begin;
    } else {
        return m_cursor_end;
    }
}

std::pair<CPSize, CPSize> MultiEdit::GetDoubleButtonDownWordIndices(CPSize char_index)
{
    //std::cout << "GetDoubleButtonDownWordIndices index: " << Value(char_index) << std::endl;
    const unsigned int ticks = GUI::GetGUI()->Ticks();
    if (ticks - this->m_last_button_down_time <= GUI::GetGUI()->DoubleClickInterval())
        m_in_double_click_mode = true;
    this->m_last_button_down_time = ticks;
    this->m_double_click_cursor_pos = std::pair<CPSize, CPSize>(CP0, CP0);
    if (m_in_double_click_mode) {
        //std::cout << "GetDoubleButtonDownWordIndices in double click mode!" << std::endl;
        const auto words = GUI::GetGUI()->FindWords(Text());
        const auto it = std::find_if(words.begin(), words.end(),
                                     [char_index](auto word) { return word.first < char_index && char_index < word.second; });
        if (it != words.end())
            this->m_double_click_cursor_pos = *it;
    }
    return this->m_double_click_cursor_pos;
}

void MultiEdit::LButtonDown(Pt pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    // when a button press occurs, record the character position under the
    // cursor, and remove any previous selection range
    const auto click_pos = GlyphAt(ScreenToClient(pt));
    m_cursor_begin = m_cursor_end = click_pos;

    CPSize idx = CodePointIndexOfLineAndGlyph(click_pos.first, click_pos.second, GetLineData());
    this->m_cursor_pos = {idx, idx};

    // double-click-drag whole-word selection disabled due to the following code
    // resulting in weird highlighting glitches, possibly due to inconsistency
    // between this->m_cursor_pos and  m_cursor_pos and m_cursor_end

    //const auto word_indices = GetDoubleButtonDownWordIndices(idx);
    ////std::cout << "Edit::LButtonDown got word indices: " << word_indices.first << ", " << word_indices.second << std::endl;
    //if (word_indices.first != word_indices.second)
    //    this->m_cursor_pos = word_indices;
}

void MultiEdit::LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    // when a drag occurs, move m_cursor_end to where the mouse is, which
    // selects a range of characters
    Pt click_pos = ScreenToClient(pt);
    m_cursor_end = GlyphAt(click_pos);

    //std::cout << "\n\nMultiEdit::LDrag at row: " << m_cursor_end.first << ", glyph: " << Value(m_cursor_end.second) << std::endl;

    if (m_in_double_click_mode) {
        // if drag-selecting after a double click, select full words
        std::pair<CPSize, CPSize> initial_indices = DoubleButtonDownCursorPos();

        CPSize cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, GetLineData());
        std::pair<CPSize, CPSize> word_indices = GetDoubleButtonDownDragWordCPIndices(cp_idx);

        std::pair<CPSize, CPSize> final_indices;
        if (word_indices.first == word_indices.second) {
            if (cp_idx < initial_indices.first) {
                final_indices.second = cp_idx;
                final_indices.first = initial_indices.second;
            } else if (initial_indices.second < cp_idx) {
                final_indices.second = cp_idx;
                final_indices.first = initial_indices.first;
            } else {
                final_indices = initial_indices;
            }
        } else {
            if (word_indices.first <= initial_indices.first) {
                final_indices.second = word_indices.first;
                final_indices.first = initial_indices.second;
            } else {
                final_indices.second = word_indices.second;
                final_indices.first = initial_indices.first;
            }
        }
        m_cursor_begin = GlyphAt(final_indices.first);
        m_cursor_end = GlyphAt(final_indices.second);
    }

    const auto& line_data = GetLineData();

    // need to convert from rendered character (glyph) index to code point index in the underlying text
    // any tags that are not rendered will not be included in character counting but will affect the
    // code point index
    const auto [begin_cursor_cp_idx, end_cursor_cp_idx] = (m_cursor_end >= m_cursor_begin) ?
        std::pair{CodePointIndexOfLineAndGlyph(m_cursor_begin.first, m_cursor_begin.second, line_data),
                  CodePointIndexAfterPreviousGlyph(m_cursor_end.first, m_cursor_end.second, line_data)} :
        std::pair{CodePointIndexAfterPreviousGlyph(m_cursor_begin.first, m_cursor_begin.second, line_data),
                  CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, line_data)};

    m_cursor_pos = {begin_cursor_cp_idx, end_cursor_cp_idx};

    //std::cout << "cursor begin: " << Value(m_cursor_begin.second) << " end: " << Value(m_cursor_end.second)
    //          << " cp idx begin: " << Value(begin_cursor_cp_idx) << " end: " << Value(end_cursor_cp_idx)
    //          << std::endl;

    // if dragging past the currently visible text, adjust
    // the view so more text can be selected
    if (click_pos.x < X0 || click_pos.x > ClientSize().x ||
        click_pos.y < Y0 || click_pos.y > ClientSize().y)
    { AdjustView(); }
}

void MultiEdit::MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys)
{
    if (Disabled() || !m_vscroll) {
        ForwardEventToParent();
        return;
    }
    m_vscroll->ScrollLineIncr(-move);
    SignalScroll(*m_vscroll, true);
}

void MultiEdit::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (Disabled()) {
        TextControl::KeyPress(key, key_code_point, mod_keys);
        return;
    }

    if (m_style & MULTI_READ_ONLY) {
        ForwardEventToParent();
        return;
    }

    bool shift_down = mod_keys & (MOD_KEY_LSHIFT | MOD_KEY_RSHIFT);
    bool ctrl_down = mod_keys & (MOD_KEY_CTRL | MOD_KEY_RCTRL);
    bool emit_signal = false;
    bool numlock_on = mod_keys & MOD_KEY_NUM;


    if (!numlock_on) {
        // convert keypad keys into corresponding non-number keys
        switch (key) {
        case Key::GGK_KP0:       key = Key::GGK_INSERT;   break;
        case Key::GGK_KP1:       key = Key::GGK_END;      break;
        case Key::GGK_KP2:       key = Key::GGK_DOWN;     break;
        case Key::GGK_KP3:       key = Key::GGK_PAGEDOWN; break;
        case Key::GGK_KP4:       key = Key::GGK_LEFT;     break;
        case Key::GGK_KP5:                                break;
        case Key::GGK_KP6:       key = Key::GGK_RIGHT;    break;
        case Key::GGK_KP7:       key = Key::GGK_HOME;     break;
        case Key::GGK_KP8:       key = Key::GGK_UP;       break;
        case Key::GGK_KP9:       key = Key::GGK_PAGEUP;   break;
        case Key::GGK_KP_PERIOD: key = Key::GGK_DELETE;   break;
        default:                                          break;
        }
    }

    const auto& linedata{GetLineData()};
    const CPSize cd_cursor_first_sz = (linedata.size() > m_cursor_begin.first) ?
        CPSize{linedata.at(m_cursor_begin.first).char_data.size()} : CP0;
    const CPSize cd_cursor_end_sz = (linedata.size() > m_cursor_end.first) ?
        CPSize{linedata.at(m_cursor_end.first).char_data.size()} : CP0;

    switch (key) {
    case Key::GGK_RETURN:
    case Key::GGK_KP_ENTER: {
        if (MultiSelected())
            ClearSelected();
        Insert(m_cursor_end.first, m_cursor_end.second, '\n');
        ++m_cursor_end.first;
        m_cursor_end.second = CP0;
        // the cursor might be off the bottom if the bottom row was just chopped off to satisfy m_max_lines_history
        if (linedata.empty()) {
            m_cursor_end.first = 0;
            m_cursor_end.second = CP0;
        } else if (linedata.size() <= m_cursor_end.first) {
            m_cursor_end.first = linedata.size() - 1;
            m_cursor_end.second = cd_cursor_first_sz;
            if (LineEndsWithEndlineCharacter(linedata, m_cursor_end.first, Text()))
                --m_cursor_end.second;
        }
        m_cursor_begin = m_cursor_end;
        emit_signal = true;
        break;
    }

    case Key::GGK_LEFT: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = LowCursorPos();
        } else if (CP0 < m_cursor_end.second) {
            // move to previous char on current line
            --m_cursor_end.second;
        } else if (0 < m_cursor_end.first) {
            // move to previous line
            if (linedata.empty()) {
                m_cursor_end.first = 0;
                m_cursor_end.second = CP0;
            } else {
                --m_cursor_end.first;
                const auto& line = linedata.at(m_cursor_end.first);
                m_cursor_end.second = CPSize(line.char_data.size());
                //std::cout << "put cursor end at " << m_cursor_end.first
                //          << " : " << Value(m_cursor_end.second) << std::endl;
                
                if (auto res = LineEndsWithEndlineCharacter(line, Text())) {
                    --m_cursor_end.second;
                    //std::cout << "last char is newline so moved to " << m_cursor_end.first
                    //          << " : " << Value(m_cursor_end.second) << std::endl;
                }
            }
        }
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case Key::GGK_RIGHT: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = HighCursorPos();
        } else if (!linedata.empty() && m_cursor_end.second <
                    cd_cursor_end_sz -
                    (LineEndsWithEndlineCharacter(linedata, m_cursor_end.first, Text()) ? CP1 : CP0))
        {
            ++m_cursor_end.second;
        } else if (!linedata.empty() && m_cursor_end.first < linedata.size() - 1) {
            ++m_cursor_end.first;
            m_cursor_end.second = CP0;
        }
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case Key::GGK_UP: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = LowCursorPos();
        } else if (0 < m_cursor_end.first) {
            X row_start = RowStartX(m_cursor_end.first);
            X char_offset = CharXOffset(m_cursor_end.first, m_cursor_end.second);
            --m_cursor_end.first;
            m_cursor_end.second = GlyphAt(m_cursor_end.first, row_start + char_offset);
            if (!shift_down)
                m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case Key::GGK_DOWN: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = HighCursorPos();
        } else if (!linedata.empty() && m_cursor_end.first < linedata.size() - 1) {
            X row_start = RowStartX(m_cursor_end.first);
            X char_offset = CharXOffset(m_cursor_end.first, m_cursor_end.second);
            ++m_cursor_end.first;
            m_cursor_end.second = GlyphAt(m_cursor_end.first, row_start + char_offset);
            if (!shift_down)
                m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case Key::GGK_HOME: {
        m_cursor_end.second = CP0;
        if (ctrl_down)
            m_cursor_end.first = 0;
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case Key::GGK_END: {
        m_cursor_end.second = cd_cursor_first_sz;
        if (LineEndsWithEndlineCharacter(linedata, m_cursor_end.first, Text()))
            --m_cursor_end.second;
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case Key::GGK_PAGEUP: {
        if (m_vscroll) {
            m_vscroll->ScrollPageDecr();
            SignalScroll(*m_vscroll, true);
            std::size_t rows_moved = m_vscroll->PageSize() / Value(GetFont()->Lineskip());
            m_cursor_end.first = m_cursor_end.first < rows_moved ? 0 : m_cursor_end.first - rows_moved;
            if (cd_cursor_end_sz < m_cursor_end.second)
                m_cursor_end.second = cd_cursor_first_sz;
            m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case Key::GGK_PAGEDOWN: {
        if (m_vscroll) {
            m_vscroll->ScrollPageIncr();
            SignalScroll(*m_vscroll, true);
            std::size_t rows_moved = m_vscroll->PageSize() / Value(GetFont()->Lineskip());
            if (linedata.empty()) {
                m_cursor_end.first = 0;
                m_cursor_end.second = CP0;
            } else {
                m_cursor_end.first = std::min(m_cursor_end.first + rows_moved, NumLines());
                if (cd_cursor_end_sz < m_cursor_end.second)
                    m_cursor_end.second = cd_cursor_first_sz;
            }
            m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case Key::GGK_BACKSPACE: {
        if (MultiSelected()) {
            ClearSelected();
            emit_signal = true;
        } else if (CP0 < m_cursor_begin.second) {
            m_cursor_end.second = --m_cursor_begin.second;
            Erase(m_cursor_begin.first, m_cursor_begin.second, CP1);
            emit_signal = true;
        } else if (!linedata.empty() && 0 < m_cursor_begin.first) {
            m_cursor_end.first = --m_cursor_begin.first;
            m_cursor_begin.second = CPSize(linedata.at(m_cursor_begin.first).char_data.size());
            if (LineEndsWithEndlineCharacter(linedata, m_cursor_begin.first, Text()))
                --m_cursor_begin.second;
            m_cursor_end.second = m_cursor_begin.second;
            Erase(m_cursor_begin.first, m_cursor_begin.second, CP1);
            emit_signal = true;
        }
        break;
    }

    case Key::GGK_DELETE: {
        if (MultiSelected()) {
            ClearSelected();
            emit_signal = true;
        } else if (!linedata.empty() &&
                   Value(m_cursor_begin.second) < linedata.at(m_cursor_begin.first).char_data.size())
        {
            // cursor is not at the end of its current line
            Erase(m_cursor_begin.first, m_cursor_begin.second, CP1);
            emit_signal = true;
        } else if (m_cursor_begin.first < linedata.size() - 1) {
            // cursor is not on the last line, but is at the end of its line
            std::size_t next_line = m_cursor_begin.first + 1;
            CPSize start_of_line = CP0;
            Erase(m_cursor_begin.first, m_cursor_begin.second, next_line, start_of_line);
            emit_signal = true;
        }
        break;
    }

    default: {
        // When text is typed, first a KeyPress event arrives here, where
        // nothing happens. Then a separate TextInput event happens, which leads
        // to a call to the TextInput function below, which will handle actual
        // typing / text input
        return;
        break;
    }
    }

    CPSize begin_cursor_cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_begin.first, m_cursor_begin.second, linedata);
    CPSize end_cursor_cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, linedata);
    this->m_cursor_pos = {begin_cursor_cp_idx, end_cursor_cp_idx};


    AdjustView();
    if (emit_signal)
        EditedSignal(Text());
}

void MultiEdit::TextInput(const std::string& text) {
    // Typed or pasted text. If typed, this function is called from a TextInput event after the
    // KeyPress event leads to a call to MultiEdit::KeyPress, which should itself do nothing.
    if (Disabled()) {
        TextControl::TextInput(text);
        return;
    }

    if (text.empty() || !Interactive() || m_style & MULTI_READ_ONLY)
        return;

    Edit::TextInput(text);  // will call AcceptPastedText, which should be the class' override
}

void MultiEdit::RecreateScrolls()
{
    DetachChildAndReset(m_vscroll);
    DetachChildAndReset(m_hscroll);
    AdjustScrolls();
}

void MultiEdit::PreserveTextPositionOnNextSetText()
{ m_preserve_text_position_on_next_set_text = true; }

void MultiEdit::ValidateStyle()
{
    if (m_style & MULTI_TERMINAL_STYLE) {
        m_style &= ~(MULTI_TOP | MULTI_VCENTER);
        m_style |= MULTI_BOTTOM;
    } else {
        m_style &= ~(MULTI_VCENTER | MULTI_BOTTOM);
        m_style |= MULTI_TOP;
    }

    int dup_ct = 0;   // duplication count
    if (m_style & MULTI_LEFT) ++dup_ct;
    if (m_style & MULTI_RIGHT) ++dup_ct;
    if (m_style & MULTI_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use MULTI_LEFT by default
        m_style &= ~(MULTI_RIGHT | MULTI_CENTER);
        m_style |= MULTI_LEFT;
    }

    if (m_style & (MULTI_LINEWRAP | MULTI_WORDBREAK)) {
        m_style |= MULTI_NO_HSCROLL;
    }
}

void MultiEdit::ClearSelected()
{
    //std::cout << "ClearSelected\n";
    //std::cout << "initial cursor begin: line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << "\n";
    //std::cout << "initial cursor end:   line: " << m_cursor_end.first << " char: " << m_cursor_end.second << "\n";
    //std::cout << "text before erase:" << Text() << std::endl;

    // predetermine post-erase cursor position because erasing can mess up state of cursor position members
    std::pair<std::size_t, CPSize> low_pos = LowCursorPos();
    std::pair<std::size_t, CPSize> high_pos = HighCursorPos();

    Erase(low_pos.first, low_pos.second, high_pos.first, high_pos.second);
    //std::cout << "text after  erase:" << Text() << "\n";
    //std::cout << "after erase cursor begin: line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << "\n";
    //std::cout << "after erase cursor end:   line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;

    m_cursor_end = m_cursor_begin = low_pos;
    //std::cout << "low of cursor begin/end:  line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << std::endl;

    CPSize cursor_pos_cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, GetLineData());
    //std::cout << "got cursor pos: " << cursor_pos << std::endl;
    this->m_cursor_pos = {cursor_pos_cp_idx, cursor_pos_cp_idx};
}

void MultiEdit::AdjustView()
{
    static constexpr CPSize CP5{5};
    const Pt cl_sz = ClientSize();
    const auto format = GetTextFormat();
    const X excess_width = m_contents_sz.x - cl_sz.x;
    const Y excess_height = m_contents_sz.y - cl_sz.y;
    X horz_min(X0);            // these are default values for MULTI_LEFT and MULTI_TOP
    X horz_max = excess_width;
    Y vert_min(Y0);
    Y vert_max = excess_height;

    if (format & FORMAT_RIGHT) {
        horz_min = -excess_width;
        horz_max = horz_min + m_contents_sz.x;
    } else if (format & FORMAT_CENTER) {
        horz_min = -excess_width / 2;
        horz_max = horz_min + m_contents_sz.x;
    }
    if ((format & FORMAT_BOTTOM) && Y0 <= excess_height) {
        vert_min = -excess_height;
        vert_max = vert_min + m_contents_sz.y;
    }

    // make sure that m_first_row_shown_y_from_top_of_text and m_first_col_shown_x_from_left_of_text are within sane bounds
    if (excess_width <= X0 || !m_hscroll) {
        m_first_col_shown_x_from_left_of_text = X0;
    } else {
        m_hscroll->ScrollTo(Value(std::max(horz_min, std::min(m_first_col_shown_x_from_left_of_text, horz_max))));
        SignalScroll(*m_hscroll, true);
    }

    if (excess_height <= Y0 || !m_vscroll) {
        m_first_row_shown_y_from_top_of_text = Y0;
    } else {
        m_vscroll->ScrollTo(Value(std::max(vert_min, std::min(m_first_row_shown_y_from_top_of_text, vert_max))));
        SignalScroll(*m_vscroll, true);
    }

    // adjust m_first_row_shown_y_from_top_of_text position to bring the cursor into view
    std::size_t first_fully_vis_row = FirstFullyVisibleRow();
    if (m_cursor_end.first < first_fully_vis_row && m_vscroll) {
        std::size_t diff = (first_fully_vis_row - m_cursor_end.first);
        m_vscroll->ScrollTo(Value(std::max(vert_min, m_first_row_shown_y_from_top_of_text) - GetFont()->Lineskip() * static_cast<int>(diff)));
        SignalScroll(*m_vscroll, true);
    }
    std::size_t last_fully_vis_row = LastFullyVisibleRow();
    if (last_fully_vis_row < m_cursor_end.first && m_vscroll) {
        std::size_t diff = (m_cursor_end.first - last_fully_vis_row);
        m_vscroll->ScrollTo(Value(std::min(m_first_row_shown_y_from_top_of_text + GetFont()->Lineskip() * static_cast<int>(diff), vert_max)));
        SignalScroll(*m_vscroll, true);
    }

    // adjust m_first_col_shown_x_from_left_of_text position to bring the cursor into view
    CPSize first_visible_char = FirstVisibleChar(m_cursor_end.first);
    CPSize last_visible_char = LastVisibleChar(m_cursor_end.first);
    X client_char_posn = RowStartX(m_cursor_end.first) + CharXOffset(m_cursor_end.first, m_cursor_end.second);
    if (client_char_posn < X0 && m_hscroll) { // if the caret is at a place left of the current visible area
        if (first_visible_char - m_cursor_end.second < CP5) { // if the caret is fewer than five characters before first_visible_char
            // try to move the caret by five characters
            X five_char_distance =
                CharXOffset(m_cursor_end.first, first_visible_char) -
                CharXOffset(m_cursor_end.first, (CP5 < first_visible_char) ? first_visible_char - CP5 : CP0);
            m_hscroll->ScrollTo(Value(m_first_col_shown_x_from_left_of_text - five_char_distance));
            SignalScroll(*m_hscroll, true);
        } else { // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_hscroll->ScrollTo(Value(horz_min + m_first_col_shown_x_from_left_of_text + client_char_posn));
            SignalScroll(*m_hscroll, true);
        }
    } else if (cl_sz.x <= client_char_posn && m_hscroll) { // if the caret is moving to a place right of the current visible area
        if (m_cursor_end.second - last_visible_char < CP5) { // if the caret is fewer than five characters after last_visible_char
            // try to move the caret by five characters
            CPSize last_char_of_line = CodePointIndexOfLineAndGlyph(m_cursor_end.first, INVALID_CP_SIZE, GetLineData());
            X five_char_distance =
                CharXOffset(m_cursor_end.first, (last_visible_char + CP5 < last_char_of_line) ? last_visible_char + CP5 : last_char_of_line) -
                CharXOffset(m_cursor_end.first, last_visible_char);
            m_hscroll->ScrollTo(Value(m_first_col_shown_x_from_left_of_text + five_char_distance));
            SignalScroll(*m_hscroll, true);
        } else { // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_hscroll->ScrollTo(Value(std::min(horz_min + m_first_col_shown_x_from_left_of_text + client_char_posn, horz_max)));
            SignalScroll(*m_hscroll, true);
        }
    }
}

void MultiEdit::AdjustScrolls()
{
    if (m_ignore_adjust_scrolls)
        return;

    ScopedAssign<bool> assignment(m_ignore_adjust_scrolls, true);

    // this client area calculation disregards the thickness of scrolls
    Pt cl_sz = Edit::ClientLowerRight() - Edit::ClientUpperLeft();
    m_contents_sz.y = static_cast<int>(GetLineData().size()) * GetFont()->Lineskip();
    X excess_width = m_contents_sz.x - cl_sz.x;

    const int INT_SCROLL_WIDTH = static_cast<int>(SCROLL_WIDTH);
    bool need_vert =
        !(m_style & MULTI_NO_VSCROLL) &&
        (m_first_row_shown_y_from_top_of_text != Y0 ||
         (m_contents_sz.y > cl_sz.y ||
          (m_contents_sz.y > cl_sz.y - INT_SCROLL_WIDTH &&
           m_contents_sz.x > cl_sz.x - INT_SCROLL_WIDTH)));
    bool need_horz =
        !(m_style & MULTI_NO_HSCROLL) &&
        (m_first_col_shown_x_from_left_of_text != X0 ||
         (m_contents_sz.x > cl_sz.x ||
          (m_contents_sz.x > cl_sz.x - INT_SCROLL_WIDTH &&
           m_contents_sz.y > cl_sz.y - INT_SCROLL_WIDTH)));

    // This probably looks a little odd.  We only want to show scrolls if they
    // are needed, that is if the data shown exceed the bounds of the client
    // area.  However, if we are going to show scrolls, we want to allow them
    // to range such that the first row shown can be any of the N rows.  Dead
    // space after the last row is fine.
    if (!GetLineData().empty() &&
        !(m_style & MULTI_TERMINAL_STYLE) &&
        GetFont()->Lineskip() < cl_sz.y)
    { m_contents_sz.y += cl_sz.y - GetFont()->Lineskip(); }

    Pt orig_cl_sz = ClientSize();

    static constexpr int GAP = PIXEL_MARGIN - 2; // the space between the client area and the border

    const auto& style = GetStyleFactory();

    Y vscroll_min = (m_style & MULTI_TERMINAL_STYLE) ? cl_sz.y - m_contents_sz.y : Y0;
    if (cl_sz.y - m_contents_sz.y > Y0 )
        vscroll_min = Y0;
    X hscroll_min(X0); // default value for MULTI_LEFT
    if (m_style & MULTI_RIGHT) {
        hscroll_min = -excess_width;
    } else if (m_style & MULTI_CENTER) {
        hscroll_min = -excess_width / 2;
    }
    Y vscroll_max = vscroll_min + m_contents_sz.y - 1;
    X hscroll_max = hscroll_min + m_contents_sz.x - 1;

    static constexpr int INT_GAP = static_cast<int>(GAP);
    if (m_vscroll) { // if scroll already exists...
        if (!need_vert) { // remove scroll
            DetachChild(m_vscroll.get());
            m_vscroll = nullptr;
        } else { // ensure vertical scroll has the right logical and physical dimensions
            unsigned int line_size = (m_vscroll_wheel_scroll_increment != 0
                                        ? m_vscroll_wheel_scroll_increment
                                        : Value(GetFont()->Lineskip())*4);

            unsigned int page_size = std::abs(Value(cl_sz.y - (need_horz ? INT_SCROLL_WIDTH : 0)));

            m_vscroll->SizeScroll(Value(vscroll_min), Value(vscroll_max),
                                  line_size, std::max(line_size, page_size));
            X scroll_x = cl_sz.x + INT_GAP - INT_SCROLL_WIDTH;
            Y scroll_y{-GAP};
            m_vscroll->SizeMove(Pt(scroll_x, scroll_y),
                                Pt(scroll_x + INT_SCROLL_WIDTH,
                                   scroll_y + cl_sz.y + 2 * INT_GAP - (need_horz ? INT_SCROLL_WIDTH : 0)));
        }
    } else if (!m_vscroll && need_vert) { // if scroll doesn't exist but is needed
        m_vscroll = style.NewMultiEditVScroll(m_color, CLR_SHADOW);
        m_vscroll->MoveTo(Pt(cl_sz.x + INT_GAP - INT_SCROLL_WIDTH, Y(-GAP)));
        m_vscroll->Resize(Pt(X(SCROLL_WIDTH), cl_sz.y + 2 * INT_GAP - (need_horz ? INT_SCROLL_WIDTH : 0)));

        unsigned int line_size = (m_vscroll_wheel_scroll_increment != 0
                                    ? m_vscroll_wheel_scroll_increment
                                    : Value(GetFont()->Lineskip())*4);

        unsigned int page_size = std::abs(Value(cl_sz.y - (need_horz ? INT_SCROLL_WIDTH : 0)));

        namespace ph = boost::placeholders;

        m_vscroll->SizeScroll(Value(vscroll_min), Value(vscroll_max),
                              line_size, std::max(line_size, page_size));
        AttachChild(m_vscroll);
        m_vscroll->ScrolledSignal.connect(
            boost::bind(&MultiEdit::VScrolled, this, ph::_1, ph::_2, ph::_3, ph::_4));
    }

    if (m_hscroll) { // if scroll already exists...
        if (!need_horz) { // remove scroll
            DetachChild(m_hscroll.get());
            m_hscroll = nullptr;
        } else { // ensure horizontal scroll has the right logical and physical dimensions
            unsigned int line_size = (m_hscroll_wheel_scroll_increment != 0
                                        ? m_hscroll_wheel_scroll_increment
                                        : Value(GetFont()->Lineskip())*4);

            unsigned int page_size = std::abs(Value(cl_sz.x - (need_vert ? INT_SCROLL_WIDTH : 0)));

            m_hscroll->SizeScroll(Value(hscroll_min), Value(hscroll_max),
                                  line_size, std::max(line_size, page_size));
            X scroll_x{-GAP};
            Y scroll_y = cl_sz.y + INT_GAP - INT_SCROLL_WIDTH;
            m_hscroll->SizeMove(Pt(scroll_x, scroll_y),
                                Pt(scroll_x + cl_sz.x + 2 * INT_GAP - (need_vert ? INT_SCROLL_WIDTH : 0),
                                   scroll_y + INT_SCROLL_WIDTH));
        }
    } else if (!m_hscroll && need_horz) { // if scroll doesn't exist but is needed
        m_hscroll = style.NewMultiEditHScroll(m_color, CLR_SHADOW);
        m_hscroll->MoveTo(Pt(X(-GAP), cl_sz.y + GAP - INT_SCROLL_WIDTH));
        m_hscroll->Resize(Pt(cl_sz.x + 2 * GAP - (need_vert ? INT_SCROLL_WIDTH : 0), Y(SCROLL_WIDTH)));

        unsigned int line_size = (m_hscroll_wheel_scroll_increment != 0
                                    ? m_hscroll_wheel_scroll_increment
                                    : Value(GetFont()->Lineskip())*4);

        unsigned int page_size = std::abs(Value(cl_sz.x - (need_vert ? INT_SCROLL_WIDTH : 0)));

        namespace ph = boost::placeholders;

        m_hscroll->SizeScroll(Value(hscroll_min), Value(hscroll_max),
                              line_size, std::max(line_size, page_size));
        AttachChild(m_hscroll);
        m_hscroll->ScrolledSignal.connect(
            boost::bind(&MultiEdit::HScrolled, this, ph::_1, ph::_2, ph::_3, ph::_4));
    }

    // if the new client dimensions changed after adjusting the scrolls,
    // they are unequal to the extent of the text, and there is some kind
    // of wrapping going on, we need to re-SetText()
    Pt new_cl_sz = ClientSize();
    if (orig_cl_sz != new_cl_sz &&
        (new_cl_sz.x != m_contents_sz.x || new_cl_sz.y != m_contents_sz.y) &&
        (m_style & (MULTI_WORDBREAK | MULTI_LINEWRAP)))
    {
        SetText(Text());
    }
}

void MultiEdit::VScrolled(int upper, int lower, int range_upper, int range_lower)
{ m_first_row_shown_y_from_top_of_text = Y(upper); }

void MultiEdit::HScrolled(int upper, int lower, int range_upper, int range_lower)
{ m_first_col_shown_x_from_left_of_text = X(upper); }

void MultiEdit::AcceptPastedText(const std::string& text)
{
    // Edit doesn't know about the MultiEdit style, in which read-only is
    // encoded, so check it here before passing to the default behaviour
    if (m_style & MULTI_READ_ONLY)
        return;

    bool modified_text = false;

    //std::cout << "before modifying text, cursor begin at line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << "\n";
    //std::cout << "before modifying text, cursor end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << "\n";
    //std::cout << "before modifying text, cursor pos: " << this->m_cursor_pos.first << " // " << this->m_cursor_pos.second << "\n";

    if (MultiSelected()) {
        ClearSelected();
        modified_text = true;
        //std::cout << "right after clearing, cursor begin at line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << "\n";
        //std::cout << "right after clearing, cursor end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << "\n";
        //std::cout << "right after clearing, cursor pose: " << this->m_cursor_pos.first << " // " << this->m_cursor_pos.second << std::endl;
        m_cursor_pos.second = m_cursor_pos.first;
    }

    if (!text.empty()) {
        Insert(m_cursor_pos.first, text);
        modified_text = true;
        //std::cout << "Accepted and Inserted, new text:" << Text() << std::endl;
        //std::cout << "right after inserting, cursor begin at line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << "\n";
        //std::cout << "right after inserting, cursor end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << "\n";
        //std::cout << "right after inserting, cursor pose: " << this->m_cursor_pos.first << " // " << this->m_cursor_pos.second << std::endl;
    }

    const auto& line_data = GetLineData();

    if (modified_text) {
        // moves cursor to end of pasted text
        //std::cout << "initial cursor pos: " << m_cursor_pos.first << " - " << m_cursor_pos.second << std::endl;
        CPSize text_span{static_cast<std::size_t>(utf8::distance(text.begin(), text.end()))};
        //std::cout << "text span: " << text_span << std::endl;
        CPSize new_cursor_pos = std::max(CP0, std::min(Length(), m_cursor_pos.second + text_span));
        //std::cout << "new cursor pos: " << new_cursor_pos << std::endl;

        // place cursor start and end after text (at same place so nothing is selected)
        m_cursor_pos.second = new_cursor_pos;
        m_cursor_pos.first = m_cursor_pos.second;

        // convert Edit::m_cursor_pos to equivalent MultiEdit::m_cursor_begin and MultiEdit::m_cursor_end
        m_cursor_begin = GlyphAt(m_cursor_pos.first);
        m_cursor_end = m_cursor_begin;
        //std::cout << "after positioning at cursor pos, cursor begin/end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;

        // the cursor might be off the bottom if the bottom row was just
        // chopped off to satisfy m_max_lines_history
        if (line_data.empty()) {
            m_cursor_begin.first = 0;
            m_cursor_begin.second = CP0;
        } else if (line_data.size() - 1 < m_cursor_begin.first) {
            m_cursor_begin.first = line_data.size() - 1;
            m_cursor_begin.second = CPSize(line_data.at(m_cursor_begin.first).char_data.size());
        }
        m_cursor_end = m_cursor_begin;

        //std::cout << "after newline checks, cursor begin/end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;
    }

    CPSize begin_cursor_cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_begin.first, m_cursor_begin.second, line_data);
    CPSize end_cursor_cp_idx = CodePointIndexOfLineAndGlyph(m_cursor_end.first, m_cursor_end.second, line_data);
    this->m_cursor_pos = {begin_cursor_cp_idx, end_cursor_cp_idx};

    //std::cout << "after converting cursor begin/end to cursor pos, cursor pos: " << m_cursor_pos.first << " - " << m_cursor_pos.second << std::endl;

    AdjustView();
    if (modified_text)
        EditedSignal(Text());
}

