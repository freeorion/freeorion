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

    struct SetPreformattedIfPREP
    {
        typedef void result_type;

        void operator()(const std::string* str, const Font::Substring::IterPair& tag_match_iter_pair,
                        bool& is_preformatted, bool set_to_value) const
        {
            if (str) {
                const Font::Substring tag_ss(*str, tag_match_iter_pair);
                if (tag_ss == Font::PRE_TAG)
                    is_preformatted = set_to_value;
            }
        }
    };
    const boost::xpressive::function<SetPreformattedIfPREP>::type SetPreformattedIfPRE = {{}};

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
#if !(defined(__cpp_lib_constexpr_string) && defined(_MSC_VER) && (_MSC_VER >= 1934))
const std::string Font::Substring::EMPTY_STRING{};
#else
static_assert(Font::Substring(Font::Substring::EMPTY_STRING).empty());
static_assert(Font::Substring(Font::Substring::EMPTY_STRING).data() == Font::Substring::EMPTY_STRING.data());
#endif


namespace {
    constexpr struct U8NextFn {
        uint32_t operator()(std::string::const_iterator& text_it, const std::string::const_iterator& end_it) const
        { return utf8::next(text_it, end_it); }
    } u8next_fn;

    template <typename GlyphMap, typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void SetTextElementWidths(const std::string& text, std::vector<Font::TextElement>& text_elements,
                                             std::vector<Font::TextElement>::iterator start, const GlyphMap& glyphs,
                                             int8_t space_width, const TextNextFn& text_next_fn = u8next_fn)
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
                uint32_t c = text_next_fn(text_it, end_it);
                if (c != WIDE_NEWLINE) {
                    auto it = glyphs.find(c);
                    // use a space when an unrendered glyph is requested (the
                    // space chararacter is always renderable)
                    elem.widths.back() = (it != glyphs.end()) ? it->second.advance : space_width;
                }
            }
        }
    }

    template <typename GlyphMap, typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void SetTextElementWidths(const std::string& text, std::vector<Font::TextElement>& text_elements,
                                             const GlyphMap& glyphs, int8_t space_width, const TextNextFn& text_next_fn = u8next_fn)
    { return SetTextElementWidths(text, text_elements, text_elements.begin(), glyphs, space_width, text_next_fn); }
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

    CONSTEXPR_FONT void SetJustification(bool& last_line_of_curr_just, Font::LineData& line_data,
                                         Alignment orig_just, Alignment prev_just) noexcept
    {
        if (last_line_of_curr_just) {
            line_data.justification = orig_just;
            last_line_of_curr_just = false;
        } else {
            line_data.justification = prev_just;
        }
    }

    CONSTEXPR_FONT void AddNewline(X& x, bool& last_line_of_curr_just, Font::LineVec& line_data,
                                   const Alignment orig_just)
    {
        line_data.emplace_back();
        SetJustification(last_line_of_curr_just,
                         line_data.back(),
                         orig_just,
                         line_data.at(line_data.size() - 2).justification);
        x = X0;
    }

    template <typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void AddWhitespace(X& x, const Font::Substring elem_text, const int8_t space_width, const X box_width,
                                      const bool expand_tabs, const X tab_pixel_width, const Flags<TextFormat> format,
                                      Font::LineVec& line_data, bool& last_line_of_curr_just,
                                      const Alignment orig_just, const StrSize original_string_offset,
                                      CPSize& code_point_offset, std::vector<Font::TextElement>& pending_formatting_tags,
                                      const TextNextFn& text_next_fn)
    {
        auto it = elem_text.begin();
        const auto end_it = elem_text.end();
        while (it != end_it) {
            const StrSize char_index{static_cast<std::size_t>(std::distance(elem_text.begin(), it))};
            const uint32_t c = text_next_fn(it, end_it);
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
                                         line_data.at(line_data.size() - 2).justification);
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
                                         line_data.at(line_data.size() - 2).justification);
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

    template <typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void AddTextWordbreak(X& x, const Font::TextElement& elem, const Flags<TextFormat> format,
                                         const X box_width, Font::LineVec& line_data,
                                         bool& last_line_of_curr_just, const Alignment orig_just,
                                         const StrSize original_string_offset, CPSize& code_point_offset,
                                         std::vector<Font::TextElement>& pending_formatting_tags,
                                         const TextNextFn& text_next_fn)
    {
        // if the text "word" overruns this line, and isn't alone on
        // this line, move it down to the next line
        if (box_width < x + elem.Width() && x != X0) {
            line_data.emplace_back();
            x = X0;
            SetJustification(last_line_of_curr_just,
                             line_data.back(),
                             orig_just,
                             line_data.at(line_data.size() - 2).justification);
        }
        auto it = elem.text.begin();
        const auto end_it = elem.text.end();
        std::size_t j = 0;
        while (it != end_it) {
            const StrSize char_index{static_cast<std::size_t>(std::distance(elem.text.begin(), it))};
            text_next_fn(it, end_it);
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

    template <typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void AddTextNoWordbreak(X& x, const Font::TextElement& elem, const Flags<TextFormat> format,
                                           const X box_width, Font::LineVec& line_data,
                                           bool& last_line_of_curr_just, const Alignment orig_just,
                                           const StrSize original_string_offset, CPSize& code_point_offset,
                                           std::vector<Font::TextElement>& pending_formatting_tags,
                                           const TextNextFn& text_next_fn)
    {
        auto it = elem.text.begin();
        const auto end_it = elem.text.end();
        std::size_t j = 0;
        while (it != end_it) {
            const StrSize char_index{static_cast<std::size_t>(std::distance(elem.text.begin(), it))};
            text_next_fn(it, end_it);
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
                                 line_data.at(line_data.size() - 2).justification);
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

    template <typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void AddText(X& x, const Font::TextElement& elem, const Flags<TextFormat> format,
                                const X box_width, Font::LineVec& line_data,
                                bool& last_line_of_curr_just, const Alignment orig_just,
                                const StrSize original_string_offset, CPSize& code_point_offset,
                                std::vector<Font::TextElement>& pending_formatting_tags,
                                const TextNextFn& text_next_fn)
    {
        if (format & FORMAT_WORDBREAK) {
            AddTextWordbreak(x, elem, format, box_width, line_data, last_line_of_curr_just,
                             orig_just, original_string_offset, code_point_offset,
                             pending_formatting_tags, text_next_fn);
        } else {
            AddTextNoWordbreak(x, elem, format, box_width, line_data, last_line_of_curr_just,
                               orig_just, original_string_offset, code_point_offset,
                               pending_formatting_tags, text_next_fn);
        }
    }

    template <typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void AddOpenTag(const Font::TextElement& elem, Alignment& justification,
                                   bool& last_line_of_curr_just, CPSize& code_point_offset,
                                   std::vector<Font::TextElement>& pending_formatting_tags)
    {
        if (elem.tag_name == Font::ALIGN_LEFT_TAG)
            justification = ALIGN_LEFT;
        else if (elem.tag_name == Font::ALIGN_CENTER_TAG)
            justification = ALIGN_CENTER;
        else if (elem.tag_name == Font::ALIGN_RIGHT_TAG)
            justification = ALIGN_RIGHT;
        else if (elem.tag_name != Font::PRE_TAG)
            pending_formatting_tags.push_back(elem);
        last_line_of_curr_just = false;
        code_point_offset += elem.CodePointSize();
    }

    template <typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT void AddCloseTag(const Font::TextElement& elem, const Alignment justification,
                                    bool& last_line_of_curr_just, CPSize& code_point_offset,
                                    std::vector<Font::TextElement>& pending_formatting_tags)
    {
        if ((elem.tag_name == Font::ALIGN_LEFT_TAG && justification == ALIGN_LEFT) ||
            (elem.tag_name == Font::ALIGN_CENTER_TAG && justification == ALIGN_CENTER) ||
            (elem.tag_name == Font::ALIGN_RIGHT_TAG && justification == ALIGN_RIGHT))
        {
            last_line_of_curr_just = true;
        } else if (elem.tag_name != Font::PRE_TAG) {
            pending_formatting_tags.push_back(elem);
        }
        code_point_offset += elem.CodePointSize();
    }

    template <typename TextNextFn = U8NextFn>
    CONSTEXPR_FONT auto AssembleLineData(Flags<TextFormat> format, X box_width,
                                         const std::vector<Font::TextElement>& text_elements, int8_t space_width,
                                         const TextNextFn& text_next_fn = u8next_fn)
    {
        format = ValidateFormat(format); // may modify format

        using TextElement = Font::TextElement;

        constexpr int tab_width = 8; // default tab width
        const X tab_pixel_width = X{tab_width * space_width}; // get the length of a tab stop
        const bool expand_tabs = format & FORMAT_LEFT; // tab expansion only takes place when the lines are left-justified (otherwise, tabs are just spaces)
        const Alignment orig_just =
            (format & FORMAT_LEFT) ? ALIGN_LEFT :
            (format & FORMAT_CENTER) ? ALIGN_CENTER :
            (format & FORMAT_RIGHT) ? ALIGN_RIGHT : ALIGN_NONE;
        bool last_line_of_curr_just = false; // is this the last line of the current justification? (for instance when a </right> tag is encountered)

        Font::LineVec line_data;
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
                AddWhitespace(x, elem.text, space_width, box_width, expand_tabs, tab_pixel_width, format,
                              line_data, last_line_of_curr_just, orig_just, original_string_offset,
                              code_point_offset, pending_formatting_tags, text_next_fn);
                break;
            case TextElement::TextElementType::TEXT:
                AddText(x, elem, format, box_width, line_data, last_line_of_curr_just, orig_just,
                        original_string_offset, code_point_offset, pending_formatting_tags, text_next_fn);
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

        return line_data;
    }
}

///////////////////////////////////////
// Free Functions
///////////////////////////////////////
std::ostream& GG::operator<<(std::ostream& os, Font::Substring substr)
{
    std::ostream_iterator<char> out_it(os);
    std::copy(substr.begin(), substr.end(), out_it);
    return os;
}

namespace {
    CONSTEXPR_FONT CPSize GlyphIndexInLines(
        std::size_t line_idx, CPSize glyph_index, const Font::LineVec& line_data)
    {
        const auto num_lines = line_data.size();
        CPSize retval = CP0;
        if (line_data.empty() || (num_lines == 1u && line_data.front().Empty()))
            return retval; // no text

        // sum line glyph counts before the target row
        for (std::size_t loop_line_idx = 0u; loop_line_idx < line_idx && loop_line_idx < num_lines; ++loop_line_idx)
            retval += line_data.at(loop_line_idx).char_data.size();

        if (line_idx >= num_lines)
            return retval; // 

        // add glyphs for target row: lesser of number of glyphs in row and requested glyph index
        const auto& line_chars = line_data.at(line_idx).char_data;
        retval += std::min(glyph_index, CPSize(line_chars.size()));

        return retval;
    }
}

CPSize GG::GlyphIndexOfLineAndGlyph(std::size_t line_idx, CPSize glyph_index, const Font::LineVec& line_data)
{ return GlyphIndexInLines(line_idx, glyph_index, line_data); }


namespace {
    CONSTEXPR_FONT CPSize CodePointIndexFromLineAndGlyphInLines(
        std::size_t line_index, CPSize glyph_index, const Font::LineVec& line_data)
    {
        if (line_data.size() <= line_index) {
            // requested line is not present
            // return one past the last code point in the last non-empty line
            auto it = line_data.rbegin();
            auto end_it = line_data.rend();
            while (it != end_it) {
                if (!it->char_data.empty())
                    return it->char_data.back().code_point_index + CP1;
                ++it;
            }

        } else if (Value(glyph_index) < line_data.at(line_index).char_data.size()) {
            // requested line and glyph index are valid
            return line_data.at(line_index).char_data.at(Value(glyph_index)).code_point_index;

        } else {
            // requested line is OK but requested glyph is not present on that line.
            // return the last code point in the next prior non-empty line
            auto it = line_data.rbegin() + (line_data.size() - 1 - line_index);
            auto end_it = line_data.rend();
            while (it != end_it) {
                if (!it->char_data.empty())
                    return it->char_data.back().code_point_index + CP1;
                ++it;
            }
        }

        return CP0;
    }
}

CPSize GG::CodePointIndexOfLineAndGlyph(
    std::size_t line_index, CPSize glyph_index, const Font::LineVec& line_data)
{ return CodePointIndexFromLineAndGlyphInLines(line_index, glyph_index, line_data); }

namespace {
  /** Returns the code point index (CPI) after the previous glyph to the glyph at \a glyph_index.
    *
    * Ranges of glyphs are specified [closed, open) eg. [1, 3) includes glyphs
    * 1 and 2, but not the 3rd glyph. If finding the corresponding CPI range,
    * for the end glyph, the result should not include any CPI after the end of
    * the second to last glyph, even if there are non-glyph code points between them.
    *
    * For example, "ab<i>c" has glyphs at CPIs 0 (a), 1 (b), and 5 (c) and also has
    * non-glyph (ie. tag) CPIs 2 (<), 3 (i), and 4 (>). If just getting the CPI for
    * the starts of glyphs, the end glyph would start at code point 5 (c), but the
    * CPI after the last glyph included in the range is actually 2 (<). */
    CONSTEXPR_FONT CPSize CodePointIndexAfterPreviousGlyphInLines(
        std::size_t line_index, CPSize glyph_index, const Font::LineVec& line_data)
    {
        if (glyph_index == CP0 && line_index == 0)
            return CP0; // there is no prior glyph, so just use the first code point, which will be before any tags that affect the first glyph

        if (line_data.size() <= line_index) // requested line is not present; return one past the last code point in the last non-empty line
            return CodePointIndexFromLineAndGlyphInLines(line_index, glyph_index, line_data);

        const auto& cds = line_data.at(line_index).char_data;

        if (glyph_index == CP0 || cds.empty()) {
            // get glyph at the end of the next previous non-empty line
            auto it = line_data.rbegin() + (line_data.size() - 1 - (line_index-1));
            const auto end_it = line_data.rend();
            while (it != end_it) {
                const auto& cd = it->char_data;
                if (cd.empty()) {
                    ++it; // keep searching
                    continue;
                }
                return cd.back().code_point_index + CP1; // code point immediately after prior glyph
            }
            return CP0; // there is no prior glyph, so just use the first code point, which will be before any tags that affect the first glyph
        }

        if (Value(glyph_index) >= cds.size()) // line is present but requested glyph is past end of that line, return one past end of line
            return cds.back().code_point_index + CP1;

        // line is present and glyph is present and is not the first glyph in the line
        // return one past code point of previous glyph in the line
        return cds.at(Value(glyph_index - CP1)).code_point_index + CP1;
    }
}

CPSize GG::CodePointIndexAfterPreviousGlyph(
    std::size_t line_index, CPSize glyph_index, const Font::LineVec& line_data)
{ return CodePointIndexAfterPreviousGlyphInLines(line_index, glyph_index, line_data); }


namespace {
    CONSTEXPR_FONT std::pair<std::size_t, CPSize>
    LineIndexAndCPIndexFromCodePointInLines(CPSize cp_index, const Font::LineVec& line_data)
    {
        for (std::size_t i = 0; i < line_data.size(); ++i) {
            const auto& char_data = line_data.at(i).char_data;
            if (!char_data.empty() &&
                char_data.front().code_point_index <= cp_index &&
                cp_index <= char_data.back().code_point_index)
            { return {i, cp_index - char_data.front().code_point_index}; }
        }
        return {std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE};
    }


#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr struct DummyNextFn {
        static constexpr uint32_t cont_byte(uint8_t c) noexcept
        { return c & 0b00111111; };

        static constexpr uint32_t cont_byte(char c) noexcept
        { return cont_byte(static_cast<uint8_t>(c)); };

        template <typename char_iterator_t>
        static constexpr uint32_t cont_byte(char_iterator_t char_it) noexcept
        {
            static_assert(std::is_same_v<std::decay_t<decltype(*char_it)>, char>);
            return cont_byte(*char_it);
        };

        // constexpr OK alternative to utf8::next
        // does not validate utf8 continuation bytes, but just increments
        // based on initial byte-determined utf8 sequence length
        template <typename char_iterator_t>
        constexpr uint32_t operator()(char_iterator_t& text_it, const char_iterator_t& end_it) const
        {
            static_assert(std::is_same_v<std::decay_t<decltype(*text_it)>, char>);
            if (text_it == end_it)
                return 0u;

            // first byte determines sequence length
            uint32_t c0 = static_cast<uint32_t>(static_cast<uint8_t>(*text_it)); // via uint8_t to ensure values map to 0-255
            const std::ptrdiff_t sequence_length =
                (c0 <= 0x7F) ? 1 :
                (c0 >= 0xC2 && c0 <= 0xDF) ? 2 :
                (c0 >= 0xE0 && c0 <= 0xEF) ? 3 :
                (c0 >= 0xF0) ? 4 : 0;

            if (sequence_length == 0)
                throw std::invalid_argument("dummy utf8 next given invalid initial byte");
            if (sequence_length == 1) {
                ++text_it;
                return c0;
            }

            // verify that there are enough bytes in iterator range for sequence length
            const std::ptrdiff_t it_length = std::distance(text_it, end_it);
            if (it_length < sequence_length)
                throw std::invalid_argument("dummy utf8 next given too short sequence for indicated sequence length");

            // extract, mask, and compile continuation bytes with masked initial byte
            if (sequence_length == 2) {
                c0 &= 0b00011111;
                uint32_t c1 = cont_byte(text_it + 1);
                text_it += 2;
                return (c0 << 6) + c1;

            } else if (sequence_length == 3) {
                c0 &= 0b00001111;
                uint32_t c1 = cont_byte(text_it + 1);
                uint32_t c2 = cont_byte(text_it + 2);
                text_it += 3;
                return (c0 << 12) + (c1 << 6) + c2;

            } else if (sequence_length == 4) {
                c0 &= 0b00000111;
                uint32_t c1 = cont_byte(text_it + 1);
                uint32_t c2 = cont_byte(text_it + 2);
                uint32_t c3 = cont_byte(text_it + 3);
                text_it += 4;
                return (c0 << 18) + (c1 << 12) + (c2 << 6) + c3;

            } else {
                throw std::invalid_argument("dummy utf8 next somehow got unexpected sequence length");
            }
        }

        template <typename char_iterator_t>
        constexpr std::pair<uint32_t, std::ptrdiff_t> operator()(const char_iterator_t&& text_it, const char_iterator_t&& end_it) const
        {
            char_iterator_t local_it(text_it);
            const char_iterator_t local_end_it(end_it);
            return {operator()(local_it, local_end_it), std::distance(text_it, local_it)};
        }

        constexpr std::pair<uint32_t, std::ptrdiff_t> operator()(const std::string_view sv) const
        { return operator()(sv.begin(), sv.end()); }

        constexpr std::pair<uint32_t, std::ptrdiff_t> operator()(const std::string_view sv, std::size_t offset) const
        { return operator()(sv.substr(offset)); }
    } dummy_next_fn;

    static_assert('!' < 0b00111111 && DummyNextFn::cont_byte('!') == '!');
    static_assert('A' > 0b00111111 && DummyNextFn::cont_byte('A') == (uint8_t('A') & 0b00111111));


    // text with all single-byte chars
    constexpr std::string_view all_ascii_sv = "!Ascii";
    using cdp = std::pair<uint32_t, std::ptrdiff_t>;
    static_assert(dummy_next_fn(all_ascii_sv) == cdp('!', 1));
    static_assert(dummy_next_fn(all_ascii_sv, 1) == cdp('A', 1));
    static_assert(dummy_next_fn(all_ascii_sv.end(), all_ascii_sv.end()) == cdp(0, 0));


    // for constexpr test purposes
    struct DummyPair {
        uint32_t first = 0;
        struct { int8_t advance = 4; } second;
    };
    constexpr struct DummyGlyphMap {
        static constexpr DummyPair value{};
        constexpr auto* find(uint32_t) const noexcept { return &value; }
        constexpr decltype(value)* end() const noexcept { return nullptr; }
    } dummy_glyph_map;

    static_assert(dummy_glyph_map.find(0)->second.advance == 4);


    // text with multi-byte chars
#  if defined(__cpp_lib_char8_t)
    constexpr std::u8string_view long_chars = u8"αbåオ🠞و";
    constexpr auto long_chars_arr = []() {
        std::array<std::string_view::value_type, long_chars.size()> retval{};
        for (std::size_t idx = 0; idx < retval.size(); ++idx)
            retval[idx] = long_chars[idx];
        return retval;
    }();
    constexpr std::string_view long_chars_sv(long_chars_arr.data(), long_chars_arr.size());
#  else
    constexpr std::string_view long_chars_sv = u8"αbåオ🠞و";
    constexpr auto long_chars_arr = []() {
        std::array<std::string_view::value_type, long_chars_sv.size()> retval{};
        for (std::size_t idx = 0; idx < retval.size(); ++idx)
            retval[idx] = long_chars_sv[idx];
        return retval;
    }();
#  endif

#  if defined(__cpp_lib_constexpr_vector)
    CONSTEXPR_FONT std::vector<Font::TextElement> ElementsForLongCharsText(const std::string& text)
    {
        std::vector<Font::TextElement> elems;
        elems.emplace_back(Font::Substring(text, 0u, 14u));
        SetTextElementWidths(text, elems, dummy_glyph_map, 4, dummy_next_fn);
        return elems;
    }
#  endif

    constexpr std::array<uint8_t, 14> long_chars_as_uint8_expected{
        0xCE, 0xB1,   'b',   0xC3, 0xA5,   0xE3, 0x82, 0xAA,   0xF0, 0x9F, 0xA0, 0x9E,   0xD9, 0x88};

    constexpr auto check_eq = [](const auto& lhs, const auto& rhs) -> bool {
        if (lhs.size() != rhs.size())
            return false;
        for (std::size_t idx = 0; idx < lhs.size(); ++idx)
            if (lhs[idx] != static_cast<std::decay_t<decltype(lhs[0])>>(rhs[idx]))
                return false;
        return true;
    };
    static_assert(check_eq(long_chars_arr, long_chars_as_uint8_expected));

    constexpr std::array<cdp, 6> long_chars_as_uint32_t_and_length_expected{{
        {0x3B1, 2}, {'b', 1}, {0xE5, 2}, {0x30AA, 3}, {0x1F81E, 4}, {0x648, 2}}};
    constexpr std::array<cdp, 6> long_chars_as_uint32_t_and_length_extracted{{
        dummy_next_fn(long_chars_sv), dummy_next_fn(long_chars_sv.substr(2)), dummy_next_fn(long_chars_sv.substr(3)),
        dummy_next_fn(long_chars_sv.substr(5)), dummy_next_fn(long_chars_sv.substr(8)), dummy_next_fn(long_chars_sv.substr(12))}};
    static_assert(check_eq(long_chars_as_uint32_t_and_length_expected, long_chars_as_uint32_t_and_length_extracted));

#  if defined(__cpp_lib_constexpr_vector)
    // tests getting line and code point index in line from overall code point index text with multi-byte chars
    constexpr auto test_multibyte_cpidx_to_line_and_cp = []() {
        const std::string text(long_chars_sv);
        const auto elems = ElementsForLongCharsText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        const auto c_to_l_and_c = [line_data](CPSize c_idx) { return LineIndexAndCPIndexFromCodePointInLines(c_idx, line_data); };

        return std::array{
            c_to_l_and_c(CPSize{0}), c_to_l_and_c(CPSize{1}),
            c_to_l_and_c(CPSize{2}), c_to_l_and_c(CPSize{3}),
            c_to_l_and_c(CPSize{4}), c_to_l_and_c(CPSize{5}),
            c_to_l_and_c(CPSize{6}), c_to_l_and_c(CPSize{7})
        };
    }();

    constexpr std::array<std::pair<std::size_t, CPSize>, 8> test_multibyte_line_and_cp_expected{{
        {0u, CPSize{0}}, {0u, CPSize{1}},
        {0u, CPSize{2}}, {0u, CPSize{3}},
        {0u, CPSize{4}}, {0u, CPSize{5}},
        {std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE}, {std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE}
    }};

    static_assert(test_multibyte_cpidx_to_line_and_cp == test_multibyte_line_and_cp_expected);
#  endif

    constexpr std::string_view multi_line_text = "ab\ncd\n\nef";
#  if defined(__cpp_lib_constexpr_vector)
    CONSTEXPR_FONT std::vector<Font::TextElement> ElementsForMultiLineText(const std::string& text)
    {
        std::vector<Font::TextElement> elems;
        elems.reserve(6);
        elems.emplace_back(Font::Substring(text, 0u, 2u));
        elems.emplace_back(Font::Substring(text, 2u, 3u), Font::TextElement::TextElementType::NEWLINE);
        elems.emplace_back(Font::Substring(text, 3u, 5u));
        elems.emplace_back(Font::Substring(text, 5u, 6u), Font::TextElement::TextElementType::NEWLINE);
        elems.emplace_back(Font::Substring(text, 6u, 7u), Font::TextElement::TextElementType::NEWLINE);
        elems.emplace_back(Font::Substring(text, 7u, 9u));

        SetTextElementWidths(text, elems, dummy_glyph_map, 4, dummy_next_fn);

        return elems;
    }
#  endif

#  if defined(__cpp_lib_constexpr_vector)
    // tests line data for multi-line text
    constexpr auto test_multline_line_data = []() {
        const std::string text(multi_line_text);
        const auto elems = ElementsForMultiLineText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        return AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);
    };
    static_assert(test_multline_line_data().size() == 4);
    static_assert(test_multline_line_data()[0].char_data.size() == 2);
    static_assert(test_multline_line_data()[0].char_data[0].string_index == S0);
    static_assert(test_multline_line_data()[0].char_data[0].string_size == S1);
    static_assert(test_multline_line_data()[0].char_data[0].code_point_index == CP0);
    static_assert(test_multline_line_data()[0].char_data[0].tags.empty());
    static_assert(test_multline_line_data()[0].char_data[1].string_index == S1);
    static_assert(test_multline_line_data()[0].char_data[1].string_size == S1);
    static_assert(test_multline_line_data()[0].char_data[1].code_point_index == CP1);
    static_assert(test_multline_line_data()[1].char_data.size() == 2);
    static_assert(test_multline_line_data()[1].char_data[0].string_index == StrSize{3});    // newlines count for string index
    static_assert(test_multline_line_data()[1].char_data[0].string_size == S1);
    static_assert(test_multline_line_data()[1].char_data[0].code_point_index == CPSize{2}); // newlines don't count for code-point indices
    static_assert(test_multline_line_data()[1].char_data[0].tags.empty());
    static_assert(test_multline_line_data()[1].char_data[1].string_index == StrSize{4});
    static_assert(test_multline_line_data()[1].char_data[1].string_size == S1);
    static_assert(test_multline_line_data()[1].char_data[1].code_point_index == CPSize{3});
    static_assert(test_multline_line_data()[2].char_data.empty());
    static_assert(test_multline_line_data()[3].char_data.size() == 2);
    static_assert(test_multline_line_data()[3].char_data[0].string_index == StrSize{7});
    static_assert(test_multline_line_data()[3].char_data[0].string_size == S1);
    static_assert(test_multline_line_data()[3].char_data[0].code_point_index == CPSize{4});
    static_assert(test_multline_line_data()[3].char_data[0].tags.empty());
    static_assert(test_multline_line_data()[3].char_data[1].string_index == StrSize{8});
    static_assert(test_multline_line_data()[3].char_data[1].string_size == S1);
    static_assert(test_multline_line_data()[3].char_data[1].code_point_index == CPSize{5});


    // tests getting line and code point index in line from overall code point index text with newlines
    constexpr auto test_multiline_cpidx_to_line_and_cp = []() {
        const std::string text(multi_line_text);
        const auto elems = ElementsForMultiLineText(text);

        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);
        const auto c_to_l_and_c = [line_data](CPSize c_idx) { return LineIndexAndCPIndexFromCodePointInLines(c_idx, line_data); };

        return std::array{
            c_to_l_and_c(CPSize{0}), c_to_l_and_c(CPSize{1}),
            c_to_l_and_c(CPSize{2}), c_to_l_and_c(CPSize{3}),
            c_to_l_and_c(CPSize{4}), c_to_l_and_c(CPSize{5}),
            c_to_l_and_c(CPSize{6}), c_to_l_and_c(CPSize{7})
        };
    }();

    constexpr std::array<std::pair<std::size_t, CPSize>, 8> test_multiline_line_and_cp_expected{{
        {0u, CPSize{0}}, {0u, CPSize{1}},
        {1u, CPSize{0}}, {1u, CPSize{1}},
        {3u, CPSize{0}}, {3u, CPSize{1}},
        {std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE}, {std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE}
    }};

    static_assert(test_multiline_cpidx_to_line_and_cp == test_multiline_line_and_cp_expected);


    // tests getting overall glyph index from line and in-line glyph index in text with newlines
    constexpr auto test_multiline_lineidx_and_inline_glyph_to_overall_glyph =
        [](std::size_t line_idx, std::size_t glyph_idx)
    {
        const std::string text(multi_line_text);
        const auto elems = ElementsForMultiLineText(text);

        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        return Value(GlyphIndexInLines(line_idx, CPSize(glyph_idx), line_data));
    };

    constexpr auto gltg00 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(0, 0);
    constexpr auto gltg01 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(0, 1);
    constexpr auto gltg02 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(0, 2);
    constexpr auto gltg03 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(0, 3);
    constexpr auto gltg10 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(1, 0);
    constexpr auto gltg11 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(1, 1);
    constexpr auto gltg12 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(1, 2);
    constexpr auto gltg20 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(2, 0);
    constexpr auto gltg21 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(2, 1);
    constexpr auto gltg22 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(2, 2);
    constexpr auto gltg30 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(3, 0);
    constexpr auto gltg31 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(3, 1);
    constexpr auto gltg32 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(3, 2);
    constexpr auto gltg40 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(4, 0);
    constexpr auto gltg41 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(4, 1);
    constexpr auto gltg50 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(5, 0);
    constexpr auto gltg51 = test_multiline_lineidx_and_inline_glyph_to_overall_glyph(5, 1);

    // total glyph: 01  23    456
    // line:        00  11  2 33
    // lineglyph:   01  01    01
    //             "ab\ncd\n\nef";
    static_assert(gltg00 == 0u);
    static_assert(gltg01 == 1u);
    static_assert(gltg02 == 2u);
    static_assert(gltg03 == 2u);
    static_assert(gltg10 == 2u);
    static_assert(gltg11 == 3u);
    static_assert(gltg12 == 4u);
    static_assert(gltg20 == 4u);
    static_assert(gltg21 == 4u);
    static_assert(gltg22 == 4u);
    static_assert(gltg30 == 4u);
    static_assert(gltg31 == 5u);
    static_assert(gltg32 == 6u);
    static_assert(gltg40 == 6u);
    static_assert(gltg41 == 6u);
    static_assert(gltg50 == 6u);
    static_assert(gltg51 == 6u);
#  endif

    constexpr std::string_view tagged_test_text = "ab<i>cd</i>ef";
#  if defined(__cpp_lib_constexpr_vector)
    CONSTEXPR_FONT std::vector<Font::TextElement> ElementsForTaggedText(const std::string& text)
    {
        std::vector<Font::TextElement> elems;
        elems.reserve(5);
        elems.emplace_back(Font::Substring(text, 0u, 2u));
        elems.emplace_back(Font::Substring(text, 2u, 5u), Font::Substring(text, 3u, 4u), Font::TextElement::TextElementType::OPEN_TAG);
        elems.emplace_back(Font::Substring(text, 5u, 7u));
        elems.emplace_back(Font::Substring(text, 7u, 11u), Font::Substring(text, 9u, 10u), Font::TextElement::TextElementType::OPEN_TAG);
        elems.emplace_back(Font::Substring(text, 11u, 13u));

        SetTextElementWidths(text, elems, dummy_glyph_map, 4, dummy_next_fn);

        return elems;
    }
#  endif

#  if defined(__cpp_lib_constexpr_vector)
    // tests getting line and code point index in line from overall code point index text with tags
    constexpr auto test_tagged_cpidx_to_line_and_cp = []() {
        const std::string text(tagged_test_text);
        const auto elems = ElementsForTaggedText(text);

        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);
        const auto c_to_l_and_c = [line_data](CPSize c_idx) { return LineIndexAndCPIndexFromCodePointInLines(c_idx, line_data); };

        return std::array{
            c_to_l_and_c(CPSize{0}), c_to_l_and_c(CPSize{1}),
            c_to_l_and_c(CPSize{2}), c_to_l_and_c(CPSize{3}),
            c_to_l_and_c(CPSize{4}), c_to_l_and_c(CPSize{5}),
            c_to_l_and_c(CPSize{6}), c_to_l_and_c(CPSize{7}),
            c_to_l_and_c(CPSize{8}), c_to_l_and_c(CPSize{9}),
            c_to_l_and_c(CPSize{10}), c_to_l_and_c(CPSize{11}),
            c_to_l_and_c(CPSize{12}), c_to_l_and_c(CPSize{13})
        };
    }();

    constexpr std::array<std::pair<std::size_t, CPSize>, 14> test_tagged_line_and_cp_expected{{
        {0u, CPSize{0}}, {0u, CPSize{1}}, {0u, CPSize{2}}, {0u, CPSize{3}},
        {0u, CPSize{4}}, {0u, CPSize{5}}, {0u, CPSize{6}}, {0u, CPSize{7}},
        {0u, CPSize{8}}, {0u, CPSize{9}}, {0u, CPSize{10}}, {0u, CPSize{11}},
        {0u, CPSize{12}}, {std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE}
    }};

    static_assert(test_tagged_cpidx_to_line_and_cp == test_tagged_line_and_cp_expected);


    // tests getting code point index after previous glyph in text with newlines
    constexpr auto test_multiline_line_glyph_to_after_prev_glyph_cpi = [](std::size_t line_idx, std::size_t glyph_idx) {
        const std::string text(multi_line_text);
        const auto elems = ElementsForMultiLineText(text);

        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);
        return Value(CodePointIndexAfterPreviousGlyphInLines(line_idx, CPSize{glyph_idx}, line_data));
    };

    //  CPSize: 01  23    456
    //  line:   00  11    33
    //  glyph:  01  01    01
    //         "ab\ncd\n\nef";
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(0, 0) == 0); // a
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(0, 1) == 1); // b
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(0, 2) == 2); // c
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(0, 3) == 2); // c
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(1, 0) == 2); // c
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(1, 1) == 3); // d
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(2, 0) == 4); // e
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(3, 0) == 4); // e
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(3, 1) == 5); // f
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(3, 2) == 6); // null
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(5, 0) == 6); // null
    static_assert(test_multiline_line_glyph_to_after_prev_glyph_cpi(5, 999) == 6); // null


    // tests getting code point index after previous glyph in text with tags
    constexpr auto test_tagged_line_glyph_to_after_prev_glyph_cpi = [](std::size_t glyph_idx) {
        const std::string text(tagged_test_text);
        const auto elems = ElementsForTaggedText(text);

        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);
        return Value(CodePointIndexAfterPreviousGlyphInLines(0u, CPSize{glyph_idx}, line_data));
    };

    //  CPSize: 01234567890123
    //  glyph:  01   23    45
    //         "ab<i>cd</i>ef";
    static_assert(test_tagged_line_glyph_to_after_prev_glyph_cpi(0) == 0); // a
    static_assert(test_tagged_line_glyph_to_after_prev_glyph_cpi(1) == 1); // b
    static_assert(test_tagged_line_glyph_to_after_prev_glyph_cpi(2) == 2); // <
    static_assert(test_tagged_line_glyph_to_after_prev_glyph_cpi(3) == 6); // d
    static_assert(test_tagged_line_glyph_to_after_prev_glyph_cpi(4) == 7); // <
    static_assert(test_tagged_line_glyph_to_after_prev_glyph_cpi(5) == 12); // f
    static_assert(test_tagged_line_glyph_to_after_prev_glyph_cpi(6) == 13); // null
#  endif
#endif
}


namespace {
    CONSTEXPR_FONT std::pair<std::size_t, CPSize> LineIndexAndCPIndexFromGlyphInLines(
        std::size_t target_glyph_index, const Font::LineVec& line_data)
    {
        // find target glyph index, return corresponding line and codepoint index
        std::size_t glyph_count = 0u;
        for (std::size_t loop_line_idx = 0; loop_line_idx < line_data.size(); ++loop_line_idx) {
            const auto& cd = line_data.at(loop_line_idx).char_data;
            const auto num_line_glyphs = cd.size();
            const auto& target_glyph_idx_in_line = target_glyph_index - glyph_count;

            if (target_glyph_idx_in_line >= num_line_glyphs) {
                // target glyph is on a later line, if any
                glyph_count += num_line_glyphs; // skip whole line
                continue;
            }
            // glyph should be on this line
            const auto& target_glyph = cd.at(target_glyph_idx_in_line);
            const auto target_glyph_cp_idx = target_glyph.code_point_index;
            const auto& first_glyph_in_line = cd.front();
            const auto first_glyph_in_line_cp_idx = first_glyph_in_line.code_point_index;

            return {loop_line_idx, target_glyph_cp_idx - first_glyph_in_line_cp_idx};
        }
        return {std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE};
    }
}


namespace {
    CONSTEXPR_FONT std::pair<StrSize, StrSize> StringIndexAndStrSizeOfGlyphIndex(CPSize glyph_idx, const Font::LineVec& line_data)
    {
        if (line_data.empty())
            return {S0, S0}; // no glyphs, so just return default 0 position and length

        // seach through lines for the requested glyph
        CPSize glyph_count = CP0;
        for (const auto& line : line_data) {
            const auto line_sz = CPSize(line.char_data.size());
            if (glyph_count + line_sz <= glyph_idx) {
                // requested glyph is on a later line
                glyph_count += line_sz;
                continue;
            }
            // glyph should be on this line
            const auto glyph_offset_in_line = Value(glyph_idx - glyph_count);
            const auto& glyph = line.char_data.at(glyph_offset_in_line);
            return {glyph.string_index, glyph.string_size};
        }

        // glyph not found, return past end of last glyph by searching backwards from end for a line with a glyph
        for (std::size_t loop_line_idx = line_data.size()-1u; loop_line_idx >= 0u; --loop_line_idx) {
            const auto& line = line_data.at(loop_line_idx);
            if (line.Empty())
                continue;
            const auto& last_glyph = line.char_data.back();
            return {last_glyph.string_index + last_glyph.string_size, S0};
        }

        return {S0, S0}; // no glyphs, so just return default 0 position and length
    }


    CONSTEXPR_FONT std::pair<StrSize, StrSize> StringIndexAndLengthOfLineAndGlyphInLines(
        std::size_t line_idx, CPSize glyph_index, const Font::LineVec& line_data)
    {
        if (line_data.empty())
            return {S0, S0};

        // get requested line
        if (line_idx >= line_data.size()) {
            // no such line, return string index after last glyph

            // find last glyph by searching backwards from end for a line with a glyph
            for (std::size_t loop_line_idx = line_data.size()-1u; loop_line_idx >= 0u; --loop_line_idx) {
                const auto& line = line_data.at(loop_line_idx);
                if (line.Empty())
                    continue;
                const auto& last_glyph = line.char_data.back();
                return {last_glyph.string_index + last_glyph.string_size, S0};
            }
            return {S0, S0}; // no glyphs, so just return default 0 position and length
        }
        const auto& cd = line_data.at(line_idx).char_data;

        // get requested glyph
        if (Value(glyph_index) >= cd.size()) {
            // no such glyph. return string index after last glyph of this line
            return {cd.back().string_index + cd.back().string_size, S0};
        }

        // return requeste glyph position and length
        const auto& cdgi = cd.at(Value(glyph_index));
        return {cdgi.string_index, cdgi.string_size};
    }
}


std::pair<std::size_t, CPSize> GG::LinePositionOfGlyph(CPSize index, const Font::LineVec& line_data)
{ return LineIndexAndCPIndexFromGlyphInLines(Value(index), line_data); }

std::pair<std::size_t, CPSize> GG::LinePositionOfCodePoint(CPSize index, const Font::LineVec& line_data)
{ return LineIndexAndCPIndexFromCodePointInLines(index, line_data); }



namespace {
    // if line_idx is past the last line, return the string index of just past the last glyph on the last line
    //
    // if line_idx is within the valid lines and the requested code point is within the number of glyphs in that line,
    // return the string index of that glyph on that line
    //
    // if line_idx is within the valid lines but the requested glyph index is past the end of that line,
    // return the string index of just past the previous glyph, searching backwards from the end of that line
    // the returned string index may be for the last glyph on the same line, or on a previous line if there are
    // no glyphs on the requested line
    //
    // if searching back and no previous lines have a code point on them, return string index S0
    CONSTEXPR_FONT std::pair<StrSize, StrSize> StringIndexFromLineAndGlyphInLines(
        std::size_t line_idx, CPSize index, const Font::LineVec& line_data)
    {
        if (line_idx < line_data.size() && Value(index) < line_data.at(line_idx).char_data.size()) {
            // line is valid, and requested code point index is within the line
            const auto& cd = line_data.at(line_idx).char_data.at(Value(index));
            return std::pair{cd.string_index, cd.string_size};
        }

        // if there is no such line, start searching at the last (rbegin) line.
        // if there is such a line, but (due to above check) it has not enough code points, start
        // searching for the previous (rnext) code point, from the end of the requested line
        // eg. line_data.size() = 10, line_idx = 9, roffset = 10 - 1 - 9 = 0 -> line with forward idx = 9
        // eg. line_data.size() = 10, line_idx = 0, roffset = 10 - 1 - 0 = 9 -> line with forward idx = 0
        const std::size_t roffset = (line_idx >= line_data.size()) ?    // is requested line valid/present?
            0 :                                 // no -> search back from last line
            (line_data.size() - 1 - line_idx);  // yes -> search back from requested line

        for (auto rit = line_data.rbegin() + roffset; rit != line_data.rend(); ++rit) {
            if (rit->char_data.empty())
                continue;
            // found a line with at least one glyph on it
            const auto& cd = rit->char_data.back();
            return std::pair{cd.string_index + cd.string_size, S0}; // return string index to after the last glyph on the line
        }

        return std::pair(S0, S0); // no code point found, so just return index 0
    }


#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))

#  if defined(__cpp_lib_constexpr_vector)
    // tests getting string index from glyph index in text with newlines
    constexpr auto test_line_glyph_to_str_idx = []() {
        const std::string text(multi_line_text);
        const auto elems = ElementsForMultiLineText(text);

        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);
        const auto c_to_s = [line_data](std::size_t line_idx, CPSize c_idx)
        { return StringIndexFromLineAndGlyphInLines(line_idx, c_idx, line_data).first; };
        const CPSize CP2{2u}, CP3{3u};

        return std::array{
            c_to_s(0,CP0), c_to_s(0,CP1), c_to_s(0,CP2), c_to_s(0,CP3),
            c_to_s(1,CP0), c_to_s(1,CP1), c_to_s(1,CP2), c_to_s(1,CP3),
            c_to_s(2,CP0), c_to_s(2,CP1), c_to_s(2,CP2), c_to_s(2,CP3),
            c_to_s(3,CP0), c_to_s(3,CP1), c_to_s(3,CP2), c_to_s(3,CP3),
            c_to_s(4,CP0), c_to_s(4,CP1), c_to_s(4,CP2), c_to_s(4,CP3),
            c_to_s(5,CP0), c_to_s(5,CP1), c_to_s(5,CP2), c_to_s(5,CP3),
        };
    }();
    constexpr std::array<StrSize, 24> test_line_glyph_str_idx_expected{
        StrSize{0}, StrSize{1}, StrSize{2}, StrSize{2},
        StrSize{3}, StrSize{4}, StrSize{5}, StrSize{5},
        StrSize{5}, StrSize{5}, StrSize{5}, StrSize{5},
        StrSize{7}, StrSize{8}, StrSize{9}, StrSize{9},
        StrSize{9}, StrSize{9}, StrSize{9}, StrSize{9},
        StrSize{9}, StrSize{9}, StrSize{9}, StrSize{9}
    };
    static_assert(test_line_glyph_to_str_idx == test_line_glyph_str_idx_expected);


    // tests getting string index and code point string length from glyph index in text with tags
    constexpr auto test_tagged_glyph_idx_to_str_idx = [](std::size_t idx) {
        const std::string text(tagged_test_text);
        const auto elems = ElementsForTaggedText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        return StringIndexAndLengthOfLineAndGlyphInLines(0u, CPSize(idx), line_data);
    };

    constexpr std::pair<std::size_t, std::size_t> Value(std::pair<StrSize, StrSize> szs)
    { return {Value(szs.first), Value(szs.second)}; };

    static_assert(Value(test_tagged_glyph_idx_to_str_idx(0u)) == std::pair{0, 1});
    static_assert(Value(test_tagged_glyph_idx_to_str_idx(1u)) == std::pair{1, 1});
    static_assert(Value(test_tagged_glyph_idx_to_str_idx(2u)) == std::pair{5, 1});
    static_assert(Value(test_tagged_glyph_idx_to_str_idx(3u)) == std::pair{6, 1});
    static_assert(Value(test_tagged_glyph_idx_to_str_idx(4u)) == std::pair{11, 1});
    static_assert(Value(test_tagged_glyph_idx_to_str_idx(5u)) == std::pair{12, 1});
    static_assert(Value(test_tagged_glyph_idx_to_str_idx(6u)) == std::pair{13, 0});
    static_assert(Value(test_tagged_glyph_idx_to_str_idx(999u)) == std::pair{13, 0});


    // tests getting string index and code point string length from glyph index in text with multi-byte characters
    constexpr auto test_multibyte_glyph_to_str_idx_len = [](std::size_t glyph_idx) {
        const std::string text(long_chars_sv);
        const auto elems = ElementsForLongCharsText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        const auto [line_idx, cp_in_line_idx] = LineIndexAndCPIndexFromGlyphInLines(glyph_idx, line_data);
        return StringIndexAndLengthOfLineAndGlyphInLines(line_idx, cp_in_line_idx, line_data);
    };

    static_assert(Value(test_multibyte_glyph_to_str_idx_len(0u)) == std::pair{0, 2});
    static_assert(Value(test_multibyte_glyph_to_str_idx_len(1u)) == std::pair{2, 1});
    static_assert(Value(test_multibyte_glyph_to_str_idx_len(2u)) == std::pair{3, 2});
    static_assert(Value(test_multibyte_glyph_to_str_idx_len(3u)) == std::pair{5, 3});
    static_assert(Value(test_multibyte_glyph_to_str_idx_len(4u)) == std::pair{8, 4});
    static_assert(Value(test_multibyte_glyph_to_str_idx_len(5u)) == std::pair{12, 2});
    static_assert(Value(test_multibyte_glyph_to_str_idx_len(6u)) == std::pair{14, 0});


    // tests getting the code point from line and glyph index in text with a
    // single line with multi-byte characters. should output same index as input
    // index, up to the limit of characters in string, and one past end otherwise
    constexpr auto test_multibyte_line_glyph_to_cp = [](std::size_t line_idx, CPSize index) {
        const std::string text(long_chars_sv);
        const auto elems = ElementsForLongCharsText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        return CodePointIndexFromLineAndGlyphInLines(line_idx, index, line_data);
    };

    static_assert(Value(test_multibyte_line_glyph_to_cp(0u, CP0)) == 0u);
    static_assert(Value(test_multibyte_line_glyph_to_cp(0u, CP1)) == 1u);
    static_assert(Value(test_multibyte_line_glyph_to_cp(0u, CPSize{4})) == 4u);
    static_assert(Value(test_multibyte_line_glyph_to_cp(0u, CPSize{5})) == 5u);
    static_assert(Value(test_multibyte_line_glyph_to_cp(0u, CPSize{6})) == 6u);
    static_assert(Value(test_multibyte_line_glyph_to_cp(0u, CPSize{7})) == 6u);
    static_assert(Value(test_multibyte_line_glyph_to_cp(1u, CP0)) == 6u);
    static_assert(Value(test_multibyte_line_glyph_to_cp(1u, CP1)) == 6u);


    // tests getting the code point from line and glyph index in text with tags
    constexpr auto test_tagged_line_idx_to_cp = [](std::size_t line_idx, CPSize index) {
        const std::string text(tagged_test_text);
        const auto elems = ElementsForTaggedText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        return CodePointIndexFromLineAndGlyphInLines(line_idx, index, line_data);
    };

    static_assert(Value(test_tagged_line_idx_to_cp(0u, CP0)) == 0u);
    static_assert(Value(test_tagged_line_idx_to_cp(0u, CP1)) == 1u);
    static_assert(Value(test_tagged_line_idx_to_cp(0u, CPSize{2})) == 5u);
    static_assert(Value(test_tagged_line_idx_to_cp(0u, CPSize{3})) == 6u);
    static_assert(Value(test_tagged_line_idx_to_cp(0u, CPSize{4})) == 11u);
    static_assert(Value(test_tagged_line_idx_to_cp(0u, CPSize{5})) == 12u);
    static_assert(Value(test_tagged_line_idx_to_cp(0u, CPSize{6})) == 13u);
    static_assert(Value(test_tagged_line_idx_to_cp(0u, CPSize{7})) == 13u);
    static_assert(Value(test_tagged_line_idx_to_cp(1u, CP0)) == 13u);
    static_assert(Value(test_tagged_line_idx_to_cp(2u, CP1)) == 13u);


    // tests getting the code point from line and glyph index in text with newlines
    constexpr auto test_multiline_line_glyph_to_cp = [](std::size_t line_idx, CPSize index) {
        const std::string text(multi_line_text);
        const auto elems = ElementsForMultiLineText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        return CodePointIndexFromLineAndGlyphInLines(line_idx, index, line_data);
    };

    static_assert(Value(test_multiline_line_glyph_to_cp(0u, CP0)) == 0u);
    static_assert(Value(test_multiline_line_glyph_to_cp(0u, CP1)) == 1u);
    static_assert(Value(test_multiline_line_glyph_to_cp(0u, CPSize{2})) == 2u);
    static_assert(Value(test_multiline_line_glyph_to_cp(0u, CPSize{3})) == 2u);
    static_assert(Value(test_multiline_line_glyph_to_cp(1u, CP0)) == 2u);
    static_assert(Value(test_multiline_line_glyph_to_cp(1u, CP1)) == 3u);
    static_assert(Value(test_multiline_line_glyph_to_cp(1u, CPSize{2})) == 4u);
    static_assert(Value(test_multiline_line_glyph_to_cp(1u, CPSize{3})) == 4u);
    static_assert(Value(test_multiline_line_glyph_to_cp(2u, CP0)) == 4u);
    static_assert(Value(test_multiline_line_glyph_to_cp(2u, CP1)) == 4u);
    static_assert(Value(test_multiline_line_glyph_to_cp(3u, CP0)) == 4u);
    static_assert(Value(test_multiline_line_glyph_to_cp(3u, CP1)) == 5u);
    static_assert(Value(test_multiline_line_glyph_to_cp(3u, CPSize{2})) == 6u);
    static_assert(Value(test_multiline_line_glyph_to_cp(4u, CP1)) == 6u);
    static_assert(Value(test_multiline_line_glyph_to_cp(4u, CP1)) == 6u);
    static_assert(Value(test_multiline_line_glyph_to_cp(5u, CPSize{2})) == 6u);
    static_assert(Value(test_multiline_line_glyph_to_cp(6u, CPSize{3})) == 6u);
#  endif
#endif
}

namespace {
    CONSTEXPR_FONT CPSize OnePastLastGlyphIdx(const Font::LineVec& line_data)
    {
        // index of one past last glyph in line data: search backwards from last line for a glyph
        for (auto rit = line_data.rbegin(); rit != line_data.rend(); ++rit) {
            const auto& cd = rit->char_data;
            if (!cd.empty())
                return CPSize(cd.size()) + CP1;
        }
        return CP0; // no glyphs in text?
    }


    CONSTEXPR_FONT std::pair<StrSize, StrSize> GlyphIndicesRangeToStringSizeIndicesInLines(
        CPSize start_idx, CPSize end_idx, const Font::LineVec& line_data)
    {
        if (start_idx == INVALID_CP_SIZE || end_idx == INVALID_CP_SIZE)
            return {S0, S0};
        CPSize checked_start_glyph_idx = std::max(CP0, std::min(start_idx, end_idx));

        // index of one past last glyph
        CPSize end_glyph_idx = OnePastLastGlyphIdx(line_data);
        CPSize clamped_end_idx = std::min(end_glyph_idx, std::max(start_idx, end_idx));

        // get string index of first glyph
        const auto start_str_idx = StringIndexAndStrSizeOfGlyphIndex(checked_start_glyph_idx, line_data).first;
        if (checked_start_glyph_idx == clamped_end_idx || clamped_end_idx == CP0)
            return {start_str_idx, start_str_idx};

        // get string index of glyph prior to end index
        const auto [last_str_idx, last_str_size] = StringIndexAndStrSizeOfGlyphIndex(clamped_end_idx - CP1, line_data);


        // string index of start code point and just after final code point...
        return {start_str_idx, last_str_idx + last_str_size};
    }

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
#  if defined(__cpp_lib_constexpr_vector)
    // tests getting a range pair of string indices from a range pair of glyph indices into text with tags,
    // where the second value should be just after the previous character in the text, which is not the same as
    // before the second character, since there could be non-rendered tag-text code points between them
    constexpr auto test_tagged_glyph_to_str_idx_range = [](std::size_t low_idx, std::size_t high_idx) {
        const std::string text(tagged_test_text);
        const auto elems = ElementsForTaggedText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);

        return GlyphIndicesRangeToStringSizeIndicesInLines(CPSize{low_idx}, CPSize{high_idx}, line_data);
    };


    constexpr auto to_chars = [](std::pair<StrSize, StrSize> idx) -> std::pair<char, char>
    { return std::pair(tagged_test_text[Value(idx.first)], tagged_test_text[Value(idx.second)]); };

    static_assert(to_chars(test_tagged_glyph_to_str_idx_range(0u, 999u)) == std::pair('a', 0));
    static_assert(to_chars(test_tagged_glyph_to_str_idx_range(1u, 2u)) == std::pair('b', '<'));
    static_assert(to_chars(test_tagged_glyph_to_str_idx_range(0u, 4u)) == std::pair('a', '<'));
    static_assert(to_chars(test_tagged_glyph_to_str_idx_range(2u, 4u)) == std::pair('c', '<'));


    constexpr auto to_sv = [](std::pair<StrSize, StrSize> idx) -> std::string_view
    { return tagged_test_text.substr(Value(idx.first), Value(idx.second-idx.first)); };

    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(0u, 999u)) == tagged_test_text);
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(0u, 6u)) == tagged_test_text);
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(0u, 5u)) == "ab<i>cd</i>e");
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(0u, 4u)) == "ab<i>cd");
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(0u, 3u)) == "ab<i>c");
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(1u, 2u)) == "b");
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(1u, 3u)) == "b<i>c");
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(1u, 5u)) == "b<i>cd</i>e");
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(2u, 4u)) == "cd");
    static_assert(to_sv(test_tagged_glyph_to_str_idx_range(2u, 5u)) == "cd</i>e");
#  endif
#endif
}


std::pair<StrSize, StrSize> GG::GlyphIndicesRangeToStringSizeIndices(
    CPSize start_idx, CPSize end_idx, const Font::LineVec& line_data)
{ return GlyphIndicesRangeToStringSizeIndicesInLines(start_idx, end_idx, line_data); }


namespace {
    CONSTEXPR_FONT StrSize CodePointIndexToStringSizeIndexInLines(
        CPSize cp_idx, const Font::LineVec& line_data)
    {
        // LineData contain info about glyphs, not code points, so can't directly access
        // the underlying code points in the text. Instead, can check the code point indices
        // of the glyphs to find the requested ones.
        for (const auto& line : line_data) {
            if (line.Empty())
                continue;
            const auto& char_data = line.char_data;
            if (char_data.front().code_point_index > cp_idx)
                break; // no such code point in glyphs
            if (char_data.back().code_point_index < cp_idx)
                continue; // is not yet on this line
            // does a glyph in this line have the requested code point index?
            for (const auto& glyph : char_data) {
                if (glyph.code_point_index == cp_idx)
                    return glyph.string_index;
            }
        }
        return INVALID_S_SIZE; // didn't find glyph with requested code point index
    }


    CONSTEXPR_FONT std::pair<StrSize, StrSize> CodePointIndicesRangeToStringSizeIndicesInLines(
        CPSize start_idx, CPSize end_idx, const Font::LineVec& line_data)
    {
        return {CodePointIndexToStringSizeIndexInLines(start_idx, line_data),
                CodePointIndexToStringSizeIndexInLines(end_idx, line_data)};
    }

}

std::pair<StrSize, StrSize> GG::CodePointIndicesRangeToStringSizeIndices(
    CPSize start_idx, CPSize end_idx, const Font::LineVec& line_data)
{ return CodePointIndicesRangeToStringSizeIndicesInLines(start_idx, end_idx, line_data); }


namespace {
    // These are the types found by the regular expression: XML open/close tags, text and
    // whitespace.  Each type will correspond to a type of TextElement.
    constexpr std::size_t full_regex_tag_idx = 0;
    constexpr std::size_t tag_name_tag_idx = 1;
    constexpr std::size_t open_bracket_tag_idx = 2;
    constexpr std::size_t close_bracket_tag_idx = 3;
    constexpr std::size_t whitespace_tag_idx = 4;
    constexpr std::size_t text_tag_idx = 5;




    /** TagHandler stores a set of all known tags and provides pre-compiled regexs for those tags.

    Known tags are tags that will be parsed into TextElement OPEN_TAG or CLOSE_TAG. */
    class TagHandler {
    public:
        /** Add a tag to the set of known tags.*/
        void Insert(std::vector<std::string_view> tags)
        {
            std::copy_if(tags.begin(), tags.end(), std::back_inserter(m_custom_tags),
                         [this](const auto tag) { return !IsKnown(tag); });
        }

        bool IsKnown(std::string_view tag) const
        {
            const auto matches_tag = [tag](const auto sv) noexcept{ return sv == tag; };
            return std::any_of(m_default_tags.begin(), m_default_tags.end(), matches_tag)
                || std::any_of(m_custom_tags.begin(), m_custom_tags.end(), matches_tag);
        }

    private:
        // set of tags known to the handler
        static constexpr std::array<std::string_view, 11> m_default_tags{
            {Font::ITALIC_TAG, Font::SHADOW_TAG, Font::UNDERLINE_TAG, Font::SUPERSCRIPT_TAG, Font::SUBSCRIPT_TAG,
            Font::RGBA_TAG, Font::ALIGN_LEFT_TAG, Font::ALIGN_CENTER_TAG, Font::ALIGN_RIGHT_TAG, Font::PRE_TAG, Font::RESET_TAG}};

        std::vector<std::string_view> m_custom_tags;
    } tag_handler;
    namespace xpr = boost::xpressive;

    /** CompiledRegex maintains a compiled boost::xpressive regular
        expression that includes a tag stack which can be cleared and
        provided to callers without the overhead of recompiling the
        regular expression.*/
    class CompiledRegex {
    public:
        CompiledRegex()
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

            //+_w one or more greedy word chars,  () group no capture,  [] semantic operation
            const xpr::sregex TAG_NAME =
                (+xpr::_w)[xpr::check(boost::bind(&CompiledRegex::MatchesKnownTag, this, _1))];

            // *blank  'zero or more greedy whitespace',   >> 'followed by',    _ln 'newline',
            // (set = 'a', 'b') is '[ab]',    +blank 'one or more greedy blank'
            static const xpr::sregex WHITESPACE =
                (*xpr::blank >> (xpr::_ln | (xpr::set = '\n', '\r', '\f'))) | +xpr::blank;

            // < followed by not space or <   or one or more not space or <
            static const xpr::sregex TEXT =
                ('<' >> *~xpr::set[xpr::_s | '<']) | (+~xpr::set[xpr::_s | '<']);

            m_EVERYTHING =
                ('<'                                                                    // < open tag
                 >> (tag_name_tag = TAG_NAME)                                           // TAG_NAME 
                    [xpr::check(boost::bind(&CompiledRegex::NotPreformatted, this, _1))]// unless in preformatted mode
                 >> xpr::repeat<0, 9>(+xpr::blank >> TAG_PARAM)                         // repeat 0 to 9 times: blank followed by TAG_PARAM
                 >> (open_bracket_tag.proto_base() = '>')                               // > close tag
                    [SetPreformattedIfPRE(xpr::ref(m_text), tag_name_tag, xpr::ref(m_preformatted), true)]
                ) |

                ("</"                                                                           // </ open tag with slash
                 >> (tag_name_tag = TAG_NAME)                                                   // TAG_NAME
                    [xpr::check(boost::bind(&CompiledRegex::NotPreformattedOrIsPre, this, _1))] // unless in preformatted mode or unless tag is </pre>
                 >> (close_bracket_tag.proto_base() = '>')                                      // > close tag
                    [SetPreformattedIfPRE(xpr::ref(m_text), tag_name_tag, xpr::ref(m_preformatted), false)]
                ) |

                (whitespace_tag = WHITESPACE) |

                (text_tag = TEXT);
        }

        xpr::sregex& BindRegexToText(const std::string& new_text, bool ignore_tags) noexcept
        {
            m_text = &new_text;
            m_ignore_tags = ignore_tags;
            return m_EVERYTHING;
        }

    private:
        bool MatchesKnownTag(const boost::xpressive::ssub_match& sub) const
        { return !m_ignore_tags && tag_handler.IsKnown(sub.str()); }

        bool NotPreformatted(const boost::xpressive::ssub_match&) const noexcept
        { return !m_preformatted; }

        bool NotPreformattedOrIsPre(const boost::xpressive::ssub_match& sub) const
        { return !m_preformatted || sub.str() == Font::PRE_TAG; }

        const std::string*  m_text = nullptr;

        // The combined regular expression.
        xpr::sregex         m_EVERYTHING;

        bool                m_ignore_tags = false;
        bool                m_preformatted = false;
    } regex_with_tags;
}


///////////////////////////////////////
// class GG::Font::TextElement
///////////////////////////////////////
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
namespace {
    constexpr std::string_view TEST_TEXT_WITH_TAGS = "default<i>ital<u>_ul_it_</i>   _just_ul_</u>\nsecond line<i><sup>is";

#  if defined(__cpp_lib_constexpr_vector)
    constexpr auto TestTextElems(const std::string& text)
    {
        std::vector<Font::TextElement> text_elems;
        text_elems.emplace_back(Font::Substring(text, 0u, 7u));
        text_elems.emplace_back(Font::Substring(text, 7u, 10u), Font::Substring(text, 8u, 9u),
                                Font::TextElement::TextElementType::OPEN_TAG);
        text_elems.emplace_back(Font::Substring(text, 10u, 14u));
        text_elems.emplace_back(Font::Substring(text, 14u, 17u), Font::Substring(text, 15u, 16u),
                                Font::TextElement::TextElementType::OPEN_TAG);
        text_elems.emplace_back(Font::Substring(text, 17u, 24u));
        text_elems.emplace_back(Font::Substring(text, 24u, 28u), Font::Substring(text, 26u, 27u),
                                Font::TextElement::TextElementType::CLOSE_TAG);
        text_elems.emplace_back(Font::Substring(text, 28u, 31u), Font::TextElement::TextElementType::WHITESPACE);
        text_elems.emplace_back(Font::Substring(text, 31u, 40u));
        text_elems.emplace_back(Font::Substring(text, 40u, 44u), Font::Substring(text, 42u, 43u),
                                Font::TextElement::TextElementType::CLOSE_TAG);
        text_elems.emplace_back(Font::Substring(text, 44u, 45u), Font::TextElement::TextElementType::NEWLINE);
        text_elems.emplace_back(Font::Substring(text, 45u, 51u));
        text_elems.emplace_back(Font::Substring(text, 51u, 52u), Font::TextElement::TextElementType::WHITESPACE);
        text_elems.emplace_back(Font::Substring(text, 52u, 56u));
        text_elems.emplace_back(Font::Substring(text, 56u, 59u), Font::Substring(text, 57u, 58u),
                                Font::TextElement::TextElementType::OPEN_TAG);
        text_elems.emplace_back(Font::Substring(text, 59u, 64u), Font::Substring(text, 60u, 63u),
                                Font::TextElement::TextElementType::OPEN_TAG);
        text_elems.emplace_back(Font::Substring(text, 64u, 66u));

        SetTextElementWidths(text, text_elems, dummy_glyph_map, 4, dummy_next_fn);

        return text_elems;
    }

    static_assert([]() {
        const std::string test_text(TEST_TEXT_WITH_TAGS);
        const auto text_elems = TestTextElems(test_text);

        return text_elems.size() == 16 &&
            std::string_view{text_elems[0].text} == "default" &&
            text_elems[1].IsOpenTag() && std::string_view{text_elems[1].tag_name} == "i" && Value(text_elems[1].StringSize()) == 3 &&
            std::string_view{text_elems[2].text} == "ital" &&
            text_elems[3].IsOpenTag() && std::string_view{text_elems[3].tag_name} == "u" && Value(text_elems[3].StringSize()) == 3 &&
            std::string_view{text_elems[4].text} == "_ul_it_" &&
            text_elems[5].IsCloseTag() && std::string_view{text_elems[5].tag_name} == "i" && Value(text_elems[5].StringSize()) == 4 &&
            text_elems[6].IsWhiteSpace() && std::string_view{text_elems[6].text} == "   " &&
            std::string_view{text_elems[7].text} == "_just_ul_" &&
            text_elems[8].IsCloseTag() && std::string_view{text_elems[8].tag_name} == "u" && Value(text_elems[8].StringSize()) == 4 &&
            text_elems[9].IsNewline() && std::string_view{text_elems[9].text} == "\n" &&
            std::string_view{text_elems[10].text} == "second" &&
            text_elems[11].IsWhiteSpace() &&
            std::string_view{text_elems[12].text} == "line" &&
            text_elems[13].IsOpenTag() &&
            text_elems[14].IsOpenTag() &&
            std::string_view{text_elems[15].text} == "is";
    }());

    constexpr auto element_widths = []() {
        const std::string test_text(TEST_TEXT_WITH_TAGS);
        const auto text_elems = TestTextElems(test_text);

        std::array<int, 16> widths{0};
        std::array<std::size_t, 16> widths_sizes{0};

        for (std::size_t idx = 0; idx < std::min(widths.size(), text_elems.size()); ++idx) {
            widths[idx] = Value(text_elems[idx].Width());
            widths_sizes[idx++] = text_elems[idx].widths.size();
        }
        return std::pair{widths, widths_sizes};
    }().first;

    constexpr decltype(element_widths) element_widths_expected{{28,0,16,0,28,0,12,0, 16,0,24,0,16,0,20,0}};
    namespace{
        static_assert(element_widths.size() == element_widths_expected.size());
        static_assert(element_widths[0] == element_widths_expected[0]);
        static_assert(element_widths[1] == element_widths_expected[1]);
        static_assert(element_widths[2] == element_widths_expected[2]);
        static_assert(element_widths[3] == element_widths_expected[3]);
        static_assert(element_widths[4] == element_widths_expected[4]);
        static_assert(element_widths[5] == element_widths_expected[5]);
        static_assert(element_widths[6] == element_widths_expected[6]);
        static_assert(element_widths[7] == element_widths_expected[7]);
        static_assert(element_widths[8] == element_widths_expected[8]);
        static_assert(element_widths[9] == element_widths_expected[9]);
        static_assert(element_widths[10] == element_widths_expected[10]);
        static_assert(element_widths[11] == element_widths_expected[11]);
        static_assert(element_widths[12] == element_widths_expected[12]);
        static_assert(element_widths[13] == element_widths_expected[13]);
        static_assert(element_widths[14] == element_widths_expected[14]);
        static_assert(element_widths[15] == element_widths_expected[15]);
    }
#  endif
}
#endif


namespace {
    CONSTEXPR_FONT std::pair<CPSize, CPSize> GlyphAndCPIndicesOfXInLine(const Font::LineData::CharVec& char_data, X x)
    {
        if (char_data.empty())
            return {CP0, CP0};
        const CPSize cd_size_cp{char_data.size()}; // must be >= CP1

        CPSize glyph_idx = CP0;
        for (; glyph_idx < cd_size_cp; ++glyph_idx) {
            const X curr_extent = char_data.at(Value(glyph_idx)).extent;

            if (x <= curr_extent) {
                // the point falls within the character at index retval
                const X prev_extent = (glyph_idx > CP0) ? char_data.at(Value(glyph_idx - CP1)).extent : X0;

                // if the point is more than halfway across the character, put the cursor *after* the character
                if ((prev_extent + curr_extent) <= x*2)
                    ++glyph_idx;
                break;
            }
        }

        glyph_idx = std::min(glyph_idx, cd_size_cp); // range check for safety

        CPSize cp_idx = CP0;
        if (glyph_idx == CP0) {
            cp_idx = CP0;
        } else if (glyph_idx < cd_size_cp) {
            cp_idx = char_data.at(Value(glyph_idx)).code_point_index;
        } else {
            const auto& last_glyph_data = char_data.at(Value(glyph_idx - CP1));
            cp_idx = last_glyph_data.code_point_index + CP1; // one code point past last glyph
        }
        return {glyph_idx, cp_idx};
    }

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
#  if defined(__cpp_lib_constexpr_vector)

    constexpr auto x_glyph_cp = []() {            //  01234567890123    // code points
        const std::string text(tagged_test_text); // "ab<i>cd</i>ef"    // raw text
                                                  //  01   23    45     // glyphs
        const auto elems = ElementsForTaggedText(text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), elems, 4u, dummy_next_fn);
        const auto& cd0 = line_data.front().char_data;

        const auto get_gly_cp = [&cd0](uint8_t x) {
            const auto temp = GlyphAndCPIndicesOfXInLine(cd0, X{x});
            return std::pair{static_cast<uint8_t>(temp.first), static_cast<uint8_t>(temp.second)};
        };

        std::array<uint8_t, 32> retval1{};
        std::array<uint8_t, 32> retval2{};
        for (uint8_t x_idx = 0u; x_idx < retval1.size(); ++x_idx)
            std::tie(retval1[x_idx], retval2[x_idx]) = get_gly_cp(x_idx);

        return std::pair{retval1, retval2};
    }();

    constexpr std::array<uint8_t, 32> glyphs_of_x_expected{0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4,
                                                                5,5,5,5, 6,6,6,6, 6,6,6,6, 6,6};
    static_assert(x_glyph_cp.first == glyphs_of_x_expected);

    constexpr std::array<uint8_t, 32> cps_of_x_expected{0,0, 1,1,1,1, 5,5,5,5, 6,6,6,6, 11,11,11,11,
                                                             12,12,12,12, 13,13,13,13, 13,13,13,13, 13,13};
    static_assert(x_glyph_cp.second == cps_of_x_expected);

#  endif
#endif
}

CPSize GG::GlyphIndexOfX(const Font::LineData::CharVec& char_data, X x, X offset)
{ return GlyphAndCPIndicesOfXInLine(char_data, x + offset).first; }

CPSize GG::GlyphIndexOfXOnLine0(const Font::LineVec& line_data, X x, X offset)
{ return line_data.empty() ? CP0 : GlyphAndCPIndicesOfXInLine(line_data.front().char_data, x + offset).first; }

CPSize GG::CodePointIndexOfX(const Font::LineData::CharVec& char_data, X x, X offset)
{ return GlyphAndCPIndicesOfXInLine(char_data, x + offset).second; }

CPSize GG::CodePointIndexOfXOnLine0(const Font::LineVec& line_data, X x, X offset)
{ return line_data.empty() ? CP0 : GlyphAndCPIndicesOfXInLine(line_data.front().char_data, x + offset).second; }


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

    auto Extract()
    {
        SetTextElementWidths(m_text, m_text_elements, m_font.GetGlyphs(), Value(m_font.SpaceWidth()));
        return std::pair(std::move(m_text), std::move(m_text_elements));
    }

    /** Return the constructed TextElements.*/
    const auto& Elements()
    {
        SetTextElementWidths(m_text, m_text_elements, m_font.GetGlyphs(), Value(m_font.SpaceWidth()));
        return m_text_elements;
    }

    /** Add an open tag iff it exists as a recognized tag.*/
    void AddOpenTag(std::string_view tag)
    {
        if (!tag_handler.IsKnown(tag))
            return;

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
        const auto begin = m_text.size();
        const auto end = m_text.append(text).size();
        m_text_elements.emplace_back(Substring{m_text, begin, end});
    }

    /** Add a white space element.*/
    void AddWhitespace(std::string_view whitespace)
    {
        auto begin = m_text.size();
        auto end = m_text.append(whitespace).size();
        m_text_elements.emplace_back(Substring(m_text, begin, end),
                                     Font::TextElement::TextElementType::WHITESPACE);
    }

    /** Add a newline element.*/
    void AddNewline()
    { m_text_elements.emplace_back(Font::TextElement::TextElementType::NEWLINE); }

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
                      const LineVec& line_data, RenderState& render_state) const
{
    RenderText(ul, lr, text, format, line_data, render_state, 0, CP0, line_data.size(),
               line_data.empty() ? CP0 : CPSize(line_data.back().char_data.size()));
}

void Font::RenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                      const LineVec& line_data, RenderState& render_state,
                      std::size_t begin_line, CPSize begin_char,
                      std::size_t end_line, CPSize end_char) const
{
    PreRenderText(ul, lr, text, format, line_data, render_state,
                  begin_line, begin_char, end_line, end_char, shared_cache);
    RenderCachedText(shared_cache);
}

void Font::PreRenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                         RenderCache& cache, const LineVec& line_data,
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
        std::array<GLubyte, 4u> retval{0, 0, 0, 255};
        const auto param_count = std::min(retval.size(), params.size());

#if defined(__cpp_lib_to_chars)
        for (std::size_t n = 0u; n < param_count; ++n) {
            const auto& param{params[n]};
            if (param.empty())
                return {retval, false};
            auto ec = std::from_chars(param.data(), param.data() + param.size(), retval[n]).ec;
            if (ec != std::errc())
                return {retval, false};
        }
#else
        for (std::size_t n = 0u; n < param_count; ++n) {
            const auto& param{params[n]};
            if (param.empty())
                return {retval, false};
            retval[n] = CharsToUInt8(param);
        }
#endif

        return {retval, true};
    }

    void HandleTag(const Font::TextElement& tag, Font::RenderState& render_state)
    {
        if (tag.tag_name == Font::ITALIC_TAG) {
            if (tag.IsCloseTag()) {
                if (render_state.use_italics)
                    --render_state.use_italics;
            } else {
                ++render_state.use_italics;
            }
        } else if (tag.tag_name == Font::UNDERLINE_TAG) {
            if (tag.IsCloseTag()) {
                if (render_state.draw_underline)
                    --render_state.draw_underline;
            } else {
                ++render_state.draw_underline;
            }
        } else if (tag.tag_name == Font::SHADOW_TAG) {
            if (tag.IsCloseTag()) {
                if (render_state.use_shadow)
                    --render_state.use_shadow;
            } else {
                ++render_state.use_shadow;
            }
        } else if (tag.tag_name == Font::SUPERSCRIPT_TAG) {
            if (tag.IsCloseTag())
                --render_state.super_sub_shift;
            else
                ++render_state.super_sub_shift;

        } else if (tag.tag_name == Font::SUBSCRIPT_TAG) {
            if (tag.IsCloseTag())
                ++render_state.super_sub_shift;
            else
                --render_state.super_sub_shift;

        } else if (tag.tag_name == Font::RGBA_TAG) {
            if (tag.IsCloseTag()) {
                // Popping is ok also for an empty color stack.
                render_state.PopColor();

            } else {
                auto [color, well_formed_tag] = TagParamsToColor(tag.params);
                if (well_formed_tag) {
                    glColor4ubv(color.data());
                    render_state.PushColor(color[0], color[1], color[2], color[3]);
                }
                /*else {
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
                }*/
            }
        } else if (tag.tag_name == Font::RESET_TAG) {
            render_state.Reset();
        }
    }
}


void Font::PreRenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                         const LineVec& line_data, RenderState& render_state,
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
        const auto& line = line_data.at(i);
        if (line.Empty())
            continue;
        const auto& line_char_data = line.char_data;
        const CPSize cd_size{line_char_data.size()};

        const X x_origin = LineOriginX(ul.x, lr.x, line.Width(), line.justification);
        X x = x_origin;

        const Y y = LinePosY(y_origin, i, begin_line, m_lineskip);

        const CPSize start = (i != begin_line) ? CP0 : std::min(begin_char, cd_size - CP1);
        const CPSize end = (i != end_line - 1) ? cd_size : std::min(end_char, cd_size);


        for (CPSize j = start; j < end; ++j) {
            const auto& glyph = line_char_data.at(Value(j));
            for (const auto& tag : glyph.tags)
                HandleTag(tag, render_state);
            const uint32_t c = get_next_char(Value(glyph.string_index));

            if (c == WIDE_NEWLINE)
                continue;
            const auto it = m_glyphs.find(c);
            if (it == m_glyphs.end())
                x = x_origin + glyph.extent; // move forward to the right-side extent of the character when a whitespace or unprintable glyph is requested
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

void Font::ProcessLineTagsBefore(const LineData::CharVec& char_data,
                                 RenderState& render_state, CPSize end_char)
{
    const auto end_char_idx = std::min(Value(end_char), char_data.size());
    for (std::size_t j = 0; j < end_char_idx; ++j)
        for (auto& tag : char_data.at(j).tags)
            HandleTag(tag, render_state);
}

void Font::ProcessLineTags(const LineData::CharVec& char_data, RenderState& render_state)
{ ProcessLineTagsBefore(char_data, render_state, static_cast<CPSize>(char_data.size())); }

void Font::ProcessTagsBefore(const LineVec& line_data, RenderState& render_state,
                             std::size_t begin_line, CPSize begin_char)
{
    if (line_data.empty())
        return;
    begin_line = std::min(begin_line, line_data.size());

    for (std::size_t i = 0; i <= begin_line; ++i) {
        const auto& char_data = line_data.at(i).char_data;
        const CPSize cd_size{char_data.size()};
        const auto line_end_char = (i == begin_line && begin_char <= cd_size) ? begin_char : cd_size;
        ProcessLineTagsBefore(char_data, render_state, line_end_char);
    }
}

void Font::ProcessTags(const LineVec& line_data, RenderState& render_state)
{ ProcessTagsBefore(line_data, render_state, std::numeric_limits<std::size_t>::max(), std::numeric_limits<CPSize>::max()); }

std::string Font::StripTags(std::string_view text)
{
    using namespace boost::xpressive;
    std::string text_str{text};

    auto& regex = regex_with_tags.BindRegexToText(text_str, false);

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

Pt Font::TextExtent(const LineVec& line_data) const noexcept
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
                            const X box_width, const Font::LineVec& line_data)
    {
        std::cout << "Font::DetermineLines(text=\"" << text << "\" (@ "
                  << static_cast<const void*>(text.c_str()) << ") format="
                  << format << " box_width=" << Value(box_width) << ")" << std::endl;

        std::cout << "Line breakdown:\n";
        for (std::size_t i = 0; i < line_data.size(); ++i) {
            auto& char_data = line_data.at(i).char_data;

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
                for (auto& tag_elem : char_data.at(j).tags) {
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
            std::cout << "    justification=" << line_data.at(i).justification << "\n" << std::endl;
        }
    }
}

std::vector<Font::TextElement>
Font::ExpensiveParseFromTextToTextElements(const std::string& text, const Flags<TextFormat> format,
                                           const GlyphMap& glyphs, int8_t space_width)
{
    std::vector<TextElement> text_elements;

    using namespace boost::xpressive;
#if defined(__cpp_using_enum)
    using enum TextElement::TextElementType;
#else
    static constexpr auto OPEN_TAG = TextElement::TextElementType::OPEN_TAG;
    static constexpr auto CLOSE_TAG = TextElement::TextElementType::CLOSE_TAG;
    static constexpr auto WHITESPACE = TextElement::TextElementType::WHITESPACE;
    static constexpr auto NEWLINE = TextElement::TextElementType::NEWLINE;
#endif

    if (text.empty())
        return text_elements;

    const bool ignore_tags = format & FORMAT_IGNORETAGS;

    // Fetch and use the regular expression from the TagHandler which parses all the known XML tags.
    const sregex& regex = regex_with_tags.BindRegexToText(text, ignore_tags);
    sregex_iterator it(text.begin(), text.end(), regex);
    const sregex_iterator end_it;

    for (; it != end_it; ++it) {
        const auto& it_elem = *it;

        if (it_elem[text_tag_idx].matched) {
            auto matched_text = Substring(text, it_elem[text_tag_idx]);
            if (!matched_text.empty())
                text_elements.emplace_back(matched_text); // Basic text element.

        } else if (it_elem[open_bracket_tag_idx].matched) {
            // Open XML tag.
            Substring text_substr{text, it_elem[full_regex_tag_idx]};
            Substring tag_name_substr{text, it_elem[tag_name_tag_idx]};

            // Check open tags for submatches which are parameters.
            // For example, a color tag might have RGB parameters.
            const auto& nested_results{it_elem.nested_results()};
            std::vector<Substring> params;
            if (1 < nested_results.size()) {
                params.reserve(nested_results.size() - 1);
                for (auto nested_it = std::next(nested_results.begin()); nested_it != nested_results.end(); ++nested_it)
                {
                    const auto& nested_elem = *nested_it;
                    params.emplace_back(text, nested_elem[full_regex_tag_idx]);
                }
            }

            text_elements.emplace_back(text_substr, tag_name_substr, std::move(params), OPEN_TAG);

        } else if (it_elem[close_bracket_tag_idx].matched) {
            // Close XML tag
            Substring text_substr{text, it_elem[full_regex_tag_idx]};
            Substring tag_name_substr{text, it_elem[tag_name_tag_idx]};

            text_elements.emplace_back(text_substr, tag_name_substr, CLOSE_TAG);

        } else if (it_elem[whitespace_tag_idx].matched) {
            // Whitespace element
            Substring text_substr{text, it_elem[full_regex_tag_idx]};
            const auto& ws_elem = text_elements.emplace_back(text_substr, WHITESPACE);
            const char last_char = ws_elem.text.empty() ? static_cast<char>(0) : *std::prev(ws_elem.text.end());

            // If the last character of a whitespace element is a line ending then create a
            // newline TextElement.
            if (last_char == '\n' || last_char == '\f' || last_char == '\r')
                text_elements.emplace_back(NEWLINE);
        }
    }

    // fill in the widths of code points in each TextElement
    SetTextElementWidths(text, text_elements, glyphs, space_width);

#if DEBUG_DETERMINELINES
    DebugOutput::PrintParseResults(text_elements);
#endif
    return text_elements;
}

void Font::ChangeTemplatedText(std::string& text, std::vector<TextElement>& text_elements,
                               const std::string& new_text, std::size_t targ_offset,
                               const GlyphMap& glyphs, uint8_t space_width)
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
        if (te_it->Type() == Font::TextElement::TextElementType::TEXT) {
            // Change the target text element
            if (targ_offset == curr_offset) {
                // Change text
                auto ii_sub_begin = static_cast<std::size_t>(te_it->text.begin() - text.begin());
                auto sub_len = static_cast<std::size_t>(te_it->text.end() - te_it->text.begin());
                text.erase(ii_sub_begin, sub_len);
                text.insert(ii_sub_begin, new_text); // C++20 constexpr

                change_of_len = static_cast<decltype(change_of_len)>(new_text.size() - sub_len);
                te_it->text = Font::Substring(text, ii_sub_begin, ii_sub_begin + new_text.size());
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
            te_it->text = Font::Substring(text, ii_sub_begin + change_of_len, ii_sub_end + change_of_len);

            ++te_it;
        }
    }

    SetTextElementWidths(text, text_elements, start_of_reflow, glyphs, space_width);
}


#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
namespace {
#  if defined(__cpp_lib_constexpr_vector)
    constexpr auto lines_and_lengths = []() {
        const std::string test_text(TEST_TEXT_WITH_TAGS);
        const auto text_elems = TestTextElems(test_text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), text_elems, 4u, dummy_next_fn);

        return std::array<std::size_t, 3>{line_data.size(), line_data.at(0).char_data.size(), line_data.at(1).char_data.size()};
    }();
    static_assert(lines_and_lengths == std::array<std::size_t, 3>{2, 30, 13});

    constexpr auto test_text_tags_line0 = []() {
        const std::string test_text(TEST_TEXT_WITH_TAGS);
        const auto text_elems = TestTextElems(test_text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), text_elems, 4u, dummy_next_fn);

        const auto& cd0 = line_data.front().char_data;
        std::array<char, 30> tag_names0{0};
        for (std::size_t char_idx = 0u; char_idx < std::min(tag_names0.size(), cd0.size()); ++char_idx) {
            const auto& tags = cd0.at(char_idx).tags;
            if (!tags.empty())
                if (!tags.front().tag_name.empty())
                    tag_names0[char_idx] = tags.front().tag_name.data()[0];
        }
        return tag_names0;
    }();
    static_assert(test_text_tags_line0.size() == 30);

    static_assert(std::array<std::array<int, 2>, 3>{0} == std::array<std::array<int, 2>, 3>{{{0,0}, {0,0}, {0,0}}});

    constexpr auto test_text_tags_line1 = []() {
        const std::string test_text(TEST_TEXT_WITH_TAGS);
        const auto text_elems = TestTextElems(test_text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), text_elems, 4u, dummy_next_fn);

        const auto& cd1 = line_data.at(1).char_data;
        std::array<std::array<char, 2>, 13> tag_names1{{{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}}};
        for (std::size_t char_idx = 0u; char_idx < std::min(tag_names1.size(), cd1.size()); ++char_idx) {
            const auto& tags = cd1.at(char_idx).tags;
            if (!tags.empty()) {
                if (!tags.front().tag_name.empty())
                    tag_names1[char_idx][0] = tags.front().tag_name.data()[0];
                if (tags.size() > 1) {
                    if (!tags.back().tag_name.empty())
                        tag_names1[char_idx][1] = tags.back().tag_name.data()[0];
                }
            }
        }
        return tag_names1;
    }();

    constexpr std::array<std::array<char, 2>, 13> test_text_tags_line1_expected{{
        {'u',0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {'i','s'}, {0,0}}};
    static_assert(test_text_tags_line1.size() == test_text_tags_line1_expected.size());
    static_assert(test_text_tags_line1 == test_text_tags_line1_expected);

    constexpr auto test_text_str_idxs_chars = []() {
        const std::string test_text(TEST_TEXT_WITH_TAGS);
        const auto text_elems = TestTextElems(test_text);
        const auto fmt = FORMAT_LEFT | FORMAT_TOP;
        const auto line_data = AssembleLineData(fmt, GG::X(99999), text_elems, 4u, dummy_next_fn);

        const auto& cd0 = line_data.at(0).char_data;
        const auto& cd1 = line_data.at(1).char_data;

        std::array<StrSize, 30 + 13> test_text_str_idxs{S0};
        std::array<char, 30 + 13 + 1> test_text_chars{0};
        std::size_t out_idx = 0;

        for (std::size_t char_data_idx = 0u; char_data_idx < cd0.size(); ++char_data_idx) {
            const auto str_idx = cd0.at(char_data_idx).string_index;
            test_text_str_idxs[out_idx] = str_idx;
            test_text_chars[out_idx++] = test_text.at(Value(str_idx));
        }

        for (std::size_t char_data_idx = 0u; char_data_idx < cd1.size(); ++char_data_idx) {
            const auto str_idx = cd1.at(char_data_idx).string_index;
            test_text_str_idxs[out_idx] = str_idx;
            test_text_chars[out_idx++] = test_text.at(Value(str_idx));
        }

        return std::pair(test_text_str_idxs, test_text_chars);
    }();
    constexpr std::string_view test_text_chars(test_text_str_idxs_chars.second.data(), 43);
    static_assert(test_text_chars.size() == 43 && test_text_chars == "defaultital_ul_it_   _just_ul_second lineis");

    constexpr auto test_text_str_idxs = test_text_str_idxs_chars.first;
    static_assert(test_text_str_idxs.size() == 43);
    constexpr auto idxs_expected = []() {
        std::array<uint8_t, test_text_str_idxs.size()> temp{
             0,1,2,3,4,5,6,             // default
             10,11,12,13,               // ital
             17,18,19,20,21,22,23,      // _ul_ital_
             28,29,30,                  // "   "
             31,32,33,34,35,36,37,38,39,// _just_ul_
             45,46,47,48,49,50,         // second
             51,                        // " "
             52,53,54,55,               // line
             64,65                      // is
        };
        std::array<StrSize, test_text_str_idxs.size()> rv{};
        for (std::size_t idx = 0; idx < rv.size(); ++idx)
            rv[idx] = StrSize(temp[idx]);
        return rv;
    }();
    static_assert(static_cast<decltype(test_text_str_idxs)>(idxs_expected) == test_text_str_idxs);
#  endif
}
#endif

Font::LineVec Font::DetermineLines(
    const std::string& text, Flags<TextFormat> format, X box_width,
    const std::vector<TextElement>& text_elements) const
{
    // HACK - Workaround for #2166
    // On OSX, right clicking an unowned planet at game start may result in utf8::invalid_utf8 or utf8::not_enough_room
    if (!utf8::is_valid(text.begin(), text.end())) {
        std::cerr << "Invalid UTF8 in text: " << text;
        return {};
    }

 #if DEBUG_DETERMINELINES
    auto line_data = AssembleLineData(format, box_width, text_elements, m_space_width);
    DebugOutput::PrintLineBreakdown(text, format, box_width, line_data);
    return line_data;
#else
    return AssembleLineData(format, box_width, text_elements, m_space_width);
#endif
}


StrSize GG::StringIndexOfLineAndGlyph(std::size_t line, CPSize index, const Font::LineVec& line_data)
{ return StringIndexFromLineAndGlyphInLines(line, index, line_data).first; }

namespace {
    StrSize StringIndexOfCodePointInLines(CPSize index, const Font::LineVec& line_data)
    { return S0; } // TODO!
}

StrSize GG::StringIndexOfCodePoint(CPSize index, const Font::LineVec& line_data)
{ return StringIndexOfCodePointInLines(index, line_data); }

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

bool Font::IsDefaultFont() const noexcept
{ return m_font_filename == StyleFactory::DefaultFontName(); }

std::shared_ptr<Font> Font::GetDefaultFont(unsigned int pts)
{ return GUI::GetGUI()->GetStyleFactory().DefaultFont(pts); }


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
