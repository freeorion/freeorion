// -*- C++ -*-
#ifndef _CUIStyle_h_
#define _CUIStyle_h_

#include <GG/StyleFactory.h>

class CUIStyle : public GG::StyleFactory
{
public:
    virtual GG::Button*            NewButton(const std::string& str,
                                             const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::DropDownList*      NewDropDownList(size_t num_shown_elements, GG::Clr color,
                                                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Edit*              NewEdit(const std::string& str, const boost::shared_ptr<GG::Font>& font,
                                           GG::Clr color, GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                           GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::ListBox*           NewListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Clr color, GG::Clr interior = GG::CLR_ZERO,
                                              GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Scroll*            NewScroll(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Orientation orientation, GG::Clr color, GG::Clr interior,
                                             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Slider<int>*        NewIntSlider(int min, int max, GG::Orientation orientation,
                                                 GG::SliderLineStyle style, GG::Clr color, int tab_width, int line_width = 5,
                                                 GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;


    virtual GG::TabBar*            NewTabBar(GG::X x, GG::Y y, GG::X w, const boost::shared_ptr<GG::Font>& font, GG::Clr color,
                                             GG::Clr text_color = GG::CLR_BLACK, GG::TabBarStyle style = GG::TAB_BAR_ATTACHED,
                                             GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewScrollUpButton(const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewScrollDownButton(const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewVScrollTabButton(const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewScrollLeftButton(const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewScrollRightButton(const std::string& str,
                                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                        GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewHScrollTabButton(const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewVSliderTabButton(const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewHSliderTabButton(const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewSpinIncrButton(const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewSpinDecrButton(const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::StateButton*       NewTabBarTab(const std::string& str,
                                                const boost::shared_ptr<GG::Font>& font, GG::Flags<GG::TextFormat> format, GG::Clr color,
                                                GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                GG::StateButtonStyle style = GG::SBSTYLE_3D_TOP_ATTACHED_TAB, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewTabBarLeftButton(const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual GG::Button*            NewTabBarRightButton(const std::string& str,
                                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                        GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const;

    virtual void                   DeleteWnd(GG::Wnd* wnd) const;
};

#endif // _CUIStyle_h_
