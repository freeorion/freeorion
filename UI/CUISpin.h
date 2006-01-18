// -*- C++ -*-
#ifndef _CUISpin_h_
#define _CUISpin_h_

#ifndef _CUIControls_h_
#include "CUIControls.h"
#endif

#ifndef _GG_Spin_h_
#include <GG/Spin.h>
#endif

#ifndef _HumanClientApp_h_
#include "../client/human/HumanClientApp.h"
#endif

#ifndef _OptionsDB_h_
#include "../util/OptionsDB.h"
#endif


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
    CUISpin(int x, int y, int w, T value, T step, T min, T max, bool edits) :
        GG::Spin<T>(x, y, w, value, step, min, max, edits, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), ClientUI::CTRL_BORDER_COLOR, 
                    ClientUI::TEXT_COLOR, GG::CLR_ZERO)
    {
        GG::Connect(GG::Spin<T>::ValueChangedSignal, detail::PlayValueChangedSound(), -1);
        if (GG::Spin<T>::GetEdit())
            GG::Spin<T>::GetEdit()->SetHiliteColor(ClientUI::EDIT_HILITE_COLOR);
    }

    /** \name Mutators */ //@{
    virtual void Render()
    {
        GG::Clr color_to_use = this->Disabled() ? DisabledColor(this->Color()) : this->Color();
        GG::Clr int_color_to_use = this->Disabled() ? DisabledColor(this->InteriorColor()) : this->InteriorColor();
        GG::Pt ul = this->UpperLeft(), lr = this->LowerRight();
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, 1);
    }
    //@}
};

namespace detail {
    inline void PlayValueChangedSound::operator()(double) const
    {
        std::string sound_dir = GetOptionsDB().Get<std::string>("settings-dir");
        if (!sound_dir.empty() && sound_dir[sound_dir.size() - 1] != '/')
            sound_dir += '/';
        sound_dir += "data/sound/";
        if (GetOptionsDB().Get<bool>("UI.sound.enabled"))
            HumanClientApp::GetApp()->PlaySound(sound_dir + GetOptionsDB().Get<std::string>("UI.sound.button-click"));
    }
    inline void PlayValueChangedSound::operator()(int) const {operator()(0.0);}
}

inline std::pair<std::string, std::string> CUISpinRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _CUISpin_h_
