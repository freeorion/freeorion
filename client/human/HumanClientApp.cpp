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
#include <ctime>
class NetTestWnd : public GG::Wnd
{
public:
   NetTestWnd() : 
      GG::Wnd(100, 100, 300, 600),
      m_music(0)
   {
      m_start_game =       new GG::Button(20,  20, 260, 30, "Start a game (3 AIs, 2 humans)", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      m_terminate =        new GG::Button(20,  70, 260, 30, "Terminate", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      m_send_doc_bn =      new GG::Button(20, 120, 260, 30, "Send an XMLDoc", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      m_send_text_bn =     new GG::Button(20, 170, 260, 30, "Send plain text", "arial.ttf", 12, GG::Clr(1.0, 1.0, 1.0, 0.75));
      
      AttachChild(m_start_game);
      AttachChild(m_terminate);
      AttachChild(m_send_doc_bn);
      AttachChild(m_send_text_bn);

      GG::Connect(m_start_game->ClickedSignal(), &NetTestWnd::StartGameClicked, this);
      GG::Connect(m_terminate->ClickedSignal(), &NetTestWnd::TerminateClicked, this);
      GG::Connect(m_send_doc_bn->ClickedSignal(), &NetTestWnd::SendDocClicked, this);
      GG::Connect(m_send_text_bn->ClickedSignal(), &NetTestWnd::SendTextClicked, this);
      
      // start playin' some tunes
      HumanClientApp::GetApp()->PlayMusic("music.ogg", 1);
      HumanClientApp::GetApp()->PlaySound("sample.wav", 0);
      int j=0, k=0;
      for (int i = 0; i < 200000000; ++i) {
         --k;
      }
      j = k;
      
      HumanClientApp::GetApp()->PlaySound("sample.wav", 0);
      HumanClientApp::GetApp()->FreeSound("sample.wav");
      HumanClientApp::GetApp()->FreeAllSounds();
      HumanClientApp::GetApp()->FreeSound("sample.wav");
   }
   
   ~NetTestWnd()
   {
   }
   
   virtual int Render()
   {
      HumanClientApp::GetApp()->Enter2DMode();
      GG::FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, GG::Clr(1.0, 1.0, 1.0, 0.75), GG::CLR_WHITE, 1);
      HumanClientApp::GetApp()->Exit2DMode();
		return 1;
   }
   
private:
   void StartGameClicked()
   {
      // start server
      const std::string SERVER_EXE = "freeoriond.exe";
      std::vector<std::string> args;
      args.push_back(SERVER_EXE);
      m_server_process = Process(SERVER_EXE, args);

      // connect to server
      while (!HumanClientApp::GetApp()->NetworkCore().ConnectToLocalhostServer()) ;

		//sleep(1);

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

   void TerminateClicked()
   {
      HumanClientApp::GetApp()->NetworkCore().SendMessage(EndGameMessage(HumanClientApp::GetApp()->PlayerID(), -1));
      HumanClientApp::GetApp()->Exit(0);
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

   GG::Button* m_start_game;
   GG::Button* m_terminate;
   GG::Button* m_send_doc_bn;
   GG::Button* m_send_text_bn;
   
   Mix_Music *m_music;
   
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
                      elem.Child("app_name").Text()),
   m_current_music(0)
{
}

HumanClientApp::~HumanClientApp()
{
   if (m_current_music) {
      Mix_HaltMusic();
      Mix_FreeMusic(m_current_music);
      m_current_music = 0;
   }
   Mix_HaltChannel(-1); // stop all sound playback
   for (std::map<std::string, Mix_Chunk*>::iterator it = m_sounds.begin(); it != m_sounds.end(); ++it) {
      Mix_FreeChunk(it->second);
   }
}

void HumanClientApp::PlayMusic(const std::string& filename, int repeats, int ms/* = 0*/, double position/* = 0.0*/)
{
   if (m_current_music) {
      Mix_HaltMusic();
      Mix_FreeMusic(m_current_music);
      m_current_music = 0;
   }
   m_current_music = Mix_LoadMUS(filename.c_str());
   if (m_current_music) {
   	if (Mix_PlayMusic(m_current_music, repeats + 1) == -1) {
         Mix_HaltMusic();
         Mix_FreeMusic(m_current_music);
         m_current_music = 0;
         Logger().errorStream() << "HumanClientApp::PlayMusic : An error occured while attempting to play \"" << 
            filename << "\"; SDL_mixer error: " << Mix_GetError();
   	}
   } else {
      Logger().errorStream() << "HumanClientApp::PlayMusic : An error occured while attempting to load \"" << 
         filename << "\"; SDL_mixer error: " << Mix_GetError();
	}
}

void HumanClientApp::PlaySound(const std::string& filename, int repeats, int timeout/* = -1*/)
{
   // load and cache the sound data
   std::map<std::string, Mix_Chunk*>::iterator it = m_sounds.find(filename);
   if (it == m_sounds.end()) {
      Mix_Chunk* data = Mix_LoadWAV(filename.c_str());
      if (!data) {
         Logger().errorStream() << "HumanClientApp::PlaySound : An error occured while attempting to load \"" << 
            filename << "\"; SDL_mixer error: " << Mix_GetError();
         return;
      } else {
         m_sounds[filename] = data;
      }
   }
   
   // find a free channel, creating an additional channel if needed
   Mix_Chunk* data = m_sounds[filename];
   int channel = 0;
   int num_channels = Mix_AllocateChannels(-1);
   for (; channel < num_channels; ++channel) {
      if (m_channels[channel] == "")
         break;
   }
   // there are not enough channels, so create one
   if (channel == num_channels) {
      Mix_AllocateChannels(channel);
      m_channels.resize(channel);
   }
   
   // play
   if (Mix_PlayChannel(channel, data, repeats) != channel) {
      Logger().errorStream() << "HumanClientApp::PlaySound : An error occured while attempting to play \"" << 
         filename << "\"; SDL_mixer error: " << Mix_GetError();
   } else {
      m_channels[channel] = filename;
   }
}

void HumanClientApp::FreeSound(const std::string& filename)
{
   if (m_sounds.find(filename) != m_sounds.end()) {
      bool still_playing = false;
      for (unsigned int i = 0; i < m_channels.size(); ++i) {
         if (m_channels[i] == filename) {
            still_playing = true;
            break;
         }
      }
      if (!still_playing) {
         Mix_FreeChunk(m_sounds[filename]);
         m_sounds.erase(filename);
         m_sounds_to_free.erase(filename);
      } else {
         m_sounds_to_free.insert(filename);
      }
   }
}
   
void HumanClientApp::FreeAllSounds()
{
   for (std::map<std::string, Mix_Chunk*>::iterator it = m_sounds.begin(); it != m_sounds.end();) {
      std::map<std::string, Mix_Chunk*>::iterator temp = it++;
      FreeSound(temp->first);
   }
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

HumanClientApp* HumanClientApp::GetApp()
{
   return dynamic_cast<HumanClientApp*>(GG::App::GetApp());
}
   
void HumanClientApp::SDLInit()
{
   const SDL_VideoInfo* vid_info = 0;

   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) < 0) {
      Logger().errorStream() << "SDL initialization failed: " << SDL_GetError();
      Exit(1);
   }
   
   int freq = MIX_DEFAULT_FREQUENCY; // sampling frequency
   Uint16 format = MIX_DEFAULT_FORMAT;
   int channels = 2; // stereo
   int chunk_sz = 2048;
   if (Mix_OpenAudio(freq, format, channels, chunk_sz)) {
      Logger().fatalStream() << "SDL Mixer initialization failed with parameters (frequency= " << freq << 
         ", format= " << format << ", channels= " << channels << ", chunksize= " << chunk_sz << "): " << Mix_GetError();
      Exit(1);
   } else {
      // ensure the correct runtime is being used      
      const SDL_version* link_version = Mix_Linked_Version();
      SDL_version compile_version;
      MIX_VERSION(&compile_version);
      if (compile_version.major != link_version->major || compile_version.minor != link_version->minor || 
         compile_version.patch != link_version->patch) {
         Logger().fatalStream() << "Version of SDL Mixer headers compiled with this program (v" << 
            compile_version.major << "." << compile_version.minor << "." << compile_version.patch << 
            ") does not match version in runtime library (v" <<
            link_version->major << "." << link_version->minor << "." << link_version->patch << ")";
         Exit(1);
      }      
      
      // check to see what values are actually being used, in case we didn't get what we wanted from initialization
      int actual_freq;
      Uint16 actual_format;
      int actual_channels;
      Mix_QuerySpec(&actual_freq, &actual_format, &actual_channels);
      if (freq != actual_freq) {
         Logger().debugStream() << "WARNING: SDL Mixer initialization was attempted with frequency= " << freq << ", but"
            "the actual frequency being used is " << actual_freq;
      }
      if (format != actual_format) {
         Logger().debugStream() << "WARNING: SDL Mixer initialization was attempted with format= " << format << ", but"
            "the actual format being used is " << actual_format;
      }
      if (channels != actual_channels) {
         Logger().debugStream() << "WARNING: SDL Mixer initialization was attempted in " << 
            (channels == 1 ? "mono" : "stereo") << ", but " << 
            (actual_channels == 1 ? "mono" : "stereo") << " is being used";
      }
     	Mix_HookMusicFinished(&HumanClientApp::EndOfMusicCallback);
      Mix_ChannelFinished(&HumanClientApp::EndOfSoundCallback);
      m_channels.resize(actual_channels, "");
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

void HumanClientApp::HandleSDLEvent(const SDL_Event& event)
{
   bool send_to_gg = false;
   GG::App::EventType gg_event;
   GG::Key key = GGKeyFromSDLKey(event.key.keysym);
   Uint32 key_mods = SDL_GetModState();
   GG::Pt mouse_pos(event.motion.x, event.motion.y);
   GG::Pt mouse_rel(event.motion.xrel, event.motion.yrel);

   switch(event.type) {
   case SDL_KEYDOWN: {
      if (key < GG::GGK_NUMLOCK)
         send_to_gg = true;
      gg_event = GG::App::KEYPRESS;
      break;
	}

   case SDL_MOUSEMOTION: {
      send_to_gg = true;
      gg_event = GG::App::MOUSEMOVE;
      break;
	}

   case SDL_MOUSEBUTTONDOWN: {
      send_to_gg = true;
      switch (event.button.button) {
      case SDL_BUTTON_LEFT:   gg_event = GG::App::LPRESS; break;
      case SDL_BUTTON_MIDDLE: gg_event = GG::App::MPRESS; break;
      case SDL_BUTTON_RIGHT:  gg_event = GG::App::RPRESS; break;
      }
      key_mods = SDL_GetModState();
      break;
	}

   case SDL_MOUSEBUTTONUP: {
      send_to_gg = true;
      switch (event.button.button) {
      case SDL_BUTTON_LEFT:   gg_event = GG::App::LRELEASE; break;
      case SDL_BUTTON_MIDDLE: gg_event = GG::App::MRELEASE; break;
      case SDL_BUTTON_RIGHT:  gg_event = GG::App::RRELEASE; break;
      }
      key_mods = SDL_GetModState();
      break;
	}

   case SDL_USEREVENT: {
      int net2_type = NET2_GetEventType(const_cast<SDL_Event*>(&event));
      if (net2_type == NET2_ERROREVENT || 
			 net2_type == NET2_TCPACCEPTEVENT || 
			 net2_type == NET2_TCPRECEIVEEVENT || 
			 net2_type == NET2_TCPCLOSEEVENT || 
			 net2_type == NET2_UDPRECEIVEEVENT)
		   m_network_core.HandleNetEvent(const_cast<SDL_Event&>(event));
      break;
	}

   case SDL_QUIT: {
      Exit(0);
      break;
	}
   }
   if (send_to_gg)
      GG::App::HandleEvent(gg_event, key, key_mods, mouse_pos, mouse_rel);
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
   if (NetworkCore().Connected()) {
      NetworkCore().DisconnectFromServer();
   }
   m_server_process.Kill();
}

void HumanClientApp::SDLQuit()
{
   FinalCleanup();
   NET2_Quit();
   FE_Quit();
   TTF_Quit();
   SDLNet_Quit();
   Mix_CloseAudio();   
   SDL_Quit();
   Logger().debugStream() << "SDLQuit() complete.";
}

void HumanClientApp::HandleMessageImpl(const Message& msg)
{
   switch (msg.Type()) {
   case Message::SERVER_STATUS:
      Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received SERVER_STATUS";
      break;
   case Message::HOST_GAME:
      if (msg.Sender() == -1 && msg.GetText() == "ACK")
         Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received HOST_GAME acknowledgement";
      break;
   case Message::JOIN_GAME:
      if (msg.Sender() == -1) {
         if (m_player_id == -1) {
            m_player_id = boost::lexical_cast<int>(msg.GetText());
            Logger().debugStream() << "HumanClientApp::HandleMessageImpl : Received JOIN_GAME acknowledgement";
         } else {
            Logger().errorStream() << "HumanClientApp::HandleMessageImpl : Received erroneous JOIN_GAME acknowledgement when already in a game";
         }
      }
      break;
   default:
      Logger().errorStream() << "HumanClientApp::HandleMessageImpl : Received unknown Message type code " << msg.Type();
      break;
   }
}

void HumanClientApp::EndOfMusicCallback()
{
   HumanClientApp* this_ptr = GetApp();
   if (!this_ptr->m_current_music)
      throw std::runtime_error("HumanClientApp::EndOfMusicCallback : End of a song was reached, but HumanClientApp::m_current_music == 0!");

   Mix_HaltMusic();
   Mix_FreeMusic(this_ptr->m_current_music);
   this_ptr->m_current_music = 0;
}

void HumanClientApp::EndOfSoundCallback(int channel)
{
   HumanClientApp* this_ptr = GetApp();
   std::map<std::string, Mix_Chunk*>::iterator it = this_ptr->m_sounds.find(this_ptr->m_channels[channel]);
   if (it == this_ptr->m_sounds.end()) {
      throw std::runtime_error("HumanClientApp::EndOfSoundCallback : End of a sound was reached, but there's no "
         "record of the filename associated with the channel that just stopped playing.");
   }
   std::string filename = it->first;
   this_ptr->m_channels[channel] = ""; 
   if (this_ptr->m_sounds_to_free.find(filename) != this_ptr->m_sounds_to_free.end())
      this_ptr->FreeSound(filename);
}

