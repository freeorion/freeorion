//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
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
    struct EditedEcho
    {
        EditedEcho(const std::string& name) : m_name(name) {}
        void operator()(const std::string& str)
        { std::cerr << "GG SIGNAL : " << m_name << "(str=" << str << ")" << std::endl; }
        std::string m_name;
    };

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
        EditedSignal.connect(EditedEcho("Edit::EditedSignal"));
        FocusUpdateSignal.connect(EditedEcho("Edit::FocusUpdateSignal"));
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

    X first_char_offset = FirstCharOffset();
    Y text_y_pos = ToY(ul.y + ((lr.y - ul.y) - GetFont()->Height()) / 2.0);
    CPSize last_visible_char = LastVisibleChar();
    const StrSize INDEX_0 = StringIndexOfLineAndGlyph(0, m_first_char_shown, GetLineData());
    const StrSize INDEX_END = StringIndexOfLineAndGlyph(0, last_visible_char, GetLineData());
    Font::RenderState rs{text_color_to_use};
    const auto& font = GetFont();


    if (!GetLineData().empty() && MultiSelected()) {
        const auto& char_data = GetLineData()[0].char_data;

        // if one or more chars are selected, hilite, then draw the range in the selected-text color
        CPSize low_cursor_pos  = std::min(CPSize(char_data.size()), std::max(CP0, std::min(m_cursor_pos.first, m_cursor_pos.second)));
        CPSize high_cursor_pos = std::min(CPSize(char_data.size()), std::max(CP0, std::max(m_cursor_pos.first, m_cursor_pos.second)));

        // draw hilighting background box
        Pt hilite_ul(client_ul.x + (low_cursor_pos < CP1 ? X0 : char_data[Value(low_cursor_pos - CP1)].extent) - first_char_offset, client_ul.y);
        Pt hilite_lr(client_ul.x + (high_cursor_pos < CP1 ? X0 : char_data[Value(high_cursor_pos - CP1)].extent) - first_char_offset, client_lr.y);
        FlatRectangle(hilite_ul, hilite_lr, hilite_color_to_use, CLR_ZERO, 0);

        // draw text
        font->RenderText(Pt(client_ul.x, text_y_pos), Text().substr(Value(INDEX_0), Value(INDEX_END - INDEX_0)), rs);

    } else { // no selected text
        font->RenderText(Pt(client_ul.x, text_y_pos), Text().substr(Value(INDEX_0), Value(INDEX_END - INDEX_0)), rs);

        if (GUI::GetGUI()->FocusWnd().get() == this) {
            // if we have focus, draw the caret as a simple vertical line
            X caret_x = ScreenPosOfChar(m_cursor_pos.second);
            Line(caret_x, client_ul.y, caret_x, client_lr.y, text_color_to_use);
        }
    }

    EndScissorClipping();
}

void Edit::SetColor(Clr c)
{ Control::SetColor(c); }

void Edit::SetInteriorColor(Clr c)
{ m_int_color = c; }

void Edit::SetHiliteColor(Clr c)
{ m_hilite_color = c; }

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
        CPSize{GetLineData()[0].char_data.size()} < m_cursor_pos.first)
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

CPSize Edit::GlyphIndexOf(X x) const
{
    CPSize retval;
    const X first_char_offset = FirstCharOffset();
    for (retval = CP0; retval < Length(); ++retval) {
        X curr_extent;
        if (x + first_char_offset <= (curr_extent = GetLineData()[0].char_data[Value(retval)].extent)) { // the point falls within the character at index retval
            const X prev_extent = retval != CP0 ? GetLineData()[0].char_data[Value(retval - CP1)].extent : X0;
            const X half_way = (prev_extent + curr_extent) / 2;
            if (half_way <= x + first_char_offset) // if the point is more than halfway across the character, put the cursor *after* the character
                ++retval;
            break;
        }
    }
    return retval;
}

X Edit::FirstCharOffset() const
{
    const auto& line_data{GetLineData()};
    if (line_data.empty() || m_first_char_shown == CP0)
        return X0;

    const auto& char_data{line_data.front().char_data};
    if (char_data.empty())
        return X0;

    auto char_idx = std::min(char_data.size() - 1, Value(m_first_char_shown) - 1);
    return char_data[char_idx].extent;
}

X Edit::ScreenPosOfChar(CPSize idx) const
{
    const auto& line_data{GetLineData()};
    if (line_data.empty())
        return ClientUpperLeft().x;

    X line_first_char_x = ClientUpperLeft().x - FirstCharOffset();
    if (idx == CP0)
        return line_first_char_x;

    const auto& char_data{line_data.front().char_data};
    // get index of previous character to the location of the requested char
    // get the extent to the right of that char to get the left position of the requested char
    auto char_idx = std::min(char_data.size() - 1, Value(idx) - 1);
    X line_extent_to_idx_char = char_data[char_idx].extent;

    return line_first_char_x + line_extent_to_idx_char;
}

CPSize Edit::LastVisibleChar() const
{
    const auto& line_data = GetLineData();
    if (line_data.empty())
        return CP0;
    const auto& char_data = line_data[0].char_data;

    const CPSize line_limit = std::min(Length(), CPSize(char_data.size()));
    const X client_size_x = ClientSize().x;
    const X first_char_offset = FirstCharOffset();

    CPSize retval = m_first_char_shown;
    for (; retval < line_limit; ++retval) {
        if (retval == CP0) {
            if (client_size_x <= X0 - first_char_offset)
                break;
        } else {
            const std::size_t retval_minus_1 = Value(retval) - 1;
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
    const CPSize idx = GlyphIndexOf(click_xpos);
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
    const CPSize idx = GlyphIndexOf(xpos);
    //std::cout << "GlyphIndexOf mouse x-pos: " << xpos << std::endl;

    if (m_in_double_click_mode) {
        const auto word_indices = GetDoubleButtonDownDragWordIndices(idx);

        if (word_indices.first == word_indices.second) {
            if (idx < m_double_click_cursor_pos.first) {
                m_cursor_pos.second = idx;
                m_cursor_pos.first = m_double_click_cursor_pos.second;
            } else if (m_double_click_cursor_pos.second < idx) {
                m_cursor_pos.second = idx;
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
        // when a single-click drag occurs, move m_cursor_pos.second to where the mouse is, which selects a range of characters
        m_cursor_pos.second = idx;
        if (xpos < X0 || ClientSize().x < xpos) // if we're dragging past the currently visible text
            AdjustView();
    }

    //std::cout << "LDrag selected from: " << m_cursor_pos.first << "  to: " << m_cursor_pos.second << std::endl;
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
                --m_cursor_pos.second;
                const auto& ld{GetLineData()};
                if (!ld.empty()) {
                    const auto& ld0cd{ld[0].char_data};
                    const X extent = ld0cd[Value(m_cursor_pos.second)].extent;
                    while (CP0 < m_cursor_pos.second && extent == ld0cd[Value(m_cursor_pos.second) - 1].extent)
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
                if (!GetLineData().empty()) {
                    X extent = GetLineData()[0].char_data[Value(m_cursor_pos.second)].extent;
                    while (m_cursor_pos.second < Length() && extent == GetLineData()[0].char_data[Value(m_cursor_pos.second)].extent)
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

std::pair<CPSize, CPSize> Edit::GetDoubleButtonDownWordIndices(CPSize char_index)
{
    const auto ticks = GUI::GetGUI()->Ticks();
    if (ticks - m_last_button_down_time <= GUI::GetGUI()->DoubleClickInterval())
        m_in_double_click_mode = true;
    m_last_button_down_time = ticks;

    m_double_click_cursor_pos = std::pair<CPSize, CPSize>(char_index, char_index);
    if (m_in_double_click_mode)
        m_double_click_cursor_pos = GetDoubleButtonDownDragWordIndices(char_index);

    return m_double_click_cursor_pos;
}

std::pair<CPSize, CPSize> Edit::GetDoubleButtonDownDragWordIndices(CPSize char_index)
{
    const auto words = GUI::GetGUI()->FindWords(Text());
    const auto it = std::find_if(words.begin(), words.end(),
                                 [char_index](auto word) { return word.first < char_index && char_index < word.second; });
    return (it != words.end()) ? *it : std::pair<CPSize, CPSize>{char_index, char_index};
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
    const X text_space = ClientSize().x;
    const X first_char_offset = FirstCharOffset();
    static constexpr CPSize CP5{5};

    if (m_cursor_pos.second < m_first_char_shown) { // if the caret is at a place left of the current visible area
        if (m_first_char_shown - m_cursor_pos.second < CP5) // if the caret is less than five characters before m_first_char_shown
            m_first_char_shown = (CP5 < m_first_char_shown) ? m_first_char_shown - CP5 : CP0; // try to move the caret by five characters
        else // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_first_char_shown = m_cursor_pos.second;
        return;
    }

    const auto length = Length();
    if (length == CP0)
        return;

    const auto& ld{GetLineData()};
    if (ld.empty())
        return;

    const auto& char_data{ld[0].char_data};
    const X char_data_extent = -first_char_offset +
        ((m_cursor_pos.second != CP0) ? char_data[Value(m_cursor_pos.second) - 1].extent : X0);

    if (text_space <= char_data_extent) { // if the caret is moving to a place right of the current visible area
        // try to move the text by five characters, or to the end if caret is at a location before the end - 5th character
        const CPSize last_idx_to_use = std::min(m_cursor_pos.second + CP5, length - CP1);

        // number of pixels that the caret position overruns the right side of text area
        X pixels_to_move = (char_data[Value(last_idx_to_use)].extent - first_char_offset) - text_space;
        if (last_idx_to_use == length - CP1) // if the caret is at the very end of the string, add the length of some spaces
            pixels_to_move += static_cast<int>(Value(m_cursor_pos.second + CP5 - length - CP1)) * GetFont()->SpaceWidth();

        CPSize move_to = m_first_char_shown;
        while (move_to < CPSize{char_data.size()} &&
                char_data[Value(move_to)].extent - first_char_offset < pixels_to_move)
        { ++move_to; }

        m_first_char_shown = move_to;
    }
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

