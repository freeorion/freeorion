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
    m_save_btn = new CUIButton(UserString("GAME_MENU_SAVE"));
    m_load_btn = new CUIButton(UserString("GAME_MENU_LOAD"));
    m_options_btn = new CUIButton(UserString("INTRO_BTN_OPTIONS"));
    m_exit_btn = new CUIButton(UserString("GAME_MENU_RESIGN"));
    m_done_btn = new CUIButton(UserString("DONE"));

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

    DoLayout();
}

InGameMenu::~InGameMenu()
{}

GG::X InGameMenu::MinimizedWidth() const
{ return GG::X(135); }

void InGameMenu::Render()
{ CUIWnd::Render(); }

void InGameMenu::DoLayout() {
    //size calculation consts and variables
    const GG::X MIN_BUTTON_WIDTH(160);
    const GG::Y MIN_BUTTON_HEIGHT(40);
    const GG::X H_BUTTON_MARGIN(16);    //horizontal empty space
    const GG::Y V_BUTTON_MARGIN(16);    //vertical empty space
    GG::X button_width(0);              //width of the buttons
    GG::Y button_height(0);             //height of the buttons
    const GG::X H_MAINMENU_MARGIN(40);  //horizontal empty space
    const GG::Y V_MAINMENU_MARGIN(40);  //vertical empty space
    GG::X mainmenu_width(0);            //width of the mainmenu
    GG::Y mainmenu_height(0);           //height of the mainmenu

    //calculate necessary button width
    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    button_width = std::max(button_width, font->TextExtent(m_save_btn->Text()).x);
    button_width = std::max(button_width, font->TextExtent(m_load_btn->Text()).x);
    button_width = std::max(button_width, font->TextExtent(m_options_btn->Text()).x);
    button_width = std::max(button_width, font->TextExtent(m_exit_btn->Text()).x);
    button_width = std::max(button_width, font->TextExtent(m_done_btn->Text()).x);
    button_width += H_BUTTON_MARGIN;
    button_width = std::max(MIN_BUTTON_WIDTH, button_width);

    //calculate  necessary button height
    button_height = std::max(MIN_BUTTON_HEIGHT, font->Height() + V_BUTTON_MARGIN);
    //culate window width and height
    mainmenu_width  =        button_width  + H_MAINMENU_MARGIN;
    mainmenu_height = 5.75 * button_height + V_MAINMENU_MARGIN; // 8 rows + 0.75 before exit button

    // place buttons
    GG::Pt button_ul(GG::X(15), GG::Y(12));
    GG::Pt button_lr(button_width, ClientUI::GetFont()->Lineskip() + 6);

    button_lr += button_ul;

    m_save_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_height);
    button_lr.y += GG::Y(button_height);
    m_load_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_height);
    button_lr.y += GG::Y(button_height);
    m_options_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_height);
    button_lr.y += GG::Y(button_height);
    m_exit_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_height) * 1.75;
    button_lr.y += GG::Y(button_height) * 1.75;
    m_done_btn->SizeMove(button_ul, button_lr);

    // position menu window
    GG::Pt ul(GG::GUI::GetGUI()->AppWidth()  * 0.5 - mainmenu_width/2,
              GG::GUI::GetGUI()->AppHeight() * 0.5 - mainmenu_height/2);
    GG::Pt lr(GG::GUI::GetGUI()->AppWidth()  * 0.5 + mainmenu_width/2,
              GG::GUI::GetGUI()->AppHeight() * 0.5 + mainmenu_height/2);

    this->SizeMove(ul, lr);
}

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
            if (!app->CanSaveNow()) {
                Logger().errorStream() << "InGameMenu::Save aborting; Client app can't save now";
                throw std::runtime_error(UserString("UNABLE_TO_SAVE_NOW_TRY_AGAIN"));
            }
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
