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

#include <GG/MultiEdit.h>

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/Scroll.h>
#include <GG/StyleFactory.h>
#include <GG/utf8/checked.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    bool LineEndsWithEndlineCharacter(const std::vector<Font::LineData>& lines,
                                      std::size_t line,
                                      const std::string& original_string)
    {
        assert(line < lines.size());
        if (lines[line].Empty())
            return false;
        else
            return original_string[Value(lines[line].char_data.back().string_index)] == '\n';
    }
}

///////////////////////////////////////
// MultiEditStyle
///////////////////////////////////////
const MultiEditStyle GG::MULTI_NONE             (0);
const MultiEditStyle GG::MULTI_WORDBREAK        (1 << 0);
const MultiEditStyle GG::MULTI_LINEWRAP         (1 << 1);
const MultiEditStyle GG::MULTI_VCENTER          (1 << 2);
const MultiEditStyle GG::MULTI_TOP              (1 << 3);
const MultiEditStyle GG::MULTI_BOTTOM           (1 << 4);
const MultiEditStyle GG::MULTI_CENTER           (1 << 5);
const MultiEditStyle GG::MULTI_LEFT             (1 << 6);
const MultiEditStyle GG::MULTI_RIGHT            (1 << 7);
const MultiEditStyle GG::MULTI_READ_ONLY        (1 << 8);
const MultiEditStyle GG::MULTI_TERMINAL_STYLE   (1 << 9);
const MultiEditStyle GG::MULTI_INTEGRAL_HEIGHT  (1 << 10);
const MultiEditStyle GG::MULTI_NO_VSCROLL       (1 << 11);
const MultiEditStyle GG::MULTI_NO_HSCROLL       (1 << 12);

GG_FLAGSPEC_IMPL(MultiEditStyle);

namespace {
    bool RegisterMultiEditStyles()
    {
        FlagSpec<MultiEditStyle>& spec = FlagSpec<MultiEditStyle>::instance();
        spec.insert(MULTI_NONE,             "MULTI_NONE",           true);
        spec.insert(MULTI_WORDBREAK,        "MULTI_WORDBREAK",      true);
        spec.insert(MULTI_LINEWRAP,         "MULTI_LINEWRAP",       true);
        spec.insert(MULTI_VCENTER,          "MULTI_VCENTER",        true);
        spec.insert(MULTI_TOP,              "MULTI_TOP",            true);
        spec.insert(MULTI_BOTTOM,           "MULTI_BOTTOM",         true);
        spec.insert(MULTI_CENTER,           "MULTI_CENTER",         true);
        spec.insert(MULTI_LEFT,             "MULTI_LEFT",           true);
        spec.insert(MULTI_RIGHT,            "MULTI_RIGHT",          true);
        spec.insert(MULTI_READ_ONLY,        "MULTI_READ_ONLY",      true);
        spec.insert(MULTI_TERMINAL_STYLE,   "MULTI_TERMINAL_STYLE", true);
        spec.insert(MULTI_INTEGRAL_HEIGHT,  "MULTI_INTEGRAL_HEIGHT",true);
        spec.insert(MULTI_NO_VSCROLL,       "MULTI_NO_VSCROLL",     true);
        spec.insert(MULTI_NO_HSCROLL,       "MULTI_NO_HSCROLL",     true);
        return true;
    }
    bool dummy = RegisterMultiEditStyles();
}

const Flags<MultiEditStyle> GG::MULTI_NO_SCROLL (MULTI_NO_VSCROLL | MULTI_NO_HSCROLL);


////////////////////////////////////////////////
// GG::MultiEdit
////////////////////////////////////////////////
// static(s)
const std::size_t MultiEdit::ALL_LINES = std::numeric_limits<std::size_t>::max();
const unsigned int MultiEdit::SCROLL_WIDTH = 14;
const unsigned int MultiEdit::BORDER_THICK = 2;

MultiEdit::MultiEdit(const std::string& str, const std::shared_ptr<Font>& font, Clr color,
                     Flags<MultiEditStyle> style/* = MULTI_LINEWRAP*/, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/) :
    Edit(str, font, color, text_color, interior),
    m_style(style),
    m_cursor_begin(0, CP0),
    m_cursor_end(0, CP0),
    m_first_col_shown(0),
    m_first_row_shown(0),
    m_max_lines_history(ALL_LINES),
    m_vscroll(nullptr),
    m_hscroll(nullptr),
    m_vscroll_wheel_scroll_increment(0),
    m_hscroll_wheel_scroll_increment(0),
    m_preserve_text_position_on_next_set_text(false),
    m_ignore_adjust_scrolls(false)
{
    SetColor(color);
}

 void MultiEdit::CompleteConstruction()
{
    SetStyle(m_style);
    SizeMove(UpperLeft(), LowerRight()); // do this to set up the scrolls, and in case MULTI_INTEGRAL_HEIGHT is in effect
}

MultiEdit::~MultiEdit()
{}

Pt MultiEdit::MinUsableSize() const
{
    return Pt(X(4 * SCROLL_WIDTH + 2 * BORDER_THICK),
              Y(4 * SCROLL_WIDTH + 2 * BORDER_THICK));
}

Pt MultiEdit::FullSize() const
{
  return Pt(Width(), m_contents_sz.y + Y(PIXEL_MARGIN) * 2);
}

Pt MultiEdit::ClientLowerRight() const
{ return Edit::ClientLowerRight() - Pt(RightMargin(), BottomMargin()); }

Flags<MultiEditStyle> MultiEdit::Style() const
{ return m_style; }

std::size_t MultiEdit::MaxLinesOfHistory() const
{ return m_max_lines_history; }

void MultiEdit::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
    Clr sel_text_color_to_use = Disabled() ? DisabledColor(SelectedTextColor()) : SelectedTextColor();
    Clr hilite_color_to_use = Disabled() ? DisabledColor(HiliteColor()) : HiliteColor();
    Clr text_color_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();

    Pt ul = UpperLeft(), lr = LowerRight();
    Pt cl_ul = ClientUpperLeft();
    Pt cl_lr = ClientLowerRight();

    BeveledRectangle(ul, lr, int_color_to_use, color_to_use, false, BORDER_THICK);

    const auto& lines = GetLineData();
    if (lines.empty()) {
        // no text to render
        return;
    }

    // clip text to client area
    BeginScissorClipping(Pt(cl_ul.x - 1, cl_ul.y), cl_lr);

    Font::RenderState state(text_color_to_use);
    std::size_t first_visible_row = FirstVisibleRow();
    std::size_t last_visible_row = LastVisibleRow();

    // safety checks
    if (first_visible_row > last_visible_row || (last_visible_row > lines.size())) {
        EndScissorClipping();
        return;
    }

    // process tags
    GetFont()->ProcessTagsBefore(lines, state, first_visible_row, CP0);

    Flags<TextFormat> text_format = (TextFormat() & ~(FORMAT_TOP | FORMAT_BOTTOM)) | FORMAT_VCENTER;
    for (std::size_t row = first_visible_row; row <= last_visible_row && row < lines.size(); ++row) {
        Y row_y_pos = ((m_style & MULTI_TOP) || m_contents_sz.y - ClientSize().y < 0) ?
            cl_ul.y + static_cast<int>(row) * GetFont()->Lineskip() - m_first_row_shown :
            cl_lr.y - static_cast<int>(lines.size() - row) * GetFont()->Lineskip() -
                m_first_row_shown + (m_vscroll && m_hscroll ? BottomMargin() : Y0);

        Pt text_pos(cl_ul.x + RowStartX(row), row_y_pos);
        X initial_text_x_pos = text_pos.x;

        const Font::LineData& line = lines[row];
        if (!line.Empty()) {
            // if one or more chars of this row are selected, highlight, then
            // draw the range in the selected-text color
            auto low_cursor_pos  = LowCursorPos();
            auto high_cursor_pos = HighCursorPos();

            if (low_cursor_pos.first <= row && row <= high_cursor_pos.first && MultiSelected()) {
                // idx0 to idx1 is unhilited, idx1 to idx2 is hilited, and
                // idx2 to idx3 is unhilited; each range may be empty
                CPSize idx0(0);
                CPSize idx1 = low_cursor_pos.first == row ? std::max(idx0, low_cursor_pos.second) : idx0;
                CPSize idx3(line.char_data.size());
                // TODO: review if the following adjustment to idx3 is truly necessary; tests suggest it is not
                // if it is determined necessary, please comment why
                if (LineEndsWithEndlineCharacter(lines, row, Text()))
                    --idx3;
                CPSize idx2 = high_cursor_pos.first == row ? std::min(high_cursor_pos.second, idx3) : idx3;

                // draw text
                glColor(text_color_to_use);
                Pt text_lr((idx0 != idx1 ? initial_text_x_pos + line.char_data[Value(idx1 - 1)].extent : text_pos.x), text_pos.y + GetFont()->Height());
                GetFont()->RenderText(text_pos, text_lr, Text(), text_format, lines, state, row, idx0, row + 1, idx1);
                text_pos.x = text_lr.x;

                // draw hiliting
                text_lr.x = idx1 != idx2 ? initial_text_x_pos + line.char_data[Value(idx2 - 1)].extent : text_lr.x;
                FlatRectangle(text_pos, Pt(text_lr.x, text_pos.y + GetFont()->Lineskip()), hilite_color_to_use, CLR_ZERO, 0);
                // draw hilited text
                glColor(sel_text_color_to_use);
                GetFont()->RenderText(text_pos, text_lr, Text(), text_format, lines, state, row, idx1, row + 1, idx2);
                text_pos.x = text_lr.x;

                glColor(text_color_to_use);
                text_lr.x = idx2 != idx3 ? initial_text_x_pos + line.char_data[Value(idx3 - 1)].extent : text_lr.x;
                // render the following all the way through to the end of the line, even if ends with newline,
                // so that any tages associated with that finel character will be processed.
                GetFont()->RenderText(text_pos, text_lr, Text(), text_format, lines, state, row, idx2, row + 1, CPSize(line.char_data.size()));
            } else { // just draw normal text on this line
                Pt text_lr = text_pos + Pt(line.char_data.back().extent, GetFont()->Height());
                glColor(text_color_to_use);
                GetFont()->RenderText(text_pos, text_lr, Text(), text_format, lines, state, row, CP0, row + 1, CPSize(line.char_data.size()));
            }
        }

        // if there's no selected text, but this row contains the caret (and
        // MULTI_READ_ONLY is not in effect)
        if (GUI::GetGUI()->FocusWnd().get() == this &&
            !MultiSelected() &&
            m_cursor_begin.first == row &&
            !(m_style & MULTI_READ_ONLY))
        {
            X caret_x = CharXOffset(m_cursor_begin.first, m_cursor_begin.second) + initial_text_x_pos;
            glColor(text_color_to_use);
            Line(caret_x, row_y_pos, caret_x, row_y_pos + GetFont()->Lineskip());
        }
    }

    EndScissorClipping();
}

void MultiEdit::SizeMove(const Pt& ul, const Pt& lr)
{
    Pt lower_right = lr;
    if (m_style & MULTI_INTEGRAL_HEIGHT)
        lower_right.y -= ((lr.y - ul.y) - (2 * PIXEL_MARGIN)) % GetFont()->Lineskip();
    bool resized = lower_right - ul != Size();

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
    m_cursor_begin = std::pair<std::size_t, CPSize>(0, CP0);
    m_cursor_end = GetLineData().empty() ? m_cursor_begin : std::pair<std::size_t, CPSize>(
        GetLineData().size() - 1,
        CPSize(GetLineData()[GetLineData().size() - 1].char_data.size()));

    CPSize begin_cursor_pos = CharIndexOf(m_cursor_begin.first, m_cursor_begin.second);
    CPSize end_cursor_pos = CharIndexOf(m_cursor_end.first, m_cursor_end.second);
    this->m_cursor_pos = {begin_cursor_pos, end_cursor_pos};
}

void MultiEdit::DeselectAll()
{
    m_cursor_begin = std::pair<std::size_t, CPSize>(0, CP0);
    m_cursor_end = m_cursor_begin;

    CPSize cursor_pos = CharIndexOf(m_cursor_begin.first, m_cursor_begin.second);
    this->m_cursor_pos = {cursor_pos, cursor_pos};
}

void MultiEdit::SetText(const std::string& str)
{
    if (!utf8::is_valid(str.begin(), str.end()))
        return;

    if (m_preserve_text_position_on_next_set_text) {
        TextControl::SetText(str);
    } else {
        bool scroll_to_end = (m_style & MULTI_TERMINAL_STYLE) &&
            (!m_vscroll || m_vscroll->ScrollRange().second - m_vscroll->PosnRange().second <= 1);

        // trim the rows, if required by m_max_lines_history
        Pt cl_sz = ClientSize();
        Flags<TextFormat> format = GetTextFormat();
        if (m_max_lines_history == ALL_LINES) {
            TextControl::SetText(str);
        } else {
            auto text_elements = GetFont()->ExpensiveParseFromTextToTextElements(str, format);
            auto lines = GetFont()->DetermineLines(str, format, cl_sz.x, text_elements);
            if (m_max_lines_history < lines.size()) {
                std::size_t first_line = 0;
                std::size_t last_line = m_max_lines_history - 1;
                CPSize cursor_begin_idx = INVALID_CP_SIZE; // used to correct the cursor range when lines get chopped
                CPSize cursor_end_idx = INVALID_CP_SIZE;
                if (m_style & MULTI_TERMINAL_STYLE) {
                    first_line = lines.size() - 1 - m_max_lines_history;
                    last_line = lines.size() - 1;
                }
                CPSize first_line_first_char_idx = CharIndexOf(first_line, CP0, &lines);
                if (m_style & MULTI_TERMINAL_STYLE) {
                    // chopping these lines off the front will invalidate the cursor range unless we do this
                    CPSize cursor_begin_string_index = CharIndexOf(m_cursor_begin.first, m_cursor_begin.second, &lines);
                    cursor_begin_idx = first_line_first_char_idx < cursor_begin_string_index ? CP0 : cursor_begin_string_index - first_line_first_char_idx;
                    CPSize cursor_end_string_index = CharIndexOf(m_cursor_end.first, m_cursor_end.second, &lines);
                    cursor_end_idx = first_line_first_char_idx < cursor_end_string_index ? CP0 : cursor_end_string_index - first_line_first_char_idx;
                }
                StrSize first_line_first_string_idx = StringIndexOf(first_line, CP0, lines);
                StrSize last_line_last_string_idx = last_line < lines.size() - 1 ? StringIndexOf(last_line + 1, CP0, lines) : StringIndexOf(lines.size() - 1, CP0, lines);
                TextControl::SetText(str.substr(Value(first_line_first_string_idx), Value(last_line_last_string_idx - first_line_first_string_idx)));
                if (cursor_begin_idx != INVALID_CP_SIZE && cursor_end_idx != INVALID_CP_SIZE) {
                    bool found_cursor_begin = false;
                    bool found_cursor_end = false;
                    for (std::size_t i = 0; i < GetLineData().size(); ++i) {
                        if (!found_cursor_begin && !GetLineData()[i].Empty() && cursor_begin_idx <= GetLineData()[i].char_data.back().code_point_index) {
                            m_cursor_begin.first = i;
                            m_cursor_begin.second = cursor_begin_idx - CharIndexOf(i, CP0);
                            found_cursor_begin = true;
                        }
                        if (!found_cursor_end && !GetLineData()[i].Empty() && cursor_end_idx <= GetLineData()[i].char_data.back().code_point_index) {
                            m_cursor_end.first = i;
                            m_cursor_end.second = cursor_end_idx - CharIndexOf(i, CP0);
                            found_cursor_end = true;
                        }
                    }
                }
            } else {
                TextControl::SetText(str);
            }
        }

        // make sure the change in text did not make the cursor position invalid
        if (GetLineData().empty()) {
            m_cursor_end.first = 0;
            m_cursor_end.second = CP0;
        } else if (GetLineData().size() <= m_cursor_end.first) {
            m_cursor_end.first = GetLineData().size() - 1;
            m_cursor_end.second = CPSize(GetLineData()[m_cursor_end.first].char_data.size());
        } else if (GetLineData()[m_cursor_end.first].char_data.size() < m_cursor_end.second) {
            m_cursor_end.second = CPSize(GetLineData()[m_cursor_end.first].char_data.size());
        }
        m_cursor_begin = m_cursor_end; // eliminate any hiliting

        CPSize cursor_pos = CharIndexOf(m_cursor_end.first, m_cursor_end.second);
        this->m_cursor_pos = {cursor_pos, cursor_pos};

        m_contents_sz = GetFont()->TextExtent(GetLineData());

        AdjustScrolls();
        AdjustView();
        if (scroll_to_end && m_vscroll) {
            m_vscroll->ScrollTo(m_vscroll->ScrollRange().second - m_vscroll->PageSize());
            SignalScroll(*m_vscroll, true);
        }
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
        std::pair<int, int> range = m_hscroll->ScrollRange();
        if (pt.x < range.first)
            pt.x = X(range.first);
        if (pt.x > range.second)
            pt.x = X(range.second);
        std::pair<int, int> posn_range = m_hscroll->PosnRange();
        if (pt.x != posn_range.first) {
            m_hscroll->ScrollTo(Value(pt.x));
            SignalScroll(*m_hscroll, true);
        }
    }
    if (m_vscroll) {
        std::pair<int, int> range = m_vscroll->ScrollRange();
        if (pt.y < range.first)
            pt.y = Y(range.first);
        if (pt.y > range.second)
            pt.y = Y(range.second);
        std::pair<int, int> posn_range = m_vscroll->PosnRange();
        if (pt.y != posn_range.first) {
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

bool MultiEdit::MultiSelected() const
{ return m_cursor_begin != m_cursor_end; }

X MultiEdit::RightMargin() const
{ return X(m_vscroll ? SCROLL_WIDTH : 0); }

Y MultiEdit::BottomMargin() const
{ return Y(m_hscroll ? SCROLL_WIDTH : 0); }

std::pair<std::size_t, CPSize> MultiEdit::CharAt(const Pt& pt) const
{
    if (GetLineData().empty())
        return std::pair<std::size_t, CPSize>(0, CP0);

    std::pair<std::size_t, CPSize> retval;

    size_t row = RowAt(pt.y);
    retval.first = std::min(row, GetLineData().size() - 1);

    if (row > retval.first)
        retval.second = CPSize(GetLineData()[retval.first].char_data.size());
    else
        retval.second = std::min(CharAt(row, pt.x),
                                 CPSize(GetLineData()[retval.first].char_data.size()));

    return retval;
}

std::pair<std::size_t, CPSize> MultiEdit::CharAt(CPSize idx) const
{
    const auto& lines = GetLineData();

    std::pair<std::size_t, CPSize> retval(0, CP0);
    if (lines.empty() || idx > Text().size())
        return retval;

    retval = LinePositionOf(idx, lines);

    if (retval.second == INVALID_CP_SIZE) {
        retval.first = lines.size() - 1;
        retval.second = CPSize(lines.back().char_data.size());
    }

    return retval;
}

Pt MultiEdit::ScrollPosition() const
{
    Pt retval(X0, Y0);
    if (m_hscroll)
        retval.x = X(m_hscroll->PosnRange().first);
    if (m_vscroll)
        retval.y = Y(m_vscroll->PosnRange().first);
    return retval;
}

CPSize MultiEdit::CharIndexOf(std::size_t row, CPSize char_idx,
                              const std::vector<Font::LineData>* line_data) const
{
    const auto& lines = line_data ? *line_data : GetLineData();

    if (lines.empty())
        return CP0; // no text
    if (lines[row].Empty() && row == 0)
        return CP0; // empty first line

    // if selecting into an empty line, return one past the end of the previous line
    if (lines[row].Empty())
        return lines[row-1].char_data.back().code_point_index + 1;

    // if at start of (non-empty) line, return first character of that line
    if (char_idx == CP0)
        return lines[row].char_data.front().code_point_index;

    const Font::LineData& line = lines[row];

    // if at end of line, go with one past the last character of the line
    if (char_idx >= line.char_data.size())
        return line.char_data.back().code_point_index + 1;

    // "rewind" the first position to encompass all tag text that is
    // associated with that position
    CPSize retval = line.char_data[Value(char_idx)].code_point_index;
    for (const auto& tag : line.char_data[Value(char_idx)].tags)
        retval -= tag->CodePointSize();

    return retval;
}

X MultiEdit::RowStartX(std::size_t row) const
{
    X retval = -m_first_col_shown;

    Pt cl_sz = ClientSize();
    X excess_width = m_contents_sz.x - cl_sz.x;
    if (m_style & MULTI_RIGHT)
        retval -= excess_width;
    else if (m_style & MULTI_CENTER)
        retval -= excess_width / 2;

    if (!GetLineData().empty() && !GetLineData()[row].Empty()) {
        X line_width = GetLineData()[row].char_data.back().extent;
        if (GetLineData()[row].justification == ALIGN_LEFT) {
            retval += (m_vscroll && m_hscroll ? RightMargin() : X0);
        } else if (GetLineData()[row].justification == ALIGN_RIGHT) {
            retval += m_contents_sz.x - line_width + (m_vscroll && m_hscroll ? RightMargin() : X0);
        } else if (GetLineData()[row].justification == ALIGN_CENTER) {
            retval += (m_contents_sz.x - line_width + (m_vscroll && m_hscroll ? RightMargin() : X0)) / 2;
        }
    }

    return retval;
}

X MultiEdit::CharXOffset(std::size_t row, CPSize idx) const
{ return (0 < idx && !GetLineData().empty() ? GetLineData()[row].char_data[Value(idx - 1)].extent : X0); }

std::size_t MultiEdit::RowAt(Y y) const
{
    std::size_t retval = 0;
    Flags<TextFormat> format = GetTextFormat();
    y += m_first_row_shown;
    if ((format & FORMAT_TOP) || m_contents_sz.y - ClientSize().y < 0) {
        retval = Value(y / GetFont()->Lineskip());
    } else { // FORMAT_BOTTOM
        retval = NumLines() -
            Value((ClientSize().y + (m_vscroll && m_hscroll ? BottomMargin() : Y0) - y - 1) / GetFont()->Lineskip());
    }
    return retval;
}

CPSize MultiEdit::CharAt(std::size_t row, X x) const
{
    if (GetLineData().empty())
        return CP0;
    // out of range of rows?
    if (row >= GetLineData().size()) {
        return CPSize(GetLineData().back().char_data.size());
    }

    //std::cout << "CharAt row: " << row << " X: " << x << std::endl;
    const Font::LineData& line = GetLineData()[row];
    // empty line?
    if (line.char_data.empty())
        return CP0;

    x -= RowStartX(row);

    // past end of line?
    if (x > line.char_data.back().extent) {
        //std::cout << "past line right edge at: " << line.char_data.back().extent << std::endl;
        //std::cout << "row: " << "  last row: " << GetLineData().size() - 1 << std::endl;
        if (row < GetLineData().size() - 1) {
            // know this row is not empty due to above check
            return CPSize(line.char_data.size()-1); // last character should be a newline if this is not the last row
        } else {
            return CPSize(line.char_data.size());   // want one past the last actual character
        }
    }

    // in middle of line. advance characters until within or left of position x
    CPSize retval(0);
    while (retval < line.char_data.size() &&
           line.char_data[Value(retval)].extent < x)
    { ++retval; }

    // pick between previous and partially-past character
    if (retval < line.char_data.size()) {
        X prev_extent = retval ? line.char_data[Value(retval - 1)].extent : X0;
        X half_way = (prev_extent + line.char_data[Value(retval)].extent) / 2;
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
    if (m_first_row_shown % GetFont()->Lineskip())
        ++retval;
    return std::min(retval, NumLines());
}

std::size_t MultiEdit::LastFullyVisibleRow() const
{
    std::size_t retval = RowAt(ClientSize().y);
    if ((m_first_row_shown + ClientSize().y + BottomMargin()) % GetFont()->Lineskip())
        --retval;
    return std::min(retval, NumLines());
}

CPSize MultiEdit::FirstVisibleChar(std::size_t row) const
{
    if (GetLineData().empty())
        return CP0;
    if (GetLineData()[row].Empty())
        return CharAt(row, X0);
    else
        return std::min(CharAt(row, X0), CPSize(GetLineData()[row].char_data.size()) - 1);
}

CPSize MultiEdit::LastVisibleChar(std::size_t row) const
{
    if (GetLineData().empty())
        return CP0;
    if (GetLineData()[row].Empty())
        return CharAt(row, ClientSize().x);
    else
        return std::min(CharAt(row, ClientSize().x), CPSize(GetLineData()[row].char_data.size()) - 1);
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

namespace {
    struct InRange
    {
        InRange(CPSize value) : m_value(value) {}
        bool operator()(const std::pair<CPSize, CPSize>& p) const
        { return p.first < m_value && m_value < p.second; }
        const CPSize m_value;
    };
}

std::pair<CPSize, CPSize> MultiEdit::GetDoubleButtonDownWordIndices(CPSize char_index)
{
    //std::cout << "GetDoubleButtonDownWordIndices index: " << Value(char_index) << std::endl;
    unsigned int ticks = GUI::GetGUI()->Ticks();
    if (ticks - this->m_last_button_down_time <= GUI::GetGUI()->DoubleClickInterval())
        m_in_double_click_mode = true;
    this->m_last_button_down_time = ticks;
    this->m_double_click_cursor_pos = std::pair<CPSize, CPSize>(CP0, CP0);
    if (m_in_double_click_mode) {
        //std::cout << "GetDoubleButtonDownWordIndices in double click mode!" << std::endl;
        auto words = GUI::GetGUI()->FindWords(Text());
        auto it = std::find_if(words.begin(), words.end(), InRange(char_index));
        if (it != words.end())
            this->m_double_click_cursor_pos = *it;
    }
    return this->m_double_click_cursor_pos;
}

void MultiEdit::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    // when a button press occurs, record the character position under the
    // cursor, and remove any previous selection range
    std::pair<std::size_t, CPSize> click_pos = CharAt(ScreenToClient(pt));
    m_cursor_begin = m_cursor_end = click_pos;
    //std::cout << "click pos: " << click_pos.first << " / " << click_pos.second << std::endl;

    CPSize idx = CharIndexOf(click_pos.first, click_pos.second);
    this->m_cursor_pos = {idx, idx};
    //std::cout << "cursor pos: " << this->m_cursor_pos.first << std::endl;

    // double-click-drag whole-word selection disabled due to the following code
    // resulting in weird highlighting glitches, possibly due to inconsistency
    // between this->m_cursor_pos and  m_cursor_pos and m_cursor_end

    //std::pair<CPSize, CPSize> word_indices = GetDoubleButtonDownWordIndices(idx);
    ////std::cout << "Edit::LButtonDown got word indices: " << word_indices.first << ", " << word_indices.second << std::endl;
    //if (word_indices.first != word_indices.second)
    //    this->m_cursor_pos = word_indices;
}

void MultiEdit::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    // when a drag occurs, move m_cursor_end to where the mouse is, which
    // selects a range of characters
    Pt click_pos = ScreenToClient(pt);
    m_cursor_end = CharAt(click_pos);

    //std::cout << "MultiEdit::LDrag at row: " << m_cursor_end.first << ", col: " << m_cursor_end.second << std::endl << std::flush;

    if (m_in_double_click_mode) {
        // if drag-selecting after a double click, select full words
        std::pair<CPSize, CPSize> initial_indices = DoubleButtonDownCursorPos();

        CPSize idx = CharIndexOf(m_cursor_end.first, m_cursor_end.second);
        std::pair<CPSize, CPSize> word_indices = GetDoubleButtonDownDragWordIndices(idx);

        std::pair<CPSize, CPSize> final_indices;
        if (word_indices.first == word_indices.second) {
            if (idx < initial_indices.first) {
                final_indices.second = idx;
                final_indices.first = initial_indices.second;
            } else if (initial_indices.second < idx) {
                final_indices.second = idx;
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
        m_cursor_begin = CharAt(final_indices.first);
        m_cursor_end = CharAt(final_indices.second);
    }

    CPSize begin_cursor_pos = CharIndexOf(m_cursor_begin.first, m_cursor_begin.second);
    CPSize end_cursor_pos = CharIndexOf(m_cursor_end.first, m_cursor_end.second);

    //std::cout << "MultiEdit::LDrag cursor covers code points: " << begin_cursor_pos << " to " << end_cursor_pos << std::endl << std::flush;

    this->m_cursor_pos = {begin_cursor_pos, end_cursor_pos};

    // if dragging past the currently visible text, adjust
    // the view so more text can be selected
    if (click_pos.x < 0 || click_pos.x > ClientSize().x ||
        click_pos.y < 0 || click_pos.y > ClientSize().y)
    { AdjustView(); }
}

void MultiEdit::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    if (Disabled() || !m_vscroll) {
        ForwardEventToParent();
        return;
    }
    m_vscroll->ScrollLineIncr(-move);
    SignalScroll(*m_vscroll, true);
}

void MultiEdit::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
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
        case GGK_KP0:       key = GGK_INSERT;   break;
        case GGK_KP1:       key = GGK_END;      break;
        case GGK_KP2:       key = GGK_DOWN;     break;
        case GGK_KP3:       key = GGK_PAGEDOWN; break;
        case GGK_KP4:       key = GGK_LEFT;     break;
        case GGK_KP5:                           break;
        case GGK_KP6:       key = GGK_RIGHT;    break;
        case GGK_KP7:       key = GGK_HOME;     break;
        case GGK_KP8:       key = GGK_UP;       break;
        case GGK_KP9:       key = GGK_PAGEUP;   break;
        case GGK_KP_PERIOD: key = GGK_DELETE;   break;
        default:                                break;
        }
    }

    switch (key) {
    case GGK_RETURN:
    case GGK_KP_ENTER: {
        if (MultiSelected())
            ClearSelected();
        Insert(m_cursor_end.first, m_cursor_end.second, '\n');
        ++m_cursor_end.first;
        m_cursor_end.second = CP0;
        // the cursor might be off the bottom if the bottom row was just chopped off to satisfy m_max_lines_history
        if (GetLineData().empty()) {
            m_cursor_end.first = 0;
            m_cursor_end.second = CP0;
        } else if (GetLineData().size() <= m_cursor_end.first) {
            m_cursor_end.first = GetLineData().size() - 1;
            m_cursor_end.second = CPSize(GetLineData()[m_cursor_end.first].char_data.size());
            if (LineEndsWithEndlineCharacter(GetLineData(), m_cursor_end.first, Text()))
                --m_cursor_end.second;
        }
        m_cursor_begin = m_cursor_end;
        emit_signal = true;
        break;
    }

    case GGK_LEFT: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = LowCursorPos();
        } else if (0 < m_cursor_end.second) {
            --m_cursor_end.second;
        } else if (0 < m_cursor_end.first) {
            if (GetLineData().empty()) {
                m_cursor_end.first = 0;
                m_cursor_end.second = CP0;
            } else {
                --m_cursor_end.first;
                m_cursor_end.second = CPSize(GetLineData()[m_cursor_end.first].char_data.size());
                if (LineEndsWithEndlineCharacter(GetLineData(), m_cursor_end.first, Text()))
                    --m_cursor_end.second;
            }
        }
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case GGK_RIGHT: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = HighCursorPos();
        } else if (!GetLineData().empty() && m_cursor_end.second <
                    GetLineData()[m_cursor_end.first].char_data.size() -
                    (LineEndsWithEndlineCharacter(GetLineData(), m_cursor_end.first, Text()) ? 1 : 0))
        {
            ++m_cursor_end.second;
        } else if (!GetLineData().empty() && m_cursor_end.first < GetLineData().size() - 1) {
            ++m_cursor_end.first;
            m_cursor_end.second = CP0;
        }
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case GGK_UP: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = LowCursorPos();
        } else if (0 < m_cursor_end.first) {
            X row_start = RowStartX(m_cursor_end.first);
            X char_offset = CharXOffset(m_cursor_end.first, m_cursor_end.second);
            --m_cursor_end.first;
            m_cursor_end.second = CharAt(m_cursor_end.first, row_start + char_offset);
            if (!shift_down)
                m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case GGK_DOWN: {
        if (MultiSelected() && !shift_down) {
            m_cursor_begin = m_cursor_end = HighCursorPos();
        } else if (!GetLineData().empty() && m_cursor_end.first < GetLineData().size() - 1) {
            X row_start = RowStartX(m_cursor_end.first);
            X char_offset = CharXOffset(m_cursor_end.first, m_cursor_end.second);
            ++m_cursor_end.first;
            m_cursor_end.second = CharAt(m_cursor_end.first, row_start + char_offset);
            if (!shift_down)
                m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case GGK_HOME: {
        m_cursor_end.second = CP0;
        if (ctrl_down)
            m_cursor_end.first = 0;
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case GGK_END: {
        m_cursor_end.second = CPSize(GetLineData()[m_cursor_end.first].char_data.size());
        if (LineEndsWithEndlineCharacter(GetLineData(), m_cursor_end.first, Text()))
            --m_cursor_end.second;
        if (!shift_down)
            m_cursor_begin = m_cursor_end;
        break;
    }

    case GGK_PAGEUP: {
        if (m_vscroll) {
            m_vscroll->ScrollPageDecr();
            SignalScroll(*m_vscroll, true);
            std::size_t rows_moved = m_vscroll->PageSize() / Value(GetFont()->Lineskip());
            m_cursor_end.first = m_cursor_end.first < rows_moved ? 0 : m_cursor_end.first - rows_moved;
            if (GetLineData()[m_cursor_end.first].char_data.size() < m_cursor_end.second)
                m_cursor_end.second = CPSize(GetLineData()[m_cursor_end.first].char_data.size());
            m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case GGK_PAGEDOWN: {
        if (m_vscroll) {
            m_vscroll->ScrollPageIncr();
            SignalScroll(*m_vscroll, true);
            std::size_t rows_moved = m_vscroll->PageSize() / Value(GetFont()->Lineskip());
            if (GetLineData().empty()) {
                m_cursor_end.first = 0;
                m_cursor_end.second = CP0;
            } else {
                m_cursor_end.first = std::min(m_cursor_end.first + rows_moved, NumLines());
                if (GetLineData()[m_cursor_end.first].char_data.size() < m_cursor_end.second)
                    m_cursor_end.second = CPSize(GetLineData()[m_cursor_end.first].char_data.size());
            }
            m_cursor_begin = m_cursor_end;
        }
        break;
    }

    case GGK_BACKSPACE: {
        if (MultiSelected()) {
            ClearSelected();
            emit_signal = true;
        } else if (0 < m_cursor_begin.second) {
            m_cursor_end.second = --m_cursor_begin.second;
            Erase(m_cursor_begin.first, m_cursor_begin.second, CP1);
            emit_signal = true;
        } else if (!GetLineData().empty() && 0 < m_cursor_begin.first) {
            m_cursor_end.first = --m_cursor_begin.first;
            m_cursor_begin.second = CPSize(GetLineData()[m_cursor_begin.first].char_data.size());
            if (LineEndsWithEndlineCharacter(GetLineData(), m_cursor_begin.first, Text()))
                --m_cursor_begin.second;
            m_cursor_end.second = m_cursor_begin.second;
            Erase(m_cursor_begin.first, m_cursor_begin.second, CP1);
            emit_signal = true;
        }
        break;
    }

    case GGK_DELETE: {
        if (MultiSelected()) {
            ClearSelected();
            emit_signal = true;
        } else if (!GetLineData().empty() && m_cursor_begin.second < GetLineData()[m_cursor_begin.first].char_data.size()) {
            // cursor is not at the end of its current line
            Erase(m_cursor_begin.first, m_cursor_begin.second, CP1);
            emit_signal = true;
        } else if (m_cursor_begin.first < GetLineData().size() - 1) {
            // cursor is not on the last line, but is at the end of its line
            size_t next_line = m_cursor_begin.first + 1;
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

    CPSize begin_cursor_pos = CharIndexOf(m_cursor_begin.first, m_cursor_begin.second);
    CPSize end_cursor_pos = CharIndexOf(m_cursor_end.first, m_cursor_end.second);
    this->m_cursor_pos = {begin_cursor_pos, end_cursor_pos};

    AdjustView();
    if (emit_signal)
        EditedSignal(Text());
}

void MultiEdit::TextInput(const std::string* text) {
    // Typed or pasted text. If typed, this function is called from a TextInput event after the
    // KeyPress event leads to a call to MultiEdit::KeyPress, which should itself do nothing.
    if (Disabled()) {
        TextControl::TextInput(text);
        return;
    }

    if (!text || !Interactive() || m_style & MULTI_READ_ONLY)
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
    //std::cout << "ClearSelected" << std::endl;
    //std::cout << "initial cursor begin: line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << std::endl;
    //std::cout << "initial cursor end:   line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;
    //std::cout << "text before erase:" << Text() << std::endl;

    // predetermine post-erase cursor position because erasing can mess up state of cursor position members
    std::pair<size_t, CPSize> low_pos = LowCursorPos();
    std::pair<size_t, CPSize> high_pos = HighCursorPos();

    Erase(low_pos.first, low_pos.second, high_pos.first, high_pos.second);
    //std::cout << "text after  erase:" << Text() << std::endl;
    //std::cout << "after erase cursor begin: line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << std::endl;
    //std::cout << "after erase cursor end:   line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;

    m_cursor_end = m_cursor_begin = low_pos;
    //std::cout << "low of cursor begin/end:  line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << std::endl;

    CPSize cursor_pos = CharIndexOf(m_cursor_end.first, m_cursor_end.second);
    //std::cout << "got cursor pos: " << cursor_pos << std::endl;
    this->m_cursor_pos = {cursor_pos, cursor_pos};
}

void MultiEdit::AdjustView()
{
    Pt cl_sz = ClientSize();
    Flags<TextFormat> format = GetTextFormat();
    X excess_width = m_contents_sz.x - cl_sz.x;
    Y excess_height = m_contents_sz.y - cl_sz.y;
    X horz_min(0);            // these are default values for MULTI_LEFT and MULTI_TOP
    X horz_max = excess_width;
    Y vert_min(0);
    Y vert_max = excess_height;

    if (format & FORMAT_RIGHT) {
        horz_min = -excess_width;
        horz_max = horz_min + m_contents_sz.x;
    } else if (format & FORMAT_CENTER) {
        horz_min = -excess_width / 2;
        horz_max = horz_min + m_contents_sz.x;
    }
    if ((format & FORMAT_BOTTOM) && 0 <= excess_height) {
        vert_min = -excess_height;
        vert_max = vert_min + m_contents_sz.y;
    }

    // make sure that m_first_row_shown and m_first_col_shown are within sane bounds
    if (excess_width <= 0 || !m_hscroll) {
        m_first_col_shown = X0;
    } else {
        m_hscroll->ScrollTo(Value(std::max(horz_min, std::min(m_first_col_shown, horz_max))));
        SignalScroll(*m_hscroll, true);
    }

    if (excess_height <= 0 || !m_vscroll) {
        m_first_row_shown = Y0;
    } else {
        m_vscroll->ScrollTo(Value(std::max(vert_min, std::min(m_first_row_shown, vert_max))));
        SignalScroll(*m_vscroll, true);
    }

    // adjust m_first_row_shown position to bring the cursor into view
    std::size_t first_fully_vis_row = FirstFullyVisibleRow();
    if (m_cursor_end.first < first_fully_vis_row && m_vscroll) {
        std::size_t diff = (first_fully_vis_row - m_cursor_end.first);
        m_vscroll->ScrollTo(Value(std::max(vert_min, m_first_row_shown) - GetFont()->Lineskip() * static_cast<int>(diff)));
        SignalScroll(*m_vscroll, true);
    }
    std::size_t last_fully_vis_row = LastFullyVisibleRow();
    if (last_fully_vis_row < m_cursor_end.first && m_vscroll) {
        std::size_t diff = (m_cursor_end.first - last_fully_vis_row);
        m_vscroll->ScrollTo(Value(std::min(m_first_row_shown + GetFont()->Lineskip() * static_cast<int>(diff), vert_max)));
        SignalScroll(*m_vscroll, true);
    }

    // adjust m_first_col_shown position to bring the cursor into view
    CPSize first_visible_char = FirstVisibleChar(m_cursor_end.first);
    CPSize last_visible_char = LastVisibleChar(m_cursor_end.first);
    X client_char_posn = RowStartX(m_cursor_end.first) + CharXOffset(m_cursor_end.first, m_cursor_end.second);
    if (client_char_posn < 0 && m_hscroll) { // if the caret is at a place left of the current visible area
        if (first_visible_char - m_cursor_end.second < 5) { // if the caret is fewer than five characters before first_visible_char
            // try to move the caret by five characters
            X five_char_distance =
                CharXOffset(m_cursor_end.first, first_visible_char) -
                CharXOffset(m_cursor_end.first, (5 < first_visible_char) ? first_visible_char - 5 : CP0);
            m_hscroll->ScrollTo(Value(m_first_col_shown - five_char_distance));
            SignalScroll(*m_hscroll, true);
        } else { // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_hscroll->ScrollTo(Value(horz_min + m_first_col_shown + client_char_posn));
            SignalScroll(*m_hscroll, true);
        }
    } else if (cl_sz.x <= client_char_posn && m_hscroll) { // if the caret is moving to a place right of the current visible area
        if (m_cursor_end.second - last_visible_char < 5) { // if the caret is fewer than five characters after last_visible_char
            // try to move the caret by five characters
            CPSize last_char_of_line = CodePointIndexOf(m_cursor_end.first, INVALID_CP_SIZE, GetLineData());
            X five_char_distance =
                CharXOffset(m_cursor_end.first, (last_visible_char + 5 < last_char_of_line) ? last_visible_char + 5 : last_char_of_line) -
                CharXOffset(m_cursor_end.first, last_visible_char);
            m_hscroll->ScrollTo(Value(m_first_col_shown + five_char_distance));
            SignalScroll(*m_hscroll, true);
        } else { // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_hscroll->ScrollTo(Value(std::min(horz_min + m_first_col_shown + client_char_posn, horz_max)));
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
        (m_first_row_shown ||
         (m_contents_sz.y > cl_sz.y ||
          (m_contents_sz.y > cl_sz.y - INT_SCROLL_WIDTH &&
           m_contents_sz.x > cl_sz.x - INT_SCROLL_WIDTH)));
    bool need_horz =
        !(m_style & MULTI_NO_HSCROLL) &&
        (m_first_col_shown ||
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

    const int GAP = PIXEL_MARGIN - 2; // the space between the client area and the border

    const auto& style = GetStyleFactory();

    Y vscroll_min = (m_style & MULTI_TERMINAL_STYLE) ? cl_sz.y - m_contents_sz.y : Y0;
    if (cl_sz.y - m_contents_sz.y > 0 ) {
        vscroll_min = Y0;
    }
    X hscroll_min(0); // default value for MULTI_LEFT
    if (m_style & MULTI_RIGHT) {
        hscroll_min = -excess_width;
    } else if (m_style & MULTI_CENTER) {
        hscroll_min = -excess_width / 2;
    }
    Y vscroll_max = vscroll_min + m_contents_sz.y - 1;
    X hscroll_max = hscroll_min + m_contents_sz.x - 1;

    const int INT_GAP = static_cast<int>(GAP);
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
            Y scroll_y(-GAP);
            m_vscroll->SizeMove(Pt(scroll_x, scroll_y),
                                Pt(scroll_x + INT_SCROLL_WIDTH,
                                   scroll_y + cl_sz.y + 2 * INT_GAP - (need_horz ? INT_SCROLL_WIDTH : 0)));
        }
    } else if (!m_vscroll && need_vert) { // if scroll doesn't exist but is needed
        m_vscroll = style->NewMultiEditVScroll(m_color, CLR_SHADOW);
        m_vscroll->MoveTo(Pt(cl_sz.x + INT_GAP - INT_SCROLL_WIDTH, Y(-GAP)));
        m_vscroll->Resize(Pt(X(SCROLL_WIDTH), cl_sz.y + 2 * INT_GAP - (need_horz ? INT_SCROLL_WIDTH : 0)));

        unsigned int line_size = (m_vscroll_wheel_scroll_increment != 0
                                    ? m_vscroll_wheel_scroll_increment
                                    : Value(GetFont()->Lineskip())*4);

        unsigned int page_size = std::abs(Value(cl_sz.y - (need_horz ? INT_SCROLL_WIDTH : 0)));

        m_vscroll->SizeScroll(Value(vscroll_min), Value(vscroll_max),
                              line_size, std::max(line_size, page_size));
        AttachChild(m_vscroll);
        m_vscroll->ScrolledSignal.connect(
            boost::bind(&MultiEdit::VScrolled, this, _1, _2, _3, _4));
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
            X scroll_x(-GAP);
            Y scroll_y = cl_sz.y + INT_GAP - INT_SCROLL_WIDTH;
            m_hscroll->SizeMove(Pt(scroll_x, scroll_y),
                                Pt(scroll_x + cl_sz.x + 2 * INT_GAP - (need_vert ? INT_SCROLL_WIDTH : 0),
                                   scroll_y + INT_SCROLL_WIDTH));
        }
    } else if (!m_hscroll && need_horz) { // if scroll doesn't exist but is needed
        m_hscroll = style->NewMultiEditHScroll(m_color, CLR_SHADOW);
        m_hscroll->MoveTo(Pt(X(-GAP), cl_sz.y + GAP - INT_SCROLL_WIDTH));
        m_hscroll->Resize(Pt(cl_sz.x + 2 * GAP - (need_vert ? INT_SCROLL_WIDTH : 0), Y(SCROLL_WIDTH)));

        unsigned int line_size = (m_hscroll_wheel_scroll_increment != 0
                                    ? m_hscroll_wheel_scroll_increment
                                    : Value(GetFont()->Lineskip())*4);

        unsigned int page_size = std::abs(Value(cl_sz.x - (need_vert ? INT_SCROLL_WIDTH : 0)));

        m_hscroll->SizeScroll(Value(hscroll_min), Value(hscroll_max),
                              line_size, std::max(line_size, page_size));
        AttachChild(m_hscroll);
        m_hscroll->ScrolledSignal.connect(
            boost::bind(&MultiEdit::HScrolled, this, _1, _2, _3, _4));
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
{ m_first_row_shown = Y(upper); }

void MultiEdit::HScrolled(int upper, int lower, int range_upper, int range_lower)
{ m_first_col_shown = X(upper); }

void MultiEdit::AcceptPastedText(const std::string& text)
{
    // Edit doesn't know about the MultiEdit style, in which read-only is
    // encoded, so check it here before passing to the default behaviour
    if (m_style & MULTI_READ_ONLY)
        return;

    bool modified_text = false;

    //std::cout << "before modifying text, cursor begin at line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << std::endl;
    //std::cout << "before modifying text, cursor end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;
    //std::cout << "before modifying text, cursor pos: " << this->m_cursor_pos.first << " // " << this->m_cursor_pos.second << std::endl;

    if (MultiSelected()) {
        ClearSelected();
        modified_text = true;
        //std::cout << "right after clearing, cursor begin at line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << std::endl;
        //std::cout << "right after clearing, cursor end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;
        //std::cout << "right after clearing, cursor pose: " << this->m_cursor_pos.first << " // " << this->m_cursor_pos.second << std::endl;
        m_cursor_pos.second = m_cursor_pos.first;
    }

    if (!text.empty()) {
        Insert(m_cursor_pos.first, text);
        modified_text = true;
        //std::cout << "Accepted and Inserted, new text:" << Text() << std::endl;
        //std::cout << "right after inserting, cursor begin at line: " << m_cursor_begin.first << " char: " << m_cursor_begin.second << std::endl;
        //std::cout << "right after inserting, cursor end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;
        //std::cout << "right after inserting, cursor pose: " << this->m_cursor_pos.first << " // " << this->m_cursor_pos.second << std::endl;
    }

    if (modified_text) {
        // moves cursor to end of pasted text
        //std::cout << "initial cursor pos: " << m_cursor_pos.first << " - " << m_cursor_pos.second << std::endl;
        CPSize text_span(utf8::distance(text.begin(), text.end()));
        //std::cout << "text span: " << text_span << std::endl;
        CPSize new_cursor_pos = std::max(CP0, std::min(Length(), m_cursor_pos.second + text_span));
        //std::cout << "new cursor pos: " << new_cursor_pos << std::endl;

        // place cursor start and end after text (at same place so nothing is selected)
        m_cursor_pos.second = new_cursor_pos;
        m_cursor_pos.first = m_cursor_pos.second;

        // convert Edit::m_cursor_pos to equivalent MultiEdit::m_cursor_begin and MultiEdit::m_cursor_end
        m_cursor_begin = CharAt(m_cursor_pos.first);
        m_cursor_end = m_cursor_begin;
        //std::cout << "after positioning at cursor pos, cursor begin/end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;

        // the cursor might be off the bottom if the bottom row was just
        // chopped off to satisfy m_max_lines_history
        if (GetLineData().empty()) {
            m_cursor_begin.first = 0;
            m_cursor_begin.second = CP0;
        } else if (GetLineData().size() - 1 < m_cursor_begin.first) {
            m_cursor_begin.first = GetLineData().size() - 1;
            m_cursor_begin.second = CPSize(GetLineData()[m_cursor_begin.first].char_data.size());
        }
        m_cursor_end = m_cursor_begin;

        //std::cout << "after newline checks, cursor begin/end at line: " << m_cursor_end.first << " char: " << m_cursor_end.second << std::endl;
    }

    CPSize begin_cursor_pos = CharIndexOf(m_cursor_begin.first, m_cursor_begin.second);
    CPSize end_cursor_pos = CharIndexOf(m_cursor_end.first, m_cursor_end.second);
    this->m_cursor_pos = {begin_cursor_pos, end_cursor_pos};

    //std::cout << "after converting cursor begin/end to cursor pos, cursor pos: " << m_cursor_pos.first << " - " << m_cursor_pos.second << std::endl;

    AdjustView();
    if (modified_text)
        EditedSignal(Text());
}

