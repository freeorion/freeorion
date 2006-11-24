//IntroScreen.cpp
#include "IntroScreen.h"

#include "../client/human/HumanClientApp.h"

#include "About.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/Directories.h"
#include "GalaxySetupWnd.h"
#include "../network/Message.h"
#include "../UI/MultiplayerLobbyWnd.h"
#include "../util/MultiplayerCommon.h"
#include "OptionsWnd.h"
#include "../util/OptionsDB.h"
#include "Splash.h"
#include "../util/Serialize.h"
#include "ServerConnectWnd.h"
#include "../util/Version.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/Texture.h>

#include <boost/filesystem/fstream.hpp>

#include <cstdlib>
#include <fstream>
#include <string>

namespace {
    const int SERVER_CONNECT_TIMEOUT = 30000; // in ms
    int MAIN_MENU_WIDTH = 200;
    int MAIN_MENU_HEIGHT = 340;

    void Options(OptionsDB& db)
    {
        db.AddFlag("force-external-server",  "OPTIONS_DB_FORCE_EXTERNAL_SERVER", false);
        db.Add("UI.main-menu.x", "OPTIONS_DB_UI_MAIN_MENU_X", 0.75, RangedValidator<double>(0.0, 1.0));
        db.Add("UI.main-menu.y", "OPTIONS_DB_UI_MAIN_MENU_Y", 0.35, RangedValidator<double>(0.0, 1.0));
    }

    bool foo_bool = RegisterOptions(&Options);

}

//****************************************************************************************************
class CreditsWnd : public GG::Wnd
{
public:
    CreditsWnd(int x, int y, int w, int h,const XMLElement &credits,int cx, int cy, int cw, int ch,int co);
        
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, Uint32 keys) {m_bRender=false;}

private:
    XMLElement m_credits;
    int m_cx,m_cy,m_cw,m_ch,m_co;
    int m_start_time;
    int m_bRender,m_bFadeIn;
};

CreditsWnd::CreditsWnd(int x, int y, int w, int h,const XMLElement &credits,int cx, int cy, int cw, int ch,int co) :
    GG::Wnd(x, y, w, h,GG::CLICKABLE),m_credits(credits),m_cx(cx),m_cy(cy),m_cw(cw),m_ch(ch),m_co(co),
    m_start_time(GG::GUI::GetGUI()->Ticks()),
    m_bRender(true),
    m_bFadeIn(true)
{}

void CreditsWnd::Render()
{
    if(!m_bRender)
        return;

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    boost::shared_ptr<GG::Font> font=HumanClientApp::GetApp()->GetFont(ClientUI::Font(), static_cast<int>(ClientUI::Pts()*1.3));;
    Uint32 format = GG::TF_CENTER | GG::TF_TOP;

    GG::FlatRectangle(ul.x,ul.y,lr.x,lr.y,GG::Clr(0.0,0.0,0.0,0.5),GG::CLR_ZERO,0);
    glColor(GG::CLR_WHITE);

    int offset=m_co;

    offset -= (GG::GUI::GetGUI()->Ticks() - m_start_time)/40;

    int transparency = 255;

    if (m_bFadeIn)
    {
        double fade_in = (GG::GUI::GetGUI()->Ticks() - m_start_time)/2000.0;
        if(fade_in>1.0)
            m_bFadeIn=false;
        else
            transparency = static_cast<int>(255*fade_in);
    }

    glColor(GG::Clr(transparency,transparency,transparency,255));

    GG::BeginScissorClipping(ul.x+m_cx,ul.y+m_cy,ul.x+m_cx+m_cw,ul.y+m_cy+m_ch);

    std::string credit;
    for(int i = 0; i<m_credits.NumChildren();i++) {
        if(0==m_credits.Child(i).Tag().compare("GROUP"))
        {
            XMLElement group = m_credits.Child(i);
            for(int j = 0; j<group.NumChildren();j++) {
                if(0==group.Child(j).Tag().compare("PERSON"))
                {
                    XMLElement person = group.Child(j);
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
            }
            offset+=font->Lineskip()+2;
        }
    }
    GG::EndScissorClipping();
    if(offset<0)
    {
        m_co = 0;
        m_start_time = GG::GUI::GetGUI()->Ticks()+m_ch*40;
    }
}
//****************************************************************************************************

IntroScreen::IntroScreen() :
    CUIWnd(UserString("INTRO_WINDOW_TITLE"), 
           static_cast<int>(GG::GUI::GetGUI()->AppWidth() * GetOptionsDB().Get<double>("UI.main-menu.x") - MAIN_MENU_WIDTH / 2),
           static_cast<int>(GG::GUI::GetGUI()->AppWidth() * GetOptionsDB().Get<double>("UI.main-menu.y") - MAIN_MENU_HEIGHT / 2),
           MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, GG::ONTOP | GG::CLICKABLE),
    m_credits_wnd(0)
{
    LoadSplashGraphics(m_bg_graphics);

    //create buttons
    m_single_player = new CUIButton(15, 12, 160, UserString("INTRO_BTN_SINGLE_PLAYER"));
    m_multi_player = new CUIButton(15, 52, 160, UserString("INTRO_BTN_MULTI_PLAYER"));
    m_load_game = new CUIButton(15, 92, 160, UserString("INTRO_BTN_LOAD_GAME"));
    m_options = new CUIButton(15, 132, 160, UserString("INTRO_BTN_OPTIONS"));
    m_about = new CUIButton(15, 172, 160, UserString("INTRO_BTN_ABOUT"));
    m_credits = new CUIButton(15, 212, 160, UserString("INTRO_BTN_CREDITS"));
    m_exit_game = new CUIButton(15, 282, 160, UserString("INTRO_BTN_EXIT"));
    
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
        int start_time = GG::GUI::GetGUI()->Ticks();
        while (!HumanClientApp::GetApp()->NetworkCore().ConnectToLocalhostServer()) {
            if (SERVER_CONNECT_TIMEOUT < GG::GUI::GetGUI()->Ticks() - start_time) {
                ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                failed = true;
                break;
            } else {
                FE_PumpEvents();
            }
        }

        if (!failed) {
            ClientUI::GetClientUI()->ScreenNewGame();

            // TODO: Select number and difficulty of AIs
            SinglePlayerSetupData setup_data;
            galaxy_wnd.Panel().GetSetupData(setup_data);
            setup_data.m_host_player_name = "Happy_Player";
            setup_data.m_empire_name = galaxy_wnd.EmpireName();
            setup_data.m_empire_color = galaxy_wnd.EmpireColor();
            setup_data.m_AIs = 4;
            HumanClientApp::GetApp()->NetworkCore().SendMessage(HostSPGameMessage(HumanClientApp::GetApp()->PlayerID(), setup_data));
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
            int start_time = GG::GUI::GetGUI()->Ticks();
            while (!HumanClientApp::GetApp()->NetworkCore().ConnectToServer(server_name)) {
                if (SERVER_CONNECT_TIMEOUT < GG::GUI::GetGUI()->Ticks() - start_time) {
                    ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                    if (server_connect_wnd.Result().second == "HOST GAME SELECTED")
                        HumanClientApp::GetApp()->KillServer();
                    failed = true;
                    break;
                } else {
                    FE_PumpEvents();
                }
            }
        }
    }

    if (failed) {
        ClientUI::GetClientUI()->ScreenIntro();
    } else {
        HumanClientApp::GetApp()->NetworkCore().SendMessage(server_connect_wnd.Result().second == "HOST GAME SELECTED" ? 
                                                            HostMPGameMessage(HumanClientApp::GetApp()->PlayerID(), server_connect_wnd.Result().first) : 
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

    XMLDoc doc;
    boost::filesystem::ifstream ifs(GetSettingsDir() / "credits.xml");
    doc.ReadDoc(ifs);
    ifs.close();


    if (!doc.root_node.ContainsChild("CREDITS"))
        return;

    XMLElement credits = doc.root_node.Child("CREDITS");
    // only the area between the upper and lower line of the splash screen should be darkend
    // if we use another splash screen we have the chenge the following values
    int nUpperLine = ( 79 * GG::GUI::GetGUI()->AppHeight()) / 768,
        nLowerLine = (692 * GG::GUI::GetGUI()->AppHeight()) / 768;

    m_credits_wnd = new CreditsWnd(0,nUpperLine,
                                   GG::GUI::GetGUI()->AppWidth(),nLowerLine-nUpperLine,
                                   credits,
                                   60,0,600,(nLowerLine-nUpperLine),((nLowerLine-nUpperLine))/2);

    GG::GUI::GetGUI()->Register(m_credits_wnd);
}

void IntroScreen::OnExitGame()
{
    if (m_credits_wnd) {
        delete m_credits_wnd;
        m_credits_wnd = 0;
    }
    GG::GUI::GetGUI()->Exit(0); // exit with 0, good error code
}

void IntroScreen::KeyPress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_ESCAPE) // Same behaviour as if "done" was pressed
        OnExitGame();
}

void IntroScreen::Render()
{
    boost::shared_ptr<GG::Font> font = HumanClientApp::GetApp()->GetFont(ClientUI::Font(), ClientUI::Pts());
    CUIWnd::Render();
    GG::Pt size=font->TextExtent(FreeOrionVersionString());
    font->RenderText(GG::GUI::GetGUI()->AppWidth()-size.x,
                     GG::GUI::GetGUI()->AppHeight()-size.y,
                     FreeOrionVersionString());
}
