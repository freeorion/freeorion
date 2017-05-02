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

/** \file Edit.h \brief Contains the Edit class, a single-line editable
    text-box control. */

#ifndef _GG_Edit_h_
#define _GG_Edit_h_

#include <GG/ClrConstants.h>
#include <GG/TextControl.h>

#include <boost/signals2/signal.hpp>


namespace GG {

/** \brief This is a single-line edit box control.

    Note that the constructor determines the height of the control based on
    the height of the font used and the value of the constant PIXEL_MARGIN.
    There are two types of signals emitted by an Edit control.  The first is
    EditedSignal; this is emitted every time the contents of the Edit change.
    Sometimes, however, you don't want that.  For instance, say you want to
    keep the value of the text in the Edit to between (numerical values) 100
    and 300.  If the Edit currently reads "200", the user may decide to
    highlight the "2", hit delete, then type a "1".  If updates are immediate,
    you will receive notification that the Edit says "00" (an invalid value),
    when that is just a temporary value you don't care about.  In such
    situations, the other signal, FocusUpdateSignal, should be useful.  It is
    only emitted when the Edit has changed and is losing focus, or one of
    enter or return is pressed.  So you would only receive a single update,
    namely "100", which is a valid number for that control, and you would
    receive it only when it is certain that the user is finished editing the
    text (when the focus changes).  Note that both signals may be used at the
    same time, if desired. */
class GG_API Edit : public TextControl
{
public:
    /** \name Signal Types */ ///@{
    /** Emitted whenever the text of the Edit are altered (keypresses,
        deletes, etc.); provides the new text of the Edit. */
    typedef boost::signals2::signal<void (const std::string&)> EditedSignalType;

    /** Emitted whenever the Edit has changed and has lost the input focus, or
        one of enter or return has been pressed; provides the new text of the
        Edit. */
    typedef boost::signals2::signal<void (const std::string&)> FocusUpdateSignalType;
    //@}

    /** \name Structors */ ///@{
    /** Ctor. Height is determined from the font and point size used. */
    Edit(const std::string& str, const std::shared_ptr<Font>& font, Clr color,
         Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO);
    //@}

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;
    Pt ClientUpperLeft() const override;
    Pt ClientLowerRight() const override;

    /** Returns the minimum usable size if the text were reflowed into a \a width box.*/
    Pt MinUsableSize(X width) const override;

    /** Returns the current position of the cursor (first selected character
        to one past the last selected one). */
    const std::pair<CPSize, CPSize>& CursorPosn() const;

    /** Returns the text that is selected in this control. */
    std::string SelectedText() const;

    /** Returns the color used to render the iterior of the control. */
    Clr InteriorColor() const;

    /** Returns the color used to render hiliting around selected text. */
    Clr HiliteColor() const;

    /** Returns the color used to render selected text. */
    Clr SelectedTextColor() const;

    /** The edited signal object for this Edit. */
    mutable EditedSignalType EditedSignal;

    /** The focus update signal object for this Edit. */
    mutable FocusUpdateSignalType FocusUpdateSignal;
    //@}

    /** \name Mutators */ ///@{
    void Render() override;

    void SetColor(Clr c) override;

    /** Sets the interior color of the control. */
    void SetInteriorColor(Clr c);

    /** Sets the color used to render hiliting around selected text. */
    void SetHiliteColor(Clr c);

    /** Sets the color used to render selected text. */
    void SetSelectedTextColor(Clr c);

    /** Selects all text in the given range.  When \a from == \a to, this
        function just places the caret at \a from.  Note that it is legal to
        pass values such that \a to < \a from.  The difference is that \a from
        < \a to simulates a drag-selection from left to right, and \a to < \a
        from simulates one from right to left.  The direction of the simulated
        drag affects which part of the text is visible at the end of the
        function call. */
    virtual void SelectRange(CPSize from, CPSize to);

    /** Selects all text in the entire control.  This function leaves the
        beginning of the text in view; see SelectRange(). */
    virtual void SelectAll();

    /** Deselects text */
    virtual void DeselectAll();

    void SetText(const std::string& str) override;

    /** Replaces selected text with, or inserts at cursor, the text in \a text. */
    virtual void AcceptPastedText(const std::string& text);
    //@}

    /** The number of pixels to leave between the text and the control's
        frame. */
    static const int PIXEL_MARGIN;

protected:
    /** \name Accessors */ ///@{
    /** Returns true if >= 1 characters selected. */
    virtual bool MultiSelected() const;

    /** Returns the index of the first character visible in the Edit. */
    CPSize FirstCharShown() const;

    /** Returns true iff the contents have been changed since the last time
        the focus was gained. */
    bool RecentlyEdited() const;

    /** Returns the index of the code point \a x pixels from left edge of
        visible portion of string. */
    CPSize CharIndexOf(X x) const;

    /** Returns the distance from the beginning of the string to just before
        the first visible character. */
    X FirstCharOffset() const;

    /** Returns the screen x-coordinate of the left side of the code point at
        index \a idx. */
    X ScreenPosOfChar(CPSize idx) const;

    /** Returns the last visible char (\a not one past the last visible
        char). */
    CPSize LastVisibleChar() const;

    /** Returns the value of GUI::Ticks() at the last left button press. */
    unsigned int LastButtonDownTime() const;

    /** Returns true iff the button is still down after being pressed twice
        within GUI::DoubleClickInterval() ticks. */
    bool InDoubleButtonDownMode() const;

    /** Returns the cursor position at the time of the most recent
        double-button-down. */
    std::pair<CPSize, CPSize> DoubleButtonDownCursorPos() const;

    /** Return the index of the last LineData() or 0 if LineData is empty.
     Allows index based Edit to handle empty line data.*/
    std::vector<GG::Font::LineData>::size_type NumLines() const;
    //@}

    /** \name Mutators */ ///@{
    void LButtonDown(const Pt& pt, Flags<ModKey> mod_keys) override;
    void LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys) override;
    void LButtonUp(const Pt& pt, Flags<ModKey> mod_keys) override;
    void LClick(const Pt& pt, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void TextInput(const std::string* text) override;
    void GainingFocus() override;
    void LosingFocus() override;

    /** Does a bit more than its name suggests.  Records the current time, and
        if it's within GUI::DoubleClickInterval() of the last button down
        time, returns the code point indices that delimit the word around
        index \a char_index.  If not within the time limit, or if no such word
        exists, the returned range will be empty (its .first and .second
        members will be equal).  This function should be called in
        LButtonDown() overrides. */
    virtual std::pair<CPSize, CPSize> GetDoubleButtonDownWordIndices(CPSize char_index);

    /** Returns the code point indices that delimit the word around index \a
        char_index.  If no such word exists, the returned range will be empty
        (its .first and .second members will be equal).  This function should
        be called in LDrag() overrides when InDoubleButtonDownMode() is
        true. */
    virtual std::pair<CPSize, CPSize> GetDoubleButtonDownDragWordIndices(CPSize char_index);

    /** Sets the value of InDoubleButtonDownMode() to false.  This should be
        called in LClick() and LButtonUp() overrides. */
    void ClearDoubleButtonDownMode();
    //@}

    /** If .first == .second, the caret is drawn before character at
        m_cursor_pos.first; otherwise, the range is selected (when range is
        selected, caret is considered at .second) */
    std::pair<CPSize, CPSize>   m_cursor_pos;

    unsigned int                m_last_button_down_time;
    bool                        m_in_double_click_mode;
    std::pair<CPSize, CPSize>   m_double_click_cursor_pos;

private:
    void ClearSelected(); ///< Clears (deletes) selected characters, as when a del, backspace, or character is entered
    void AdjustView();    ///< Makes sure the caret ends up in view after an arbitrary move

    CPSize       m_first_char_shown; ///< Index of the first character on the left end of the control's viewable area
    Clr          m_int_color;        ///< Color of background inside text box
    Clr          m_hilite_color;     ///< Color behind selected range
    Clr          m_sel_text_color;   ///< Color of selected text

    bool         m_recently_edited;  ///< The contents when the focus was last gained
};

void GG_API GetTranslatedCodePoint(Key key,
                                   std::uint32_t key_code_point,
                                   Flags<ModKey> mod_keys,
                                   std::string& translated_code_point);

CPSize GG_API NextWordEdgeFrom(const std::string& text,
                               CPSize from_position,
                               bool search_right = true);


} // namespace GG

#endif
