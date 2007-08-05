// -*- C++ -*-
#ifndef _CUIStyle_h_
#define _CUIStyle_h_

#include <GG/StyleFactory.h>

class CUIStyle : public GG::StyleFactory
{
public:
    virtual GG::Button*            NewButton(int x, int y, int w, int h, const std::string& str,
                                             const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                             GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::StateButton*       NewStateButton(int x, int y, int w, int h, const std::string& str,
                                                  const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt, GG::Clr color,
                                                  GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                  GG::StateButtonStyle style = GG::SBSTYLE_3D_XBOX, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::DropDownList*      NewDropDownList(int x, int y, int w, int h, int drop_ht, GG::Clr color,
                                                   GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Edit*              NewEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font,
                                           GG::Clr color, GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                           GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::ListBox*           NewListBox(int x, int y, int w, int h, GG::Clr color, GG::Clr interior = GG::CLR_ZERO,
                                              GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::MultiEdit*         NewMultiEdit(int x, int y, int w, int h, const std::string& str,
                                                const boost::shared_ptr<GG::Font>& font, GG::Clr color, Uint32 style = GG::TF_LINEWRAP,
                                                GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Scroll*            NewScroll(int x, int y, int w, int h, GG::Orientation orientation, GG::Clr color, GG::Clr interior,
                                             GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Slider*            NewSlider(int x, int y, int w, int h, int min, int max, GG::Orientation orientation,
                                             GG::SliderLineStyle style, GG::Clr color, int tab_width, int line_width = 5,
                                             GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Spin<int>*         NewIntSpin(int x, int y, int w, int value, int step, int min, int max, bool edits,
                                              const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                              GG::Clr interior = GG::CLR_ZERO, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Spin<double>*      NewDoubleSpin(int x, int y, int w, double value, double step, double min, double max, bool edits,
                                                 const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                 GG::Clr interior = GG::CLR_ZERO, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Button*            NewScrollUpButton(int x, int y, int w, int h, const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewScrollDownButton(int x, int y, int w, int h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewVScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Button*            NewScrollLeftButton(int x, int y, int w, int h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewScrollRightButton(int x, int y, int w, int h, const std::string& str,
                                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                        GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewHScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Button*            NewVSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Button*            NewHSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Button*            NewSpinIncrButton(int x, int y, int w, int h, const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::Button*            NewSpinDecrButton(int x, int y, int w, int h, const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                     GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN) const;

    virtual GG::StateButton*       NewTabBarTab(int x, int y, int w, int h, const std::string& str,
                                                const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt, GG::Clr color,
                                                GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                GG::StateButtonStyle style = GG::SBSTYLE_3D_TOP_ATTACHED_TAB, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Button*            NewTabBarLeftButton(int x, int y, int w, int h, const std::string& str,
                                                       const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                       GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual GG::Button*            NewTabBarRightButton(int x, int y, int w, int h, const std::string& str,
                                                        const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                        GG::Flags<GG::WndFlag> flags = GG::CLICKABLE) const;

    virtual void                   DeleteWnd(GG::Wnd* wnd) const;
};

#endif // _CUIStyle_h_
