//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/DrawUtil.h>
#include <GG/TextControl.h>
#include <GG/utf8/checked.h>

using namespace GG;

namespace {
    [[nodiscard]] constexpr Flags<TextFormat> ValidateFormat(Flags<TextFormat> format) noexcept
    {
        int dup_ct = 0;   // duplication count
        if (format & FORMAT_LEFT) ++dup_ct;
        if (format & FORMAT_RIGHT) ++dup_ct;
        if (format & FORMAT_CENTER) ++dup_ct;
        if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_CENTER by default
            format &= ~(FORMAT_RIGHT | FORMAT_LEFT);
            format |= FORMAT_CENTER;
        }
        dup_ct = 0;
        if (format & FORMAT_TOP) ++dup_ct;
        if (format & FORMAT_BOTTOM) ++dup_ct;
        if (format & FORMAT_VCENTER) ++dup_ct;
        if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_VCENTER by default
            format &= ~(FORMAT_TOP | FORMAT_BOTTOM);
            format |= FORMAT_VCENTER;
        }
        if ((format & FORMAT_WORDBREAK) && (format & FORMAT_LINEWRAP))   // only one of these can be picked; FORMAT_WORDBREAK overrides FORMAT_LINEWRAP
            format &= ~FORMAT_LINEWRAP;
        return format;
    }
}

////////////////////////////////////////////////
// GG::TextControl
////////////////////////////////////////////////
TextControl::TextControl(X x, Y y, X w, Y h, std::string str,
                         std::shared_ptr<Font> font, Clr color,
                         Flags<TextFormat> format,
                         Flags<WndFlag> flags) :
    Control(x, y, w, h, flags),
    m_format(ValidateFormat(format)),
    m_text_color(color),
    m_font(std::move(font))
{ TextControl::SetText(std::move(str)); }

TextControl::TextControl(X x, Y y, X w, Y h, std::string str,
                         std::vector<Font::TextElement> text_elements,
                         std::shared_ptr<Font> font,
                         Clr color, Flags<TextFormat> format,
                         Flags<WndFlag> flags) :
    Control(x, y, w, h, flags),
    m_format(ValidateFormat(format)),
    m_text_color(color),
    m_font(std::move(font))
{ TextControl::SetText(std::move(str), std::move(text_elements)); }

TextControl::TextControl(const TextControl& that) :
    Control(that.Left(), that.Top(), that.Width(), that.Height()),
    m_text(that.m_text),
    m_format(that.m_format),
    m_text_color(that.m_text_color),
    m_clip_text(that.m_clip_text),
    m_set_min_size(that.m_set_min_size),
    m_text_elements(that.m_text_elements),
    m_code_points(that.m_code_points),
    m_font(that.m_font),
    m_cached_minusable_size_width(that.m_cached_minusable_size_width),
    m_cached_minusable_size(that.m_cached_minusable_size)
{
    for (auto& elem : m_text_elements)
        elem.Bind(m_text);
}

TextControl& TextControl::operator=(const TextControl& that)
{
    m_text = that.m_text;
    m_format = that.m_format;
    m_text_color = that.m_text_color;
    m_clip_text = that.m_clip_text;
    m_set_min_size = that.m_set_min_size;
    m_text_elements = that.m_text_elements;
    m_code_points = that.m_code_points;
    m_font = that.m_font;
    m_render_cache.clear();
    m_cached_minusable_size_width = that.m_cached_minusable_size_width;
    m_cached_minusable_size = that.m_cached_minusable_size;

    for (auto& elem : m_text_elements)
        elem.Bind(m_text);

    return *this;
}

Pt TextControl::MinUsableSize() const noexcept
{ return m_text_lr - m_text_ul; }

Pt TextControl::MinUsableSize(X width) const
{
    // If the requested width is within one space width of the cached width
    // don't recalculate the size
    X min_delta = m_font ? m_font->SpaceWidth() : X1;
    X abs_delta_w = X(std::abs(Value(m_cached_minusable_size_width - width)));
    if (m_cached_minusable_size_width != X0 &&  abs_delta_w < min_delta)
        return m_cached_minusable_size;

    // Calculate and cache the minimum usable size when m_cached_minusable_size is equal to width.
    // Create dummy line data with line breaks added so that lines are not wider than width.
    Flags<TextFormat> dummy_format(m_format);
    auto dummy_line_data = m_font ?
        m_font->DetermineLines(m_text, dummy_format, width, m_text_elements) : Font::LineVec{};
    m_cached_minusable_size = (m_font ? m_font->TextExtent(dummy_line_data) : Pt{})
        + (ClientUpperLeft() - UpperLeft()) + (LowerRight() - ClientLowerRight());
    m_cached_minusable_size_width = width;
    return m_cached_minusable_size;
}

std::string_view TextControl::Text(CPSize from, CPSize to) const
{
    if (from == INVALID_CP_SIZE || to == INVALID_CP_SIZE)
        return "";

    std::tie(from, to) = [from, to]() { return std::pair{std::min(from, to), std::max(from, to)}; }();

    const auto txt_sz = m_text.size();
    auto [low_string_idx_strsz, high_string_idx_strsz] = CodePointIndicesRangeToStringSizeIndices(from, to, m_line_data);
    const auto low_string_idx = std::min(Value(low_string_idx_strsz), txt_sz);
    const auto high_string_idx = std::min(Value(high_string_idx_strsz), txt_sz);
    const auto out_length = std::max(low_string_idx, high_string_idx) - std::min(low_string_idx, high_string_idx);

    const auto low_it = m_text.begin() + low_string_idx;

    try {
        return {&*low_it, out_length};
    } catch (...) {
        return {};
    }
}

Pt TextControl::TextUpperLeft() const noexcept
{ return UpperLeft() + m_text_ul; }

Pt TextControl::TextLowerRight() const noexcept
{ return UpperLeft() + m_text_lr; }

void TextControl::Render()
{
    if (!m_font)
        return;

    RefreshCache();
    if (m_clip_text)
        BeginClipping();

    glPushMatrix();
    Pt ul = ClientUpperLeft();
    glTranslated(Value(ul.x), Value(ul.y), 0);
    m_font->RenderCachedText(m_render_cache);
    glPopMatrix();

    if (m_clip_text)
        EndClipping();
}

void TextControl::RefreshCache() {
    m_render_cache.clear();
    Font::RenderState rs(TextColor());
    if (m_font)
        m_font->PreRenderText(Pt0, Size(), m_text, m_format, m_render_cache, m_line_data, rs);
}

void TextControl::SetText(std::string str)
{
    if (!utf8::is_valid(str.begin(), str.end()))
        return;
    m_text = std::move(str);

    if (!m_font)
        return;

    m_text_elements = m_font->ExpensiveParseFromTextToTextElements(m_text, m_format);
    RecomputeLineData();
}

void TextControl::SetText(std::string str, std::vector<Font::TextElement> text_elements)
{
    if (!utf8::is_valid(str.begin(), str.end()))
        return;

    // before rebinding text elements to str, they may be invalid if whatever they were
    // pointing to before the call to this function may have been moved from to create
    // str. this is OK though, as the elem.text.size() calls don't depend on the
    // pointed-to string of the Substring elem.text

    std::size_t expected_length(0);
    for (auto& elem : text_elements)
        expected_length += elem.text.size();

    if (expected_length > str.size())
        return;

    m_text = std::move(str);

    m_text_elements = std::move(text_elements);
    for (auto& elem : m_text_elements)
        elem.Bind(m_text);

    RecomputeLineData();
}

void TextControl::ChangeTemplatedText(const std::string& new_text, std::size_t targ_offset) {
    if (m_font)
        m_font->ChangeTemplatedText(m_text, m_text_elements, new_text, targ_offset);
    RecomputeLineData();
}

void TextControl::RecomputeLineData() {
    if (!m_font)
        return;

    m_code_points = CPSize(utf8::distance(m_text.begin(), m_text.end())); // ??? should be based on line data probably?

    m_line_data = m_font->DetermineLines(m_text, m_format, ClientSize().x, m_text_elements);
    Pt text_sz = m_font->TextExtent(m_line_data);
    m_text_ul = Pt0;
    m_text_lr = text_sz;
    m_render_cache.clear();
    if (m_format & FORMAT_NOWRAP)
        Resize(text_sz);
    else
        RecomputeTextBounds();

    m_cached_minusable_size_width = X0;
}

void TextControl::SetFont(std::shared_ptr<Font> font)
{
    m_font = std::move(font);
    SetText(std::move(m_text));
}

void TextControl::SizeMove(Pt ul, Pt lr)
{
    const auto old_size = Size();
    Wnd::SizeMove(ul, lr);
    const bool resized = old_size != Size();
    bool redo_determine_lines = false;
    X client_width = ClientSize().x;

    if (m_text.empty()) {
        // don't redo lines
    } else if (resized && m_format != FORMAT_LEFT &&
               m_format != FORMAT_NONE)
    {
        // for text with non-trivial alignment, be that centred, justified, right,
        // or multi-line, or vertical alignments, need to redo for any resize
        redo_determine_lines = true;
    } else if (resized &&
               !(m_format & FORMAT_NOWRAP) &&
               (m_format & FORMAT_WORDBREAK || m_format & FORMAT_LINEWRAP))
    {
        // if breaking text across lines, need to redo layout when the available
        // width is less than that needed to fit the text on one line
        X text_width = m_text_lr.x - m_text_ul.x;
        redo_determine_lines = client_width < text_width ||
                              (text_width < client_width && 1u < m_line_data.size());
    }

    if (redo_determine_lines && m_font) {
        if (m_text_elements.empty())
            m_text_elements = m_font->ExpensiveParseFromTextToTextElements(m_text, m_format);
        m_line_data = m_font->DetermineLines(m_text, m_format, client_width, m_text_elements);
        Pt text_sz = m_font->TextExtent(m_line_data);
        m_text_ul = Pt();
        m_text_lr = text_sz;
        m_render_cache.clear();
    }
    RecomputeTextBounds();
}

void TextControl::SetTextFormat(Flags<TextFormat> format)
{
    const auto initial_format = m_format;
    m_format = ValidateFormat(format);
    if (m_format != initial_format)
        SetText(std::move(m_text));
}

void TextControl::SetTextColor(Clr color)
{
    m_text_color = color;
    m_render_cache.clear();
}

void TextControl::SetColor(Clr c) noexcept
{
    Control::SetColor(c);
    m_text_color = c;
    m_render_cache.clear();
}

void TextControl::ClipText(bool b)
{ m_clip_text = b; }

void TextControl::SetResetMinSize(bool b)
{
    m_set_min_size = b;
    AdjustMinimumSize();
}

void TextControl::operator+=(const std::string& s)
{ SetText(m_text + s); }

namespace {
    constexpr bool IsValidUTFChar(char c) noexcept {
        if constexpr (std::is_signed_v<char>)
            return c >= 0;
        else
            return c <= 0x7f;
    }
}

void TextControl::operator+=(char c)
{
    if (!IsValidUTFChar(c))
        throw utf8::invalid_utf8(c);
    SetText(m_text + c);
}

void TextControl::Clear()
{ SetText(""); }

void TextControl::Insert(CPSize pos, char c)
{
    std::size_t line;
    std::tie(line, pos) = LinePositionOfGlyph(pos, m_line_data);
    Insert(line, pos, c);
}

void TextControl::Insert(CPSize pos, const std::string& s)
{
    std::size_t line;
    std::tie(line, pos) = LinePositionOfGlyph(pos, m_line_data);
    Insert(line, pos, s);
}

void TextControl::Erase(CPSize pos, CPSize num)
{
    std::size_t line;
    std::tie(line, pos) = LinePositionOfGlyph(pos, m_line_data);
    Erase(line, pos, num);
}

void TextControl::Insert(std::size_t line, CPSize pos, char c)
{
    if (!IsValidUTFChar(c))
        return;
    m_text.insert(Value(StringIndexOfLineAndGlyph(line, pos, m_line_data)), 1, c);
    SetText(std::move(m_text));
}

void TextControl::Insert(std::size_t line, CPSize pos, const std::string& s)
{
    if (!utf8::is_valid(s.begin(), s.end()))
        return;
    m_text.insert(Value(StringIndexOfLineAndGlyph(line, pos, m_line_data)), s);
    SetText(std::move(m_text));
}

void TextControl::Erase(std::size_t line, CPSize pos, CPSize num)
{
    auto it = m_text.begin() + Value(StringIndexOfLineAndGlyph(line, pos, m_line_data));
    auto end_it = m_text.begin() + Value(StringIndexOfLineAndGlyph(line, pos + num, m_line_data));
    if (it == end_it)
        return;
    m_text.erase(it, end_it);
    SetText(std::move(m_text));
}

void TextControl::Erase(std::size_t line1, CPSize pos1, std::size_t line2, CPSize pos2)
{
    //std::cout << "TextControl::Erase(" << line1 << ", " << pos1 << "," << line2 << ", " << pos2 << ")" << std::endl;

    std::size_t offset1 = Value(StringIndexOfLineAndGlyph(line1, pos1, m_line_data));
    std::size_t offset2 = Value(StringIndexOfLineAndGlyph(line2, pos2, m_line_data));
    if (offset1 == offset2)
        return;
    //std::cout << "TextControl::Erase offsets: " << offset1 << " // " << offset2 << std::endl;
    auto it = m_text.begin() + std::min(offset1, offset2);
    auto end_it = m_text.begin() + std::max(offset1, offset2);
    m_text.erase(it, end_it);
    SetText(std::move(m_text));
}

void TextControl::AdjustMinimumSize()
{
    if (!m_set_min_size)
        return;

    // disable while doing to prevent recursive loop via
    // SetMinSize(..) -> AdjustMinimumSize() -> RecomputeTextBounds() -> Resize(..) -> SetMinSize(..)
    // which occurred when Font::TextExtent returned a large value
    m_set_min_size = false;
    SetMinSize(m_text_lr - m_text_ul);
    m_set_min_size = true;
}

void TextControl::RecomputeTextBounds()
{
    const Pt text_sz = TextLowerRight() - TextUpperLeft();
    m_text_ul.y = Y0; // default value for FORMAT_TOP
    if (m_format & FORMAT_BOTTOM)
        m_text_ul.y = Size().y - text_sz.y;
    else if (m_format & FORMAT_VCENTER)
        m_text_ul.y = ToY((Size().y - text_sz.y) / 2.0);
    m_text_ul.x = X0; // default for FORMAT_LEFT
    if (m_format & FORMAT_RIGHT)
        m_text_ul.x = Size().x - text_sz.x;
    else if (m_format & FORMAT_CENTER)
        m_text_ul.x = ToX((Size().x - text_sz.x) / 2.0);
    m_text_lr = m_text_ul + text_sz;
    AdjustMinimumSize();
}
