#include "CUIStyle.h"

#include "CUIControls.h"
#include "CUISlider.h"
#include "../util/i18n.h"


std::string CUIStyle::Translate(const std::string& text) const
{
    if (text == "New")
        return UserString("COLOR_DLG_NEW");
    else if (text == "Old")
        return UserString("COLOR_DLG_OLD");
    else if (text == "R:")
        return UserString("COLOR_DLG_RED");
    else if (text == "G:")
        return UserString("COLOR_DLG_GREEN");
    else if (text == "B:")
        return UserString("COLOR_DLG_BLUE");
    else if (text == "H:")
        return UserString("COLOR_DLG_HUE");
    else if (text == "S:")
        return UserString("COLOR_DLG_SATURATION");
    else if (text == "V:")
        return UserString("COLOR_DLG_VALUE");
    else if (text == "A:")
        return UserString("COLOR_DLG_ALPHA");
    else if (text == "Ok")
        return UserString("OK");
    else if (text == "Cancel")
        return UserString("CANCEL");
    else if (text == "File(s):")
        return UserString("FILE_DLG_FILES");
    else if (text == "Type(s):")
        return UserString("FILE_DLG_FILE_TYPES");
    else if (text == "%1% exists.\nOk to overwrite it?")
        return UserString("FILE_DLG_OVERWRITE_PROMPT");
    else if (text == "\"%1%\"\nis a directory.")
        return UserString("FILE_DLG_FILENAME_IS_A_DIRECTORY");
    else if (text == "File \"%1%\"\ndoes not exist.")
        return UserString("FILE_DLG_FILE_DOES_NOT_EXIST");
    else if (text == "Device is not ready.")
        return UserString("FILE_DLG_DEVICE_IS_NOT_READY");
    else if (text == "Save")
        return UserString("SAVE");
    else if (text == "Open")
        return UserString("OPEN");

    return UserString("ERROR");
}

std::shared_ptr<GG::Button> CUIStyle::NewButton(const std::string& str, const std::shared_ptr<GG::Font>& font,
                                                GG::Clr color, GG::Clr text_color/* = CLR_BLACK*/,
                                                GG::Flags<GG::WndFlag> flags/* = INTERACTIVE*/) const
{ return GG::Wnd::Create<CUIButton>(str); }


std::shared_ptr<GG::DropDownList> CUIStyle::NewDropDownList(size_t num_shown_elements, GG::Clr color) const
{ return GG::Wnd::Create<CUIDropDownList>(num_shown_elements); }

std::shared_ptr<GG::Edit> CUIStyle::NewEdit(const std::string& str, const std::shared_ptr<GG::Font>& font,
                                            GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                            GG::Clr interior/* = GG::CLR_ZERO*/) const
{ return GG::Wnd::Create<CUIEdit>(str); }

std::shared_ptr<GG::ListBox> CUIStyle::NewListBox(GG::Clr color, GG::Clr interior/* = GG::CLR_ZERO*/) const
{ return GG::Wnd::Create<CUIListBox>(); }

std::shared_ptr<GG::Scroll> CUIStyle::NewScroll(GG::Orientation orientation, GG::Clr color, GG::Clr interior) const
{ return GG::Wnd::Create<CUIScroll>(orientation); }

std::shared_ptr<GG::Slider<int>> CUIStyle::NewIntSlider(int min, int max, GG::Orientation orientation,
                                                        GG::Clr color, int tab_width, int line_width/* = 5*/) const
{ return GG::Wnd::Create<CUISlider<int>>(min, max, orientation); }


std::shared_ptr<GG::TabBar> CUIStyle::NewTabBar(const std::shared_ptr<GG::Font>& font, GG::Clr color,
                                                GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return GG::Wnd::Create<CUITabBar>(font, color, text_color); }

std::shared_ptr<GG::Button> CUIStyle::NewScrollUpButton(GG::Clr color) const
{ return nullptr; }

std::shared_ptr<GG::Button> CUIStyle::NewScrollDownButton(GG::Clr color) const
{ return NewScrollUpButton(color); }

std::shared_ptr<GG::Button> CUIStyle::NewVScrollTabButton(GG::Clr color) const
{
    return GG::Wnd::Create<CUIScroll::ScrollTab>(GG::VERTICAL, 1,
                                                 (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color,
                                                 ClientUI::CtrlBorderColor());
}

std::shared_ptr<GG::Button> CUIStyle::NewScrollLeftButton(GG::Clr color) const
{ return NewScrollUpButton(color); }

std::shared_ptr<GG::Button> CUIStyle::NewScrollRightButton(GG::Clr color) const
{ return NewScrollUpButton(color); }

std::shared_ptr<GG::Button> CUIStyle::NewHScrollTabButton(GG::Clr color) const
{
    return GG::Wnd::Create<CUIScroll::ScrollTab>(GG::HORIZONTAL, 1,
                                                 (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color,
                                                 ClientUI::CtrlBorderColor());
}

std::shared_ptr<GG::Button> CUIStyle::NewVSliderTabButton(GG::Clr color) const
{ return GG::Wnd::Create<CUIScroll::ScrollTab>(GG::VERTICAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

std::shared_ptr<GG::Button> CUIStyle::NewHSliderTabButton(GG::Clr color) const
{ return GG::Wnd::Create<CUIScroll::ScrollTab>(GG::HORIZONTAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

std::shared_ptr<GG::Button> CUIStyle::NewSpinIncrButton(
    const std::shared_ptr<GG::Font>& font, GG::Clr color) const
{ return GG::Wnd::Create<CUIArrowButton>(ShapeOrientation::UP, false, GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN); }

std::shared_ptr<GG::Button> CUIStyle::NewSpinDecrButton(
    const std::shared_ptr<GG::Font>& font, GG::Clr color) const
{ return GG::Wnd::Create<CUIArrowButton>(ShapeOrientation::DOWN, false, GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN); }

std::shared_ptr<GG::StateButton> CUIStyle::NewTabBarTab(
    const std::string& str, const std::shared_ptr<GG::Font>& font,
    GG::Flags<GG::TextFormat> format, GG::Clr color,
    GG::Clr text_color/* = GG::CLR_BLACK*/) const
{
    auto retval = GG::Wnd::Create<CUIStateButton>(str, format, std::make_shared<CUITabRepresenter>());
    retval->SetColor(ClientUI::WndColor());
    retval->GetLabel()->SetTextColor(DarkColor(ClientUI::TextColor()));
    retval->Resize(retval->MinUsableSize() + GG::Pt(GG::X(12), GG::Y0));
    return retval;
}

std::shared_ptr<GG::Button> CUIStyle::NewTabBarLeftButton(
    const std::shared_ptr<GG::Font>& font,
    GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return GG::Wnd::Create<CUIArrowButton>(ShapeOrientation::LEFT, true, GG::INTERACTIVE); }

std::shared_ptr<GG::Button> CUIStyle::NewTabBarRightButton(
    const std::shared_ptr<GG::Font>& font,
    GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return GG::Wnd::Create<CUIArrowButton>(ShapeOrientation::RIGHT, true, GG::INTERACTIVE); }
