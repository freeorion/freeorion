#include "ServerApp.h"

#include "fastevents.h"
#include "../network/Message.h"
#include "XMLDoc.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>

// for Videodriver setenv-hack
#include "SDL_getenv.h"

#include <ctime>

////////////////////////////////////////////////
// ServerApp::PlayerInfo
////////////////////////////////////////////////
ServerApp::PlayerInfo::PlayerInfo(const ServerNetworkCore::ConnectionInfo& conn) : 
   ServerNetworkCore::ConnectionInfo(conn),
   name(""),
   host(false)
{
}

ServerApp::PlayerInfo::PlayerInfo(const ServerNetworkCore::ConnectionInfo& conn, const std::string& _name, 
                                  bool _host/* = false*/) : 
	ServerNetworkCore::ConnectionInfo(conn),
   name(_name),
   host(_host)
{
}



////////////////////////////////////////////////
// ServerApp
////////////////////////////////////////////////
// static member(s)
ServerApp*  ServerApp::s_app = 0;

ServerApp::ServerApp(int argc, char* argv[]) : 
   m_current_combat(0), 
   m_log_category(log4cpp::Category::getRoot()),
   m_state(SERVER_IDLE)
{
   if (s_app)
      throw std::runtime_error("Attempted to construct a second instance of singleton class ServerApp");
   
   s_app = this;
   
   const std::string SERVER_LOG_FILENAME("freeoriond.log");
   
   // a platform-independent way to erase the old log
   std::ofstream temp(SERVER_LOG_FILENAME.c_str());
   temp.close();
   
   log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", SERVER_LOG_FILENAME);
   log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
   layout->setConversionPattern("%d %p : %m%n");
   appender->setLayout(layout);
   m_log_category.setAdditivity(false);  // make appender the only appender used...
   m_log_category.setAppender(appender);
   m_log_category.setAdditivity(true);   // ...but allow the addition of others later
   m_log_category.setPriority(log4cpp::Priority::DEBUG);
   m_log_category.debug("freeoriond logger initialized.");
   m_log_category.errorStream() << "ServerApp::ServerApp : Server now in mode " << SERVER_IDLE << " (SERVER_IDLE).";
   m_temp = 0;
}

ServerApp::~ServerApp()
{
   m_log_category.debug("Shutting down freeoriond logger...");
	log4cpp::Category::shutdown();
}

GG::XMLDoc ServerApp::ServerStatus() const
{
   GG::XMLDoc retval;
   GG::XMLElement elem("server_state");
   elem.SetAttribute("value", boost::lexical_cast<std::string>(m_state));
   retval.root_node.AppendChild(elem);
   return retval;
}

void ServerApp::operator()()
{
   Run();
}

void ServerApp::Exit(int code)
{
   Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
   SDLQuit();
   exit(code);
}

log4cpp::Category& ServerApp::Logger()
{
   return m_log_category;
}

void ServerApp::CreateAIClients(const GG::XMLElement& elem)
{
   int ai_client_base_number = time(0) % 999; // get a random number from which to start numbering the AI clients
   const std::string AI_CLIENT_EXE = "freeorionca.exe";
   for (int i = 0; i < elem.NumChildren(); ++i) {
      if (elem.Child(i).Tag() == "AI_client") {
// TODO: add other command line args to AI client invocation as needed
         std::string player_name = "AI_" + boost::lexical_cast<std::string>(ai_client_base_number + i); // AI player's "name"
         m_expected_ai_players.insert(player_name);
         std::vector<std::string> args;
         args.push_back(AI_CLIENT_EXE);
         args.push_back(player_name);
         m_ai_clients.push_back(Process(AI_CLIENT_EXE, args));
      }
   }
}

void ServerApp::HandleMessage(const Message& msg)
{
   switch (msg.Type()) {
   case Message::END_GAME: {
      if (0 <= msg.Sender() && msg.Sender() < static_cast<int>(m_players_info.size()) && m_players_info[msg.Sender()].host) {
         for (unsigned int i = 0; i < m_players_info.size(); ++i) {
				if (static_cast<int>(i) != msg.Sender())
               m_network_core.SendMessage(EndGameMessage(-1, i));
         }
         m_network_core.DumpAllConnections();
         Exit(0);
      }
      break;
   }
      
   default: {
      m_log_category.errorStream() << "ServerApp::HandleMessage : Received an unknown message type \"" << msg.Type() << "\".";
      break;
   }
   }
}

void ServerApp::HandleNonPlayerMessage(const Message& msg, const ServerNetworkCore::ConnectionInfo& connection)
{
   switch (msg.Type()) {
   case Message::HOST_GAME: {
      if (m_players_info.empty() && m_expected_ai_players.empty()) {
         std::stringstream stream(msg.GetText());
         GG::XMLDoc doc;
         doc.ReadDoc(stream);
         std::string host_player_name = doc.root_node.Child("host_player_name").Text();
         m_expected_players = boost::lexical_cast<int>(doc.root_node.Child("num_players").Attribute("value"));
         m_universe_size = boost::lexical_cast<int>(doc.root_node.Child("universe_params").Attribute("size"));
	 m_universe_shape = boost::lexical_cast<int>(doc.root_node.Child("universe_params").Attribute("shape"));
	 if (m_universe_shape == 4)
           m_universe_file = doc.root_node.Child("universe_params").Attribute("file");
         CreateAIClients(doc.root_node);
         m_state = SERVER_GAME_SETUP;
         m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
         m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Universe size set to " << m_universe_size << " (SERVER_GAME_SETUP).";
         m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Universe shape set to " << m_universe_shape << " (SERVER_GAME_SETUP).";
	 if (m_universe_shape == 6)
	 {
           m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Universe file set to " << m_universe_file << " (SERVER_GAME_SETUP).";
	   ServerApp::Universe().CreateUniverse(m_universe_file, m_universe_size, 5, 2);
           m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Created universe (SERVER_GAME_SETUP).";
	 }
	 else 
	 { 
	   ServerApp::Universe().CreateUniverse((ClientUniverse::Shape)m_universe_shape, m_universe_size, 5, 3); 
           m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Created universe without file (SERVER_GAME_SETUP).";
	 }
	 
         if (m_network_core.EstablishPlayer(m_players_info.size(), connection.socket)) {
            m_network_core.SendMessage(HostAckMessage(m_players_info.size()));
            m_network_core.SendMessage(JoinAckMessage(m_players_info.size()));
            m_players_info.push_back(PlayerInfo(connection, host_player_name, true));
         }

	 // set up the universe with the details obtained from the client - currently fixed to one AI
      } else {
         const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
         m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to host "
            "a new game but there was already one in progress or one being setup.  Terminating connection to " << 
            (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
         m_network_core.DumpConnection(connection.socket);
      }
      break;
   }

   case Message::JOIN_GAME: {
      std::string msg_text = msg.GetText(); // the player name should be the entire text
      std::set<std::string>::iterator it = m_expected_ai_players.find(msg_text);
      if (it != m_expected_ai_players.end()) { // incoming AI player connection
         // let the server network core know what socket this player is on
         if (m_network_core.EstablishPlayer(m_players_info.size(), connection.socket)) {
            m_network_core.SendMessage(JoinAckMessage(m_players_info.size()));
            m_players_info.push_back(PlayerInfo(connection, msg_text));
            m_expected_ai_players.erase(msg_text); // only allow one connection per AI
         }
      } else { // non-AI player connection
         if (static_cast<int>(m_expected_ai_players.size() + m_players_info.size()) < m_expected_players) {
            if (m_network_core.EstablishPlayer(m_players_info.size(), connection.socket)) {
               m_players_info.push_back(PlayerInfo(connection, msg_text));
               m_network_core.SendMessage(JoinAckMessage(m_players_info.size() - 1));
            }
         } else {
            const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
            m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to join "
               "the game but there was not enough room.  Terminating connection to " << 
               (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
            m_network_core.DumpConnection(connection.socket);
         }
      }
      if (static_cast<int>(m_expected_ai_players.size() + m_players_info.size()) == m_expected_players) { // if we've gotten all the players joined up
         GameInit();
         m_state = SERVER_WAITING;
         m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
      }
      break;
   }

   default: {
      const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
      m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : Received an invalid message type \"" <<
         msg.Type() << "\" for a non-player Message.  Terminating connection to " << 
         (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
      m_network_core.DumpConnection(connection.socket);
      break;
   }
   }
}

void ServerApp::PlayerDisconnected(int id)
{
   if (0 <= id && id < static_cast<int>(m_players_info.size())) {
      m_players_info[id].socket = -1;
      m_state = SERVER_DISCONNECT;
// TODO: try to reconnect
// TODO: send request to host player to ask what to do
   } else {
      m_log_category.errorStream() << "ServerApp::PlayerDisconnected : Received unknown player id \"" << id << "\"";
   }
}

ServerApp* ServerApp::GetApp()
{
   return s_app;
}

ServerUniverse& ServerApp::Universe()
{
   return ServerApp::GetApp()->m_universe;
}

ServerEmpireManager& ServerApp::Empires()
{
   return ServerApp::GetApp()->m_empires;
}

CombatModule* ServerApp::CurrentCombat()
{
   return ServerApp::GetApp()->m_current_combat;
}

ServerNetworkCore& ServerApp::NetworkCore()
{
   return ServerApp::GetApp()->m_network_core;
}

void ServerApp::Run()
{
   try {
      SDLInit();
      Initialize();
      while (1)
         Poll();
   } catch (const std::invalid_argument& exception) {
      m_log_category.fatal("std::invalid_argument Exception caught in ServerApp::Run(): " + std::string(exception.what()));
      Exit(1);
   } catch (const std::runtime_error& exception) {
      m_log_category.fatal("std::runtime_error Exception caught in ServerApp::Run(): " + std::string(exception.what()));
      Exit(1);
   }
}

void ServerApp::SDLInit()
{
   /* Dirty hack to active the dummy video handler of SDL
    * if the user has already set SDL_VIDEODRIVER, we trust him */
   if (getenv("SDL_VIDEODRIVER") == NULL) {
       putenv("SDL_VIDEODRIVER=dummy");
   }
 
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
      Logger().errorStream() << "SDL initialization failed: " << SDL_GetError();
      Exit(1);
   }

   if (SDLNet_Init() < 0) {
      Logger().errorStream() << "SDL Net initialization failed: " << SDLNet_GetError();
      Exit(1);
   }
  
   if (FE_Init() < 0) {
      Logger().errorStream() << "FastEvents initialization failed: " << FE_GetError();
      Exit(1);
   }

   if (NET2_Init() < 0) {
      Logger().errorStream() << "SDL Net2 initialization failed: " << NET2_GetError();
      Exit(1);
   }
  
   Logger().debugStream() << "SDLInit() complete.";
}

void ServerApp::Initialize()
{
   m_network_core.ListenToPorts();
}

void ServerApp::Poll()
{
   // handle events
   SDL_Event event;
   while (FE_WaitEvent(&event)) {
      int net2_type = NET2_GetEventType(&event);
      if (event.type == SDL_USEREVENT && 
          (net2_type == NET2_ERROREVENT || 
           net2_type == NET2_TCPACCEPTEVENT || 
           net2_type == NET2_TCPRECEIVEEVENT || 
           net2_type == NET2_TCPCLOSEEVENT || 
           net2_type == NET2_UDPRECEIVEEVENT)) { // an SDL_net2 event
         m_network_core.HandleNetEvent(event);
      } else { // some other SDL event
		   switch (event.type) {
			case SDL_QUIT:
			   Exit(0);
			   break;
			}
// TODO: handle other relevant SDL events here
      }
   }
}

void ServerApp::FinalCleanup()
{
	NetworkCore().DumpAllConnections();
	for (unsigned int i = 0; i < m_ai_clients.size(); ++i)
	   m_ai_clients[i].Kill();
}

void ServerApp::SDLQuit()
{
   FinalCleanup();
   NET2_Quit();
   FE_Quit();
   SDLNet_Quit();
   SDL_Quit();
   Logger().debugStream() << "SDLQuit() complete.";
}

void ServerApp::GameInit()
{
}

