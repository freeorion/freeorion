// -*- C++ -*-
#ifndef _OptionsWnd_h_
#define _OptionsWnd_h_

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

class CUIButton;
class CUIStateButton;
class CUISlider;
namespace GG {
    class TextControl;
}

//! This is a dialog box that allows the user to control certain basic game parameters, such as sound and music
class OptionsWnd : public CUI_Wnd
{
public:
    //! \name Structors
    //!@{
    OptionsWnd();  //!< default ctor
    ~OptionsWnd(); //!< dtor
    //!@}

    //! \name Mutators
    //!@{
    virtual void Keypress (GG::Key key, Uint32 key_mods);
    //!@}

    bool m_end_with_done;    //!< determines whether or not we ended the dialog with OK or not

private:
    void Init();
    void DoneClicked();
	void MusicCicked(bool checked);
	void UIEffectsCicked(bool checked);
    void MusicVolumeSlid(int pos, int low, int high);
    void UISoundsVolumeSlid(int pos, int low, int high);

    CUIButton*       m_done_btn;    //!< Done button
    CUIStateButton*  m_music;       //!< Music enabled/disabled
    CUIStateButton*  m_ui_effects;  //!< UI sound effects enabled/disabled
    CUISlider*       m_music_volume;
    GG::TextControl* m_music_volume_label;
    CUISlider*       m_ui_sounds_volume;
    GG::TextControl* m_ui_sounds_volume_label;

    GG::TextControl* m_audio_str;   //!< Audio title string
};

inline std::pair<std::string, std::string> OptionsWndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _OptionsWnd_h_
