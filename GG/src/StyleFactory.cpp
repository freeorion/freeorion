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

#include <GG/StyleFactory.h>

#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/GroupBox.h>
#include <GG/ListBox.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>
#include <GG/TextControl.h>

#include <GG/dialogs/ColorDlg.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include "DefaultFont.h"


using namespace GG;

StyleFactory::StyleFactory()
{}

StyleFactory::~StyleFactory()
{}

boost::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts/* = 12*/) const
{
    if (GetFontManager().HasFont(DefaultFontName(), pts)) {
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, std::vector<unsigned char>());
    } else {
        std::vector<unsigned char> bytes;
        VeraTTFBytes(bytes);
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, bytes);
    }
}

boost::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts,
                                                  const UnicodeCharset* first,
                                                  const UnicodeCharset* last) const
{
    if (GetFontManager().HasFont(DefaultFontName(), pts, first, last)) {
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, std::vector<unsigned char>(), first, last);
    } else {
        std::vector<unsigned char> bytes;
        VeraTTFBytes(bytes);
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, bytes, first, last);
    }
}

Button* StyleFactory::NewButton(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Button(x, y, w, h, str, font, color, text_color, flags); }

RadioButtonGroup* StyleFactory::NewRadioButtonGroup(Orientation orientation) const
{ return new RadioButtonGroup(orientation); }

DropDownList* StyleFactory::NewDropDownList(X x, Y y, X w, Y h, Y drop_ht, Clr color,
                                            Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new DropDownList(x, y, w, h, drop_ht, color, flags); }

Edit* StyleFactory::NewEdit(const std::string& str, const boost::shared_ptr<Font>& font,
                            Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                            Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Edit(X0, Y0, X1, str, font, color, text_color, interior, flags); }

ListBox* StyleFactory::NewListBox(X x, Y y, X w, Y h, Clr color, Clr interior/* = CLR_ZERO*/,
                                  Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new ListBox(x, y, w, h, color, interior, flags); }

Scroll* StyleFactory::NewScroll(X x, Y y, X w, Y h, Orientation orientation, Clr color, Clr interior,
                                Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return new Scroll(x, y, w, h, orientation, color, interior, flags); }

Slider<int>* StyleFactory::NewIntSlider(int min, int max, Orientation orientation,
                                        SliderLineStyle style, Clr color, int tab_width, int line_width/* = 5*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Slider<int>(GG::X0, GG::Y0, GG::X1, GG::Y1, min, max, orientation, style, color, tab_width, line_width, flags); }

TextControl* StyleFactory::NewTextControl(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                                          Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = NO_WND_FLAGS*/) const
{ return new TextControl(x, y, w, h, str, font, color, format, flags); }

TextControl* StyleFactory::NewTextControl(const std::string& str, const boost::shared_ptr<Font>& font,
                                          Clr color/* = CLR_BLACK*/, Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = NO_WND_FLAGS*/) const
{ return new TextControl(X0, Y0, str, font, color, format, flags); }

TabBar* StyleFactory::NewTabBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                TabBarStyle style/* = TAB_BAR_ATTACHED*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new TabBar(x, y, w, font, color, text_color, style, flags); }

ListBox* StyleFactory::NewDropDownListListBox(X x, Y y, X w, Y h, Clr color, Clr interior/* = CLR_ZERO*/,
                                              Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewListBox(x, y, w, h, color, interior, flags); }

Scroll* StyleFactory::NewListBoxVScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, VERTICAL, color, interior, flags); }

Scroll* StyleFactory::NewListBoxHScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, HORIZONTAL, color, interior, flags); }

Scroll* StyleFactory::NewMultiEditVScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, VERTICAL, color, interior, flags); }

Scroll* StyleFactory::NewMultiEditHScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, HORIZONTAL, color, interior, flags); }

Button* StyleFactory::NewScrollUpButton(X x, Y y, X w, Y h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewScrollDownButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewVScrollTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewScrollLeftButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewScrollRightButton(X x, Y y, X w, Y h, const std::string& str,
                                           const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                           Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewHScrollTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewVSliderTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewHSliderTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewSpinIncrButton(const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(GG::X0, GG::Y0, GG::X1, GG::Y1, str, font, color, text_color, flags); }

Button* StyleFactory::NewSpinDecrButton(const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(GG::X0, GG::Y0, GG::X1, GG::Y1, str, font, color, text_color, flags); }

Edit* StyleFactory::NewSpinEdit(const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewEdit(str, font, color, text_color, interior, flags); }

StateButton* StyleFactory::NewTabBarTab(const std::string& str,
                                        const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                        Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                        StateButtonStyle style/* = SBSTYLE_3D_TOP_ATTACHED_TAB*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{
    StateButton* retval = new StateButton(X0, Y0, X1, Y1, str, font, format, color, text_color, interior, style, flags);
    retval->Resize(retval->MinUsableSize() + Pt(X(12), Y0));
    return retval;
}

Button* StyleFactory::NewTabBarLeftButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewTabBarRightButton(X x, Y y, X w, Y h, const std::string& str,
                                           const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                           Flags<WndFlag> flags /*= INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

ThreeButtonDlg* StyleFactory::NewThreeButtonDlg(X x, Y y, X w, Y h, const std::string& msg,
                                                const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                                Clr button_color, Clr text_color, int buttons,
                                                const std::string& zero/* = ""*/, const std::string& one/* = ""*/,
                                                const std::string& two/* = ""*/) const
{
    return new ThreeButtonDlg(x, y, w, h, msg, font, color, border_color, button_color, text_color,
                              buttons, zero, one, two);
}

ThreeButtonDlg* StyleFactory::NewThreeButtonDlg(X w, Y h, const std::string& msg, const boost::shared_ptr<Font>& font,
                                                Clr color, Clr border_color, Clr button_color, Clr text_color,
                                                int buttons, const std::string& zero/* = ""*/,
                                                const std::string& one/* = ""*/, const std::string& two/* = ""*/) const
{
    return new ThreeButtonDlg(w, h, msg, font, color, border_color, button_color, text_color,
                              buttons, zero, one, two);
}

void StyleFactory::DeleteWnd(Wnd* wnd) const
{ delete wnd; }

const std::string& StyleFactory::DefaultFontName()
{
    static std::string retval = DEFAULT_FONT_NAME;
    return retval;
}
