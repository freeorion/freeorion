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

#include <GG/TextControl.h>

#include <GG/DrawUtil.h>
#include <GG/utf8/checked.h>

#include <boost/assign/list_of.hpp>


using namespace GG;

namespace {
    const Pt INVALID_USABLE_SIZE(-X1, -Y1);

    struct TextSignalEcho
    {
        TextSignalEcho(const TextControl& text_control, const std::string& name) :
            m_text_control(text_control),
            m_name(name)
            {}
        void operator()()
            { std::cerr << "GG_SIGNAL : " << m_name << " ()\n"; }
        void operator()(const std::string& new_text)
            { std::cerr << "GG SIGNAL : " << m_name << "() : new_text : " << new_text << "\n"; }

        const TextControl& m_text_control;
        std::string m_name;
    };
}

////////////////////////////////////////////////
// GG::TextControl
////////////////////////////////////////////////
TextControl::TextControl() :
    Control(),
    m_format(FORMAT_NONE),
    m_clip_text(false),
    m_set_min_size(false),
    m_code_points(0),
    m_fit_to_text(false)
{}

TextControl::TextControl(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                         Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) :
    Control(x, y, w, h, flags),
    m_format(format),
    m_text_color(color),
    m_clip_text(false),
    m_set_min_size(false),
    m_code_points(0),
    m_font(font),
    m_fit_to_text(false)
{
    ValidateFormat();
    SetText(str);
    if (INSTRUMENT_ALL_SIGNALS) {
        InstrumentSignals();
    }
}

TextControl::TextControl(X x, Y y, const std::string& str, const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                         Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) :
    Control(x, y, X0, Y0, flags),
    m_format(format),
    m_text_color(color),
    m_clip_text(false),
    m_set_min_size(false),
    m_code_points(0),
    m_font(font),
    m_fit_to_text(true)
{
    ValidateFormat();
    SetText(str);
    if (INSTRUMENT_ALL_SIGNALS) {
        InstrumentSignals();
    }
}

void TextControl::InstrumentSignals() {
    Connect(AfterTextChangedSignal, TextSignalEcho(*this, "TextBox::AfterTextChangedSignal"));
}

Pt TextControl::MinUsableSize() const
{ return m_text_lr - m_text_ul; }

const std::string& TextControl::Text() const
{ return m_text; }

std::string TextControl::Text(CPSize from, CPSize to) const
{
    if (from == INVALID_CP_SIZE || to == INVALID_CP_SIZE)
        return "";
    CPSize low = std::max(CPSize(0), std::min(from, to));
    CPSize high = std::min(Length(), std::max(from, to));

    std::string::const_iterator it = m_text.begin() + Value(low);
    std::string::const_iterator end_it = m_text.begin() + Value(high);
    return std::string(it, end_it);
}

Flags<TextFormat> TextControl::GetTextFormat() const
{ return m_format; }

Clr TextControl::TextColor() const
{ return m_text_color; }

bool TextControl::ClipText() const
{ return m_clip_text; }

bool TextControl::SetMinSize() const
{ return m_set_min_size; }

TextControl::operator const std::string&() const
{ return m_text; }

bool TextControl::Empty() const
{ return m_text.empty(); }

CPSize TextControl::Length() const
{ return m_code_points; }

Pt TextControl::TextUpperLeft() const
{ return UpperLeft() + m_text_ul; }

Pt TextControl::TextLowerRight() const
{ return UpperLeft() + m_text_lr; }

void TextControl::Render()
{
    Clr clr_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();
    glColor(clr_to_use);
    if (m_font) {
        if (m_clip_text)
            BeginClipping();
        m_font->RenderText(UpperLeft(), LowerRight(), m_text, m_format, &m_line_data);
        if (m_clip_text)
            EndClipping();
    }
}

void TextControl::SetText(const std::string& str)
{
    m_text = str;
    if (m_font) {
        m_code_points = CPSize(utf8::distance(str.begin(), str.end()));
        m_text_elements.clear();
        Pt text_sz =
            m_font->DetermineLines(m_text, m_format, ClientSize().x, m_line_data, m_text_elements);
        m_text_ul = Pt();
        m_text_lr = text_sz;
        AdjustMinimumSize();
        if (m_fit_to_text) {
            Resize(text_sz);
        } else {
            RecomputeTextBounds();
        }
    }
    AfterTextChangedSignal(m_text);
}

void TextControl::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    bool redo_determine_lines = false;
    X client_width = ClientSize().x;
    if (!m_fit_to_text && (m_format | FORMAT_WORDBREAK || m_format | FORMAT_LINEWRAP)) {
        X text_width = m_text_lr.x - m_text_ul.x;
        redo_determine_lines =
            client_width < text_width ||
            text_width < client_width && 1u < m_line_data.size();
    }
    if (redo_determine_lines) {
        Pt text_sz;
        if (m_text_elements.empty()) {
            text_sz =
                m_font->DetermineLines(m_text, m_format, client_width, m_line_data, m_text_elements);
        } else {
            text_sz =
                m_font->DetermineLines(m_text, m_format, client_width, m_text_elements, m_line_data);
        }
        m_text_ul = Pt();
        m_text_lr = text_sz;
        AdjustMinimumSize();
    }
    RecomputeTextBounds();
}

void TextControl::SetTextFormat(Flags<TextFormat> format)
{
    m_format = format;
    ValidateFormat();
    if (m_format != format)
        SetText(m_text);
}

void TextControl::SetTextColor(Clr color)
{ m_text_color = color; }

void TextControl::SetColor(Clr c)
{
    Control::SetColor(c);
    m_text_color = c;
}

void TextControl::ClipText(bool b)
{ m_clip_text = b; }

void TextControl::SetMinSize(bool b)
{
    m_set_min_size = b;
    AdjustMinimumSize();
}

void TextControl::operator+=(const std::string& s)
{ SetText(m_text + s); }

void TextControl::operator+=(char c)
{
    if (!detail::ValidUTFChar<char>()(c))
        throw utf8::invalid_utf8(c);
    SetText(m_text + c);
}

void TextControl::Clear()
{ SetText(""); }

void TextControl::Insert(CPSize pos, char c)
{
    std::size_t line;
    boost::tie(line, pos) = LinePositionOf(pos, m_line_data);
    Insert(line, pos, c);
}

void TextControl::Insert(CPSize pos, const std::string& s)
{
    std::size_t line;
    boost::tie(line, pos) = LinePositionOf(pos, m_line_data);
    Insert(line, pos, s);
}

void TextControl::Erase(CPSize pos, CPSize num/* = CP1*/)
{
    std::size_t line;
    boost::tie(line, pos) = LinePositionOf(pos, m_line_data);
    Erase(line, pos, num);
}

void TextControl::Insert(std::size_t line, CPSize pos, char c)
{
    if (!detail::ValidUTFChar<char>()(c))
        throw utf8::invalid_utf8(c);
    m_text.insert(Value(StringIndexOf(line, pos, m_line_data)), 1, c);
    SetText(m_text);
}

void TextControl::Insert(std::size_t line, CPSize pos, const std::string& s)
{
    m_text.insert(Value(StringIndexOf(line, pos, m_line_data)), s);
    SetText(m_text);
}

void TextControl::Erase(std::size_t line, CPSize pos, CPSize num/* = CP1*/)
{
    std::string::iterator it = m_text.begin() + Value(StringIndexOf(line, pos, m_line_data));
    std::string::iterator end_it = m_text.begin() + Value(StringIndexOf(line, pos + num, m_line_data));
    m_text.erase(it, end_it);
    SetText(m_text);
}

const std::vector<Font::LineData>& TextControl::GetLineData() const
{ return m_line_data; }

const boost::shared_ptr<Font>& TextControl::GetFont() const
{ return m_font; }

bool TextControl::FitToText() const
{ return m_fit_to_text; }

void TextControl::ValidateFormat()
{
    int dup_ct = 0;   // duplication count
    if (m_format & FORMAT_LEFT) ++dup_ct;
    if (m_format & FORMAT_RIGHT) ++dup_ct;
    if (m_format & FORMAT_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_CENTER by default
        m_format &= ~(FORMAT_RIGHT | FORMAT_LEFT);
        m_format |= FORMAT_CENTER;
    }
    dup_ct = 0;
    if (m_format & FORMAT_TOP) ++dup_ct;
    if (m_format & FORMAT_BOTTOM) ++dup_ct;
    if (m_format & FORMAT_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_VCENTER by default
        m_format &= ~(FORMAT_TOP | FORMAT_BOTTOM);
        m_format |= FORMAT_VCENTER;
    }
    if ((m_format & FORMAT_WORDBREAK) && (m_format & FORMAT_LINEWRAP))   // only one of these can be picked; FORMAT_WORDBREAK overrides FORMAT_LINEWRAP
        m_format &= ~FORMAT_LINEWRAP;
}

void TextControl::AdjustMinimumSize()
{
    if (m_set_min_size)
        SetMinSize(m_text_lr - m_text_ul);
}

void TextControl::RecomputeTextBounds()
{
    Pt text_sz = TextLowerRight() - TextUpperLeft();
    m_text_ul.y = Y0; // default value for FORMAT_TOP
    if (m_format & FORMAT_BOTTOM)
        m_text_ul.y = Size().y - text_sz.y;
    else if (m_format & FORMAT_VCENTER)
        m_text_ul.y = (Size().y - text_sz.y) / 2.0;
    m_text_ul.x = X0; // default for FORMAT_LEFT
    if (m_format & FORMAT_RIGHT)
        m_text_ul.x = Size().x - text_sz.x;
    else if (m_format & FORMAT_CENTER)
        m_text_ul.x = (Size().x - text_sz.x) / 2.0;
    m_text_lr = m_text_ul + text_sz;
}
