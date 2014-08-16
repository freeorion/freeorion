#include "CUIStyle.h"

#include "CUIControls.h"
#include "CUISpin.h"
#include "CUISlider.h"


GG::Button* CUIStyle::NewButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    CUIButton* retval = new CUIButton(str);
    retval->MoveTo(GG::Pt(x, y));
    retval->Resize(GG::Pt(w, retval->MinUsableSize().y));
    return retval;
}

GG::DropDownList* CUIStyle::NewDropDownList(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Y drop_ht, GG::Clr color,
                                            GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    CUIDropDownList* retval = new CUIDropDownList(drop_ht);
    retval->MoveTo(GG::Pt(x, y));
    retval->Resize(GG::Pt(w, h));
    return retval;
}

GG::Edit* CUIStyle::NewEdit(const std::string& str, const boost::shared_ptr<GG::Font>& font,
                            GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                            GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIEdit(str); }

GG::ListBox* CUIStyle::NewListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Clr color, GG::Clr interior/* = GG::CLR_ZERO*/,
                                  GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    CUIListBox* retval = new CUIListBox();
    retval->MoveTo(GG::Pt(x, y));
    retval->Resize(GG::Pt(w, h));
    return retval;
}

GG::Scroll* CUIStyle::NewScroll(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Orientation orientation, GG::Clr color, GG::Clr interior,
                                GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return new CUIScroll(x, y, w, h, orientation); }

GG::Slider<int>* CUIStyle::NewIntSlider(int min, int max, GG::Orientation orientation,
                                        GG::SliderLineStyle style, GG::Clr color, int tab_width, int line_width/* = 5*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUISlider<int>(GG::X0, GG::Y0, GG::X1, GG::Y1, min, max, orientation); }


GG::TabBar* CUIStyle::NewTabBar(GG::X x, GG::Y y, GG::X w, const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                GG::TabBarStyle style/* = GG::TAB_BAR_ATTACHED*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUITabBar(x, y, w, font, color, text_color, style, flags); }

GG::Button* CUIStyle::NewScrollUpButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return new GG::Button(-GG::X1, -GG::Y1, GG::X1, GG::Y1, "", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO, GG::CLR_ZERO, GG::NO_WND_FLAGS); }

GG::Button* CUIStyle::NewScrollDownButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return NewScrollUpButton(x, y, w, h, str, font, color, text_color, flags); }

GG::Button* CUIStyle::NewVScrollTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, Value(w), (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewScrollLeftButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return NewScrollUpButton(x, y, w, h, str, font, color, text_color, flags); }

GG::Button* CUIStyle::NewScrollRightButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                           const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                           GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return NewScrollUpButton(x, y, w, h, str, font, color, text_color, flags); }

GG::Button* CUIStyle::NewHScrollTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, Value(h), (color == GG::CLR_ZERO) ? ClientUI::CtrlColor() : color, ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewVSliderTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::VERTICAL, Value(w), ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewHSliderTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{ return new CUIScroll::ScrollTab(GG::HORIZONTAL, Value(h), ClientUI::CtrlColor(), ClientUI::CtrlBorderColor()); }

GG::Button* CUIStyle::NewSpinIncrButton(const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return new CUIArrowButton(GG::X0, GG::Y0, GG::X1, GG::Y1, SHAPE_UP, flags); }

GG::Button* CUIStyle::NewSpinDecrButton(const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN*/) const
{ return new CUIArrowButton(GG::X0, GG::Y0, GG::X1, GG::Y1, SHAPE_DOWN, flags); }

GG::StateButton* CUIStyle::NewTabBarTab(const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format, GG::Clr color,
                                        GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                                        GG::StateButtonStyle style/* = GG::SBSTYLE_3D_TOP_ATTACHED_TAB*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    GG::StateButton* retval = new CUIStateButton(str, format, GG::SBSTYLE_3D_TOP_DETACHED_TAB);
    retval->Resize(retval->MinUsableSize() + GG::Pt(GG::X(12), GG::Y0));
    return retval;
}

GG::Button* CUIStyle::NewTabBarLeftButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(x, y, w, h, SHAPE_LEFT, flags);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

GG::Button* CUIStyle::NewTabBarRightButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                           const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                           GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(x, y, w, h, SHAPE_RIGHT, flags);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

void CUIStyle::DeleteWnd(GG::Wnd* wnd) const
{ delete wnd; }
