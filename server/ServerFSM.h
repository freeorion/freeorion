// -*- C++ -*-
#ifndef _ServerFSM_h_
#define _ServerFSM_h_

#include "../network/Message.h"

#include <boost/shared_ptr.hpp>
#include <boost/mpl/list.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deferral.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/in_state_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>

#include <set>
#include <vector>


struct MultiplayerLobbyData;
class ServerApp;
struct SinglePlayerSetupData;
class PlayerConnection;
struct PlayerSaveGameData;
struct ServerSaveGameData;
class System;
typedef boost::shared_ptr<PlayerConnection> PlayerConnectionPtr;

namespace sc = boost::statechart;

// Non-Message events
struct Disconnection : sc::event<Disconnection>
{
    Disconnection(PlayerConnectionPtr& player_connection);
    PlayerConnectionPtr& m_player_connection;
};

struct CheckStartConditions : sc::event<CheckStartConditions>            {};
struct ProcessTurn : sc::event<ProcessTurn>                              {};

struct ResolveCombat : sc::event<ResolveCombat>
{
    ResolveCombat(System* system, std::set<int>& empire_ids);

    System* const m_system;
    std::set<int> m_empire_ids;
};

// TODO: For prototyping only.
struct CombatComplete : sc::event<CombatComplete>                        {};

//  Message events
/** The base class for all state machine events that are based on Messages. */
struct MessageEventBase
{
    MessageEventBase(Message& message, PlayerConnectionPtr& player_connection); ///< Basic ctor.

    Message              m_message;
    PlayerConnectionPtr& m_player_connection;
};

// Define Boost.Preprocessor list of all Message events
#define MESSAGE_EVENTS                          \
    (HostMPGame)                                \
        (HostSPGame)                            \
        (StartMPGame)                           \
        (LobbyUpdate)                           \
        (LobbyChat)                             \
        (LobbyHostAbort)                        \
        (LobbyNonHostExit)                      \
        (JoinGame)                              \
        (SaveGameRequest)                       \
        (TurnOrders)                            \
        (CombatTurnOrders)                      \
        (ClientSaveData)                        \
        (RequestObjectID)                       \
        (RequestDesignID)                       \
        (PlayerChat)


#define DECLARE_MESSAGE_EVENT(r, data, name)                            \
    struct name :                                                       \
        sc::event<name>,                                 \
        MessageEventBase                                                \
    {                                                                   \
        name(Message& message, PlayerConnectionPtr& player_connection) : \
            MessageEventBase(message, player_connection)                \
            {}                                                          \
    };

BOOST_PP_SEQ_FOR_EACH(DECLARE_MESSAGE_EVENT, _, MESSAGE_EVENTS)

#undef DECLARE_MESSAGE_EVENT


// Top-level server states
struct Idle;
struct MPLobby;
struct WaitingForJoiners;
struct PlayingGame;

// Substates of PlayingGame
struct WaitingForTurnEnd;
struct ProcessingTurn;

// Substates of WaitingForTurnEnd
struct WaitingForTurnEndIdle;
struct WaitingForSaveData;

// Substates of ProcessingTurn
struct ProcessingTurnIdle;
struct ResolvingCombat;


#define SERVER_ACCESSOR private: ServerApp& Server() { return context<ServerFSM>().Server(); }

/** The finite state machine that represents the server's operation. */
struct ServerFSM : sc::state_machine<ServerFSM, Idle>
{
    ServerFSM(ServerApp &server);

    void unconsumed_event(const sc::event_base &event);
    ServerApp& Server();
    void HandleNonLobbyDisconnection(const Disconnection& d);

    boost::shared_ptr<MultiplayerLobbyData>     m_lobby_data;
    boost::shared_ptr<SinglePlayerSetupData>    m_single_player_setup_data;
    std::vector<PlayerSaveGameData>             m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>       m_server_save_game_data;

private:
    ServerApp& m_server;
};


/** The server's initial state. */
struct Idle : sc::state<Idle, ServerFSM>
{
    typedef boost::mpl::list<
        sc::custom_reaction<HostMPGame>,
        sc::custom_reaction<HostSPGame>
    > reactions;

    Idle(my_context c);
    ~Idle();

    sc::result react(const HostMPGame& msg);
    sc::result react(const HostSPGame& msg);

    SERVER_ACCESSOR
};


/** The server state in which the multiplayer lobby is active. */
struct MPLobby : sc::state<MPLobby, ServerFSM>
{
    typedef boost::mpl::list<
        sc::custom_reaction<Disconnection>,
        sc::custom_reaction<JoinGame>,
        sc::custom_reaction<LobbyUpdate>,
        sc::custom_reaction<LobbyChat>,
        sc::custom_reaction<LobbyHostAbort>,
        sc::custom_reaction<LobbyNonHostExit>,
        sc::custom_reaction<StartMPGame>
    > reactions;

    MPLobby(my_context c);
    ~MPLobby();

    sc::result react(const Disconnection& d);
    sc::result react(const JoinGame& msg);
    sc::result react(const LobbyUpdate& msg);
    sc::result react(const LobbyChat& msg);
    sc::result react(const LobbyHostAbort& msg);
    sc::result react(const LobbyNonHostExit& msg);
    sc::result react(const StartMPGame& msg);

    boost::shared_ptr<MultiplayerLobbyData> m_lobby_data;
    std::vector<PlayerSaveGameData>         m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>   m_server_save_game_data;

    SERVER_ACCESSOR
};


/** The server state in which a new single-player game has been initiated, and
  * the server is waiting for all players to join. */
struct WaitingForSPGameJoiners : sc::state<WaitingForSPGameJoiners, ServerFSM>
{
    typedef boost::mpl::list<
        sc::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        sc::custom_reaction<JoinGame>,
        sc::custom_reaction<CheckStartConditions>
    > reactions;

    WaitingForSPGameJoiners(my_context c);
    ~WaitingForSPGameJoiners();

    sc::result react(const JoinGame& msg);
    sc::result react(const CheckStartConditions& u);

    boost::shared_ptr<SinglePlayerSetupData> m_single_player_setup_data;
    std::vector<PlayerSaveGameData>          m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>    m_server_save_game_data;
    std::set<std::string>                    m_expected_ai_player_names;
    int                                      m_num_expected_players;

    SERVER_ACCESSOR
};


/** The server state in which a multiplayer game has been initiated, and the
  * server is waiting for all players to join. */
struct WaitingForMPGameJoiners : sc::state<WaitingForMPGameJoiners, ServerFSM>
{
    typedef boost::mpl::list<
        sc::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        sc::custom_reaction<JoinGame>
    > reactions;

    WaitingForMPGameJoiners(my_context c);
    ~WaitingForMPGameJoiners();

    sc::result react(const JoinGame& msg);

    boost::shared_ptr<MultiplayerLobbyData> m_lobby_data;
    std::vector<PlayerSaveGameData>         m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>   m_server_save_game_data;
    std::set<std::string>                   m_expected_ai_player_names;
    int                                     m_num_expected_players;

    SERVER_ACCESSOR
};


/** The server state in which a game has been started, and is actually being
  * played. */
struct PlayingGame : sc::state<PlayingGame, ServerFSM, WaitingForTurnEnd>
{
    typedef boost::mpl::list<
        sc::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        sc::custom_reaction<PlayerChat>
    > reactions;

    PlayingGame(my_context c);
    ~PlayingGame();

    sc::result react(const PlayerChat& msg);

    SERVER_ACCESSOR
};


/** The substate of PlayingGame in which players are playing their turns and
  * the server is waiting for all players to finish their moves, after which
  * the server will process the turn. */
struct WaitingForTurnEnd : sc::state<WaitingForTurnEnd, PlayingGame, WaitingForTurnEndIdle>
{
    typedef boost::mpl::list<
        sc::custom_reaction<HostSPGame>,
        sc::custom_reaction<TurnOrders>,
        sc::custom_reaction<RequestObjectID>,
        sc::custom_reaction<RequestDesignID>
    > reactions;

    WaitingForTurnEnd(my_context c);
    ~WaitingForTurnEnd();

    sc::result react(const HostSPGame& msg);
    sc::result react(const TurnOrders& msg);
    sc::result react(const RequestObjectID& msg);
    sc::result react(const RequestDesignID& msg);

    std::string m_save_filename;

    SERVER_ACCESSOR
};


/** The default substate of WaitingForTurnEnd. */
struct WaitingForTurnEndIdle : sc::state<WaitingForTurnEndIdle, WaitingForTurnEnd>
{
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
struct WaitingForSaveData : sc::state<WaitingForSaveData, WaitingForTurnEnd>
{
    typedef boost::mpl::list<
        sc::custom_reaction<ClientSaveData>,
        sc::deferral<SaveGameRequest>,
        sc::deferral<HostSPGame>,
        sc::deferral<TurnOrders>,
        sc::deferral<PlayerChat>
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
struct ProcessingTurn : sc::state<ProcessingTurn, PlayingGame, ProcessingTurnIdle>
{
    typedef boost::mpl::list<
        sc::custom_reaction<ProcessTurn>,
        sc::deferral<SaveGameRequest>,
        sc::deferral<TurnOrders>
    > reactions;

    ProcessingTurn(my_context c);
    ~ProcessingTurn();

    sc::result react(const ProcessTurn& u);

    System* m_combat_system;
    std::set<int> m_combat_empire_ids;

    SERVER_ACCESSOR
};


/** The default substate of ProcessingTurn. */
struct ProcessingTurnIdle : sc::state<ProcessingTurnIdle, ProcessingTurn>
{
    typedef boost::mpl::list<
        sc::custom_reaction<ResolveCombat>
    > reactions;

    ProcessingTurnIdle(my_context c);
    ~ProcessingTurnIdle();

    sc::result react(const ResolveCombat& u);

    SERVER_ACCESSOR
};


/** The substate of ProcessingTurn in which a single combat is resolved. */
struct ResolvingCombat : sc::state<ResolvingCombat, ProcessingTurn>
{
    typedef boost::mpl::list<
        sc::custom_reaction<CombatComplete>,
        sc::custom_reaction<CombatTurnOrders>
    > reactions;

    ResolvingCombat(my_context c);
    ~ResolvingCombat();

    sc::result react(const CombatComplete& msg);
    sc::result react(const CombatTurnOrders& msg);

    SERVER_ACCESSOR
};

#undef SERVER_ACCESSOR

#endif
