// -*- C++ -*-
#ifndef _ServerApp_h_
#define _ServerApp_h_

#include "ServerFSM.h"
#include "../util/AppInterface.h"
#include "../util/Process.h"
#include "../Empire/EmpireManager.h"
#include "../network/Networking.h"
#include "../network/ServerNetworking.h"
#include "../universe/Universe.h"
#include "../util/MultiplayerCommon.h"

#include <set>
#include <vector>


namespace log4cpp {
    class Category;
}
class CombatModule;
class Message;
class OrderSet;
struct PlayerSetupData;
struct SaveGameUIData;

/** contains the data that must be saved for a single player.  Note that the m_empire member is not deallocated by
    PlayerSaveGameData.  Users of PlayerSaveGameData are resposible for managing its lifetime. */
struct PlayerSaveGameData
{
    PlayerSaveGameData(); ///< default ctor
    PlayerSaveGameData(const std::string& name, Empire* empire, const boost::shared_ptr<OrderSet>& orders, const boost::shared_ptr<SaveGameUIData>& ui_data); ///< ctor

    std::string                       m_name;
    Empire*                           m_empire;
    boost::shared_ptr<OrderSet>       m_orders;
    boost::shared_ptr<SaveGameUIData> m_ui_data;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** the application framework class for the FreeOrion server. */
class ServerApp
{
public:                   
    /** \name Structors */ //@{   
    ServerApp();
    ~ServerApp();
    //@}

    /** \name Accessors */ //@{
    int             CurrentTurn() const {return m_current_turn;}                ///< returns current turn of the server
    //@}

    /** \name Mutators */ //@{
    void                 operator()();     ///< external interface to Run()
    void                 Exit(int code);   ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
    log4cpp::Category&   Logger();         ///< returns the debug logging object for the app

    /** creates an AI client child process for each element of \a AIs*/
    void CreateAIClients(const std::vector<PlayerSetupData>& AIs, std::set<std::string>& expected_ai_player_names);

    /**  Adds an existing empire to turn processing. The position the empire is in the vector is it's position in the turn processing.*/
    void AddEmpireTurn(int empire_id);

    /** Removes an empire from turn processing. This is most likely called when an empire is eliminated from the game */
    void RemoveEmpireTurn(int empire_id);

    /** Adds turn orders for the given empire for the current turn. pOrderSet will be freed when all processing is done for the turn */
    void SetEmpireTurnOrders(int empire_id, OrderSet* pOrderSet);

    /** Determines if all empired have submitted their orders for this turn It will loop the turn squence vector and check for a set pOrderSet. A pOrderSet 
     * of NULL indicates that the empire has not yet submitted their orders for the given turn */
    bool AllOrdersReceived();

    /** Processes all empires in the manager in the order that they are added. Will delete all pOrderSets assigned.*/
    void ProcessTurns();

    void NewGameInit(boost::shared_ptr<SinglePlayerSetupData> setup_data);     ///< intializes game universe, sends out initial game state to clients, and signals clients to start first turn
    void LoadGameInit(boost::shared_ptr<SinglePlayerSetupData> setup_data, const std::vector<PlayerSaveGameData>& player_save_game_data);    ///< restores saved game universe, sends out game state and saved pending orders to clients, and signals clients to finish current turn

    void NewGameInit(boost::shared_ptr<MultiplayerLobbyData> lobby_data);     ///< intializes game universe, sends out initial game state to clients, and signals clients to start first turn
    void LoadGameInit(boost::shared_ptr<MultiplayerLobbyData> lobby_data, const std::vector<PlayerSaveGameData>& player_save_game_data);    ///< restores saved game universe, sends out game state and saved pending orders to clients, and signals clients to finish current turn
    //@}

    static ServerApp*             GetApp();         ///< returns a ClientApp pointer to the singleton instance of the app
    static Universe&              GetUniverse();    ///< returns server's copy of Universe
    static EmpireManager&         Empires();        ///< returns the server's copy of the Empires
    static CombatModule*          CurrentCombat();  ///< returns the server's currently executing Combat; may be 0
    static ServerNetworking&      Networking();     ///< returns the networking object for the server

private:
    const ServerApp& operator=(const ServerApp&); // disabled
    ServerApp(const ServerApp&); // disabled

    void Run();             ///< initializes app state, then executes main event handler/render loop (Poll())
    void CleanupAIs();      ///< cleans up AI processes

    /** handles an incoming message from the server with the appropriate action or response */
    void HandleMessage(const Message& msg, PlayerConnectionPtr player_connection);

    /** when Messages arrive from connections that are not established players, they arrive via a call to this function*/
    void HandleNonPlayerMessage(const Message& msg, PlayerConnectionPtr player_connection);

    /** called by ServerNetworking when a player's TCP connection is closed*/
    void PlayerDisconnected(PlayerConnectionPtr player_connection);

    Empire* GetPlayerEmpire(int player_id) const;   ///< returns the object for the empire that that the player with ID \a player_id is playing
    int     GetEmpirePlayerID(int empire_id) const; ///< returns the player ID for the player playing the empire with ID \a empire_id

    boost::asio::io_service   m_io_service;

    Universe                  m_universe;
    EmpireManager             m_empires;
    CombatModule*             m_current_combat;
    ServerNetworking          m_networking;

    log4cpp::Category&        m_log_category;         ///< reference to the log4cpp object used to log events to file

    ServerFSM                 m_fsm;
   
    int                       m_current_turn;         ///< current turn number

    std::vector<Process>      m_ai_clients;           ///< AI client child processes
    std::set<int>             m_ai_IDs;               ///< player IDs of AI clients

    bool                      m_single_player_game;   ///< true when the game being played is single-player

    // turn sequence map is used for turn processing. Each empire is added at the start of a game or reload and then the map maintains OrderSets for that turn
    std::map<int, OrderSet*>  m_turn_sequence;

    std::set<int>             m_losers;               ///< the IDs of players who have been eliminated during the normal course of the game

    static ServerApp*         s_app;

    // Give FSM and its states direct access.  We are using the FSM code as a control-flow mechanism; it is all
    // notionally part of this class.
    friend struct ServerFSM;
    friend struct Idle;
    friend struct MPLobby;
    friend struct WaitingForSPGameJoiners;
    friend struct WaitingForMPGameJoiners;
    friend struct PlayingGame;
    friend struct WaitingForTurnEnd;
    friend struct WaitingForTurnEndIdle;
    friend struct WaitingForSaveData;
};

// template implementations
template <class Archive>
void PlayerSaveGameData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_empire)
        & BOOST_SERIALIZATION_NVP(m_orders)
        & BOOST_SERIALIZATION_NVP(m_ui_data);
}

#endif // _ServerApp_h_

