//OptionsWnd.cpp

#include "OptionsWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../util/OptionsDB.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "GGApp.h"
#include "GGClr.h"
#include "GGDrawUtil.h"
#include "dialogs/GGFileDlg.h"

#include <fstream>


namespace {
    bool temp_header_bool = RecordHeaderFile(OptionsWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


////////////////////////////////////////////
//   CONSTRUCTION/DESTRUCTION
////////////////////////////////////////////

OptionsWnd::OptionsWnd():
    CUI_Wnd(ClientUI::String("ABOUT_WINDOW_TITLE"),100,100,300,200, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_done(false)
{

    m_done_btn = new CUIButton(20,140,75,ClientUI::String("DONE"));
    m_music = new CUIStateButton(30,40,75,20,ClientUI::String("OPTIONS_MUSIC"),0);
	m_music->SetCheck(!(GetOptionsDB().Get<bool>("music-off")));
	
	m_audio_str = new GG::TextControl(15,20,ClientUI::String("OPTIONS_AUDIO"),ClientUI::FONT,ClientUI::SIDE_PANEL_PLANET_NAME_PTS,ClientUI::TEXT_COLOR);
	//m_audio_str = new GG::TextControl(10,10,"Audio",ClientUI::FONT,GG::CLR_WHITE,0,0);

    Init();    //attaches children and connects signals to slots
}//OptionsWnd()

void OptionsWnd::Init()
{
    //add children
    AttachChild(m_done_btn);
    AttachChild(m_music);
	AttachChild(m_audio_str);

    //attach signals
    GG::Connect(m_done_btn->ClickedSignal(), &OptionsWnd::OnDone, this);
    GG::Connect(m_music->CheckedSignal(), &OptionsWnd::OnMusic, this);
}//Init()

OptionsWnd::~OptionsWnd()
{

}//~OptionsWnd

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

bool OptionsWnd::Render()
{
    CUI_Wnd::Render();

    return true;
}//Render()

void OptionsWnd::Keypress (GG::Key key, Uint32 key_mods)
{
    if ((key == GG::GGK_RETURN) || (key == GG::GGK_ESCAPE)) // Same behaviour as if "done" was pressed
    {
      OnDone();
    }
}//Keypress()

///////////////////////////////////////////////
//   ACCESSORS
///////////////////////////////////////////////

///////////////////////////////////////////////
//   EVENT HANDLERS
///////////////////////////////////////////////

void OptionsWnd::OnDone()
{
    m_done = true;
}//OnDone()

//void OptionsWnd::OnMusic(GG::StateButton::CheckedSignalType& CST)
void OptionsWnd::OnMusic(bool checked)
{
	if (!checked)
	{
		GetOptionsDB().Set("music-off", true);
		HumanClientApp::GetApp()->StopMusic();
	}
	if (checked)
	{
		GetOptionsDB().Set("music-off", false);
		HumanClientApp::GetApp()->StartMusic();
	}
   
}//OnMusic()
