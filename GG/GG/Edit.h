//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Edit.h
//!
//! Contains the Edit class, a single-line editable text-box control.

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
    /** Emitted whenever the text of the Edit are altered (keypresses,
        deletes, etc.); provides the new text of the Edit. */
    typedef boost::signals2::signal<void (const std::string&)> EditedSignalType;

    /** Emitted whenever the Edit has changed and has lost the input focus, or
        one of enter or return has been pressed; provides the new text of the
        Edit. */
    typedef boost::signals2::signal<void (const std::string&)> FocusUpdateSignalType;

    /** Ctor. Height is determined from the font and point size used. */
    Edit(std::string str, std::shared_ptr<Font> font, Clr color,
         Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO);

    Pt MinUsableSize() const noexcept override;
    Pt ClientUpperLeft() const noexcept override { return UpperLeft() + Pt(X(PIXEL_MARGIN), Y(PIXEL_MARGIN)); }
    Pt ClientLowerRight() const noexcept override { return LowerRight() - Pt(X(PIXEL_MARGIN), Y(PIXEL_MARGIN)); }

    /** Returns the minimum usable size if the text were reflowed into a \a width box.*/
    Pt MinUsableSize(X width) const override { return MinUsableSize(); }

    /** Returns the current position of the cursor (first selected character
        to one past the last selected one). */
    auto CursorPosn() const noexcept { return m_cursor_pos; }

    /** Returns the text that is selected in this control. */
    std::string_view SelectedText() const;

    /** Returns the color used to render the iterior of the control. */
    Clr InteriorColor() const noexcept { return m_int_color; }

    /** Returns the color used to render hiliting around selected text. */
    Clr HiliteColor() const noexcept { return m_hilite_color; }

    /** The edited signal object for this Edit. */
    mutable EditedSignalType EditedSignal;

    /** The focus update signal object for this Edit. */
    mutable FocusUpdateSignalType FocusUpdateSignal;

    void Render() override;

    void SetColor(Clr c) noexcept override { Control::SetColor(c); }

    /** Sets the interior color of the control. */
    void SetInteriorColor(Clr c) noexcept { m_int_color = c; }

    /** Sets the color used to render hiliting around selected text. */
    void SetHiliteColor(Clr c) noexcept { m_hilite_color = c; }

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

    void SetText(std::string str) override;

    /** Replaces selected text with, or inserts at cursor, the text in \a text. */
    virtual void AcceptPastedText(const std::string& text);

    /** The number of pixels to leave between the text and the control's frame. */
    static constexpr int PIXEL_MARGIN = 5;

protected:
    /** Returns true if >= 1 characters selected. */
    virtual bool MultiSelected() const noexcept { return m_cursor_pos.first != m_cursor_pos.second; }

    /** Returns the index of the first character visible in the Edit. */
    CPSize FirstCharShown() const noexcept { return m_first_char_shown; }

    /** Returns true iff the contents have been changed since the last time
        the focus was gained. */
    bool RecentlyEdited() const noexcept { return m_recently_edited; }

    /** Returns the index of the glyph \a x pixels from left edge of
        visible portion of string. */
    CPSize GlyphIndexAt(X x) const;

    /** Returns the code point index of the start of the glyph \a x pixels from left edge of
        visible portion of string. */
    CPSize CPIndexOfGlyphAt(X x) const;

    /** Returns the distance from the beginning of the string to just before
        the first visible character. */
    X FirstCharOffset() const;

    /** Returns the screen x-coordinate of the left side of the code point at
        index \a idx. */
    X ScreenPosOfChar(CPSize idx) const;

    /** Returns the last visible char (\a not one past the last visible char). */
    CPSize LastVisibleChar() const;

    /** Returns the value of GUI::Ticks() at the last left button press. */
    auto LastButtonDownTime() const noexcept { return m_last_button_down_time; }

    /** Returns true iff the button is still down after being pressed twice
        within GUI::DoubleClickInterval() ticks. */
    bool InDoubleButtonDownMode() const noexcept { return m_in_double_click_mode; }

    /** Returns the cursor position at the time of the most recent
        double-button-down. */
    auto DoubleButtonDownCursorPos() const noexcept { return m_double_click_cursor_pos; }

    /** Return the index of the last LineData() or 0 if LineData is empty.
     Allows index based Edit to handle empty line data.*/
    std::size_t NumLines() const noexcept;

    void LButtonDown(Pt pt, Flags<ModKey> mod_keys) override;
    void LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys) override;
    void LButtonUp(Pt pt, Flags<ModKey> mod_keys) override;
    void LClick(Pt pt, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void TextInput(const std::string& text) override;
    void GainingFocus() override;
    void LosingFocus() override;

    /** Does a bit more than its name suggests.  Records the current time, and
        if it's within GUI::DoubleClickInterval() of the last button down
        time, returns the code point indices that delimit the word around
        index \a char_index.  If not within the time limit, or if no such word
        exists, the returned range will be empty (its .first and .second
        members will be equal).  This function should be called in
        LButtonDown() overrides. */
    virtual std::pair<CPSize, CPSize> GetDoubleButtonDownWordIndices(CPSize cp_index);

    /** Returns the code point indices that delimit the word around index \a
        char_index.  If no such word exists, the returned range will be empty
        (its .first and .second members will be equal).  This function should
        be called in LDrag() overrides when InDoubleButtonDownMode() is
        true. */
    virtual std::pair<CPSize, CPSize> GetDoubleButtonDownDragWordCPIndices(CPSize cp_index);

    /** Sets the value of InDoubleButtonDownMode() to false.  This should be
        called in LClick() and LButtonUp() overrides. */
    void ClearDoubleButtonDownMode();

    /** Code point indices (not glyphs) of cursor.
        If .first == .second, the caret is drawn before character at
        m_cursor_pos.first; otherwise, the range is selected (when range is
        selected, caret is considered at .second) */
    std::pair<CPSize, CPSize> m_cursor_pos = {CP0, CP0};
    unsigned int              m_last_button_down_time = 0;
    bool                      m_in_double_click_mode = false;
    std::pair<CPSize, CPSize> m_double_click_cursor_pos = {CP0, CP0};

private:
    void ClearSelected(); ///< Clears (deletes) selected characters, as when a del, backspace, or character is entered
    void AdjustView();    ///< Makes sure the caret ends up in view after an arbitrary move

    CPSize m_first_char_shown = CP0;    ///< Index of the first character on the left end of the control's viewable area
    Clr    m_int_color;                 ///< Color of background inside text box
    Clr    m_hilite_color = CLR_SHADOW; ///< Color behind selected range

    bool   m_recently_edited = false; ///< The contents when the focus was last gained
};

void GG_API GetTranslatedCodePoint(Key key, uint32_t key_code_point,
                                   Flags<ModKey> mod_keys, std::string& translated_code_point);

CPSize GG_API NextWordEdgeFrom(std::string_view text, CPSize from_position, bool search_right = true);

}


#endif
