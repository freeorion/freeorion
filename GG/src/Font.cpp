//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2021 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <cctype>
#if __has_include(<charconv>)
  #include <charconv>
#endif
#include <cmath>
#include <iterator>
#include <numeric>
#include <sstream>
#include <boost/format.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <GG/Base.h>
#include <GG/Font.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/utf8/checked.h>


#define DEBUG_DETERMINELINES 0

namespace GG::detail {
    FTFaceWrapper::~FTFaceWrapper()
    { if (m_face) FT_Done_Face(m_face); }
}

using namespace GG;

namespace {
    constexpr uint32_t WIDE_SPACE = ' ';
    constexpr uint32_t WIDE_NEWLINE = '\n';
    constexpr uint32_t WIDE_CR = '\r';
    constexpr uint32_t WIDE_FF = '\f';
    constexpr uint32_t WIDE_TAB = '\t';

    constexpr std::string_view ITALIC_TAG = "i";
    constexpr std::string_view SHADOW_TAG = "s";
    constexpr std::string_view UNDERLINE_TAG = "u";
    constexpr std::string_view SUPERSCRIPT_TAG = "sup";
    constexpr std::string_view SUBSCRIPT_TAG = "sub";
    constexpr std::string_view RGBA_TAG = "rgba";
    constexpr std::string_view ALIGN_LEFT_TAG = "left";
    constexpr std::string_view ALIGN_CENTER_TAG = "center";
    constexpr std::string_view ALIGN_RIGHT_TAG = "right";
    constexpr std::string_view PRE_TAG = "pre";

    template <typename T>
    [[nodiscard]] constexpr auto abs(T i) noexcept { return (i >= T{0}) ? i : -i; }

    template <typename T>
    constexpr T NextPowerOfTwo(T input)
    {
        if constexpr (T{-1} <= T{0}) // std::is_signed_v doesn't work due to X and Y being in namespace GG, so can't easily specialized std::is_signed in the macro that defines X and Y
            input = abs(input);
        T value{1};
        while (value < input)
            value *= 2;
        return value;
    }
    static_assert(NextPowerOfTwo(-15) == 16);
    static_assert(NextPowerOfTwo(Y{-15}) == Y{16});
    static_assert(NextPowerOfTwo(X0) == X1);
    static_assert(NextPowerOfTwo(2) == 2);
    static_assert(NextPowerOfTwo(3) == 4);
    static_assert(NextPowerOfTwo(4) == 4);


    /** This is used to collect data on the glyphs as they are recorded into buffers,
      * for use in creating Glyph objects at the end of Font's constructor.*/
    struct TempGlyphData
    {
        constexpr TempGlyphData() = default;
        constexpr TempGlyphData(Pt ul_, Pt lr_, int8_t y_ofs, int8_t lb, int8_t a) noexcept :
            ul(ul_), lr(lr_), y_offset(y_ofs), left_b(lb), adv(a) {}
        Pt       ul, lr;        ///< area of glyph subtexture within texture
        int8_t   y_offset = 0;  ///< vertical offset to draw texture (may be negative!)
        int8_t   left_b = 0;    ///< left bearing (see Glyph)
        int8_t   adv = 0;       ///< advance of glyph (see Glyph)
    };

    // utility to less verbosely quiet warnings
    template <typename T>
    [[nodiscard]] constexpr std::size_t to_sz_t(T i) noexcept { return static_cast<std::size_t>(abs(Value(i))); }

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
            m_capacity_width(abs(initial_width)),
            m_capacity_height(abs(initial_height)),
            m_data(to_sz_t(abs(initial_width))*to_sz_t(abs(initial_height)), default_value),
            m_current_width(abs(initial_width)),
            m_current_height(abs(initial_height)),
            m_default_value(default_value)
        {}

        /// Access point \x,\y, expanding the buffer if it does not exist yet
        T& at(X x, Y y)
        {
            EnsureFit(x, y);
            return this->get(x,y);
        }

        /// Access point \x, \y without any checks
        T& get(X x, Y y) noexcept(noexcept(std::declval<std::vector<T>>()[Value(X{})*Value(X{}) + Value(Y{})]))
        { return m_data[Value(m_capacity_width)*Value(y) + Value(x)]; }

        /// Returns the current highest x the user has requested to exist
        X CurrentWidth() const noexcept { return m_current_width; }

        /// Returns the current highest y the user has requested to exist
        Y CurrentHeight() const noexcept { return m_capacity_height; }

        /// Returns the actual width of the storage area allocated so far
        X BufferWidth() const noexcept { return m_capacity_width; }

        /// Returns the actual height of the storage area allocated so far
        Y BufferHeight() const noexcept { return m_capacity_height; }

        /// Return a pointer to the storage buffer where the data is kept
        T* Buffer() noexcept { return &m_data.front(); }

        /// Returns the size of the storage buffer where the data is kept
        auto BufferSize() const noexcept { return m_data.size(); }

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
            X new_width = std::max(m_current_width, abs(x) + 1); // Zero indexed => width = max_x + 1
            Y new_height = std::max(m_current_height, abs(y) + 1); // Also zero indexed
            X new_capacity_width = m_capacity_width;
            Y new_capacity_height = m_capacity_height;
            while (new_width > new_capacity_width)
                new_capacity_width *= 2;
            while (new_height > new_capacity_height)
                new_capacity_height *= 2;

            ResizeCapacity(new_capacity_width, new_capacity_height);
            m_current_width = new_width;
            m_current_height = new_height;
        }

        void ResizeCapacity(X new_capacity_width, Y new_capacity_height)
        {
            new_capacity_width = abs(new_capacity_width);
            new_capacity_height = abs(new_capacity_height);
            // If there really was a change, we need to recreate our storage.
            // This is expensive, but since we double the size every time,
            // the cost of adding data here in some sane order is amortized constant
            if (new_capacity_width != m_capacity_width || new_capacity_height != m_capacity_height) {
                // Create new storage and copy old data. A linear copy won't do, there
                // will be a new mapping from 2d indexes to 1d memory.
                std::vector<T> new_data(to_sz_t(new_capacity_width)*to_sz_t(new_capacity_height), m_default_value);
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
        FTLibraryWrapper()
        {
            if (!m_library && FT_Init_FreeType(&m_library)) // if no library exists and we can't create one...
                throw FailedFTLibraryInit("Unable to initialize FreeType font library object");
        }
        ~FTLibraryWrapper() { FT_Done_FreeType(m_library); }
        FT_Library m_library = nullptr;
    } g_library;

    struct PushSubmatchOntoStackP
    {
        typedef void result_type;
        void operator()(const std::string* str,
                        std::stack<Font::Substring>& tag_stack,
                        bool& ignore_tags,
                        const boost::xpressive::ssub_match& sub) const
        {
            tag_stack.emplace(*str, sub);
            if (tag_stack.top() == PRE_TAG)
                ignore_tags = true;
        }
    };
    const boost::xpressive::function<PushSubmatchOntoStackP>::type PushP = {{}};

    constexpr double ITALICS_SLANT_ANGLE = 12; // degrees
    const double ITALICS_FACTOR = 1.0 / tan((90 - ITALICS_SLANT_ANGLE) * 3.1415926 / 180.0); // factor used to shear glyphs ITALICS_SLANT_ANGLE degrees CW from straight up

    constexpr std::array<std::pair<uint32_t, uint32_t>, 2> PRINTABLE_ASCII_ALPHA_RANGES{{
        {0x41, 0x5B},
        {0x61, 0x7B}}};

    constexpr std::array<std::pair<uint32_t, uint32_t>, 7> PRINTABLE_ASCII_NONALPHA_RANGES{{
        {0x09, 0x0D},
        {0x20, 0x21},
        {0x30, 0x3A},
        {0x21, 0x30},
        {0x3A, 0x41},
        {0x5B, 0x61},
        {0x7B, 0x7F}}};
}

namespace {
    // writes 1-3 chars, starting at to_it, and outputs how many were written
    // written chars represent \a n as decimal digits
    constexpr auto ToChars = [](auto& to_it, uint8_t n) noexcept
    {
        uint8_t hundreds = n / 100;
        uint8_t remainder = n % 100;
        uint8_t tens = remainder / 10;
        uint8_t ones = n % 10;

        (*to_it) = ('0' + hundreds);
        to_it += (hundreds > 0);
        (*to_it) = ('0' + tens);
        to_it += (hundreds > 0 || tens > 0);
        (*to_it) = ('0' + ones);
        ++to_it;

        static_assert(noexcept((*std::declval<decltype(to_it)>()) += ('0' + uint8_t(24))));
        static_assert(noexcept(++std::declval<decltype(to_it)>()));
    };
    constexpr auto one_zero_nine = []() {
        std::array<char, 4> retval = {};
        auto it = retval.begin();
        ToChars(it, 109);
        return retval;
    }();
    static_assert(std::string_view{one_zero_nine.data()} == "109");
    constexpr auto three_zero = []() {
        std::array<char, 4> retval = {};
        auto it = retval.begin();
        ToChars(it, 30);
        return retval;
    }();
    static_assert(std::string_view{three_zero.data()} == "30");

#if defined(__cpp_lib_constexpr_string) && \
    (defined(_MSC_VER) || defined(__cplusplus) && (__cplusplus >= 202002L)) && \
    (!defined(_MSC_VER) || (_MSC_VER >= 1934))
    static_assert([](){
        const std::string str{"a"};
        return str.size() >= 1 &&
            str[str.size()] == 0 &&
            str[0] == 'a' &&
            *(str.data() + 0) == 'a' &&
            *(str.data() + 1) == 0;
        }());
#endif
    static_assert(noexcept(std::declval<const std::string>().data() + 1) &&
                  noexcept(std::declval<const std::string>().cbegin() + 1));

    constexpr auto ClrToChars = [](auto& to_it, const Clr c) noexcept
    {
        ToChars(to_it, c.r);
        *(to_it++) = ' ';
        ToChars(to_it, c.g);
        *(to_it++) = ' ';
        ToChars(to_it, c.b);
        *(to_it++) = ' ';
        ToChars(to_it, c.a);
    };

    constexpr std::size_t buf_sz = 6 + 4*4 + 1;
    constexpr std::array<std::string::value_type, buf_sz> rgba_buffer{"<rgba \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"}; // rest should be nulls without being explicit, but are not considered appropriately initialized in constexpr evaluations in MSVC 2022 in my tests
    static_assert(rgba_buffer[22] == 0);
    static_assert(rgba_buffer[6] == 0);
    static_assert(rgba_buffer[5] == ' ');

    constexpr auto ToRGBATagChars(const Clr c) noexcept
    {
        auto buffer{rgba_buffer};
        auto it = buffer.begin() + 6;
        static_assert(noexcept(*(it++) = ' ') && noexcept(buffer.begin() + 6));
        ClrToChars(it, c);
        *it = '>';
        return buffer;
    }

    constexpr auto chars_from_red{ToRGBATagChars(GG::CLR_RED)};
    static_assert(std::string_view("<rgba 255 0 0 255>") == std::string_view{chars_from_red.data()});

    constexpr auto chars_from_white{ToRGBATagChars(GG::CLR_WHITE)};
    static_assert(std::string_view("<rgba 255 255 255 255>") == std::string_view{chars_from_white.data()});
}


///////////////////////////////////////
// function GG::RgbaTag
///////////////////////////////////////
std::string GG::RgbaTag(Clr c)
{
    auto buffer{ToRGBATagChars(c)};
    return {buffer.data()};
}


///////////////////////////////////////
// TextFormat
///////////////////////////////////////
namespace GG {
GG_FLAGSPEC_IMPL(TextFormat);
}

namespace {
    bool RegisterTextFormats()
    {
        FlagSpec<TextFormat>& spec = FlagSpec<TextFormat>::instance();
        spec.insert(FORMAT_NONE,        "FORMAT_NONE");
        spec.insert(FORMAT_VCENTER,     "FORMAT_VCENTER");
        spec.insert(FORMAT_TOP,         "FORMAT_TOP");
        spec.insert(FORMAT_BOTTOM,      "FORMAT_BOTTOM");
        spec.insert(FORMAT_CENTER,      "FORMAT_CENTER");
        spec.insert(FORMAT_LEFT,        "FORMAT_LEFT");
        spec.insert(FORMAT_RIGHT,       "FORMAT_RIGHT");
        spec.insert(FORMAT_NOWRAP,      "FORMAT_NOWRAP");
        spec.insert(FORMAT_WORDBREAK,   "FORMAT_WORDBREAK");
        spec.insert(FORMAT_LINEWRAP,    "FORMAT_LINEWRAP");
        spec.insert(FORMAT_IGNORETAGS,  "FORMAT_IGNORETAGS");
        return true;
    }
    bool dummy = RegisterTextFormats();
}


///////////////////////////////////////
// class GG::Font::Substring
///////////////////////////////////////
const std::string Font::Substring::EMPTY_STRING{};

bool Font::Substring::operator==(const std::string& rhs) const
{ return size() == rhs.size() && !std::memcmp(str->data() + first, rhs.data(), size()); }

bool Font::Substring::operator==(std::string_view rhs) const
{ return size() == rhs.size() && !std::memcmp(str->data() + first, rhs.data(), size()); }

bool Font::Substring::operator==(const Substring& rhs) const
{ return size() == rhs.size() && !std::memcmp(str->data() + first, rhs.data() + rhs.first, size()); }

///////////////////////////////////////
// Free Functions
///////////////////////////////////////
std::ostream& GG::operator<<(std::ostream& os, Font::Substring substr)
{
    std::ostream_iterator<char> out_it(os);
    std::copy(substr.begin(), substr.end(), out_it);
    return os;
}

CPSize GG::CodePointIndexOf(std::size_t line, CPSize index,
                            const std::vector<Font::LineData>& line_data)
{
    CPSize retval(CP0);
    if (line_data.size() <= line) {
        auto it = line_data.rbegin();
        auto end_it = line_data.rend();
        while (it != end_it) {
            if (!it->char_data.empty()) {
                retval = it->char_data.back().code_point_index + CP1;
                break;
            }
            ++it;
        }
    } else if (Value(index) < line_data[line].char_data.size()) {
        retval = line_data[line].char_data[Value(index)].code_point_index;
    } else {
        auto it = line_data.rbegin() + (line_data.size() - 1 - line);
        auto end_it = line_data.rend();
        while (it != end_it) {
            if (!it->char_data.empty()) {
                retval = it->char_data.back().code_point_index + CP1;
                break;
            }
            ++it;
        }
    }
    return retval;
}

StrSize GG::StringIndexOf(std::size_t line, CPSize index,
                          const std::vector<Font::LineData>& line_data)
{
    StrSize retval(S0);
    if (line_data.size() <= line) {
        auto it = line_data.rbegin();
        auto end_it = line_data.rend();
        while (it != end_it) {
            if (!it->char_data.empty()) {
                retval = it->char_data.back().string_index + it->char_data.back().string_size;
                break;
            }
            ++it;
        }
    } else if (Value(index) < line_data[line].char_data.size()) {
        retval = line_data[line].char_data[Value(index)].string_index;
    } else {
        auto it = line_data.rbegin() + (line_data.size() - 1 - line);
        auto end_it = line_data.rend();
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

std::pair<std::size_t, CPSize> GG::LinePositionOf(
    CPSize index, const std::vector<Font::LineData>& line_data)
{
    std::pair<std::size_t, CPSize> retval(std::numeric_limits<std::size_t>::max(),
                                          INVALID_CP_SIZE);
    for (std::size_t i = 0; i < line_data.size(); ++i) {
        const auto& char_data = line_data[i].char_data;
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

namespace {
    // These are the types found by the regular expression: XML open/close tags, text and
    // whitespace.  Each type will correspond to a type of TextElement.
    constexpr std::size_t full_regex_tag_idx = 0;
    constexpr std::size_t tag_name_tag_idx = 1;
    constexpr std::size_t open_bracket_tag_idx = 2;
    constexpr std::size_t close_bracket_tag_idx = 3;
    constexpr std::size_t whitespace_tag_idx = 4;
    constexpr std::size_t text_tag_idx = 5;


    namespace xpr = boost::xpressive;

    /** CompiledRegex maintains a compiled boost::xpressive regular
        expression that includes a tag stack which can be cleared and
        provided to callers without the overhead of recompiling the
        regular expression.*/
    template <typename TagHandlerT>
    class CompiledRegex {
    public:
        CompiledRegex(const TagHandlerT& tag_handler, bool strip_unpaired_tags) :
            m_tag_handler(tag_handler)
        {
            // Synonyms for s1 thru s5 sub matches
            xpr::mark_tag tag_name_tag(tag_name_tag_idx);
            xpr::mark_tag open_bracket_tag(open_bracket_tag_idx);
            xpr::mark_tag close_bracket_tag(close_bracket_tag_idx);
            xpr::mark_tag whitespace_tag(whitespace_tag_idx);
            xpr::mark_tag text_tag(text_tag_idx);

            using boost::placeholders::_1;

            // The comments before each regex are intended to clarify the mapping from xpressive
            // notation to the more typical regex notation.  If you read xpressive or don't read
            // regex then ignore them.

            // -+ 'non-greedy',   ~ 'not',   set[|] 'set',    _s 'space' = 'anything but space or <'
            static const xpr::sregex TAG_PARAM =
                -+~xpr::set[xpr::_s | '<'];

            //+_w one or more greed word chars,  () group no capture,  [] semantic operation
            const xpr::sregex OPEN_TAG_NAME =
                (+xpr::_w)[xpr::check(boost::bind(&CompiledRegex::MatchesKnownTag, this, _1))];

            // (+_w) one or more greedy word check matches stack
            const xpr::sregex CLOSE_TAG_NAME =
                (+xpr::_w)[xpr::check(boost::bind(&CompiledRegex::MatchesTopOfStack, this, _1))];

            // *blank  'zero or more greedy whitespace',   >> 'followed by',    _ln 'newline',
            // (set = 'a', 'b') is '[ab]',    +blank 'one or more greedy blank'
            static const xpr::sregex WHITESPACE =
                (*xpr::blank >> (xpr::_ln | (xpr::set = '\n', '\r', '\f'))) | +xpr::blank;

            // < followed by not space or <   or one or more not space or <
            static const xpr::sregex TEXT =
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

        xpr::sregex& BindRegexToText(const std::string& new_text, bool ignore_tags)
            noexcept(noexcept(std::stack<Font::Substring>{}))
        {
            if (!m_tag_stack.empty()) {
                std::stack<Font::Substring> empty_stack;
                std::swap(m_tag_stack, empty_stack);
            }
            m_text = &new_text;
            m_ignore_tags = ignore_tags;
            return m_EVERYTHING;
        }

    private:
        bool MatchesKnownTag(const boost::xpressive::ssub_match& sub)
        { return !m_ignore_tags && m_tag_handler.IsKnown(sub.str()); }

        bool MatchesTopOfStack(const boost::xpressive::ssub_match& sub) noexcept {
            bool retval = !m_tag_stack.empty() && m_tag_stack.top() == sub;
            if (retval) {
                m_tag_stack.pop();
                if (m_tag_stack.empty() || m_tag_stack.top() != PRE_TAG)
                    m_ignore_tags = false;
            }
            return retval;
        }

        const std::string* m_text = nullptr;
        const TagHandlerT& m_tag_handler;
        bool m_ignore_tags = false;

        // m_tag_stack is used to track XML opening/closing tags.
        std::stack<Font::Substring> m_tag_stack;

        // The combined regular expression.
        xpr::sregex m_EVERYTHING;
    };

    /** TagHandler stores a set of all known tags and provides pre-compiled regexs for those tags.

     Known tags are tags that will be parsed into TextElement OPEN_TAG or CLOSE_TAG. */
    class TagHandler {
    public:
        TagHandler() :
            m_regex_w_tags(*this, false),
            m_regex_w_tags_skipping_unmatched(*this, true)
        {}

        /** Add a tag to the set of known tags.*/
        void Insert(std::vector<std::string_view> tags)
        {
            std::copy_if(tags.begin(), tags.end(), std::back_inserter(m_custom_tags),
                         [this](const auto tag) { return !IsKnown(tag); });
        }

        /** Remove a tag from the set of known tags.*/
        void Erase(std::string_view tag)
        {
            const auto it = std::find(m_custom_tags.begin(), m_custom_tags.end(), tag);
            if (it != m_custom_tags.end())
                m_custom_tags.erase(it);
        }

        /** Remove all tags from the set of known tags.*/
        void Clear() noexcept
        { m_custom_tags.clear(); }

        bool IsKnown(std::string_view tag) const
        {
            const auto matches_tag = [tag](const auto sv) noexcept{ return sv == tag; };
            return std::any_of(m_default_tags.begin(), m_default_tags.end(), matches_tag)
                || std::any_of(m_custom_tags.begin(), m_custom_tags.end(), matches_tag);
        }

        // Return a regex bound to \p text using the currently known
        // tags.  If required \p ignore_tags and/or \p strip_unpaired_tags.
        xpr::sregex& Regex(const std::string& text, bool ignore_tags, bool strip_unpaired_tags = false)
        {
            if (!strip_unpaired_tags)
                return m_regex_w_tags.BindRegexToText(text, ignore_tags);
            else
                return m_regex_w_tags_skipping_unmatched.BindRegexToText(text, ignore_tags);
        }

    private:
        // set of tags known to the handler
        static constexpr std::array<std::string_view, 10> m_default_tags{
            {ITALIC_TAG, SHADOW_TAG, UNDERLINE_TAG, SUPERSCRIPT_TAG, SUBSCRIPT_TAG,
             RGBA_TAG, ALIGN_LEFT_TAG, ALIGN_CENTER_TAG, ALIGN_RIGHT_TAG, PRE_TAG}};

        std::vector<std::string_view> m_custom_tags;

        // Compiled regular expression including tag stack
        CompiledRegex<TagHandler> m_regex_w_tags;

        // Compiled regular expression using tags but skipping unmatched tags.
        CompiledRegex<TagHandler> m_regex_w_tags_skipping_unmatched;
    };

    TagHandler tag_handler{};
}

///////////////////////////////////////
// class GG::Font::TextElement
///////////////////////////////////////
X Font::TextElement::Width() const
{
    if (cached_width == -X1)
        cached_width = std::accumulate(widths.begin(), widths.end(), X0);
    return cached_width;
}


///////////////////////////////////////
// class GG::Font::TextAndElementsAssembler
///////////////////////////////////////
class Font::TextAndElementsAssembler::Impl
{
public:
    explicit Impl(const Font& font) :
        m_font(font)
    {}
    Impl(const Font& font, std::size_t text_capacity, std::size_t elements_capacity) :
        m_font(font)
    {
        m_text.reserve(text_capacity);
        m_text_elements.reserve(elements_capacity);
    }

    /** Return the constructed text.*/
    const auto& Text() const noexcept
    { return m_text; }

    std::pair<std::string, std::vector<TextElement>> Extract()
    {
        if (!m_are_widths_calculated)
            m_font.FillTemplatedText(m_text, m_text_elements, m_text_elements.begin());
        return std::pair(std::move(m_text), std::move(m_text_elements));
    }

    /** Return the constructed TextElements.*/
    const auto& Elements()
    {
        if (!m_are_widths_calculated)
            m_font.FillTemplatedText(m_text, m_text_elements, m_text_elements.begin());
        return m_text_elements;
    }

    /** Add an open tag iff it exists as a recognized tag.*/
    void AddOpenTag(std::string_view tag)
    {
        if (!tag_handler.IsKnown(tag))
            return;

        m_are_widths_calculated = false;

        // Create open tag like "<tag>" with no parameters
        const auto tag_begin = m_text.size();
        const auto tag_name_begin = m_text.append("<").size();
        const auto tag_name_end = m_text.append(tag).size();
        const auto tag_end = m_text.append(">").size();

        Substring text{m_text, tag_begin, tag_end};
        Substring tag_name{m_text, tag_name_begin, tag_name_end};
        m_text_elements.emplace_back(text, tag_name, Font::TextElement::TextElementType::OPEN_TAG);
    }

    /** Add an open tag iff it exists as a recognized tag.*/
    void AddOpenTag(std::string_view tag, const std::vector<std::string>& params)
    {
        if (!tag_handler.IsKnown(tag))
            return;

        m_are_widths_calculated = false;

        const auto tag_begin = m_text.size();

        // Create the opening part of an open tag, like "<tag"
        const auto tag_name_begin = m_text.append("<").size();
        const auto tag_name_end = m_text.append(tag).size();
        Substring tag_name{m_text, tag_name_begin, tag_name_end};

        std::vector<Substring> pass_params;
        pass_params.reserve(params.size());
        // add params, like: "... param1 param2 ..."
        for (const auto& param : params) {
            m_text.append(" ");
            const auto param_begin = m_text.size();
            const auto param_end = m_text.append(param).size();
            pass_params.emplace_back(m_text, param_begin, param_end);
        }

        // Create the close part of an open tag to complete the tag, like ">"
        auto tag_end = m_text.append(">").size();
        Substring text{m_text, tag_begin, tag_end};

        m_text_elements.emplace_back(text, tag_name, std::move(pass_params), TextElement::TextElementType::OPEN_TAG);
    }

    /** Add a close tag iff it exists as a recognized tag.*/
    void AddCloseTag(std::string_view tag)
    {
        if (!tag_handler.IsKnown(tag))
            return;

        m_are_widths_calculated = false;

        // Create a close tag that looks like "</tag>"
        const auto tag_begin = m_text.size();
        const auto tag_name_begin = m_text.append("</").size();
        const auto tag_name_end = m_text.append(tag).size();
        const auto tag_end = m_text.append(">").size();

        Substring text{m_text, tag_begin, tag_end};
        Substring tag_name{m_text, tag_name_begin, tag_name_end};
        m_text_elements.emplace_back(text, tag_name, TextElement::TextElementType::CLOSE_TAG);
    }

    /** Add a text element.  Any whitespace in this text element will be non-breaking.*/
    template <typename S>
    void AddText(S&& text)
    {
        m_are_widths_calculated = false;

        const auto begin = m_text.size();
        const auto end = m_text.append(text).size();
        m_text_elements.emplace_back(Substring{m_text, begin, end});
    }

    /** Add a white space element.*/
    void AddWhitespace(std::string_view whitespace)
    {
        m_are_widths_calculated = false;

        auto begin = m_text.size();
        auto end = m_text.append(whitespace).size();

        m_text_elements.emplace_back(Substring(m_text, begin, end),
                                     Font::TextElement::TextElementType::WHITESPACE);
    }

    /** Add a newline element.*/
    void AddNewline()
    {
        m_are_widths_calculated = false;
        m_text_elements.emplace_back(Font::TextElement::TextElementType::NEWLINE);
    }

    /** Add open color tag.*/
    void AddOpenTag(Clr color)
    {
        std::vector<std::string> params = { std::to_string(color.r),
                                            std::to_string(color.g),
                                            std::to_string(color.b),
                                            std::to_string(color.a) };

        AddOpenTag("rgba", params);
    }

private:
    const Font& m_font;
    std::string m_text;
    std::vector<TextElement> m_text_elements;
    bool m_are_widths_calculated = false;
};


Font::TextAndElementsAssembler::TextAndElementsAssembler(const Font& font) :
    m_impl(std::make_unique<Impl>(font))
{}

Font::TextAndElementsAssembler::TextAndElementsAssembler(const Font& font, std::size_t text_capacity,
                                                         std::size_t elements_capacity) :
    m_impl(std::make_unique<Impl>(font, text_capacity, elements_capacity))
{}

// Required because Impl is defined here
Font::TextAndElementsAssembler::~TextAndElementsAssembler() = default;

const std::string& Font::TextAndElementsAssembler::Text() const noexcept
{ return m_impl->Text(); }

const std::vector<Font::TextElement>& Font::TextAndElementsAssembler::Elements() const
{ return m_impl->Elements(); }

std::pair<std::string, std::vector<Font::TextElement>> Font::TextAndElementsAssembler::Extract()
{ return m_impl->Extract(); }

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddOpenTag(std::string_view tag)
{
    m_impl->AddOpenTag(tag);
    return *this;
}

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddOpenTag(
    std::string_view tag, const std::vector<std::string>& params)
{
    m_impl->AddOpenTag(tag, params);
    return *this;
}

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddCloseTag(std::string_view tag)
{
    m_impl->AddCloseTag(tag);
    return *this;
}

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddText(std::string_view text)
{
    m_impl->AddText(text);
    return *this;
}

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddText(std::string&& text)
{
    m_impl->AddText(std::move(text));
    return *this;
}

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddWhitespace(std::string_view whitespace)
{
    m_impl->AddWhitespace(whitespace);
    return *this;
}

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddNewline()
{
    m_impl->AddNewline();
    return *this;
}

Font::TextAndElementsAssembler& Font::TextAndElementsAssembler::AddOpenTag(Clr color)
{
    m_impl->AddOpenTag(color);
    return *this;
}


///////////////////////////////////////
// class GG::Font::RenderState
///////////////////////////////////////
namespace {
    std::array<GLubyte, 4> GetCurrentByteColor() {
        std::array<GLfloat, 4> current_float{};
        glGetFloatv(GL_CURRENT_COLOR, current_float.data());
        return std::array<GLubyte, 4>{static_cast<GLubyte>(current_float[0]*255),
                                      static_cast<GLubyte>(current_float[1]*255),
                                      static_cast<GLubyte>(current_float[2]*255),
                                      static_cast<GLubyte>(current_float[3]*255)};
    }
}

Font::RenderState::RenderState()
{
    // Initialize the color stack with the current color
    auto clr = GetCurrentByteColor();
    PushColor(clr[0], clr[1], clr[2], clr[3]);
}

Font::RenderState::RenderState(Clr color)
{ PushColor(color.r, color.g, color.b, color.a); }

void Font::RenderState::PushColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    // The same color may end up being stored multiple times, but the cost of
    // deduplication is greater than the cost of just letting it be so.
    color_index_stack.push(static_cast<uint8_t>(used_colors.size()));
    used_colors.emplace_back(r, g, b, a);
}

void Font::RenderState::PushColor(Clr clr)
{ PushColor(clr.r, clr.g, clr.b, clr.a); }

void Font::RenderState::PopColor()
{
    // Never remove the initial color from the stack
    if (color_index_stack.size() > 1)
        color_index_stack.pop();
}

Clr Font::RenderState::CurrentColor() const
{ return used_colors[CurrentIndex()]; }


///////////////////////////////////////
// class GG::Font::LineData::CharData
///////////////////////////////////////
Font::LineData::CharData::CharData(X extent_, StrSize str_index, StrSize str_size, CPSize cp_index,
                                   const std::vector<TextElement>& tags_) :
    extent(extent_),
    string_index(str_index),
    string_size(str_size),
    code_point_index(cp_index)
{
    tags.reserve(tags_.size());
    for (auto& tag : tags_)
        if (tag.IsTag())
            tags.push_back(tag);
}


///////////////////////////////////////
// struct GG::Font::Glyph
///////////////////////////////////////
Font::Glyph::Glyph(std::shared_ptr<Texture> texture, Pt ul, Pt lr, int8_t y_ofs, int8_t lb, int8_t adv) :
    sub_texture(std::move(texture), ul.x, ul.y, lr.x, lr.y),
    y_offset(y_ofs),
    left_bearing(lb),
    advance(adv),
    width(std::min<int8_t>(std::numeric_limits<int8_t>::max(), Value(ul.x - lr.x)))
{}

///////////////////////////////////////
// class GG::Font
///////////////////////////////////////
namespace {
    Font::RenderCache shared_cache{};
}

Font::Font(std::string font_filename, unsigned int pts) :
    m_font_filename(std::move(font_filename)),
    m_pt_sz(pts)
{
    if (!m_font_filename.empty()) {
        detail::FTFaceWrapper wrapper;
        FT_Error error = GetFace(wrapper.m_face);
        CheckFace(wrapper.m_face, error);
        Init(wrapper.m_face);
    }
}

Font::Font(std::string font_filename, unsigned int pts,
           const std::vector<uint8_t>& file_contents) :
    m_font_filename(std::move(font_filename)),
    m_pt_sz(pts)
{
    assert(!file_contents.empty());
    detail::FTFaceWrapper wrapper;
    FT_Error error = GetFace(file_contents, wrapper.m_face);
    CheckFace(wrapper.m_face, error);
    Init(wrapper.m_face);
}

X Font::RenderText(Pt pt, const std::string_view text, const RenderState& render_state) const
{
    const X orig_x = pt.x;

    glBindTexture(GL_TEXTURE_2D, m_texture->OpenGLId());

    shared_cache.clear();

    for (auto text_it = text.begin(); text_it != text.end();) {
        const uint32_t c = utf8::next(text_it, text.end());
        const auto it = m_glyphs.find(c);
        if (it == m_glyphs.end())
            pt.x += m_space_width; // move forward by the extent of the character when a whitespace or unprintable glyph is requested
        else
            pt.x += StoreGlyph(pt, it->second, render_state, shared_cache);
    }

    shared_cache.vertices.createServerBuffer();
    shared_cache.coordinates.createServerBuffer();
    shared_cache.colors.createServerBuffer();
    RenderCachedText(shared_cache);

    return pt.x - orig_x;
}

void Font::RenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                      const std::vector<LineData>& line_data, RenderState& render_state) const
{
    RenderText(ul, lr, text, format, line_data, render_state, 0, CP0, line_data.size(),
               line_data.empty() ? CP0 : CPSize(line_data.back().char_data.size()));
}

void Font::RenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                      const std::vector<LineData>& line_data, RenderState& render_state,
                      std::size_t begin_line, CPSize begin_char,
                      std::size_t end_line, CPSize end_char) const
{
    PreRenderText(ul, lr, text, format, line_data, render_state,
                  begin_line, begin_char, end_line, end_char, shared_cache);
    RenderCachedText(shared_cache);
}

void Font::PreRenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                         RenderCache& cache, const std::vector<LineData>& line_data,
                         RenderState& render_state) const
{
    PreRenderText(ul, lr, text, format, line_data, render_state, 0, CP0, line_data.size(),
                  line_data.empty() ? CP0 : CPSize(line_data.back().char_data.size()), cache);
}

namespace {
    constexpr X LineOriginX(const X left, const X right, const X line_width, const Alignment align) noexcept
    {
        const X middle = left + (right - left)/2;
        return (align & ALIGN_LEFT) ? left :
            (align & ALIGN_RIGHT) ? (right - line_width) :
            (align & ALIGN_CENTER) ? (middle - line_width/2) :
            left;
    }

    constexpr Y LineOriginY(const Y top, const Y bottom, const Flags<TextFormat> format,
                            const Y font_lineskip, const Y font_height,
                            std::size_t end_line, std::size_t begin_line) noexcept
    {
        const int num_lines_less_1 = static_cast<int>(end_line) - static_cast<int>(begin_line) - 1;
        const Y rendered_height = font_height + num_lines_less_1*font_lineskip;
        const Y middle = top + (bottom - top)/2;

        return (format & FORMAT_TOP) ? top :
            (format & FORMAT_BOTTOM) ? (bottom - rendered_height) :
            (format & FORMAT_VCENTER) ? (middle - rendered_height/2) :
            top;
    }

    constexpr Y LinePosY(const Y origin, const std::size_t line_num,
                         const std::size_t begin_line, const Y lineskip) noexcept
    { return origin + (static_cast<int>(line_num) - static_cast<int>(begin_line)) * lineskip; }
}

void Font::PreRenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                         const std::vector<LineData>& line_data, RenderState& render_state,
                         std::size_t begin_line, CPSize begin_char,
                         std::size_t end_line, CPSize end_char,
                         RenderCache& cache) const
{
    const Y y_origin = LineOriginY(ul.y, lr.y, format, m_lineskip, m_height, end_line, begin_line);

    std::size_t glyph_count = std::transform_reduce(std::next(line_data.begin(), begin_line),
                                                    std::next(line_data.begin(), end_line),
                                                    0, std::plus{},
                                                    [](const auto& line) { return line.char_data.size(); });
    cache.clear();

    //std::cout << "glyphs: " << glyph_count << ": lines " << begin_line << " to " << end_line << ": ";
    //for (std::size_t i = begin_line; i < end_line; ++i) {
    //    const LineData& line = line_data[i];
    //    for (auto& c : line.char_data)
    //        std::cout << static_cast<char>(c.code_point_index);
    //}
    //std::cout << std::endl;

    cache.coordinates.reserve(glyph_count*4);
    cache.vertices.reserve(glyph_count*4);
    cache.colors.reserve(glyph_count*4);

    const auto get_next_char = [&text, string_end_it = text.end()](std::size_t string_index) -> uint32_t {
        const auto text_next_it = std::next(text.begin(), string_index);
        try {
            const uint32_t c = utf8::peek_next(text_next_it, string_end_it);
            assert((text[string_index] == '\n') == (c == WIDE_NEWLINE));
            return c;
        } catch (...) {
            return WIDE_NEWLINE;
        }
    };


    for (std::size_t i = begin_line; i < end_line; ++i) {
        const LineData& line = line_data[i];

        const X x_origin = LineOriginX(ul.x, lr.x, line.Width(), line.justification);
        X x = x_origin;

        const Y y = LinePosY(y_origin, i, begin_line, m_lineskip);


        const CPSize start = (i != begin_line) ?
            CP0 : std::max(CP0, std::min(begin_char, CPSize(line.char_data.size() - 1)));
        const CPSize end = (i != end_line - 1) ? CPSize(line.char_data.size()) :
            std::max(CP0, std::min(end_char, CPSize(line.char_data.size())));


        for (CPSize j = start; j < end; ++j) {
            const auto& char_data = line.char_data[Value(j)];
            for (const auto& tag : char_data.tags)
                HandleTag(tag, render_state);
            const uint32_t c = get_next_char(Value(char_data.string_index));

            if (c == WIDE_NEWLINE)
                continue;
            const auto it = m_glyphs.find(c);
            if (it == m_glyphs.end())
                x = x_origin + char_data.extent; // move forward by the extent of the character when a whitespace or unprintable glyph is requested
            else
                x += StoreGlyph(Pt(x, y), it->second, render_state, cache);

        }
    }

    cache.vertices.createServerBuffer();
    cache.coordinates.createServerBuffer();
    cache.colors.createServerBuffer();
}

void Font::RenderCachedText(RenderCache& cache) const
{
    glBindTexture(GL_TEXTURE_2D, m_texture->OpenGLId());

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    cache.vertices.activate();
    cache.coordinates.activate();
    cache.colors.activate();
    glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(cache.vertices.size()));

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if (!cache.underline_vertices.empty()) {
        cache.underline_vertices.activate();
        cache.underline_colors.activate();
        glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(cache.underline_vertices.size()));
    }

    glPopClientAttrib();
}

void Font::ProcessTagsBefore(const std::vector<LineData>& line_data, RenderState& render_state,
                             std::size_t begin_line, CPSize begin_char) const
{
    if (line_data.empty())
        return;

    for (std::size_t i = 0; i <= begin_line; ++i) {
        const LineData& line = line_data[i];
        for (CPSize j = CP0;
             j < ((i == begin_line) ? begin_char : CPSize(line.char_data.size()));
             ++j)
        {
            for (auto& tag : line.char_data[Value(j)].tags) {
                HandleTag(tag, render_state);
            }
        }
    }
}

std::string Font::StripTags(std::string_view text, bool strip_unpaired_tags)
{
    using namespace boost::xpressive;
    std::string text_str{text}; // temporary until tag_handler.Regex returns a cregex
    auto& regex = tag_handler.Regex(text_str, false, strip_unpaired_tags);

    std::string retval;
    retval.reserve(text.size());

    // scan through matched markup and text, saving only the non-tag-text
    sregex_iterator it(text_str.begin(), text_str.end(), regex);
    const sregex_iterator end_it;
    for (; it != end_it; ++it) {
        auto& text_match = (*it)[text_tag_idx];
        if (text_match.matched) {
            retval.append(text_match.first, text_match.second);

        } else {
            auto& whitespace_match = (*it)[whitespace_tag_idx];
            if (whitespace_match.matched)
                retval.append(whitespace_match.first, whitespace_match.second);
        }
    }

    return retval;
}

Pt Font::TextExtent(const std::vector<LineData>& line_data) const noexcept
{
    X x = X0;
    for (const LineData& line : line_data)
        x = std::max(x, line.Width());

    const auto ld_size = static_cast<int>(line_data.size());
    const bool is_empty = line_data.empty() || (ld_size == 1 && line_data.front().Empty());
    Y y = is_empty ? Y0 : ((ld_size - 1) * m_lineskip + m_height);
    return {x, y};
}

void Font::RegisterKnownTags(std::vector<std::string_view> tags)
{ tag_handler.Insert(std::move(tags)); }

void Font::RemoveKnownTag(std::string_view tag)
{ tag_handler.Erase(tag); }

void Font::ClearKnownTags()
{ tag_handler.Clear(); }

void Font::ThrowBadGlyph(const std::string& format_str, uint32_t c)
{
    boost::format format(isprint(c) ? "%c" : "U+%x");
    throw BadGlyph(boost::io::str(boost::format(format_str) % boost::io::str(format % c)));
}

namespace DebugOutput {
    void PrintParseResults(const std::vector<Font::TextElement>& text_elements) {
        std::cout << "results of parse:\n";
        for (const auto& elem : text_elements) {
            if (elem.IsTag()) {
                std::cout << "FormattingTag\n    text=\"" << elem.text << "\" (@ "
                          << static_cast<const void*>(elem.text.data()) << ")\n    widths=";
                for (const auto width : elem.widths)
                    std::cout << width << " ";
                std::cout << "\n    whitespace=" << elem.IsWhiteSpace() << "\n    newline="
                          << elem.IsNewline() << "\n    params=\n";
                for (const auto& param : elem.params)
                    std::cout << "        \"" << param << "\"\n";
                std::cout << "    tag_name=\"" << elem.tag_name << "\"\n    close_tag="
                          << elem.IsCloseTag() << "\n";

            } else {
                std::cout << "TextElement\n    text=\"" << elem.text << "\" (@ "
                          << static_cast<const void*>(elem.text.data()) << ")\n    widths=";
                for (const auto width : elem.widths)
                    std::cout << width << " ";
                std::cout << "\n    whitespace=" << elem.IsWhiteSpace() << "\n    newline=" << elem.IsNewline() << "\n";
            }
            std::cout << "    string_size=" << Value(elem.StringSize()) << "\n";
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    void PrintLineBreakdown(const std::string& text, const Flags<TextFormat> format,
                            const X box_width, const std::vector<Font::LineData>& line_data)
    {
        std::cout << "Font::DetermineLines(text=\"" << text << "\" (@ "
                  << static_cast<const void*>(text.c_str()) << ") format="
                  << format << " box_width=" << Value(box_width) << ")" << std::endl;

        std::cout << "Line breakdown:\n";
        for (std::size_t i = 0; i < line_data.size(); ++i) {
            auto& char_data = line_data[i].char_data;

            std::cout << "Line " << i << ":\n    extents=";
            for (const auto& character : char_data)
                std::cout << Value(character.extent) << " ";
            std::cout << "\n    string indices=";
            for (const auto& character : char_data)
                std::cout << Value(character.string_index) << " ";
            std::cout << "\n    code point indices=";
            for (const auto& character : char_data)
                std::cout << Value(character.code_point_index) << " ";
            std::cout << "\n    chars on line: \"";
            for (const auto& character : char_data)
                std::cout << text[Value(character.string_index)];
            std::cout << "\"\n";

            for (std::size_t j = 0; j < char_data.size(); ++j) {
                for (auto& tag_elem : char_data[j].tags) {
                    std::cout << "FormattingTag @" << j << "\n    text=\"" << tag_elem.text << "\"\n    widths=";
                    for (const auto width : tag_elem.widths)
                        std::cout << width << " ";
                    std::cout << "\n    whitespace=" << tag_elem.IsWhiteSpace()
                              << "\n    newline=" << tag_elem.IsNewline() << "\n    params=\n";
                    for (const auto& param : tag_elem.params)
                        std::cout << "        \"" << param << "\"\n";
                    std::cout << "    tag_name=\"" << tag_elem.tag_name << "\"\n    close_tag="
                              << tag_elem.IsCloseTag() << "\n";
                }
            }
            std::cout << "    justification=" << line_data[i].justification << "\n" << std::endl;
        }
    }
}

std::vector<Font::TextElement>
Font::ExpensiveParseFromTextToTextElements(const std::string& text, const Flags<TextFormat> format) const
{
    std::vector<TextElement> text_elements;

    using namespace boost::xpressive;

    if (text.empty())
        return text_elements;

    const bool ignore_tags = format & FORMAT_IGNORETAGS;

    // Fetch and use the regular expression from the TagHandler which parses all the known XML tags.
    const sregex& regex = tag_handler.Regex(text, ignore_tags);
    sregex_iterator it(text.begin(), text.end(), regex);

    const sregex_iterator end_it;
    while (it != end_it)
    {
        // Consolidate adjacent blocks of text.  If adjacent found substrings are all text, merge
        // them into a single Substring.
        bool need_increment = true;
        Substring combined_text;
        sub_match<std::string::const_iterator> const* text_match;
        while (it != end_it
               && (text_match = &(*it)[text_tag_idx])
               && text_match->matched)
        {
            need_increment = false;
            if (combined_text.empty())
                combined_text = Substring(text, *text_match);
            else
                combined_text += *text_match;
            ++it;
        }

        // If the element is not a text element then it must be an open tag, a close tag or whitespace.
        if (combined_text.empty()) {
            const auto& it_elem = *it;

            if (it_elem[open_bracket_tag_idx].matched) {
                // Open XML tag.
                Substring text_substr{text, it_elem[full_regex_tag_idx]};
                Substring tag_name_substr{text, it_elem[tag_name_tag_idx]};

                // Check open tags for submatches which are parameters.  For example, a color tag
                // might have RGB parameters.
                const auto& nested_results{it_elem.nested_results()};
                std::vector<Substring> params;
                if (1 < nested_results.size()) {
                    params.reserve(nested_results.size() - 1);
                    for (auto nested_it = std::next(nested_results.begin());
                         nested_it != nested_results.end(); ++nested_it)
                    {
                        const auto& nested_elem = *nested_it;
                        params.emplace_back(text, nested_elem[full_regex_tag_idx]);
                    }
                }

                text_elements.emplace_back(text_substr, tag_name_substr, std::move(params),
                                           Font::TextElement::TextElementType::OPEN_TAG);

            } else if (it_elem[close_bracket_tag_idx].matched) {
                // Close XML tag
                Substring text_substr{text, it_elem[full_regex_tag_idx]};
                Substring tag_substr{text, it_elem[tag_name_tag_idx]};
                text_elements.emplace_back(text_substr, tag_substr,
                                           Font::TextElement::TextElementType::CLOSE_TAG);

            } else if (it_elem[whitespace_tag_idx].matched) {
                // Whitespace element
                Substring text_substr{text, it_elem[whitespace_tag_idx]};
                const auto& ws_elem = text_elements.emplace_back(
                    text_substr, Font::TextElement::TextElementType::WHITESPACE);
                const char last_char = ws_elem.text.empty() ? static_cast<char>(0) : *std::prev(ws_elem.text.end());

                // If the last character of a whitespace element is a line ending then create a
                // newline TextElement.
                if (last_char == '\n' || last_char == '\f' || last_char == '\r')
                    text_elements.emplace_back(Font::TextElement::TextElementType::NEWLINE);
            }

        // Basic text element.
        } else {
            text_elements.emplace_back(combined_text);
        }

        if (need_increment)
            ++it;
    }

    // fill in the widths of code points in each TextElement
    FillTemplatedText(text, text_elements, text_elements.begin());

#if DEBUG_DETERMINELINES
    DebugOutput::PrintParseResults(text_elements);
#endif
    return text_elements;
}


void Font::FillTemplatedText(const std::string& text, std::vector<Font::TextElement>& text_elements,
                             std::vector<Font::TextElement>::iterator start) const
{
    // For each TextElement in text_elements starting from start.
    for (auto te_it = start; te_it != text_elements.end(); ++te_it) {
        auto& elem = *te_it;

        // For each character in the TextElement.
        auto text_it = elem.text.begin();
        const auto end_it = elem.text.end();
        while (text_it != end_it) {
            // Find and set the width of the character glyph.
            elem.widths.push_back(0);
            uint32_t c = utf8::next(text_it, end_it);
            if (c != WIDE_NEWLINE) {
                auto it = m_glyphs.find(c);
                // use a space when an unrendered glyph is requested (the
                // space chararacter is always renderable)
                elem.widths.back() = (it != m_glyphs.end()) ? it->second.advance : m_space_width;
            }
        }
    }
}

void Font::ChangeTemplatedText(std::string& text, std::vector<Font::TextElement>& text_elements,
                               const std::string& new_text, std::size_t targ_offset) const
{
    if (targ_offset >= text_elements.size())
        return;

    if (new_text.empty())
        return;

    int change_of_len = 0;

    // Find the target text element.
    std::size_t curr_offset = 0;
    auto te_it = text_elements.begin();
    while (te_it != text_elements.end()) {
        if (te_it->Type() == TextElement::TextElementType::TEXT) {
            // Change the target text element
            if (targ_offset == curr_offset) {
                // Change text
                auto ii_sub_begin = static_cast<std::size_t>(te_it->text.begin() - text.begin());
                auto sub_len = static_cast<std::size_t>(te_it->text.end() - te_it->text.begin());
                text.erase(ii_sub_begin, sub_len);
                text.insert(ii_sub_begin, new_text);

                change_of_len = static_cast<decltype(change_of_len)>(new_text.size() - sub_len);
                te_it->text = Substring(text, ii_sub_begin, ii_sub_begin + new_text.size());
                break;
            }
            ++curr_offset;
        }
        ++te_it;
    }

    if (te_it == text_elements.end())
        return;

    auto start_of_reflow = te_it;

    if (change_of_len != 0) {
        ++te_it;
        // Adjust the offset of each subsequent text_element
        while (te_it != text_elements.end())
        {
            auto ii_sub_begin = te_it->text.begin() - text.begin();
            auto ii_sub_end = te_it->text.end() - text.begin();
            te_it->text = Substring(text, ii_sub_begin + change_of_len, ii_sub_end + change_of_len);

            ++te_it;
        }
    }

    FillTemplatedText(text, text_elements, start_of_reflow);
}


namespace {
    [[nodiscard]] constexpr Flags<TextFormat> ValidateFormat(Flags<TextFormat> format) noexcept
    {
        // correct any disagreements in the format flags
        uint8_t dup_ct = 0;   // duplication count
        if (format & FORMAT_LEFT) ++dup_ct;
        if (format & FORMAT_RIGHT) ++dup_ct;
        if (format & FORMAT_CENTER) ++dup_ct;
        if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_LEFT by default
            format &= ~(FORMAT_RIGHT | FORMAT_CENTER);
            format |= FORMAT_LEFT;
        }
        uint8_t dup_ct2 = 0;
        if (format & FORMAT_TOP) ++dup_ct2;
        if (format & FORMAT_BOTTOM) ++dup_ct2;
        if (format & FORMAT_VCENTER) ++dup_ct2;
        if (dup_ct2 != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_TOP by default
            format &= ~(FORMAT_BOTTOM | FORMAT_VCENTER);
            format |= FORMAT_TOP;
        }
        if ((format & FORMAT_WORDBREAK) && (format & FORMAT_LINEWRAP))   // only one of these can be picked; FORMAT_WORDBREAK overrides FORMAT_LINEWRAP
            format &= ~FORMAT_LINEWRAP;

        return format;
    }

    void SetJustification(bool& last_line_of_curr_just, Font::LineData& line_data,
                          Alignment orig_just, Alignment prev_just) noexcept
    {
        if (last_line_of_curr_just) {
            line_data.justification = orig_just;
            last_line_of_curr_just = false;
        } else {
            line_data.justification = prev_just;
        }
    }

    void AddNewline(X& x, bool& last_line_of_curr_just, std::vector<Font::LineData>& line_data,
                    const Alignment orig_just)
    {
        line_data.emplace_back();
        SetJustification(last_line_of_curr_just,
                         line_data.back(),
                         orig_just,
                         line_data[line_data.size() - 2].justification);
        x = X0;
    }

    void AddWhitespace(X& x, const Font::Substring elem_text, const int8_t space_width, const X box_width,
                       const bool expand_tabs, const X tab_pixel_width, const Flags<TextFormat> format,
                       std::vector<Font::LineData>& line_data, bool& last_line_of_curr_just,
                       const Alignment orig_just, const StrSize original_string_offset,
                       CPSize& code_point_offset, std::vector<Font::TextElement>& pending_formatting_tags)
    {
        auto it = elem_text.begin();
        const auto end_it = elem_text.end();
        while (it != end_it) {
            const StrSize char_index{static_cast<std::size_t>(std::distance(elem_text.begin(), it))};
            const uint32_t c = utf8::next(it, end_it);
            const StrSize char_size{std::distance(elem_text.begin(), it) - Value(char_index)};
            if (c != WIDE_CR && c != WIDE_FF) {
                const X advance_position =
                    (c == WIDE_TAB && expand_tabs) ? (((x / tab_pixel_width) + 1) * tab_pixel_width) :
                    (c == WIDE_NEWLINE) ? x : (x + space_width);
                const X advance = advance_position - x;

                // if we're using linewrap and this space won't fit on this line
                if ((format & FORMAT_LINEWRAP) && (box_width < advance_position)) {
                    if (x == X0 && box_width < advance) {
                        // if the space is larger than the line and alone
                        // on the line, let the space overrun this line
                        // and then start a new one
                        line_data.emplace_back();
                        x = X0; // reset the x-position to 0
                        SetJustification(last_line_of_curr_just,
                                         line_data.back(),
                                         orig_just,
                                         line_data[line_data.size() - 2].justification);
                    } else {
                        // otherwise start a new line and put the space there
                        line_data.emplace_back();
                        x = advance;
                        line_data.back().char_data.emplace_back(
                            x,
                            original_string_offset + char_index,
                            char_size,
                            code_point_offset,
                            pending_formatting_tags);

                        pending_formatting_tags.clear();
                        SetJustification(last_line_of_curr_just,
                                         line_data.back(),
                                         orig_just,
                                         line_data[line_data.size() - 2].justification);
                    }
                } else { // there's room for the space, or we're not using linewrap
                    x += advance;
                    line_data.back().char_data.emplace_back(
                        x,
                        original_string_offset + char_index,
                        char_size,
                        code_point_offset,
                        pending_formatting_tags);
                    pending_formatting_tags.clear();
                }
            }
            ++code_point_offset;
        }
    }

    void AddTextWordbreak(X& x, const Font::TextElement& elem, const Flags<TextFormat> format,
                          const X box_width, std::vector<Font::LineData>& line_data,
                          bool& last_line_of_curr_just, const Alignment orig_just,
                          const StrSize original_string_offset, CPSize& code_point_offset,
                          std::vector<Font::TextElement>& pending_formatting_tags)
    {
        // if the text "word" overruns this line, and isn't alone on
        // this line, move it down to the next line
        if (box_width < x + elem.Width() && x != X0) {
            line_data.emplace_back();
            x = X0;
            SetJustification(last_line_of_curr_just,
                             line_data.back(),
                             orig_just,
                             line_data[line_data.size() - 2].justification);
        }
        auto it = elem.text.begin();
        const auto end_it = elem.text.end();
        std::size_t j = 0;
        while (it != end_it) {
            const StrSize char_index{static_cast<std::size_t>(std::distance(elem.text.begin(), it))};
            utf8::next(it, end_it);
            const StrSize char_size{std::distance(elem.text.begin(), it) - Value(char_index)};
            x += elem.widths[j];
            line_data.back().char_data.emplace_back(
                x,
                original_string_offset + char_index,
                char_size,
                code_point_offset,
                pending_formatting_tags);
            pending_formatting_tags.clear();
            ++j;
            ++code_point_offset;
        }
    }

    void AddTextNoWordbreak(X& x, const Font::TextElement& elem, const Flags<TextFormat> format,
                            const X box_width, std::vector<Font::LineData>& line_data,
                            bool& last_line_of_curr_just, const Alignment orig_just,
                            const StrSize original_string_offset, CPSize& code_point_offset,
                            std::vector<Font::TextElement>& pending_formatting_tags)
    {
        auto it = elem.text.begin();
        const auto end_it = elem.text.end();
        std::size_t j = 0;
        while (it != end_it) {
            const StrSize char_index{static_cast<std::size_t>(std::distance(elem.text.begin(), it))};
            utf8::next(it, end_it);
            const StrSize char_size{std::distance(elem.text.begin(), it) - Value(char_index)};
            // if the char overruns this line, and isn't alone on this
            // line, move it down to the next line
            if ((format & FORMAT_LINEWRAP) && box_width < x + elem.widths[j] && x != X0) {
                line_data.emplace_back();
                x = X{elem.widths[j]};
                line_data.back().char_data.emplace_back(
                    x,
                    original_string_offset + char_index,
                    char_size,
                    code_point_offset,
                    pending_formatting_tags);
                pending_formatting_tags.clear();
                SetJustification(last_line_of_curr_just,
                                 line_data.back(),
                                 orig_just,
                                 line_data[line_data.size() - 2].justification);
            } else {
                // there's room for this char on this line, or there's no wrapping in use
                x += elem.widths[j];
                line_data.back().char_data.emplace_back(
                    x,
                    original_string_offset + char_index,
                    char_size,
                    code_point_offset,
                    pending_formatting_tags);
                pending_formatting_tags.clear();
            }
            ++j;
            ++code_point_offset;
        }
    }

    void AddText(X& x, const Font::TextElement& elem, const Flags<TextFormat> format,
                 const X box_width, std::vector<Font::LineData>& line_data,
                 bool& last_line_of_curr_just, const Alignment orig_just,
                 const StrSize original_string_offset, CPSize& code_point_offset,
                 std::vector<Font::TextElement>& pending_formatting_tags)
    {
        if (format & FORMAT_WORDBREAK) {
            AddTextWordbreak(x, elem, format, box_width, line_data, last_line_of_curr_just,
                             orig_just, original_string_offset, code_point_offset,
                             pending_formatting_tags);
        } else {
            AddTextNoWordbreak(x, elem, format, box_width, line_data, last_line_of_curr_just,
                               orig_just, original_string_offset, code_point_offset,
                               pending_formatting_tags);
        }
    }

    void AddOpenTag(const Font::TextElement& elem, Alignment& justification,
                    bool& last_line_of_curr_just, CPSize& code_point_offset,
                    std::vector<Font::TextElement>& pending_formatting_tags)
    {
        if (elem.tag_name == ALIGN_LEFT_TAG)
            justification = ALIGN_LEFT;
        else if (elem.tag_name == ALIGN_CENTER_TAG)
            justification = ALIGN_CENTER;
        else if (elem.tag_name == ALIGN_RIGHT_TAG)
            justification = ALIGN_RIGHT;
        else if (elem.tag_name != PRE_TAG)
            pending_formatting_tags.push_back(elem);
        last_line_of_curr_just = false;
        code_point_offset += elem.CodePointSize();
    }

    void AddCloseTag(const Font::TextElement& elem, const Alignment justification,
                     bool& last_line_of_curr_just, CPSize& code_point_offset,
                     std::vector<Font::TextElement>& pending_formatting_tags)
    {
        if ((elem.tag_name == ALIGN_LEFT_TAG && justification == ALIGN_LEFT) ||
            (elem.tag_name == ALIGN_CENTER_TAG && justification == ALIGN_CENTER) ||
            (elem.tag_name == ALIGN_RIGHT_TAG && justification == ALIGN_RIGHT))
        {
            last_line_of_curr_just = true;
        } else if (elem.tag_name != PRE_TAG) {
            pending_formatting_tags.push_back(elem);
        }
        code_point_offset += elem.CodePointSize();
    }
}

std::vector<Font::LineData> Font::DetermineLines(
    const std::string& text, Flags<TextFormat> format, X box_width,
    const std::vector<TextElement>& text_elements) const
{
    // HACK - Workaround for #2166
    // On OSX, right clicking an unowned planet at game start may result in utf8::invalid_utf8 or utf8::not_enough_room
    if (!utf8::is_valid(text.begin(), text.end())) {
        std::cerr << "Invalid UTF8 in text: " << text;
        return std::vector<Font::LineData>{};
    }

    format = ValidateFormat(format); // may modify format

    static constexpr int tab_width = 8; // default tab width
    const X tab_pixel_width = X{tab_width * m_space_width}; // get the length of a tab stop
    const bool expand_tabs = format & FORMAT_LEFT; // tab expansion only takes place when the lines are left-justified (otherwise, tabs are just spaces)
    const Alignment orig_just =
        (format & FORMAT_LEFT) ? ALIGN_LEFT :
        (format & FORMAT_CENTER) ? ALIGN_CENTER :
        (format & FORMAT_RIGHT) ? ALIGN_RIGHT : ALIGN_NONE;
    bool last_line_of_curr_just = false; // is this the last line of the current justification? (for instance when a </right> tag is encountered)

    std::vector<Font::LineData> line_data;
    if (!text_elements.empty())
        line_data.emplace_back(orig_just);

    X x = X0;
    // the position within the original string of the current TextElement
    StrSize original_string_offset(S0);
    // the index of the first code point of the current TextElement
    CPSize code_point_offset(CP0);
    std::vector<TextElement> pending_formatting_tags;

    for (const auto& elem : text_elements) {
        switch (elem.Type()) {
        case TextElement::TextElementType::NEWLINE:
            AddNewline(x, last_line_of_curr_just, line_data, orig_just);
            break;
        case TextElement::TextElementType::WHITESPACE:
            AddWhitespace(x, elem.text, m_space_width, box_width, expand_tabs, tab_pixel_width, format,
                          line_data, last_line_of_curr_just, orig_just, original_string_offset,
                          code_point_offset, pending_formatting_tags);
            break;
        case TextElement::TextElementType::TEXT:
            AddText(x, elem, format, box_width, line_data, last_line_of_curr_just, orig_just,
                    original_string_offset, code_point_offset, pending_formatting_tags);
            break;
        case TextElement::TextElementType::OPEN_TAG:
            AddOpenTag(elem, line_data.back().justification, last_line_of_curr_just,
                       code_point_offset, pending_formatting_tags);
            break;
        case TextElement::TextElementType::CLOSE_TAG:
            AddCloseTag(elem, line_data.back().justification, last_line_of_curr_just,
                        code_point_offset, pending_formatting_tags);
            break;
        }
        original_string_offset += elem.StringSize();
    }
    // disregard the final pending formatting tag, if any, since this is the
    // end of the text, and so it cannot have any effect

#if DEBUG_DETERMINELINES
    DebugOutput::PrintLineBreakdown(text, format, box_width, line_data);
#endif

    return line_data;
}

FT_Error Font::GetFace(FT_Face& face)
{ return FT_New_Face(g_library.m_library, m_font_filename.c_str(), 0, &face); }

FT_Error Font::GetFace(const std::vector<uint8_t>& file_contents, FT_Face& face)
{ return FT_New_Memory_Face(g_library.m_library, &file_contents[0], file_contents.size(), 0, &face); }

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
    if (!m_pt_sz)
        throw InvalidPointSize("Attempted to create font \"" + m_font_filename + "\" with 0 point size");

    // Set the character size and use default 72 DPI
    if (FT_Set_Char_Size(face, 0, m_pt_sz * 64, 0, 0)) // if error is returned
        throw BadPointSize("Could not set font size while attempting to create font \"" + m_font_filename + "\"");

    // Get the scalable font metrics for this font
    const auto scale = face->size->metrics.y_scale;
    m_ascent = Y(static_cast<int>(face->size->metrics.ascender / 64.0)); // convert from fixed-point 26.6 format
    m_descent = Y(static_cast<int>(face->size->metrics.descender / 64.0)); // convert from fixed-point 26.6 format
    m_height = m_ascent - m_descent + 1;
    m_lineskip = Y(static_cast<int>(face->size->metrics.height / 64.0));
    // underline info
    m_underline_offset = std::floor(FT_MulFix(face->underline_position, scale) / 64.0);
    m_underline_height = std::ceil(FT_MulFix(face->underline_thickness, scale) / 64.0);
    if (m_underline_height < 1.0)
        m_underline_height = 1.0;

    // italics info
    m_italics_offset = ITALICS_FACTOR * m_height / 2.0;
    // shadow info
    m_shadow_offset = 1.0;
    // super/subscript
    m_super_sub_offset = m_height / 4.0;

    // we always need these whitespace, number, and punctuation characters
    std::vector<std::pair<uint32_t, uint32_t>> range_vec(
        PRINTABLE_ASCII_NONALPHA_RANGES.begin(),
        PRINTABLE_ASCII_NONALPHA_RANGES.end());

    // add ASCII letter characters or user-specified scripts to them, if the user specifies a specific set of
    // characters
    if (m_charsets.empty()) {
        range_vec.insert(range_vec.end(),
                         PRINTABLE_ASCII_ALPHA_RANGES.begin(),
                         PRINTABLE_ASCII_ALPHA_RANGES.end());
    } else {
        for (std::size_t i = 0; i < m_charsets.size(); ++i)
            range_vec.emplace_back(m_charsets[i].m_first_char, m_charsets[i].m_last_char);
    }

    //Get maximum texture size
    GLint GL_TEX_MAX_SIZE;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GL_TEX_MAX_SIZE);
    const std::size_t TEX_MAX_SIZE = GL_TEX_MAX_SIZE;

    std::vector<std::pair<uint32_t, TempGlyphData>> temp_glyph_data;
    temp_glyph_data.reserve(1000); // rough guesstimate

    // Start with width and height of 16,
    // increase them as needed.
    // We will lay out the glyphs on the texture side by side
    // until the width would reach TEX_MAX_SIZE, then go to the next row.
    // QUESTION: Would a more square-like shape be better for the texture?
    Buffer2d<uint16_t> buffer(X{16}, Y{16}, 0);

    X x = X0;
    Y y = Y0;
    X max_x = X0;
    Y max_y = Y0;
    for (const auto &[low, high] : range_vec) {
        for (uint32_t c = low; c < high; ++c) {
            // skip already-existing glphys
            if (std::any_of(temp_glyph_data.begin(), temp_glyph_data.end(),
                            [c](const auto& tgd) { return tgd.first == c; }))
            { continue; }
            const auto generated = GenerateGlyph(face, c);
            if (!generated)
                continue;

            // copy glyph images
            const FT_Bitmap& glyph_bitmap = face->glyph->bitmap;
            if ((glyph_bitmap.width > TEX_MAX_SIZE) || (glyph_bitmap.rows > TEX_MAX_SIZE))
                ThrowBadGlyph("GG::Font::Init : Glyph too large for buffer'%1%'", c); // catch broken fonts

            if (static_cast<std::size_t>(Value(x)) + glyph_bitmap.width >= TEX_MAX_SIZE) { // start a new row of glyph images
                if (x > max_x) max_x = x;
                x = X0;
                y = max_y;
            }
            if (static_cast<std::size_t>(Value(y)) + glyph_bitmap.rows >= TEX_MAX_SIZE) {
                // We cannot make the texture any larger. The font does not fit.
                ThrowBadGlyph("GG::Font::Init : Face too large for buffer. First glyph to no longer fit: '%1%'", c);
            }
            if (y + Y(glyph_bitmap.rows) > max_y)
                max_y = y + Y(glyph_bitmap.rows + 1); //Leave a one pixel gap between glyphs

            uint8_t* const src_start = glyph_bitmap.buffer;
            // Resize buffer to fit new data
            buffer.at(x + X(glyph_bitmap.width), y + Y(glyph_bitmap.rows)) = 0;

            for (unsigned int row = 0; row < glyph_bitmap.rows; ++row) {
                uint8_t* src = src_start + row * glyph_bitmap.pitch;
                uint16_t* dst = &buffer.get(x, y + Y(row));
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
            int16_t y_offset = static_cast<int16_t>(Value(m_height - 1 + m_descent - face->glyph->bitmap_top));
            int16_t left_b = static_cast<int16_t>(std::ceil(face->glyph->metrics.horiBearingX / 64.0));
            int16_t adv = static_cast<int16_t>(std::ceil(face->glyph->metrics.horiAdvance / 64.0));

            static constexpr int16_t int8max = std::numeric_limits<int8_t>::max();
            static constexpr int16_t int8min = std::numeric_limits<int8_t>::min();
            if (y_offset > int8max) {
                std::cout << "glyph " << c << " y offset too big: " << y_offset;
                y_offset = int8max;
            }
            if (y_offset < int8min) {
                std::cout << "glyph " << c << " y offset too small: " << y_offset;
                y_offset = int8min;
            }
            if (left_b > int8max) {
                std::cout << "glyph " << c << " left bearing too big: " << left_b;
                left_b = int8max;
            }
            if (left_b < int8min) {
                std::cout << "glyph " << c << " left bearing too small: " << left_b;
                left_b = int8min;
            }
            if (adv > int8max) {
                std::cout << "glyph " << c << " advance too big: " << adv;
                adv = int8max;
            }
            if (adv < int8min) {
                std::cout << "glyph " << c << " advance too small: " << adv;
                adv = int8min;
            }

            temp_glyph_data.emplace_back(std::piecewise_construct,
                                         std::forward_as_tuple(c),
                                         std::forward_as_tuple(
                                             Pt(x, y),
                                             Pt(x + X(glyph_bitmap.width), y + Y(glyph_bitmap.rows)),
                                             static_cast<int8_t>(y_offset),
                                             static_cast<int8_t>(left_b),
                                             static_cast<int8_t>(adv)));

            // advance buffer write-position
            x += X(glyph_bitmap.width + 1); //Leave a one pixel gap between glyphs
        }
    }

    buffer.MakePowerOfTwo();

    // create opengl texture from buffer
    m_texture = std::make_shared<Texture>();
    m_texture->Init(buffer.BufferWidth(), buffer.BufferHeight(),
                    (uint8_t*)buffer.Buffer(), GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2);

    // create Glyph objects from temp glyph data
    for (const auto& [codepoint, glyph_data] : temp_glyph_data) {
        m_glyphs[codepoint] = Glyph(m_texture, glyph_data.ul, glyph_data.lr, glyph_data.y_offset,
                                    glyph_data.left_b, glyph_data.adv);
    }

    // record the width of the space character
    auto glyph_it = m_glyphs.find(WIDE_SPACE);
    assert(glyph_it != m_glyphs.end());
    m_space_width = glyph_it->second.advance;
}

bool Font::GenerateGlyph(FT_Face face, uint32_t ch)
{
    bool retval = true;

    // load the glyph
    if (!face)
        throw BadFace("GG::Font::GetGlyphBitmap : invalid font or font face");

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

void Font::StoreGlyphImpl(Font::RenderCache& cache, Clr color, Pt pt,
                          const Glyph& glyph, int x_top_offset, int y_shift) const
{
    const auto tc = glyph.sub_texture.TexCoords();
    const auto l = static_cast<GLfloat>(pt.x + glyph.left_bearing);
    const auto w = static_cast<GLfloat>(glyph.sub_texture.Width());
    const auto t = static_cast<GLfloat>(pt.y + glyph.y_offset);

    cache.coordinates.store(std::array{tc[0], tc[1], tc[2], tc[1], tc[2], tc[3], tc[0], tc[3]});

    cache.vertices.store(std::array{l + x_top_offset,      t + y_shift,
                                    l + w + x_top_offset,  t + y_shift,
                                    l + w - x_top_offset,  t + glyph.sub_texture.Height() + y_shift,
                                    l - x_top_offset,      t + glyph.sub_texture.Height() + y_shift});

    cache.colors.store<4>(color);
}

void Font::StoreUnderlineImpl(Font::RenderCache& cache, Clr color, Pt pt, const Glyph& glyph,
                              Y descent, Y height, Y underline_height, Y underline_offset) const
{
    X x1 = pt.x;
    Y y1(pt.y + height + descent - underline_offset);
    X x2 = x1 + glyph.advance;
    Y y2(y1 + underline_height);

    cache.underline_vertices.store(x1, y1);
    cache.underline_colors.store(color);
    cache.underline_vertices.store(x2, y1);
    cache.underline_colors.store(color);
    cache.underline_vertices.store(x2, y2);
    cache.underline_colors.store(color);
    cache.underline_vertices.store(x1, y2);
    cache.underline_colors.store(color);
}

X Font::StoreGlyph(Pt pt, const Glyph& glyph, const Font::RenderState& render_state,
                   Font::RenderCache& cache) const
{
    int italic_top_offset = 0;
    int shadow_offset = 0;
    int super_sub_offset = 0;

    if (render_state.use_italics) // Should we enable sub pixel italics offsets?
        italic_top_offset = static_cast<int>(m_italics_offset);
    if (render_state.use_shadow)
        shadow_offset = static_cast<int>(m_shadow_offset);
    super_sub_offset = -static_cast<int>(render_state.super_sub_shift * m_super_sub_offset);

    // render shadows?
    if (shadow_offset > 0) {
        StoreGlyphImpl(cache, CLR_BLACK, pt + Pt(X1, Y0), glyph, italic_top_offset, super_sub_offset);
        StoreGlyphImpl(cache, CLR_BLACK, pt + Pt(X0, Y1), glyph, italic_top_offset, super_sub_offset);
        StoreGlyphImpl(cache, CLR_BLACK, pt + Pt(-X1, Y0), glyph, italic_top_offset, super_sub_offset);
        StoreGlyphImpl(cache, CLR_BLACK, pt + Pt(X0, -Y1), glyph, italic_top_offset, super_sub_offset);
        if (render_state.draw_underline) {
            StoreUnderlineImpl(cache, CLR_BLACK, pt + Pt(X0, Y1), glyph, m_descent,
                               m_height, Y(m_underline_height), Y(m_underline_offset));
            StoreUnderlineImpl(cache, CLR_BLACK, pt + Pt(X0, -Y1), glyph, m_descent,
                               m_height, Y(m_underline_height), Y(m_underline_offset));
        }
    }

    // render main text
    StoreGlyphImpl(cache, render_state.CurrentColor(), pt, glyph, italic_top_offset, super_sub_offset);
    if (render_state.draw_underline) {
        StoreUnderlineImpl(cache, render_state.CurrentColor(), pt, glyph, m_descent,
                           m_height, Y(m_underline_height), Y(m_underline_offset));
    }

    return X{glyph.advance};
}

namespace {
    constexpr uint8_t CharsToUInt8(std::string_view txt) {
        if (txt.empty())
            return 0u;

        uint32_t retval = 0u;
        for (auto c : txt) {
            if (c > '9' || c < '0')
                break;
            retval *= 10;
            retval += (c - '0');
        }

        return static_cast<uint8_t>(retval);
    }
    static_assert(CharsToUInt8("") == 0);
    static_assert(CharsToUInt8("abcdefgh") == 0);
    static_assert(CharsToUInt8("0") == 0);
    static_assert(CharsToUInt8("-25") == 0);
    static_assert(CharsToUInt8("25") == 25);
    static_assert(CharsToUInt8("00001") == 1);
    static_assert(CharsToUInt8("888") == 888-3*256);
    static_assert(CharsToUInt8(std::string_view{one_zero_nine.data()}) == 109);
    static_assert(CharsToUInt8(std::string_view{three_zero.data()}) == 30);

    std::pair<std::array<GLubyte, 4u>, bool> TagParamsToColor(const std::vector<Font::Substring>& params) {
        std::array<GLubyte, 4u> retval{};
        if (params.size() != 4u)
            return {retval, false};

#if defined(__cpp_lib_to_chars)
        for (std::size_t n = 0u; n < 4u; ++n) {
            const auto& param{params[n]};
            if (param.empty())
                return {retval, false};
            auto ec = std::from_chars(param.data(), param.data() + param.size(), retval[n]).ec;
            if (ec != std::errc())
                return {retval, false};
        }
#else
        for (std::size_t n = 0u; n < 4u; ++n) {
            const auto& param{params[n]};
            if (param.empty())
                return {retval, false};
            retval[n] = CharsToUInt8(param);
        }
#endif

        return {retval, true};
    }
}

void Font::HandleTag(const TextElement& tag, RenderState& render_state) const
{
    if (tag.tag_name == ITALIC_TAG) {
        if (tag.IsCloseTag()) {
            if (render_state.use_italics)
                --render_state.use_italics;
        } else {
            ++render_state.use_italics;
        }
    } else if (tag.tag_name == UNDERLINE_TAG) {
        if (tag.IsCloseTag()) {
            if (render_state.draw_underline)
                --render_state.draw_underline;
        } else {
            ++render_state.draw_underline;
        }
    } else if (tag.tag_name == SHADOW_TAG) {
        if (tag.IsCloseTag()) {
            if (render_state.use_shadow)
                --render_state.use_shadow;
        } else {
            ++render_state.use_shadow;
        }
    } else if (tag.tag_name == SUPERSCRIPT_TAG) {
        if (tag.IsCloseTag())
            --render_state.super_sub_shift;
        else
            ++render_state.super_sub_shift;

    } else if (tag.tag_name == SUBSCRIPT_TAG) {
        if (tag.IsCloseTag())
            ++render_state.super_sub_shift;
        else
            --render_state.super_sub_shift;

    } else if (tag.tag_name == RGBA_TAG) {
        if (tag.IsCloseTag()) {
            // Popping is ok also for an empty color stack.
            render_state.PopColor();

        } else {
            auto [color, well_formed_tag] = TagParamsToColor(tag.params);
            if (well_formed_tag) {
                glColor4ubv(color.data());
                render_state.PushColor(color[0], color[1], color[2], color[3]);
            } else {
                std::cerr << "GG::Font : Encountered malformed <rgba> formatting tag from text";
                if (tag.text.IsDefaultEmpty())
                    std::cerr << ": (default EMPTY Substring)";
                else if (tag.text.empty())
                    std::cerr << ": (empty Substring)";
                else
                    std::cerr << " (" << tag.text.size() << "): " << tag.text;
                std::cerr << " ... tag_name: " << tag.tag_name << " ... params (" << tag.params.size() << ") :";
                for (const auto& p : tag.params)
                    std::cerr << " (" << p.size() << "):" << p;
                std::cerr << std::endl;
            }
        }
    }
}

bool Font::IsDefaultFont() const noexcept
{ return m_font_filename == StyleFactory::DefaultFontName(); }

std::shared_ptr<Font> Font::GetDefaultFont(unsigned int pts)
{ return GUI::GetGUI()->GetStyleFactory()->DefaultFont(pts); }


///////////////////////////////////////
// class GG::FontManager
///////////////////////////////////////
const std::shared_ptr<Font> FontManager::EMPTY_FONT{std::make_shared<Font>("", 0)};

bool FontManager::HasFont(std::string_view font_filename, unsigned int pts) const noexcept
{ return FontLookup(font_filename, pts) != m_rendered_fonts.end(); }

namespace {
    const std::vector<UnicodeCharset> empty_charsets;
    const auto empty_it = empty_charsets.end();
}

std::shared_ptr<Font> FontManager::GetFont(std::string_view font_filename, unsigned int pts)
{ return GetFont(font_filename, pts, empty_it, empty_it); }

std::shared_ptr<Font> FontManager::GetFont(std::string_view font_filename, unsigned int pts,
                                           const std::vector<uint8_t>& file_contents)
{ return GetFont(font_filename, pts, file_contents, empty_it, empty_it); }

void FontManager::FreeFont(std::string_view font_filename, unsigned int pts)
{
    auto it = FontLookup(font_filename, pts);
    if (it != m_rendered_fonts.end())
        m_rendered_fonts.erase(it);
}

FontManager& GG::GetFontManager()
{
    static FontManager manager;
    return manager;
}
