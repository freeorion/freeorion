#include "ServerApp.h"

#include "../SDL_net2/fastevents.h"
#include "../network/Message.h"
#include "../GG/XML/XMLDoc.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>

#include <ctime>

////////////////////////////////////////////////
// ServerApp::PlayerInfo
////////////////////////////////////////////////
ServerApp::PlayerInfo::PlayerInfo(const ServerNetworkCore::ConnectionInfo& conn) : 
   ServerNetworkCore::ConnectionInfo(conn),
   name("")
{
}

ServerApp::PlayerInfo::PlayerInfo(const ServerNetworkCore::ConnectionInfo& conn, const std::string& _name) : 
   ServerNetworkCore::ConnectionInfo(conn),
   name(_name)
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
         CreateAIClients(doc.root_node);
         m_state = SERVER_GAME_SETUP;
         if (m_network_core.EstablishPlayer(m_players_info.size(), connection.socket)) {
            m_players_info.push_back(PlayerInfo(connection, host_player_name));
            m_network_core.SendMessage(HostAckMessage(m_players_info.size() - 1));
            m_network_core.SendMessage(JoinAckMessage(m_players_info.size() - 1));
         }
      } else {
         const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
         m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to host "
            "a new game but there was already one in progress or being setup.  Terminating connection to " << 
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
            m_players_info.push_back(PlayerInfo(connection, msg_text));
            m_expected_ai_players.erase(msg_text); // only allow one connection per AI
            m_network_core.SendMessage(JoinAckMessage(m_players_info.size() - 1));
         }
      } else { // non-AI player connection
         if (m_expected_ai_players.size() + m_players_info.size() < m_expected_players) {
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
      if (m_expected_ai_players.size() + m_players_info.size() == m_expected_players) { // if we've gotten all the players joined up
         GameInit();
         m_state = SERVER_WAITING;
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
   if (0 <= id && id < m_players_info.size()) {
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
   while (FE_PollEvent(&event)) {
      int net2_type = NET2_GetEventType(&event);
      if (event.type == SDL_USEREVENT && 
          (net2_type == NET2_ERROREVENT || 
           net2_type == NET2_TCPACCEPTEVENT || 
           net2_type == NET2_TCPRECEIVEEVENT || 
           net2_type == NET2_TCPCLOSEEVENT || 
           net2_type == NET2_UDPRECEIVEEVENT)) { // an SDL_net2 event
         m_network_core.HandleNetEvent(event);
      } else { // some other SDL event
// TODO: handle other relevant SDL events here
      }
   }
}

void ServerApp::FinalCleanup()
{
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

