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

/** \file TextControl.h \brief Contains the TextControl class, a control which
    represents a certain text string in a certain font, justification, etc. */

#ifndef _GG_TextControl_h_
#define _GG_TextControl_h_

#include <GG/ClrConstants.h>
#include <GG/Control.h>
#include <GG/Font.h>

#include <boost/lexical_cast.hpp>


namespace GG {

class StateButton;

/** \brief Displays a piece of text.

    TextControl's know how to center, left- or right-justify, etc. themselves
    within their window areas.  The format flags used with TextControl are
    defined in the TextFormat flag type. TextControl has std::string-like
    operators and functions that allow the m_text member string to be
    manipulated directly.  In addition, the << and >> operators allow
    virtually any type (int, float, char, etc.) to be read from a TextControl
    object as if it were an input or output stream, thanks to
    boost::lexical_cast.  Note that the TextControl stream operators only read
    the first instance of the specified type from m_text, and overwrite the
    entire m_text string when writing to it; both operators may throw.

    <br>TextControl is based on pre-rendered font glyphs.  The text is
    rendered character by character from a prerendered font. The font used is
    gotten from the GUI's font manager.  Since a shared_ptr to the font is
    kept, the font is guaranteed to exist at least as long as the TextControl
    object that refers to it.  This also means that if the font is explicitly
    released from the font manager but is still held by at least one
    TextControl object, it will not be destroyed, due to the shared_ptr.  Note
    that if "" is supplied as the font_filename parameter, no text will be
    rendered, but a valid TextControl object will be constructed, which may
    later contain renderable text. TextControl objects support text with
    formatting tags. See GG::Font for details.*/
class GG_API TextControl : public Control
{
public:
    using Wnd::SetMinSize;

    /** \name Structors */ ///@{
    /** Ctor. */
    TextControl(X x, Y y, X w, Y h, const std::string& str,
                const std::shared_ptr<Font>& font,
                Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
                Flags<WndFlag> flags = NO_WND_FLAGS);

    /** Fast constructor.

     This constructor requires a \p str and \text_elements that are consistent
     with each other. Font::ExpensiveParseFromTextToTextElements() will not be
     called on \p str.  Hence this constructor is much faster than the first
     constructor.*/
    TextControl(X x, Y y, X w, Y h, const std::string& str,
                const std::vector<std::shared_ptr<Font::TextElement>>&text_elements,
                const std::shared_ptr<Font>& font,
                Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
                Flags<WndFlag> flags = NO_WND_FLAGS);

    /** Copy constructor.

        Text Control requires a copy-constructor because m_text_elements
        contains pointers to m_text which need to be bound with Bind() to the
        m_text in the new TextControl.

        Using the copy constructor is faster than constructing a TextControl
        with text from another TextControl because it avoids the XML parse
        overhead.

        Since Control does not have a way to access Flags the copy using default
        flags.
    */
    explicit TextControl(const TextControl& that);

    virtual ~TextControl();
    //@}

    /** Assignment operator.

        Text Control requires an assignment operator because m_text_elements
        contains pointers to m_text which need to be bound with Bind() to the
        m_text in this TextControl.

        Using the assignment operator is faster than using SetText() with text
        from another TextControl because it avoids the XML parse overhead.
    */
    TextControl& operator=(const TextControl& that);

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;

    /** Returns the minimum usable size if the text were reflowed into a \a width box.*/
    virtual Pt MinUsableSize(X width) const;

    /** Returns the text displayed in this control. */
    const std::string& Text() const;

    /** Returns the text displayed in this control between the specified
        position \a from through position \a to. */
    std::string Text(CPSize from, CPSize to) const;

    /** Returns the text format (vertical and horizontal justification, use of
        word breaks and line wrapping, etc.) */
    Flags<TextFormat> GetTextFormat() const;

    /** Returns the text color (this may differ from the Control::Color() in
        some subclasses) */
    Clr               TextColor() const;

    /** Returns true iff the text control clips its text to its client area;
        by default this is not done. */
    bool              ClipText() const;

    /** Returns true iff the text control sets its MinSize() when the bounds
        of its text change because of a call to SetText() or SetTextFormat();
        by default this is not done.  The minimum size of the control in each
        dimension will be the larger of the text size and the current
        MinSize(), if any has been set.  Note that this operates independently
        of fit-to-text behavior, which sets the window size, not its minimum
        size. */
    bool              IsResetMinSize() const;

    /** Sets the value of \a t to the interpreted value of the control's text.
        If the control's text can be interpreted as an object of type T by
        boost::lexical_cast (and thus by a stringstream), then the >> operator
        will do so.  Note that the return type is void, so multiple >>
        operations cannot be strung together.  Also, because lexical_cast
        attempts to convert the entire contents of the string to a single
        value, a TextControl containing the string "4.15 3.8" will fill a
        float with 0.0 (the default construction of float), even though there
        is a perfectly valid 4.15 value that occurs first in the string.
        \note boost::lexical_cast usually throws boost::bad_lexical_cast when
        it cannot perform a requested cast, though >> will return a
        default-constructed T if one cannot be deduced from the control's
        text. */
    template <class T> void operator>>(T& t) const;

    /** Returns the value of the control's text, interpreted as an object of
        type T.  If the control's text can be interpreted as an object of type
        T by boost::lexical_cast (and thus by a stringstream), then GetValue()
        will do so.  Because lexical_cast attempts to convert the entire
        contents of the string to a single value, a TextControl containing the
        string "4.15 3.8" will throw, even though there is a perfectly valid
        4.15 value that occurs first in the string.  \throw
        boost::bad_lexical_cast boost::lexical_cast throws
        boost::bad_lexical_cast when it cannot perform a requested cast. This
        is handy for validating data in a dialog box; Otherwise, using
        operator>>(), you may get the default value, even though the text in
        the control may not be the default value at all, but garbage. */
    template <class T> T GetValue() const;

    /** Returns the control's text; allows TextControl's to be used as
        std::string's. */
    operator const std::string&() const;

    bool   Empty() const;   ///< Returns true iff text string equals "".
    CPSize Length() const;  ///< Returns the number of code points in the text.

    /** Returns the upper-left corner of the text as it is would be rendered
        if it were not bound to the dimensions of this control. */
    Pt TextUpperLeft() const;

    /** Returns the lower-right corner of the text as it is would be rendered
        if it were not bound to the dimensions of this control. */
    Pt TextLowerRight() const;
    //@}

    /** \name Mutators */ ///@{
    void Render() override;

    void SizeMove(const Pt& ul, const Pt& lr) override;

    /** Just like Control::SetColor(), except that this one also adjusts the
        text color. */
    void SetColor(Clr c) override;

    /** Sets the text displayed in this control to \a str.  May resize the
        window.  If the control was constructed with FORMAT_NOWRAP, calls
        to this function cause the window to be resized to whatever space
        the newly rendered text occupies. */
    virtual void SetText(const std::string& str);

    /** Sets the text displayed in this control to the \p str \p text_elements
        pair.  This is faster than SetText without \p text_elements.

        This may resize the window.  If the control was constructed with FORMAT_NOWRAP, calls to
        this function cause the window to be resized to whatever space the newly rendered text
        occupies.

        If the \p str and \p text_elements are inconsistent and \p str is shorter than expected
        from examining \p text_elements then it will return without changing the TextControl.
    */
    virtual void SetText(const std::string& str,
                         const std::vector<std::shared_ptr<Font::TextElement>>&text_elements);

    /** Change TextControl's text to replace the text at templated \p targ_offset with \p new_text.

        This replaces the entire text of the TextElement at offset \p targ_offset and adjusts the
        string \p text to be consistent even if the \p new_text is longer/shorter than the original
        TEXT type TextElement.

        This does not reparse the TextControl's text. It is faster than SetText on a new
        string. It will not find white space in the inserted text.

        \p targ_offset is the zero based offset of the TextElements of type TEXT.  It ignores
        other types of TextElements such as TAGS, WHITESPACE and NEWLINE, when determining the
        offset.

        Here is an example of changing a ship name from "oldname" to "New Ship Name":

        original text:             "<i>Ship:<\i> oldname ID:"
        orignal m_text_elements:   [<OPEN_TAG i>, <TEXT "Ship:">, <CLOSE_TAG i>, <WHITESPACE>, <TEXT oldname>, <WHITESPACE>, <TEXT ID:>]

        ChangeTemplatedText(text, text_elements, "New Ship Name", 1);

        changed text:              "<i>Ship:<\i> New Ship Name ID:"
        changed m_text_elements:   [<OPEN_TAG i>, <TEXT "Ship:">, <CLOSE_TAG i>, <WHITESPACE>, <TEXT New Ship Name>, <WHITESPACE>, <TEXT ID:>]

    */

    void ChangeTemplatedText(const std::string& new_text, size_t targ_offset);

    /** Returns the Font used by this TextControl to render its text. */
    const std::shared_ptr<Font>& GetFont() const;

    /** Sets the Font used by this TextControl to render its text. */
    void SetFont(std::shared_ptr<Font> font);

    /** Sets the text format; ensures that the flags are sane. */
    void         SetTextFormat(Flags<TextFormat> format);

    /** Sets the text color. */
    void         SetTextColor(Clr color);

    /** Enables/disables text clipping to the client area. */
    void         ClipText(bool b);

    /** Enables/disables setting the minimum size of the window to be the text
        size. */
    void         SetResetMinSize(bool b);

    /** Sets the value of the control's text to the stringified version of t.
        If t can be converted to a string representation by a
        boost::lexical_cast (and thus by a stringstream), then the << operator
        will do so, e.g. double(4.15) to string("4.15").  Note that the return
        type is void, so multiple << operations cannot be strung together.
        \throw boost::bad_lexical_cast boost::lexical_cast throws
        boost::bad_lexical_cast when it is confused.*/
    template <class T>
    void operator<<(T t);

    void  operator+=(const std::string& s); ///< Appends \a s to text.
    void  operator+=(char c);               ///< Appends \a c to text.
    void  Clear();                          ///< Sets text to the empty string.

    /** Inserts \a c at position \a pos within the text.  \note Just as with
        most string parameters throughout GG, \a c must be a valid UTF-8
        sequence. */
    void  Insert(CPSize pos, char c);

    /** Inserts \a s at position \a pos within the text. */
    void  Insert(CPSize pos, const std::string& s);

    /** Erases \a num code points from the text starting at position \a
        pos up to the end of the line that pos is on. */
    void  Erase(CPSize pos, CPSize num = CP1);

    /** Inserts \a c at text position \a pos within line \a line.  \note Just
        as with most string parameters throughout GG, \a c must be a valid
        UTF-8 sequence. */
    void  Insert(std::size_t line, CPSize pos, char c);

    /** Inserts \a s at text position \a pos within line \a line. */
    void  Insert(std::size_t line, CPSize pos, const std::string& s);

    /** Erases \a num code points from the text starting at position \a
        pos within line \a line up to the end of the line \a line. */
    void  Erase(std::size_t line, CPSize pos, CPSize num = CP1);

    /** Erases code points from the text between the specified starting and
      * ending line and character positions. */
    void  Erase(std::size_t line1, CPSize pos1, std::size_t line2, CPSize pos2);
    //@}

protected:
    /** \name Accessors */ ///@{
    /** Returns the line data for the text in this TextControl. */
    virtual const std::vector<Font::LineData>& GetLineData() const;

    friend class StateButtonRepresenter;
    //@}

private:
    void ValidateFormat();      ///< ensures that the format flags are consistent
    void AdjustMinimumSize();
    void RecomputeTextBounds(); ///< recalculates m_text_ul and m_text_lr
    void RefreshCache();
    void PurgeCache();

    /** Recompute line data, code points, text extent and minusable size cache when
        m_text_elements changes.*/
    void RecomputeLineData();

    std::string                 m_text;
    Flags<TextFormat>           m_format;      ///< the formatting used to display the text (vertical and horizontal alignment, etc.)
    Clr                         m_text_color;  ///< the color of the text itself (may differ from GG::Control::m_color)
    bool                        m_clip_text;
    bool                        m_set_min_size;
    std::vector<std::shared_ptr<Font::TextElement>> m_text_elements;
    std::vector<Font::LineData> m_line_data;
    CPSize                      m_code_points;
    std::shared_ptr<Font> m_font;
    Pt                          m_text_ul;     ///< stored relative to the control's UpperLeft()
    Pt                          m_text_lr;     ///< stored relative to the control's UpperLeft()
    std::unique_ptr<Font::RenderCache> m_render_cache;///< Cache much of text rendering.

    mutable X                   m_cached_minusable_size_width;
    mutable Pt                  m_cached_minusable_size;
};

typedef TextControl Label;

} // namespace GG

// template implementations
template <class T>
void GG::TextControl::operator>>(T& t) const
{
    try {
        t = boost::lexical_cast<T>(m_text);
    } catch (boost::bad_lexical_cast) {
        t = T();
    }
}

template <class T>
T GG::TextControl::GetValue() const
{ return boost::lexical_cast<T, std::string>(m_text); }

template <class T>
void GG::TextControl::operator<<(T t)
{ SetText(boost::lexical_cast<std::string>(t)); }

#endif
