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
    CUI_Wnd(ClientUI::String("ABOUT_WINDOW_TITLE"),100,100,300,200, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_done(false)
{
    TempUISoundDisabler sound_disabler;

    m_done_btn = new CUIButton(20,140,75,ClientUI::String("DONE"));
    m_music = new CUIStateButton(30,40,75,20,ClientUI::String("OPTIONS_MUSIC"),GG::TF_LEFT);
	m_music->SetCheck(!GetOptionsDB().Get<bool>("music-off"));
    m_ui_effects = new CUIStateButton(30,65,75,20,ClientUI::String("OPTIONS_UI_EFFECTS"),GG::TF_LEFT);
	m_ui_effects->SetCheck(!GetOptionsDB().Get<bool>("UI.sound.enabled"));
	
	m_audio_str = new GG::TextControl(15,20,ClientUI::String("OPTIONS_AUDIO"),ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS,ClientUI::TEXT_COLOR);

    Init();
}

void OptionsWnd::Init()
{
    AttachChild(m_done_btn);
    AttachChild(m_music);
	AttachChild(m_ui_effects);
	AttachChild(m_audio_str);

    GG::Connect(m_done_btn->ClickedSignal(), &OptionsWnd::DoneClicked, this);
    GG::Connect(m_music->CheckedSignal(), &OptionsWnd::MusicCicked, this);
    GG::Connect(m_ui_effects->CheckedSignal(), &OptionsWnd::UIEffectsCicked, this);
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
		HumanClientApp::GetApp()->StartMusic();
	}
   
}

void OptionsWnd::UIEffectsCicked(bool checked)
{
	if (checked)
	{
		GetOptionsDB().Set("UI.sound.enabled", true);
	}
    else
	{
		GetOptionsDB().Set("UI.sound.enabled", false);
	}
   
}
