//OptionsWnd.cpp

#include "OptionsWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "GGApp.h"
#include "GGClr.h"
#include "GGDrawUtil.h"

#include <fstream>


namespace {
    bool temp_header_bool = RecordHeaderFile(OptionsWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


OptionsWnd::OptionsWnd():
    CUI_Wnd(ClientUI::String("ABOUT_WINDOW_TITLE"),100,100,300,205, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_done(false)
{
    bool UI_sounds_enabled = GetOptionsDB().Get<bool>("UI.sound.enabled");
    TempUISoundDisabler sound_disabler;

	m_audio_str = new GG::TextControl(15,20,ClientUI::String("OPTIONS_AUDIO"),ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS,ClientUI::TEXT_COLOR);

    m_music = new CUIStateButton(30,40,75,20,ClientUI::String("OPTIONS_MUSIC"),GG::TF_LEFT);
    m_ui_effects = new CUIStateButton(30,65,75,20,ClientUI::String("OPTIONS_UI_SOUNDS"),GG::TF_LEFT);
	
    m_music_volume = new CUISlider(30, 90, 150, 14, 0, 255, CUISlider::HORIZONTAL);
    m_music_volume_label = new GG::TextControl(30,105,ClientUI::String("OPTIONS_MUSIC_VOLUME"),ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS,ClientUI::TEXT_COLOR);
    m_ui_sounds_volume = new CUISlider(30, 130, 150, 14, 0, 255, CUISlider::HORIZONTAL);
    m_ui_sounds_volume_label = new GG::TextControl(30,145,ClientUI::String("OPTIONS_UI_SOUNDS_VOLUME"),ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS,ClientUI::TEXT_COLOR);

    m_done_btn = new CUIButton(20,170,75,ClientUI::String("DONE"));

	m_music->SetCheck(!GetOptionsDB().Get<bool>("music-off"));
	m_ui_effects->SetCheck(UI_sounds_enabled);
    m_music_volume->SlideTo(GetOptionsDB().Get<int>("music-volume"));
    m_ui_sounds_volume->SlideTo(GetOptionsDB().Get<int>("UI.sound.volume"));

    Init();
}

void OptionsWnd::Init()
{
	AttachChild(m_audio_str);
    AttachChild(m_music);
	AttachChild(m_ui_effects);
    AttachChild(m_music_volume);
    AttachChild(m_music_volume_label);
	AttachChild(m_ui_sounds_volume);
	AttachChild(m_ui_sounds_volume_label);
    AttachChild(m_done_btn);

    GG::Connect(m_done_btn->ClickedSignal(), &OptionsWnd::DoneClicked, this);
    GG::Connect(m_music->CheckedSignal(), &OptionsWnd::MusicCicked, this);
    GG::Connect(m_ui_effects->CheckedSignal(), &OptionsWnd::UIEffectsCicked, this);
    GG::Connect(m_music_volume->SlidSignal(), &OptionsWnd::MusicVolumeSlid, this);
    GG::Connect(m_ui_sounds_volume->SlidAndStoppedSignal(), &OptionsWnd::UISoundsVolumeSlid, this);
}

OptionsWnd::~OptionsWnd()
{
}

void OptionsWnd::Keypress (GG::Key key, Uint32 key_mods)
{
    if ((key == GG::GGK_RETURN) || (key == GG::GGK_ESCAPE)) // Same behaviour as if "done" was pressed
        DoneClicked();
}

void OptionsWnd::DoneClicked()
{
    m_done = true;
}

void OptionsWnd::MusicCicked(bool checked)
{
	if (!checked)
	{
		GetOptionsDB().Set("music-off", true);
		HumanClientApp::GetApp()->StopMusic();
	}
    else
	{
		GetOptionsDB().Set("music-off", false);
        HumanClientApp::GetApp()->PlayMusic(ClientUI::SOUND_DIR + GetOptionsDB().Get<std::string>("bg-music"), true);
	}
   
}

void OptionsWnd::UIEffectsCicked(bool checked)
{
    GetOptionsDB().Set("UI.sound.enabled", checked);
}

void OptionsWnd::MusicVolumeSlid(int pos, int low, int high)
{
    HumanClientApp::GetApp()->SetMusicVolume(pos);
}

void OptionsWnd::UISoundsVolumeSlid(int pos, int low, int high)
{
    HumanClientApp::GetApp()->SetUISoundsVolume(pos);
    HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.button-click"));
}
