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
#include "MultiplayerLobbyWnd.h"
#include "OptionsWnd.h"
#include "../util/OptionsDB.h"
#include "ServerConnectWnd.h"

#include <cstdlib>
#include <string>

namespace {
const int SERVER_CONNECT_TIMEOUT = 30000; // in ms

void Options(OptionsDB& db)
{
    db.AddFlag("force-external-server", 
               "Force the client not to start a server, even when hosting a game on localhost, playing single player, etc.");
   
    db.Add("UI.main-menu.left", "Position of the intro screen main menu (left)",(1024 + 300) / 2);
    db.Add("UI.main-menu.top", "Position of the intro screen main menu (top)",300);
}

bool foo_bool = RegisterOptions(&Options);
}

//****************************************************************************************************
class CreditsWnd : public GG::Wnd
{
  public:
    CreditsWnd(int x, int y, int w, int h,const std::vector<std::string> &credits,int cx, int cy, int cw, int ch,int co);
        
    virtual bool Render();

  private:
    std::vector<std::string> m_credits;
    int m_cx,m_cy,m_cw,m_ch,m_co;
    int m_start_time;
};

CreditsWnd::CreditsWnd(int x, int y, int w, int h,
                       const std::vector<std::string> &credits,
                       int cx, int cy, int cw, int ch,int co)
: GG::Wnd(x, y, w, h,0),m_credits(credits),m_cx(cx),m_cy(cy),m_cw(cw),m_ch(ch),m_co(co),
  m_start_time(GG::App::GetApp()->Ticks())
{}

bool CreditsWnd::Render()
{
  GG::Pt ul = UpperLeft(), lr = LowerRight();
  boost::shared_ptr<GG::Font> font=HumanClientApp::GetApp()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::SIDE_PANEL_PTS*1.3));;
  Uint32 format = GG::TF_CENTER | GG::TF_TOP;

  GG::FlatRectangle(ul.x,ul.y,lr.x,lr.y,GG::Clr(0.0,0.0,0.0,0.5),GG::CLR_ZERO,0);
  glColor4ubv(GG::CLR_WHITE.v);

  int offset=m_co;

  offset -= (GG::App::GetApp()->Ticks() - m_start_time)/40;

  GG::BeginScissorClipping(ul.x+m_cx,ul.y+m_cy,ul.x+m_cx+m_cw,ul.y+ul.y+m_ch);
  for(unsigned int i =0;i<m_credits.size();i++)
  {
    font->RenderText(ul.x+m_cx,ul.y+m_cy+offset,ul.x+m_cx+m_cw,ul.y+ul.y+m_ch, m_credits[i], format, 0, true);
    offset+=font->TextExtent(m_credits[i], format).y+2;
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
    CUI_Wnd(ClientUI::String("INTRO_WINDOW_TITLE"), 
            GetOptionsDB().Get<int>("UI.main-menu.left"), GetOptionsDB().Get<int>("UI.main-menu.top"), 
            200, 340, GG::Wnd::ONTOP | GG::Wnd::CLICKABLE),
    m_credits_wnd(NULL)
{
    //create staticgraphic from the image
    m_bg_graphic = new GG::StaticGraphic(0, 0, GG::App::GetApp()->AppWidth(), GG::App::GetApp()->AppHeight(), 
                                         GG::App::GetApp()->GetTexture(ClientUI::ART_DIR + "splash01.png"), 
                                         GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
    GG::App::GetApp()->Register(m_bg_graphic);
    
    //create buttons
    m_single_player = new CUIButton(20, 30, 160, ClientUI::String("INTRO_BTN_SINGLE_PLAYER"));
    m_multi_player = new CUIButton(20, 70, 160, ClientUI::String("INTRO_BTN_MULTI_PLAYER"));
    m_load_game = new CUIButton(20, 110, 160, ClientUI::String("INTRO_BTN_LOAD_GAME"));
    m_options = new CUIButton(20, 150, 160, ClientUI::String("INTRO_BTN_OPTIONS"));
    m_about = new CUIButton(20, 190, 160, ClientUI::String("INTRO_BTN_ABOUT"));
    m_credits = new CUIButton(20, 230, 160, ClientUI::String("INTRO_BTN_CREDITS"));
    m_exit_game = new CUIButton(20, 300, 160, ClientUI::String("INTRO_BTN_EXIT"));
    
    //attach buttons
    AttachChild(m_single_player);
    AttachChild(m_multi_player);
    AttachChild(m_load_game);
    AttachChild(m_options);
    AttachChild(m_about);
    AttachChild(m_credits);
    AttachChild(m_exit_game);

    //connect signals and slots
    GG::Connect(m_single_player->ClickedSignal(), &IntroScreen::OnSinglePlayer, this);
    GG::Connect(m_multi_player->ClickedSignal(), &IntroScreen::OnMultiPlayer, this);
    GG::Connect(m_load_game->ClickedSignal(), &IntroScreen::OnLoadGame, this);
    GG::Connect(m_options->ClickedSignal(), &IntroScreen::OnOptions, this);
    GG::Connect(m_about->ClickedSignal(), &IntroScreen::OnAbout, this);
    GG::Connect(m_credits->ClickedSignal(), &IntroScreen::OnCredits, this);
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

  if(m_credits_wnd){
    GG::App::GetApp()->Register(m_credits_wnd);
    delete m_credits_wnd;
  }
}

GG::XMLElement IntroScreen::XMLEncode() const
{
    GG::XMLElement retval("IntroScreen");
    //TODO: encode to XML
    return retval;
}

void IntroScreen::OnSinglePlayer()
{
    if(m_credits_wnd){
      GG::App::GetApp()->Register(m_credits_wnd);
      delete m_credits_wnd;m_credits_wnd=0;
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
        // TODO: Select AIs, AI difficulty setting(s)
        string player_name = "Happy_Player";
        int num_AIs = 4;

        GG::XMLDoc game_parameters;
        game_parameters.root_node.AppendChild(GG::XMLElement("host_player_name", player_name));
        game_parameters.root_node.AppendChild(GG::XMLElement("num_players", lexical_cast<string>(num_AIs + 1)));
        game_parameters.root_node.AppendChild(galaxy_wnd.Panel().XMLEncode());
        game_parameters.root_node.AppendChild(GG::XMLElement("empire_name", galaxy_wnd.EmpireName()));
        game_parameters.root_node.AppendChild(GG::XMLElement("empire_color", galaxy_wnd.EmpireColor().XMLEncode()));

        for (int i = 0; i < num_AIs; ++i) {
            game_parameters.root_node.AppendChild(GG::XMLElement("AI_client"));
        }

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
    if(m_credits_wnd){
      GG::App::GetApp()->Register(m_credits_wnd);
      delete m_credits_wnd;m_credits_wnd=0;
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
    if(m_credits_wnd){
      GG::App::GetApp()->Register(m_credits_wnd);
      delete m_credits_wnd;m_credits_wnd=0;
    }
    Hide();
    if (!HumanClientApp::GetApp()->LoadSinglePlayerGame())
        Show();
}

void IntroScreen::OnOptions()
{
    if(m_credits_wnd){
      GG::App::GetApp()->Register(m_credits_wnd);
      delete m_credits_wnd;m_credits_wnd=0;
    }
    OptionsWnd options_wnd;
	options_wnd.Run();
}

void IntroScreen::OnAbout()
{
    if(m_credits_wnd){
      GG::App::GetApp()->Register(m_credits_wnd);
      delete m_credits_wnd;m_credits_wnd=0;
    }
    About about_wnd;
    about_wnd.Run();
}

void IntroScreen::OnCredits()
{
  if(m_credits_wnd){
    GG::App::GetApp()->Register(m_credits_wnd);
    delete m_credits_wnd;m_credits_wnd=0;
  }

  GG::XMLDoc doc;
  std::ifstream ifs((ClientUI::DIR + "credits.xml").c_str());
  doc.ReadDoc(ifs);
  ifs.close();


  if (!doc.root_node.ContainsChild("CREDITS"))
    return;

  GG::XMLElement credits = doc.root_node.Child("CREDITS");
  std::vector<std::string> credits_vec;
  for(int i = 0; i<credits.NumChildren();i++)
    if(0==credits.Child(i).Tag().compare("GROUP"))
    {
      if(credits_vec.size()>0)
        credits_vec.push_back("");

      GG::XMLElement group = credits.Child(i);
      for(int j = 0; j<group.NumChildren();j++)
        if(0==group.Child(j).Tag().compare("PERSON"))
        {
          GG::XMLElement person = group.Child(j);
          std::string entry;
          entry = "";
          if(person.ContainsAttribute("name"))
            entry+=person.Attribute("name");
          if(person.ContainsAttribute("nick") && person.Attribute("nick").length()>0)
          {
            entry+=" <rgba 153 153 153 255>(";
            entry+=person.Attribute("nick");
            entry+=")</rgba>";
          }
          if(person.ContainsAttribute("task") && person.Attribute("task").length()>0)
          {
            entry+=" - <rgba 204 204 204 255>";
            entry+=person.Attribute("task");
            entry+="</rgba>";
          }
          credits_vec.push_back(entry);
        }
  }

  // only the area between the upper and lower line of the splash screen should be darkend
  // if we use another splash screen we have the chenge the following values
  int nUpperLine = ( 79 * GG::App::GetApp()->AppHeight()) / 768,
      nLowerLine = (692 * GG::App::GetApp()->AppHeight()) / 768;

  m_credits_wnd = new CreditsWnd(0,nUpperLine,
                                 GG::App::GetApp()->AppWidth(),nLowerLine-nUpperLine,
                                 credits_vec,
                                 60,50,600,(nLowerLine-nUpperLine)-60*2,((nLowerLine-nUpperLine)-60*2)/2);

  GG::App::GetApp()->Register(m_credits_wnd);
}

void IntroScreen::OnExitGame()
{
    if(m_credits_wnd){
      GG::App::GetApp()->Register(m_credits_wnd);
      delete m_credits_wnd;m_credits_wnd=0;
    }
    //exit the application
    GG::App::GetApp()->Exit(0); //exit with 0, good error code
}

void IntroScreen::Keypress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_ESCAPE) // Same behaviour as if "done" was pressed
        OnExitGame();
}
