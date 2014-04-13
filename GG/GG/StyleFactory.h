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
#include  <GG/DynamicGraphic.h>
#include  <GG/FontFwd.h>
#include  <GG/MultiEditFwd.h>


namespace GG {

class Button;
class ColorDlg;
class DropDownList;
class DynamicGraphic;
class FileDlg;
class Edit;
class Font;
class GroupBox;
class ListBox;
class MenuBar;
class MultiEdit;
class RadioButtonGroup;
class Scroll;
template <class T>
class Slider;
template <class T>
class Spin;
class StateButton;
class StaticGraphic;
class TabBar;
class TabWnd;
class TextControl;
class Texture;
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
    virtual Button*            NewButton(X x, Y y, X w, Y h, const std::string& str,
                                         const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                         Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG StateButton. */
    virtual StateButton*       NewStateButton(X x, Y y, X w, Y h, const std::string& str,
                                              const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                              Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                              StateButtonStyle style = SBSTYLE_3D_XBOX, Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG RadioButtonGroup. */
    virtual RadioButtonGroup*  NewRadioButtonGroup(X x, Y y, X w, Y h, Orientation orientation) const;

    /** Returns a new GG DropDownList. */
    virtual DropDownList*      NewDropDownList(X x, Y y, X w, Y h, Y drop_ht, Clr color,
                                               Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG DynamicGraphic. */
    virtual DynamicGraphic*    NewDynamicGraphic(X x, Y y, X w, Y h, bool loop, X frame_width, Y frame_height,
                                                 int margin, const std::vector<boost::shared_ptr<Texture> >& textures,
                                                 Flags<GraphicStyle> style = GRAPHIC_NONE, int frames = DynamicGraphic::ALL_FRAMES,
                                                 Flags<WndFlag> flags = Flags<WndFlag>()) const;

    /** Returns a new GG Edit. */
    virtual Edit*              NewEdit(X x, Y y, X w, const std::string& str, const boost::shared_ptr<Font>& font,
                                       Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                       Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG ListBox. */
    virtual ListBox*           NewListBox(X x, Y y, X w, Y h, Clr color, Clr interior = CLR_ZERO,
                                          Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG MenuBar. */
    virtual MenuBar*           NewMenuBar(X x, Y y, X w, const boost::shared_ptr<Font>& font,
                                          Clr text_color = CLR_WHITE, Clr color = CLR_BLACK,
                                          Clr interior = CLR_SHADOW) const;

    /** Returns a new GG MultiEdit. */
    virtual MultiEdit*         NewMultiEdit(X x, Y y, X w, Y h, const std::string& str,
                                            const boost::shared_ptr<Font>& font, Clr color, Flags<MultiEditStyle> style = MULTI_LINEWRAP,
                                            Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                            Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG Scroll. */
    virtual Scroll*            NewScroll(X x, Y y, X w, Y h, Orientation orientation, Clr color, Clr interior,
                                         Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new GG Slider<int>. */
    virtual Slider<int>*       NewIntSlider(X x, Y y, X w, Y h, int min, int max, Orientation orientation,
                                            SliderLineStyle style, Clr color, int tab_width, int line_width = 5,
                                            Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG Slider<double>. */
    virtual Slider<double>*    NewDoubleSlider(X x, Y y, X w, Y h, double min, double max, Orientation orientation,
                                            SliderLineStyle style, Clr color, int tab_width, int line_width = 5,
                                            Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG Spin<int>. */
    virtual Spin<int>*         NewIntSpin(X x, Y y, X w, int value, int step, int min, int max, bool edits,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                          Clr interior = CLR_ZERO, Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG Spin<int>. */
    virtual Spin<double>*      NewDoubleSpin(X x, Y y, X w, double value, double step, double min, double max, bool edits,
                                             const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                             Clr interior = CLR_ZERO, Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG StaticGraphic. */
    virtual StaticGraphic*     NewStaticGraphic(X x, Y y, X w, Y h, const boost::shared_ptr<Texture>& texture,
                                                Flags<GraphicStyle> style = GRAPHIC_NONE, Flags<WndFlag> flags = Flags<WndFlag>()) const;

    /** Returns a new GG TabBar. */
    virtual TabBar*            NewTabBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, Clr color,
                                         Clr text_color = CLR_BLACK, TabBarStyle style = TAB_BAR_ATTACHED,
                                         Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new GG TextControl. */
    virtual TextControl*       NewTextControl(X x, Y y, X w, Y h, const std::string& str,
                                              const boost::shared_ptr<Font>& font, Clr color = CLR_BLACK,
                                              Flags<TextFormat> format = FORMAT_NONE, Flags<WndFlag> flags = Flags<WndFlag>()) const;

    /** Returns a new GG TextControl whose size is exactly that required to hold its text. */
    virtual TextControl*       NewTextControl(X x, Y y, const std::string& str, const boost::shared_ptr<Font>& font,
                                              Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
                                              Flags<WndFlag> flags = Flags<WndFlag>()) const;

    /** Returns a new GG GroupBox. */
    virtual GroupBox*          NewGroupBox(X x, Y y, X w, Y h, const std::string& label, const boost::shared_ptr<Font>& font,
                                           Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                           Flags<WndFlag> flags = Flags<WndFlag>()) const;
    //@}

    /** \name Subcontrols */ ///@{
    /** Returns a new ListBox, to be used in a DropDownList. */
    virtual ListBox*           NewDropDownListListBox(X x, Y y, X w, Y h, Clr color, Clr interior = CLR_ZERO,
                                                      Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new vertical Scroll, to be used in a ListBox. */
    virtual Scroll*            NewListBoxVScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new horizontal Scroll, to be used in a ListBox. */
    virtual Scroll*            NewListBoxHScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new vertical Scroll, to be used in a MultiEdit. */
    virtual Scroll*            NewMultiEditVScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                                   Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new horizontal Scroll, to be used in a MultiEdit. */
    virtual Scroll*            NewMultiEditHScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                                   Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new up (decrease) Button, to be used in a vertical Scroll. */
    virtual Button*            NewScrollUpButton(X x, Y y, X w, Y h, const std::string& str,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new down (increase) Button, to be used in a vertical Scroll. */
    virtual Button*            NewScrollDownButton(X x, Y y, X w, Y h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new tab Button, to be used in a vertical Scroll. */
    virtual Button*            NewVScrollTabButton(X x, Y y, X w, Y h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new left (decrease) Button, to be used in a horizontal Scroll. */
    virtual Button*            NewScrollLeftButton(X x, Y y, X w, Y h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new right (increase) Button, to be used in a horizontal Scroll. */
    virtual Button*            NewScrollRightButton(X x, Y y, X w, Y h, const std::string& str,
                                                    const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                    Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new tab Button, to be used in a horizontal Scroll. */
    virtual Button*            NewHScrollTabButton(X x, Y y, X w, Y h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new tab Button, to be used in a vertical Slider. */
    virtual Button*            NewVSliderTabButton(X x, Y y, X w, Y h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new tab Button, to be used in a horizontal Slider. */
    virtual Button*            NewHSliderTabButton(X x, Y y, X w, Y h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new increase Button, to be used in a Spin. */
    virtual Button*            NewSpinIncrButton(X x, Y y, X w, Y h, const std::string& str,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new decrease Button, to be used in a Spin. */
    virtual Button*            NewSpinDecrButton(X x, Y y, X w, Y h, const std::string& str,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = INTERACTIVE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new Edit, to be used in an editable Spin. */
    virtual Edit*              NewSpinEdit(X x, Y y, X w, const std::string& str, const boost::shared_ptr<Font>& font,
                                           Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                           Flags<WndFlag> flags = INTERACTIVE | REPEAT_KEY_PRESS) const;

    /** Returns a new StateButton, to be used in a TabBar. */
    virtual StateButton*       NewTabBarTab(X x, Y y, X w, Y h, const std::string& str,
                                            const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                            Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                            StateButtonStyle style = SBSTYLE_3D_TOP_ATTACHED_TAB, Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual Button*            NewTabBarLeftButton(X x, Y y, X w, Y h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = INTERACTIVE) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual Button*            NewTabBarRightButton(X x, Y y, X w, Y h, const std::string& str,
                                                    const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                    Flags<WndFlag> flags = INTERACTIVE) const;
    //@}

    /** \name Wnds */ ///@{
    /** Returns a new GG TabWnd. */
    virtual TabWnd*            NewTabWnd(X x, Y y, X w, Y h, const boost::shared_ptr<Font>& font, Clr color,
                                         Clr text_color = CLR_BLACK, TabBarStyle style = TAB_BAR_ATTACHED,
                                         Flags<WndFlag> flags = INTERACTIVE | DRAGABLE) const;
    //@}

    /** \name Dialogs */ ///@{
    /** Returns a new GG ColorDlg. */
    virtual ColorDlg*          NewColorDlg(X x, Y y, const boost::shared_ptr<Font>& font,
                                           Clr dialog_color, Clr border_color, Clr text_color = CLR_BLACK) const;

    /** Returns a new GG ColorDlg that has a starting color specified. */
    virtual ColorDlg*          NewColorDlg(X x, Y y, Clr original_color, const boost::shared_ptr<Font>& font,
                                           Clr dialog_color, Clr border_color, Clr text_color = CLR_BLACK) const;

    /** Returns a new GG FileDlg. */
    virtual FileDlg*           NewFileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                          Clr text_color = CLR_BLACK) const;

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
