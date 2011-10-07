// -*- C++ -*-
#ifndef _CUISlider_h_
#define _CUISlider_h_

#include "CUIControls.h"

#include <GG/Slider.h>

/** a FreeOrion Slider control */
template <class T>
class CUISlider : public GG::Slider<T>
{
public:
    /** \name Structors */ //@{
    CUISlider(GG::X x, GG::Y y, GG::X w, GG::Y h, T min, T max,
              GG::Orientation orientation, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) :
        GG::Slider<T>(x, y, w, h, min, max, orientation, GG::FLAT, ClientUI::CtrlColor(),
                      orientation == GG::VERTICAL ? Value(w) : Value(h), 5, flags)
    {}
    //@}

    /** \name Mutators */ //@{
    virtual void    Render()
    {
        const GG::Pt UL = UpperLeft();
        const GG::Pt LR = LowerRight();
        GG::Clr border_color_to_use = Disabled() ? GG::DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor();
        int tab_width = GetOrientation() == GG::VERTICAL ? Value(Tab()->Height()) : Value(Tab()->Width());
        GG::Pt ul, lr;
        if (GetOrientation() == GG::VERTICAL) {
            ul.x = ((LR.x + UL.x) - static_cast<int>(LineWidth())) / 2;
            lr.x   = ul.x + static_cast<int>(LineWidth());
            ul.y = UL.y + tab_width / 2;
            lr.y   = LR.y - tab_width / 2;
        } else {
            ul.x = UL.x + tab_width / 2;
            lr.x   = LR.x - tab_width / 2;
            ul.y = ((LR.y + UL.y) - static_cast<int>(LineWidth())) / 2;
            lr.y   = ul.y + static_cast<int>(LineWidth());
        }
        GG::FlatRectangle(ul, lr, GG::CLR_ZERO, border_color_to_use, 1);
    }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr)
    {
        Wnd::SizeMove(ul, lr);
        if (GetOrientation() == GG::VERTICAL) {
            Tab()->Resize(GG::Pt(GG::X(TabWidth()), GG::Y(TabWidth())));
            Tab()->MoveTo(GG::Pt((Width() - Tab()->Width()) / 2, Tab()->RelativeUpperLeft().y));
            Tab()->SetMinSize(GG::Pt(Tab()->MinSize().x, GG::Y(10)));
        } else {
            Tab()->SizeMove(GG::Pt(GG::X(2), GG::Y0), GG::Pt(GG::X(TabWidth()), GG::Y(TabWidth())));
            Tab()->MoveTo(GG::Pt(Tab()->RelativeUpperLeft().x, (Height() - Tab()->Height()) / 2));
            Tab()->SetMinSize(GG::Pt(GG::X(10), Tab()->MinSize().y));
        }
        MoveTabToPosn();
    }
    //@}
};

#endif // _CUISlider_h_
