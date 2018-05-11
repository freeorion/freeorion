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

/** \file Button.h \brief Contains the Button push-button control class; the
    StateButton control class, which represents check boxes and radio buttons;
    and the RadioButtonGroup control class, which allows multiple radio
    buttons to be combined into a single control. */

#ifndef _GG_Button_h_
#define _GG_Button_h_

#include <GG/ClrConstants.h>
#include <GG/TextControl.h>
#include <GG/Enum.h>

#include <boost/signals2/signal.hpp>


namespace GG {

/** \brief This is a basic button control.

    Has three states: BN_UNPRESSED, BN_PRESSED, and BN_ROLLOVER.  BN_ROLLOVER
    is when the cursor "rolls over" the button, without depressing it,
    allowing rollover effects on the button.  To create a bitmap button,
    simply set the unpressed, pressed, and/or rollover graphics to the desired
    SubTextures. \see GG::SubTexture */
class GG_API Button : public Control
{
public:
    /// the states of being for a GG::Button
    GG_CLASS_ENUM(ButtonState,
        BN_PRESSED,    ///< The button is being pressed by the user, and the cursor is over the button
        BN_UNPRESSED,  ///< The button is unpressed
        BN_ROLLOVER    ///< The button has the cursor over it, but is unpressed
    )

    /** \name Signal Types */ ///@{
    /** Emitted when the button is clicked by the user */
    typedef boost::signals2::signal<void ()> ClickedSignalType;
    //@}

   /** \name Structors */ ///@{
    Button(const std::string& str, const std::shared_ptr<Font>& font, Clr color,
           Clr text_color = CLR_BLACK, Flags<WndFlag> flags = INTERACTIVE);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;

    /** Returns button state \see ButtonState */
    ButtonState State() const;

    const std::string& Text() const;             ///< Returns the label to be used as the button label
    const SubTexture& UnpressedGraphic() const;  ///< Returns the SubTexture to be used as the image of the button when unpressed
    const SubTexture& PressedGraphic() const;    ///< Returns the SubTexture to be used as the image of the button when pressed
    const SubTexture& RolloverGraphic() const;   ///< Returns the SubTexture to be used as the image of the button when it contains the cursor, but is not pressed

    /** The left clicked signal object for this Button */
    mutable ClickedSignalType LeftClickedSignal;
    /** The right clicked signal object for this Button */
    mutable ClickedSignalType RightClickedSignal;
    /** The left pressed signal object for this Button */
    mutable ClickedSignalType LeftPressedSignal;
    /** The right pressed signal object for this Button */
    mutable ClickedSignalType RightPressedSignal;
    //@}

    /** \name Mutators */ ///@{
    void Show() override;
    void Render() override;
    void SizeMove(const Pt& ul, const Pt& lr) override;

    /** Sets the control's color; does not affect the text color. */
    void SetColor(Clr c) override;

    /** Sets button state programmatically \see ButtonState */
    void SetState(ButtonState state);

    void SetText(const std::string& text);          ///< Sets the text to be used as the button label
    void SetUnpressedGraphic(const SubTexture& st); ///< Sets the SubTexture to be used as the image of the button when unpressed
    void SetPressedGraphic(const SubTexture& st);   ///< Sets the SubTexture to be used as the image of the button when pressed
    void SetRolloverGraphic(const SubTexture& st);  ///< Sets the SubTexture to be used as the image of the button when it contains the cursor, but is not pressed
    //@}

protected:
    /** \name Mutators */ ///@{
    void LButtonDown(const Pt& pt, Flags<ModKey> mod_keys) override;
    void LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys) override;
    void LButtonUp(const Pt& pt, Flags<ModKey> mod_keys) override;
    void LClick(const Pt& pt, Flags<ModKey> mod_keys) override;
    void RButtonDown(const Pt& pt, Flags<ModKey> mod_keys) override;
    void RDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys) override;
    void RButtonUp(const Pt& pt, Flags<ModKey> mod_keys) override;
    void RClick(const Pt& pt, Flags<ModKey> mod_keys) override;
    void MouseEnter(const Pt& pt, Flags<ModKey> mod_keys) override;
    void MouseHere(const Pt& pt, Flags<ModKey> mod_keys) override;
    void MouseLeave() override;

    /** Draws the button unpressed.  If an unpressed graphic has been supplied, it is used. */
    virtual void RenderUnpressed();
    /** Draws the button pressed.  If an pressed graphic has been supplied, it is used. */
    virtual void RenderPressed();
    /** Draws the button rolled-over.  If an rollover graphic has been supplied, it is used. */
    virtual void RenderRollover();
    //@}

    std::shared_ptr<TextControl> m_label;   ///< Label used to display text

private:
    void RenderDefault();     ///< This just draws the default unadorned square-and-rectangle button

    ButtonState    m_state;             ///< Button is always in exactly one of the ButtonState states above
    SubTexture     m_unpressed_graphic; ///< Graphic used to display button when it's unpressed
    SubTexture     m_pressed_graphic;   ///< Graphic used to display button when it's depressed
    SubTexture     m_rollover_graphic;  ///< Graphic used to display button when it's under the mouse and not pressed
};


class StateButtonRepresenter;


/** \brief This is a basic state button control.

    This class is for checkboxes and radio buttons, etc.  The button/checkbox
    area is determined from the text height and format; the button height and
    width will be the text height, and the the button will be positioned to
    the left of the text and vertically the same as the text, unless the text
    is centered, in which case the button and text will be centered, and the
    button will appear above or below the text.  Whenever there is not room to
    place the button and the text in the proper orientation because the entire
    control's size is too small, the button and text are positioned in their
    default spots (button on left, text on right, centered vertically). */
class GG_API StateButton : public Control
{
public:
    /// the states of being for a GG::Button
    GG_CLASS_ENUM(ButtonState,
        BN_PRESSED,    ///< The button is being pressed by the user, and the cursor is over the button
        BN_UNPRESSED,  ///< The button is unpressed
        BN_ROLLOVER    ///< The button has the cursor over it, but is unpressed
    )

    /** \name Signal Types */ ///@{
    /** Emitted when the StateButton is checked or unchecked; the checked or
        unchecked status is indicated by the bool parameter */
    typedef boost::signals2::signal<void (bool)> CheckedSignalType;
    //@}

    /** \name Structors */ ///@{
    StateButton(const std::string& str, const std::shared_ptr<Font>& font, Flags<TextFormat> format,
                Clr color, std::shared_ptr<StateButtonRepresenter> representer, Clr text_color = CLR_BLACK); ///< Ctor
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ ///@{
    Pt                  MinUsableSize() const override;

    /** Returns button state \see ButtonState */
    ButtonState         State() const;

    const std::string&  Text() const;        ///< Returns the label to be used as the button label

    bool                Checked() const;       ///< Returns true if button is checked

    TextControl*        GetLabel() const;

    mutable CheckedSignalType CheckedSignal; ///< The checked signal object for this StaticButton
    //@}

    /** \name Mutators */ ///@{
    void Show() override;
    void Render() override;
    void SizeMove(const Pt& ul, const Pt& lr) override;

    void Reset();                 ///< Unchecks button
    void SetCheck(bool b = true); ///< (Un)checks button
    void SetTextColor(Clr c); ///< Sets the color of the box label text
    //@}

protected:
    /** \name Mutators */ ///@{
    void LButtonDown(const Pt& pt, Flags<ModKey> mod_keys) override;
    void LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys) override;
    void LButtonUp(const Pt& pt, Flags<ModKey> mod_keys) override;
    void LClick(const Pt& pt, Flags<ModKey> mod_keys) override;
    void MouseHere(const Pt& pt, Flags<ModKey> mod_keys) override;
    void MouseLeave() override;

    /** Sets button state programmatically \see ButtonState */
    void SetState(ButtonState next_state);
    //@}

private:
    std::shared_ptr<StateButtonRepresenter> m_representer;
    std::shared_ptr<TextControl>            m_label;       ///< Label used to display text
    ButtonState                             m_state;       ///< Button is always in exactly one of the ButtonState states above
    bool                                    m_checked;     ///< true when this button in a checked, active state
};


/** \brief Visual representation of a state button.

    The StateButtonRepresenter is a stub interface to implement a visual
    representation of a StateButton instance.  A representer can be shared
    amongst multiple instances of a StateButton, so no subclass should
    store instance specific attributes of a state button.
 */
class GG_API StateButtonRepresenter
{
public:
    /** \brief Render the given state button according to its state.

        \param button The StateButton instance to render.
     */
    virtual void Render(const StateButton& button) const;

    /** \brief Respong to a button state change.

        This method is called whenever a Button chanes its state.

        \param button  The button that changed its state and is in the most
                       recent state.
        \param previous_state  The previous button state.
     */
    virtual void OnChanged(const StateButton& button, StateButton::ButtonState previous_state) const;

    /** \brief Respond to a state button change.

        This method is called whenever a StateButton changes its
        state.

        \param checked True if the state button was checked, False
                       if not.
     */
    virtual void OnChecked(bool checked) const;

    /** \brief Return the minimum size required for a usable representation

        \param button The state button to calculate the minimum size of.
        \return The minimum size to create a usable representation of the
           given state button.
     */
    virtual Pt MinUsableSize(const StateButton& button) const;

protected:
    /** \brief Calculate the layout of text and button component.

        \param button The StateButton instance to layout.
        \param button_ul[out] The upper left corner of the visual
               button.
        \param button_lr[out] The lower right corner of the visual
               button.
        \param text_ul[out] The upper left corner of the button
               label.
     */
    virtual void DoLayout(const GG::StateButton& button, Pt& button_ul, Pt& button_lr, Pt& text_ul) const;
};


/** \brief Builtin representation of a check box state button. */
class GG_API BeveledCheckBoxRepresenter: public StateButtonRepresenter
{
public:
    explicit BeveledCheckBoxRepresenter(Clr int_color = CLR_ZERO);

    void Render(const StateButton& button) const override;

private:
    Clr m_int_color;
};


/** \brief Builtin representation of a radio state button. */
class GG_API BeveledRadioRepresenter: public StateButtonRepresenter
{
public:
    explicit BeveledRadioRepresenter(Clr int_color = CLR_ZERO);

    void Render(const StateButton& button) const override;

private:
    Clr m_int_color;
};


/** \brief Builtin representation of a tab state button (part of the TabWnd). */
class GG_API BeveledTabRepresenter: public StateButtonRepresenter
{
public:
    void Render(const StateButton& button) const override;

    Pt MinUsableSize(const StateButton& button) const override;
};


/** \brief This is a class that encapsulates multiple GG::StateButtons into a
    single radio-button control.

    RadioButtonGroup emits a signal whenever its currently-checked button
    changes.  The signal indicates which button has been pressed, by passing
    the index of the button; the currently-checked button index is NO_BUTTON
    when no button is checked.  Any StateButton-derived controls can be used
    in a RadioButtonGroup. */
class GG_API RadioButtonGroup : public Control
{
public:
    /** \name Signal Types */ ///@{
    /** emitted when the currently-selected button has changed; the new
        selected button's index in the group is provided (this may be
        NO_BUTTON if no button is currently selected) */
    typedef boost::signals2::signal<void (std::size_t)> ButtonChangedSignalType;
    //@}

    /** \name Structors */ ///@{
    RadioButtonGroup(Orientation orientation);
    //@}

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;

    /** Returns the orientation of the buttons in the group */
    Orientation      GetOrientation() const;

    /** Returns true iff NumButtons() == 0 */
    bool             Empty() const;

    /** Returns the number of buttons in this control */
    std::size_t      NumButtons() const;

    /** Returns the index of the currently checked button, or NO_BUTTON if
        none are checked */
    std::size_t      CheckedButton() const;

    /** Returns true iff the buttons in the group are to be expanded to fill
        the group's available space.  If false, this indicates that the
        buttons are to be spaced out evenly, and that they should all be their
        MinUsableSize()s. */
    bool             ExpandButtons() const;

    /** Returns true iff the buttons in the group are to be expanded in
        proportion to their initial sizes.  If false, this indicates that the
        buttons are to be expanded evenly.  Note that this has no effect if
        ExpandButtons() is false. */
    bool             ExpandButtonsProportionally() const;

    /** Returns true iff this button group will render an outline of itself;
        this is sometimes useful for debugging purposes */
    bool             RenderOutline() const;

    mutable ButtonChangedSignalType ButtonChangedSignal; ///< The button changed signal object for this RadioButtonGroup
    //@}

    /** \name Mutators */ ///@{
    void Render() override;

    /** Checks the index-th button, and unchecks all others.  If there is no
        index-th button, they are all unchecked, and the currently-checked
        button index is set to NO_BUTTON. */
    void SetCheck(std::size_t index);

    /** Disables (with b == true) or enables (with b == false) the index-th
        button, if it exists.  If the button exists, is being disabled, and is
        the one currently checked, the currently-checked button index is set
        to NO_BUTTON. */
    void DisableButton(std::size_t index, bool b = true); 

    /** Adds a button to the end of the group. The button group owns \p bn.*/
    void AddButton(std::shared_ptr<StateButton> bn);

    /** Adds a button to the group at position \a index.  \a index must be in
        the range [0, NumButtons()]. The button group owns \p bn.*/
    void InsertButton(std::size_t index, std::shared_ptr<StateButton> bn);

    /** Removes \a button from the group.  If \a button is at index i, and is
        the currently-checked button, the currently-checked button index is
        set to NO_BUTTON.  If the currently-checked button is after i, the
        currently-checked button index will be decremented.  In either case, a
        ButtonChangedSignal will be emitted.  Note that this causes the layout
        to relinquish responsibility for \a wnd's memory management. */
    void RemoveButton(StateButton* button);

    /** Set this to true if the buttons in the group are to be expanded to
        fill the group's available space.  If set to false, the buttons are to
        be spaced out evenly, and they will all be at least their
        MinUsableSize()s. */
    void ExpandButtons(bool expand);

    /** Set this to true if the buttons in the group are to be expanded in
        proportion to their initial sizes.  If set to false, this indicates
        that the buttons are to be expanded evenly.  Note that this has no
        effect if ExpandButtons() is false. */
    void ExpandButtonsProportionally(bool proportional);

    /** Set this to true if this button group should render an outline of
        itself; this is sometimes useful for debugging purposes */
    void RenderOutline(bool render_outline);

    /** Raises the currently-selected button to the top of the child z-order.
        If there is no currently-selected button, no action is taken. */
    void RaiseCheckedButton();

    /** The invalid button position index that there is no currently-checked
        button. */
    static const std::size_t NO_BUTTON;
    //@}

protected:
    /** \brief Encapsulates all data pertaining ot a single button in a
        RadioButtonGroup. */
    struct GG_API ButtonSlot
    {
        ButtonSlot(std::shared_ptr<StateButton>& button_);

        std::shared_ptr<StateButton> button;

        boost::signals2::connection connection;
    };

    /** \name Accessors */ ///@{
    const std::vector<ButtonSlot>& ButtonSlots() const; ///< returns the state buttons in the group
    //@}

private:
    void ConnectSignals();
    void SetCheckImpl(std::size_t index, bool signal);
    void Reconnect();

    const Orientation       m_orientation;
    std::vector<ButtonSlot> m_button_slots;
    std::size_t             m_checked_button; ///< the index of the currently-checked button; NO_BUTTON if none is clicked
    bool                    m_expand_buttons;
    bool                    m_expand_buttons_proportionally;
    bool                    m_render_outline;
};

} // namespace GG

#endif
