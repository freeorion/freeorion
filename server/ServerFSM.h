#ifndef _ServerFSM_h_
#define _ServerFSM_h_


#include "../network/Message.h"

#include <boost/mpl/list.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deferral.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/in_state_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/asio/high_resolution_timer.hpp>

#include <memory>
#include <set>
#include <vector>
#include <unordered_set>
#include <chrono>


struct MultiplayerLobbyData;
class ServerApp;
struct SinglePlayerSetupData;
class PlayerConnection;
struct PlayerSaveGameData;
struct ServerSaveGameData;
class System;
typedef std::shared_ptr<PlayerConnection> PlayerConnectionPtr;

namespace sc = boost::statechart;

// Non-Message events
struct Disconnection : sc::event<Disconnection> {
    Disconnection(PlayerConnectionPtr& player_connection);
    PlayerConnectionPtr& m_player_connection;
};

struct LoadSaveFileFailed : sc::event<LoadSaveFileFailed>               {};
struct CheckStartConditions : sc::event<CheckStartConditions>           {};
struct CheckEndConditions : sc::event<CheckEndConditions>               {};
struct CheckTurnEndConditions : sc::event<CheckTurnEndConditions>       {};
struct ProcessTurn : sc::event<ProcessTurn>                             {};
struct DisconnectClients : sc::event<DisconnectClients>                 {};
struct ShutdownServer : sc::event<ShutdownServer>                       {};


//  MessagePacket events
/** The base class for all state machine events that are based on Messages. */
struct MessageEventBase {
    MessageEventBase(const MessagePacket& message, PlayerConnectionPtr& player_connection);

    MessagePacket              m_message;
    PlayerConnectionPtr  m_player_connection;
};

// Define Boost.Preprocessor list of all MessagePacket events
#define MESSAGE_EVENTS                      \
    (HostMPGame)                            \
    (HostSPGame)                            \
    (StartMPGame)                           \
    (LobbyUpdate)                           \
    (LobbyChat)                             \
    (JoinGame)                              \
    (LeaveGame)                             \
    (SaveGameRequest)                       \
    (TurnOrders)                            \
    (CombatTurnOrders)                      \
    (ClientSaveData)                        \
    (RequestObjectID)                       \
    (RequestDesignID)                       \
    (RequestCombatLogs)                     \
    (PlayerChat)                            \
    (Diplomacy)                             \
    (ModeratorAct)                          \
    (Error)


#define DECLARE_MESSAGE_EVENT(r, data, name)                                    \
    struct name :                                                               \
        sc::event<name>,                                                        \
        MessageEventBase                                                        \
    {                                                                           \
        name(const MessagePacket& message, PlayerConnectionPtr& player_connection) :  \
            MessageEventBase(message, player_connection)                        \
        {}                                                                      \
    };

BOOST_PP_SEQ_FOR_EACH(DECLARE_MESSAGE_EVENT, _, MESSAGE_EVENTS)

#undef DECLARE_MESSAGE_EVENT


// Top-level server states
struct Idle;
struct MPLobby;
struct WaitingForSPGameJoiners;
struct WaitingForMPGameJoiners;
struct PlayingGame;
struct ShuttingDownServer;

// Substates of PlayingGame
struct WaitingForTurnEnd;
struct ProcessingTurn;

// Substates of WaitingForTurnEnd
struct WaitingForTurnEndIdle;
struct WaitingForSaveData;


#define SERVER_ACCESSOR private: ServerApp& Server() { return context<ServerFSM>().Server(); }

/** The finite state machine that represents the server's operation. */
struct ServerFSM : sc::state_machine<ServerFSM, Idle> {
    ServerFSM(ServerApp &server);

    void unconsumed_event(const sc::event_base &event);
    ServerApp& Server();
    void HandleNonLobbyDisconnection(const Disconnection& d);

    std::shared_ptr<MultiplayerLobbyData> m_lobby_data;
    std::shared_ptr<SinglePlayerSetupData> m_single_player_setup_data;
    std::vector<PlayerSaveGameData>             m_player_save_game_data;
    std::shared_ptr<ServerSaveGameData> m_server_save_game_data;

private:
    ServerApp& m_server;
};


/** The server's initial state. */
struct Idle : sc::state<Idle, ServerFSM> {
    typedef boost::mpl::list<
        sc::custom_reaction<HostMPGame>,
        sc::custom_reaction<HostSPGame>,
        sc::custom_reaction<ShutdownServer>,
        sc::custom_reaction<Error>
    > reactions;

    Idle(my_context c);
    ~Idle();

    sc::result react(const HostMPGame& msg);
    sc::result react(const HostSPGame& msg);
    sc::result react(const ShutdownServer& u);
    sc::result react(const Error& msg);

    SERVER_ACCESSOR
};


/** The server state in which the multiplayer lobby is active. */
struct MPLobby : sc::state<MPLobby, ServerFSM> {
    typedef boost::mpl::list<
        sc::custom_reaction<Disconnection>,
        sc::custom_reaction<JoinGame>,
        sc::custom_reaction<LobbyUpdate>,
        sc::custom_reaction<LobbyChat>,
        sc::custom_reaction<StartMPGame>,
        sc::custom_reaction<HostMPGame>,
        sc::custom_reaction<HostSPGame>,
        sc::custom_reaction<ShutdownServer>,
        sc::custom_reaction<Error>
    > reactions;

    MPLobby(my_context c);
    ~MPLobby();

    sc::result react(const Disconnection& d);
    sc::result react(const JoinGame& msg);
    sc::result react(const LobbyUpdate& msg);
    sc::result react(const LobbyChat& msg);
    sc::result react(const StartMPGame& msg);
    sc::result react(const HostMPGame& msg);
    sc::result react(const HostSPGame& msg);
    sc::result react(const ShutdownServer& u);
    sc::result react(const Error& msg);

    std::shared_ptr<MultiplayerLobbyData> m_lobby_data;
    std::vector<PlayerSaveGameData>         m_player_save_game_data;
    std::shared_ptr<ServerSaveGameData> m_server_save_game_data;

    SERVER_ACCESSOR
};


/** The server state in which a new single-player game has been initiated, and
  * the server is waiting for all players to join. */
struct WaitingForSPGameJoiners : sc::state<WaitingForSPGameJoiners, ServerFSM> {
    typedef boost::mpl::list<
        sc::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        sc::custom_reaction<JoinGame>,
        sc::custom_reaction<CheckStartConditions>,
        sc::custom_reaction<LoadSaveFileFailed>,
        sc::custom_reaction<Error>
    > reactions;

    WaitingForSPGameJoiners(my_context c);
    ~WaitingForSPGameJoiners();

    sc::result react(const JoinGame& msg);
    sc::result react(const CheckStartConditions& u);
    // in SP games, save data is loaded when setting up AI clients, in order to
    // know which / how many to set up.  Loading might fail.
    sc::result react(const LoadSaveFileFailed& u);
    sc::result react(const Error& msg);

    std::shared_ptr<SinglePlayerSetupData>   m_single_player_setup_data;
    std::vector<PlayerSaveGameData>          m_player_save_game_data;
    std::shared_ptr<ServerSaveGameData>      m_server_save_game_data;
    std::map<std::string, int>               m_expected_ai_names_and_ids;
    int                                      m_num_expected_players;

    SERVER_ACCESSOR
};


/** The server state in which a multiplayer game has been initiated, and the
  * server is waiting for all players to join. */
struct WaitingForMPGameJoiners : sc::state<WaitingForMPGameJoiners, ServerFSM> {
    typedef boost::mpl::list<
        sc::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        sc::custom_reaction<JoinGame>,
        sc::custom_reaction<CheckStartConditions>,
        sc::custom_reaction<Error>
    > reactions;

    WaitingForMPGameJoiners(my_context c);
    ~WaitingForMPGameJoiners();

    sc::result react(const JoinGame& msg);
    sc::result react(const CheckStartConditions& u);
    // unlike in SP game setup, no save file data needs to be loaded in this
    // state, as all the relevant info about AIs is provided by the lobby data.
    // as such, no file load error handling reaction is needed in this state.
    sc::result react(const Error& msg);

    std::shared_ptr<MultiplayerLobbyData> m_lobby_data;
    std::vector<PlayerSaveGameData>         m_player_save_game_data;
    std::shared_ptr<ServerSaveGameData> m_server_save_game_data;
    std::set<std::string>                   m_expected_ai_player_names;
    int                                     m_num_expected_players;

    SERVER_ACCESSOR
};


/** The server state in which a game has been started, and is actually being
  * played. */
struct PlayingGame : sc::state<PlayingGame, ServerFSM, WaitingForTurnEnd> {
    typedef boost::mpl::list<
        sc::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        sc::custom_reaction<PlayerChat>,
        sc::custom_reaction<Diplomacy>,
        sc::custom_reaction<ModeratorAct>,
        sc::custom_reaction<RequestCombatLogs>,
        sc::custom_reaction<ShutdownServer>,
        sc::custom_reaction<Error>
    > reactions;

    PlayingGame(my_context c);
    ~PlayingGame();

    sc::result react(const PlayerChat& msg);
    sc::result react(const Diplomacy& msg);
    sc::result react(const ModeratorAct& msg);
    sc::result react(const ShutdownServer& u);
    sc::result react(const RequestCombatLogs& msg);
    sc::result react(const Error& msg);

    SERVER_ACCESSOR
};


/** The substate of PlayingGame in which players are playing their turns and
  * the server is waiting for all players to finish their moves, after which
  * the server will process the turn. */
struct WaitingForTurnEnd : sc::state<WaitingForTurnEnd, PlayingGame, WaitingForTurnEndIdle> {
    typedef boost::mpl::list<
        sc::custom_reaction<TurnOrders>,
        sc::custom_reaction<RequestObjectID>,
        sc::custom_reaction<RequestDesignID>,
        sc::custom_reaction<CheckTurnEndConditions>
    > reactions;

    WaitingForTurnEnd(my_context c);
    ~WaitingForTurnEnd();

    sc::result react(const TurnOrders& msg);
    sc::result react(const RequestObjectID& msg);
    sc::result react(const RequestDesignID& msg);
    sc::result react(const CheckTurnEndConditions& c);

    std::string m_save_filename;

    SERVER_ACCESSOR
};


/** The default substate of WaitingForTurnEnd. */
struct WaitingForTurnEndIdle : sc::state<WaitingForTurnEndIdle, WaitingForTurnEnd> {
    typedef boost::mpl::list<
        sc::custom_reaction<SaveGameRequest>
    > reactions;

    WaitingForTurnEndIdle(my_context c);
    ~WaitingForTurnEndIdle();

    sc::result react(const SaveGameRequest& msg);

    SERVER_ACCESSOR
};


/** The substate of WaitingForTurnEnd in which a player has initiated a save
  * and the server is waiting for all players to send their save data, after
  * which the server will actually preform the save. */
struct WaitingForSaveData : sc::state<WaitingForSaveData, WaitingForTurnEnd> {
    typedef boost::mpl::list<
        sc::custom_reaction<ClientSaveData>,
        sc::deferral<SaveGameRequest>,
        sc::deferral<TurnOrders>,
        sc::deferral<PlayerChat>,
        sc::deferral<Diplomacy>
    > reactions;

    WaitingForSaveData(my_context c);
    ~WaitingForSaveData();

    sc::result react(const ClientSaveData& msg);

    std::set<int>                   m_needed_reponses;
    std::set<int>                   m_players_responded;
    std::vector<PlayerSaveGameData> m_player_save_game_data;

    SERVER_ACCESSOR
};


/** The substate of PlayingGame in which the server has received turn orders
  * from players and is determining what happens between turns.  This includes
  * executing orders, resolving combat, various steps in determining what
  * happens before and after combats occur, and updating players on changes in
  * the Universe. */
struct ProcessingTurn : sc::state<ProcessingTurn, PlayingGame> {
    typedef boost::mpl::list<
        sc::custom_reaction<ProcessTurn>,
        sc::deferral<SaveGameRequest>,
        sc::deferral<TurnOrders>,
        sc::deferral<Diplomacy>,
        sc::custom_reaction<CheckTurnEndConditions>
    > reactions;

    ProcessingTurn(my_context c);
    ~ProcessingTurn();

    sc::result react(const ProcessTurn& u);
    sc::result react(const CheckTurnEndConditions& c);

    SERVER_ACCESSOR
};

/** The server state in which the server is shutting down and waiting for AIs to exit. */
struct ShuttingDownServer : sc::state<ShuttingDownServer, ServerFSM> {
    using Clock = std::chrono::high_resolution_clock;
    typedef boost::mpl::list<
        sc::in_state_reaction<Disconnection>,
        sc::custom_reaction<LeaveGame>,
        sc::custom_reaction<CheckEndConditions>,
        sc::custom_reaction<DisconnectClients>,
        sc::custom_reaction<Error>
    > reactions;

    ShuttingDownServer(my_context c);
    ~ShuttingDownServer();

    sc::result react(const LeaveGame& msg);
    sc::result react(const CheckEndConditions& u);
    sc::result react(const DisconnectClients& u);
    sc::result react(const Disconnection& d);
    sc::result react(const Error& msg);

    std::unordered_set<int>            m_player_id_ack_expected;
    boost::asio::high_resolution_timer m_timeout;

    SERVER_ACCESSOR
};

#undef SERVER_ACCESSOR

#endif
