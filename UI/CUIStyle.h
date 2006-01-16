// -*- C++ -*-
#ifndef _CUIStyle_h_
#define _CUIStyle_h_

#include <GG/StyleFactory.h>

class CUIStyle : public GG::StyleFactory
{
public:
    virtual GG::Button*            NewButton(int x, int y, int w, int h, const std::string& str,
                                             const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                             Uint32 flags = GG::CLICKABLE) const;

    virtual GG::StateButton*       NewStateButton(int x, int y, int w, int h, const std::string& str,
                                                  const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt, GG::Clr color,
                                                  GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                  GG:: StateButtonStyle style = GG::SBSTYLE_3D_XBOX, Uint32 flags = GG::CLICKABLE) const;

    virtual GG::DropDownList*      NewDropDownList(int x, int y, int w, int h, int drop_ht, GG::Clr color,
                                                   Uint32 flags = GG::CLICKABLE) const;

    virtual GG::Edit*              NewEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font,
                                           GG::Clr color, GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                           Uint32 flags = GG::CLICKABLE | GG::DRAG_KEEPER) const;

    virtual GG::ListBox*           NewListBox(int x, int y, int w, int h, GG::Clr color, GG::Clr interior = GG::CLR_ZERO,
                                              Uint32 flags = GG::CLICKABLE | GG::DRAG_KEEPER) const;

    virtual GG::MultiEdit*         NewMultiEdit(int x, int y, int w, int h, const std::string& str,
                                                const boost::shared_ptr<GG::Font>& font, GG::Clr color, Uint32 style = GG::TF_LINEWRAP,
                                                GG::Clr text_color = GG::CLR_BLACK, GG::Clr interior = GG::CLR_ZERO,
                                                Uint32 flags = GG::CLICKABLE |GG::DRAG_KEEPER) const;

    virtual GG::Scroll*            NewScroll(int x, int y, int w, int h, GG::Orientation orientation, GG::Clr color, GG::Clr interior,
                                             Uint32 flags = GG::CLICKABLE) const;

    virtual GG::Slider*            NewSlider(int x, int y, int w, int h, int min, int max, GG::Orientation orientation,
                                             GG::SliderLineStyle style, GG::Clr color, int tab_width, int line_width = 5,
                                             Uint32 flags = GG::CLICKABLE) const;

    virtual GG::Spin<int>*         NewIntSpin(int x, int y, int w, int value, int step, int min, int max, bool edits,
                                              const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                              GG::Clr interior = GG::CLR_ZERO, Uint32 flags = GG::CLICKABLE | GG::DRAG_KEEPER) const;

    virtual GG::Spin<double>*      NewDoubleSpin(int x, int y, int w, double value, double step, double min, double max, bool edits,
                                                 const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color = GG::CLR_BLACK,
                                                 GG::Clr interior = GG::CLR_ZERO, Uint32 flags = GG::CLICKABLE | GG::DRAG_KEEPER) const;

    virtual void                   DeleteWnd(GG::Wnd* wnd) const;
};

#endif // _CUIStyle_h_
