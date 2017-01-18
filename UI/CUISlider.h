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
    CUISlider(T min, T max,
              GG::Orientation orientation, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE) :
        GG::Slider<T>(min, max, orientation, ClientUI::CtrlColor(), ClientUI::ScrollWidth(), 5, flags)
    {}
    //@}

    /** \name Mutators */ //@{
    void Render() override {
        const GG::Pt UL = this->UpperLeft();
        const GG::Pt LR = this->LowerRight();
        GG::Clr border_color_to_use = this->Disabled() ? GG::DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor();
        int tab_width = this->GetOrientation() == GG::VERTICAL ? Value(this->Tab()->Height()) : Value(this->Tab()->Width());
        GG::Pt ul, lr;
        if (this->GetOrientation() == GG::VERTICAL) {
            ul.x = ((LR.x + UL.x) - static_cast<int>(this->LineWidth())) / 2;
            lr.x   = ul.x + static_cast<int>(this->LineWidth());
            ul.y = UL.y + tab_width / 2;
            lr.y   = LR.y - tab_width / 2;
        } else {
            ul.x = UL.x + tab_width / 2;
            lr.x   = LR.x - tab_width / 2;
            ul.y = ((LR.y + UL.y) - static_cast<int>(this->LineWidth())) / 2;
            lr.y   = ul.y + static_cast<int>(this->LineWidth());
        }
        GG::FlatRectangle(ul, lr, GG::CLR_ZERO, border_color_to_use, 1);
    }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        GG::Wnd::SizeMove(ul, lr);
        if (this->GetOrientation() == GG::VERTICAL) {
            this->Tab()->Resize(GG::Pt(GG::X(this->TabWidth()), GG::Y(this->TabWidth())));
            this->Tab()->MoveTo(GG::Pt((this->Width() - this->Tab()->Width()) / 2, this->Tab()->RelativeUpperLeft().y));
            this->Tab()->SetMinSize(GG::Pt(this->Tab()->MinSize().x, GG::Y(10)));
        } else {
            this->Tab()->SizeMove(GG::Pt(GG::X(2), GG::Y0), GG::Pt(GG::X(this->TabWidth()), GG::Y(this->TabWidth())));
            this->Tab()->MoveTo(GG::Pt(this->Tab()->RelativeUpperLeft().x, (this->Height() - this->Tab()->Height()) / 2));
            this->Tab()->SetMinSize(GG::Pt(GG::X(10), this->Tab()->MinSize().y));
        }
        this->MoveTabToPosn();
    }
    //@}
};

#endif // _CUISlider_h_
