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

/** \file StyleFactory.h \brief Contains the StyleFactory class, which creates
    new controls for internal use by dialogs and other controls. */

#ifndef _GG_StyleFactory_h_
#define _GG_StyleFactory_h_

#include  <GG/ClrConstants.h>
#include  <GG/FontFwd.h>
#include  <GG/Wnd.h>


namespace GG {

class Button;
class DropDownList;
class Edit;
class Font;
class GroupBox;
class ListBox;
class RadioButtonGroup;
class Scroll;
template <class T>
class Slider;
class StateButton;
class TabBar;
class TextControl;
class ThreeButtonDlg;
struct UnicodeCharset;

/** \brief Creates new dialogs and Controls.

    This class can be used to create a look for the entire GUI by providing
    user-defined subclasses of the standard Controls.  A Control or dialog can
    then use the StyleFactory to create the dialogs/controls it needs (e.g. a
    vertical Scroll uses NewVScrollTabButton() to create its tab).  This
    reduces the amount of subclass code that is required to produce a set of
    custom GG classes.  Note that the subcontrol factory methods below may be
    the same as or different from their generic counterparts, allowing greater
    flexibility in which controls are created in different contexts.  For
    example, NewButton() may create a generic, basic GG Button, but
    NewHSliderTabButton() may produce a specialized button that looks better
    on horizontal sliders.  By default, all subcontrol methods invoke the more
    generic control method for the type of control they each return. */
class GG_API StyleFactory
{
public:
    /** \name Structors */ ///@{
    StyleFactory(); ///< Default ctor.
    virtual ~StyleFactory(); ///< Virtual dtor.
    //@}

    /** Returns the default font for this style, in the size \a pts,
        supporting all printable ASCII characters. */
    virtual boost::shared_ptr<Font> DefaultFont(unsigned int pts = 12) const;

    /** Returns the default font for this style, in the size \a pts,
        supporting all the characters in the UnicodeCharsets in the range
        [first, last). */
    virtual boost::shared_ptr<Font> DefaultFont(unsigned int pts,
                                                const UnicodeCharset* first,
                                                const UnicodeCharset* last) const;

    /** \name Controls */ ///@{
    /** Returns a new GG Button. */
    virtual Button*            NewButton(const std::string& str,
                                         const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                         Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG RadioButtonGroup. */
    virtual RadioButtonGroup*  NewRadioButtonGroup(Orientation orientation) const;

    /** Returns a new GG DropDownList. */
    virtual DropDownList*      NewDropDownList(size_t num_shown_elements, Clr color,
                                               Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG Edit. */
    virtual Edit*              NewEdit(const std::string& str, const boost::shared_ptr<Font>& font,
                                       Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                       Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG ListBox. */
    virtual ListBox*           NewListBox(Clr color, Clr interior = CLR_ZERO,
                                          Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG Scroll. */
    virtual Scroll*            NewScroll(Orientation orientation, Clr color, Clr interior,
                                         Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new GG Slider<int>. */
    virtual Slider<int>*       NewIntSlider(int min, int max, Orientation orientation,
                                            SliderLineStyle style, Clr color, int tab_width, int line_width = 5) const;

    /** Returns a new GG TabBar. */
    virtual TabBar*            NewTabBar(const boost::shared_ptr<Font>& font, Clr color,
                                         Clr text_color = CLR_BLACK, TabBarStyle style = TAB_BAR_ATTACHED,
                                         Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG TextControl. */
    virtual TextControl*       NewTextControl(const std::string& str, const boost::shared_ptr<Font>& font,
                                              Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
                                              Flags<WndFlag> flags = NO_WND_FLAGS) const;

    /** \name Subcontrols */ ///@{
    /** Returns a new ListBox, to be used in a DropDownList. */
    virtual ListBox*           NewDropDownListListBox(Clr color, Clr interior = CLR_ZERO) const;

    /** Returns a new vertical Scroll, to be used in a ListBox. */
    virtual Scroll*            NewListBoxVScroll(Clr color, Clr interior,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new horizontal Scroll, to be used in a ListBox. */
    virtual Scroll*            NewListBoxHScroll(Clr color, Clr interior,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new vertical Scroll, to be used in a MultiEdit. */
    virtual Scroll*            NewMultiEditVScroll(Clr color, Clr interior,
                                                   Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new horizontal Scroll, to be used in a MultiEdit. */
    virtual Scroll*            NewMultiEditHScroll(Clr color, Clr interior,
                                                   Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new up (decrease) Button, to be used in a vertical Scroll. */
    virtual Button*            NewScrollUpButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new down (increase) Button, to be used in a vertical Scroll. */
    virtual Button*            NewScrollDownButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new tab Button, to be used in a vertical Scroll. */
    virtual Button*            NewVScrollTabButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new left (decrease) Button, to be used in a horizontal Scroll. */
    virtual Button*            NewScrollLeftButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new right (increase) Button, to be used in a horizontal Scroll. */
    virtual Button*            NewScrollRightButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new tab Button, to be used in a horizontal Scroll. */
    virtual Button*            NewHScrollTabButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new tab Button, to be used in a vertical Slider. */
    virtual Button*            NewVSliderTabButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new tab Button, to be used in a horizontal Slider. */
    virtual Button*            NewHSliderTabButton(Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new increase Button, to be used in a Spin. */
    virtual Button*            NewSpinIncrButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new decrease Button, to be used in a Spin. */
    virtual Button*            NewSpinDecrButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new Edit, to be used in an editable Spin. */
    virtual Edit*              NewSpinEdit(const std::string& str, const boost::shared_ptr<Font>& font,
                                           Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                           Flags<WndFlag> flags = INTERACTIVE | REPEAT_KEY_PRESS) const;

    /** Returns a new StateButton, to be used in a TabBar. */
    virtual StateButton*       NewTabBarTab(const std::string& str,
                                            const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                            Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                            StateButtonStyle style = SBSTYLE_3D_TOP_ATTACHED_TAB, Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual Button*            NewTabBarLeftButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual Button*            NewTabBarRightButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK) const;
    //@}

    /** \name Dialogs */ ///@{
    /** Returns a new GG ThreeButtonDlg. */
    virtual ThreeButtonDlg*    NewThreeButtonDlg(X x, Y y, X w, Y h, const std::string& msg,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                                 Clr button_color, Clr text_color, int buttons, const std::string& zero = "",
                                                 const std::string& one = "", const std::string& two = "") const;

    /** Returns a new GG ThreeButtonDlg that automatically centers itself in
        the app. */
    virtual ThreeButtonDlg*    NewThreeButtonDlg(X w, Y h, const std::string& msg, const boost::shared_ptr<Font>& font,
                                                 Clr color, Clr border_color, Clr button_color, Clr text_color, int buttons,
                                                 const std::string& zero = "", const std::string& one = "",
                                                 const std::string& two = "") const;
    //@}

    /** Deletes \a wnd.  It is only necessary to use this method to destroy
        Wnds when the factory that created them exists in a plugin. */
    virtual void               DeleteWnd(Wnd* wnd) const;

    /** The "filename" of the default font. */
    static const std::string&  DefaultFontName();
};

} // namespace GG

#endif
