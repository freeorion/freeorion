#include "InGameMenu.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Networking.h"
#include "../util/i18n.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Logger.h"

#include <GG/Button.h>
#include <GG/Clr.h>
#include <GG/DrawUtil.h>
#include <GG/utf8/checked.h>

#include <boost/filesystem/operations.hpp>

namespace {
    const GG::X IN_GAME_OPTIONS_WIDTH(150);
    const GG::Y IN_GAME_OPTIONS_HEIGHT(280);
}


InGameMenu::InGameMenu():
    CUIWnd(UserString("GAME_MENU_WINDOW_TITLE"), (GG::GUI::GetGUI()->AppWidth() - IN_GAME_OPTIONS_WIDTH) / 2,
           (GG::GUI::GetGUI()->AppHeight() - IN_GAME_OPTIONS_HEIGHT) / 2, IN_GAME_OPTIONS_WIDTH, IN_GAME_OPTIONS_HEIGHT, GG::INTERACTIVE | GG::MODAL)
{
    const GG::X BUTTON_WIDTH = IN_GAME_OPTIONS_WIDTH - 60;
    const GG::X BUTTON_X = (ClientWidth() - BUTTON_WIDTH) / 2;
    m_save_btn = new CUIButton(BUTTON_X, GG::Y(22), BUTTON_WIDTH, UserString("GAME_MENU_SAVE"));
    m_load_btn = new CUIButton(BUTTON_X, GG::Y(62), BUTTON_WIDTH, UserString("GAME_MENU_LOAD"));
    m_options_btn = new CUIButton(BUTTON_X, GG::Y(102), BUTTON_WIDTH, UserString("INTRO_BTN_OPTIONS"));
    m_exit_btn = new CUIButton(BUTTON_X, GG::Y(142), BUTTON_WIDTH, UserString("GAME_MENU_RESIGN"));
    m_done_btn = new CUIButton(BUTTON_X, GG::Y(192), BUTTON_WIDTH, UserString("DONE"));

    // call to InGameMenu::MinimizedWidth() because MinimizedWidth is virtual
    SetMinSize(GG::Pt(InGameMenu::MinimizedWidth(), MinSize().y));

    AttachChild(m_save_btn);
    AttachChild(m_load_btn);
    AttachChild(m_options_btn);
    AttachChild(m_exit_btn);
    AttachChild(m_done_btn);

    GG::Connect(m_save_btn->LeftClickedSignal,      &InGameMenu::Save,      this);
    GG::Connect(m_load_btn->LeftClickedSignal,      &InGameMenu::Load,      this);
    GG::Connect(m_options_btn->LeftClickedSignal,   &InGameMenu::Options,   this);
    GG::Connect(m_exit_btn->LeftClickedSignal,      &InGameMenu::Exit,      this);
    GG::Connect(m_done_btn->LeftClickedSignal,      &InGameMenu::Done,      this);

    if (!HumanClientApp::GetApp()->SinglePlayerGame()) {
        // need lobby to load a multiplayer game; menu load of a file is insufficient
        m_load_btn->Disable();
    }

    if (!HumanClientApp::GetApp()->CanSaveNow()) {
        m_save_btn->Disable();
    }
}

InGameMenu::~InGameMenu()
{}

GG::X InGameMenu::MinimizedWidth() const
{ return GG::X(135); }

void InGameMenu::Render()
{ CUIWnd::Render(); }

void InGameMenu::KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    // Same behaviour as if "done" was pressed
    if (key == GG::GGK_RETURN || key == GG::GGK_ESCAPE || key == GG::GGK_F10)
        Done();
}

void InGameMenu::Save() {
    Logger().debugStream() << "InGameMenu::Save";

    HumanClientApp* app = HumanClientApp::GetApp();
    if (!app)
        return;
    if (!app->CanSaveNow()) {
        Logger().errorStream() << "InGameMenu::Save aborting; Client app can't save now";
        return;
    }

    const std::string SAVE_GAME_EXTENSION =
        HumanClientApp::GetApp()->SinglePlayerGame() ?
        SP_SAVE_FILE_EXTENSION : MP_SAVE_FILE_EXTENSION;

    std::vector<std::pair<std::string, std::string> > save_file_types;
    save_file_types.push_back(std::make_pair(UserString("GAME_MENU_SAVE_FILES"), "*" + SAVE_GAME_EXTENSION));

    try {
        Logger().debugStream() << "... getting save path string";
        std::string path_string = PathString(GetSaveDir());
        Logger().debugStream() << "... got save path string: " << path_string;

        Logger().debugStream() << "... running file dialog";
        FileDlg dlg(path_string, "", true, false, save_file_types);
        dlg.Run();
        if (!dlg.Result().empty()) {
            Logger().debugStream() << "... initiating save";
            app->SaveGame(*dlg.Result().begin());
            CloseClicked();
            Logger().debugStream() << "... save done";
        }
    } catch (const std::exception& e) {
        Logger().errorStream() << "Exception thrown attempting save: " << e.what();
        ClientUI::MessageBox(e.what(), true);
    }
}

void InGameMenu::Load() {
    Hide();
    HumanClientApp::GetApp()->LoadSinglePlayerGame();
    CloseClicked();
}

void InGameMenu::Options() {
    OptionsWnd options_wnd;
    options_wnd.Run();
}

void InGameMenu::Exit() {
    HumanClientApp::GetApp()->EndGame();
    CloseClicked();
}

void InGameMenu::Done()
{ m_done = true; }
