//IntroScreen.cpp
#include "IntroScreen.h"

#include "../client/human/HumanClientApp.h"

#include "About.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "GalaxySetupWnd.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "GGTexture.h"
#include "../network/Message.h"
#include "../UI/MultiplayerLobbyWnd.h"
#include "../util/MultiplayerCommon.h"
#include "OptionsWnd.h"
#include "../util/OptionsDB.h"
#include "Splash.h"
#include "ServerConnectWnd.h"

#include <cstdlib>
#include <fstream>
#include <string>

namespace {
    const int SERVER_CONNECT_TIMEOUT = 30000; // in ms
    int MAIN_MENU_WIDTH = 200;
    int MAIN_MENU_HEIGHT = 340;

    void Options(OptionsDB& db)
    {
        db.AddFlag("force-external-server", 
                   "Force the client not to start a server, even when hosting a game on localhost, playing single player, etc.");
   
        db.Add("UI.main-menu.x", "Position of the center of the intro screen main menu, as a portion of the application's total width.", 0.75, RangedValidator<double>(0.0, 1.0));
        db.Add("UI.main-menu.y", "Position of the center of the intro screen main menu, as a portion of the application's total height.", 0.35, RangedValidator<double>(0.0, 1.0));
    }

    bool foo_bool = RegisterOptions(&Options);

    bool temp_header_bool = RecordHeaderFile(IntroScreenRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

//****************************************************************************************************
class CreditsWnd : public GG::Wnd
{
public:
    CreditsWnd(int x, int y, int w, int h,const GG::XMLElement &credits,int cx, int cy, int cw, int ch,int co);
        
    virtual bool Render();
    virtual void LClick(const GG::Pt& pt, Uint32 keys) {m_bRender=false;}

private:
    GG::XMLElement m_credits;
    int m_cx,m_cy,m_cw,m_ch,m_co;
    int m_start_time;
    int m_bRender,m_bFadeIn;
};

CreditsWnd::CreditsWnd(int x, int y, int w, int h,const GG::XMLElement &credits,int cx, int cy, int cw, int ch,int co) :
    GG::Wnd(x, y, w, h,GG::Wnd::CLICKABLE),m_credits(credits),m_cx(cx),m_cy(cy),m_cw(cw),m_ch(ch),m_co(co),
    m_start_time(GG::App::GetApp()->Ticks()),
    m_bRender(true),
    m_bFadeIn(true)
{}

bool CreditsWnd::Render()
{
    if(!m_bRender)
        return true;

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    boost::shared_ptr<GG::Font> font=HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.3));;
    Uint32 format = GG::TF_CENTER | GG::TF_TOP;

    GG::FlatRectangle(ul.x,ul.y,lr.x,lr.y,GG::Clr(0.0,0.0,0.0,0.5),GG::CLR_ZERO,0);
    glColor4ubv(GG::CLR_WHITE.v);

    int offset=m_co;

    offset -= (GG::App::GetApp()->Ticks() - m_start_time)/40;

    int transparency = 255;

    if (m_bFadeIn)
    {
        double fade_in = (GG::App::GetApp()->Ticks() - m_start_time)/2000.0;
        if(fade_in>1.0)
            m_bFadeIn=false;
        else
            transparency = static_cast<int>(255*fade_in);
    }

    glColor4ubv(GG::Clr(transparency,transparency,transparency,255).v);

    GG::BeginScissorClipping(ul.x+m_cx,ul.y+m_cy,ul.x+m_cx+m_cw,ul.y+m_cy+m_ch);

    std::string credit;
    for(int i = 0; i<m_credits.NumChildren();i++)
        if(0==m_credits.Child(i).Tag().compare("GROUP"))
        {
            GG::XMLElement group = m_credits.Child(i);
            for(int j = 0; j<group.NumChildren();j++)
                if(0==group.Child(j).Tag().compare("PERSON"))
                {
                    GG::XMLElement person = group.Child(j);
                    credit = "";
                    if(person.ContainsAttribute("name"))
                        credit+=person.Attribute("name");
                    if(person.ContainsAttribute("nick") && person.Attribute("nick").length()>0)
                    {
                        credit+=" <rgba 153 153 153 " + boost::lexical_cast<std::string>(transparency) +">(";
                        credit+=person.Attribute("nick");
                        credit+=")</rgba>";
                    }
                    if(person.ContainsAttribute("task") && person.Attribute("task").length()>0)
                    {
                        credit+=" - <rgba 204 204 204 " + boost::lexical_cast<std::string>(transparency) +">";
                        credit+=person.Attribute("task");
                        credit+="</rgba>";
                    }
                    font->RenderText(ul.x+m_cx,ul.y+m_cy+offset,ul.x+m_cx+m_cw,ul.y+m_cy+m_ch,credit, format, 0);
                    offset+=font->TextExtent(credit, format).y+2;
                }
            font->RenderText(ul.x+m_cx,ul.y+m_cy+offset,ul.x+m_cx+m_cw,ul.y+m_cy+m_ch,"", format, 0);
            offset+=font->TextExtent("", format).y+2;
        }
    GG::EndScissorClipping();

    if(offset<0)
    {
        m_co = 0;
        m_start_time = GG::App::GetApp()->Ticks()+m_ch*40;
    }


    return true;
}
//****************************************************************************************************

IntroScreen::IntroScreen() :
    CUI_Wnd(UserString("INTRO_WINDOW_TITLE"), 
            static_cast<int>(GG::App::GetApp()->AppWidth() * GetOptionsDB().Get<double>("UI.main-menu.x") - MAIN_MENU_WIDTH / 2),
            static_cast<int>(GG::App::GetApp()->AppWidth() * GetOptionsDB().Get<double>("UI.main-menu.y") - MAIN_MENU_HEIGHT / 2),
            MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, GG::Wnd::ONTOP | GG::Wnd::CLICKABLE),
    m_credits_wnd(0)
{
    LoadSplashGraphics(m_bg_graphics);
    
    //create buttons
    m_single_player = new CUIButton(20, 30, 160, UserString("INTRO_BTN_SINGLE_PLAYER"));
    m_multi_player = new CUIButton(20, 70, 160, UserString("INTRO_BTN_MULTI_PLAYER"));
    m_load_game = new CUIButton(20, 110, 160, UserString("INTRO_BTN_LOAD_GAME"));
    m_options = new CUIButton(20, 150, 160, UserString("INTRO_BTN_OPTIONS"));
    m_about = new CUIButton(20, 190, 160, UserString("INTRO_BTN_ABOUT"));
    m_credits = new CUIButton(20, 230, 160, UserString("INTRO_BTN_CREDITS"));
    m_exit_game = new CUIButton(20, 300, 160, UserString("INTRO_BTN_EXIT"));
    
    //attach buttons
    AttachChild(m_single_player);
    AttachChild(m_multi_player);
    AttachChild(m_load_game);
    AttachChild(m_options);
    AttachChild(m_about);
    AttachChild(m_credits);
    AttachChild(m_exit_game);

    //connect signals and slots
    GG::Connect(m_single_player->ClickedSignal, &IntroScreen::OnSinglePlayer, this);
    GG::Connect(m_multi_player->ClickedSignal, &IntroScreen::OnMultiPlayer, this);
    GG::Connect(m_load_game->ClickedSignal, &IntroScreen::OnLoadGame, this);
    GG::Connect(m_options->ClickedSignal, &IntroScreen::OnOptions, this);
    GG::Connect(m_about->ClickedSignal, &IntroScreen::OnAbout, this);
    GG::Connect(m_credits->ClickedSignal, &IntroScreen::OnCredits, this);
    GG::Connect(m_exit_game->ClickedSignal, &IntroScreen::OnExitGame, this);
}

IntroScreen::~IntroScreen()
{
    for (unsigned int y = 0; y < m_bg_graphics.size(); ++y) {
        for (unsigned int x = 0; x < m_bg_graphics[y].size(); ++x) {
            delete m_bg_graphics[y][x];
        }
    }
    delete m_credits_wnd;
}

void IntroScreen::OnSinglePlayer()
{
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }
    using boost::lexical_cast;
    using std::string;

    bool failed = false;
    Hide();

    if (!GetOptionsDB().Get<bool>("force-external-server"))
        HumanClientApp::GetApp()->StartServer();
    GalaxySetupWnd galaxy_wnd;    
    galaxy_wnd.Run();
    if (galaxy_wnd.EndedWithOk()) {
        // TODO: Select number and difficulty of AIs
        string player_name = "Happy_Player";
        int num_AIs = 4;

        GG::XMLDoc game_parameters;
        game_parameters.root_node.AppendChild(GG::XMLElement("host_player_name", player_name));
        game_parameters.root_node.AppendChild(GG::XMLElement("num_players", lexical_cast<string>(num_AIs + 1)));
        game_parameters.root_node.AppendChild(galaxy_wnd.Panel().XMLEncode());
        game_parameters.root_node.AppendChild(GG::XMLElement("empire_name", galaxy_wnd.EmpireName()));
        game_parameters.root_node.AppendChild(GG::XMLElement("empire_color", ClrToXML(galaxy_wnd.EmpireColor())));

        for (int i = 0; i < num_AIs; ++i) {
            game_parameters.root_node.AppendChild(GG::XMLElement("AI_client"));
        }

        int start_time = GG::App::GetApp()->Ticks();
        while (!HumanClientApp::GetApp()->NetworkCore().ConnectToLocalhostServer()) {
            if (SERVER_CONNECT_TIMEOUT < GG::App::GetApp()->Ticks() - start_time) {
                ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                failed = true;
                break;
            }
        }

        if (!failed) {
            ClientUI::GetClientUI()->ScreenNewGame();
            HumanClientApp::GetApp()->NetworkCore().SendMessage(HostGameMessage(HumanClientApp::GetApp()->PlayerID(), game_parameters));
        }
    } else {
        failed = true;
    }

    if (failed) {
        HumanClientApp::GetApp()->KillServer();
        ClientUI::GetClientUI()->ScreenIntro();
    }
}

void IntroScreen::OnMultiPlayer()
{
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }
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
                if (!GetOptionsDB().Get<bool>("force-external-server")) {
                    HumanClientApp::GetApp()->StartServer();
                    HumanClientApp::GetApp()->FreeServer();
                }
                server_name = "localhost";
            }
            int start_time = GG::App::GetApp()->Ticks();
            while (!HumanClientApp::GetApp()->NetworkCore().ConnectToServer(server_name)) {
                if (SERVER_CONNECT_TIMEOUT < GG::App::GetApp()->Ticks() - start_time) {
                    ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                    if (server_connect_wnd.Result().second == "HOST GAME SELECTED")
                        HumanClientApp::GetApp()->KillServer();
                    failed = true;
                    break;
                }
            }
        }
    }

    if (failed) {
        ClientUI::GetClientUI()->ScreenIntro();
    } else {
        HumanClientApp::GetApp()->NetworkCore().SendMessage(server_connect_wnd.Result().second == "HOST GAME SELECTED" ? 
                                                            HostGameMessage(HumanClientApp::GetApp()->PlayerID(), server_connect_wnd.Result().first) : 
                                                            JoinGameMessage(server_connect_wnd.Result().first));
        MultiplayerLobbyWnd multiplayer_lobby_wnd(server_connect_wnd.Result().second == "HOST GAME SELECTED");
        multiplayer_lobby_wnd.Run();
        if (!multiplayer_lobby_wnd.Result()) {
            HumanClientApp::GetApp()->KillServer();
            ClientUI::GetClientUI()->ScreenIntro();
        } else {
            if (multiplayer_lobby_wnd.LoadSelected())
                ClientUI::GetClientUI()->ScreenLoad();
            else
                ClientUI::GetClientUI()->ScreenNewGame();
        }
    }
}

void IntroScreen::OnLoadGame()
{  
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }
    Hide();
    if (!HumanClientApp::GetApp()->LoadSinglePlayerGame())
        Show();
}

void IntroScreen::OnOptions()
{
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }
    OptionsWnd options_wnd;
	options_wnd.Run();
}

void IntroScreen::OnAbout()
{
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }
    About about_wnd;
    about_wnd.Run();
}

void IntroScreen::OnCredits()
{
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }

    GG::XMLDoc doc;
    std::ifstream ifs((ClientUI::DIR + "credits.xml").c_str());
    doc.ReadDoc(ifs);
    ifs.close();


    if (!doc.root_node.ContainsChild("CREDITS"))
        return;

    GG::XMLElement credits = doc.root_node.Child("CREDITS");
    // only the area between the upper and lower line of the splash screen should be darkend
    // if we use another splash screen we have the chenge the following values
    int nUpperLine = ( 79 * GG::App::GetApp()->AppHeight()) / 768,
        nLowerLine = (692 * GG::App::GetApp()->AppHeight()) / 768;

    m_credits_wnd = new CreditsWnd(0,nUpperLine,
                                   GG::App::GetApp()->AppWidth(),nLowerLine-nUpperLine,
                                   credits,
                                   60,0,600,(nLowerLine-nUpperLine),((nLowerLine-nUpperLine))/2);

    GG::App::GetApp()->Register(m_credits_wnd);
}

void IntroScreen::OnExitGame()
{
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }
    GG::App::GetApp()->Exit(0); // exit with 0, good error code
}

void IntroScreen::Keypress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_ESCAPE) // Same behaviour as if "done" was pressed
        OnExitGame();
}
