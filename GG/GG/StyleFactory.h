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
    StyleFactory();

    virtual ~StyleFactory();
    //@}

    /** Returns the default font for this style, in the size \a pts,
        supporting all printable ASCII characters. */
    virtual std::shared_ptr<Font> DefaultFont(unsigned int pts = 12) const;

    /** Returns the default font for this style, in the size \a pts,
        supporting all the characters in the UnicodeCharsets in the range
        [first, last). */
    virtual std::shared_ptr<Font> DefaultFont(unsigned int pts,
                                              const UnicodeCharset* first,
                                              const UnicodeCharset* last) const;

    /** Returns translations for common phrases used in the different
        dialoges provided by GiGi. */
    virtual std::string Translate(const std::string& key) const;

    /** \name Controls */ ///@{
    /** Returns a new GG Button. */
    virtual std::shared_ptr<Button> NewButton(const std::string& str,
                              const std::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                              Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG RadioButtonGroup. */
    virtual std::shared_ptr<RadioButtonGroup> NewRadioButtonGroup(Orientation orientation) const;

    /** Returns a new GG DropDownList. */
    virtual std::shared_ptr<DropDownList> NewDropDownList(size_t num_shown_elements, Clr color) const;

    /** Returns a new GG Edit. */
    virtual std::shared_ptr<Edit> NewEdit(const std::string& str, const std::shared_ptr<Font>& font,
                          Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO) const;

    /** Returns a new GG ListBox. */
    virtual std::shared_ptr<ListBox> NewListBox(Clr color, Clr interior = CLR_ZERO) const;

    /** Returns a new GG Scroll. */
    virtual std::shared_ptr<Scroll> NewScroll(Orientation orientation, Clr color, Clr interior) const;

    /** Returns a new GG Slider<int>. */
    virtual std::shared_ptr<Slider<int>> NewIntSlider(int min, int max, Orientation orientation,
                                      Clr color, int tab_width, int line_width = 5) const;

    /** Returns a new GG TabBar. */
    virtual std::shared_ptr<TabBar> NewTabBar(const std::shared_ptr<Font>& font, Clr color,
                              Clr text_color = CLR_BLACK) const;

    /** Returns a new GG TextControl. */
    virtual std::shared_ptr<TextControl> NewTextControl(const std::string& str, const std::shared_ptr<Font>& font,
                                        Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE) const;

    /** \name Subcontrols */ ///@{
    /** Returns a new ListBox, to be used in a DropDownList. */
    virtual std::shared_ptr<ListBox> NewDropDownListListBox(Clr color, Clr interior = CLR_ZERO) const;

    /** Returns a new vertical Scroll, to be used in a ListBox. */
    virtual std::shared_ptr<Scroll> NewListBoxVScroll(Clr color, Clr interior) const;

    /** Returns a new horizontal Scroll, to be used in a ListBox. */
    virtual std::shared_ptr<Scroll> NewListBoxHScroll(Clr color, Clr interior) const;

    /** Returns a new vertical Scroll, to be used in a MultiEdit. */
    virtual std::shared_ptr<Scroll> NewMultiEditVScroll(Clr color, Clr interior) const;

    /** Returns a new horizontal Scroll, to be used in a MultiEdit. */
    virtual std::shared_ptr<Scroll> NewMultiEditHScroll(Clr color, Clr interior) const;

    /** Returns a new up (decrease) Button, to be used in a vertical Scroll. */
    virtual std::shared_ptr<Button> NewScrollUpButton(Clr color) const;

    /** Returns a new down (increase) Button, to be used in a vertical Scroll. */
    virtual std::shared_ptr<Button> NewScrollDownButton(Clr color) const;

    /** Returns a new tab Button, to be used in a vertical Scroll. */
    virtual std::shared_ptr<Button> NewVScrollTabButton(Clr color) const;

    /** Returns a new left (decrease) Button, to be used in a horizontal Scroll. */
    virtual std::shared_ptr<Button> NewScrollLeftButton(Clr color) const;

    /** Returns a new right (increase) Button, to be used in a horizontal Scroll. */
    virtual std::shared_ptr<Button> NewScrollRightButton(Clr color) const;

    /** Returns a new tab Button, to be used in a horizontal Scroll. */
    virtual std::shared_ptr<Button> NewHScrollTabButton(Clr color) const;

    /** Returns a new tab Button, to be used in a vertical Slider. */
    virtual std::shared_ptr<Button> NewVSliderTabButton(Clr color) const;

    /** Returns a new tab Button, to be used in a horizontal Slider. */
    virtual std::shared_ptr<Button> NewHSliderTabButton(Clr color) const;

    /** Returns a new increase Button, to be used in a Spin. */
    virtual std::shared_ptr<Button> NewSpinIncrButton(const std::shared_ptr<Font>& font, Clr color) const;

    /** Returns a new decrease Button, to be used in a Spin. */
    virtual std::shared_ptr<Button> NewSpinDecrButton(const std::shared_ptr<Font>& font, Clr color) const;

    /** Returns a new Edit, to be used in an editable Spin. */
    virtual std::shared_ptr<Edit> NewSpinEdit(const std::string& str, const std::shared_ptr<Font>& font,
                              Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO) const;

    /** Returns a new StateButton, to be used in a TabBar. */
    virtual std::shared_ptr<StateButton> NewTabBarTab(const std::string& str,
                                      const std::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                      Clr text_color = CLR_BLACK) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual std::shared_ptr<Button> NewTabBarLeftButton(const std::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual std::shared_ptr<Button> NewTabBarRightButton(const std::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK) const;
    //@}

    /** \name Dialogs */ ///@{
    /** Returns a new GG ThreeButtonDlg that automatically centers itself in
        the app. */
    virtual std::shared_ptr<ThreeButtonDlg> NewThreeButtonDlg(X w, Y h, const std::string& msg, const std::shared_ptr<Font>& font,
                                              Clr color, Clr border_color, Clr button_color, Clr text_color, int buttons,
                                              const std::string& zero = "", const std::string& one = "",
                                              const std::string& two = "") const;
    //@}

    /** The "filename" of the default font. */
    static const std::string& DefaultFontName();
};

} // namespace GG

#endif
