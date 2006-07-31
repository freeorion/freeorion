// -*- C++ -*-
#ifndef _ServerApp_h_
#define _ServerApp_h_

#ifndef _AppInterface_h_
#include "../util/AppInterface.h"
#endif

#ifndef _Process_h_
#include "../util/Process.h"
#endif

#ifndef _ServerEmpire_h_
#include "../Empire/ServerEmpireManager.h"
#endif

#ifndef _ServerNetworkCore_h_
#include "../network/ServerNetworkCore.h"
#endif

#ifndef _Universe_h_
#include "../universe/Universe.h"
#endif 

#include <set>
#include <vector>


namespace log4cpp {
    class Category;
}
class CombatModule;
class Message;
class OrderSet;
struct PlayerSetupData;
class XMLDoc;
class XMLElement;

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
    ServerState     State() const {return m_state;} ///< returns the current state of the server (one of the ServerState enum values)
    XMLDoc          ServerStatusDoc() const;        ///< returns an XMLDoc that represents the status of the server, suitable for transmission to a client
    int             CurrentTurn() const {return m_current_turn;}                ///< returns current turn of the server
    //@}

    /** \name Mutators */ //@{
    void                 operator()();     ///< external interface to Run()
    void                 Exit(int code);   ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
    log4cpp::Category&   Logger();         ///< returns the debug logging object for the app

    /** creates an AI client child process for each element of \a AIs*/
    void CreateAIClients(const std::vector<PlayerSetupData>& AIs);

    /** creates a single AI client child process for each AI_client subelement of \a elem.  This function is provided as a convenience 
        interface to void CreateAIClients(const LobbyModeData& AIs).*/
    void CreateAIClients(const XMLElement& elem);

    /** handles an incoming message from the server with the appropriate action or response */
    void HandleMessage(const Message& msg);

    /** when Messages arrive from connections that are not established players, they arrive via a call to this function*/
    void HandleNonPlayerMessage(const Message& msg, const PlayerInfo& connection); 

    /** called by ServerNetworkCore when a player's TCP connection is closed*/
    void PlayerDisconnected(int id);

    /**  Adds an existing empire to turn processing. The position the empire is in the vector is it's position in the turn processing.*/
    void AddEmpireTurn( int empire_id );

    /** Removes an empire from turn processing. This is most likely called when an empire is eliminated from the game */
    void RemoveEmpireTurn( int empire_id );

    /** Adds turn orders for the given empire for the current turn. pOrderSet will be freed when all processing is done for the turn */
    void SetEmpireTurnOrders( int empire_id , OrderSet *pOrderSet );

    /** Determines if all empired have submitted their orders for this turn It will loop the turn squence vector and check for a set pOrderSet. A pOrderSet 
     * of NULL indicates that the empire has not yet submitted their orders for the given turn */
    bool AllOrdersReceived( );

    /** Processes all empires in the manager in the order that they are added. Will delete all pOrderSets assigned.*/
    void ProcessTurns( );
    //@}

    static ServerApp*             GetApp();         ///< returns a ClientApp pointer to the singleton instance of the app
    static Universe&              GetUniverse();    ///< returns server's copy of Universe
    static ServerEmpireManager&   Empires();        ///< returns the server's copy of the Empires
    static CombatModule*          CurrentCombat();  ///< returns the server's currently executing Combat; may be 0
    static ServerNetworkCore&     NetworkCore();    ///< returns the network core object for the server

private:
    const ServerApp& operator=(const ServerApp&); // disabled
    ServerApp(const ServerApp&); // disabled

    void Run();             ///< initializes app state, then executes main event handler/render loop (Poll())

    void SDLInit();         ///< initializes SDL and SDL-related libs
    void Initialize();      ///< app initialization

    void Poll();            ///< handles all waiting SDL messages

    void FinalCleanup();    ///< app final cleanup
    void SDLQuit();         ///< cleans up FE and SDL

    void NewGameInit();     ///< intializes game universe, sends out initial game state to clients, and signals clients to start first turn
    void LoadGameInit();    ///< restores saved game universe, sends out game state and saved pending orders to clients, and signals clients to finish current turn

    /** Returns true iff the versions of settings files and/or source code differ between the server and the client sending \a doc.
        As a side effect, logs an error message and sends a failure message to the sending player as appropriate. */
    bool VersionMismatch(int player_id, const PlayerInfo& player_info, const PlayerInfo& connection, const XMLDoc& doc);

    XMLDoc CreateTurnUpdate(int empire_id); ///< creates encoded universe and empire data for the specified empire
    XMLDoc LobbyUpdateDoc() const;          ///< returns an MP lobby-mode update XMLDoc containing all relevant parts of the lobby state
    XMLDoc LobbyStartDoc() const;           ///< returns an MP lobby-mode update XMLDoc containing just the initial server-side data that the clients don't have
    XMLDoc SaveGameUpdateDoc() const;       ///< returns an MP lobby-mode update XMLDoc containing just empire data for the currently-selected save game

    void SaveGameVars(XMLDoc& doc) const;   ///< adds all game-state variables to \a doc
    void LoadGameVars(const XMLDoc& doc);   ///< assigns all game-state variables from \a doc

    Empire* GetPlayerEmpire(int player_id) const;   ///< returns the object for the empire that that the player with ID \a player_id is playing
    int     GetEmpirePlayerID(int empire_id) const; ///< returns the player ID for the player playing the empire with ID \a empire_id
   
    Universe                m_universe;
    ServerEmpireManager     m_empires;
    CombatModule*           m_current_combat;
    ServerNetworkCore       m_network_core;

    log4cpp::Category&      m_log_category;         ///< reference to the log4cpp object used to log events to file

    ServerState             m_state;                ///< the server's current state of execution
   
    int                     m_current_turn;         ///< current turn number

    std::vector<Process>    m_ai_clients;           ///< AI client child processes
    std::set<int>           m_ai_IDs;               ///< player IDs of AI clients

    // SERVER_GAME_SETUP variables -- These should only be useful when a new game is being set up
    std::set<std::string>   m_expected_ai_players;  ///< the player names expected from valid AI clients; this prevents malicious users from "spoofing" as AI clients.  Should be empty after all players have joined a game.
    int                     m_expected_players;     ///< the total desired number of players in the game

    int                          m_galaxy_size;     ///< the size of the galaxy (the number of star systems)
    Shape                        m_galaxy_shape;    ///< the shape of the galaxy
    Age                          m_galaxy_age;      ///< the age of the galaxy
    StarlaneFrequency            m_starlane_freq;   ///< the frequency of starlanes
    PlanetDensity                m_planet_density;  ///< the density of planets within systems
    SpecialsFrequency            m_specials_freq;   ///< the frequency of planetary and system specials
    std::string                  m_galaxy_file;     ///< file to use for generating the galaxy
    // end SERVER_GAME_SETUP variables

    bool                      m_single_player_game;    ///< true when the game being played is single-player

    std::set<int>             m_players_responded;     ///< tracks which players have responded to a server request (eg for save-data)
    std::map<int, XMLElement> 
    m_player_save_game_data; ///< stores the save game data coming in from the players during a save game operation

    // turn sequence map is used for turn processing. Each empire is added at the start of a game or reload and then the map maintains OrderSets for that turn
    std::map<int, OrderSet*>  m_turn_sequence;

    std::set<int>             m_losers;                ///< the IDs of players who have been eliminated during the normal course of the game

    static ServerApp*         s_app;
};

#endif // _ServerApp_h_

