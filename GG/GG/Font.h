// -*- C++ -*-
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

/** \file Font.h \brief Contains the Font class, a class that encapsulates the
    rendering of a single FreeType-compatible font in italics, with
    underlining, left-, right-, or center- justified, etc., and the
    FontManager class which provides application-wide management of Font
    objects. */

#ifndef _GG_Font_h_
#define _GG_Font_h_

#include <GG/AlignmentFlags.h>
#include <GG/FontFwd.h>
#include <GG/Texture.h>
#include <GG/UnicodeCharsets.h>

#include <boost/unordered_map.hpp>
#include <boost/graph/graph_concepts.hpp>

#include <memory>
#include <set>
#include <stack>


struct FT_FaceRec_;
typedef struct FT_FaceRec_*  FT_Face;
typedef int FT_Error;

namespace GG {

class GLRGBAColorBuffer;
class GLTexCoordBuffer;
class GL2DVertexBuffer;

/** Returns a string of the form "<rgba r g b a>" from a Clr object with color
    channels r, b, g, a. */
GG_API std::string RgbaTag(const Clr& c);


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
    RegisterKnownTag(), RemoveKnownTag(), and ClearKnownTags() for details.
    It is not possible to remove the built-in tags using these methods.  If
    you wish not to use tags at all, call DetermineLines() and RenderText()
    with the format parameter containing FORMAT_IGNORETAGS, or include a
    \<pre> tag at the beginning of the text to be rendered.
   */
class GG_API Font
{
public:
    /** \brief A range of iterators into a std::string that defines a
        substring found in a string being rendered by Font.

        Substring is bound to a particular instance of a std::string.  If
        that particular std::string goes out of scope or is deleted then
        behavior is undefined, but may seg fault with the next access. */
    class GG_API Substring
    {
    public:
        typedef std::pair<std::string::const_iterator, std::string::const_iterator> IterPair;

        Substring();

        /** Ctor.  \a first_ must be <= \a second_. */
        Substring(const std::string& str_,
                  std::string::const_iterator first_,
                  std::string::const_iterator second_);

        /** Construction from base.  \a pair.first must be <= \a
            pair.second. */
        Substring(const std::string& str_, const IterPair& pair);

        /** Attach this Substring to \p str_.

            This changes the iterators from pointing into the previous
            std::string to pointing into \p str_.
        */
        void Bind(const std::string& str_);

        /** Returns an iterator to the beginning of the substring. */
        std::string::const_iterator begin() const;

        /** Returns an iterator to one-past-the-end of the substring. */
        std::string::const_iterator end() const;

        /** True iff .first == .second. */
        bool empty() const;

        /** Length, in original string chars, of the substring. */
        std::size_t size() const;

        /** Implicit conversion to std::string. */
        operator std::string() const;

        /** Comparison with std::string. */
        bool operator==(const std::string& rhs) const;

        /** Comparison with std::string. */
        bool operator!=(const std::string& rhs) const;

        /** Concatenation with base.  \a rhs.first must be <= \a rhs.second.
            .second must be equal to \a rhs.first (*this and \a rhs must be
            contiguous). */
        Substring& operator+=(const IterPair& rhs);

    private:
        const std::string* str;
        std::ptrdiff_t first;
        std::ptrdiff_t second;

        static const std::string EMPTY_STRING;
    };

    /** \brief Used to encapsulate a token-like piece of text to be rendered
        using GG::Font. */
    struct GG_API TextElement
    {
        /** The types of token-like entities that can be represented by a
            TextElement. */
        enum TextElementType {
            OPEN_TAG,   ///< An opening text formatting tag (e.g. "<rgba 0 0 0 255>").
            CLOSE_TAG,  ///< A closing text formatting tag (e.g. "</rgba>").
            TEXT,       ///< Some non-whitespace text (e.g. "The").
            WHITESPACE, ///< Some whitespace text (e.g. "  \n").

            /** A newline.  Newline TextElements represent the newline code
                point when it is encountered in a rendered string, though they
                do not contain the actual newline character -- their \a text
                members are always ""). */
            NEWLINE
        };

        /** Ctor.  \a ws indicates that the element contains only whitespace;
            \a nl indicates that it is a newline element. */
        TextElement(bool ws, bool nl);

        virtual ~TextElement();

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
        virtual void Bind(const std::string& whole_text);

        /** Returns the TextElementType of the element. */
        virtual TextElementType Type() const;

        /** Returns the width of the element. */
        X Width() const;

        /* Returns the number of characters in the original string that the
           element represents. */
        StrSize StringSize() const;

        /** Returns the number of code points in the original string that the
            element represents. */
        CPSize CodePointSize() const;

        virtual bool operator==(const TextElement &rhs) const;

        /** The text from the original string represented by the element. */
        Substring text;

        std::vector<X> widths;     ///< The widths of the glyphs in \a text.
        const bool     whitespace; ///< True iff this is a whitespace element.
        const bool     newline;    ///< True iff this is a newline element.

    protected:
        TextElement();

    private:
        mutable X cached_width;
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
        TextAndElementsAssembler(const Font& font);
        ~TextAndElementsAssembler();

        /** Return the constructed text.*/
        const std::string& Text();
        /** Return the constructed TextElements.*/
        const std::vector<std::shared_ptr<TextElement>>& Elements();

        /** Add an open tag iff it exists as a recognized tag.*/
        TextAndElementsAssembler& AddOpenTag(const std::string& tag);
        /** Add an open tag iff it exists as a recognized tag.*/
        TextAndElementsAssembler& AddOpenTag(const std::string& tag, const std::vector<std::string>& params);
        /** Add a close tag iff it exists as a recognized tag.*/
        TextAndElementsAssembler& AddCloseTag(const std::string& tag);
        /** Add a text element.  Any whitespace in this text element will be non-breaking.*/
        TextAndElementsAssembler& AddText(const std::string& text);
        /** Add a white space element.*/
        TextAndElementsAssembler& AddWhitespace(const std::string& whitespace);
        /** Add a new line element.*/
        TextAndElementsAssembler& AddNewline();

        /** Add an open Clr tag.*/
        TextAndElementsAssembler& AddOpenTag(const Clr& color);

    private:
        class Impl;
        std::unique_ptr<Impl> const m_impl;
    };

    /** \brief The type of TextElement that represents a text formatting
        tag. */
    struct GG_API FormattingTag : TextElement
    {
        /** Ctor.  \a close indicates that the tag is a close-tag
            (e.g. "</rgba>"). */
        FormattingTag(bool close);

        /** Attach to \p whole_text by binding all Substring data members,
            both the base class and the data member tag_name to the string
            \p whole_text.*/
        void Bind(const std::string& whole_text) override;

        TextElementType Type() const override;

        bool operator==(const TextElement &rhs) const override;

        /** The parameter strings within the tag, e.g. "0", "0", "0", and "255"
            for the tag "<rgba 0 0 0 255>". */
        std::vector<Substring> params;

        /** The name of the tag (e.g. for the tag "<i>", tag_name is "i"). */
        Substring tag_name;

        /** True iff this is a close-tag. */
        const bool close_tag;

    private:
        FormattingTag();
    };

    /** \brief Holds the essential data on each line that a string occupies when
        rendered with given format flags.

        \a char_data contains the visible glyphs for each line, plus any text
        formatting tags present on that line as well. */
    struct GG_API LineData
    {
        LineData();

        /** \brief Contains the extent, the index into the original string,
            and the text formatting tags that should be applied before
            rendering of a visible glyph. */
        struct GG_API CharData
        {
            CharData();

            CharData(X extent_, StrSize str_index, StrSize str_size, CPSize cp_index,
                     const std::vector<std::shared_ptr<TextElement>>& tags_);

            /** The furthest-right extent of this glyph as it appears on the
                line. */
            X extent;

            /** The position in the original string of the first character of
                this glyph. */
            StrSize string_index;

            /** The size in the original string of the characters that make up
                this glyph. */
            StrSize string_size;

            /** The code point index of this glyph. */
            CPSize code_point_index;

            /** The text formatting tags that should be applied before
                rendering this glyph. */
            std::vector<std::shared_ptr<FormattingTag>> tags;
        };

        X    Width() const; ///< Returns the width of the line.
        bool Empty() const; ///< Returns true iff char_data has size 0.

        /** Data on each individual glyph. */
        std::vector<CharData> char_data;

        /** FORMAT_LEFT, FORMAT_CENTER, or FORMAT_RIGHT; derived from text
            format flags and/or formatting tags in the text. */
        Alignment justification;
    };

    /** \brief Holds the state of tags during rendering of text.

        By keeping track of this state across multiple calls to RenderText(),
        the user can preserve the functionality of the text formatting tags,
        if present. */
    struct GG_API RenderState
    {
        RenderState();

        RenderState(Clr color); //< Takes default text color as parameter

        /** The count of open \<i> tags seen since the last \</i> seen. */
        std::size_t     use_italics;

        /** The count of open \<s> tags seen since the last \</s> seen. */
        std::size_t     use_shadow;

        /** The count of open \<u> tags seen since the last \</u> seen. */
        std::size_t     draw_underline;

        /** The count of open \<super> (positive) minus \<sub> tags seen. */
        int             super_sub_shift;

        /** The stack of text color indexes (as set by previous tags). */
        std::stack<int> color_index_stack;

        /** All colors that have been used. **/
        std::vector<Clr> used_colors;

        /// Add color to stack and remember it has been used
        void PushColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a);

        /// Return to the previous used color, or remain as default
        void PopColor();

        /// Return the index of the current color in used_colors
        int CurrentIndex() const;

        const Clr& CurrentColor()  const;

        /// Return true if there are no more colors to pop.
        bool ColorsEmpty() const;
    };

    /** \brief Holds precomputed glyph position information for rendering.
     */
    struct RenderCache
    {
        std::unique_ptr<GL2DVertexBuffer> vertices;
        std::unique_ptr<GLTexCoordBuffer> coordinates;
        std::unique_ptr<GLRGBAColorBuffer> colors;

        std::unique_ptr<GL2DVertexBuffer> underline_vertices;
        std::unique_ptr<GLRGBAColorBuffer> underline_colors;

        RenderCache();

        ~RenderCache();
    };

    /** \name Structors */ ///@{
    /** Ctor.  Construct a font using only the printable ASCII characters.
        \throw Font::Exception Throws a subclass of Font::Exception if the
        condition specified for the subclass is met. */
    Font(const std::string& font_filename, unsigned int pts);

    /** Ctor.  Construct a font using only the printable ASCII characters,
        from the in-memory contents \a file_contents.  \throw Font::Exception
        Throws a subclass of Font::Exception if the condition specified for
        the subclass is met. */
    Font(const std::string& font_filename, unsigned int pts,
         const std::vector<unsigned char>& file_contents);

    /** Ctor.  Construct a font using all the code points in the
        UnicodeCharsets in the range [first, last).  \throw Font::Exception
        Throws a subclass of Font::Exception if the condition specified for
        the subclass is met. */
    template <class CharSetIter>
    Font(const std::string& font_filename, unsigned int pts,
         CharSetIter first, CharSetIter last);

    /** Ctor.  Construct a font using all the code points in the
        UnicodeCharsets in the range [first, last), from the in-memory
        contents \a file_contents.  \throw Font::Exception Throws a subclass
        of Font::Exception if the condition specified for the subclass is
        met. */
    template <class CharSetIter>
    Font(const std::string& font_filename, unsigned int pts,
         const std::vector<unsigned char>& file_contents,
         CharSetIter first, CharSetIter last);

    ~Font(); ///< Dtor.
    //@}

    /** \name Accessors */ ///@{
    /** Returns the name of the file from which this font was created. */
    const std::string& FontName() const;

    /** Returns the point size in which the characters in the font object are
        rendered. */
    unsigned int PointSize() const;

    const std::shared_ptr<Texture> GetTexture() const
    { return m_texture; }

    /** Returns the range(s) of code points rendered in the font */
    const std::vector<UnicodeCharset>& UnicodeCharsets() const;

    /** Returns the maximum amount above the baseline the text can go. */
    Y    Ascent() const;

    /** Returns the maximum amount below the baseline the text can go. */
    Y    Descent() const;

    /** Returns (Ascent() - Descent()). */
    Y    Height() const;

    /** Returns the distance that should be placed between lines.  This is
        usually not equal to Height(). */
    Y    Lineskip() const;

    /** Returns the width of the glyph for the space character. */
    X    SpaceWidth() const;

    /** Unformatted text rendering; repeatedly calls RenderGlyph, then returns
        advance of entire string. */
    X    RenderText(const Pt& pt, const std::string& text) const;

    /** Formatted text rendering. */
    void RenderText(const Pt& pt1, const Pt& pt2, const std::string& text, Flags<TextFormat>& format,
                    const std::vector<LineData>& line_data, RenderState* render_state = nullptr) const;

    /** Formatted text rendering over a subset of lines and code points.  The
        glyphs rendered are in the range [CodePointIndexOf(<i>begin_line</i>,
        <i>begin_char</i>, <i>line_data</i>), CodePointIndexOf(<i>end_line</i> -
        1, <i>end_char</i>, <i>line_data</i>)). */
    void RenderText(const Pt& pt1, const Pt& pt2, const std::string& text, Flags<TextFormat>& format,
                    const std::vector<LineData>& line_data, RenderState& render_state,
                    std::size_t begin_line, CPSize begin_char,
                    std::size_t end_line, CPSize end_char) const;

    /** Wrapper around PreRenderText that provides dummy values for line start and end values.*/
    void PreRenderText(const Pt& ul, const Pt& lr, const std::string& text, Flags<TextFormat>& format,
                       RenderCache& cache, const std::vector<LineData>& line_data,
                       RenderState* render_state = nullptr) const;

    /** Fill the \p cache with glyphs corresponding to the passed in \p text and \p line_data.*/
    void PreRenderText(const Pt& pt1, const Pt& pt2, const std::string& text,
                       Flags<TextFormat>& format, const std::vector<LineData>& line_data,
                       RenderState& render_state, std::size_t begin_line, CPSize begin_char,
                       std::size_t end_line, CPSize end_char, RenderCache& cache) const;

    /** Render the glyphs from the \p cache.*/
    void RenderCachedText(RenderCache& cache) const;

    /** Sets \a render_state as if all the text before (<i>begin_line</i>,
        <i>begin_char</i>) had just been rendered. */
    void ProcessTagsBefore(const std::vector<LineData>& line_data, RenderState& render_state,
                           std::size_t begin_line, CPSize begin_char) const;

    /** Return a vector of TextElements parsed from \p text, using the
        FORMAT_IGNORETAGS bit in \p format to determine if all KnownTags()
        are ignored.

        This function is costly even on single character texts. Do not call
        it from tight loops.  Do not call it from within Render().  Do not
        call it repeatedly on a known text.
    */
    std::vector<std::shared_ptr<Font::TextElement>> ExpensiveParseFromTextToTextElements(const std::string& text,
                                                                                         const Flags<TextFormat>& format) const;

    /** Fill \p text_elements with the font widths of characters from \p text starting from \p
        starting_from. */
    void FillTemplatedText(const std::string& text,
                           std::vector<std::shared_ptr<TextElement>>& text_elements,
                           std::vector<std::shared_ptr<TextElement>>::iterator starting_from) const;

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
    void ChangeTemplatedText(std::string& text,
                             std::vector<std::shared_ptr<TextElement>>& text_elements,
                             const std::string& new_text,
                             size_t targ_offset) const;

    /** DetermineLines() returns the \p line_data resulting from adding the necessary line
        breaks, to  the \p text formatted with \p format and parsed into \p text_elements, to fit
        the \p text into a box of width \p box_width.

        It accounts for alignment, wrapping and justification of the \p text.

        A \p box_width of X0 will add a line break at every whitespace element in \p text_elements.

        Supplying a \p text and \p text_elements that are incompatible will result in undefined
        behavior.  \p text_elements contains internal pointers to the \p text to which it is
        bound.  Compatible means the exact same \p text object, not the same text content.
        */
    std::vector<LineData>   DetermineLines(const std::string& text, Flags<TextFormat>& format, X box_width,
                                           const std::vector<std::shared_ptr<TextElement>>& text_elements) const;

    /** Returns the maximum dimensions of the text in x and y. */
    Pt   TextExtent(const std::vector<LineData>& line_data) const;
    //@}

    /** Adds \a tag to the list of embedded tags that Font should not print
        when rendering text.  Passing "foo" will cause Font to treat "<foo>",
        "<foo [arg1 [arg2 ...]]>", and "</foo>" as tags. */
    static void RegisterKnownTag(const std::string& tag);

    /** Removes \a tag from the known tag list.  Does not remove the built in
        tags: \<i>, \<u>, \<rgba r g b a>, and \<pre>. */
    static void RemoveKnownTag(const std::string& tag);

    /** Removes all tags from the known tag list.  Does not remove the built
        in tags: \<i>, \<u>, \<rgba r g b a>, and \<pre>. */
    static void ClearKnownTags();

    /** Returns the input \a text, stripped of any formatting tags. */
    static std::string StripTags(const std::string& text, bool strip_unpaired_tags = true);

    /** \name Exceptions */ ///@{
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
    //@}

    /** Throws a BadGlyph exception, with \a c converted to a printable ASCII
        character (if possible), or as a Unicode code point.  \a format_str
        should contain the Boost.Format positional notation formatting tag
        "%1%" where the code point should appear. */
    static void ThrowBadGlyph(const std::string& format_str, std::uint32_t c);

protected:
    /** \name Structors */ ///@{
    Font();
    //@}

private:
    /** \brief This just holds the essential data necessary to render a glyph
        from the OpenGL texture(s) created at GG::Font creation time. */
    struct Glyph
    {
        Glyph();

        Glyph(const std::shared_ptr<Texture>& texture, const Pt& ul, const Pt& lr, Y y_ofs,
              X lb, X adv); ///< Ctor

        SubTexture  sub_texture;   ///< The subtexture containing just this glyph
        Y           y_offset;      ///< The vertical offset to draw this glyph (may be negative!)
        X           left_bearing;  ///< The space that should remain before the glyph
        X           advance;       ///< The amount of space the glyph should occupy, including glyph graphic and inter-glyph spacing
        X           width;         ///< The width of the glyph only
    };

    typedef boost::unordered_map<std::uint32_t, Glyph> GlyphMap;

    FT_Error          GetFace(FT_Face& face);
    FT_Error          GetFace(const std::vector<unsigned char>& file_contents, FT_Face& face);
    void              CheckFace(FT_Face font, FT_Error error);
    void              Init(FT_Face& font);

    bool GenerateGlyph(FT_Face font, std::uint32_t ch);

    void              ValidateFormat(Flags<TextFormat>& format) const;

    X                 StoreGlyph(const Pt& pt, const Glyph& glyph, const RenderState* render_state,
                                 RenderCache& cache) const;
    void              StoreGlyphImpl(RenderCache& cache, GG::Clr color, const Pt& pt,
                                     const Glyph& glyph, int x_top_offset,
                                     int y_shift) const;
    void              StoreUnderlineImpl(RenderCache& cache, GG::Clr color, const Pt& pt,
                                         const Glyph& glyph, Y descent, Y height,
                                         Y underline_height, Y underline_offset) const;

    void              HandleTag(const std::shared_ptr<FormattingTag>& tag, double* orig_color,
                                RenderState& render_state) const;
    bool              IsDefaultFont();

    std::shared_ptr<Font> GetDefaultFont(unsigned int pts);

    std::string          m_font_filename;
    unsigned int         m_pt_sz;
    std::vector<UnicodeCharset>
                         m_charsets;    ///< The sets of glyphs that are covered by this font object
    Y                    m_ascent;      ///< Maximum amount above the baseline the text can go
    Y                    m_descent;     ///< Maximum amount below the baseline the text can go
    Y                    m_height;      ///< Ascent - descent
    Y                    m_lineskip;    ///< Distance that should be placed between lines
    double               m_underline_offset; ///< Amount below the baseline that the underline sits
    double               m_underline_height; ///< Height (thickness) of underline
    double               m_italics_offset;   ///< Amount that the top of an italicized glyph is left of the bottom
    double               m_super_sub_offset; ///< Ammount to shift super or subscript text
    double               m_shadow_offset;    ///< Amount that shadows rendered under texts are displaced from the text
    X                    m_space_width; ///< The width of the glyph for the space character
    GlyphMap             m_glyphs;      ///< The locations of the images of each glyph within the textures

    /** The OpenGL texture object in which the glyphs can be found. */
    std::shared_ptr<Texture> m_texture;
};

/** Stream output operator for Font::Substring. */
GG_API std::ostream& operator<<(std::ostream& os, const Font::Substring& substr);

/** Returns the code point index of the <i>index</i>-th code point on line \a
    line within the text represented by \a line_data.  Returns the index of
    the code point one past the end of the text if \a line or \a index are out
    of bounds. */
GG_API CPSize CodePointIndexOf(std::size_t line, CPSize index,
                               const std::vector<Font::LineData>& line_data);

/** Returns the string index of the <i>index</i>-th code point on line \a line
    within the text represented by \a line_data.  Returns the index of the
    character one past the end of the text if \a line or \a index are out of
    bounds. */
GG_API StrSize StringIndexOf(std::size_t line, CPSize index,
                             const std::vector<Font::LineData>& line_data);

/** Returns the line L and the code point index within L of the
    <i>index</i>-th code point within the text represented by \a line_data.
    Returns (std::numeric_limits<std::size_t>::max(), INVALID_CP_SIZE) if \a
    index is out of bounds. */
GG_API std::pair<std::size_t, CPSize>
LinePositionOf(CPSize index, const std::vector<Font::LineData>& line_data);


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
        FontKey(const std::string& str, unsigned int pts); ///< Ctor.
        bool operator<(const FontKey& rhs) const; ///< Lexocograhpical ordering on filename then points.

        std::string  filename; ///< The name of the file from which this font was created.
        unsigned int points;   ///< The point size in which this font was rendered.
    };

public:
    /** \name Accessors */ ///@{
    /** Returns true iff this manager contains a font with the given filename
        and point size, regardless of charsets. */
    bool HasFont(const std::string& font_filename, unsigned int pts) const;

    /** Returns true iff this manager contains a font with the given filename
        and point size, containing the given charsets. */
    template <class CharSetIter>
    bool HasFont(const std::string& font_filename, unsigned int pts,
                 CharSetIter first, CharSetIter last) const;
    //@}

    /** \name Mutators */ ///@{
    /** Returns a shared_ptr to the requested font, supporting all printable
        ASCII characters.  \note May load font if unavailable at time of
        request. */
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts);

    /** Returns a shared_ptr to the requested font, supporting all printable
        ASCII characters, from the in-memory contents \a file_contents.  \note
        May load font if unavailable at time of request. */
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts,
                                  const std::vector<unsigned char>& file_contents);

    /** Returns a shared_ptr to the requested font, supporting all the
        code points in the UnicodeCharsets in the range [first, last).  \note
        May load font if unavailable at time of request. */
    template <class CharSetIter>
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts,
                                  CharSetIter first, CharSetIter last);

    /** Returns a shared_ptr to the requested font, supporting all the code
        points in the UnicodeCharsets in the range [first, last), from the
        in-memory contents \a file_contents.  \note May load font if
        unavailable at time of request. */
    template <class CharSetIter>
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts,
                                  const std::vector<unsigned char>& file_contents,
                                  CharSetIter first, CharSetIter last);

    /** Removes the indicated font from the font manager.  Due to shared_ptr
        semantics, the font may not be deleted until much later. */
    void                    FreeFont(const std::string& font_filename, unsigned int pts);
    //@}

private:
    FontManager();
    template <class CharSetIter>
    std::shared_ptr<Font> GetFontImpl(const std::string& font_filename, unsigned int pts,
                                      const std::vector<unsigned char>* file_contents,
                                      CharSetIter first, CharSetIter last);

    std::map<FontKey, std::shared_ptr<Font>> m_rendered_fonts;

    static const std::shared_ptr<Font> EMPTY_FONT;

    friend GG_API FontManager& GetFontManager();
};

/** Returns the singleton FontManager instance. */
GG_API FontManager& GetFontManager();

/** Thrown when initialization of the FreeType library fails. */
GG_EXCEPTION(FailedFTLibraryInit);

namespace detail {
    template <class CharT, bool CharIsSigned = boost::is_signed<CharT>::value>
    struct ValidUTFChar;

    template <class CharT>
    struct ValidUTFChar<CharT, true>
    {
        bool operator()(CharT c)
            { return 0x0 <= c; }
    };

    template <class CharT>
    struct ValidUTFChar<CharT, false>
    {
        bool operator()(CharT c)
            { return c <= 0x7f; }
    };

    struct GG_API FTFaceWrapper
    {
        FTFaceWrapper();
        ~FTFaceWrapper();
        FT_Face m_face;
    };
}

} // namespace GG


// template implementations
template <class CharSetIter>
GG::Font::Font(const std::string& font_filename, unsigned int pts,
               CharSetIter first, CharSetIter last) :
    m_font_filename(font_filename),
    m_pt_sz(pts),
    m_charsets(first, last),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_super_sub_offset(0.0),
    m_shadow_offset(0.0),
    m_space_width(0)
{
    if (!m_font_filename.empty()) {
        detail::FTFaceWrapper wrapper;
        FT_Error error = GetFace(wrapper.m_face);
        CheckFace(wrapper.m_face, error);
        Init(wrapper.m_face);
    }
}

template <class CharSetIter>
GG::Font::Font(const std::string& font_filename, unsigned int pts,
               const std::vector<unsigned char>& file_contents,
               CharSetIter first, CharSetIter last) :
    m_font_filename(font_filename),
    m_pt_sz(pts),
    m_charsets(first, last),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_super_sub_offset(0.0),
    m_shadow_offset(0.0),
    m_space_width(0)
{
    assert(!file_contents.empty());
    detail::FTFaceWrapper wrapper;
    FT_Error error = GetFace(file_contents, wrapper.m_face);
    CheckFace(wrapper.m_face, error);
    Init(wrapper.m_face);
}

template <class CharSetIter>
bool GG::FontManager::HasFont(const std::string& font_filename, unsigned int pts,
                              CharSetIter first, CharSetIter last) const
{
    bool retval = false;
    FontKey key(font_filename, pts);
    auto it = m_rendered_fonts.find(key);
    if (it != m_rendered_fonts.end()) {
        std::set<UnicodeCharset> requested_charsets(first, last);
        std::set<UnicodeCharset> found_charsets(it->second->UnicodeCharsets().begin(),
                                                it->second->UnicodeCharsets().end());
        retval = requested_charsets == found_charsets;
    }
    return retval;
}

template <class CharSetIter>
std::shared_ptr<GG::Font>
GG::FontManager::GetFont(const std::string& font_filename, unsigned int pts,
                         CharSetIter first, CharSetIter last)
{ return GetFontImpl(font_filename, pts, nullptr, first, last); }

template <class CharSetIter>
std::shared_ptr<GG::Font>
GG::FontManager::GetFont(const std::string& font_filename, unsigned int pts,
                         const std::vector<unsigned char>& file_contents,
                         CharSetIter first, CharSetIter last)
{ return GetFontImpl(font_filename, pts, &file_contents, first, last); }


template <class CharSetIter>
std::shared_ptr<GG::Font>
GG::FontManager::GetFontImpl(const std::string& font_filename, unsigned int pts,
                             const std::vector<unsigned char>* file_contents,
                             CharSetIter first, CharSetIter last)
{
    FontKey key(font_filename, pts);
    auto it = m_rendered_fonts.find(key);
    if (it == m_rendered_fonts.end()) { // if no such font has been created, create it now
        if (font_filename.empty()) {
            // keeps this function from throwing; "" is the only invalid font
            // filename that shouldn't throw
            return EMPTY_FONT;
        } else {
            std::shared_ptr<Font> font(
                file_contents ?
                new Font(font_filename, pts, *file_contents, first, last) :
                new Font(font_filename, pts, first, last)
            );
            m_rendered_fonts[key] = font;
            return m_rendered_fonts[key];
        }
    // if a font like this has been created, but it doesn't have all the right
    // glyphs, release it and create a new one
    } else {
        std::set<UnicodeCharset> requested_charsets(first, last);
        std::set<UnicodeCharset> found_charsets(it->second->UnicodeCharsets().begin(),
                                                it->second->UnicodeCharsets().end());
        if (requested_charsets != found_charsets) {
            std::vector<UnicodeCharset> united_charsets;
            std::set_union(requested_charsets.begin(), requested_charsets.end(),
                           found_charsets.begin(), found_charsets.end(),
                           std::back_inserter(united_charsets));
            m_rendered_fonts.erase(it);
            std::shared_ptr<Font> font(
                file_contents ?
                new Font(font_filename, pts, *file_contents,
                         united_charsets.begin(), united_charsets.end()) :
                new Font(font_filename, pts,
                         united_charsets.begin(), united_charsets.end())
            );
            m_rendered_fonts[key] = font;
            return m_rendered_fonts[key];
        } else { // otherwise, the font we found works, so just return it
            return it->second;
        }
    }
}

#endif
