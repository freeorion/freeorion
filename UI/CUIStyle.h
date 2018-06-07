#ifndef _CUIStyle_h_
#define _CUIStyle_h_

#include <GG/StyleFactory.h>

class CUIStyle : public GG::StyleFactory
{
public:
    std::string Translate(const std::string& text) const override;

    std::shared_ptr<GG::Button> NewButton(const std::string& str,
                              const std::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                              GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) const override;

    std::shared_ptr<GG::DropDownList> NewDropDownList(size_t num_shown_elements, GG::Clr color) const override;

    std::shared_ptr<GG::Edit> NewEdit(const std::string& str, const std::shared_ptr<GG::Font>& font,
                                      GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                      GG::Clr interior = GG::CLR_ZERO) const override;

    std::shared_ptr<GG::ListBox> NewListBox(GG::Clr color, GG::Clr interior = GG::CLR_ZERO) const override;

    std::shared_ptr<GG::Scroll> NewScroll(GG::Orientation orientation, GG::Clr color, GG::Clr interior) const override;

    std::shared_ptr<GG::Slider<int>> NewIntSlider(int min, int max, GG::Orientation orientation,
                                                  GG::Clr color, int tab_width, int line_width = 5) const override;

    std::shared_ptr<GG::TabBar> NewTabBar(const std::shared_ptr<GG::Font>& font, GG::Clr color,
                                          GG::Clr text_color = GG::CLR_BLACK) const override;

    std::shared_ptr<GG::Button> NewScrollUpButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewScrollDownButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewVScrollTabButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewScrollLeftButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewScrollRightButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewHScrollTabButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewVSliderTabButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewHSliderTabButton(GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewSpinIncrButton(const std::shared_ptr<GG::Font>& font, GG::Clr color) const override;

    std::shared_ptr<GG::Button> NewSpinDecrButton(const std::shared_ptr<GG::Font>& font, GG::Clr color) const override;

    std::shared_ptr<GG::StateButton> NewTabBarTab(const std::string& str,
                                                  const std::shared_ptr<GG::Font>& font,
                                                  GG::Flags<GG::TextFormat> format, GG::Clr color,
                                                  GG::Clr text_color = GG::CLR_BLACK) const override;

    std::shared_ptr<GG::Button> NewTabBarLeftButton(const std::shared_ptr<GG::Font>& font, GG::Clr color,
                                                    GG::Clr text_color = GG::CLR_BLACK) const override;

    std::shared_ptr<GG::Button> NewTabBarRightButton(const std::shared_ptr<GG::Font>& font, GG::Clr color,
                                                     GG::Clr text_color = GG::CLR_BLACK) const override;
};

#endif // _CUIStyle_h_
