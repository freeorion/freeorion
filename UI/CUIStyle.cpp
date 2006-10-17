#include "CUIStyle.h"

#include "CUIControls.h"
#include "CUISpin.h"


GG::Button* CUIStyle::NewButton(int x, int y, int w, int h, const std::string& str,
                                const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIButton(x, y, w, str);
}

GG::StateButton* CUIStyle::NewStateButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt, GG::Clr color,
                                          GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                                          GG::StateButtonStyle style/* = GG::SBSTYLE_3D_XBOX*/, Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIStateButton(x, y, w, h, str, text_fmt, style);
}

GG::DropDownList* CUIStyle::NewDropDownList(int x, int y, int w, int h, int drop_ht, GG::Clr color,
                                            Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIDropDownList(x, y, w, h, drop_ht);
}

GG::Edit* CUIStyle::NewEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font,
                            GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                            Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIEdit(x, y, w, str);
}

GG::ListBox* CUIStyle::NewListBox(int x, int y, int w, int h, GG::Clr color, GG::Clr interior/* = GG::CLR_ZERO*/,
                                  Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIListBox(x, y, w, h);
}

GG::MultiEdit* CUIStyle::NewMultiEdit(int x, int y, int w, int h, const std::string& str,
                                      const boost::shared_ptr<GG::Font>& font, GG::Clr color, Uint32 style/* = GG::TF_LINEWRAP*/,
                                      GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                                      Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIMultiEdit(x, y, w, h, str);
}

GG::Scroll* CUIStyle::NewScroll(int x, int y, int w, int h, GG::Orientation orientation, GG::Clr color, GG::Clr interior,
                                Uint32 flags/* = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN*/) const
{
    return new CUIScroll(x, y, w, h, orientation);
}

GG::Slider* CUIStyle::NewSlider(int x, int y, int w, int h, int min, int max, GG::Orientation orientation,
                                GG::SliderLineStyle style, GG::Clr color, int tab_width, int line_width/* = 5*/,
                                Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUISlider(x, y, w, h, min, max, orientation);
}

GG::Spin<int>* CUIStyle::NewIntSpin(int x, int y, int w, int value, int step, int min, int max, bool edits,
                                    const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                    GG::Clr interior/* = GG::CLR_ZERO*/, Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUISpin<int>(x, y, w, value, step, min, max, edits);
}

GG::Spin<double>* CUIStyle::NewDoubleSpin(int x, int y, int w, double value, double step, double min, double max, bool edits,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          GG::Clr interior/* = GG::CLR_ZERO*/, Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUISpin<double>(x, y, w, value, step, min, max, edits);
}

GG::Button* CUIStyle::NewScrollUpButton(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        Uint32 flags/* = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN*/) const
{
    return new GG::Button(-1, -1, 1, 1, "", boost::shared_ptr<GG::Font>(), GG::CLR_ZERO, GG::CLR_ZERO, 0);
}

GG::Button* CUIStyle::NewScrollDownButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          Uint32 flags/* = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN*/) const
{
    return NewScrollUpButton(x, y, w, h, str, font, color, text_color, flags);
}

GG::Button* CUIStyle::NewVScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIScroll::ScrollTab(GG::VERTICAL, w, (color == GG::CLR_ZERO) ? ClientUI::ScrollTabColor() : color, ClientUI::CtrlBorderColor());
}

GG::Button* CUIStyle::NewScrollLeftButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          Uint32 flags/* = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN*/) const
{
    return NewScrollUpButton(x, y, w, h, str, font, color, text_color, flags);
}

GG::Button* CUIStyle::NewScrollRightButton(int x, int y, int w, int h, const std::string& str,
                                           const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                           Uint32 flags/* = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN*/) const
{
    return NewScrollUpButton(x, y, w, h, str, font, color, text_color, flags);
}

GG::Button* CUIStyle::NewHScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIScroll::ScrollTab(GG::HORIZONTAL, h, (color == GG::CLR_ZERO) ? ClientUI::ScrollTabColor() : color, ClientUI::CtrlBorderColor());
}

GG::Button* CUIStyle::NewVSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIScroll::ScrollTab(GG::VERTICAL, w, ClientUI::ScrollTabColor(), ClientUI::CtrlBorderColor());
}

GG::Button* CUIStyle::NewHSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          Uint32 flags/* = GG::CLICKABLE*/) const
{
    return new CUIScroll::ScrollTab(GG::HORIZONTAL, h, ClientUI::ScrollTabColor(), ClientUI::CtrlBorderColor());
}

GG::Button* CUIStyle::NewSpinIncrButton(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        Uint32 flags/* = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN*/) const
{
    return new CUIArrowButton(0, 0, 1, 1, SHAPE_UP, ClientUI::DropDownListArrowColor(), flags);
}

GG::Button* CUIStyle::NewSpinDecrButton(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                        Uint32 flags/* = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN*/) const
{
    return new CUIArrowButton(0, 0, 1, 1, SHAPE_DOWN, ClientUI::DropDownListArrowColor(), flags);
}

GG::StateButton* CUIStyle::NewTabBarTab(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt, GG::Clr color,
                                        GG::Clr text_color/* = GG::CLR_BLACK*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                                        GG::StateButtonStyle style/* = GG::SBSTYLE_3D_TOP_ATTACHED_TAB*/, Uint32 flags/* = GG::CLICKABLE*/) const
{
    GG::StateButton* retval = new CUIStateButton(x, y, w, h, str, text_fmt, GG::SBSTYLE_3D_TOP_DETACHED_TAB);
    retval->Resize(retval->MinUsableSize() + GG::Pt(12, 0));
    return retval;
}

GG::Button* CUIStyle::NewTabBarLeftButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                          Uint32 flags/* = GG::CLICKABLE*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(x, y, w, h, SHAPE_LEFT, ClientUI::DropDownListArrowColor(), flags);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

GG::Button* CUIStyle::NewTabBarRightButton(int x, int y, int w, int h, const std::string& str,
                                           const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color/* = GG::CLR_BLACK*/,
                                           Uint32 flags/* = GG::CLICKABLE*/) const
{
    CUIArrowButton* retval = new CUIArrowButton(x, y, w, h, SHAPE_RIGHT, ClientUI::DropDownListArrowColor(), flags);
    retval->FillBackgroundWithWndColor(true);
    return retval;
}

void CUIStyle::DeleteWnd(GG::Wnd* wnd) const
{
    delete wnd;
}
