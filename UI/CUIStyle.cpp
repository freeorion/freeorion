#include "CUIStyle.h"

#include "CUIControls.h"
#include "CUISpin.h"
#include "CUISlider.h"


GG::Button* CUIStyle::NewButton(const std::string& str,
                                const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIButton(str); }

GG::DropDownList* CUIStyle::NewDropDownList(size_t num_shown_elements, GG::Clr color) const
{ return new CUIDropDownList(num_shown_elements); }

GG::Edit* CUIStyle::NewEdit(const std::string& str, const boost::shared_ptr<GG::Font>& font,
                            GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/) const
{ return new CUIEdit(str); }

GG::ListBox* CUIStyle::NewListBox(GG::Clr color, GG::Clr interior/* = GG::CLR_ZERO*/) const
{ return new CUIListBox(); }

GG::Scroll* CUIStyle::NewScroll(GG::Orientation orientation, GG::Clr color, GG::Clr interior) const
{ return new CUIScroll(orientation); }

GG::Slider<int>* CUIStyle::NewIntSlider(int min, int max, GG::Orientation orientation,
                                        GG::Clr color, int tab_width, int line_width/* = 5*/) const
{ return new CUISlider<int>(min, max, orientation); }


GG::TabBar* CUIStyle::NewTabBar(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                GG::TabBarStyle style/* = GG::TAB_BAR_ATTACHED*/) const
{ return new CUITabBar(font, color, text_color, style); }

GG::Button* CUIStyle::NewScrollUpButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return 0; }

GG::Button* CUIStyle::NewScrollDownButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return NewScrollUpButton(color, text_color); }

GG::Button* CUIStyle::NewVScrollTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, 1, (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewScrollLeftButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return NewScrollUpButton(color, text_color); }

GG::Button* CUIStyle::NewScrollRightButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return NewScrollUpButton(color, text_color); }

GG::Button* CUIStyle::NewHScrollTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, 1, (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewVSliderTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewHSliderTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewSpinIncrButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIArrowButton(SHAPE_UP, GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN); }

GG::Button* CUIStyle::NewSpinDecrButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{ return new CUIArrowButton(SHAPE_DOWN, GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN); }

GG::StateButton* CUIStyle::NewTabBarTab(const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format, GG::Clr color,
                                        GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                                        GG::StateButtonStyle style/* = GG::SBSTYLE_3D_TOP_ATTACHED_TAB*/) const
{
    GG::StateButton* retval = new CUIStateButton(str, format, GG::SBSTYLE_3D_TOP_DETACHED_TAB);
    retval->Resize(retval->MinUsableSize() + GG::Pt(GG::X(12), GG::Y0));
    return retval;
}

GG::Button* CUIStyle::NewTabBarLeftButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(SHAPE_LEFT, GG::INTERACTIVE);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

GG::Button* CUIStyle::NewTabBarRightButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(SHAPE_RIGHT, GG::INTERACTIVE);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

void CUIStyle::DeleteWnd(GG::Wnd* wnd) const
{ delete wnd; }
