#include "InGameOptions.h"

#include "ClientUI.h"
#include "CUIControls.h"
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
           "save-directory", 
           "The directory in which saved games are saved and from which they are loaded.  Directory names are taken to be relative to the location of the executable.",
           std::string("save"));
}

bool foo_bool = RegisterOptions(&Options);
}


InGameOptions::InGameOptions():
    CUI_Wnd(ClientUI::String("INGAMEOPTIONS_WINDOW_TITLE"), (GG::App::GetApp()->AppWidth() / 2) - 55,
            (GG::App::GetApp()->AppHeight() / 2) - 120, 135, 240, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL)
{
    m_save_btn = new CUIButton(30,40,75,ClientUI::String("INGAMEOPTIONS_SAVE"));
    m_load_btn = new CUIButton(30,80,75,ClientUI::String("INGAMEOPTIONS_LOAD"));
    m_quit_btn = new CUIButton(30,120,75,ClientUI::String("INGAMEOPTIONS_QUIT"));
    m_done_btn = new CUIButton(30,180,75,ClientUI::String("DONE"));

    Init(); //attaches children and connects signals to slots
}

InGameOptions::~InGameOptions()
{
}

int InGameOptions::Render()
{
    CUI_Wnd::Render();
    return 1;
}

int InGameOptions::Keypress (GG::Key key, Uint32 key_mods)
{
    if ((key == GG::GGK_RETURN) || (key == GG::GGK_ESCAPE)) {
        Done(); // Same behaviour as if "done" was pressed
    }
    return 1;
}

void InGameOptions::Init()
{
    //add children
    AttachChild(m_save_btn);
    AttachChild(m_load_btn);
    AttachChild(m_quit_btn);
    AttachChild(m_done_btn);

    //attach signals
    GG::Connect(m_save_btn->ClickedSignal(), &InGameOptions::Save, this);
    GG::Connect(m_load_btn->ClickedSignal(), &InGameOptions::Load, this);
    GG::Connect(m_quit_btn->ClickedSignal(), &InGameOptions::Quit, this);
    GG::Connect(m_done_btn->ClickedSignal(), &InGameOptions::Done, this);

    if (!HumanClientApp::GetApp()->SinglePlayerGame()) {
        if (HumanClientApp::GetApp()->PlayerID() != NetworkCore::HOST_PLAYER_ID)
            m_save_btn->Disable();
        m_load_btn->Disable();
    }
}

void InGameOptions::Save()
{
    std::vector<std::pair<std::string, std::string> > save_file_types;
    save_file_types.push_back(std::pair<std::string, std::string>(ClientUI::String("INGAMEOPTIONS_SAVE_FILES"), "*.sav"));

    GG::FileDlg dlg(GetOptionsDB().Get<std::string>("save-directory"), "", true, false, save_file_types, 
                    ClientUI::FONT, ClientUI::PTS, ClientUI::WND_COLOR, ClientUI::WND_OUTER_BORDER_COLOR, ClientUI::TEXT_COLOR);
    dlg.Run();
    std::string filename;
    if (!dlg.Result().empty()) {
        filename = *dlg.Result().begin();
        if (filename.find(".sav") != filename.size() - 4)
            filename += ".sav";

        Message response;
        bool save_succeeded = HumanClientApp::GetApp()->NetworkCore().SendSynchronousMessage(HostSaveGameMessage(HumanClientApp::GetApp()->PlayerID(), filename), response);
        if (save_succeeded) {
            CloseClicked();
        } else {
            ClientUI::MessageBox("Could not save game as \"" + filename + "\".");
        }
    }
}

void InGameOptions::Load()
{
    if (HumanClientApp::GetApp()->LoadSinglePlayerGame())
        CloseClicked();
}

void InGameOptions::Quit()
{
    GG::App::GetApp()->Exit(0);
}

void InGameOptions::Done()
{
    m_done = true;
}
