#include "InGameOptions.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "GGButton.h"
#include "GGClr.h"
#include "GGDrawUtil.h"
#include "dialogs/GGFileDlg.h"
#include "../client/human/HumanClientApp.h"
#include "../util/OptionsDB.h"

#include <boost/filesystem/operations.hpp>

#include <fstream>

namespace {
    void Options(OptionsDB& db)
    {
        db.Add('S',
               "save-dir", 
               "The directory in which saved games are saved and from which they are loaded.  Directory names are taken to be relative to the location of the executable.",
               std::string("save"));
    }

    bool foo_bool = RegisterOptions(&Options);

    bool temp_header_bool = RecordHeaderFile(InGameOptionsRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


InGameOptions::InGameOptions():
    CUI_Wnd(ClientUI::String("INGAMEOPTIONS_WINDOW_TITLE"), (GG::App::GetApp()->AppWidth() / 2) - 55,
            (GG::App::GetApp()->AppHeight() / 2) - 140, 135, 280, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL)
{
    m_save_btn = new CUIButton(30,40,75,ClientUI::String("INGAMEOPTIONS_SAVE"));
    m_load_btn = new CUIButton(30,80,75,ClientUI::String("INGAMEOPTIONS_LOAD"));
	m_options_btn = new CUIButton(30,120,75,ClientUI::String("INTRO_BTN_OPTIONS"));
    m_exit_btn = new CUIButton(30,160,75,ClientUI::String("INGAMEOPTIONS_RESIGN"));
    m_done_btn = new CUIButton(30,210,75,ClientUI::String("DONE"));

    // call to InGameOptions::MinimizedLength() because MinimizedLength is virtual
    SetMinSize(GG::Pt(InGameOptions::MinimizedLength(),MinSize().y));
    Init(); //attaches children and connects signals to slots
}

InGameOptions::~InGameOptions()
{
}

int InGameOptions::MinimizedLength() const
{ 
    return 135;
}

bool InGameOptions::Render()
{
    CUI_Wnd::Render();
    return true;
}

void InGameOptions::Keypress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_RETURN || key == GG::GGK_ESCAPE) // Same behaviour as if "done" was pressed
        Done();
}

void InGameOptions::Init()
{
    //add children
    AttachChild(m_save_btn);
    AttachChild(m_load_btn);
	AttachChild(m_options_btn);
    AttachChild(m_exit_btn);
    AttachChild(m_done_btn);

    //attach signals
    GG::Connect(m_save_btn->ClickedSignal(), &InGameOptions::Save, this);
    GG::Connect(m_load_btn->ClickedSignal(), &InGameOptions::Load, this);
	GG::Connect(m_options_btn->ClickedSignal(), &InGameOptions::Options, this);
    GG::Connect(m_exit_btn->ClickedSignal(), &InGameOptions::Exit, this);
    GG::Connect(m_done_btn->ClickedSignal(), &InGameOptions::Done, this);

    if (!HumanClientApp::GetApp()->SinglePlayerGame()) {
        m_load_btn->Disable();
    }
}

void InGameOptions::Save()
{
    const std::string SAVE_GAME_EXTENSION = HumanClientApp::GetApp()->SinglePlayerGame() ? ".sav" : ".mps";

    std::vector<std::pair<std::string, std::string> > save_file_types;
    save_file_types.push_back(std::pair<std::string, std::string>(ClientUI::String("INGAMEOPTIONS_SAVE_FILES"), "*" + SAVE_GAME_EXTENSION));

    try {
        GG::FileDlg dlg(GetOptionsDB().Get<std::string>("save-dir"), "", true, false, save_file_types, 
                        ClientUI::FONT, ClientUI::PTS, ClientUI::WND_COLOR, ClientUI::WND_OUTER_BORDER_COLOR, ClientUI::TEXT_COLOR);
        dlg.Run();
        std::string filename;
        if (!dlg.Result().empty()) {
            filename = *dlg.Result().begin();
            if (filename.find(SAVE_GAME_EXTENSION) != filename.size() - SAVE_GAME_EXTENSION.size())
                filename += SAVE_GAME_EXTENSION;

            Message response;
            bool save_succeeded = HumanClientApp::GetApp()->NetworkCore().SendSynchronousMessage(HostSaveGameMessage(HumanClientApp::GetApp()->PlayerID(), filename), response);
            if (save_succeeded) {
                CloseClicked();
            } else {
                ClientUI::MessageBox("Could not save game as \"" + filename + "\".");
            }
        }
    } catch (const GG::FileDlg::InitialDirectoryDoesNotExistException& e) {
        ClientUI::MessageBox(e.Message());
    }
}

void InGameOptions::Load()
{
    if (HumanClientApp::GetApp()->LoadSinglePlayerGame())
        CloseClicked();
}

void InGameOptions::Options()
{
    OptionsWnd options_wnd;
	options_wnd.Run();
}

void InGameOptions::Exit()
{
    if (HumanClientApp::GetApp()->NetworkCore().Connected())
        HumanClientApp::GetApp()->NetworkCore().DisconnectFromServer();
    HumanClientApp::GetApp()->EndGame();
    CloseClicked();
}

void InGameOptions::Done()
{
    m_done = true;
}
