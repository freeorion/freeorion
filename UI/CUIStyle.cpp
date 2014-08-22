#include "CUIStyle.h"

#include "CUIControls.h"
#include "CUISpin.h"
#include "CUISlider.h"


GG::Button* CUIStyle::NewButton(const std::string& str,
                                const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIButton(str); }

GG::DropDownList* CUIStyle::NewDropDownList(size_t num_shown_elements, GG::Clr color,
                                            GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIDropDownList(num_shown_elements); }

GG::Edit* CUIStyle::NewEdit(const std::string& str, const boost::shared_ptr<GG::Font>& font,
                            GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                            GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIEdit(str); }

GG::ListBox* CUIStyle::NewListBox(GG::Clr color, GG::Clr interior/* = GG::CLR_ZERO*/,
                                  GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIListBox(); }

GG::Scroll* CUIStyle::NewScroll(GG::Orientation orientation, GG::Clr color, GG::Clr interior,
                                GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return new CUIScroll(orientation); }

GG::Slider<int>* CUIStyle::NewIntSlider(int min, int max, GG::Orientation orientation,
                                        GG::SliderLineStyle style, GG::Clr color, int tab_width, int line_width/* = 5*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUISlider<int>(GG::X0, GG::Y0, GG::X1, GG::Y1, min, max, orientation); }


GG::TabBar* CUIStyle::NewTabBar(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                GG::TabBarStyle style/* = GG::TAB_BAR_ATTACHED*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUITabBar(font, color, text_color, style, flags); }

GG::Button* CUIStyle::NewScrollUpButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return 0; }

GG::Button* CUIStyle::NewScrollDownButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return NewScrollUpButton(color, text_color, flags); }

GG::Button* CUIStyle::NewVScrollTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, 1, (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewScrollLeftButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return NewScrollUpButton(color, text_color, flags); }

GG::Button* CUIStyle::NewScrollRightButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                           GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return NewScrollUpButton(color, text_color, flags); }

GG::Button* CUIStyle::NewHScrollTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, 1, (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewVSliderTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewHSliderTabButton(GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, 0, ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewSpinIncrButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return new CUIArrowButton(SHAPE_UP, flags); }

GG::Button* CUIStyle::NewSpinDecrButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return new CUIArrowButton(SHAPE_DOWN, flags); }

GG::StateButton* CUIStyle::NewTabBarTab(const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format, GG::Clr color,
                                        GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                                        GG::StateButtonStyle style/* = GG::SBSTYLE_3D_TOP_ATTACHED_TAB*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    GG::StateButton* retval = new CUIStateButton(str, format, GG::SBSTYLE_3D_TOP_DETACHED_TAB);
    retval->Resize(retval->MinUsableSize() + GG::Pt(GG::X(12), GG::Y0));
    return retval;
}

GG::Button* CUIStyle::NewTabBarLeftButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(SHAPE_LEFT, flags);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

GG::Button* CUIStyle::NewTabBarRightButton(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                           GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(SHAPE_RIGHT, flags);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

void CUIStyle::DeleteWnd(GG::Wnd* wnd) const
{ delete wnd; }
