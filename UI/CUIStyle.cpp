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

GG::DropDownList* CUIStyle::NewDropDownList(size_t num_shown_elements, GG::Clr color) const
{ return new CUIDropDownList(num_shown_elements); }

GG::Edit* CUIStyle::NewEdit(const std::string& str, const std::shared_ptr<GG::Font>& font,
                            GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/) const
{ return new CUIEdit(str); }

GG::ListBox* CUIStyle::NewListBox(GG::Clr color, GG::Clr interior/* = GG::CLR_ZERO*/) const
{ return new CUIListBox(); }

GG::Scroll* CUIStyle::NewScroll(GG::Orientation orientation, GG::Clr color, GG::Clr interior) const
{ return new CUIScroll(orientation); }

GG::Slider<int>* CUIStyle::NewIntSlider(int min, int max, GG::Orientation orientation,
                                        GG::Clr color, int tab_width, int line_width/* = 5*/) const
{ return new CUISlider<int>(min, max, orientation); }


GG::TabBar* CUIStyle::NewTabBar(const std::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUITabBar(font, color, text_color); }

GG::Button* CUIStyle::NewScrollUpButton(GG::Clr color) const
{ return nullptr; }

GG::Button* CUIStyle::NewScrollDownButton(GG::Clr color) const
{ return NewScrollUpButton(color); }

GG::Button* CUIStyle::NewVScrollTabButton(GG::Clr color) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, 1, (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewScrollLeftButton(GG::Clr color) const
{ return NewScrollUpButton(color); }

GG::Button* CUIStyle::NewScrollRightButton(GG::Clr color) const
{ return NewScrollUpButton(color); }

GG::Button* CUIStyle::NewHScrollTabButton(GG::Clr color) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, 1, (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewVSliderTabButton(GG::Clr color) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewHSliderTabButton(GG::Clr color) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewSpinIncrButton(const std::shared_ptr<GG::Font>& font, GG::Clr color) const
{ return new CUIArrowButton(SHAPE_UP, false, GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN); }

GG::Button* CUIStyle::NewSpinDecrButton(const std::shared_ptr<GG::Font>& font, GG::Clr color) const
{ return new CUIArrowButton(SHAPE_DOWN, false, GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN); }

GG::StateButton* CUIStyle::NewTabBarTab(const std::string& str,
                                        const std::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format, GG::Clr color,
                                        GG::Clr text_color/* = GG::CLR_BLACK*/) const
{
    auto retval = new CUIStateButton(str, format, std::make_shared<CUITabRepresenter>());
    retval->SetColor(ClientUI::WndColor());
    retval->GetLabel()->SetTextColor(DarkColor(ClientUI::TextColor()));
    retval->Resize(retval->MinUsableSize() + GG::Pt(GG::X(12), GG::Y0));
    return retval;
}

GG::Button* CUIStyle::NewTabBarLeftButton(const std::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIArrowButton(SHAPE_LEFT, true, GG::INTERACTIVE); }

GG::Button* CUIStyle::NewTabBarRightButton(const std::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIArrowButton(SHAPE_RIGHT, true, GG::INTERACTIVE); }

void CUIStyle::DeleteWnd(GG::Wnd* wnd) const
{ delete wnd; }
