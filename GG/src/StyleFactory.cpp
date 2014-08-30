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

// Don't translate, just pass the key.
std::string StyleFactory::Translate(const std::string& key) const
{ return key; }

Button* StyleFactory::NewButton(const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Button(str, font, color, text_color, flags); }

RadioButtonGroup* StyleFactory::NewRadioButtonGroup(Orientation orientation) const
{ return new RadioButtonGroup(orientation); }

DropDownList* StyleFactory::NewDropDownList(size_t num_shown_elements, Clr color) const
{ return new DropDownList(num_shown_elements, color); }

Edit* StyleFactory::NewEdit(const std::string& str, const boost::shared_ptr<Font>& font,
                            Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/) const
{ return new Edit(str, font, color, text_color, interior); }

ListBox* StyleFactory::NewListBox(Clr color, Clr interior/* = CLR_ZERO*/) const
{ return new ListBox(color, interior); }

Scroll* StyleFactory::NewScroll(Orientation orientation, Clr color, Clr interior) const
{ return new Scroll(orientation, color, interior); }

Slider<int>* StyleFactory::NewIntSlider(int min, int max, Orientation orientation,
                                        Clr color, int tab_width, int line_width/* = 5*/) const
{ return new Slider<int>(min, max, orientation, color, tab_width, line_width, INTERACTIVE); }

TextControl* StyleFactory::NewTextControl(const std::string& str, const boost::shared_ptr<Font>& font,
                                          Clr color/* = CLR_BLACK*/, Flags<TextFormat> format/* = FORMAT_NONE*/) const
{ return new TextControl(X0, Y0, X1, Y1, str, font, color, format, NO_WND_FLAGS); }

TabBar* StyleFactory::NewTabBar(const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                TabBarStyle style/* = TAB_BAR_ATTACHED*/) const
{ return new TabBar(font, color, text_color, style, INTERACTIVE); }

ListBox* StyleFactory::NewDropDownListListBox(Clr color, Clr interior/* = CLR_ZERO*/) const
{ return NewListBox(color, interior); }

Scroll* StyleFactory::NewListBoxVScroll(Clr color, Clr interior) const
{ return NewScroll(VERTICAL, color, interior); }

Scroll* StyleFactory::NewListBoxHScroll(Clr color, Clr interior) const
{ return NewScroll(HORIZONTAL, color, interior); }

Scroll* StyleFactory::NewMultiEditVScroll(Clr color, Clr interior) const
{ return NewScroll(VERTICAL, color, interior); }

Scroll* StyleFactory::NewMultiEditHScroll(Clr color, Clr interior) const
{ return NewScroll(HORIZONTAL, color, interior); }

Button* StyleFactory::NewScrollUpButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewScrollDownButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewVScrollTabButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE); }

Button* StyleFactory::NewScrollLeftButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewScrollRightButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewHScrollTabButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE); }

Button* StyleFactory::NewVSliderTabButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE); }

Button* StyleFactory::NewHSliderTabButton(Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("", boost::shared_ptr<Font>(), color, text_color, INTERACTIVE); }

Button* StyleFactory::NewSpinIncrButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("+", font, color, text_color, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewSpinDecrButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("-", font, color, text_color, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Edit* StyleFactory::NewSpinEdit(const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/) const
{ return NewEdit(str, font, color, text_color, interior); }

StateButton* StyleFactory::NewTabBarTab(const std::string& str,
                                        const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                        Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                        StateButtonStyle style/* = SBSTYLE_3D_TOP_ATTACHED_TAB*/) const
{
    StateButton* retval = new StateButton(str, font, format, color, text_color, interior, style);
    retval->Resize(retval->MinUsableSize() + Pt(X(12), Y0));
    return retval;
}

Button* StyleFactory::NewTabBarLeftButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("<", font, color, text_color, INTERACTIVE); }

Button* StyleFactory::NewTabBarRightButton(const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton(">", font, color, text_color, INTERACTIVE); }

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
