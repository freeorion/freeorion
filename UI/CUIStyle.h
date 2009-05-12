// -*- C++ -*-
#ifndef _CUIStyle_h_
#define _CUIStyle_h_

#include <GG/StyleFactory.h>

class CUIStyle : public GG::StyleFactory
{
public:
    virtual GG::Button*            NewButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                             const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::StateButton*       NewStateButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                  const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format, GG::Clr color,
                                                  GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                  GG::StateButtonStyle style = GG::SBSTYLE_3D_XBOX, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::DropDownList*      NewDropDownList(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Y drop_ht, GG::Clr color,
                                                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Edit*              NewEdit(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font,
                                           GG::Clr color, GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                           GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::ListBox*           NewListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Clr color, GG::Clr interior = GG::CLR_ZERO,
                                              GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::MultiEdit*         NewMultiEdit(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP,
                                                GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Scroll*            NewScroll(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Orientation orientation, GG::Clr color, GG::Clr interior,
                                             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Slider*            NewSlider(GG::X x, GG::Y y, GG::X w, GG::Y h, int min, int max, GG::Orientation orientation,
                                             GG::SliderLineStyle style, GG::Clr color, int tab_width, int line_width = 5,
                                             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Spin<int>*         NewIntSpin(GG::X x, GG::Y y, GG::X w, int value, int step, int min, int max, bool edits,
                                              const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                              GG::Clr interior = GG::CLR_ZERO, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Spin<double>*      NewDoubleSpin(GG::X x, GG::Y y, GG::X w, double value, double step, double min, double max, bool edits,
                                                 const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                 GG::Clr interior = GG::CLR_ZERO, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::TabBar*            NewTabBar(GG::X x, GG::Y y, GG::X w, const boost::shared_ptr<GG::Font>& font, GG::Clr color,
                                             GG::Clr text_color = GG::CLR_BLACK, GG::TabBarStyle style = GG::TAB_BAR_ATTACHED,
                                             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewScrollUpButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewScrollDownButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewVScrollTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewScrollLeftButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewScrollRightButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                        GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewHScrollTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewVSliderTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewHSliderTabButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewSpinIncrButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewSpinDecrButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::StateButton*       NewTabBarTab(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format, GG::Clr color,
                                                GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                GG::StateButtonStyle style = GG::SBSTYLE_3D_TOP_ATTACHED_TAB, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewTabBarLeftButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewTabBarRightButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                        GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual void                   DeleteWnd(GG::Wnd* wnd) const;
};

#endif // _CUIStyle_h_
