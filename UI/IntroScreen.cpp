//IntroScreen.cpp
#include "IntroScreen.h"

#include "../client/human/HumanClientApp.h"

#include "About.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "EmpireSelect.h"
#include "GalaxySetupWnd.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "GGTexture.h"
#include "../network/Message.h"
#include "MultiplayerLobbyWnd.h"
#include "../util/OptionsDB.h"
#include "ServerConnectWnd.h"

#include <cstdlib>
#include <string>

namespace {
const int SERVER_CONNECT_TIMEOUT = 30000; // in ms

void Options(OptionsDB& db)
{
    db.Add("force-external-server", 
           "Force the client not to start a server, even when hosting a game on localhost, playing single player, etc.",
           false);
}

bool foo_bool = RegisterOptions(&Options);
}


IntroScreen::IntroScreen() :
    CUI_Wnd(ClientUI::String("INTRO_WINDOW_TITLE"), (1024 + 300) / 2, 300, 200, 340, GG::Wnd::ONTOP | GG::Wnd::CLICKABLE)
{
    //create staticgraphic from the image
    m_bg_graphic = new GG::StaticGraphic(0, 0, GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight(), 
                                         GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "splash01.png"));
    GG::App::GetApp()->Register(m_bg_graphic);
    
    //create buttons
    m_single_player = new CUIButton(20, 30, 160, ClientUI::String("INTRO_BTN_SINGLE_PLAYER"));
    m_multi_player = new CUIButton(20, 70, 160, ClientUI::String("INTRO_BTN_MULTI_PLAYER"));
    m_load_game = new CUIButton(20, 110, 160, ClientUI::String("INTRO_BTN_LOAD_GAME"));
    m_options = new CUIButton(20, 150, 160, ClientUI::String("INTRO_BTN_OPTIONS"));
    m_about = new CUIButton(20, 190, 160, ClientUI::String("INTRO_BTN_ABOUT"));
    m_exit_game = new CUIButton(20, 230, 160, ClientUI::String("INTRO_BTN_EXIT"));
    
    //attach buttons
    AttachChild(m_single_player);
    AttachChild(m_multi_player);
    AttachChild(m_load_game);
    AttachChild(m_options);
    AttachChild(m_about);
    AttachChild(m_exit_game);

    //connect signals and slots
    GG::Connect(m_single_player->ClickedSignal(), &IntroScreen::OnSinglePlayer, this);
    GG::Connect(m_multi_player->ClickedSignal(), &IntroScreen::OnMultiPlayer, this);
    GG::Connect(m_load_game->ClickedSignal(), &IntroScreen::OnLoadGame, this);
    GG::Connect(m_options->ClickedSignal(), &IntroScreen::OnOptions, this);
    GG::Connect(m_about->ClickedSignal(), &IntroScreen::OnAbout, this);
    GG::Connect(m_exit_game->ClickedSignal(), &IntroScreen::OnExitGame, this);
}

IntroScreen::IntroScreen(const GG::XMLElement &elem):
    CUI_Wnd(elem.Child("CUI_Wnd"))
{
    //TODO: load from XML
}

IntroScreen::~IntroScreen()
{
    GG::App::GetApp()->Remove(m_bg_graphic);
    delete m_bg_graphic;
}

GG::XMLElement IntroScreen::XMLEncode() const
{
    GG::XMLElement retval("IntroScreen");
    //TODO: encode to XML
    return retval;
}

void IntroScreen::OnSinglePlayer()
{
    using boost::lexical_cast;
    using std::string;

    bool failed = false;
    Hide();

    if (!GetOptionsDB().Get<bool>("force-external-server"))
        HumanClientApp::GetApp()->StartServer();
    GalaxySetupWnd galaxy_wnd;    
    galaxy_wnd.Run();
    if (galaxy_wnd.EndedWithOk()) {
        //TODO: Select AIs, AI difficulty setting(s), player name, and empire
        string player_name = "Happy Player";
        int num_AIs = 4;

        GG::XMLDoc game_parameters;
        GG::XMLElement temp("host_player_name", player_name);
        game_parameters.root_node.AppendChild(temp);

        temp = GG::XMLElement("num_players");
        temp.SetAttribute("value", lexical_cast<string>(num_AIs + 1));
        game_parameters.root_node.AppendChild(temp);

        temp = GG::XMLElement("universe_params");
        temp.SetAttribute("shape", lexical_cast<string>(galaxy_wnd.GalaxyShape()));
        temp.SetAttribute("size", lexical_cast<string>(galaxy_wnd.Systems()));
        GG::XMLElement temp2("file", galaxy_wnd.GalaxyFile());
        temp.AppendChild(temp2);
        game_parameters.root_node.AppendChild(temp);

        for (int i = 0; i < num_AIs; ++i)
            game_parameters.root_node.AppendChild(GG::XMLElement("AI_client"));

        int start_time = GG::App::GetApp()->Ticks();
        while (!HumanClientApp::GetApp()->NetworkCore().ConnectToLocalhostServer()) {
            if (SERVER_CONNECT_TIMEOUT < GG::App::GetApp()->Ticks() - start_time) {
                ClientUI::MessageBox(ClientUI::String("ERR_CONNECT_TIMED_OUT"));
                failed = true;
                break;
            }
        }

        if (!failed)
            HumanClientApp::GetApp()->NetworkCore().SendMessage(HostGameMessage(HumanClientApp::GetApp()->PlayerID(), game_parameters));
    } else {
        failed = true;
    }

    if (failed) {
        HumanClientApp::GetApp()->KillServer();
        Show();
    }
}

void IntroScreen::OnMultiPlayer()
{
    bool failed = false;
    Hide();

    ServerConnectWnd server_connect_wnd;
    while (!failed && !HumanClientApp::GetApp()->NetworkCore().Connected()) {
        server_connect_wnd.Run();

        if (server_connect_wnd.Result().second == "") {
            failed = true;
        } else {
            std::string server_name = server_connect_wnd.Result().second;
            if (server_connect_wnd.Result().second == "HOST GAME SELECTED") {
                if (!GetOptionsDB().Get<bool>("force-external-server"))
                    HumanClientApp::GetApp()->StartServer();
                server_name = "localhost";
            }
            int start_time = GG::App::GetApp()->Ticks();
            while (!HumanClientApp::GetApp()->NetworkCore().ConnectToServer(server_name)) {
                if (SERVER_CONNECT_TIMEOUT < GG::App::GetApp()->Ticks() - start_time) {
                    ClientUI::MessageBox(ClientUI::String("ERR_CONNECT_TIMED_OUT"));
                    if (server_connect_wnd.Result().second == "HOST GAME SELECTED")
                        HumanClientApp::GetApp()->KillServer();
                    break;
                }
            }
        }
    }

    if (failed) {
        Show();
    } else {
        HumanClientApp::GetApp()->NetworkCore().SendMessage(server_connect_wnd.Result().second == "HOST GAME SELECTED" ? 
                                                            HostGameMessage(HumanClientApp::GetApp()->PlayerID(), server_connect_wnd.Result().first) : 
                                                            JoinGameMessage(server_connect_wnd.Result().first));
        MultiplayerLobbyWnd multiplayer_lobby_wnd(server_connect_wnd.Result().second == "HOST GAME SELECTED");
        multiplayer_lobby_wnd.Run();
        if (!multiplayer_lobby_wnd.Result()) {
            HumanClientApp::GetApp()->KillServer();
            Show();
        }
    }
}

void IntroScreen::OnLoadGame()
{  
    Hide();
    if (!HumanClientApp::GetApp()->LoadSinglePlayerGame())
        Show();
}

void IntroScreen::OnOptions()
{
}

void IntroScreen::OnAbout()
{
   About about_wnd;
   about_wnd.Run();
}

void IntroScreen::OnExitGame()
{
    //exit the application
    GG::App::GetApp()->Exit(0); //exit with 0, good error code
}

int IntroScreen::Keypress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_ESCAPE) // Same behaviour as if "done" was pressed
    {
      OnExitGame();
    }
    return 1;
}
