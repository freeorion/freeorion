#ifndef _ServerApp_h_
#define _ServerApp_h_

#ifndef _ServerUniverse_h_
#include "../universe/ServerUniverse.h"
#endif 

#ifndef _ServerEmpire_h_
#include "../Empire/ServerEmpireManager.h"
#endif

#ifndef _ServerNetworkCore_h_
#include "../network/ServerNetworkCore.h"
#endif

#ifndef _Process_h_
#include "../util/Process.h"
#endif

#include <set>
#include <vector>


namespace log4cpp {class Category;}
namespace GG {class XMLDoc; class XMLElement;}
class CombatModule;
class Message;

/** the application framework class for the FreeOrion server. */
class ServerApp
{
public:                   
   /** \name Structors */ //@{   
   ServerApp(int argc, char* argv[]);
   ~ServerApp();
   //@}

   /** \name Accessors */ //@{
   GG::XMLDoc ServerStatus() const; ///< creates an XMLDoc that represents the status of the server, suitable for transmission to a client
   //@}

   /** \name Mutators */ //@{
   void                 operator()();     ///< external interface to Run()
   void                 Exit(int code);   ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
   log4cpp::Category&   Logger();         ///< returns the debug logging object for the app
   
   /** creates a single AI client child process for each AI_client subelement of \a elem*/
   void CreateAIClients(const GG::XMLElement& elem);

   /** handles an incoming message from the server with the appropriate action or response */
   void HandleMessage(const Message& msg);
   
   /** when Messages arrive from connections that are not established players, they arrive via a call to this function*/
   void HandleNonPlayerMessage(const Message& msg, const ServerNetworkCore::ConnectionInfo& connection); 
   
   /** called by ServerNetworkCore when a player's TCP connection is closed*/
   void PlayerDisconnected(int id); 
   //@}

   static ServerApp*             GetApp();         ///< returns a ClientApp pointer to the singleton instance of the app
   static ServerUniverse&        Universe();       ///< returns server's copy of Universe
   static ServerEmpireManager&   Empires();        ///< returns the server's copy of the Empires
   static CombatModule*          CurrentCombat();  ///< returns the server's currently executing Combat; may be 0
   static ServerNetworkCore&     NetworkCore();    ///< returns the network core object for the server
   
private:
   /** contains the info needed to manage one player, including connection info */
   struct PlayerInfo : public ServerNetworkCore::ConnectionInfo
   {
      PlayerInfo(const ServerNetworkCore::ConnectionInfo& conn); ///< default ctor
      PlayerInfo(const ServerNetworkCore::ConnectionInfo& conn, const std::string& _name, bool _host = false); ///< ctor

      std::string name;    ///< the player's name
      bool        host;    ///< true if this is the host player
// TODO: add other relevant player data
   };

   const ServerApp& operator=(const ServerApp&); // disabled
   ServerApp(const ServerApp&); // disabled

   void Run();             ///< initializes app state, then executes main event handler/render loop (PollAndRender())'

   void SDLInit();         ///< initializes SDL and SDL-related libs
   void Initialize();      ///< app initialization

   void Poll();            ///< handles all waiting SDL messages

   void FinalCleanup();    ///< app final cleanup
   void SDLQuit();         ///< cleans up FE and SDL
   
   void GameInit();        ///< intializes game universe, sends out initial game state to clients, and signals clients to start first turn

   ServerUniverse          m_universe;
   ServerEmpireManager     m_empires;
   CombatModule*           m_current_combat;
   ServerNetworkCore       m_network_core;

   log4cpp::Category&      m_log_category;         ///< reference to the log4cpp object used to log events to file
   
   ServerState             m_state;                ///< the server's current state of execution

   std::vector<PlayerInfo> m_players_info;         ///< the basic info on every player is stored here
   std::vector<Process>    m_ai_clients;           ///< AI client child processes
   
   // SERVER_GAME_SETUP variables
   std::set<std::string>   m_expected_ai_players;  ///< the player names expected from valid AI clients; this prevents malicious users from "spoofing" as AI clients
   int                     m_expected_players;     ///< the total desired number of players in the game

   int                     m_universe_size;        ///< the size of the universe

   static ServerApp*       s_app;
};

#endif // _ServerApp_h_

