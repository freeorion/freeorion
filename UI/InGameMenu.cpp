#include "InGameMenu.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <GG/Button.h>
#include <GG/Clr.h>
#include <GG/DrawUtil.h>

#include <boost/filesystem/operations.hpp>

#include <fstream>

namespace {
    void Options(OptionsDB& db)
    {
        db.Add<std::string>('S',
                            "save-dir", 
                            "The directory in which saved games are saved and from which they are loaded.  Directory "
                            "names are taken to be relative to the location of the executable.",
                            (GetLocalDir() / "save").native_directory_string());
    }
    bool foo_bool = RegisterOptions(&Options);

    const int IN_GAME_OPTIONS_WIDTH = 150;
    const int IN_GAME_OPTIONS_HEIGHT = 280;

    bool temp_header_bool = RecordHeaderFile(InGameMenuRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}


InGameMenu::InGameMenu():
    CUIWnd(UserString("GAME_MENU_WINDOW_TITLE"), (GG::GUI::GetGUI()->AppWidth() - IN_GAME_OPTIONS_WIDTH) / 2,
           (GG::GUI::GetGUI()->AppHeight() - IN_GAME_OPTIONS_HEIGHT) / 2, IN_GAME_OPTIONS_WIDTH, IN_GAME_OPTIONS_HEIGHT, GG::CLICKABLE | GG::MODAL)
{
    const int BUTTON_WIDTH = IN_GAME_OPTIONS_WIDTH - 60;
    const int BUTTON_X = (ClientWidth() - BUTTON_WIDTH) / 2;
    m_save_btn = new CUIButton(BUTTON_X, 22, BUTTON_WIDTH, UserString("GAME_MENU_SAVE"));
    m_load_btn = new CUIButton(BUTTON_X, 62, BUTTON_WIDTH, UserString("GAME_MENU_LOAD"));
	m_options_btn = new CUIButton(BUTTON_X, 102, BUTTON_WIDTH, UserString("INTRO_BTN_OPTIONS"));
    m_exit_btn = new CUIButton(BUTTON_X, 142, BUTTON_WIDTH, UserString("GAME_MENU_RESIGN"));
    m_done_btn = new CUIButton(BUTTON_X, 192, BUTTON_WIDTH, UserString("DONE"));

    // call to InGameMenu::MinimizedLength() because MinimizedLength is virtual
    SetMinSize(GG::Pt(InGameMenu::MinimizedLength(), MinSize().y));
    Init(); //attaches children and connects signals to slots
}

InGameMenu::~InGameMenu()
{
}

int InGameMenu::MinimizedLength() const
{ 
    return 135;
}

void InGameMenu::Render()
{
    CUIWnd::Render();
}

void InGameMenu::KeyPress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_RETURN || key == GG::GGK_ESCAPE || key == GG::GGK_F10) // Same behaviour as if "done" was pressed
        Done();
}

void InGameMenu::Init()
{
    //add children
    AttachChild(m_save_btn);
    AttachChild(m_load_btn);
	AttachChild(m_options_btn);
    AttachChild(m_exit_btn);
    AttachChild(m_done_btn);

    //attach signals
    GG::Connect(m_save_btn->ClickedSignal, &InGameMenu::Save, this);
    GG::Connect(m_load_btn->ClickedSignal, &InGameMenu::Load, this);
	GG::Connect(m_options_btn->ClickedSignal, &InGameMenu::Options, this);
    GG::Connect(m_exit_btn->ClickedSignal, &InGameMenu::Exit, this);
    GG::Connect(m_done_btn->ClickedSignal, &InGameMenu::Done, this);

    if (!HumanClientApp::GetApp()->SinglePlayerGame()) {
        m_load_btn->Disable();
    }
}

void InGameMenu::Save()
{
    const std::string SAVE_GAME_EXTENSION = HumanClientApp::GetApp()->SinglePlayerGame() ? ".sav" : ".mps";

    std::vector<std::pair<std::string, std::string> > save_file_types;
    save_file_types.push_back(std::pair<std::string, std::string>(UserString("GAME_MENU_SAVE_FILES"), "*" + SAVE_GAME_EXTENSION));

    try {
        FileDlg dlg(GetOptionsDB().Get<std::string>("save-dir"), "", true, false, save_file_types);
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
                ClientUI::MessageBox("Could not save game as \"" + filename + "\".", true);
            }
        }
    } catch (const FileDlg::BadInitialDirectory& e) {
        ClientUI::MessageBox(e.what(), true);
    }
}

void InGameMenu::Load()
{
    if (HumanClientApp::GetApp()->LoadSinglePlayerGame())
        CloseClicked();
}

void InGameMenu::Options()
{
    OptionsWnd options_wnd;
	options_wnd.Run();
}

void InGameMenu::Exit()
{
    if (HumanClientApp::GetApp()->NetworkCore().Connected())
        HumanClientApp::GetApp()->NetworkCore().DisconnectFromServer();
    HumanClientApp::GetApp()->EndGame();
    CloseClicked();
}

void InGameMenu::Done()
{
    m_done = true;
}
