//InGameOptions.cpp

#include "InGameOptions.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "GGApp.h"
#include "GGClr.h"
#include "GGDrawUtil.h"
#include "dialogs/GGFileDlg.h"

#include <fstream>

////////////////////////////////////////////
//   CONSTANTS
////////////////////////////////////////////


////////////////////////////////////////////
//   CONSTRUCTION/DESTRUCTION
////////////////////////////////////////////

InGameOptions::InGameOptions():
    CUI_Wnd(ClientUI::String("INGAMEOPTIONS_WINDOW_TITLE"),(GG::App::GetApp()->AppWidth()/2) - 55,
    (GG::App::GetApp()->AppHeight()/2) - 120,135,240,
    GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_done(false)
{

    m_save_btn = new CUIButton(30,40,75,ClientUI::String("INGAMEOPTIONS_SAVE"));
    m_load_btn = new CUIButton(30,80,75,ClientUI::String("INGAMEOPTIONS_LOAD"));
    m_quit_btn = new CUIButton(30,120,75,ClientUI::String("INGAMEOPTIONS_QUIT"));
    m_done_btn = new CUIButton(30,180,75,ClientUI::String("DONE"));

    Init();    //attaches children and connects signals to slots
}//InGameOptions()

void InGameOptions::Init()
{
    //add children
    AttachChild(m_save_btn);
    AttachChild(m_load_btn);
    AttachChild(m_quit_btn);
    AttachChild(m_done_btn);

    //attach signals
    GG::Connect(m_save_btn->ClickedSignal(), &InGameOptions::OnSave, this);
    GG::Connect(m_load_btn->ClickedSignal(), &InGameOptions::OnLoad, this);
    GG::Connect(m_quit_btn->ClickedSignal(), &InGameOptions::OnQuit, this);
    GG::Connect(m_done_btn->ClickedSignal(), &InGameOptions::OnDone, this);

    m_quit = false;

}//Init()

InGameOptions::~InGameOptions()
{

}//~InGameOptions

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

int InGameOptions::Render()
{
    CUI_Wnd::Render();

    return true;
}//Render()

///////////////////////////////////////////////
//   ACCESSORS
///////////////////////////////////////////////

///////////////////////////////////////////////
//   EVENT HANDLERS
///////////////////////////////////////////////

void InGameOptions::OnSave()
{

    // TODO: implement saving of game

}//OnSave()

void InGameOptions::OnLoad()
{

    // TODO: implement loading of game

}//OnLoad()

void InGameOptions::OnQuit()
{
    GG::App::GetApp()->Exit(0);
}//OnQuit()

void InGameOptions::OnDone()
{
    m_done = true;
}//OnDone()

