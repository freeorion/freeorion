// -*- C++ -*-
#ifndef _CUISpin_h_
#define _CUISpin_h_

#include "CUIControls.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../util/OptionsDB.h"

#include <GG/Spin.h>


namespace detail {
    struct PlayValueChangedSound
    {
        void operator()(double) const;
        void operator()(int) const;
    };
}


/** a FreeOrion Spin control */
template <class T> class CUISpin : public GG::Spin<T>
{
public:
    typedef typename GG::Spin<T>::ValueType ValueType;

    /** \name Structors */ //@{
    CUISpin(GG::X x, GG::Y y, GG::X w, T value, T step, T min, T max, bool edits) :
        GG::Spin<T>(x, y, w, value, step, min, max, edits, ClientUI::GetFont(), ClientUI::CtrlBorderColor(), 
                    ClientUI::TextColor(), GG::CLR_ZERO)
    {
        GG::Connect(GG::Spin<T>::ValueChangedSignal, detail::PlayValueChangedSound(), -1);
        if (GG::Spin<T>::GetEdit())
            GG::Spin<T>::GetEdit()->SetHiliteColor(ClientUI::EditHiliteColor());
    }

    /** \name Mutators */ //@{
    virtual void Render()
    {
        GG::Clr color_to_use = this->Disabled() ? DisabledColor(this->Color()) : this->Color();
        GG::Clr int_color_to_use = this->Disabled() ? DisabledColor(this->InteriorColor()) : this->InteriorColor();
        GG::Pt ul = this->UpperLeft(), lr = this->LowerRight();
        FlatRectangle(ul, lr, int_color_to_use, color_to_use, 1);
    }
    //@}
};

namespace detail {
    inline void PlayValueChangedSound::operator()(double) const
    {
        std::string file_name = GetOptionsDB().Get<std::string>("UI.sound.button-click");
        Sound::GetSound().PlaySound(ClientUI::SoundDir() / file_name, true);
    }
    inline void PlayValueChangedSound::operator()(int) const {operator()(0.0);}
}

#endif // _CUISpin_h_
