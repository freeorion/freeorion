//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <memory>
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
#include <GG/StyleFactory.h>
#include <GG/TabWnd.h>
#include <GG/TextControl.h>
#include "DefaultFont.h"


using namespace GG;

std::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts) const
{
    if (GetFontManager().HasFont(DefaultFontName(), pts)) {
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, std::vector<uint8_t>());
    } else {
        std::vector<uint8_t> bytes;
        VeraTTFBytes(bytes);
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, bytes);
    }
}

std::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts,
                                                const UnicodeCharset* first,
                                                const UnicodeCharset* last) const
{
    if (GetFontManager().HasFont(DefaultFontName(), pts, first, last)) {
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, std::vector<uint8_t>(), first, last);
    } else {
        std::vector<uint8_t> bytes;
        VeraTTFBytes(bytes);
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, bytes, first, last);
    }
}

std::shared_ptr<Button> StyleFactory::NewButton(std::string str, const std::shared_ptr<Font>& font,
                                                Clr color, Clr text_color,
                                                Flags<WndFlag> flags) const
{ return Wnd::Create<Button>(std::move(str), font, color, text_color, flags); }

std::shared_ptr<RadioButtonGroup> StyleFactory::NewRadioButtonGroup(Orientation orientation) const
{ return Wnd::Create<RadioButtonGroup>(orientation); }

std::shared_ptr<DropDownList> StyleFactory::NewDropDownList(std::size_t num_shown_elements, Clr color) const
{ return Wnd::Create<DropDownList>(num_shown_elements, color); }

std::shared_ptr<Edit> StyleFactory::NewEdit(
    std::string str, const std::shared_ptr<Font>& font,
    Clr color, Clr text_color, Clr interior) const
{ return Wnd::Create<Edit>(std::move(str), font, color, text_color, interior); }

std::shared_ptr<ListBox> StyleFactory::NewListBox(Clr color, Clr interior) const
{ return Wnd::Create<ListBox>(color, interior); }

std::shared_ptr<Scroll> StyleFactory::NewScroll(Orientation orientation, Clr color, Clr interior) const
{ return Wnd::Create<Scroll>(orientation, color, interior); }

std::shared_ptr<Slider<int>> StyleFactory::NewIntSlider(
    int min, int max, Orientation orientation, Clr color, int tab_width, int line_width) const
{ return Wnd::Create<Slider<int>>(min, max, orientation, color, tab_width, line_width, INTERACTIVE); }

std::shared_ptr<TextControl> StyleFactory::NewTextControl(
    std::string str, const std::shared_ptr<Font>& font,
    Clr color, Flags<TextFormat> format) const
{ return Wnd::Create<TextControl>(X0, Y0, X1, Y1, std::move(str), font, color, format, NO_WND_FLAGS); }

std::shared_ptr<TabBar> StyleFactory::NewTabBar(const std::shared_ptr<Font>& font, Clr color,
                                                Clr text_color) const
{ return Wnd::Create<TabBar>(font, color, text_color, INTERACTIVE); }

std::shared_ptr<ListBox> StyleFactory::NewDropDownListListBox(Clr color, Clr interior) const
{
    auto lb = NewListBox(color, interior);
    // Because the rows of DropDownLists must be the same size, there's
    // no need to worry that the bottom entry will get cut off if the
    // scrollbar ends exactly at the list's end.
    lb->AddPaddingAtEnd(false);
    return lb;
}

std::shared_ptr<Scroll> StyleFactory::NewListBoxVScroll(Clr color, Clr interior) const
{ return NewScroll(Orientation::VERTICAL, color, interior); }

std::shared_ptr<Scroll> StyleFactory::NewListBoxHScroll(Clr color, Clr interior) const
{ return NewScroll(Orientation::HORIZONTAL, color, interior); }

std::shared_ptr<Scroll> StyleFactory::NewMultiEditVScroll(Clr color, Clr interior) const
{ return NewScroll(Orientation::VERTICAL, color, interior); }

std::shared_ptr<Scroll> StyleFactory::NewMultiEditHScroll(Clr color, Clr interior) const
{ return NewScroll(Orientation::HORIZONTAL, color, interior); }

std::shared_ptr<Button> StyleFactory::NewScrollUpButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

std::shared_ptr<Button> StyleFactory::NewScrollDownButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

std::shared_ptr<Button> StyleFactory::NewVScrollTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

std::shared_ptr<Button> StyleFactory::NewScrollLeftButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

std::shared_ptr<Button> StyleFactory::NewScrollRightButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

std::shared_ptr<Button> StyleFactory::NewHScrollTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

std::shared_ptr<Button> StyleFactory::NewVSliderTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

std::shared_ptr<Button> StyleFactory::NewHSliderTabButton(Clr color) const
{ return NewButton("", nullptr, color, CLR_BLACK, INTERACTIVE); }

std::shared_ptr<Button> StyleFactory::NewSpinIncrButton(const std::shared_ptr<Font>& font, Clr color) const
{ return NewButton("+", font, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

std::shared_ptr<Button> StyleFactory::NewSpinDecrButton(const std::shared_ptr<Font>& font, Clr color) const
{ return NewButton("-", font, color, CLR_BLACK, INTERACTIVE | REPEAT_BUTTON_DOWN); }

std::shared_ptr<Edit> StyleFactory::NewSpinEdit(
    std::string str, const std::shared_ptr<Font>& font,
    Clr color, Clr text_color, Clr interior) const
{ return NewEdit(std::move(str), font, color, text_color, interior); }

std::shared_ptr<StateButton> StyleFactory::NewTabBarTab(
    std::string str, const std::shared_ptr<Font>& font, Flags<TextFormat> format,
    Clr color, Clr text_color) const
{
    auto retval = Wnd::Create<StateButton>(
        std::move(str), font, format, color, std::make_shared<BeveledTabRepresenter>(), text_color);
    retval->Resize(retval->MinUsableSize() + Pt(X(12), Y0));
    return retval;
}

std::shared_ptr<Button> StyleFactory::NewTabBarLeftButton(
    const std::shared_ptr<Font>& font, Clr color, Clr text_color) const
{ return NewButton("<", font, color, text_color, INTERACTIVE); }

std::shared_ptr<Button> StyleFactory::NewTabBarRightButton(
    const std::shared_ptr<Font>& font, Clr color, Clr text_color) const
{ return NewButton(">", font, color, text_color, INTERACTIVE); }

std::shared_ptr<ThreeButtonDlg> StyleFactory::NewThreeButtonDlg(
    X w, Y h, std::string msg, const std::shared_ptr<Font>& font,
    Clr color, Clr border_color, Clr button_color, Clr text_color,
    int buttons, std::string zero, std::string one, std::string two) const
{
    return Wnd::Create<ThreeButtonDlg>(w, h, std::move(msg), font, color, border_color, button_color,
                                       text_color, buttons, std::move(zero), std::move(one), std::move(two));
}

std::string_view StyleFactory::DefaultFontName() noexcept
{ return DEFAULT_FONT_NAME; }
