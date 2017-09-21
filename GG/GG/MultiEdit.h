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

/** \file MultiEdit.h \brief Contains the MultiEdit class, a multi-line text
    box control. */

#ifndef _GG_MultiEdit_h_
#define _GG_MultiEdit_h_

#include <GG/Edit.h>
#include <GG/MultiEditFwd.h>


namespace GG {

class Scroll;

/** \brief This is a multi-line text input and display control.

    MultiEdit is designed to be used as a basic text-input control for text
    longer than one line, or to display large amounts of formatted or
    unformatted text.  MultiEdit supports text formatting tags.  See GG::Font
    for details.  Several style flags are available.  If the
    MULTI_TERMINAL_STYLE flag is in use, lines that exceed the history limit
    will be removed from the beginning of the text; otherwise, they are
    removed from the end.  If either MULTI_LINEWRAP of MULTI_WORDBREAK are in
    use, MULTI_NO_HSCROLL must be in use as well.  MULTI_VCENTER is not an
    allowed style; if it is specified, MULTI_TOP will be used in its place.
    The justification introduced by text formatting tags is very different
    from that introduced by the TF_* styles.  The former justifies lines
    within the space taken up by the text.  The latter justifies the entire
    block of text within the client area of the control.  So if you specify
    MULTI_LEFT and use \<right> formatting tags on the entire text, the text
    will appear to be right-justified, but you will probably only see the
    extreme left of the text area without scrolling.  If none of the no-scroll
    style flags are in use, the scrolls are created and destroyed
    automatically, as needed. */
class GG_API MultiEdit : public Edit
{
public:
    /** \name Structors */ ///@{
    /** Ctor. */
    MultiEdit(const std::string& str, const std::shared_ptr<Font>& font,
              Clr color, Flags<MultiEditStyle> style = MULTI_LINEWRAP,
              Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO);

    virtual ~MultiEdit();
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;

    Pt ClientLowerRight() const override;

    /** Returns the size to show the whole text without scrollbars. */
    Pt FullSize() const;

    /** Returns the style flags for this MultiEdit. */
    Flags<MultiEditStyle> Style() const;

    /** Returns the maximum number of lines of text that the control
        keeps. This number includes the lines that are visible in the control.
        A value of ALL_LINES indicates that there is no limit. */
    std::size_t MaxLinesOfHistory() const;

    /** Returns the positions of the scrollbars. */
    Pt ScrollPosition() const;
    //@}

    /** \name Mutators */ ///@{
    void Render() override;

    void SizeMove(const Pt& ul, const Pt& lr) override;

    void SelectAll() override;
    void DeselectAll() override;
    void SetText(const std::string& str) override;

    /** Sets the style flags for this MultiEdit to \a style. */
    void SetStyle(Flags<MultiEditStyle> style);

    /** Sets the maximum number of rows of text that the control will keep.
        ALL_LINES indicates no limit. */
    void SetMaxLinesOfHistory(std::size_t max);

    /** Sets scroll position, bound by valid range of scrolls of this MultiEdit. */
    void SetScrollPosition(Pt pt);

    /** Sets how much to scroll when scrolled using the mousewheel. */
    void SetVScrollWheelIncrement(unsigned int increment);
    void SetHScrollWheelIncrement(unsigned int increment);

    void AcceptPastedText(const std::string& text) override;
    //@}

    /** A sentinel value that indicates that there is no limit on the number
        of lines of history are to be stored.  \see MaxLinesOfHistory() \see
        SetMaxLinesOfHistory() */
    static const std::size_t ALL_LINES;

    /** The thickness with which to render the border of the control. */
    static const unsigned int BORDER_THICK;

protected:
    /** \name Accessors */ ///@{
    /** Returns true if >= 1 characters are selected. */
    bool MultiSelected() const override;

    /** Returns the width of the scrollbar on the right side of the control (0
        if none). */
    X RightMargin() const;

    /** Returns the width of the scrollbar at the bottom of the control (0 if
        none). */
    Y BottomMargin() const;

    /** Returns row and character index of \a pt, or (0, 0) if \a pt falls
        outside the text.  \a pt is in client-space coordinates. */
    std::pair<std::size_t, CPSize> CharAt(const Pt& pt) const;

    /** Returns row and character index of char at \a idx, or (0, 0) if \a idx
        falls outside the text, or if \a idx refers to a non-visible
        character. */
    std::pair<std::size_t, CPSize> CharAt(CPSize idx) const;

    /** Returns the code point index of the start of the UTF-8 sequence for
        the code point at \a <i>char_idx</i> in row \a row, using \a line_data
        instead of the current line data, if it is supplied.  If \a row, \a
        char_idx refers to a character preceeded by formatting tags, the index
        of the first character of the first formatting tag is returned instead.
        Not range-checked. */
    CPSize CharIndexOf(std::size_t row, CPSize char_idx,
                       const std::vector<Font::LineData>* line_data = nullptr) const;

    /** Returns the the x-coordinate of the beginning of row \a row, in
        cleint-space coordinates.  Not range-checked. */
    X RowStartX(std::size_t row) const;

    /** Returns the distance from the start of row \a row to the character at
        index idx.  Not range-checked. */
    X CharXOffset(std::size_t row, CPSize idx) const;

    /** Returns the line that falls under Y coordinate \a y.  \a y must be in
        client-space coordinates. */
    std::size_t RowAt(Y y) const;

    /** Returns the index of the character in row \a row that falls under X
        coordinate \a x.  \a x must be in client-space coordinates. */
    CPSize CharAt(std::size_t row, X x) const;

    /** Returns the index of the first visible row, or 0 if none. */
    std::size_t FirstVisibleRow() const;

    /** Returns the index of the last visible row, or 0 if none. */
    std::size_t LastVisibleRow() const;

    /** Returns the index of the first fully visible row, or 0 if none. */
    std::size_t FirstFullyVisibleRow() const;

    /** Returns the index of the last fully visible row, or 0 if none. */
    std::size_t LastFullyVisibleRow() const;

    /** Returns the index of the first visible character of row \a row, or 0
        if none. */
    CPSize FirstVisibleChar(std::size_t row) const;

    /** Returns the index of the last visible character of row \a row, or 0 if
        none. */
    CPSize LastVisibleChar(std::size_t row) const;

    /** Returns the greater of m_cursor_begin and m_cursor_end. */
    std::pair<std::size_t, CPSize> HighCursorPos() const;

    /** Returns the lesser of m_cursor_begin and m_cursor_end. */
    std::pair<std::size_t, CPSize> LowCursorPos() const;
    //@}

    /** \name Mutators */ ///@{
    void LButtonDown(const Pt& pt, Flags<ModKey> mod_keys) override;
    void LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys) override;
    void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void TextInput(const std::string* text) override;

    /** Recreates the vertical and horizontal scrolls as needed. */
    void RecreateScrolls();

    /** Ensures that the next call to SetText() preserves the positioning of
        the text.  This should only be called if it is known that the call to
        SetText() will not put the text-position in an illegal state.  For
        instance, if creating a MultiEdit that contains hyperlink text then
        coloring or underlining a link may require a call to SetText(), but
        may be guaranteed not to change the text layout.  Without a call to
        this function, the scroll positions will be reset. */
    void PreserveTextPositionOnNextSetText();

    std::pair<CPSize, CPSize> GetDoubleButtonDownWordIndices(CPSize char_index) override;
    //@}

    /** The width used to create the control's vertical and horizontal
        Scrolls. */
    static const unsigned int SCROLL_WIDTH;

private:
    void    ValidateStyle();
    void    ClearSelected();   ///< Clears (deletes) selected characters, as when a del, backspace, or character is entered
    void    AdjustView();      ///< Makes sure the caret ends up in view after an arbitrary move or SetText()
    void    AdjustScrolls();   ///< Sets the sizes of the scroll-space and the screen-space of the scrolls
    void    VScrolled(int upper, int lower, int range_upper, int range_lower);
    void    HScrolled(int upper, int lower, int range_upper, int range_lower);

    Flags<MultiEditStyle> m_style;

    std::pair<std::size_t, CPSize> m_cursor_begin; ///< The row and character index of the first character in the hilited selection
    std::pair<std::size_t, CPSize> m_cursor_end;   ///< The row and character index + 1 of the last character in the hilited selection
    // if m_cursor_begin == m_cursor_end, the caret is draw at m_cursor_end

    Pt              m_contents_sz;          ///< The size of the entire text block in the control (not just the visible part)

    X               m_first_col_shown;      ///< The position (counted from the left side of the text) of the first pixel shown
    Y               m_first_row_shown;      ///< The position (counted from the top of the text) of the first pixel shown

    std::size_t     m_max_lines_history;

    std::shared_ptr<Scroll>         m_vscroll;
    std::shared_ptr<Scroll>         m_hscroll;
    unsigned int    m_vscroll_wheel_scroll_increment;
    unsigned int    m_hscroll_wheel_scroll_increment;

    bool            m_preserve_text_position_on_next_set_text;
    bool            m_ignore_adjust_scrolls;
};

} // namespace GG

#endif
