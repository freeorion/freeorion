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
#include <GG/dialogs/ColorDlg.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/dialogs/ThreeButtonDlg.h>
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

#include "DefaultFont.h"

#include <memory>


using namespace GG;

StyleFactory::StyleFactory()
{}

StyleFactory::~StyleFactory()
{}

std::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts/* = 12*/) const
{
    if (GetFontManager().HasFont(DefaultFontName(), pts)) {
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, std::vector<unsigned char>());
    } else {
        std::vector<unsigned char> bytes;
        VeraTTFBytes(bytes);
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, bytes);
    }
}

std::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts,
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

Button* StyleFactory::NewButton(const std::string& str, const std::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return Wnd::Create<Button>(str, font, color, text_color, flags); }

RadioButtonGroup* StyleFactory::NewRadioButtonGroup(Orientation orientation) const
{ return Wnd::Create<RadioButtonGroup>(orientation); }

DropDownList* StyleFactory::NewDropDownList(size_t num_shown_elements, Clr color) const
{ return Wnd::Create<DropDownList>(num_shown_elements, color); }

Edit* StyleFactory::NewEdit(const std::string& str, const std::shared_ptr<Font>& font,
                            Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/) const
{ return Wnd::Create<Edit>(str, font, color, text_color, interior); }

ListBox* StyleFactory::NewListBox(Clr color, Clr interior/* = CLR_ZERO*/) const
{ return Wnd::Create<ListBox>(color, interior); }

Scroll* StyleFactory::NewScroll(Orientation orientation, Clr color, Clr interior) const
{ return Wnd::Create<Scroll>(orientation, color, interior); }

Slider<int>* StyleFactory::NewIntSlider(int min, int max, Orientation orientation,
                                        Clr color, int tab_width, int line_width/* = 5*/) const
{ return new Slider<int>(min, max, orientation, color, tab_width, line_width, INTERACTIVE); }

TextControl* StyleFactory::NewTextControl(const std::string& str, const std::shared_ptr<Font>& font,
                                          Clr color/* = CLR_BLACK*/, Flags<TextFormat> format/* = FORMAT_NONE*/) const
{ return Wnd::Create<TextControl>(X0, Y0, X1, Y1, str, font, color, format, NO_WND_FLAGS); }

TabBar* StyleFactory::NewTabBar(const std::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return new TabBar(font, color, text_color, INTERACTIVE); }

ListBox* StyleFactory::NewDropDownListListBox(Clr color, Clr interior/* = CLR_ZERO*/) const
{
    ListBox* lb = NewListBox(color, interior);
    // Because the rows of DropDownLists must be the same size, there's
    // no need to worry that the bottom entry will get cut off if the
    // scrollbar ends exactly at the list's end.
    lb->AddPaddingAtEnd(false);
    return lb;
}

Scroll* StyleFactory::NewListBoxVScroll(Clr color, Clr interior) const
{ return NewScroll(VERTICAL, color, interior); }

Scroll* StyleFactory::NewListBoxHScroll(Clr color, Clr interior) const
{ return NewScroll(HORIZONTAL, color, interior); }

Scroll* StyleFactory::NewMultiEditVScroll(Clr color, Clr interior) const
{ return NewScroll(VERTICAL, color, interior); }

Scroll* StyleFactory::NewMultiEditHScroll(Clr color, Clr interior) const
{ return NewScroll(HORIZONTAL, color, interior); }

Button* StyleFactory::NewScrollUpButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewScrollDownButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewVScrollTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

Button* StyleFactory::NewScrollLeftButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewScrollRightButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewHScrollTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

Button* StyleFactory::NewVSliderTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

Button* StyleFactory::NewHSliderTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

Button* StyleFactory::NewSpinIncrButton(const std::shared_ptr<Font>& font, Clr color) const
{ return NewButton("+", font, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Button* StyleFactory::NewSpinDecrButton(const std::shared_ptr<Font>& font, Clr color) const
{ return NewButton("-", font, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

Edit* StyleFactory::NewSpinEdit(const std::string& str, const std::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/) const
{ return NewEdit(str, font, color, text_color, interior); }

StateButton* StyleFactory::NewTabBarTab(const std::string& str,
                                        const std::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                        Clr text_color/* = CLR_BLACK*/) const
{
    auto retval = Wnd::Create<StateButton>(
        str, font, format, color, std::make_shared<BeveledTabRepresenter>(), text_color);
    retval->Resize(retval->MinUsableSize() + Pt(X(12), Y0));
    return retval;
}

Button* StyleFactory::NewTabBarLeftButton(const std::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton("<", font, color, text_color, INTERACTIVE); }

Button* StyleFactory::NewTabBarRightButton(const std::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/) const
{ return NewButton(">", font, color, text_color, INTERACTIVE); }

ThreeButtonDlg* StyleFactory::NewThreeButtonDlg(X w, Y h, const std::string& msg, const std::shared_ptr<Font>& font,
                                                Clr color, Clr border_color, Clr button_color, Clr text_color,
                                                int buttons, const std::string& zero/* = ""*/,
                                                const std::string& one/* = ""*/, const std::string& two/* = ""*/) const
{
    return Wnd::Create<ThreeButtonDlg>(w, h, msg, font, color, border_color, button_color, text_color,
                                       buttons, zero, one, two);
}

void StyleFactory::DeleteWnd(Wnd* wnd) const
{ delete wnd; }

const std::string& StyleFactory::DefaultFontName()
{
    static std::string retval = DEFAULT_FONT_NAME;
    return retval;
}
