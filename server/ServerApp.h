// -*- C++ -*-
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
struct AISetupData;
class CombatModule;
class Message;


/** contains the info needed to manage one player, including connection info */
struct PlayerInfo
{
    PlayerInfo(); ///< default ctor
    PlayerInfo(int sock, const IPaddress& addr, const std::string& player_name = "", bool host_ = false); ///< ctor

    bool        Connected() const {return socket != -1;}  ///< returns true if this player is still connected
   
    int         socket;  ///< socket on which the player is connected (-1 if there is no valid connection)
    IPaddress   address; ///< the IP address of the connected player
    std::string name;    ///< the unique name of the player
    bool        host;    ///< true if this is the host player
};


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

   /** creates an AI client child process for each element of \a AIs*/
   void CreateAIClients(const std::vector<AISetupData>& AIs);

   /** creates a single AI client child process for each AI_client subelement of \a elem.  This function is provided as a convenience 
       interface to void CreateAIClients(const LobbyModeData& AIs).*/
   void CreateAIClients(const GG::XMLElement& elem);

   /** handles an incoming message from the server with the appropriate action or response */
   void HandleMessage(const Message& msg);

   /** when Messages arrive from connections that are not established players, they arrive via a call to this function*/
   void HandleNonPlayerMessage(const Message& msg, const PlayerInfo& connection); 
   
   /** called by ServerNetworkCore when a player's TCP connection is closed*/
   void PlayerDisconnected(int id);
   //@}

   static ServerApp*             GetApp();         ///< returns a ClientApp pointer to the singleton instance of the app
   static ServerUniverse&        Universe();       ///< returns server's copy of Universe
   static ServerEmpireManager&   Empires();        ///< returns the server's copy of the Empires
   static CombatModule*          CurrentCombat();  ///< returns the server's currently executing Combat; may be 0
   static ServerNetworkCore&     NetworkCore();    ///< returns the network core object for the server
   
private:
   const ServerApp& operator=(const ServerApp&); // disabled
   ServerApp(const ServerApp&); // disabled

   void Run();             ///< initializes app state, then executes main event handler/render loop (PollAndRender())'

   void SDLInit();         ///< initializes SDL and SDL-related libs
   void Initialize();      ///< app initialization

   void Poll();            ///< handles all waiting SDL messages

   void FinalCleanup();    ///< app final cleanup
   void SDLQuit();         ///< cleans up FE and SDL
   
   void GameInit();        ///< intializes game universe, sends out initial game state to clients, and signals clients to start first turn

   GG::XMLDoc CreateTurnUpdate(int empire_id); ///< creates encoded universe and empire data for the specified empire, diffs it with the previous turn data, stores the new data over the previous turn data and returns the diff XMLElement
   GG::XMLDoc LobbyUpdateDoc() const;          ///< creates an MP lobby-mode update XMLDoc containing all relevant parts of the lobby state
   GG::XMLDoc LobbyPlayerUpdateDoc() const;    ///< creates an MP lobby-mode update XMLDoc containing just the current players

   ServerUniverse          m_universe;
   ServerEmpireManager     m_empires;
   CombatModule*           m_current_combat;
   ServerNetworkCore       m_network_core;

   log4cpp::Category&      m_log_category;         ///< reference to the log4cpp object used to log events to file
   
   ServerState             m_state;                ///< the server's current state of execution

   std::vector<Process>    m_ai_clients;           ///< AI client child processes
   
   // SERVER_GAME_SETUP variables -- These should only be useful when a new game is being set up
   std::set<std::string>   m_expected_ai_players;  ///< the player names expected from valid AI clients; this prevents malicious users from "spoofing" as AI clients.  Should be empty after all players have joined a game.
   int                     m_expected_players;     ///< the total desired number of players in the game

   int                     m_galaxy_size;          ///< the size of the galaxy (the number of star systems)
   ClientUniverse::Shape   m_galaxy_shape;         ///< the shape of the galaxy
   std::string             m_galaxy_file;          ///< file to use for generating the galaxy
   // end SERVER_GAME_SETUP variables

   std::map<int, GG::XMLDoc> m_last_turn_update_msg; ///< stores the xml encoded empire and universe data from the previous turn in order to generate diffs for turn update message.  Map is indexed by empire ID, with separate message data for each since each player sees different parts of the universe.

   static ServerApp*       s_app;
};

#endif // _ServerApp_h_

