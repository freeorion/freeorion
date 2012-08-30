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

#include <GG/Font.h>

#include <GG/GUI.h>
#include <GG/Base.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/utf8/checked.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <boost/assign/list_of.hpp>

#include <cmath>
#include <cctype>
#include <numeric>
#include <sstream>

#define DEBUG_DETERMINELINES 0

namespace GG { namespace detail {

    FTFaceWrapper::FTFaceWrapper() :
        m_face(0)
    {}

    FTFaceWrapper::~FTFaceWrapper()
    { if (m_face) FT_Done_Face(m_face); }

} }

using namespace GG;

namespace {
    const boost::uint32_t WIDE_SPACE = ' ';
    const boost::uint32_t WIDE_NEWLINE = '\n';
    const boost::uint32_t WIDE_CR = '\r';
    const boost::uint32_t WIDE_FF = '\f';
    const boost::uint32_t WIDE_TAB = '\t';

    const std::string ALIGN_LEFT_TAG = "left";
    const std::string ALIGN_CENTER_TAG = "center";
    const std::string ALIGN_RIGHT_TAG = "right";
    const std::string PRE_TAG = "pre";

    template <class T>
    T NextPowerOfTwo(T input)
    {
        T value(1);
        while (value < input)
            value *= 2;
        return value;
    }

    inline boost::uint32_t CharToUint32_t(char c)
    { return c < 0 ? 256 + c : c; }

    /** This is used to collect data on the glyphs as they are recorded into buffers, for use in creating Glyph objects at the end
        of Font's constructor.*/
    struct TempGlyphData
    {
        TempGlyphData() {}
        TempGlyphData(std::size_t i, const Pt& ul_, const Pt& lr_, Y y_ofs, X lb, X a) :
            idx(i), ul(ul_), lr(lr_), y_offset(y_ofs), left_b(lb), adv(a) {}
        std::size_t idx;      ///< index into m_textures of texture that contains this glyph
        Pt          ul, lr;   ///< area of glyph subtexture within texture
        Y           y_offset; ///< vertical offset to draw texture (may be negative!)
        X           left_b;   ///< left bearing (see Glyph)
        X           adv;      ///< advance of glyph (see Glyph)
    };

    struct FTLibraryWrapper
    {
        FTLibraryWrapper() : m_library(0)
        {
            if (!m_library && FT_Init_FreeType(&m_library)) // if no library exists and we can't create one...
                throw FailedFTLibraryInit("Unable to initialize FreeType font library object");
        }
        ~FTLibraryWrapper() { FT_Done_FreeType(m_library); }
        FT_Library m_library;
    } g_library;

    struct MatchesKnownTag
    {
        MatchesKnownTag(const std::set<std::string>& known_tags,
                        bool& ignore_tags) :
            m_known_tags(known_tags),
            m_ignore_tags(ignore_tags)
            {}
        bool operator()(const boost::xpressive::ssub_match& sub) const
            { return m_ignore_tags ? false : m_known_tags.find(sub.str()) != m_known_tags.end(); }
        const std::set<std::string>& m_known_tags;
        bool& m_ignore_tags;
    };

    struct PushSubmatchOntoStack
    {
        typedef void result_type;
        void operator()(const std::string& str,
                        std::stack<Font::Substring>& tag_stack,
                        bool& ignore_tags,
                        const boost::xpressive::ssub_match& sub) const
            {
                tag_stack.push(Font::Substring(str, sub));
                if (tag_stack.top() == PRE_TAG)
                    ignore_tags = true;
            }
    };
    const boost::xpressive::function<PushSubmatchOntoStack>::type Push = {{}};

    bool operator==(const Font::Substring& lhs, const boost::xpressive::ssub_match& rhs)
    {
        return lhs.size() == static_cast<std::size_t>(rhs.length()) &&
            !std::memcmp(&*lhs.begin(), &*rhs.first, lhs.size());
    }

    struct MatchesTopOfStack
    {
        MatchesTopOfStack(std::stack<Font::Substring>& tag_stack,
                          bool& ignore_tags) :
            m_tag_stack(tag_stack),
            m_ignore_tags(ignore_tags)
            {}
        bool operator()(const boost::xpressive::ssub_match& sub) const
            {
                bool retval = m_tag_stack.empty() ? false : m_tag_stack.top() == sub;
                if (retval) {
                    m_tag_stack.pop();
                    if (m_tag_stack.empty() || m_tag_stack.top() != PRE_TAG)
                        m_ignore_tags = false;
                }
                return retval;
            }
        std::stack<Font::Substring>& m_tag_stack;
        bool& m_ignore_tags;
    };

    void SetJustification(bool& last_line_of_curr_just, Font::LineData& line_data, Alignment orig_just, Alignment prev_just)
    {
        if (last_line_of_curr_just) {
            line_data.justification = orig_just;
            last_line_of_curr_just = false;
        } else {
            line_data.justification = prev_just;
        }
    }

    const double ITALICS_SLANT_ANGLE = 12; // degrees
    const double ITALICS_FACTOR = 1.0 / tan((90 - ITALICS_SLANT_ANGLE) * 3.1415926 / 180.0); // factor used to shear glyphs ITALICS_SLANT_ANGLE degrees CW from straight up

    const std::vector<std::pair<boost::uint32_t, boost::uint32_t> > PRINTABLE_ASCII_ALPHA_RANGES = boost::assign::list_of
        (std::pair<boost::uint32_t, boost::uint32_t>(0x41, 0x5B))
        (std::pair<boost::uint32_t, boost::uint32_t>(0x61, 0x7B));

    const std::vector<std::pair<boost::uint32_t, boost::uint32_t> > PRINTABLE_ASCII_NONALPHA_RANGES = boost::assign::list_of
        (std::pair<boost::uint32_t, boost::uint32_t>(0x09, 0x0D))
        (std::pair<boost::uint32_t, boost::uint32_t>(0x20, 0x21))
        (std::pair<boost::uint32_t, boost::uint32_t>(0x30, 0x3A))
        (std::pair<boost::uint32_t, boost::uint32_t>(0x21, 0x30))
        (std::pair<boost::uint32_t, boost::uint32_t>(0x3A, 0x41))
        (std::pair<boost::uint32_t, boost::uint32_t>(0x5B, 0x61))
        (std::pair<boost::uint32_t, boost::uint32_t>(0x7B, 0x7F));
}

///////////////////////////////////////
// Constants
///////////////////////////////////////
const StrSize GG::S0(0);
const StrSize GG::S1(1);
const StrSize GG::INVALID_STR_SIZE(std::numeric_limits<std::size_t>::max());
const CPSize GG::CP0(0);
const CPSize GG::CP1(1);
const CPSize GG::INVALID_CP_SIZE(std::numeric_limits<std::size_t>::max());


///////////////////////////////////////
// function GG::RgbaTag
///////////////////////////////////////
std::string GG::RgbaTag(const Clr& c)
{
    std::stringstream stream;
    stream << "<rgba "
           << static_cast<int>(c.r) << " "
           << static_cast<int>(c.g) << " "
           << static_cast<int>(c.b) << " "
           << static_cast<int>(c.a)
           << ">";
    return stream.str();
}


///////////////////////////////////////
// TextFormat
///////////////////////////////////////
const TextFormat GG::FORMAT_NONE         (0);
const TextFormat GG::FORMAT_VCENTER      (1 << 0);
const TextFormat GG::FORMAT_TOP          (1 << 1);
const TextFormat GG::FORMAT_BOTTOM       (1 << 2);
const TextFormat GG::FORMAT_CENTER       (1 << 3);
const TextFormat GG::FORMAT_LEFT         (1 << 4);
const TextFormat GG::FORMAT_RIGHT        (1 << 5);
const TextFormat GG::FORMAT_WORDBREAK    (1 << 6);
const TextFormat GG::FORMAT_LINEWRAP     (1 << 7);
const TextFormat GG::FORMAT_IGNORETAGS   (1 << 8);

GG_FLAGSPEC_IMPL(TextFormat);

namespace {
    bool RegisterTextFormats()
    {
        FlagSpec<TextFormat>& spec = FlagSpec<TextFormat>::instance();
        spec.insert(FORMAT_NONE, "FORMAT_NONE", true);
        spec.insert(FORMAT_VCENTER, "FORMAT_VCENTER", true);
        spec.insert(FORMAT_TOP, "FORMAT_TOP", true);
        spec.insert(FORMAT_BOTTOM, "FORMAT_BOTTOM", true);
        spec.insert(FORMAT_CENTER, "FORMAT_CENTER", true);
        spec.insert(FORMAT_LEFT, "FORMAT_LEFT", true);
        spec.insert(FORMAT_RIGHT, "FORMAT_RIGHT", true);
        spec.insert(FORMAT_WORDBREAK, "FORMAT_WORDBREAK", true);
        spec.insert(FORMAT_LINEWRAP, "FORMAT_LINEWRAP", true);
        spec.insert(FORMAT_IGNORETAGS, "FORMAT_IGNORETAGS", true);
        return true;
    }
    bool dummy = RegisterTextFormats();
}


///////////////////////////////////////
// class GG::Font::Substring
///////////////////////////////////////
const std::string Font::Substring::EMPTY_STRING;

Font::Substring::Substring() :
    str(&EMPTY_STRING),
    first(0),
    second(0)
{}

Font::Substring::Substring(const std::string& str_,
                           std::string::const_iterator first_,
                           std::string::const_iterator second_) :
    str(&str_),
    first(0),
    second(0)
{
    assert(str->begin() <= first_);
    assert(first_ <= second_);
    assert(second_ <= str->end());
    first = std::distance(str->begin(), first_);
    second = std::distance(str->begin(), second_);
}

Font::Substring::Substring(const std::string& str_, const IterPair& pair) :
    str(&str_),
    first(0),
    second(0)
{
    assert(str->begin() <= pair.first);
    assert(pair.first <= pair.second);
    assert(pair.second <= str->end());
    first = std::distance(str->begin(), pair.first);
    second = std::distance(str->begin(), pair.second);
}

std::string::const_iterator Font::Substring::begin() const
{ return boost::next(str->begin(), first); }

std::string::const_iterator Font::Substring::end() const
{ return boost::next(str->begin(), second); }

bool Font::Substring::empty() const
{ return first == second; }

std::size_t Font::Substring::size() const
{ return second - first; }

Font::Substring::operator std::string() const
{ return std::string(begin(), end()); }

bool Font::Substring::operator==(const std::string& rhs) const
{ return size() == rhs.size() && !std::memcmp(str->data() + first, rhs.data(), rhs.size()); }

bool Font::Substring::operator!=(const std::string& rhs) const
{ return !operator==(rhs); }

Font::Substring& Font::Substring::operator+=(const IterPair& rhs)
{
    assert(rhs.first <= rhs.second);
    assert(std::distance(str->begin(), rhs.first) == second);
    second = std::distance(str->begin(), rhs.second);
    return *this;
}


///////////////////////////////////////
// Free Functions
///////////////////////////////////////
std::ostream& GG::operator<<(std::ostream& os, const Font::Substring& substr)
{
    std::ostream_iterator<char> out_it(os);
    std::copy(substr.begin(), substr.end(), out_it);
    return os;
}

CPSize GG::CodePointIndexOf(std::size_t line, CPSize index, const std::vector<Font::LineData>& line_data)
{
    CPSize retval(0);
    if (line_data.size() <= line) {
        std::vector<Font::LineData>::const_reverse_iterator it = line_data.rbegin();
        std::vector<Font::LineData>::const_reverse_iterator end_it = line_data.rend();
        while (it != end_it) {
            if (!it->char_data.empty()) {
                retval = it->char_data.back().code_point_index + 1;
                break;
            }
            ++it;
        }
    } else if (index < line_data[line].char_data.size()) {
        retval = line_data[line].char_data[Value(index)].code_point_index;
    } else {
        std::vector<Font::LineData>::const_reverse_iterator it =
            line_data.rbegin() + (line_data.size() - 1 - line);
        std::vector<Font::LineData>::const_reverse_iterator end_it = line_data.rend();
        while (it != end_it) {
            if (!it->char_data.empty()) {
                retval = it->char_data.back().code_point_index + 1;
                break;
            }
            ++it;
        }
    }
    return retval;
}

StrSize GG::StringIndexOf(std::size_t line, CPSize index, const std::vector<Font::LineData>& line_data)
{
    StrSize retval(0);
    if (line_data.size() <= line) {
        std::vector<Font::LineData>::const_reverse_iterator it = line_data.rbegin();
        std::vector<Font::LineData>::const_reverse_iterator end_it = line_data.rend();
        while (it != end_it) {
            if (!it->char_data.empty()) {
                retval = it->char_data.back().string_index + it->char_data.back().string_size;
                break;
            }
            ++it;
        }
    } else if (index < line_data[line].char_data.size()) {
        retval = line_data[line].char_data[Value(index)].string_index;
    } else {
        std::vector<Font::LineData>::const_reverse_iterator it =
            line_data.rbegin() + (line_data.size() - 1 - line);
        std::vector<Font::LineData>::const_reverse_iterator end_it = line_data.rend();
        while (it != end_it) {
            if (!it->char_data.empty()) {
                retval = it->char_data.back().string_index + it->char_data.back().string_size;
                break;
            }
            ++it;
        }
    }
    return retval;
}

std::pair<std::size_t, CPSize> GG::LinePositionOf(CPSize index, const std::vector<Font::LineData>& line_data)
{
    std::pair<std::size_t, CPSize> retval(std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE);
    for (std::size_t i = 0; i < line_data.size(); ++i) {
        if (line_data[i].char_data.front().code_point_index <= index &&
            index <= line_data[i].char_data.back().code_point_index) {
            retval.first = i;
            retval.second = index - line_data[i].char_data.front().code_point_index;
            break;
        }
    }
    return retval;
}


///////////////////////////////////////
// class GG::Font::TextElement
///////////////////////////////////////
Font::TextElement::TextElement() :
    whitespace(false),
    newline(false),
    cached_width(-X1)
{}

Font::TextElement::TextElement(bool ws, bool nl) :
    whitespace(ws),
    newline(nl),
    cached_width(-X1)
{}

Font::TextElement::~TextElement()
{}

Font::TextElement::TextElementType Font::TextElement::Type() const
{ return newline ? NEWLINE : (whitespace ? WHITESPACE : TEXT); }

X Font::TextElement::Width() const
{
    if (cached_width == -X1)
        cached_width = std::accumulate(widths.begin(), widths.end(), X0);
    return cached_width;
}

StrSize Font::TextElement::StringSize() const
{ return StrSize(text.size()); }

CPSize Font::TextElement::CodePointSize() const
{ return CPSize(widths.size()); }


///////////////////////////////////////
// class GG::Font::FormattingTag
///////////////////////////////////////
Font::FormattingTag::FormattingTag() :
    TextElement(),
    close_tag(false)
{}

Font::FormattingTag::FormattingTag(bool close) :
    TextElement(false, false),
    close_tag(close)
{}

Font::FormattingTag::TextElementType Font::FormattingTag::Type() const
{ return close_tag ? CLOSE_TAG : OPEN_TAG; }


///////////////////////////////////////
// class GG::Font::LineData
///////////////////////////////////////
Font::LineData::LineData() :
    justification(ALIGN_CENTER)
{}

X Font::LineData::Width() const
{ return char_data.empty() ? X0 : char_data.back().extent; }

bool Font::LineData::Empty() const
{ return char_data.empty(); }

///////////////////////////////////////
// class GG::Font::RenderState
///////////////////////////////////////
Font::RenderState::RenderState() :
    use_italics(0),
    draw_underline(0)
{}


///////////////////////////////////////
// class GG::Font::LineData::CharData
///////////////////////////////////////
Font::LineData::CharData::CharData() :
    extent(0),
    string_index(0)
{}

Font::LineData::CharData::CharData(X extent_, StrSize str_index, StrSize str_size, CPSize cp_index,
                                   const std::vector<boost::shared_ptr<TextElement> >& tags_) :
    extent(extent_),
    string_index(str_index),
    string_size(str_size),
    code_point_index(cp_index),
    tags()
{
    for (std::size_t i = 0; i < tags_.size(); ++i) {
        tags.push_back(boost::dynamic_pointer_cast<FormattingTag>(tags_[i]));
    }
}


///////////////////////////////////////
// struct GG::Font::Glyph
///////////////////////////////////////
Font::Glyph::Glyph() :
    y_offset(0),
    left_bearing(0),
    advance(0),
    width(0)
{}

Font::Glyph::Glyph(const boost::shared_ptr<Texture>& texture, const Pt& ul, const Pt& lr, Y y_ofs, X lb, X adv) :
    sub_texture(texture, ul.x, ul.y, lr.x, lr.y),
    y_offset(y_ofs),
    left_bearing(lb),
    advance(adv),
    width(ul.x - lr.x)
{}


///////////////////////////////////////
// class GG::Font
///////////////////////////////////////
std::set<std::string> Font::s_action_tags;
std::set<std::string> Font::s_known_tags;

Font::Font() :
    m_pt_sz(0),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_space_width(0)
{}

Font::Font(const std::string& font_filename, unsigned int pts) :
    m_font_filename(font_filename),
    m_pt_sz(pts),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_space_width(0)
{
    if (m_font_filename != "") {
        detail::FTFaceWrapper wrapper;
        FT_Error error = GetFace(wrapper.m_face);
        CheckFace(wrapper.m_face, error);
        Init(wrapper.m_face);
    }
}

Font::Font(const std::string& font_filename, unsigned int pts,
           const std::vector<unsigned char>& file_contents) :
    m_font_filename(font_filename),
    m_pt_sz(pts),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_space_width(0)
{
    assert(!file_contents.empty());
    detail::FTFaceWrapper wrapper;
    FT_Error error = GetFace(file_contents, wrapper.m_face);
    CheckFace(wrapper.m_face, error);
    Init(wrapper.m_face);
}

Font::~Font()
{}

const std::string& Font::FontName() const
{ return m_font_filename; }

unsigned int Font::PointSize() const
{ return m_pt_sz; }

const std::vector<UnicodeCharset>& Font::UnicodeCharsets() const
{ return m_charsets; }

Y Font::Ascent() const
{ return m_ascent; }

Y Font::Descent() const
{ return m_descent; }

Y Font::Height() const
{ return m_height; }

Y Font::Lineskip() const
{ return m_lineskip; }

X Font::SpaceWidth() const
{ return m_space_width; }

X Font::RenderGlyph(const Pt& pt, char c) const
{
    if (!detail::ValidUTFChar<char>()(c))
        throw utf8::invalid_utf8(c);

    GlyphMap::const_iterator it = m_glyphs.find(CharToUint32_t(c));
    if (it == m_glyphs.end())
        it = m_glyphs.find(WIDE_SPACE); // print a space when an unrendered glyph is requested
    return RenderGlyph(pt, it->second, 0);
}

X Font::RenderGlyph(const Pt& pt, boost::uint32_t c) const
{
    GlyphMap::const_iterator it = m_glyphs.find(c);
    if (it == m_glyphs.end())
        it = m_glyphs.find(WIDE_SPACE); // print a space when an unrendered glyph is requested
    return RenderGlyph(pt, it->second, 0);
}

X Font::RenderText(const Pt& pt_, const std::string& text) const
{
    Pt pt = pt_;
    X orig_x = pt.x;
    std::string::const_iterator it = text.begin();
    std::string::const_iterator end_it = text.end();
    while (it != end_it) {
        pt.x += RenderGlyph(pt, utf8::next(it, end_it));
    }
    return pt.x - orig_x;
}

void Font::RenderText(const Pt& ul, const Pt& lr, const std::string& text, Flags<TextFormat>& format,
                      const std::vector<LineData>* line_data/* = 0*/, RenderState* render_state/* = 0*/) const
{
    RenderState state;
    if (!render_state)
        render_state = &state;

    // get breakdown of how text is divided into lines
    std::vector<LineData> lines;
    if (!line_data) {
        DetermineLines(text, format, lr.x - ul.x, lines);
        line_data = &lines;
    }

    RenderText(ul, lr, text, format, *line_data, *render_state,
               0, CP0, line_data->size(), CPSize(line_data->back().char_data.size()));
}

void Font::RenderText(const Pt& ul, const Pt& lr, const std::string& text, Flags<TextFormat>& format,
                      const std::vector<LineData>& line_data, RenderState& render_state,
                      std::size_t begin_line, CPSize begin_char,
                      std::size_t end_line, CPSize end_char) const
{
    double orig_color[4];
    glGetDoublev(GL_CURRENT_COLOR, orig_color);

    if (!render_state.colors.empty())
        glColor(render_state.colors.top());

    Y y_origin = ul.y; // default value for FORMAT_TOP
    if (format & FORMAT_BOTTOM)
        y_origin = lr.y - (static_cast<int>(end_line - begin_line - 1) * m_lineskip + m_height);
    else if (format & FORMAT_VCENTER)
        y_origin = ul.y + ((lr.y - ul.y) - (static_cast<int>(end_line - begin_line - 1) * m_lineskip + m_height)) / 2.0;

    for (std::size_t i = begin_line; i < end_line; ++i) {
        const LineData& line = line_data[i];
        X x_origin = ul.x; // default value for FORMAT_LEFT
        if (line.justification == ALIGN_RIGHT)
            x_origin = lr.x - line.Width();
        else if (line.justification == ALIGN_CENTER)
            x_origin = ul.x + ((lr.x - ul.x) - line.Width()) / 2.0;
        Y y = y_origin + static_cast<int>(i - begin_line) * m_lineskip;
        X x = x_origin;

        std::string::const_iterator string_end_it = text.end();
        for (CPSize j = ((i == begin_line) ? begin_char : CP0);
             j < ((i == end_line - 1) ? end_char : CPSize(line.char_data.size()));
             ++j) {
            for (std::size_t k = 0; k < line.char_data[Value(j)].tags.size(); ++k) {
                HandleTag(line.char_data[Value(j)].tags[k], orig_color, render_state);
            }
            boost::uint32_t c = utf8::peek_next(text.begin() + Value(line.char_data[Value(j)].string_index), string_end_it);
            assert((text[Value(line.char_data[Value(j)].string_index)] == '\n') == (c == WIDE_NEWLINE));
            if (c == WIDE_NEWLINE)
                continue;
            GlyphMap::const_iterator it = m_glyphs.find(c);
            if (it == m_glyphs.end())
                x = x_origin + line.char_data[Value(j)].extent; // move forward by the extent of the character when a whitespace or unprintable glyph is requested
            else
                x += RenderGlyph(Pt(x, y), it->second, &render_state);
        }
    }

    glColor4dv(orig_color);
}

void Font::ProcessTagsBefore(const std::vector<LineData>& line_data, RenderState& render_state,
                             std::size_t begin_line, CPSize begin_char) const
{
    double orig_color[4];
    glGetDoublev(GL_CURRENT_COLOR, orig_color);

    for (std::size_t i = 0; i < begin_line; ++i) {
        const LineData& line = line_data[i];
        for (CPSize j = CP0;
             j < ((i == begin_line - 1) ? begin_char : CPSize(line.char_data.size()));
             ++j) {
            for (std::size_t k = 0; k < line.char_data[Value(j)].tags.size(); ++k) {
                HandleTag(line.char_data[Value(j)].tags[k], orig_color, render_state);
            }
        }
    }
}

Pt Font::DetermineLines(const std::string& text, Flags<TextFormat>& format, X box_width,
                        std::vector<LineData>& line_data) const
{ return DetermineLinesImpl(text, format, box_width, line_data, 0); }

Pt Font::DetermineLines(const std::string& text, Flags<TextFormat>& format, X box_width,
                        std::vector<LineData>& line_data,
                        std::vector<boost::shared_ptr<TextElement> >& text_elements) const
{
    assert(text_elements.empty());
    return DetermineLinesImpl(text, format, box_width, line_data, &text_elements);
}

Pt Font::DetermineLines(const std::string& text, Flags<TextFormat>& format, X box_width,
                        const std::vector<boost::shared_ptr<TextElement> >& text_elements,
                        std::vector<LineData>& line_data) const
{
    assert(!text_elements.empty());
    return DetermineLinesImpl(text,
                              format,
                              box_width,
                              line_data,
                              const_cast<std::vector<boost::shared_ptr<TextElement> >*>(&text_elements));
}

Pt Font::TextExtent(const std::string& text, Flags<TextFormat> format/* = FORMAT_NONE*/, X box_width/* = X0*/) const
{
    std::vector<LineData> lines;
    return DetermineLines(text, format, box_width ? box_width : X(1 << 15), lines);
}

Pt Font::TextExtent(const std::string& text, const std::vector<LineData>& line_data) const
{
    Pt retval;
    for (std::size_t i = 0; i < line_data.size(); ++i) {
        if (retval.x < line_data[i].Width())
            retval.x = line_data[i].Width();
    }
    retval.y = text.empty() ? Y0 : (static_cast<int>(line_data.size()) - 1) * m_lineskip + m_height;
    return retval;
}

void Font::RegisterKnownTag(const std::string& tag)
{ s_known_tags.insert(tag); }

void Font::RemoveKnownTag(const std::string& tag)
{
    if (s_action_tags.find(tag) == s_action_tags.end())
        s_known_tags.erase(tag);
}

void Font::ClearKnownTags()
{
    s_action_tags.clear();
    s_action_tags.insert("i");
    s_action_tags.insert("u");
    s_action_tags.insert("rgba");
    s_action_tags.insert(ALIGN_LEFT_TAG);
    s_action_tags.insert(ALIGN_CENTER_TAG);
    s_action_tags.insert(ALIGN_RIGHT_TAG);
    s_action_tags.insert(PRE_TAG);
    s_known_tags = s_action_tags;
}

void Font::ThrowBadGlyph(const std::string& format_str, boost::uint32_t c)
{
    boost::format format(isprint(c) ? "%c" : "U+%x");
    throw BadGlyph(boost::io::str(boost::format(format_str) % boost::io::str(format % c)));
}

Pt Font::DetermineLinesImpl(const std::string& text,
                            Flags<TextFormat>& format,
                            X box_width,
                            std::vector<LineData>& line_data,
                            std::vector<boost::shared_ptr<TextElement> >* text_elements_ptr) const
{
    ValidateFormat(format);

#if DEBUG_DETERMINELINES
    std::cout << "Font::DetermineLines(text=\"" << text << "\" (@ "
              << static_cast<const void*>(&*text.begin()) << ") format="
              << format << " box_width=" << box_width << ")" << std::endl;
#endif

    std::vector<boost::shared_ptr<TextElement> > local_text_elements;

    std::vector<boost::shared_ptr<TextElement> >& text_elements =
        text_elements_ptr ? *text_elements_ptr : local_text_elements;

    if (text_elements.empty()) {
        using namespace boost::xpressive;

        std::stack<Substring> tag_stack;
        bool ignore_tags = format & FORMAT_IGNORETAGS;
        MatchesKnownTag matches_known_tag(s_known_tags, ignore_tags);
        MatchesTopOfStack matches_tag_stack(tag_stack, ignore_tags);

        mark_tag tag_name_tag(1);
        mark_tag open_bracket_tag(2);
        mark_tag close_bracket_tag(3);
        mark_tag whitespace_tag(4);
        mark_tag text_tag(5);

        const sregex TAG_PARAM =
            -+~set[_s | '<'];
        const sregex OPEN_TAG_NAME =
            (+_w)[check(matches_known_tag)];
        const sregex CLOSE_TAG_NAME =
            (+_w)[check(matches_tag_stack)];
        const sregex WHITESPACE =
            (*blank >> (_ln | (set = '\n', '\r', '\f'))) | +blank;
        const sregex TEXT =
            ('<' >> *~set[_s | '<']) | (+~set[_s | '<']);
        const sregex EVERYTHING =
            ('<' >> (tag_name_tag = OPEN_TAG_NAME) >> repeat<0, 9>(+blank >> TAG_PARAM) >> (open_bracket_tag.proto_base() = '>'))
            [Push(boost::xpressive::ref(text), boost::xpressive::ref(tag_stack), ref(ignore_tags), tag_name_tag)] |
            ("</" >> (tag_name_tag = CLOSE_TAG_NAME) >> (close_bracket_tag.proto_base() = '>')) |
            (whitespace_tag = WHITESPACE) |
            (text_tag = TEXT);

        sregex_iterator it(text.begin(), text.end(), EVERYTHING);
        sregex_iterator end_it;
        while (it != end_it)
        {
            // consolidate adjacent blocks of text
            bool need_increment = false;
            Substring combined_text;
            if ((*it)[text_tag].matched) {
                while (it != end_it && (*it)[text_tag].matched) {
                    if (combined_text.empty())
                        combined_text = Substring(text, (*it)[text_tag]);
                    else
                        combined_text += (*it)[text_tag];
                    ++it;
                }
            } else {
                need_increment = true;
            }

            if (combined_text.empty()) {
                if ((*it)[open_bracket_tag].matched) {
                    boost::shared_ptr<Font::FormattingTag> element(new Font::FormattingTag(false));
                    element->text = Substring(text, (*it)[0]);
                    if (1 < (*it).nested_results().size()) {
                        element->params.reserve((*it).nested_results().size() - 1);
                        for (smatch::nested_results_type::const_iterator nested_it =
                                 ++(*it).nested_results().begin();
                             nested_it != (*it).nested_results().end();
                             ++nested_it) {
                            element->params.push_back(
                                Substring(text, (*nested_it)[0]));
                        }
                    }
                    element->tag_name = Substring(text, (*it)[tag_name_tag]);
                    text_elements.push_back(element);
                } else if ((*it)[close_bracket_tag].matched) {
                    boost::shared_ptr<Font::FormattingTag> element(new Font::FormattingTag(true));
                    element->text = Substring(text, (*it)[0]);
                    element->tag_name = Substring(text, (*it)[tag_name_tag]);
                    text_elements.push_back(element);
                } else if ((*it)[whitespace_tag].matched) {
                    boost::shared_ptr<Font::TextElement> element(new Font::TextElement(true, false));
                    element->text = Substring(text, (*it)[whitespace_tag]);
                    text_elements.push_back(element);
                    char last_char = *boost::prior(element->text.end());
                    if (last_char == '\n' || last_char == '\f' || last_char == '\r') {
                        boost::shared_ptr<Font::TextElement> element(new Font::TextElement(false, true));
                        text_elements.push_back(element);
                    }
                }
            } else {
                boost::shared_ptr<Font::TextElement> element(new Font::TextElement(false, false));
                element->text = combined_text;
                text_elements.push_back(element);
            }

            if (need_increment)
                ++it;
        }

        // fill in the widths of code points in each TextElement
        const GlyphMap::const_iterator WIDE_SPACE_IT = m_glyphs.find(WIDE_SPACE);
        for (std::size_t i = 0; i < text_elements.size(); ++i) {
            std::string::const_iterator it = text_elements[i]->text.begin();
            std::string::const_iterator end_it = text_elements[i]->text.end();
            while (it != end_it) {
                text_elements[i]->widths.push_back(X0);
                boost::uint32_t c = utf8::next(it, end_it);
                if (c != WIDE_NEWLINE) {
                    GlyphMap::const_iterator it = m_glyphs.find(c);
                    // use a space when an unrendered glyph is requested (the
                    // space chararacter is always renderable)
                    if (it == m_glyphs.end())
                        it = WIDE_SPACE_IT;
                    text_elements[i]->widths.back() = it->second.advance;
                }
            }
        }

#if DEBUG_DETERMINELINES
        std::cout << "results of parse:\n";
        for (std::size_t i = 0; i < text_elements.size(); ++i) {
            if (boost::shared_ptr<FormattingTag> tag_elem = boost::dynamic_pointer_cast<FormattingTag>(text_elements[i])) {
                std::cout << "FormattingTag\n    text=\"" << tag_elem->text << "\" (@ "
                          << static_cast<const void*>(&*tag_elem->text.begin()) << ")\n    widths=";
                for (std::size_t j = 0; j < tag_elem->widths.size(); ++j) {
                    std::cout << tag_elem->widths[j] << " ";
                }
                std::cout << "\n    whitespace=" << tag_elem->whitespace << "\n    newline=" << tag_elem->newline << "\n    params=\n";
                for (std::size_t j = 0; j < tag_elem->params.size(); ++j) {
                    std::cout << "        \"" << tag_elem->params[j] << "\"\n";
                }
                std::cout << "    tag_name=\"" << tag_elem->tag_name << "\"\n    close_tag=" << tag_elem->close_tag << "\n";
            } else {
                boost::shared_ptr<TextElement> elem = text_elements[i];
                std::cout << "TextElement\n    text=\"" << elem->text << "\" (@ "
                          << static_cast<const void*>(&*elem->text.begin()) << ")\n    widths=";
                for (std::size_t j = 0; j < elem->widths.size(); ++j) {
                    std::cout << elem->widths[j] << " ";
                }
                std::cout << "\n    whitespace=" << elem->whitespace << "\n    newline=" << elem->newline << "\n";
            }
            std::cout << "    string_size=" << text_elements[i]->StringSize() << "\n";
            std::cout << "\n";
        }
        std::cout << std::endl;
#endif
    }

    RenderState render_state;
    int tab_width = 8; // default tab width
    X tab_pixel_width = tab_width * m_space_width; // get the length of a tab stop
    bool expand_tabs = format & FORMAT_LEFT; // tab expansion only takes place when the lines are left-justified (otherwise, tabs are just spaces)
    Alignment orig_just = ALIGN_NONE;
    if (format & FORMAT_LEFT)
        orig_just = ALIGN_LEFT;
    if (format & FORMAT_CENTER)
        orig_just = ALIGN_CENTER;
    if (format & FORMAT_RIGHT)
        orig_just = ALIGN_RIGHT;
    bool last_line_of_curr_just = false; // is this the last line of the current justification? (for instance when a </right> tag is encountered)

    line_data.clear();
    line_data.push_back(LineData());
    line_data.back().justification = orig_just;

    X x = X0;
    // the position within the original string of the current TextElement
    StrSize original_string_offset(0);
    // the index of the first code point of the current TextElement
    CPSize code_point_offset(0);
    std::vector<boost::shared_ptr<TextElement> > pending_formatting_tags;
    for (std::size_t i = 0; i < text_elements.size(); ++i) {
        boost::shared_ptr<TextElement> elem = text_elements[i];
        // if a newline is explicitly requested, start a new one
        if (elem->Type() == TextElement::NEWLINE) {
            line_data.push_back(LineData());
            SetJustification(last_line_of_curr_just,
                             line_data.back(),
                             orig_just,
                             line_data[line_data.size() - 2].justification);
            x = X0;
        } else if (elem->Type() == TextElement::WHITESPACE) {
            std::string::const_iterator it = elem->text.begin();
            std::string::const_iterator end_it = elem->text.end();
            while (it != end_it) {
                StrSize char_index(std::distance(elem->text.begin(), it));
                boost::uint32_t c = utf8::next(it, end_it);
                StrSize char_size = std::distance(elem->text.begin(), it) - char_index;
                if (c != WIDE_CR && c != WIDE_FF) {
                    X advance_position = x + m_space_width;
                    if (c == WIDE_TAB && expand_tabs)
                        advance_position = (((x / tab_pixel_width) + 1) * tab_pixel_width);
                    else if (c == WIDE_NEWLINE)
                        advance_position = x;
                    X advance = advance_position - x;
                    // if we're using linewrap and this space won't fit on
                    // this line
                    if ((format & FORMAT_LINEWRAP) && box_width < advance_position) {
                        if (!x && box_width < advance) {
                            // if the space is larger than the line and alone
                            // on the line, let the space overrun this line
                            // and then start a new one
                            line_data.push_back(LineData());
                            x = X0; // reset the x-position to 0
                            SetJustification(last_line_of_curr_just,
                                             line_data.back(),
                                             orig_just,
                                             line_data[line_data.size() - 2].justification);
                        } else {
                            // otherwise start a new line and put the space there
                            line_data.push_back(LineData());
                            x = advance;
                            line_data.back().char_data.push_back(
                                LineData::CharData(x,
                                                   original_string_offset + char_index,
                                                   char_size,
                                                   code_point_offset,
                                                   pending_formatting_tags));
                            pending_formatting_tags.clear();
                            SetJustification(last_line_of_curr_just,
                                             line_data.back(),
                                             orig_just,
                                             line_data[line_data.size() - 2].justification);
                        }
                    } else { // there's room for the space, or we're not using linewrap
                        x += advance;
                        line_data.back().char_data.push_back(
                            LineData::CharData(x,
                                               original_string_offset + char_index,
                                               char_size,
                                               code_point_offset,
                                               pending_formatting_tags));
                        pending_formatting_tags.clear();
                    }
                }
                ++code_point_offset;
            }
        } else if (elem->Type() == TextElement::TEXT) {
            if (format & FORMAT_WORDBREAK) {
                // if the text "word" overruns this line, and isn't alone on
                // this line, move it down to the next line
                if (box_width < x + elem->Width() && x) {
                    line_data.push_back(LineData());
                    x = X0;
                    SetJustification(last_line_of_curr_just,
                                     line_data.back(),
                                     orig_just,
                                     line_data[line_data.size() - 2].justification);
                }
                std::string::const_iterator it = elem->text.begin();
                std::string::const_iterator end_it = elem->text.end();
                std::size_t i = 0;
                while (it != end_it) {
                    StrSize char_index(std::distance(elem->text.begin(), it));
                    utf8::next(it, end_it);
                    StrSize char_size = std::distance(elem->text.begin(), it) - char_index;
                    x += elem->widths[i];
                    line_data.back().char_data.push_back(
                        LineData::CharData(x,
                                           original_string_offset + char_index,
                                           char_size,
                                           code_point_offset,
                                           pending_formatting_tags));
                    pending_formatting_tags.clear();
                    ++i;
                    ++code_point_offset;
                }
            } else {
                std::string::const_iterator it = elem->text.begin();
                std::string::const_iterator end_it = elem->text.end();
                std::size_t i = 0;
                while (it != end_it) {
                    StrSize char_index(std::distance(elem->text.begin(), it));
                    utf8::next(it, end_it);
                    StrSize char_size = std::distance(elem->text.begin(), it) - char_index;
                    // if the char overruns this line, and isn't alone on this
                    // line, move it down to the next line
                    if ((format & FORMAT_LINEWRAP) && box_width < x + elem->widths[i] && x) {
                        line_data.push_back(LineData());
                        x = elem->widths[i];
                        line_data.back().char_data.push_back(
                            LineData::CharData(x,
                                               original_string_offset + char_index,
                                               char_size,
                                               code_point_offset,
                                               pending_formatting_tags));
                        pending_formatting_tags.clear();
                        SetJustification(last_line_of_curr_just,
                                         line_data.back(),
                                         orig_just,
                                         line_data[line_data.size() - 2].justification);
                    } else {
                        // there's room for this char on this line, or there's
                        // no wrapping in use
                        x += elem->widths[i];
                        line_data.back().char_data.push_back(
                            LineData::CharData(x,
                                               original_string_offset + char_index, 
                                               char_size,
                                               code_point_offset,
                                               pending_formatting_tags));
                        pending_formatting_tags.clear();
                    }
                    ++i;
                    ++code_point_offset;
                }
            }
        } else if (elem->Type() == TextElement::OPEN_TAG) {
            assert(boost::dynamic_pointer_cast<FormattingTag>(elem));
            boost::shared_ptr<FormattingTag> elem_as_tag =
                boost::static_pointer_cast<FormattingTag>(elem);
            if (elem_as_tag->tag_name == ALIGN_LEFT_TAG)
                line_data.back().justification = ALIGN_LEFT;
            else if (elem_as_tag->tag_name == ALIGN_CENTER_TAG)
                line_data.back().justification = ALIGN_CENTER;
            else if (elem_as_tag->tag_name == ALIGN_RIGHT_TAG)
                line_data.back().justification = ALIGN_RIGHT;
            else if (elem_as_tag->tag_name != PRE_TAG)
                pending_formatting_tags.push_back(elem);
            last_line_of_curr_just = false;
            code_point_offset += elem->CodePointSize();
        } else if (elem->Type() == TextElement::CLOSE_TAG) {
            assert(boost::dynamic_pointer_cast<FormattingTag>(elem));
            boost::shared_ptr<FormattingTag> elem_as_tag =
                boost::static_pointer_cast<FormattingTag>(elem);
            if ((elem_as_tag->tag_name == ALIGN_LEFT_TAG && line_data.back().justification == ALIGN_LEFT) ||
                (elem_as_tag->tag_name == ALIGN_CENTER_TAG && line_data.back().justification == ALIGN_CENTER) ||
                (elem_as_tag->tag_name == ALIGN_RIGHT_TAG && line_data.back().justification == ALIGN_RIGHT))
                last_line_of_curr_just = true;
            else if (elem_as_tag->tag_name != PRE_TAG)
                pending_formatting_tags.push_back(elem);
            code_point_offset += elem->CodePointSize();
        }
        original_string_offset += elem->StringSize();
    }
    // disregard the final pending formatting tag, if any, since this is the
    // end of the text, and so it cannot have any effect

#if DEBUG_DETERMINELINES
    std::cout << "Line breakdown:\n";
    for (std::size_t i = 0; i < line_data.size(); ++i) {
        std::cout << "Line " << i << ":\n    extents=";
        for (std::size_t j = 0; j < line_data[i].char_data.size(); ++j) {
            std::cout << line_data[i].char_data[j].extent << " ";
        }
        std::cout << "\n    string indices=";
        for (std::size_t j = 0; j < line_data[i].char_data.size(); ++j) {
            std::cout << line_data[i].char_data[j].string_index << " ";
        }
        std::cout << "\n    code point indices=";
        for (std::size_t j = 0; j < line_data[i].char_data.size(); ++j) {
            std::cout << line_data[i].char_data[j].code_point_index << " ";
        }
        std::cout << "\n    chars on line: \"";
        for (std::size_t j = 0; j < line_data[i].char_data.size(); ++j) {
            std::cout << text[Value(line_data[i].char_data[j].string_index)];
        }
        std::cout << "\"" << std::endl;
        for (std::size_t j = 0; j < line_data[i].char_data.size(); ++j) {
            for (std::size_t k = 0; k < line_data[i].char_data[j].tags.size(); ++k) {
                if (boost::shared_ptr<FormattingTag> tag_elem = line_data[i].char_data[j].tags[k]) {
                    std::cout << "FormattingTag @" << j << "\n    text=\"" << tag_elem->text << "\"\n    widths=";
                    for (std::size_t j = 0; j < tag_elem->widths.size(); ++j) {
                        std::cout << tag_elem->widths[j] << " ";
                    }
                    std::cout << "\n    whitespace=" << tag_elem->whitespace
                              << "\n    newline=" << tag_elem->newline << "\n    params=\n";
                    for (std::size_t l = 0; l < tag_elem->params.size(); ++l) {
                        std::cout << "        \"" << tag_elem->params[l] << "\"\n";
                    }
                    std::cout << "    tag_name=\"" << tag_elem->tag_name << "\"\n    close_tag="
                              << tag_elem->close_tag << std::endl;
                }
            }
        }
        std::cout << "    justification=" << line_data[i].justification << "\n" << std::endl;
    }
#endif

    return TextExtent(text, line_data);
}

FT_Error Font::GetFace(FT_Face& face)
{ return FT_New_Face(g_library.m_library, m_font_filename.c_str(), 0, &face); }

FT_Error Font::GetFace(const std::vector<unsigned char>& file_contents, FT_Face& face)
{
    return FT_New_Memory_Face(g_library.m_library, &file_contents[0],
                              file_contents.size(), 0, &face);
}

void Font::CheckFace(FT_Face face, FT_Error error)
{
    if (error || !face)
        throw BadFile("Face object created from \"" + m_font_filename + "\" was invalid");
    if (!FT_IS_SCALABLE(face)) {
        throw UnscalableFont("Attempted to create font \"" + m_font_filename +
                             "\" with uscalable font face");
    }
}

void Font::Init(FT_Face& face)
{
    if (s_action_tags.empty()) // if this is the first Font to get initialized, it needs to initialize some static members
        ClearKnownTags();

    FT_Fixed scale;

    if (!m_pt_sz) {
        throw InvalidPointSize("Attempted to create font \"" + m_font_filename +
                               "\" with 0 point size");
    }

    // Set the character size and use default 72 DPI
    if (FT_Set_Char_Size(face, 0, m_pt_sz * 64, 0, 0)) // if error is returned
        throw BadPointSize("Could not set font size while attempting to create font \"" + m_font_filename + "\"");

    // Get the scalable font metrics for this font
    scale = face->size->metrics.y_scale;
    m_ascent = Y(static_cast<int>(face->size->metrics.ascender / 64.0)); // convert from fixed-point 26.6 format
    m_descent = Y(static_cast<int>(face->size->metrics.descender / 64.0)); // convert from fixed-point 26.6 format
    m_height = m_ascent - m_descent + 1;
    m_lineskip = Y(static_cast<int>(face->size->metrics.height / 64.0));
    // underline info
    m_underline_offset = std::floor(FT_MulFix(face->underline_position, scale) / 64.0);
    m_underline_height = std::ceil(FT_MulFix(face->underline_thickness, scale) / 64.0);
    if (m_underline_height < 1.0) {
        m_underline_height = 1.0;
    }
    // italics info
    m_italics_offset = Value(ITALICS_FACTOR * m_height / 2.0);

    // we always need these whitespace, number, and punctuation characters
    std::vector<std::pair<boost::uint32_t, boost::uint32_t> > range_vec(
        PRINTABLE_ASCII_NONALPHA_RANGES.begin(),
        PRINTABLE_ASCII_NONALPHA_RANGES.end());

    // add ASCII letter characters or user-specified scripts to them, if the user specifies a specific set of
    // characters
    if (m_charsets.empty()) {
        range_vec.insert(range_vec.end(),
                         PRINTABLE_ASCII_ALPHA_RANGES.begin(),
                         PRINTABLE_ASCII_ALPHA_RANGES.end());
    } else {
        typedef std::pair<boost::uint32_t, boost::uint32_t> Pair;
        for (std::size_t i = 0; i < m_charsets.size(); ++i) {
            range_vec.push_back(Pair(m_charsets[i].m_first_char, m_charsets[i].m_last_char));
        }
    }

    // define default image buffer size
    const X BUF_WIDTH(256);
    const Y BUF_HEIGHT(256);
    const std::size_t BUF_SZ = Value(BUF_WIDTH) * Value(BUF_HEIGHT);

    // declare std::vector of image buffers into which we will copy glyph images and create first buffer
    std::vector<boost::uint16_t*> buffer_vec; // 16 bpp: we are creating a luminance + alpha image
    std::vector<Pt> buffer_sizes;
    std::map<boost::uint32_t, TempGlyphData> temp_glyph_data;
    boost::uint16_t* temp_buf = new boost::uint16_t[BUF_SZ]; // 16 bpp: we are creating a luminance + alpha image
    std::memset(temp_buf, 0, BUF_SZ * sizeof(boost::uint16_t));
    buffer_vec.push_back(temp_buf);
    buffer_sizes.push_back(Pt(BUF_WIDTH, BUF_HEIGHT));

    X x = X0;
    Y y = Y0;
    X max_x = X0;
    Y max_y = Y0;
    for (std::size_t i = 0; i < range_vec.size(); ++i) {
        boost::uint32_t low = range_vec[i].first;
        boost::uint32_t high = range_vec[i].second;

        // copy glyph images
        for (boost::uint32_t c = low; c < high; ++c) {
            if (temp_glyph_data.find(c) == temp_glyph_data.end() && GenerateGlyph(face, c)) {
                const FT_Bitmap& glyph_bitmap = face->glyph->bitmap;
                if ((glyph_bitmap.width > BUF_WIDTH) | (glyph_bitmap.rows > BUF_HEIGHT)) {
                    ThrowBadGlyph("GG::Font::Init : Glyph too large for buffer'%1%'", c); // catch broken fonts
                }

                if (x + glyph_bitmap.width > BUF_WIDTH) { // start a new row of glyph images
                    if (x > max_x) max_x = x;
                    x = X0;
                    y = max_y;
                }
                if (y + glyph_bitmap.rows > BUF_HEIGHT) { // if there's not enough room, create a new buffer
                    // cut off bottom and side of buffer just written, if it is possible to do so and maintain power-of-two height
                    X pow_of_2_x(NextPowerOfTwo(max_x));
                    Y pow_of_2_y(NextPowerOfTwo(max_y));
                    if (pow_of_2_y < buffer_sizes.back().y)
                        buffer_sizes.back().y = pow_of_2_y;
                    if (pow_of_2_x < buffer_sizes.back().x)
                        buffer_sizes.back().x = pow_of_2_x;
                    x = X0;
                    y = Y0;
                    max_x = X0;
                    max_y = Y0;
                    temp_buf = new boost::uint16_t[BUF_SZ];
                    std::memset(temp_buf, 0, BUF_SZ * sizeof(boost::uint16_t));
                    buffer_vec.push_back(temp_buf);
                    buffer_sizes.push_back(Pt(BUF_WIDTH, BUF_HEIGHT));
                }
                if (y + glyph_bitmap.rows > max_y) {
                    max_y = y + glyph_bitmap.rows;
                }

                boost::uint8_t*  src_start = glyph_bitmap.buffer;
                boost::uint16_t* dst_start = buffer_vec.back() + Value(y) * Value(BUF_WIDTH) + Value(x);

                for (int row = 0; row < glyph_bitmap.rows; ++row) {
                    boost::uint8_t*  src = src_start + row * glyph_bitmap.pitch;
                    boost::uint16_t* dst = dst_start + row * Value(BUF_WIDTH);
                    for (int col = 0; col < glyph_bitmap.width; ++col) {
#ifdef __BIG_ENDIAN__
                        *dst++ = *src++ | (255 << 8); // big-endian uses different byte ordering
#else
                        *dst++ = (*src++ << 8) | 255; // alpha is the value from glyph_bitmap; luminance is always 100% white
#endif
                    }
                }

                // record info on how to find and use this glyph later
                temp_glyph_data[c] =
                    TempGlyphData(static_cast<int>(buffer_vec.size()) - 1,
                                  Pt(x, y), Pt(x + glyph_bitmap.width, y + glyph_bitmap.rows),
                                  Y(m_height - 1 + m_descent - face->glyph->bitmap_top),
                                  X(static_cast<int>((std::ceil(face->glyph->metrics.horiBearingX / 64.0)))),
                                  X(static_cast<int>((std::ceil(face->glyph->metrics.horiAdvance / 64.0)))));

                // advance buffer write-position
                x += glyph_bitmap.width;
            }
        }
    }

    // cut off bottom portion of last buffer, if it is possible to do so and maintain power-of-two height
    if (x > max_x) max_x = x;
    X pow_of_2_x(NextPowerOfTwo(max_x));
    Y pow_of_2_y(NextPowerOfTwo(y + m_height));
    if (pow_of_2_y < buffer_sizes.back().y)
        buffer_sizes.back().y = pow_of_2_y;
    if (pow_of_2_x < buffer_sizes.back().x)
        buffer_sizes.back().x = pow_of_2_x;

    // create opengl texture from buffer(s) and release buffer(s)
    for (std::size_t i = 0; i < buffer_vec.size(); ++i) {
        boost::shared_ptr<Texture> temp_texture(new Texture);
        temp_texture->Init(X0, Y0, buffer_sizes[i].x, buffer_sizes[i].y, BUF_WIDTH, (unsigned char*)(buffer_vec[i]), GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2);
        m_textures.push_back(temp_texture);
        delete [] buffer_vec[i];
    }

    // create Glyph objects from temp glyph data
    for (std::map<boost::uint32_t, TempGlyphData>::iterator it = temp_glyph_data.begin(); it != temp_glyph_data.end(); ++it)
        m_glyphs[it->first] = Glyph(m_textures[it->second.idx], it->second.ul, it->second.lr, it->second.y_offset, it->second.left_b, it->second.adv);

    // record the width of the space character
    GlyphMap::const_iterator glyph_it = m_glyphs.find(WIDE_SPACE);
    assert(glyph_it != m_glyphs.end());
    m_space_width = glyph_it->second.advance;
}

bool Font::GenerateGlyph(FT_Face face, boost::uint32_t ch)
{
    bool retval = true;

    // load the glyph
    if (!face)
        throw BadFace("GG::Font::GetGlyphBitmap : invalid font or font face");

    using boost::lexical_cast;
    FT_UInt index = FT_Get_Char_Index(face, ch);
    if (index) {
        if (FT_Load_Glyph(face, index, FT_LOAD_DEFAULT))
            ThrowBadGlyph("GG::Font::GetGlyphBitmap : Freetype could not load the glyph for character '%1%'", ch);

        FT_GlyphSlot glyph = face->glyph;

        // render the glyph
        if (FT_Render_Glyph(glyph, ft_render_mode_normal))
            ThrowBadGlyph("GG::Font::GetGlyphBitmap : Freetype could not render the glyph for character '%1%'", ch);
    } else {
        retval = false;
    }

    return retval;
}

void Font::ValidateFormat(Flags<TextFormat>& format) const
{
    // correct any disagreements in the format flags
    int dup_ct = 0;   // duplication count
    if (format & FORMAT_LEFT) ++dup_ct;
    if (format & FORMAT_RIGHT) ++dup_ct;
    if (format & FORMAT_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_LEFT by default
        format &= ~(FORMAT_RIGHT | FORMAT_CENTER);
        format |= FORMAT_LEFT;
    }
    dup_ct = 0;
    if (format & FORMAT_TOP) ++dup_ct;
    if (format & FORMAT_BOTTOM) ++dup_ct;
    if (format & FORMAT_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_TOP by default
        format &= ~(FORMAT_BOTTOM | FORMAT_VCENTER);
        format |= FORMAT_TOP;
    }
    if ((format & FORMAT_WORDBREAK) && (format & FORMAT_LINEWRAP))   // only one of these can be picked; FORMAT_WORDBREAK overrides FORMAT_LINEWRAP
        format &= ~FORMAT_LINEWRAP;
}

X Font::RenderGlyph(const Pt& pt, const Glyph& glyph, const Font::RenderState* render_state) const
{
    if (render_state && render_state->use_italics) {
        // render subtexture to rhombus instead of rectangle
        glBindTexture(GL_TEXTURE_2D, glyph.sub_texture.GetTexture()->OpenGLId());
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(glyph.sub_texture.TexCoords()[0], glyph.sub_texture.TexCoords()[1]);
        glVertex(pt.x + glyph.left_bearing + m_italics_offset, pt.y + glyph.y_offset);
        glTexCoord2f(glyph.sub_texture.TexCoords()[2], glyph.sub_texture.TexCoords()[1]);
        glVertex(pt.x + glyph.sub_texture.Width() + glyph.left_bearing + m_italics_offset, pt.y + glyph.y_offset);
        glTexCoord2f(glyph.sub_texture.TexCoords()[0], glyph.sub_texture.TexCoords()[3]);
        glVertex(pt.x + glyph.left_bearing - m_italics_offset, pt.y + glyph.sub_texture.Height() + glyph.y_offset);
        glTexCoord2f(glyph.sub_texture.TexCoords()[2], glyph.sub_texture.TexCoords()[3]);
        glVertex(pt.x + glyph.sub_texture.Width() + glyph.left_bearing - m_italics_offset,
                 pt.y + glyph.sub_texture.Height() + glyph.y_offset);
        glEnd();
    } else {
        glyph.sub_texture.OrthoBlit(Pt(pt.x + glyph.left_bearing, pt.y + glyph.y_offset));
    }
    if (render_state && render_state->draw_underline) {
        X x1 = pt.x;
        Y_d y1 = pt.y + m_height + m_descent - m_underline_offset;
        X x2 = x1 + glyph.advance;
        Y_d y2 = y1 + m_underline_height;
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glVertex(x1, y2);
        glVertex(x1, y1);
        glVertex(x2, y1);
        glVertex(x2, y2);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
    return glyph.advance;
}

void Font::HandleTag(const boost::shared_ptr<FormattingTag>& tag, double* orig_color,
                     RenderState& render_state) const
{
    if (tag->tag_name == "i") {
        if (tag->close_tag) {
            if (render_state.use_italics) {
                --render_state.use_italics;
            }
        } else {
            ++render_state.use_italics;
        }
    } else if (tag->tag_name == "u") {
        if (tag->close_tag) {
            if (render_state.draw_underline) {
                --render_state.draw_underline;
            }
        } else {
            ++render_state.draw_underline;
        }
    } else if (tag->tag_name == "rgba") {
        if (tag->close_tag) {
            // commenting out this assert because, I think, if a multi-word
            // substring is wrapped in rgba tags and then is split over a line-
            // break, the render state (or at least the colour portion thereof,
            // I think...) is reset, but the closing rgba tag will still be
            // present on the next line, leading to a crash due to a closing
            // rgba tag when the render state for the line has no colour
            if (!render_state.colors.empty()) {
                render_state.colors.pop();
                if (render_state.colors.empty())
                    glColor4dv(orig_color);
                else
                    glColor(render_state.colors.top());
            }
        } else {
            using boost::lexical_cast;
            bool well_formed_tag = true;
            if (4 == tag->params.size()) {
                try {
                    int temp_color[4];
                    GLubyte color[4];
                    temp_color[0] = lexical_cast<int>(tag->params[0]);
                    temp_color[1] = lexical_cast<int>(tag->params[1]);
                    temp_color[2] = lexical_cast<int>(tag->params[2]);
                    temp_color[3] = lexical_cast<int>(tag->params[3]);
                    if (0 <= temp_color[0] && temp_color[0] <= 255 &&
                        0 <= temp_color[1] && temp_color[1] <= 255 &&
                        0 <= temp_color[2] && temp_color[2] <= 255 &&
                        0 <= temp_color[3] && temp_color[3] <= 255) {
                        color[0] = temp_color[0];
                        color[1] = temp_color[1];
                        color[2] = temp_color[2];
                        color[3] = temp_color[3];
                        glColor4ubv(color);
                        render_state.colors.push(Clr(color[0], color[1], color[2], color[3]));
                    } else {
                        well_formed_tag = false;
                    }
                } catch (boost::bad_lexical_cast) {
                    try {
                        double color[4];
                        color[0] = lexical_cast<double>(tag->params[0]);
                        color[1] = lexical_cast<double>(tag->params[1]);
                        color[2] = lexical_cast<double>(tag->params[2]);
                        color[3] = lexical_cast<double>(tag->params[3]);
                        if (0.0 <= color[0] && color[0] <= 1.0 &&
                            0.0 <= color[1] && color[1] <= 1.0 &&
                            0.0 <= color[2] && color[2] <= 1.0 &&
                            0.0 <= color[3] && color[3] <= 1.0) {
                            glColor4dv(color);
                            render_state.colors.push(FloatClr(color[0], color[1], color[2], color[3]));
                        } else {
                            well_formed_tag = false;
                        }
                    } catch (boost::bad_lexical_cast) {
                        well_formed_tag = false;
                    }
                }
            } else {
                well_formed_tag = false;
            }
            if (!well_formed_tag) {
                std::cerr << "GG::Font : Encountered malformed <rgba> formatting tag: "
                          << tag->text;
            }
        }
    }
}

bool Font::IsDefaultFont()
{ return m_font_filename == StyleFactory::DefaultFontName(); }

boost::shared_ptr<Font> Font::GetDefaultFont(unsigned int pts)
{ return GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(pts); }


///////////////////////////////////////
// class GG::FontManager
///////////////////////////////////////
// FontKey 
FontManager::FontKey::FontKey(const std::string& str, unsigned int pts) :
    filename(str),
    points(pts)
{}

bool FontManager::FontKey::operator<(const FontKey& rhs) const
{ return (filename < rhs.filename || (filename == rhs.filename && points < rhs.points)); }

// FontManager
// static(s)
const boost::shared_ptr<Font> FontManager::EMPTY_FONT(new Font("", 0));

FontManager::FontManager()
{}

bool FontManager::HasFont(const std::string& font_filename, unsigned int pts) const
{ return m_rendered_fonts.find(FontKey(font_filename, pts)) != m_rendered_fonts.end(); }

boost::shared_ptr<Font> FontManager::GetFont(const std::string& font_filename, unsigned int pts)
{
    std::vector<UnicodeCharset> v;
    std::vector<UnicodeCharset>::iterator it = v.end();
    return GetFont(font_filename, pts, it, it);
}

boost::shared_ptr<Font> FontManager::GetFont(const std::string& font_filename, unsigned int pts,
                                             const std::vector<unsigned char>& file_contents)
{
    std::vector<UnicodeCharset> v;
    std::vector<UnicodeCharset>::iterator it = v.end();
    return GetFont(font_filename, pts, file_contents, it, it);
}

void FontManager::FreeFont(const std::string& font_filename, unsigned int pts)
{
    FontKey key(font_filename, pts);
    std::map<FontKey, boost::shared_ptr<Font> >::iterator it = m_rendered_fonts.find(key);
    if (it != m_rendered_fonts.end())
        m_rendered_fonts.erase(it);
}

FontManager& GG::GetFontManager()
{
    static FontManager manager;
    return manager;
}
