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
#include <GG/GLClientAndServerBuffer.h>

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

    const std::string ITALIC_TAG = "i";
    const std::string SHADOW_TAG = "s";
    const std::string UNDERLINE_TAG = "u";
    const std::string RGBA_TAG = "rgba";
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

    /** This is used to collect data on the glyphs as they are recorded into buffers, for use in creating Glyph objects at the end
        of Font's constructor.*/
    struct TempGlyphData
    {
        TempGlyphData() {}
        TempGlyphData(const Pt& ul_, const Pt& lr_, Y y_ofs, X lb, X a) :
            ul(ul_), lr(lr_), y_offset(y_ofs), left_b(lb), adv(a) {}
        Pt          ul, lr;   ///< area of glyph subtexture within texture
        Y           y_offset; ///< vertical offset to draw texture (may be negative!)
        X           left_b;   ///< left bearing (see Glyph)
        X           adv;      ///< advance of glyph (see Glyph)
    };

    /// A two dimensional grid of pixels that expands to
    /// fit any write much like an stl vector, but in 2d.
    template<typename T>
    class Buffer2d
    {
    public:
        /// Create a new 2D buffer
        /// \param initial_width Initial width to allocate
        /// \param initial_height Initial height to allocate
        /// \param default_value The value to fill empty space with whenever it appears
        Buffer2d(X initial_width, Y initial_height, const T& default_value):
            m_capacity_width(initial_width),
            m_capacity_height(initial_height),
            m_data(Value(initial_width)*Value(initial_height), default_value),
            m_current_width(initial_width),
            m_current_height(initial_height),
            m_default_value(default_value)
        {}

        /// Access point \x,\y, expanding the buffer if it does not exist yet
        T& at(X x, Y y)
        {
            EnsureFit(x, y);
            return this->get(x,y);
        }

        /// Access point \x, \y without any checks
        T& get(X x, Y y)
        { return m_data[Value(m_capacity_width)*Value(y) + Value(x)]; }

        /// Returns the current highest x the user has requested to exist
        X CurrentWidth() const
        { return m_current_width; }

        /// Returns the current highest y the user has requested to exist
        Y CurrentHeight() const
        { return m_capacity_height; }

        /// Returns the actual width of the storage area allocated so far
        X BufferWidth() const
        { return m_capacity_width; }

        /// Returns the actual height of the storage area allocated so far
        Y BufferHeight() const
        { return m_capacity_height; }

        /// Return a pointer to the storage buffer where the data is kept
        T* Buffer()
        { return &*m_data.begin(); }

        /// Returns the size of the storage buffer where the data is kept
        std::size_t BufferSize() const
        { return m_data.size(); }

        /// Makes the size of the underlying buffer the smallest power of power of two
        /// rectangle that can accommodate CurrentWidth() and CurrentHeight()
        void MakePowerOfTwo()
        { ResizeCapacity(NextPowerOfTwo(m_current_width), NextPowerOfTwo(m_current_height)); }

    private:
        X m_capacity_width; // How wide the reserved buffer is
        Y m_capacity_height; // How hight the reserved buffer is
        std::vector<T> m_data;
        X m_current_width; // The highest x coordinate written to
        Y m_current_height; // The highest y coordinate written to
        const T m_default_value; // The value with which to fill all emerging empty slots in the buffer

        void EnsureFit(X x, Y y)
        {
            X new_width = std::max(m_current_width, x + 1); // Zero indexed => width = max_x + 1
            Y new_height = std::max(m_current_height, y + 1); // Also zero indexed
            X new_capacity_width = m_capacity_width;
            Y new_capacity_height = m_capacity_height;
            while (new_width > new_capacity_width) {
                new_capacity_width *= 2;
            }
            while (new_height > new_capacity_height) {
                new_capacity_height *= 2;
            }

            ResizeCapacity(new_capacity_width, new_capacity_height);
            m_current_width = new_width;
            m_current_height = new_height;
        }

        void ResizeCapacity(X new_capacity_width, Y new_capacity_height)
        {
            // If there really was a change, we need to recreate our storage.
            // This is expensive, but since we double the size every time,
            // the cost of adding data here in some sane order is amortized constant
            if (new_capacity_width != m_capacity_width || new_capacity_height != m_capacity_height) {
                // Create new storage and copy old data. A linear copy won't do, there
                // will be a new mapping from 2d indexes to 1d memory.
                std::vector<T> new_data(Value(new_capacity_width)*Value(new_capacity_height), m_default_value);
                for (Y y_i = Y0; y_i < m_current_height && y_i < new_capacity_height; ++y_i) {
                    for (X x_i = X0; x_i < m_current_width && x_i < new_capacity_width; ++x_i) {
                        unsigned pos = Value(new_capacity_width) * Value(y_i) + Value(x_i);
                        new_data[pos] = get(x_i, y_i);
                    }
                }
                std::swap(m_data, new_data);
                m_capacity_width = new_capacity_width;
                m_capacity_height = new_capacity_height;
            }
        }
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

    struct PushSubmatchOntoStackP
    {
        typedef void result_type;
        void operator()(const std::string* str,
                        std::stack<Font::Substring>& tag_stack,
                        bool& ignore_tags,
                        const boost::xpressive::ssub_match& sub) const
        {
            tag_stack.push(Font::Substring(*str, sub));
            if (tag_stack.top() == PRE_TAG)
                ignore_tags = true;
        }
    };
    const boost::xpressive::function<PushSubmatchOntoStackP>::type PushP = {{}};

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
const TextFormat GG::FORMAT_NOWRAP       (1 << 6);
const TextFormat GG::FORMAT_WORDBREAK    (1 << 7);
const TextFormat GG::FORMAT_LINEWRAP     (1 << 8);
const TextFormat GG::FORMAT_IGNORETAGS   (1 << 9);

GG_FLAGSPEC_IMPL(TextFormat);

namespace {
    bool RegisterTextFormats()
    {
        FlagSpec<TextFormat>& spec = FlagSpec<TextFormat>::instance();
        spec.insert(FORMAT_NONE,        "FORMAT_NONE",      true);
        spec.insert(FORMAT_VCENTER,     "FORMAT_VCENTER",   true);
        spec.insert(FORMAT_TOP,         "FORMAT_TOP",       true);
        spec.insert(FORMAT_BOTTOM,      "FORMAT_BOTTOM",    true);
        spec.insert(FORMAT_CENTER,      "FORMAT_CENTER",    true);
        spec.insert(FORMAT_LEFT,        "FORMAT_LEFT",      true);
        spec.insert(FORMAT_RIGHT,       "FORMAT_RIGHT",     true);
        spec.insert(FORMAT_NOWRAP,      "FORMAT_NOWRAP",    true);
        spec.insert(FORMAT_WORDBREAK,   "FORMAT_WORDBREAK", true);
        spec.insert(FORMAT_LINEWRAP,    "FORMAT_LINEWRAP",  true);
        spec.insert(FORMAT_IGNORETAGS,  "FORMAT_IGNORETAGS",true);
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
        const std::vector<Font::LineData::CharData>& char_data = line_data[i].char_data;
        if (!char_data.empty() &&
            char_data.front().code_point_index <= index &&
            index <= char_data.back().code_point_index)
        {
            retval.first = i;
            retval.second = index - char_data.front().code_point_index;
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
    use_shadow(0),
    draw_underline(0)
{
    // Initialize the color stack with the current color
    GLfloat current[4];
    glGetFloatv(GL_CURRENT_COLOR, current);
    PushColor(current[0]*255, current[1]*255, current[2]*255, current[3]*255);
}

Font::RenderState::RenderState (Clr color):
    use_italics(0),
    use_shadow(0),
    draw_underline(0)
{
    PushColor(color.r, color.g, color.b, color.a);
}

void Font::RenderState::PushColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    GG::Clr color(r, g, b, a);
    // The same color may end up being stored multiple times, but the cost of deduplication
    // is greater than the cost of just letting it be so.
    color_index_stack.push(used_colors.size());
    used_colors.push_back(color);
}

void Font::RenderState::PopColor()
{
    // Never remove the initial color from the stack
    if(color_index_stack.size() > 1)
        color_index_stack.pop();
}

int Font::RenderState::CurrentIndex() const
{ return color_index_stack.top(); }

const Clr& Font::RenderState::CurrentColor() const
{ return used_colors[CurrentIndex()]; }

bool Font::RenderState::ColorsEmpty() const
{ return color_index_stack.size() <= 1; }


///////////////////////////////////////
// class GG::Font::RenderCache
///////////////////////////////////////

// Must be here for scoped_ptr deleter to work
Font::RenderCache::RenderCache() :
    vertices(new GG::GL2DVertexBuffer()),
    coordinates(new GG::GLTexCoordBuffer()),
    colors(new GG::GLRGBAColorBuffer()),
    underline_vertices(new GG::GL2DVertexBuffer()),
    underline_colors(new GG::GLRGBAColorBuffer())
{}

// Must be here for scoped_ptr deleter to work
Font::RenderCache::~RenderCache()
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

namespace {
    namespace xpr = boost::xpressive;

    /** CompiledRegex maintains a compiled boost::xpressive regular
        expression that includes a tag stack which can be cleared and
        provided to callers without the overhead of recompiling the
        regular expression.*/
    class CompiledRegex {
        public:
        CompiledRegex(const std::set<std::string>& known_tags, bool strip_unpaired_tags) :
            m_text(0), m_known_tags(&known_tags), m_ignore_tags(false), m_tag_stack(),
            m_EVERYTHING()
        {
            // Synonyms for s1 thru s5 sub matches
            xpr::mark_tag tag_name_tag(1);
            xpr::mark_tag open_bracket_tag(2);
            xpr::mark_tag close_bracket_tag(3);
            xpr::mark_tag whitespace_tag(4);
            xpr::mark_tag text_tag(5);

            // The comments before each regex are intended to clarify the mapping from xpressive
            // notation to the more typical regex notation.  If you read xpressive or don't read
            // regex then ignore them.

            // -+ 'non-greedy',   ~ 'not',   set[|] 'set',    _s 'space' = 'anything but space or <'
            const xpr::sregex TAG_PARAM =
                -+~xpr::set[xpr::_s | '<'];

            //+_w one or more greed word chars,  () group no capture,  [] semantic operation
            const xpr::sregex OPEN_TAG_NAME =
                (+xpr::_w)[xpr::check(boost::bind(&CompiledRegex::MatchesKnownTag, this, _1))];
            // (+_w) one or more greedy word check matches stack
            const xpr::sregex CLOSE_TAG_NAME =
                (+xpr::_w)[xpr::check(boost::bind(&CompiledRegex::MatchesTopOfStack, this, _1))];
            // *blank  'zero or more greedy whitespace',   >> 'followed by',    _ln 'newline',
            // (set = 'a', 'b') is '[ab]',    +blank 'one or more greedy blank'
            const xpr::sregex WHITESPACE =
                (*xpr::blank >> (xpr::_ln | (xpr::set = '\n', '\r', '\f'))) | +xpr::blank;

            // < followed by not space or <   or one or more not space or <
            const xpr::sregex TEXT =
                ('<' >> *~xpr::set[xpr::_s | '<']) | (+~xpr::set[xpr::_s | '<']);

            if (!strip_unpaired_tags) {
                m_EVERYTHING =
                    ('<' >> (tag_name_tag = OPEN_TAG_NAME)           // < followed by TAG_NAME
                     >> xpr::repeat<0, 9>(+xpr::blank >> TAG_PARAM)  // repeat 0 to 9 a single blank followed
                                                                     // by TAG_PARAM
                     >> (open_bracket_tag.proto_base() = '>'))       // s1. close tag and push operation
                    [PushP(xpr::ref(m_text), xpr::ref(m_tag_stack), xpr::ref(m_ignore_tags), tag_name_tag)] |
                    ("</" >> (tag_name_tag = CLOSE_TAG_NAME) >> (close_bracket_tag.proto_base() = '>')) |
                    (whitespace_tag = WHITESPACE) |
                    (text_tag = TEXT);
            } else {
                // don't care about matching with tag stack when
                // matching close tags, or updating the stack when
                // matching open tags
                m_EVERYTHING =
                    ('<' >> OPEN_TAG_NAME >> xpr::repeat<0, 9>(+xpr::blank >> TAG_PARAM) >> '>') |
                    ("</" >> OPEN_TAG_NAME >> '>') |
                    (whitespace_tag = WHITESPACE) |
                    (text_tag = TEXT);
            }
        }

        xpr::sregex& BindRegexToText(const std::string& new_text, bool ignore_tags) {
            if (!m_tag_stack.empty()) {
                std::stack<Font::Substring> empty_stack;
                std::swap(m_tag_stack, empty_stack);
            }
            m_text = &new_text;
            m_ignore_tags = ignore_tags;
            return m_EVERYTHING;
        }

        private:

        bool MatchesKnownTag(const boost::xpressive::ssub_match& sub) {
            return m_ignore_tags ? false : m_known_tags->count(sub.str()) != 0;
        }

        bool MatchesTopOfStack(const boost::xpressive::ssub_match& sub) {
            bool retval = m_tag_stack.empty() ? false : m_tag_stack.top() == sub;
            if (retval) {
                m_tag_stack.pop();
                if (m_tag_stack.empty() || m_tag_stack.top() != PRE_TAG)
                    m_ignore_tags = false;
            }
            return retval;
        }

        const std::string* m_text;
        const std::set<std::string>* m_known_tags;
        bool m_ignore_tags;

        // m_tag_stack is used to track XML opening/closing tags.
        std::stack<Font::Substring> m_tag_stack;

        // The combined regular expression.
        xpr::sregex m_EVERYTHING;
    };

    class TagHandler {
        public:
        TagHandler() :
            m_known_tags(),
            m_regex_w_tags(m_known_tags, false),
            m_regex_w_tags_skipping_unmatched(m_known_tags, true)
        {}

        void Insert(const std::string& tag) {
            m_known_tags.insert(tag);
        }

        void Erase(const std::string& tag) {
            m_known_tags.erase(tag);
        }

        void Clear() {
            m_known_tags.clear();
        }

        const std::set<std::string>& KnownTags()
        { return m_known_tags; }
        // Return a regex bound to \p text using the currently known
        // tags possible \p ignore_tags and/or \p strip_unpaired_tags
        xpr::sregex & Regex(const std::string& text, bool ignore_tags, bool strip_unpaired_tags = false) {
            if (!strip_unpaired_tags)
                return m_regex_w_tags.BindRegexToText(text, ignore_tags);
            else
                return m_regex_w_tags_skipping_unmatched.BindRegexToText(text, ignore_tags);
        }

        private:
        // set of tags known to the handler
        std::set<std::string> m_known_tags;

        // Compiled regular expression including tag stack
        CompiledRegex m_regex_w_tags;

        // Compiled regular expression using tags but skipping unmatched tags.
        CompiledRegex m_regex_w_tags_skipping_unmatched;
    };


    // Global set of known tags. Wrapped in a function for deterministic static initialization order.
    TagHandler& StaticTagHandler()
    {
        static TagHandler tag_handler;
        return tag_handler;
    }

    // Registers the default action and known tags.
    int RegisterDefaultTags() {
        StaticTagHandler().Insert(ITALIC_TAG);
        StaticTagHandler().Insert(SHADOW_TAG);
        StaticTagHandler().Insert(UNDERLINE_TAG);
        StaticTagHandler().Insert(RGBA_TAG);
        StaticTagHandler().Insert(ALIGN_LEFT_TAG);
        StaticTagHandler().Insert(ALIGN_CENTER_TAG);
        StaticTagHandler().Insert(ALIGN_RIGHT_TAG);
        StaticTagHandler().Insert(PRE_TAG);

        // Must have return value to call at static initialization time.
        return 0;
    }

    // Register the default tags at static initialization time.
    int register_tags_dummy = RegisterDefaultTags();
}

///////////////////////////////////////
// class GG::Font
///////////////////////////////////////

Font::Font() :
    m_pt_sz(0),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_shadow_offset(0.0),
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
    m_shadow_offset(0.0),
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
    m_shadow_offset(0.0),
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

X Font::RenderText(const Pt& pt_, const std::string& text) const
{
    Pt pt = pt_;
    X orig_x = pt.x;

    double orig_color[4];
    glGetDoublev(GL_CURRENT_COLOR, orig_color);
    glBindTexture(GL_TEXTURE_2D, m_texture->OpenGLId());

    RenderCache cache;
    RenderState render_state;

    for (std::string::const_iterator text_it = text.begin(); text_it != text.end();) {
        boost::uint32_t c = utf8::next (text_it, text.end());
        GlyphMap::const_iterator it = m_glyphs.find(c);
        if (it == m_glyphs.end()) {
            pt.x += m_space_width; // move forward by the extent of the character when a whitespace or unprintable glyph is requested
        } else {
            pt.x += StoreGlyph(pt, it->second, &render_state, cache);
        }
    }

    cache.vertices->createServerBuffer();
    cache.coordinates->createServerBuffer();
    cache.colors->createServerBuffer();
    RenderCachedText(cache);

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
    RenderCache cache;
    PreRenderText(ul, lr, text, format, line_data, render_state, begin_line, begin_char, end_line, end_char, cache);
    RenderCachedText(cache);
}

void Font::PreRenderText(const Pt& ul, const Pt& lr, const std::string& text, Flags<TextFormat>& format, RenderCache& cache,
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

    PreRenderText(ul, lr, text, format, *line_data, *render_state,
                  0, CP0, line_data->size(), line_data->empty() ? CP0 : CPSize(line_data->back().char_data.size()), cache);
}

void Font::PreRenderText(const Pt& ul, const Pt& lr, const std::string& text, Flags<TextFormat>& format,
                         const std::vector<LineData>& line_data, RenderState& render_state,
                         std::size_t begin_line, CPSize begin_char,
                         std::size_t end_line, CPSize end_char,
                         RenderCache& cache) const
{
    double orig_color[4];
    glGetDoublev(GL_CURRENT_COLOR, orig_color);
    //glBindTexture(GL_TEXTURE_2D, m_texture->OpenGLId());

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

        CPSize start = CP0;
        if (i == begin_line)
            start = std::max(CP0, std::min(begin_char, CPSize(line.char_data.size() - 1)));
        CPSize end = CPSize(line.char_data.size());
        if (i == end_line - 1)
            end = std::max(CP0, std::min(end_char, CPSize(line.char_data.size())));

        std::string::const_iterator string_end_it = text.end();
        for (CPSize j = start; j < end; ++j) {
            const LineData::CharData& char_data = line.char_data[Value(j)];
            for (std::size_t k = 0; k < char_data.tags.size(); ++k) {
                HandleTag(char_data.tags[k], orig_color, render_state);
            }
            boost::uint32_t c = utf8::peek_next(text.begin() + Value(char_data.string_index), string_end_it);
            assert((text[Value(char_data.string_index)] == '\n') == (c == WIDE_NEWLINE));
            if (c == WIDE_NEWLINE)
                continue;
            GlyphMap::const_iterator it = m_glyphs.find(c);
            if (it == m_glyphs.end()) {
                x = x_origin + char_data.extent; // move forward by the extent of the character when a whitespace or unprintable glyph is requested
            } else {
                x += StoreGlyph(Pt(x, y), it->second, &render_state, cache);
            }
        }
    }

    cache.vertices->createServerBuffer();
    cache.coordinates->createServerBuffer();
    cache.colors->createServerBuffer();
}

void Font::RenderCachedText(RenderCache& cache) const
{
    glBindTexture(GL_TEXTURE_2D, m_texture->OpenGLId());

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    cache.vertices->activate();
    cache.coordinates->activate();
    cache.colors->activate();
    glDrawArrays(GL_QUADS, 0,  cache.vertices->size());

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    cache.underline_vertices->activate();
    cache.underline_colors->activate();
    glDrawArrays(GL_QUADS, 0, cache.underline_vertices->size());

    glPopClientAttrib();
}

void Font::ProcessTagsBefore(const std::vector<LineData>& line_data, RenderState& render_state,
                             std::size_t begin_line, CPSize begin_char) const
{
    double orig_color[4];
    glGetDoublev(GL_CURRENT_COLOR, orig_color);

    for (std::size_t i = 0; i <= begin_line; ++i) {
        const LineData& line = line_data[i];
        for (CPSize j = CP0;
             j < ((i == begin_line) ? begin_char : CPSize(line.char_data.size()));
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

std::string Font::StripTags(const std::string& text, bool strip_unpaired_tags)
{
    using namespace boost::xpressive;

    bool temp_bool = false;
    std::stack<Substring> tag_stack;
    MatchesKnownTag matches_known_tag(StaticTagHandler().KnownTags(), temp_bool);
    MatchesTopOfStack matches_tag_stack(tag_stack, temp_bool);

    mark_tag tag_name_tag(1);
    mark_tag open_bracket_tag(2);
    mark_tag close_bracket_tag(3);
    mark_tag printable_text_tag(4);

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
    const sregex PRINTABLE_TEXT = WHITESPACE | TEXT;
    sregex EVERYTHING;
    if (!strip_unpaired_tags)
        EVERYTHING =    // push open tag matches to the tag stack, and make sure close tags match the top open tag on the stack
            ('<' >> (tag_name_tag = OPEN_TAG_NAME) >> repeat<0, 9>(+blank >> TAG_PARAM) >> (open_bracket_tag.proto_base() = '>'))
            [Push(boost::xpressive::ref(text), boost::xpressive::ref(tag_stack), ref(temp_bool), tag_name_tag)] |
            ("</" >> (tag_name_tag = CLOSE_TAG_NAME) >> (close_bracket_tag.proto_base() = '>')) |
            (printable_text_tag = PRINTABLE_TEXT);
    else
        EVERYTHING =    // don't care about matching with tag stack when matching close tags, or updating the stack when matching open tags
            ('<' >> OPEN_TAG_NAME >> repeat<0, 9>(+blank >> TAG_PARAM) >> '>') |
            ("</" >> OPEN_TAG_NAME >> '>') |
            (printable_text_tag = PRINTABLE_TEXT);

    std::string retval;

    // scan through matched markup and text, saving only the non-tag-text
    sregex_iterator it(text.begin(), text.end(), EVERYTHING);
    sregex_iterator end_it;
    for (; it != end_it; ++it) {
        retval += Substring(text, (*it)[printable_text_tag]);
    }

    return retval;
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
{ StaticTagHandler().Insert(tag); }

void Font::RemoveKnownTag(const std::string& tag)
{ StaticTagHandler().Erase(tag); }

void Font::ClearKnownTags()
{
    StaticTagHandler().Clear();

    // Always know the default tags.
    RegisterDefaultTags();
}

void Font::ThrowBadGlyph(const std::string& format_str, boost::uint32_t c)
{
    boost::format format(isprint(c) ? "%c" : "U+%x");
    throw BadGlyph(boost::io::str(boost::format(format_str) % boost::io::str(format % c)));
}

void Font::FillTextElements(const std::string& text,
                            bool ignore_tags,
                            std::vector<boost::shared_ptr<TextElement> >& text_elements) const
{
    if (text_elements.empty()) {
        using namespace boost::xpressive;

        if (text.empty())
            return;

        // Fetch and use the regular expression from the TagHandler which parses all the known XML tags.
        sregex& regex = StaticTagHandler().Regex(text, ignore_tags);
        sregex_iterator it(text.begin(), text.end(), regex);

        // These are the types found by the regular expression: XML open/close tags, text and
        // whitespace.  Each type will correspond to a type of TextElement.
        mark_tag tag_name_tag(1);
        mark_tag open_bracket_tag(2);
        mark_tag close_bracket_tag(3);
        mark_tag whitespace_tag(4);
        mark_tag text_tag(5);

        sregex_iterator end_it;
        while (it != end_it)
        {
            // Consolidate adjacent blocks of text.  If adjacent found substrings are all text, merge
            // them into a single Substring.
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

            // If the element is not a text element then it must be an open tag, a close tag or whitespace.
            if (combined_text.empty()) {

                // Open XML tag.
                if ((*it)[open_bracket_tag].matched) {
                    boost::shared_ptr<Font::FormattingTag> element(new Font::FormattingTag(false));
                    element->text = Substring(text, (*it)[0]);

                    // Check open tags for submatches which are parameters.  For example a Color tag
                    // might have RGB parameters.
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

                // Close XML tag
                } else if ((*it)[close_bracket_tag].matched) {
                    boost::shared_ptr<Font::FormattingTag> element(new Font::FormattingTag(true));
                    element->text = Substring(text, (*it)[0]);
                    element->tag_name = Substring(text, (*it)[tag_name_tag]);
                    text_elements.push_back(element);

                // Whitespace element
                } else if ((*it)[whitespace_tag].matched) {
                    boost::shared_ptr<Font::TextElement> element(new Font::TextElement(true, false));
                    element->text = Substring(text, (*it)[whitespace_tag]);
                    text_elements.push_back(element);

                    // If the last character of a whitespace element is a line ending then create a
                    // newline TextElement.
                    char last_char = *boost::prior(element->text.end());
                    if (last_char == '\n' || last_char == '\f' || last_char == '\r') {
                        // Basic text element.
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

    bool ignore_tags = format & FORMAT_IGNORETAGS;
    FillTextElements(text, ignore_tags, text_elements);

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
    // shadow info
    m_shadow_offset = 1.0;

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

    //Get maximum texture size
    GLint GL_TEX_MAX_SIZE;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GL_TEX_MAX_SIZE);
    const std::size_t TEX_MAX_SIZE = GL_TEX_MAX_SIZE;

    std::map<boost::uint32_t, TempGlyphData> temp_glyph_data;

    // Start with width and height of 16,
    // increase them as needed.
    // We will lay out the glyphs on the texture side by side
    // until the width would reach TEX_MAX_SIZE, then go to the next row.
    // QUESTION: Would a more square-like shape be better for the texture?
    Buffer2d<boost::uint16_t> buffer(X(16), Y(16), 0);

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
                if ((glyph_bitmap.width > TEX_MAX_SIZE) | (glyph_bitmap.rows > TEX_MAX_SIZE)) {
                    ThrowBadGlyph("GG::Font::Init : Glyph too large for buffer'%1%'", c); // catch broken fonts
                }

                if (Value(x) + glyph_bitmap.width >= TEX_MAX_SIZE) { // start a new row of glyph images
                    if (x > max_x) max_x = x;
                    x = X0;
                    y = max_y;
                }
                if (Value(y) + glyph_bitmap.rows >= TEX_MAX_SIZE) {
                    // We cannot make the texture any larger. The font does not fit.
                    ThrowBadGlyph("GG::Font::Init : Face too large for buffer. First glyph to no longer fit: '%1%'", c);
                }
                if (y + Y(glyph_bitmap.rows) > max_y) {
                    max_y = y + Y(glyph_bitmap.rows + 1); //Leave a one pixel gap between glyphs
                }

                boost::uint8_t*  src_start = glyph_bitmap.buffer;
                // Resize buffer to fit new data
                buffer.at(x + X(glyph_bitmap.width), y + Y(glyph_bitmap.rows)) = 0;

                for (unsigned int row = 0; row < glyph_bitmap.rows; ++row) {
                    boost::uint8_t*  src = src_start + row * glyph_bitmap.pitch;
                    boost::uint16_t* dst = &buffer.get(x, y + Y(row));
                    // Rows are always contiguous, so we can copy along a row using simple incrementation
                    for (unsigned int col = 0; col < glyph_bitmap.width; ++col) {
#ifdef __BIG_ENDIAN__
                        *dst++ = *src++ | (255 << 8); // big-endian uses different byte ordering
#else
                        *dst++ = (*src++ << 8) | 255; // alpha is the value from glyph_bitmap; luminance is always 100% white
#endif
                    }
                }

                // record info on how to find and use this glyph later
                temp_glyph_data[c] =
                    TempGlyphData(Pt(x, y), Pt(x + X(glyph_bitmap.width), y + Y(glyph_bitmap.rows)),
                                  Y(m_height - 1 + m_descent - face->glyph->bitmap_top),
                                  X(static_cast<int>((std::ceil(face->glyph->metrics.horiBearingX / 64.0)))),
                                  X(static_cast<int>((std::ceil(face->glyph->metrics.horiAdvance / 64.0)))));

                // advance buffer write-position
                x += X(glyph_bitmap.width + 1); //Leave a one pixel gap between glyphs
            }
        }
    }

    buffer.MakePowerOfTwo();

    // create opengl texture from buffer
    m_texture.reset(new Texture);
    m_texture->Init(buffer.BufferWidth(), buffer.BufferHeight(),
                    (unsigned char*)buffer.Buffer(), GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2);

    // create Glyph objects from temp glyph data
    for (std::map<boost::uint32_t, TempGlyphData>::iterator it = temp_glyph_data.begin(); it != temp_glyph_data.end(); ++it)
        m_glyphs[it->first] = Glyph(m_texture, it->second.ul, it->second.lr, it->second.y_offset, it->second.left_b, it->second.adv);

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
        if (FT_Load_Glyph(face, index, FT_LOAD_DEFAULT)) {
            // loading of a glpyh failed so we replace it with
            // the 'Replacement Character' at codepoint 0xFFFD
            FT_UInt tmp_index = FT_Get_Char_Index(face, 0xFFFD);
            if (FT_Load_Glyph(face, tmp_index, FT_LOAD_DEFAULT))
                ThrowBadGlyph("GG::Font::GetGlyphBitmap : Freetype could not load the glyph for character '%1%'", ch);
        }

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

void Font::StoreGlyphImpl(Font::RenderCache& cache, GG::Clr color, const Pt& pt, const Glyph& glyph, int x_top_offset) const {
    cache.coordinates->store(glyph.sub_texture.TexCoords()[0], glyph.sub_texture.TexCoords()[1]);
    cache.vertices->store(pt.x + glyph.left_bearing + x_top_offset, pt.y + glyph.y_offset);
    cache.colors->store(color);

    cache.coordinates->store(glyph.sub_texture.TexCoords()[2], glyph.sub_texture.TexCoords()[1]);
    cache.vertices->store(pt.x + glyph.sub_texture.Width() + glyph.left_bearing + x_top_offset, pt.y + glyph.y_offset);
    cache.colors->store(color);

    cache.coordinates->store(glyph.sub_texture.TexCoords()[2], glyph.sub_texture.TexCoords()[3]);
    cache.vertices->store(pt.x + glyph.sub_texture.Width() + glyph.left_bearing - x_top_offset,
                                        pt.y + glyph.sub_texture.Height() + glyph.y_offset);
    cache.colors->store(color);

    cache.coordinates->store(glyph.sub_texture.TexCoords()[0], glyph.sub_texture.TexCoords()[3]);
    cache.vertices->store(pt.x + glyph.left_bearing - x_top_offset, pt.y + glyph.sub_texture.Height() + glyph.y_offset);
    cache.colors->store(color);
}

void Font::StoreUnderlineImpl(Font::RenderCache& cache, GG::Clr color, const Pt& pt, const Glyph& glyph,
                        Y descent, Y height, Y underline_height, Y underline_offset) const
{
    X x1 = pt.x;
    Y y1(pt.y + height + descent - underline_offset);
    X x2 = x1 + glyph.advance;
    Y y2(y1 + underline_height);

    cache.underline_vertices->store(x1, y1);
    cache.underline_colors->store(color);
    cache.underline_vertices->store(x2, y1);
    cache.underline_colors->store(color);
    cache.underline_vertices->store(x2, y2);
    cache.underline_colors->store(color);
    cache.underline_vertices->store(x1, y2);
    cache.underline_colors->store(color);
}

X Font::StoreGlyph(const Pt& pt, const Glyph& glyph, const Font::RenderState* render_state,
                   Font::RenderCache& cache) const
{
    int italic_top_offset = 0;
    int shadow_offset = 0;

    if (render_state && render_state->use_italics) {
        // Should we enable sub pixel italics offsets?
        italic_top_offset = static_cast<int>(m_italics_offset);
    }
    if (render_state && render_state->use_shadow) {
        shadow_offset = static_cast<int>(m_shadow_offset);
    }

    // render shadows?
    if (shadow_offset > 0) {
        StoreGlyphImpl(cache, GG::CLR_BLACK, pt + GG::Pt(X1, Y0), glyph, italic_top_offset);
        StoreGlyphImpl(cache, GG::CLR_BLACK, pt + GG::Pt(X0, Y1), glyph, italic_top_offset);
        StoreGlyphImpl(cache, GG::CLR_BLACK, pt + GG::Pt(-X1, Y0), glyph, italic_top_offset);
        StoreGlyphImpl(cache, GG::CLR_BLACK, pt + GG::Pt(X0, -Y1), glyph, italic_top_offset);
        if (render_state && render_state->draw_underline) {
            StoreUnderlineImpl(cache, GG::CLR_BLACK, pt + GG::Pt(X0, Y1), glyph, m_descent, m_height, Y(m_underline_height), Y(m_underline_offset));
            StoreUnderlineImpl(cache, GG::CLR_BLACK, pt + GG::Pt(X0, -Y1), glyph, m_descent, m_height, Y(m_underline_height), Y(m_underline_offset));
        }
    }

    // render main text
    StoreGlyphImpl(cache, render_state->CurrentColor(), pt, glyph, italic_top_offset);
    if (render_state && render_state->draw_underline) {
        StoreUnderlineImpl(cache, render_state->CurrentColor(), pt, glyph, m_descent, m_height, Y(m_underline_height), Y(m_underline_offset));
    }

    return glyph.advance;
}

void Font::HandleTag(const boost::shared_ptr<FormattingTag>& tag, double* orig_color,
                     RenderState& render_state) const
{
    if (tag->tag_name == ITALIC_TAG) {
        if (tag->close_tag) {
            if (render_state.use_italics) {
                --render_state.use_italics;
            }
        } else {
            ++render_state.use_italics;
        }
    } else if (tag->tag_name == UNDERLINE_TAG) {
        if (tag->close_tag) {
            if (render_state.draw_underline) {
                --render_state.draw_underline;
            }
        } else {
            ++render_state.draw_underline;
        }
    } else if (tag->tag_name == SHADOW_TAG) {
        if (tag->close_tag) {
            if (render_state.use_shadow) {
                --render_state.use_shadow;
            }
        } else {
            ++render_state.use_shadow;
        }
    } else if (tag->tag_name == RGBA_TAG) {
        if (tag->close_tag) {
            // Popping is ok also for an empty color stack.
            render_state.PopColor();
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
                        0 <= temp_color[3] && temp_color[3] <= 255)
                    {
                        color[0] = temp_color[0];
                        color[1] = temp_color[1];
                        color[2] = temp_color[2];
                        color[3] = temp_color[3];
                        glColor4ubv(color);
                        render_state.PushColor(color[0], color[1], color[2], color[3]);
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
                            0.0 <= color[3] && color[3] <= 1.0)
                        {
                            glColor4dv(color);
                            render_state.PushColor(color[0], color[1], color[2], color[3]);
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
