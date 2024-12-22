//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2024 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/DrawUtil.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/utf8/checked.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    Y HeightFromFont(const std::shared_ptr<Font>& font, unsigned int pixel_margin) noexcept
    { return font->Height() + 2 * static_cast<int>(pixel_margin); }
}

////////////////////////////////////////////////
// GG::Edit
////////////////////////////////////////////////

Edit::Edit(std::string str, std::shared_ptr<Font> font,
           Clr color, Clr text_color, Clr interior) :
    TextControl(X0, Y0, X1, HeightFromFont(font, PIXEL_MARGIN), "", font,
                text_color, FORMAT_LEFT | FORMAT_IGNORETAGS, INTERACTIVE | REPEAT_KEY_PRESS),
    m_int_color(interior)
{
    Edit::SetColor(color);
    Edit::SetText(std::move(str));

    if (INSTRUMENT_ALL_SIGNALS) {
        EditedSignal.connect([](const std::string& str) { std::cerr << "GG SIGNAL : Edit::EditedSignal (str=" << str << ")" << std::endl; });
        FocusUpdateSignal.connect([](const std::string& str) { std::cerr << "GG SIGNAL : Edit::FocusUpdateSignal (str=" << str << ")" << std::endl; });
    }
}

Pt Edit::MinUsableSize() const noexcept
{ return Pt(X(4 * PIXEL_MARGIN), HeightFromFont(GetFont(), PIXEL_MARGIN)); }

std::string_view Edit::SelectedText() const
{
    return Text(m_cursor_pos.first, m_cursor_pos.second);
}

void Edit::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    Clr hilite_color_to_use = Disabled() ? DisabledColor(m_hilite_color) : m_hilite_color;
    Clr text_color_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();

    Pt ul = UpperLeft(), lr = LowerRight();
    Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    BeveledRectangle(ul, lr, int_color_to_use, color_to_use, false, 2);

    BeginScissorClipping(Pt(client_ul.x - 1, client_ul.y), client_lr);

    const auto& font = GetFont();
    const auto& line_data = GetLineData();

    X first_char_offset = FirstCharOffset();
    Y text_y_pos = ToY(ul.y + ((lr.y - ul.y) - font->Height()) / 2.0);
    CPSize last_visible_char = LastVisibleChar();
    const StrSize INDEX_0 = StringIndexOfLineAndGlyph(0, m_first_char_shown, line_data);
    const StrSize INDEX_END = StringIndexOfLineAndGlyph(0, last_visible_char, line_data);
    Font::RenderState rs{text_color_to_use};

    const auto text_sv = std::string_view(Text()).substr(Value(INDEX_0), Value(INDEX_END - INDEX_0));

    if (!line_data.empty() && MultiSelected()) {
        const auto& char_data = line_data.front().char_data;

        // if one or more chars are selected, hilite, then draw the range in the selected-text color
        CPSize low_cursor_pos  = std::min(CPSize(char_data.size()), std::min(m_cursor_pos.first, m_cursor_pos.second));
        CPSize high_cursor_pos = std::min(CPSize(char_data.size()), std::max(m_cursor_pos.first, m_cursor_pos.second));

        // draw hilighting background box
        Pt hilite_ul(client_ul.x + ((low_cursor_pos < CP1) ? X0 : char_data.at(Value(low_cursor_pos - CP1)).extent) - first_char_offset, client_ul.y);
        Pt hilite_lr(client_ul.x + ((high_cursor_pos < CP1) ? X0 : char_data.at(Value(high_cursor_pos - CP1)).extent) - first_char_offset, client_lr.y);
        FlatRectangle(hilite_ul, hilite_lr, hilite_color_to_use, CLR_ZERO, 0);

        // draw text
        font->RenderText(Pt(client_ul.x, text_y_pos), text_sv, rs);

    } else { // no selected text
        font->RenderText(Pt(client_ul.x, text_y_pos), text_sv, rs);

        if (GUI::GetGUI()->FocusWnd().get() == this) {
            // if we have focus, draw the caret as a simple vertical line
            X caret_x = ScreenPosOfChar(m_cursor_pos.second);
            Line(caret_x, client_ul.y, caret_x, client_lr.y, text_color_to_use);
        }
    }

    EndScissorClipping();
}

void Edit::SelectAll()
{
    m_cursor_pos.first = Length();
    m_cursor_pos.second = CP0;
    AdjustView();
}

void Edit::DeselectAll()
{
    m_cursor_pos.first = CP0;
    m_cursor_pos.second = CP0;
    AdjustView();
}

void Edit::SelectRange(CPSize from, CPSize to)
{
    //std::cout << "Selecting range from: " << from << "  to: " << to << std::endl;
    if (from < to) {
        m_cursor_pos.first = std::max(CP0, from);
        m_cursor_pos.second = std::min(to, Length());
    } else {
        m_cursor_pos.first = std::min(from, Length());
        m_cursor_pos.second = std::max(CP0, to);
    }
    AdjustView();
}

void Edit::SetText(std::string str)
{
    TextControl::SetText(std::move(str));
    m_cursor_pos.second = m_cursor_pos.first; // eliminate any hiliting

    // make sure the change in text did not make the cursor or view position invalid
    if (Text().empty() || GetLineData().empty() ||
        CPSize{GetLineData().front().char_data.size()} < m_cursor_pos.first)
    {
        m_first_char_shown = CP0;
        m_cursor_pos = {CP0, CP0};
    }

    m_recently_edited = true;
}

void Edit::AcceptPastedText(const std::string& text)
{
    if (!Interactive())
        return;
    if (!utf8::is_valid(text.begin(), text.end())) {
        std::cerr << "Pasted text is not valid UTF-8:" << text << std::endl;
        return;
    }

    bool modified_text = false;

    if (MultiSelected()) {
        ClearSelected();
        modified_text = true;
        m_cursor_pos.second = m_cursor_pos.first;
    }

    if (!text.empty()) {
        Insert(m_cursor_pos.first, text);
        modified_text = true;
    }

    if (modified_text) {
        // moves cursor to end of pasted text
        const CPSize text_span{static_cast<std::size_t>(utf8::distance(text.begin(), text.end()))}; // TODO: this looks wrong... CPSize should be code points or glyphs, not char (byte) index in string
        m_cursor_pos.second = std::max(CP0, std::min(Length(), m_cursor_pos.second + text_span));

        // ensure nothing is selected after pasting
        m_cursor_pos.first = m_cursor_pos.second;

        // notify rest of GUI of change to text in this Edit
        EditedSignal(Text());
    }
}

CPSize Edit::GlyphIndexAt(X x) const
{ return GG::GlyphIndexOfXOnLine0(GetLineData(), x, FirstCharOffset()); }

CPSize Edit::CPIndexOfGlyphAt(X x) const
{ return GG::CodePointIndexOfXOnLine0(GetLineData(), x, FirstCharOffset()); }

X Edit::FirstCharOffset() const
{
    const auto& line_data{GetLineData()};
    if (line_data.empty() || m_first_char_shown == CP0)
        return X0;

    const auto& char_data{line_data.front().char_data};
    if (char_data.empty())
        return X0;

    const auto first_char_shown = (m_first_char_shown > CP0) ? (m_first_char_shown - CP1) : CP0;
    const auto char_idx = std::min(char_data.size() - 1, Value(first_char_shown));
    return char_data.at(char_idx).extent;
}

X Edit::ScreenPosOfChar(CPSize idx) const
{
    const auto& line_data{GetLineData()};
    if (line_data.empty())
        return ClientUpperLeft().x;

    X line_first_char_x = ClientUpperLeft().x - FirstCharOffset();
    if (idx == CP0)
        return line_first_char_x;

    const auto& char_data = line_data.front().char_data;
    if (char_data.empty())
        return line_first_char_x;

    // get index of previous character to the location of the requested char
    // get the extent to the right of that char to get the left position of the requested char
    auto char_idx = std::min(char_data.size() - 1, Value(idx - CP1));
    X line_extent_to_right_of_idx_char = char_data.at(char_idx).extent;

    return line_first_char_x + line_extent_to_right_of_idx_char;
}

CPSize Edit::LastVisibleChar() const
{
    const auto& line_data = GetLineData();
    if (line_data.empty())
        return CP0;
    const auto& char_data = line_data.front().char_data;

    const CPSize line_limit = std::min(Length(), CPSize(char_data.size()));
    const X client_size_x = ClientSize().x;
    const X first_char_offset = FirstCharOffset();

    CPSize retval = m_first_char_shown;
    for (; retval < line_limit; ++retval) {
        if (retval == CP0) {
            if (client_size_x <= X0 - first_char_offset)
                break;
        } else {
            const std::size_t retval_minus_1 = Value(retval - CP1);
            const auto retval_minus_1_char_data{char_data.at(retval_minus_1)};
            if (client_size_x <= retval_minus_1_char_data.extent - first_char_offset)
                break;
        }
    }
    return retval;
}

std::size_t Edit::NumLines() const noexcept
{
    const auto ldsz = GetLineData().size();
    return (ldsz > 0) ? (ldsz - 1) : 0;
}

void Edit::LButtonDown(Pt pt, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;
    //std::cout << "Edit::LButtonDown start" << std::endl;
    const X click_xpos = ScreenToClient(pt).x; // x coord of click within text space
    const CPSize idx = GlyphIndexAt(click_xpos);
    //std::cout << "Edit::LButtonDown got idx: " << idx << std::endl;

    const auto word_indices = GetDoubleButtonDownWordIndices(idx);
    if (word_indices.first != word_indices.second)
        m_cursor_pos = word_indices;
    else
        m_cursor_pos = {idx, idx};
}

void Edit::LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys)
{
    if (Disabled())
        return;

    const X xpos = ScreenToClient(pt).x; // x coord for mouse position within text space
    const CPSize cp_idx = CPIndexOfGlyphAt(xpos);
    //std::cout << "CPIndexAt mouse x-pos: " << xpos << std::endl;

    if (m_in_double_click_mode) {
        const auto word_indices = GetDoubleButtonDownDragWordCPIndices(cp_idx);

        if (word_indices.first == word_indices.second) {
            if (cp_idx < m_double_click_cursor_pos.first) {
                m_cursor_pos.second = cp_idx;
                m_cursor_pos.first = m_double_click_cursor_pos.second;
            } else if (m_double_click_cursor_pos.second < cp_idx) {
                m_cursor_pos.second = cp_idx;
                m_cursor_pos.first = m_double_click_cursor_pos.first;
            } else {
                m_cursor_pos = m_double_click_cursor_pos;
            }
        } else {
            if (word_indices.first <= m_double_click_cursor_pos.first) {
                m_cursor_pos.second = word_indices.first;
                m_cursor_pos.first = m_double_click_cursor_pos.second;
            } else {
                m_cursor_pos.second = word_indices.second;
                m_cursor_pos.first = m_double_click_cursor_pos.first;
            }
        }
    } else {
        // when a single-click drag occurs, move m_cursor_pos.second to where the mouse is,
        // which selects a range of characters
        m_cursor_pos.second = cp_idx;
        if (xpos < X0 || ClientSize().x < xpos) // if we're dragging past the currently visible text
            AdjustView();
    }

    //std::cout << "LDrag at glyph: " << Value(GlyphIndexAt(xpos))
    //          << " selected from cp idx: " << Value(m_cursor_pos.first)
    //          << " to cp idx: " << Value(m_cursor_pos.second) << std::endl;
}

void Edit::LButtonUp(Pt pt, Flags<ModKey> mod_keys)
{ ClearDoubleButtonDownMode(); }

void Edit::LClick(Pt pt, Flags<ModKey> mod_keys)
{ ClearDoubleButtonDownMode(); }

void Edit::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (Disabled()) {
        TextControl::KeyPress(key, key_code_point, mod_keys);
        return;
    }

    const bool shift_down = mod_keys & (MOD_KEY_LSHIFT | MOD_KEY_RSHIFT);
    const bool ctrl_down = mod_keys & (MOD_KEY_CTRL | MOD_KEY_RCTRL);
    const bool numlock_on = mod_keys & MOD_KEY_NUM;

    bool emit_signal = false;

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

    switch (key) {
    case Key::GGK_HOME:
        m_first_char_shown = CP0;
        if (shift_down)
            m_cursor_pos.second = CP0;
        else
            m_cursor_pos.second = m_cursor_pos.first = CP0;
        break;
    case Key::GGK_LEFT:
        if (MultiSelected() && !shift_down) {
            if (!ctrl_down)
                m_cursor_pos.second = m_cursor_pos.first = std::min(m_cursor_pos.first, m_cursor_pos.second);
            else
                m_cursor_pos.second = m_cursor_pos.first = NextWordEdgeFrom(Text(), m_cursor_pos.second, false);

        } else if (CP0 < m_cursor_pos.second) {
            if (!ctrl_down) {
                const auto& ld{GetLineData()};
                if (ld.empty()) {
                    m_cursor_pos.second = CP0;
                } else  {
                    const auto& ld0cd{ld.front().char_data};

                    m_cursor_pos.second = std::min(m_cursor_pos.second - CP1, CPSize(ld0cd.size()));

                    const X extent = ld0cd.empty() ? X0 : 
                        (Value(m_cursor_pos.second) < ld0cd.size()) ? ld0cd.at(Value(m_cursor_pos.second)).extent :
                        ld0cd.back().extent;

                    const auto not_at_start = [sz{ld0cd.size()}](CPSize idx)
                    { return idx > CP0; };
                    const auto extent_before_pos = [&ld0cd](CPSize idx)
                    {
                        return (idx <= CP1 || ld0cd.empty()) ? X0 :
                            (Value(idx) >= ld0cd.size()) ? ld0cd.back().extent :
                            ld0cd.at(Value(idx - CP1)).extent;
                    };

                    // move end of selection left until it move past any zero-with glyphs or it reaches the start of the line
                    while (not_at_start(m_cursor_pos.second) && extent == extent_before_pos(m_cursor_pos.second))
                        --m_cursor_pos.second;
                }
            } else {
                m_cursor_pos.second = NextWordEdgeFrom(Text(), m_cursor_pos.second, false);
            }
            if (!shift_down)
                m_cursor_pos.first = m_cursor_pos.second;
        }
        AdjustView();
        break;
    case Key::GGK_RIGHT:
        if (MultiSelected() && !shift_down) {
            if (!ctrl_down)
                m_cursor_pos.second = m_cursor_pos.first = std::max(m_cursor_pos.first, m_cursor_pos.second);
            else
                m_cursor_pos.second = m_cursor_pos.first = NextWordEdgeFrom(Text(), m_cursor_pos.second, true);

        } else if (m_cursor_pos.second < Length()) {
            if (!ctrl_down) {
                const auto& ld{GetLineData()};
                if (!ld.empty()) {
                    const auto& ld0cd{ld.front().char_data};

                    const X extent = ld0cd.empty() ? X0 :
                        (Value(m_cursor_pos.second) < ld0cd.size()) ? ld0cd.at(Value(m_cursor_pos.second)).extent :
                        ld0cd.back().extent;

                    const auto not_at_end = [sz{ld0cd.size()}](CPSize idx)
                    { return Value(idx) < sz; };
                    const auto extent_at_pos = [&ld0cd](CPSize idx)
                    {
                        return (Value(idx) < ld0cd.size()) ? ld0cd.at(Value(idx)).extent :
                            ld0cd.empty() ? X0 : ld0cd.back().extent;
                    };

                    // move end of selection right until it moves past any zero-length glyphs or it reaches the end of the line
                    while (not_at_end(m_cursor_pos.second) && extent == extent_at_pos(m_cursor_pos.second))
                        ++m_cursor_pos.second;
                }
            } else {
                m_cursor_pos.second = NextWordEdgeFrom(Text(), m_cursor_pos.second, true);
            }
            if (!shift_down)
                m_cursor_pos.first = m_cursor_pos.second;
        }
        AdjustView();
        break;
    case Key::GGK_END:
        if (shift_down)
            m_cursor_pos.second = Length();
        else
            m_cursor_pos.second = m_cursor_pos.first = Length();
        AdjustView();
        break;
    case Key::GGK_BACKSPACE:
        if (MultiSelected()) {
            ClearSelected();
            emit_signal = true;
        } else if (CP0 < m_cursor_pos.first) {
            m_cursor_pos.second = --m_cursor_pos.first;
            Erase(0, m_cursor_pos.first);
            emit_signal = true;
        }
        AdjustView();
        break;
    case Key::GGK_DELETE:
        if (MultiSelected()) {
            ClearSelected();
            emit_signal = true;
        } else if (m_cursor_pos.first < Length()) {
            Erase(m_cursor_pos.first);
            emit_signal = true;
        }
        AdjustView();
        break;
    case Key::GGK_RETURN:
    case Key::GGK_KP_ENTER:
        FocusUpdateSignal(Text());
        TextControl::KeyPress(key, key_code_point, mod_keys);
        m_recently_edited = false;
        break;
    default:
        // Do actual text input in TextInput.
        break;
    }

    if (emit_signal)
        EditedSignal(Text());
}

void Edit::TextInput(const std::string& text) {
    if (Disabled()) {
        TextControl::TextInput(text);
        return;
    }

    if (text.empty() || !Interactive())
        return;

    AcceptPastedText(text);

    if (LastVisibleChar() <= m_cursor_pos.first)
        AdjustView();
}

void Edit::GainingFocus()
{ m_recently_edited = false; }

void Edit::LosingFocus()
{
    if (m_recently_edited)
        FocusUpdateSignal(Text());
}

std::pair<CPSize, CPSize> Edit::GetDoubleButtonDownWordIndices(CPSize cp_index)
{
    const auto ticks = GUI::GetGUI()->Ticks();
    if (ticks - m_last_button_down_time <= GUI::GetGUI()->DoubleClickInterval())
        m_in_double_click_mode = true;
    m_last_button_down_time = ticks;

    m_double_click_cursor_pos = std::pair<CPSize, CPSize>(cp_index, cp_index);
    if (m_in_double_click_mode)
        m_double_click_cursor_pos = GetDoubleButtonDownDragWordCPIndices(cp_index);

    return m_double_click_cursor_pos;
}

std::pair<CPSize, CPSize> Edit::GetDoubleButtonDownDragWordCPIndices(CPSize cp_index)
{
    const auto words_cp_indices = GUI::GetGUI()->FindWords(Text());
    const auto it = std::find_if(words_cp_indices.begin(), words_cp_indices.end(),
                                 [cp_index](auto word) { return word.first < cp_index && cp_index < word.second; });
    return (it != words_cp_indices.end()) ? *it : std::pair<CPSize, CPSize>{cp_index, cp_index};
}

void Edit::ClearDoubleButtonDownMode()
{ m_in_double_click_mode = false; }

void Edit::ClearSelected()
{
    const CPSize low = std::min(m_cursor_pos.first, m_cursor_pos.second);
    const CPSize high = std::max(m_cursor_pos.first, m_cursor_pos.second);
    if (m_cursor_pos.first < m_cursor_pos.second)
        m_cursor_pos.second = m_cursor_pos.first;
    else
        m_cursor_pos.first = m_cursor_pos.second;
    Erase(0, low, high - low);

    const auto& line_data = GetLineData();

    // make sure deletion has not left m_first_char_shown in an out-of-bounds position
    if (line_data.empty() || line_data.front().char_data.empty())
        m_first_char_shown = CP0;
    else if (line_data.front().char_data.size() <= Value(m_first_char_shown))
        m_first_char_shown = CodePointIndexOfLineAndGlyph(0, INVALID_CP_SIZE, line_data);
}

void Edit::AdjustView()
{
    const X text_horizontal_space = ClientSize().x;
    const X first_char_offset = FirstCharOffset();
    static constexpr CPSize CP5{5};

    if (m_cursor_pos.second < m_first_char_shown) { // if the caret is at a place left of the current visible area
        if (m_first_char_shown - m_cursor_pos.second < CP5) // if the caret is less than five characters before m_first_char_shown
            m_first_char_shown = (CP5 < m_first_char_shown) ? m_first_char_shown - CP5 : CP0; // try to move the caret by five characters
        else // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_first_char_shown = m_cursor_pos.second;
        return;
    }

    if (Length() == CP0)
        return;

    const auto& ld{GetLineData()};
    if (ld.empty())
        return;

    const auto& char_data{ld.front().char_data};
    const auto cursor_char_rel_extent = (m_cursor_pos.second == CP0 || char_data.empty()) ? X0 :
        (Value(m_cursor_pos.second) < char_data.size()) ? char_data.at(Value(m_cursor_pos.second - CP1)).extent :
        char_data.back().extent;
    const auto char_data_extent = cursor_char_rel_extent - first_char_offset;

    if (text_horizontal_space > char_data_extent)
        return; // don't need more space

    // need to adjust range of shown chars to include carat in visible range of chars

    // try to move the text by five characters, or to the end if caret is at a location before the end - 5th character
    const auto last_shown_idx = std::min(m_cursor_pos.second + CP5,
                                         char_data.empty() ? CP0 : CPSize(char_data.size() - 1u));
    const auto last_shown_extent = (Value(last_shown_idx) >= char_data.size()) ?
        X0 : char_data.at(Value(last_shown_idx)).extent;
    auto pixels_to_move = last_shown_extent - first_char_offset - text_horizontal_space;
    if (Value(last_shown_idx + CP1) >= char_data.size()) {
        const auto space_width = GetFont()->SpaceWidth();
        const auto extra_spaces = static_cast<int>(Value(m_cursor_pos.second + CP5 - CP1)) - static_cast<int>(char_data.size());
        pixels_to_move += space_width * extra_spaces;
    }

    CPSize move_to = m_first_char_shown;
    while (move_to < CPSize{char_data.size()} &&
           char_data.at(Value(move_to)).extent - first_char_offset < pixels_to_move)
    { ++move_to; }

    m_first_char_shown = move_to;
}


////////////////////////////////////////////////////////////
// Free Functions
////////////////////////////////////////////////////////////
void GG::GetTranslatedCodePoint(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys,
                                std::string& translated_code_point)
{
    // only process it if it's a valid code point or a known printable
    // key, and no significant modifiers are in use
    if (key_code_point) {
        try {
            uint32_t chars[] = { key_code_point };
            utf8::utf32to8(chars, chars + 1, std::back_inserter(translated_code_point));
        } catch (const utf8::invalid_code_point&) {
            translated_code_point.clear();
        }
    }
}

CPSize GG::NextWordEdgeFrom(std::string_view text, CPSize from_position, bool search_right) {
    auto words = GUI::GetGUI()->FindWords(text);
    CPSize retval = CP0;

    if (!search_right) {
        // searching ON THE LEFT from the reference position

        // start with the leftmost word, traverse words to the right
        // until past the reference point
        for (const auto& word_range : words) {
            if (word_range.first > from_position) {
                // found word is after of the position. can stop
                // searching and use whatever the last found word's position was
                break;

            } else if (word_range.first < from_position && word_range.second >= from_position) {
                // found word starting before and ending at/after the position. can
                // stop searching and use the start of the found word.
                retval = word_range.first;
                break;

            } else if (word_range.second < from_position) {
                // found word ending before the position. can use the start
                // or end of the found word...
                if (word_range.second < from_position - CP1) {
                    // there is a gap between the end of the word and the search
                    // reference position. use one past the end of the word
                    retval = word_range.second + CP1;
                    // don't break, as there might be later words that are closer to
                    // the search reference position
                } else {
                    // the end of the word is immediately before the search
                    // reference position. use the start of the word.
                    retval = word_range.first;
                    // can stop searching since the word is right next to the
                    // search reference position
                    break;
                }
            }
        }
        return retval;

    } else {
        // searching ON THE RIGHT from the reference position
        if (!words.empty())
            retval = std::max(from_position, words.rbegin()->second);

        // start and the rightmost end, traverse the words leftwards
        // until past the reference point
        for (auto rit = words.rbegin(); rit != words.rend(); ++rit) {
            if (rit->second < from_position) {
                // found word is before the position. can stop
                // searching and use whatever the last found word's position was
                break;

            } else if (rit->first <= from_position && rit->second > from_position) {
                // found word starting before/at and ending after the position. can
                // stop searching and use the end of the found word.
                retval = rit->second;
                break;

            } else if (rit->first > from_position) {
                // found word starting after the position. can use the start
                // or end of the found word...
                if (rit->first > from_position + CP1) {
                    // there is a gap between the end of the word and the search
                    // reference position. use one before the start of the word
                    retval = rit->first - CP1;
                } else {
                    // the start of the word is immediately after the search
                    // reference position. use the end of the word.
                    retval = rit->second;
                    // can stop searching since the word is right next to the
                    // search reference position
                    break;
                }
                // don't break, as there might be later words that are closer to
                // the search reference position
            }
        }

        return retval;
    }
}

