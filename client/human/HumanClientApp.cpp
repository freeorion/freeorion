#include "HumanClientApp.h"

#include "../GG/XML/XMLDoc.h"

#include <boost/lexical_cast.hpp>


////////////////////////////////////////////////////////////////////////////////////////////////////
// ONLY TEMPORARY!!!!!
#include "../GG/GGWnd.h"
#include "../GG/GGButton.h"
#include "../GG/GGDrawUtil.h"
#include "../network/Message.h"
#include "../util/Process.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
class NetTestWnd : public GG::Wnd
{
public:
   NetTestWnd() : 
      GG::Wnd(100, 100, 300, 600)
   {
      m_start_server =     new GG::Button(20,  20, 260, 30, "Start server process", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      m_local_connect_bn = new GG::Button(20,  70, 260, 30, "Connect to localhost", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      m_start_game =       new GG::Button(20, 120, 260, 30, "Start a game (3 AIs, 2 humans)", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      m_send_doc_bn =      new GG::Button(20, 220, 260, 30, "Send an XMLDoc", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      m_send_text_bn =     new GG::Button(20, 270, 260, 30, "Send plain text", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      
      AttachChild(m_start_server);
      AttachChild(m_local_connect_bn);
      AttachChild(m_start_game);
      AttachChild(m_send_doc_bn);
      AttachChild(m_send_text_bn);
      
      GG::Connect(m_start_server->ClickedSignal(), &NetTestWnd::StartServerClicked, this);
      GG::Connect(m_local_connect_bn->ClickedSignal(), &NetTestWnd::LocalConnectClicked, this);
      GG::Connect(m_start_game->ClickedSignal(), &NetTestWnd::StartGameClicked, this);
      GG::Connect(m_send_doc_bn->ClickedSignal(), &NetTestWnd::SendDocClicked, this);
      GG::Connect(m_send_text_bn->ClickedSignal(), &NetTestWnd::SendTextClicked, this);
   }
   
   virtual int Render()
   {
      HumanClientApp::GetApp()->Enter2DMode();
      GG::FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, GG::Clr(1.0, 1.0, 1.0, 0.75), GG::CLR_WHITE, 1);
      HumanClientApp::GetApp()->Exit2DMode();
   }
   
private:
   void StartServerClicked()
   {
      if (m_start_server->WindowText() == "Start server process") {
         std::string cmd = "freeoriond.exe";
         std::vector<std::string> cmd_line;
         cmd_line.push_back(cmd);
         // add command line params to server here
         m_server_process = Process(cmd, cmd_line);
         m_start_server->SetText("Stop server process");
      } else {
         m_server_process.Kill();
         m_start_server->SetText("Start server process");
      }

   }

   void LocalConnectClicked()
   {
      if (m_local_connect_bn->WindowText() == "Connect to localhost") {
         bool successful = HumanClientApp::GetApp()->NetworkCore().ConnectToLocalhostServer();
         if (successful) {
            m_local_connect_bn->SetText("Disconnect from localhost");
         }
      } else {
         HumanClientApp::GetApp()->NetworkCore().DisconnectFromServer();
         m_local_connect_bn->SetText("Connect to localhost");
      }
   }

   void StartGameClicked()
   {
      // start server
      const std::string SERVER_EXE = "freeoriond.exe";
      std::vector<std::string> args;
      args.push_back(SERVER_EXE);
      m_server_process = Process(SERVER_EXE, args);
      
      // connect to server
      while (!HumanClientApp::GetApp()->NetworkCore().ConnectToLocalhostServer()) ;

      // start a game on the server
      GG::XMLDoc game_parameters;
      // use (host) human player named "Zach"
      GG::XMLElement elem("host_player_name");
      elem.SetText("Zach");
      game_parameters.root_node.AppendChild(elem);
      // 5 players total
      elem = GG::XMLElement("num_players");
      elem.SetAttribute("value", "5");
      game_parameters.root_node.AppendChild(elem);
      // 3 AI clients
      elem = GG::XMLElement("AI_client");
      game_parameters.root_node.AppendChild(elem);
      elem = GG::XMLElement("AI_client");
      game_parameters.root_node.AppendChild(elem);
      elem = GG::XMLElement("AI_client");
      game_parameters.root_node.AppendChild(elem);
      // one slot for another human player
      
      HumanClientApp::GetApp()->NetworkCore().SendMessage(HostGameMessage(game_parameters));
   }

   void SendDocClicked()
   {
      GG::XMLDoc doc;
      GG::XMLElement elem("test_elem");
      elem.SetAttribute("value", "foo");
      elem.AppendChild(GG::XMLElement("bar"));
      doc.root_node.AppendChild(elem);
      
      Message msg(Message::SERVER_STATUS, 0, 1, Message::CORE, doc);
      HumanClientApp::GetApp()->NetworkCore().SendMessage(msg);
   }

   void SendTextClicked()
   {
      std::string str = "This is a test.  This is only a test.  Were this an actual emergency, something more official would "
      "have happened.  Like the president would have gone on the TV from a \"secure location\" to assure you everything is "
      "okay, to tell you to stop looting and go back inside your home.  This has been only a test.";
      
      Message msg(Message::SERVER_STATUS, 0, 1, Message::CORE, str);
      HumanClientApp::GetApp()->NetworkCore().SendMessage(msg);
   }

   GG::Button* m_start_server;
   GG::Button* m_local_connect_bn;
   GG::Button* m_start_game;
   GG::Button* m_send_doc_bn;
   GG::Button* m_send_text_bn;
   
   Process m_server_process;
};
boost::shared_ptr<NetTestWnd> g_net_test_wnd;
// ONLY TEMPORARY!!!!!
////////////////////////////////////////////////////////////////////////////////////////////////////



HumanClientApp::HumanClientApp(const GG::XMLElement& elem) : 
   ClientApp(), 
   SDLGGApp::SDLGGApp(boost::lexical_cast<int>(elem.Child("width").Attribute("value")), 
                      boost::lexical_cast<int>(elem.Child("height").Attribute("value")), 
                      boost::lexical_cast<bool>(elem.Child("calc_FPS").Attribute("value")),
                      elem.Child("app_name").Text())
{
}

HumanClientApp::~HumanClientApp()
{
}

HumanClientApp* HumanClientApp::GetApp()
{
   return dynamic_cast<HumanClientApp*>(GG::App::GetApp());
}
   
void HumanClientApp::SDLInit()
{
   const SDL_VideoInfo* vid_info = 0;

   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
      Logger().errorStream() << "SDL initialization failed: " << SDL_GetError();
      Exit(1);
   }

   if (SDLNet_Init() < 0) {
      Logger().errorStream() << "SDL Net initialization failed: " << SDLNet_GetError();
      Exit(1);
   }
  
   if (TTF_Init() < 0) {
      Logger().errorStream() << "TTF initialization failed: " << TTF_GetError();
      Exit(1);
   }

   if (FE_Init() < 0) {
      Logger().errorStream() << "FastEvents initialization failed: " << FE_GetError();
      Exit(1);
   }

   vid_info = SDL_GetVideoInfo();

   if (!vid_info) {
      Logger().errorStream() << "Video info query failed: " << SDL_GetError();
      Exit(1);
   }

   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

   if (SDL_SetVideoMode(AppWidth(), AppHeight(), 16, SDL_OPENGL) == 0) {
      Logger().errorStream() << "Video mode set failed: " << SDL_GetError();
      Exit(1);
   }
   
   if (NET2_Init() < 0) {
      Logger().errorStream() << "SDL Net2 initialization failed: " << NET2_GetError();
      Exit(1);
   }
  
   SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
   EnableMouseDragRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL / 2);

   Logger().debugStream() << "SDLInit() complete.";
   GLInit();
}

void HumanClientApp::GLInit()
{
   double ratio = AppWidth() / (float)(AppHeight());

   glEnable(GL_BLEND);
   glClearColor(0, 0, 0, 0);
   glViewport(0, 0, AppWidth() - 1, AppHeight() - 1);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(50.0, ratio, 1.0, 10.0);

   Logger().debugStream() << "GLInit() complete.";
}

void HumanClientApp::Initialize()
{
// ONLY TEMPORARY!!!!!
   g_net_test_wnd = boost::shared_ptr<NetTestWnd>(new NetTestWnd());
   Register(g_net_test_wnd.get());
// ONLY TEMPORARY!!!!!
}

void HumanClientApp::Enter2DMode()
{
	glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, AppWidth() - 1, AppHeight() - 1);

	glMatrixMode(GL_PROJECTION);
   glPushMatrix();
	glLoadIdentity();

   // set up coordinates with origin in upper-left and +x and +y directions right and down, respectively
   // the depth of the viewing volume is only 1 (from 0.0 to 1.0)
	glOrtho(0.0, (GLdouble)AppWidth(), (GLdouble)AppHeight(), 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
	glLoadIdentity();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void HumanClientApp::Exit2DMode()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();
}

void HumanClientApp::Update()
{
}

void HumanClientApp::Render()
{
   SDLGGApp::Render(); // paints GUI windows
}

void HumanClientApp::FinalCleanup()
{
}

void HumanClientApp::HandleMessageImpl(const Message& msg)
{
   switch (msg.Type()) {
   case Message::SERVER_STATUS:
      Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received SERVER_STATUS";
      break;
   case Message::HOST_GAME:
      if (msg.Sender() == -1 && msg.Receiver() == -1 && msg.GetText() == "ACK")
         Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received HOST_GAME acknowledgement";
      break;
   case Message::JOIN_GAME:
      if (msg.Sender() == -1 && msg.Receiver() == -1) {
         if (m_player_id == -1) {
            m_player_id = boost::lexical_cast<int>(msg.GetText());
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received JOIN_GAME acknowledgement";
         } else {
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received erroneous JOIN_GAME acknowledgement when already in a game";
         }
      }
      break;
   }
}

