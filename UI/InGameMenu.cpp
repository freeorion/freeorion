#include "InGameMenu.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "SaveFileDialog.h"
#include "TextBrowseWnd.h"
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

#include <boost/filesystem/operations.hpp>

namespace {
    const GG::X IN_GAME_OPTIONS_WIDTH(150);
    const GG::Y IN_GAME_OPTIONS_HEIGHT(280);
}

InGameMenu::InGameMenu():
    CUIWnd(UserString("GAME_MENU_WINDOW_TITLE"),
           GG::INTERACTIVE | GG::MODAL)
{}

void InGameMenu::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    m_save_btn = Wnd::Create<CUIButton>(UserString("GAME_MENU_SAVE"));
    m_load_btn = Wnd::Create<CUIButton>(UserString("GAME_MENU_LOAD"));
    m_options_btn = Wnd::Create<CUIButton>(UserString("INTRO_BTN_OPTIONS"));
    m_exit_btn = Wnd::Create<CUIButton>(UserString("GAME_MENU_RESIGN"));
    m_done_btn = Wnd::Create<CUIButton>(UserString("DONE"));

    AttachChild(m_save_btn);
    AttachChild(m_load_btn);
    AttachChild(m_options_btn);
    AttachChild(m_exit_btn);
    AttachChild(m_done_btn);

    m_save_btn->LeftClickedSignal.connect(
        boost::bind(&InGameMenu::Save, this));
    m_load_btn->LeftClickedSignal.connect(
        boost::bind(&InGameMenu::Load, this));
    m_options_btn->LeftClickedSignal.connect(
        boost::bind(&InGameMenu::Options, this));
    m_exit_btn->LeftClickedSignal.connect(
        boost::bind(&InGameMenu::Exit, this));
    m_done_btn->LeftClickedSignal.connect(
        boost::bind(&InGameMenu::Done, this));

    if (!HumanClientApp::GetApp()->SinglePlayerGame()) {
        // need lobby to load a multiplayer game; menu load of a file is insufficient
        m_load_btn->Disable();
    }

    if (!HumanClientApp::GetApp()->CanSaveNow()) {
        m_save_btn->Disable();
        m_save_btn->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        m_save_btn->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("BUTTON_DISABLED"),
            UserString("SAVE_DISABLED_BROWSE_TEXT"),
            GG::X(400)
        ));
    }

    ResetDefaultPosition();
    DoLayout();
}

InGameMenu::~InGameMenu()
{}

GG::Rect InGameMenu::CalculatePosition() const {
    const GG::X H_MAINMENU_MARGIN(40);  //horizontal empty space
    const GG::Y V_MAINMENU_MARGIN(40);  //vertical empty space

    // Calculate window width and height
    GG::Pt new_size(ButtonWidth() + H_MAINMENU_MARGIN,
                    5.75 * ButtonCellHeight() + V_MAINMENU_MARGIN); // 8 rows + 0.75 before exit button

    // This wnd determines its own position.
    GG::Pt new_ul((HumanClientApp::GetApp()->AppWidth()  - new_size.x) / 2,
                  (HumanClientApp::GetApp()->AppHeight() - new_size.y) / 2);

    return GG::Rect(new_ul, new_ul + new_size);
}

GG::X InGameMenu::ButtonWidth() const {
    const GG::X MIN_BUTTON_WIDTH(160);
    GG::X button_width(0);              //width of the buttons

    button_width = std::max(button_width, m_save_btn->MinUsableSize().x);
    button_width = std::max(button_width, m_load_btn->MinUsableSize().x);
    button_width = std::max(button_width, m_options_btn->MinUsableSize().x);
    button_width = std::max(button_width, m_exit_btn->MinUsableSize().x);
    button_width = std::max(button_width, m_done_btn->MinUsableSize().x);
    button_width = std::max(MIN_BUTTON_WIDTH, button_width);

    return button_width;
}

GG::Y InGameMenu::ButtonCellHeight() const {
    const GG::Y MIN_BUTTON_HEIGHT(40);
    return std::max(MIN_BUTTON_HEIGHT, m_done_btn->MinUsableSize().y);
}

void InGameMenu::DoLayout() {
    // place buttons
    GG::Pt button_ul(GG::X(15), GG::Y(12));
    GG::Pt button_lr(ButtonWidth(), m_done_btn->MinUsableSize().y);

    button_lr += button_ul;

    GG::Y button_cell_height = ButtonCellHeight();

    m_save_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_load_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_options_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height);
    button_lr.y += GG::Y(button_cell_height);
    m_exit_btn->SizeMove(button_ul, button_lr);
    button_ul.y += GG::Y(button_cell_height) * 1.75;
    button_lr.y += GG::Y(button_cell_height) * 1.75;
    m_done_btn->SizeMove(button_ul, button_lr);
}

void InGameMenu::KeyPress(GG::Key key, std::uint32_t key_code_point,
                          GG::Flags<GG::ModKey> mod_keys)
{
    // Same behaviour as if "done" was pressed
    if (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER || key == GG::GGK_ESCAPE )
        Done();
}

void InGameMenu::Save() {
    DebugLogger() << "InGameMenu::Save";

    HumanClientApp* app = HumanClientApp::GetApp();
    if (!app)
        return;
    if (!app->CanSaveNow()) {
        ErrorLogger() << "InGameMenu::Save aborting; Client app can't save now";
        return;
    }

    try {
        // When saving in multiplayer, you cannot see the old saves or
        // browse directories, only give a save file name.
        DebugLogger() << "... running save file dialog";
        auto filename = ClientUI::GetClientUI()->GetFilenameWithSaveFileDialog(
            SaveFileDialog::Purpose::Save,
            app->SinglePlayerGame() ? SaveFileDialog::SaveType::SinglePlayer : SaveFileDialog::SaveType::MultiPlayer);

        if (!filename.empty()) {
            if (!app->CanSaveNow()) {
                ErrorLogger() << "InGameMenu::Save aborting; Client app can't save now";
                throw std::runtime_error(UserString("UNABLE_TO_SAVE_NOW_TRY_AGAIN"));
            }
            DebugLogger() << "... initiating save to " << filename ;
            app->SaveGame(filename);
            CloseClicked();
        }

    } catch (const std::exception& e) {
        ErrorLogger() << "Exception thrown attempting save: " << e.what();
        ClientUI::MessageBox(e.what(), true);
    }
}

void InGameMenu::Load() {
    Hide();
    HumanClientApp::GetApp()->LoadSinglePlayerGame();
    CloseClicked();
}

void InGameMenu::Options() {
    auto options_wnd = GG::Wnd::Create<OptionsWnd>(true);
    options_wnd->Run();
}

void InGameMenu::Exit() {
    HumanClientApp::GetApp()->ResetToIntro(false);
    CloseClicked();
}

void InGameMenu::Done()
{ m_done = true; }
