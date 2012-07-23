#include "InGameMenu.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Networking.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

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

    GG::Connect(m_save_btn->ClickedSignal,      &InGameMenu::Save,      this);
    GG::Connect(m_load_btn->ClickedSignal,      &InGameMenu::Load,      this);
    GG::Connect(m_options_btn->ClickedSignal,   &InGameMenu::Options,   this);
    GG::Connect(m_exit_btn->ClickedSignal,      &InGameMenu::Exit,      this);
    GG::Connect(m_done_btn->ClickedSignal,      &InGameMenu::Done,      this);

    if (!HumanClientApp::GetApp()->SinglePlayerGame()) {
        // only host can save multiplayer games
        if (!HumanClientApp::GetApp()->Networking().PlayerIsHost(HumanClientApp::GetApp()->PlayerID()))
            m_save_btn->Disable();
        // need lobby to load a multiplayer game; menu load of a file is insufficient
        m_load_btn->Disable();
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
    const std::string SAVE_GAME_EXTENSION =
        HumanClientApp::GetApp()->SinglePlayerGame() ?
        SP_SAVE_FILE_EXTENSION : MP_SAVE_FILE_EXTENSION;

    std::vector<std::pair<std::string, std::string> > save_file_types;
    save_file_types.push_back(std::make_pair(UserString("GAME_MENU_SAVE_FILES"), "*" + SAVE_GAME_EXTENSION));

    try {
#ifndef FREEORION_WIN32
        std::string path_String = GetSaveDir().string();
#else
        boost::filesystem::path::string_type native_path_string = GetSaveDir().native();
        std::string path_string;
        utf8::utf16to8(native_path_string.begin(), native_path_string.end(), std::back_inserter(path_string));
#endif
        FileDlg dlg(path_string, "", true, false, save_file_types);
        dlg.Run();
        if (!dlg.Result().empty()) {
            HumanClientApp::GetApp()->SaveGame(*dlg.Result().begin());
            CloseClicked();
        }
    } catch (const FileDlg::BadInitialDirectory& e) {
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
