//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Font.h
//!
//! Contains the Font class, a class that encapsulates the rendering of
//! a single FreeType-compatible font in italics, with underlining, left-,
//! right-, or center- justified, etc., and the FontManager class which
//! provides application-wide management of Font objects.

#ifndef _GG_Font_h_
#define _GG_Font_h_


#include <memory>
#include <stack>
#include <boost/unordered_map.hpp>
#include <GG/AlignmentFlags.h>
#include <GG/GLClientAndServerBuffer.h>
#include <GG/FontFwd.h>
#include <GG/Texture.h>
#include <GG/UnicodeCharsets.h>


struct FT_FaceRec_;
typedef struct FT_FaceRec_*  FT_Face;
typedef int FT_Error;

namespace GG {

class GLRGBAColorBuffer;
class GLTexCoordBuffer;
class GL2DVertexBuffer;

/** Returns a string of the form "<rgba r g b a>" from a Clr object with color
    channels r, b, g, a. */
GG_API std::string RgbaTag(Clr c);

#if defined(__cpp_lib_constexpr_vector) && defined(__cpp_lib_constexpr_string)
#  define CONSTEXPR_FONT constexpr
#else
#  define CONSTEXPR_FONT
#endif

/** \brief A bitmapped font rendering class.

    Font creates one or more 16-bits-per-pixel OpenGL textures that contain
    rendered glyphs from a requested font file at the requested point size,
    including only the requested ranges of code points.  Once the textures
    have been created, text is rendered to the display by rendering quads
    textured with portions of the glyph textures.  The glyphs are rendered to
    the textures in white, with alpha blending used for antialiasing.  The
    user should set the desired text color with a call to glColor*() before
    any call to RenderText().  When text is rendered, DetermineLines() is
    called to determine where the line breaks are, so that text can be
    rendered centered, right-justified, or whatever.  To cut down on this
    computation, when the text is not changing very rapidly (ie not every
    frame), DetermineLines() can be called by the user once, and the result
    supplied to RenderText() repeatedly.  When this is done, the iteration
    through the text to determine line breaks is not necessary at render time.
    The user is responsible for ensuring that the line data applies to the
    text string supplied to RenderText().  See UnicodeCharsets.h for the
    ranges of code points available, including a function that allow one to
    determine which ranges are necessary for rendering a certain string.
    Point sizes above 250 are not supported.  Note that users should not
    normally need to use Font directly.  Users should instead use TextControl,
    Edit, or MultiEdit.

    <h3>Text Formatting Tags</h3>

    GG::Font supports a few text formatting tags for convenience.  These tags
    are similar to HTML or XML tags; there is an opening version "<tag>" and a
    closing version "</tag>" of each tag.  Tags can be nested.  For instance,
    consider the use of the italics tag \<i> here:

    \verbatim
      <i>some text <i>and </i>some more </i>text \endverbatim

    In this example, everything is italicized except for "text".  Each \<i>
    tag establishes that italics should be used for all further text until the
    next \a matching \</i> tag.  The first \<i> tag matches the second \</i>
    tag, and the two inner tags are matched.  Note that unmatched close-tags
    (e.g. \</i>) are ignored by the text parser Font uses to find tags -- they
    will appear as regular text.  The text justification tags are used on a
    per-line basis, since it makes no sense to, for instance, right-justify
    only a part of a line and center the rest. When more than one
    justification tag appears on a line, the last one is used.  A
    justification close-tag indicates that a line is to be the last one with
    that justification, and only applies if that justification is active.

    <br>The supported tags are:
    - \verbatim<i></i> \endverbatim                 Italics
    - \verbatim<u></u> \endverbatim                 Underline
    - \verbatim<s></s> \endverbatim                 Shadow
    - \verbatim<rgba r g b a></rgba> \endverbatim   Color. Sets current rendering color to that specified by parameters.  Parameters may be either floating point values in the range [0.0, 1.0], or integer values in the range [0, 255].  All parameters must be in one format or the other.  The \</rgba> tag restores the previously set \<rgba> color, or restores the default color used to render the text when there are no other \<rgba> tags in effect.  Example tag: \<rgba 0.4 0.5 0.6 0.7>.
    - \verbatim<left></left> \endverbatim           Left-justified text.
    - \verbatim<center></center> \endverbatim       Centered text.
    - \verbatim<right></right> \endverbatim         Right-justified text.
    - \verbatim<pre></pre> \endverbatim             Preformatted.  Similar to HTML \<pre> tag, except this one only causes all tags to be ignored until a subsequent \</pre> tag is seen.  Note that due to their semantics, \<pre> tags cannot be nested.

    <p>Users of Font may wish to create their own tags as well.  Though Font
    will know nothing about the semantics of the new tags, it is possible to
    let Font know about them, in order for Font to render them invisible as it
    does with the tags listed above.  See the static methods
    RegisterKnownTags(), RemoveKnownTag(), and ClearKnownTags() for details.
    It is not possible to remove the built-in tags using these methods.  If
    you wish not to use tags at all, call DetermineLines() and RenderText()
    with the format parameter containing FORMAT_IGNORETAGS, or include a
    \<pre> tag at the beginning of the text to be rendered.
   */
class GG_API Font
{
public:
    static constexpr std::string_view ITALIC_TAG = "i";
    static constexpr std::string_view SHADOW_TAG = "s";
    static constexpr std::string_view UNDERLINE_TAG = "u";
    static constexpr std::string_view SUPERSCRIPT_TAG = "sup";
    static constexpr std::string_view SUBSCRIPT_TAG = "sub";
    static constexpr std::string_view RGBA_TAG = "rgba";
    static constexpr std::string_view ALIGN_LEFT_TAG = "left";
    static constexpr std::string_view ALIGN_CENTER_TAG = "center";
    static constexpr std::string_view ALIGN_RIGHT_TAG = "right";
    static constexpr std::string_view PRE_TAG = "pre";
    static constexpr std::string_view RESET_TAG = "reset";

    /** \brief A range of iterators into a std::string that defines a
        substring found in a string being rendered by Font.

        Substring is bound to a particular instance of a std::string.  If
        that particular std::string goes out of scope or is deleted then
        behavior is undefined, but may seg fault with the next access. */
    class GG_API Substring
    {
    public:
        using IterPair = std::pair<std::string::const_iterator, std::string::const_iterator>;

        CONSTEXPR_FONT explicit Substring(const std::string& str_) noexcept :
            str(&str_)
        {}
        constexpr explicit Substring(const std::string* str_) noexcept :
            str(str_)
        {}

        /** Construction from two offsets. \a first_ must be <= \a second_. */
        constexpr Substring(const std::string* str_, uint32_t first_, uint32_t second_) noexcept :
            str(str_),
            first(first_),
            second(second_)
        {
            assert(0 <= first_);
            assert(first_ <= second_);
            assert(second_ <= str->size());
        }
        template <typename T, std::enable_if_t<std::is_unsigned_v<T>>* = nullptr>
        CONSTEXPR_FONT Substring(const std::string& str_, T first_, T second_) noexcept :
            Substring(&str_, static_cast<uint32_t>(first_), static_cast<uint32_t>(second_))
        {}

        CONSTEXPR_FONT Substring(const std::string& str_, std::ptrdiff_t first_, std::ptrdiff_t second_) noexcept :
            Substring(str_, static_cast<uint32_t>(first_), static_cast<uint32_t>(second_))
        {}

        /** Construction from two iterators. \a first_ must be <= \a second_.
          * Both must be valid iterators into \a str_. */
        CONSTEXPR_FONT Substring(const std::string& str_,
                                 std::string::const_iterator first_,
                                 std::string::const_iterator second_) :
            Substring(str_,
                      static_cast<uint32_t>(std::distance(str_.begin(), first_)),
                      static_cast<uint32_t>(std::distance(str_.begin(), second_)))
        {}
        CONSTEXPR_FONT Substring(const std::string& str_, const IterPair& pair) :
            Substring(str_, pair.first, pair.second)
        {}


        /** Attach this Substring to \p str_.
          * This changes any future-returned iterators from pointing into the previously-bound
          * string to pointing into \p str_. */
        CONSTEXPR_FONT void Bind(const std::string& str_) noexcept
        {
            assert(std::distance(str_.begin(), str_.end()) >= second);
            str = &str_;
        }

        [[nodiscard]] CONSTEXPR_FONT auto data() const noexcept -> const std::string::value_type*
        { return (str && (str->size() >= first)) ? (str->data() + first) : EMPTY_STRING.data(); }

        /** Returns an iterator to the beginning of the substring. */
        [[nodiscard]] CONSTEXPR_FONT auto begin() const noexcept -> std::string::const_iterator
        { return (str && str->size() >= first) ? (str->cbegin() + first) : EMPTY_STRING.cbegin(); }

        /** Returns an iterator to one-past-the-end of the substring. */
        [[nodiscard]] CONSTEXPR_FONT auto end() const noexcept -> std::string::const_iterator
        { return (str && str->size() >= second) ? (str->cbegin() + second) : EMPTY_STRING.cend(); }

        /** True iff .first == .second. */
        [[nodiscard]] constexpr bool empty() const noexcept { return first == second; }

        [[nodiscard]] CONSTEXPR_FONT bool IsDefaultEmpty() const noexcept { return str == &EMPTY_STRING; }

        /** Length, in original string chars, of the substring. */
        [[nodiscard]] constexpr std::size_t size() const noexcept { return static_cast<std::size_t>(second - first); }
        [[nodiscard]] constexpr auto offsets() const noexcept { return std::pair<uint32_t, uint32_t>{first, second}; }

        /** Implicit conversion to std::string. */
        [[nodiscard]] CONSTEXPR_FONT operator std::string() const { return std::string(begin(), end()); }
        static constexpr bool op_sv_nox = noexcept(std::string_view{(const char*)(nullptr), std::size_t{}});
        [[nodiscard]] CONSTEXPR_FONT operator std::string_view() const noexcept(op_sv_nox) { return {data(), size()}; }

        /** Comparison with std::string. */
        CONSTEXPR_FONT bool operator==(const std::string& rhs) const
        { return size() == rhs.size() && std::string_view(*this) == rhs; }

        CONSTEXPR_FONT bool operator==(std::string_view rhs) const
        { return size() == rhs.size() && std::string_view(*this) == rhs; }

        CONSTEXPR_FONT bool operator==(const Substring& rhs) const
        { return size() == rhs.size() && std::string_view(*this) == std::string_view(rhs); }

        /** Concatenation with base.  \a rhs.first must be <= \a rhs.second.
          * .second must be equal to \a rhs.first (*this and \a rhs must be contiguous). */
        CONSTEXPR_FONT Substring& operator+=(const IterPair& rhs)
        {
            assert(rhs.first <= rhs.second);
            assert(std::distance(str->begin(), rhs.first) == second);
            second = static_cast<decltype(second)>(std::distance(str->begin(), rhs.second));
            return *this;
        }

        CONSTEXPR_FONT Substring() noexcept = default;

#if defined(__cpp_lib_constexpr_string) && defined(_MSC_VER) && (_MSC_VER >= 1934)
        static constexpr std::string EMPTY_STRING;
#else
        static const std::string EMPTY_STRING;
#endif

    private:
        const std::string* str = &EMPTY_STRING;
        uint32_t first = 0;
        uint32_t second = 0;
    };

    /** \brief Describes a token-like piece of text to be rendered. */
    struct GG_API TextElement
    {
        /** The types of token-like entities that can be represented by a TextElement. */
        enum class TextElementType : uint8_t {
            OPEN_TAG,   ///< An opening text formatting tag (e.g. "<rgba 0 0 0 255>").
            CLOSE_TAG,  ///< A closing text formatting tag (e.g. "</rgba>").
            TEXT,       ///< Some non-whitespace text (e.g. "The").
            WHITESPACE, ///< Some whitespace text (e.g. "  \n").

            /** The newline code point when it is encountered in a rendered
                string, though they do not contain the actual newline character
                so their \a text members are always ""). */
            NEWLINE
        };

        CONSTEXPR_FONT explicit TextElement(TextElementType type_) noexcept :
            type(type_)
        {}
        CONSTEXPR_FONT explicit TextElement(Substring text_) noexcept(noexcept(Substring{std::declval<Substring>()})) :
            text(text_),
            type(TextElementType::TEXT)
        {}

        CONSTEXPR_FONT TextElement(Substring text_, TextElementType type_)
            noexcept(noexcept(Substring{std::declval<Substring>()})) :
            text(text_),
            type(type_)
        {}
        CONSTEXPR_FONT TextElement(Substring text_, Substring tag_name_,
                    TextElementType type_) noexcept(noexcept(Substring{std::declval<Substring>()})) :
            text(text_),
            tag_name(tag_name_),
            type(type_)
        {}
        CONSTEXPR_FONT TextElement(Substring text_, Substring tag_name_, std::vector<Substring> params_,
                    TextElementType type_) noexcept(noexcept(Substring{std::declval<Substring>()})) :
            text(text_),
            tag_name(tag_name_),
            params(std::move(params_)),
            type(type_)
        {}

        /** Attach this TextElement to the string \p whole_text, by
            attaching the SubString data member text to \p whole_text.

            Binding to a new \p whole_text is very fast compared to
            re-parsing the entire \p whole_text and allows TextElements of
            TextElementType TEXT to be changed quickly if it is known that
            the parse would be the same.

            It is efficient when you want to do a text parse or layout
            once, and then create several different controls that have the
            same text layout and text contents, but each of them needs to
            keep there own internal copy of the text. So, while the pointer
            diffs are all the same, since the text contents are the same,
            the pointer to the string in each TextElement (and its
            contained Substring) needs to be set to the appropriate copy of
            the string.

            This is used in TextControl and its derived classes to re-use
            entire vectors of TextElement with different std::strings
            without re-parsing the std::string.
         */
        CONSTEXPR_FONT void Bind(const std::string& whole_text) noexcept
        {
            text.Bind(whole_text);
            tag_name.Bind(whole_text);
            for (Substring& substring : params)
                substring.Bind(whole_text);
        }

        /** Returns the TextElementType of the element. */
        [[nodiscard]] CONSTEXPR_FONT TextElementType Type() const noexcept { return type; };
        [[nodiscard]] CONSTEXPR_FONT bool IsCloseTag() const noexcept { return type == TextElementType::CLOSE_TAG; }
        [[nodiscard]] CONSTEXPR_FONT bool IsOpenTag() const noexcept { return type == TextElementType::OPEN_TAG; }
        [[nodiscard]] CONSTEXPR_FONT bool IsTag() const noexcept { return IsCloseTag() || IsOpenTag(); }
        [[nodiscard]] CONSTEXPR_FONT bool IsWhiteSpace() const noexcept { return type == TextElementType::WHITESPACE; }
        [[nodiscard]] CONSTEXPR_FONT bool IsNewline() const noexcept { return type == TextElementType::NEWLINE; }

        /** Returns the width of the element. */
        [[nodiscard]] CONSTEXPR_FONT X Width() const
        {
            if (cached_width == -X1)
                cached_width = [](const auto& widths) -> X {
                    X rv = X0;
                    for (const auto& w : widths)
                        rv += w;
                    return rv;
                }(widths);
            return cached_width;
        }


        /* Returns the number of characters in the original string that the
           element represents. */
        [[nodiscard]] CONSTEXPR_FONT StrSize StringSize() const noexcept
        { return StrSize(text.size()); }

        /** Returns the number of code points in the original string that the
            element represents. */
        [[nodiscard]] CONSTEXPR_FONT CPSize CodePointSize() const noexcept
        { return CPSize(widths.size()); }

        bool operator==(const TextElement &rhs) const noexcept // ignores cached_width
        {
            return (type == rhs.type) &&
                   (text == rhs.text) &&
                   (tag_name == rhs.tag_name) &&
                   (widths == rhs.widths) &&
                   (params == rhs.params);
        }

        /** The text from the original string represented by the element. */
        Substring text;

        /** For a formatting tag, contains the tag name, eg. "rgba" for the tag "<rgba 0 0 0 255>". */
        Substring tag_name;

        /** The widths of the glyphs in \a text. */
        std::vector<int8_t> widths;

        /** The parameter strings within the tag, eg. "0", "0", "0", and "255"
            for the tag "<rgba 0 0 0 255>". */
        std::vector<Substring> params;

        TextElementType type = TextElementType::TEXT;

    protected:
        CONSTEXPR_FONT TextElement() = default;

    private:
        mutable X cached_width{-X1};
    };

    /** \brief TextAndElementsAssembler is used to assemble a matched pair of text and a vector of
        TextElement, without incurring the computational cost of parsing the text with
        ExpensiveParseFromTextToTextElements().

        The pair of string and vector returned by Text() and Elements() are consistent with each
        other and can be used with the fast constructor or the fast SetText variant of TextControl.
    */
    class GG_API TextAndElementsAssembler
    {
    public:
        explicit TextAndElementsAssembler(const Font& font);
        TextAndElementsAssembler(const Font& font, std::size_t text_capacity, std::size_t elements_capacity);
        ~TextAndElementsAssembler(); // needed for unique_ptr<Impl>

        /** Return the constructed text.*/
        [[nodiscard]] const std::string& Text() const noexcept;
        /** Return the constructed TextElements.*/
        [[nodiscard]] const std::vector<TextElement>& Elements() const;

        /** Destructively extract and return the constructed text and elements */
        [[nodiscard]] std::pair<std::string, std::vector<TextElement>> Extract();

        /** Add an open tag iff it exists as a recognized tag.*/
        TextAndElementsAssembler& AddOpenTag(std::string_view tag);
        /** Add an open tag iff it exists as a recognized tag.*/
        TextAndElementsAssembler& AddOpenTag(std::string_view tag, const std::vector<std::string>& params);
        /** Add a close tag iff it exists as a recognized tag.*/
        TextAndElementsAssembler& AddCloseTag(std::string_view tag);
        /** Add a text element.  Any whitespace in this text element will be non-breaking.*/
        TextAndElementsAssembler& AddText(std::string_view text);
        TextAndElementsAssembler& AddText(const char* text) { return AddText(std::string_view(text)); }
        TextAndElementsAssembler& AddText(std::string&& text);
        /** Add a white space element.*/
        TextAndElementsAssembler& AddWhitespace(std::string_view whitespace);
        /** Add a new line element.*/
        TextAndElementsAssembler& AddNewline();

        /** Add an open Clr tag.*/
        TextAndElementsAssembler& AddOpenTag(Clr color);

    private:
        class Impl;
        std::unique_ptr<Impl> const m_impl;
    };

    /** \brief Holds the essential data on each line that a string occupies when
        rendered with given format flags.

        \a char_data contains the visible glyphs for each line, plus any text
        formatting tags present on that line as well. */
    struct GG_API LineData
    {
        CONSTEXPR_FONT LineData() noexcept = default;
        CONSTEXPR_FONT explicit LineData(Alignment justification_) noexcept :
            justification(justification_)
        {}

        /** \brief Contains the extent, the index into the original string,
            and the text formatting tags that should be applied before
            rendering of a visible glyph. */
        struct GG_API CharData
        {
            CONSTEXPR_FONT CharData() = default;

            CONSTEXPR_FONT CharData(X extent_, StrSize str_index, StrSize str_size, CPSize cp_index,
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

            /** The furthest-right extent of this glyph as it appears on the line. */
            X extent = X0;

            /** The position in the original string of the first character of this glyph. */
            StrSize string_index = S0;

            /** The size in the original string of the characters that make up this glyph. */
            StrSize string_size = S0;

            /** The code point index of this glyph. */
            CPSize code_point_index = CP0;

            /** The text formatting tags that should be applied before rendering this glyph. */
            std::vector<TextElement> tags;
        };

        CONSTEXPR_FONT X    Width() const noexcept { return char_data.empty() ? X0 : char_data.back().extent; }
        CONSTEXPR_FONT bool Empty() const noexcept { return char_data.empty(); }

        /** Data on each individual glyph. */
        using CharVec = std::vector<CharData>;
        CharVec char_data;

        /** FORMAT_LEFT, FORMAT_CENTER, or FORMAT_RIGHT; derived from text
            format flags and/or formatting tags in the text. */
        Alignment justification = ALIGN_CENTER;
    };
    using LineVec = std::vector<LineData>;


    /** \brief Holds the state of tags during rendering of text.

        By keeping track of this state across multiple calls to RenderText(),
        the user can preserve the functionality of the text formatting tags,
        if present. */
    struct GG_API RenderState
    {
        explicit RenderState(Clr color) : //< Takes default text color as parameter
            color_stack{std::stack<Clr>::container_type{color}}
        {}

        RenderState() : RenderState(Clr{0, 0, 0, 0}) {}

        /** The count of open \<i> tags seen since the last \</i> seen. */
        uint8_t use_italics = 0;

        /** The count of open \<s> tags seen since the last \</s> seen. */
        uint8_t use_shadow = 0;

        /** The count of open \<u> tags seen since the last \</u> seen. */
        uint8_t draw_underline = 0;

        /** The count of open \<super> (positive) minus \<sub> tags seen. */
        int8_t super_sub_shift = 0;

        /** The stack of text color indexes (as set by previous tags). */
        std::stack<Clr> color_stack;

        /// Add color to stack and remember it has been used
        void PushColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
        {
            // The same color may end up being stored multiple times, but the cost of
            // deduplication is greater than the cost of just letting it be so.
            color_stack.emplace(r, g, b, a);
        }
        void PushColor(Clr clr) { color_stack.push(std::move(clr)); };

        /// Return to the previous used color, or remain as default
        void PopColor() noexcept(noexcept(color_stack.size()) && noexcept(color_stack.pop()))
        {
            // Never remove the initial color from the stack
            if (color_stack.size() > 1)
                color_stack.pop();
        }

        // Revert to no tags and initial color
        void Reset() noexcept(noexcept(color_stack.size()) && noexcept(color_stack.pop()))
        {
            // remove all but initial color
            while (color_stack.size() > 1)
                color_stack.pop();
            // remove all tags
            use_italics = 0;
            use_shadow = 0;
            draw_underline = 0;
            super_sub_shift = 0;
        }

        Clr CurrentColor() const noexcept(noexcept(color_stack.top())) { return color_stack.top(); }
    };

    /** \brief Holds precomputed glyph position information for rendering. */
    struct RenderCache
    {
        GL2DVertexBuffer vertices;
        GLTexCoordBuffer coordinates;
        GLRGBAColorBuffer colors;
        GL2DVertexBuffer underline_vertices;
        GLRGBAColorBuffer underline_colors;

        void clear() noexcept {
            vertices.clear();
            coordinates.clear();
            colors.clear();
            underline_vertices.clear();
            underline_colors.clear();
        }
    };

    /** Construct a font using only the printable ASCII characters.
        \throw Font::Exception Throws a subclass of Font::Exception if the
        condition specified for the subclass is met. */
    Font(std::string font_filename, unsigned int pts);

    /** Construct a font using only the printable ASCII characters,
        from the in-memory contents \a file_contents.  \throw Font::Exception
        Throws a subclass of Font::Exception if the condition specified for
        the subclass is met. */
    Font(std::string font_filename, unsigned int pts, const std::vector<uint8_t>& file_contents);

    /** Construct a font using all the code points in the
        UnicodeCharsets in the range [first, last).  \throw Font::Exception
        Throws a subclass of Font::Exception if the condition specified for
        the subclass is met. */
    template <typename CharSetIter>
    Font(std::string font_filename, unsigned int pts, CharSetIter first, CharSetIter last);

    /** Construct a font using all the code points in the
        UnicodeCharsets in the range [first, last), from the in-memory
        contents \a file_contents.  \throw Font::Exception Throws a subclass
        of Font::Exception if the condition specified for the subclass is
        met. */
    template <typename CharSetIter>
    Font(std::string font_filename, unsigned int pts, const std::vector<uint8_t>& file_contents,
         CharSetIter first, CharSetIter last);

    ~Font() = default;

    /** Returns the name of the file from which this font was created. */
    const auto& FontName() const noexcept { return m_font_filename; }

    /** Returns the point size in which the characters in the font object are
        rendered. */
    unsigned int PointSize() const noexcept { return m_pt_sz; }

    const auto& GetTexture() const noexcept { return m_texture; }

    /** Returns the range(s) of code points rendered in the font */
    const auto& UnicodeCharsets() const noexcept { return m_charsets; }

    /** Returns the maximum amount above the baseline the text can go. */
    Y    Ascent() const noexcept { return m_ascent; }

    /** Returns the maximum amount below the baseline the text can go. */
    Y    Descent() const noexcept { return m_descent; }

    /** Returns (Ascent() - Descent()). */
    Y    Height() const noexcept { return m_height; }

    /** Returns the distance that should be placed between lines.  This is
        usually not equal to Height(). */
    Y    Lineskip() const noexcept { return m_lineskip; }

    /** Returns the width of the glyph for the space character. */
    X    SpaceWidth() const noexcept { return X{m_space_width}; }

    const auto& GetGlyphs() const noexcept { return m_glyphs; }

    /** Unformatted text rendering; repeatedly calls RenderGlyph, then returns
        advance of entire string. */
    X    RenderText(Pt pt, const std::string_view text, const RenderState& render_state) const;

    /** Formatted text rendering. */
    void RenderText(Pt pt1, Pt pt2, const std::string& text, const Flags<TextFormat> format,
                    const LineVec& line_data, RenderState& render_state) const;

    /** Formatted text rendering over a subset of lines and code points.  The
        glyphs rendered are in the range [CodePointIndexOf(<i>begin_line</i>,
        <i>begin_char</i>, <i>line_data</i>), CodePointIndexOf(<i>end_line</i> -
        1, <i>end_char</i>, <i>line_data</i>)). */
    void RenderText(Pt pt1, Pt pt2, const std::string& text, const Flags<TextFormat> format,
                    const LineVec& line_data, RenderState& render_state,
                    std::size_t begin_line, CPSize begin_char,
                    std::size_t end_line, CPSize end_char) const;

    /** Wrapper around PreRenderText that provides dummy values for line start and end values.*/
    void PreRenderText(Pt ul, Pt lr, const std::string& text, const Flags<TextFormat> format,
                       RenderCache& cache, const LineVec& line_data,
                       RenderState& render_state) const;

    /** Fill the \p cache with glyphs corresponding to the passed in \p text and \p line_data.*/
    void PreRenderText(Pt pt1, Pt pt2, const std::string& text,
                       const Flags<TextFormat> format, const LineVec& line_data,
                       RenderState& render_state, std::size_t begin_line, CPSize begin_char,
                       std::size_t end_line, CPSize end_char, RenderCache& cache) const;

    /** Render the glyphs from the \p cache.*/
    void RenderCachedText(RenderCache& cache) const;

    /** Sets \a render_state as if all the text in \a line_data
      * before (<i>begin_line</i>, <i>begin_char</i>) had just been rendered. */
    static void ProcessTagsBefore(const LineVec& line_data, RenderState& render_state,
                                  std::size_t begin_line, CPSize begin_char);
    /** Sets \a render_state as if all the text in \a line_data had just been rendered. */
    static void ProcessTags(const LineVec& line_data, RenderState& render_state);

    /** Sets \a render_state as if all the text in \a char_aata before \a begin_char had just been rendered. */
    static void ProcessLineTagsBefore(const LineData::CharVec& char_data,
                                      RenderState& render_state, CPSize begin_char);
    /** Sets \a render_state as if all the text in \a char_data had just been rendered. */
    static void ProcessLineTags(const LineData::CharVec& char_data, RenderState& render_state);

    /** \brief This just holds the essential data necessary to render a glyph
        from the OpenGL texture(s) created at GG::Font creation time. */
    struct Glyph
    {
        Glyph() = default;
        Glyph(std::shared_ptr<Texture> texture, Pt ul, Pt lr, int8_t y_ofs, int8_t lb, int8_t adv);

        SubTexture  sub_texture;      ///< The subtexture containing just this glyph
        int8_t      y_offset = 0;     ///< The vertical offset to draw this glyph (may be negative!)
        int8_t      left_bearing = 0; ///< The space that should remain before the glyph
        int8_t      advance = 0;      ///< The amount of space the glyph should occupy, including glyph graphic and inter-glyph spacing
        int8_t      width = 0;        ///< The width of the glyph only
    };

    using GlyphMap = boost::unordered_map<uint32_t, Glyph>;

    /** Return a vector of TextElements parsed from \p text, using the
        FORMAT_IGNORETAGS bit in \p format to determine if all KnownTags()
        are ignored.

        This function is costly even on single character texts. Do not call
        it from tight loops.  Do not call it from within Render().  Do not
        call it repeatedly on a known text. */
    static std::vector<Font::TextElement> ExpensiveParseFromTextToTextElements(
        const std::string& text, const Flags<TextFormat> format, const GlyphMap& glyphs, int8_t space_width);
    std::vector<Font::TextElement> ExpensiveParseFromTextToTextElements(
        const std::string& text, const Flags<TextFormat> format) const
    { return ExpensiveParseFromTextToTextElements(text, format, m_glyphs, m_space_width); }


    /** Change \p text_elements and \p text to replace the text of the TextElement at
        \p targ_offset with \p new_text.

        This replaces the entire text of the TextElement at offset \p targ_offset and adjusts the
        string \p text to be consistent even if the \p new_text is longer/shorter than the original
        TEXT type TextElement.

        This does not recompute the text_elements. It is faster than
        ExpensiveParseFromTextToTextElements on a new string. It will not find white space in the
        inserted text.

        \p text and \p text_elements are assumed to be consistent with each other and both will be
        changed to remain consistent.

        \p targ_offset is the zero based offset of the TextElements of type TEXT.  It ignores
        other types of TextElements such as TAGS, WHITESPACE and NEWLINE, when determining the
        offset.

        Here is an example of changing a ship name from "oldname" to "New Ship Name":

        original text:             "<i>Ship:<\i> oldname ID:"
        orignal text_elements:     [<OPEN_TAG i>, <TEXT "Ship:">, <CLOSE_TAG i>, <WHITESPACE>, <TEXT oldname>, <WHITESPACE>, <TEXT ID:>]

        ChangeTemplatedText(text, text_elements, "New Ship Name", 1);

        changed text:              "<i>Ship:<\i> New Ship Name ID:"
        changed text_elements:     [<OPEN_TAG i>, <TEXT "Ship:">, <CLOSE_TAG i>, <WHITESPACE>, <TEXT New Ship Name>, <WHITESPACE>, <TEXT ID:>]
    */
    static void ChangeTemplatedText(std::string& text, std::vector<TextElement>& text_elements,
                                    const std::string& new_text, std::size_t targ_offset,
                                    const GlyphMap& glyphs, uint8_t space_width);
    void ChangeTemplatedText(std::string& text, std::vector<TextElement>& text_elements,
                             const std::string& new_text, std::size_t targ_offset) const
    { ChangeTemplatedText(text, text_elements, new_text, targ_offset, m_glyphs, m_space_width); }

    /** DetermineLines() returns the \p line_data resulting from adding the necessary line
        breaks, to the \p text formatted with \p format and parsed into \p text_elements, to fit
        the \p text into a box of width \p box_width.

        It accounts for alignment, wrapping and justification of the \p text.

        A \p box_width of X0 will add a line break at every whitespace element in \p text_elements.

        Supplying a \p text and \p text_elements that are incompatible will result in undefined
        behavior.  \p text_elements contains internal pointers to the \p text to which it is
        bound.  Compatible means the exact same \p text object, not the same text content. */
    LineVec DetermineLines(const std::string& text, Flags<TextFormat> format, X box_width,
                           const std::vector<TextElement>& text_elements) const;

    /** Returns the maximum dimensions of the text in x and y. */
    Pt TextExtent(const LineVec& line_data) const noexcept;

    /** Adds \a tag to the list of embedded tags that Font should not print
        when rendering text.  Passing "foo" will cause Font to treat "<foo>",
        "<foo [arg1 [arg2 ...]]>", and "</foo>" as tags. */
    static void RegisterKnownTags(std::vector<std::string_view> tags);

    /** Returns the input \a text, stripped of any formatting tags. */
    static std::string StripTags(std::string_view text);

    /** The base class for Font exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when valid font data cannot be read from a file. */
    GG_CONCRETE_EXCEPTION(BadFile, GG::Font, Exception);

    /** Thrown when a 0 font size is requested. */
    GG_CONCRETE_EXCEPTION(InvalidPointSize, GG::Font, Exception);

    /** Thrown when a FreeType font could be loaded, but the resulting font is
        not scalable, making it unusable by GG. */
    GG_CONCRETE_EXCEPTION(UnscalableFont, GG::Font, Exception);

    /** Thrown when an attempt is made to create a glyph from null font face
        object. */
    GG_CONCRETE_EXCEPTION(BadFace, GG::Font, Exception);

    /** Thrown when an attempt to set the size of a FreeType font face
        fails. */
    GG_CONCRETE_EXCEPTION(BadPointSize, GG::Font, Exception);

    /** Thrown when FreeType is unable to fulfill a request to load or render
        a glpyh. */
    GG_CONCRETE_EXCEPTION(BadGlyph, GG::Font, Exception);

    /** Throws a BadGlyph exception, with \a c converted to a printable ASCII
        character (if possible), or as a Unicode code point.  \a format_str
        should contain the Boost.Format positional notation formatting tag
        "%1%" where the code point should appear. */
    static void ThrowBadGlyph(const std::string& format_str, uint32_t c);

protected:
    Font() = default;

private:
    FT_Error          GetFace(FT_Face& face);
    FT_Error          GetFace(const std::vector<uint8_t>& file_contents, FT_Face& face);
    void              CheckFace(FT_Face font, FT_Error error);
    void              Init(FT_Face& font);

    bool              GenerateGlyph(FT_Face font, uint32_t ch);

    X                 StoreGlyph(Pt pt, const Glyph& glyph, const RenderState& render_state,
                                 RenderCache& cache) const;
    void              StoreGlyphImpl(RenderCache& cache, GG::Clr color, Pt pt,
                                     const Glyph& glyph, int x_top_offset,
                                     int y_shift) const;
    void              StoreUnderlineImpl(RenderCache& cache, GG::Clr color, Pt pt,
                                         const Glyph& glyph, Y descent, Y height,
                                         Y underline_height, Y underline_offset) const;

    bool              IsDefaultFont() const noexcept;

    static std::shared_ptr<Font> GetDefaultFont(unsigned int pts);

    std::string                 m_font_filename;
    unsigned int                m_pt_sz = 0;
    std::vector<UnicodeCharset> m_charsets; ///< The sets of glyphs that are covered by this font object

    Y        m_ascent = Y0;            ///< Maximum amount above the baseline the text can go
    Y        m_descent = Y0;           ///< Maximum amount below the baseline the text can go
    Y        m_height = Y0;            ///< Ascent - descent
    Y        m_lineskip = Y0;          ///< Distance that should be placed between lines
    double   m_underline_offset = 0.0; ///< Amount below the baseline that the underline sits
    double   m_underline_height = 0.0; ///< Height (thickness) of underline
    double   m_italics_offset = 0.0;   ///< Amount that the top of an italicized glyph is left of the bottom
    double   m_super_sub_offset = 0.0; ///< Ammount to shift super or subscript text
    double   m_shadow_offset = 0.0;    ///< Amount that shadows rendered under texts are displaced from the text
    int8_t   m_space_width = 0;        ///< The width of the glyph for the space character

    GlyphMap m_glyphs;                ///< The locations of the images of each glyph within the textures

    /** The OpenGL texture object in which the glyphs can be found. */
    std::shared_ptr<Texture> m_texture;
};

/** Stream output operator for Font::Substring. */
GG_API std::ostream& operator<<(std::ostream& os, Font::Substring substr);

GG_API CPSize GlyphIndexOfLineAndGlyph(std::size_t line_index, CPSize glyph_index,
                                       const Font::LineVec& line_data);

/** Returns the code point index of the <i>index</i>-th glyph on line \a
    line within the text represented by \a line_data.  Returns the index of
    the code point one past the end of the text if \a line is out of bounds.
    Returns the index of the next previous code point from the end of line
    \a line if \a glyph_index is out of bounds on that line. */
GG_API CPSize CodePointIndexOfLineAndGlyph(std::size_t line_index, CPSize glyph_index,
                                           const Font::LineVec& line_data);

GG_API CPSize CodePointIndexOfLineAndCodePoint(std::size_t line_index, CPSize cp_index,
                                               const Font::LineVec& line_data);


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
GG_API CPSize CodePointIndexAfterPreviousGlyph(std::size_t line_index, CPSize glyph_index,
                                               const Font::LineVec& line_data);

/** Returns the string index of the <i>index</i>-th glyph on line \a line
    within the text represented by \a line_data.  Returns the index of the
    character one past the end of the text if \a line or \a index are out of
    bounds. */
GG_API StrSize StringIndexOfLineAndGlyph(std::size_t line, CPSize index,
                                         const Font::LineVec& line_data);

/** Returns the string indiex of the <i>index</i>-th code point in \a line_data */
GG_API StrSize StringIndexOfCodePoint(CPSize index, const Font::LineVec& line_data);

/** Returns the line L and the code point index within L of the
    <i>index</i>-th glyph within the text represented by \a line_data.
    Returns (std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE) if \a
    index is out of bounds. */
GG_API std::pair<std::size_t, CPSize>
LinePositionOfGlyph(CPSize index, const Font::LineVec& line_data);

GG_API std::pair<std::size_t, CPSize>
LinePositionOfCodePoint(CPSize index, const Font::LineVec& line_data);

GG_API std::pair<StrSize, StrSize> GlyphIndicesRangeToStringSizeIndices(
    CPSize start_idx, CPSize end_idx, const Font::LineVec& line_data);

GG_API std::pair<StrSize, StrSize> CodePointIndicesRangeToStringSizeIndices(
    CPSize start_idx, CPSize end_idx, const Font::LineVec& line_data);

GG_API CPSize GlyphIndexOfX(const Font::LineData::CharVec& char_data, X x, X offset = X0);
GG_API CPSize GlyphIndexOfXOnLine0(const Font::LineVec& line_data, X x, X offset = X0);

GG_API CPSize CodePointIndexOfX(const Font::LineData::CharVec& char_data, X x, X offset = X0);
GG_API CPSize CodePointIndexOfXOnLine0(const Font::LineVec& line_data, X x, X offset = X0);

/** \brief A singleton that loads and stores fonts for use by GG.

    This class is essentially a very thin wrapper around a map of
    Font smart pointers, keyed on font filename/point size pairs.  The user
    need only request a font through GetFont(); if the font at the requested
    size needs to be created, the font is created at the requestd size, a
    shared_ptr to it is kept, and a copy of the shared_ptr is returned.  If
    the font has been created at the desired size, but the request includes
    code point range(s) not already created, the font at the requested size is
    created with the union of the reqested and existing ranges, stored, and
    returned as above; the only difference is that the original shared_ptr is
    released.  Due to the shared_ptr semantics, the object pointed to by the
    shared_ptr is deleted if and only if the last shared_ptr that refers to it
    is deleted.  So any requested font can be used as long as the caller
    desires, even when another caller tells the FontManager to free the
    font. */
class GG_API FontManager
{
private:
    /** \brief This GG::FontManager-private struct is used as a key type for
        the map of rendered fonts. */
    struct GG_API FontKey
    {
        template <typename S>
        FontKey(S&& str, unsigned int pts) :
            filename(std::forward<S>(str)),
            points(pts)
        {}
        ///< Lexocograhpical ordering on filename then points.
        [[nodiscard]] bool operator<(const FontKey& rhs) const noexcept
        { return (filename < rhs.filename || (filename == rhs.filename && points < rhs.points)); }

        std::string  filename; ///< The name of the file from which this font was created.
        unsigned int points;   ///< The point size in which this font was rendered.
    };

public:
    /** Returns true iff this manager contains a font with the given filename
        and point size, regardless of charsets. */
    bool HasFont(std::string_view font_filename, unsigned int pts) const noexcept;

    /** Returns true iff this manager contains a font with the given filename
        and point size, containing the given charsets. */
    template <typename CharSetIter>
    bool HasFont(std::string_view font_filename, unsigned int pts,
                 CharSetIter first, CharSetIter last) const;

    /** Returns a shared_ptr to the requested font, supporting all printable
        ASCII characters.  \note May load font if unavailable at time of
        request. */
    std::shared_ptr<Font> GetFont(std::string_view font_filename, unsigned int pts);

    /** Returns a shared_ptr to the requested font, supporting all printable
        ASCII characters, from the in-memory contents \a file_contents.  \note
        May load font if unavailable at time of request. */
    std::shared_ptr<Font> GetFont(std::string_view font_filename, unsigned int pts,
                                  const std::vector<uint8_t>& file_contents);

    /** Returns a shared_ptr to the requested font, supporting all the
        code points in the UnicodeCharsets in the range [first, last).  \note
        May load font if unavailable at time of request. */
    template <typename CharSetIter>
    std::shared_ptr<Font> GetFont(std::string_view font_filename, unsigned int pts,
                                  CharSetIter first, CharSetIter last);

    /** Returns a shared_ptr to the requested font, supporting all the code
        points in the UnicodeCharsets in the range [first, last), from the
        in-memory contents \a file_contents.  \note May load font if
        unavailable at time of request. */
    template <typename CharSetIter>
    std::shared_ptr<Font> GetFont(std::string_view font_filename, unsigned int pts,
                                  const std::vector<uint8_t>& file_contents,
                                  CharSetIter first, CharSetIter last);

    /** Removes the indicated font from the font manager.  Due to shared_ptr
        semantics, the font may not be deleted until much later. */
    void FreeFont(std::string_view font_filename, unsigned int pts);

private:
    FontManager() = default;

    template <typename CharSetIter>
    std::shared_ptr<Font> GetFontImpl(std::string_view font_filename, unsigned int pts,
                                      const std::vector<uint8_t>* file_contents,
                                      CharSetIter first, CharSetIter last);

    using FontContainer = std::vector<std::pair<FontKey, std::shared_ptr<Font>>>;
    using FontContainerIt = FontContainer::const_iterator;

    FontContainerIt FontLookup(std::string_view font_filename, unsigned int pts) const noexcept
    {
        return std::find_if(m_rendered_fonts.begin(), m_rendered_fonts.end(),
                            [font_filename, pts](const auto& key_font) {
                                return key_font.first.filename == font_filename &&
                                    key_font.first.points == pts;
                            });
    }

    FontContainer m_rendered_fonts;

    static const std::shared_ptr<Font> EMPTY_FONT;

    friend GG_API FontManager& GetFontManager();
};

/** Returns the singleton FontManager instance. */
GG_API FontManager& GetFontManager();

/** Thrown when initialization of the FreeType library fails. */
GG_EXCEPTION(FailedFTLibraryInit);

namespace detail {
    struct GG_API FTFaceWrapper
    {
        FTFaceWrapper() = default;
        ~FTFaceWrapper();
        FT_Face m_face = nullptr;
    };
}
}


template <typename CharSetIter>
GG::Font::Font(std::string font_filename, unsigned int pts,
               CharSetIter first, CharSetIter last) :
    m_font_filename(std::move(font_filename)),
    m_pt_sz(pts),
    m_charsets(first, last)
{
    if (!m_font_filename.empty()) {
        detail::FTFaceWrapper wrapper;
        FT_Error error = GetFace(wrapper.m_face);
        CheckFace(wrapper.m_face, error);
        Init(wrapper.m_face);
    }
}

template <typename CharSetIter>
GG::Font::Font(std::string font_filename, unsigned int pts,
               const std::vector<uint8_t>& file_contents,
               CharSetIter first, CharSetIter last) :
    m_font_filename(std::move(font_filename)),
    m_pt_sz(pts),
    m_charsets(first, last)
{
    assert(!file_contents.empty());
    detail::FTFaceWrapper wrapper;
    FT_Error error = GetFace(file_contents, wrapper.m_face);
    CheckFace(wrapper.m_face, error);
    Init(wrapper.m_face);
}

template <typename CharSetIter>
bool GG::FontManager::HasFont(std::string_view font_filename, unsigned int pts,
                              CharSetIter first, CharSetIter last) const
{
    const auto it = FontLookup(font_filename, pts);
    if (it == m_rendered_fonts.end())
        return false;

    std::vector<UnicodeCharset> requested_charsets(first, last);
    std::sort(requested_charsets.begin(), requested_charsets.end());
    std::vector<UnicodeCharset> found_charsets(it->second->UnicodeCharsets());
    std::sort(found_charsets.begin(), found_charsets.end());

    return (requested_charsets == found_charsets);
}

template <typename CharSetIter>
std::shared_ptr<GG::Font>
GG::FontManager::GetFont(std::string_view font_filename, unsigned int pts,
                         CharSetIter first, CharSetIter last)
{ return GetFontImpl(font_filename, pts, nullptr, first, last); }

template <typename CharSetIter>
std::shared_ptr<GG::Font>
GG::FontManager::GetFont(std::string_view font_filename, unsigned int pts,
                         const std::vector<uint8_t>& file_contents,
                         CharSetIter first, CharSetIter last)
{ return GetFontImpl(font_filename, pts, &file_contents, first, last); }


template <typename CharSetIter>
std::shared_ptr<GG::Font>
GG::FontManager::GetFontImpl(std::string_view font_filename, unsigned int pts,
                             const std::vector<uint8_t>* file_contents,
                             CharSetIter first, CharSetIter last)
{
    const auto it = FontLookup(font_filename, pts);
    if (it == m_rendered_fonts.end()) { // if no such font has been created, create it now
        if (font_filename.empty()) {
            // keeps this function from throwing; "" is the only invalid font
            // filename that shouldn't throw
            return EMPTY_FONT;
        } else {
            auto font = file_contents ?
                std::make_shared<Font>(std::string(font_filename), pts, *file_contents, first, last) :
                std::make_shared<Font>(std::string(font_filename), pts, first, last);
            return m_rendered_fonts.emplace_back(FontKey{font_filename, pts}, std::move(font)).second;
        }

    // if a font like this has been created, but it doesn't have all the right
    // glyphs, release it and create a new one
    } else {
        std::vector<UnicodeCharset> requested_charsets(first, last);
        std::sort(requested_charsets.begin(), requested_charsets.end());
        const auto requested_unique_it = std::unique(requested_charsets.begin(), requested_charsets.end());
        std::vector<UnicodeCharset> found_charsets(it->second->UnicodeCharsets());
        std::sort(found_charsets.begin(), found_charsets.end());
        const auto found_unique_it = std::unique(found_charsets.begin(), found_charsets.end());

        if (requested_charsets == found_charsets)
            return it->second;

        std::vector<UnicodeCharset> united_charsets;
        united_charsets.reserve(std::max(requested_charsets.size(), found_charsets.size()));

        std::set_union(requested_charsets.begin(), requested_unique_it,
                       found_charsets.begin(), found_unique_it,
                       std::back_inserter(united_charsets));
        m_rendered_fonts.erase(it);
        auto font = file_contents ?
            std::make_shared<Font>(std::string(font_filename), pts, *file_contents,
                                   united_charsets.begin(), united_charsets.end()) :
            std::make_shared<Font>(std::string(font_filename), pts,
                                   united_charsets.begin(), united_charsets.end());
        return m_rendered_fonts.emplace_back(FontKey{font_filename, pts}, std::move(font)).second;
    }
}


#endif
