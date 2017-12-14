#ifndef _CUISpin_h_
#define _CUISpin_h_

#include "CUIControls.h"
#include "Sound.h"
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
template <class T>
class CUISpin : public GG::Spin<T>
{
public:
    typedef typename GG::Spin<T>::ValueType ValueType;

    /** \name Structors */ //@{
    CUISpin(T value, T step, T min, T max, bool edits) :
    GG::Spin<T>(value, step, min, max, edits, ClientUI::GetFont(), ClientUI::CtrlBorderColor(),
                ClientUI::TextColor())
    {}

    void CompleteConstruction() override {
        GG::Spin<T>::CompleteConstruction();

        GG::Spin<T>::ValueChangedSignal.connect(detail::PlayValueChangedSound());
        if (GG::Spin<T>::GetEdit())
            GG::Spin<T>::GetEdit()->SetHiliteColor(ClientUI::EditHiliteColor());
        this->SetEditTextFromValue();
    }

    /** \name Mutators */ //@{
    void Render() override {
        GG::Clr color_to_use = this->Disabled() ? DisabledColor(this->Color()) : this->Color();
        GG::Clr int_color_to_use = this->Disabled() ? DisabledColor(this->InteriorColor()) : this->InteriorColor();
        GG::Pt ul = this->UpperLeft(), lr = this->LowerRight();
        FlatRectangle(ul, lr, int_color_to_use, color_to_use, 1);
    }

    void SetEditTextFromValue() override;
    //@}
};

template <class T>
void CUISpin<T>::SetEditTextFromValue()
{ GG::Spin<T>::SetEditTextFromValue(); }

template<>
void CUISpin<double>::SetEditTextFromValue();

namespace detail {
    inline void PlayValueChangedSound::operator()(double) const
    {
        std::string file_name = GetOptionsDB().Get<std::string>("ui.button.press.sound.path");
        Sound::GetSound().PlaySound(file_name, true);
    }
    inline void PlayValueChangedSound::operator()(int) const {operator()(0.0);}
}

#endif // _CUISpin_h_
