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
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>

#include <set>
#include <vector>


struct MultiplayerLobbyData;
class ServerApp;
struct SinglePlayerSetupData;
class PlayerConnection;
class PlayerSaveGameData;
class ServerSaveGameData;
class System;
typedef boost::shared_ptr<PlayerConnection> PlayerConnectionPtr;

// Non-Message events
struct Disconnection : boost::statechart::event<Disconnection>
{
    Disconnection(PlayerConnectionPtr& player_connection);

    PlayerConnectionPtr& m_player_connection;
};

struct CheckStartConditions : boost::statechart::event<CheckStartConditions>
{};

struct ResolveCombat : boost::statechart::event<ResolveCombat>
{
    ResolveCombat(System* system);

    System* const m_system;
};

// TODO: For prototyping only.
struct CombatComplete : boost::statechart::event<CombatComplete>
{};


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
        (HostMPGame)                            \
        (HostSPGame)                            \
        (StartMPGame)                           \
        (LobbyUpdate)                           \
        (LobbyChat)                             \
        (LobbyHostAbort)                        \
        (LobbyNonHostExit)                      \
        (JoinGame)                              \
        (SaveGameRequest)                       \
        (TurnOrders)                            \
        (ClientSaveData)                        \
        (RequestObjectID)                       \
        (RequestDesignID)                       \
        (PlayerChat)


#define DECLARE_MESSAGE_EVENT(r, data, name)                            \
    struct name :                                                       \
        boost::statechart::event<name>,                                 \
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

// Substates of WaitingForTurnEnd
struct WaitingForTurnEndIdle;
struct WaitingForSaveData;
struct ResolvingCombat;


#define SERVER_ACCESSOR private: ServerApp& Server() { return context<ServerFSM>().Server(); }

/** The finite state machine that represents the server's operation. */
struct ServerFSM : boost::statechart::state_machine<ServerFSM, Idle>
{
    typedef boost::statechart::state_machine<ServerFSM, Idle> Base;

    ServerFSM(ServerApp &server);

    void unconsumed_event(const boost::statechart::event_base &event);
    ServerApp& Server();
    void HandleNonLobbyDisconnection(const Disconnection& d);

    boost::shared_ptr<MultiplayerLobbyData>  m_lobby_data;
    boost::shared_ptr<SinglePlayerSetupData> m_setup_data;
    std::vector<PlayerSaveGameData>          m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>    m_server_save_game_data;

private:
    ServerApp& m_server;
};


/** The server's initial state. */
struct Idle : boost::statechart::simple_state<Idle, ServerFSM>
{
    typedef boost::statechart::simple_state<Idle, ServerFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostMPGame>,
        boost::statechart::custom_reaction<HostSPGame>
    > reactions;

    Idle();
    ~Idle();

    boost::statechart::result react(const HostMPGame& msg);
    boost::statechart::result react(const HostSPGame& msg);

    SERVER_ACCESSOR
};


/** The server state in which the multiplayer lobby is active. */
struct MPLobby : boost::statechart::state<MPLobby, ServerFSM>
{
    typedef boost::statechart::state<MPLobby, ServerFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<JoinGame>,
        boost::statechart::custom_reaction<LobbyUpdate>,
        boost::statechart::custom_reaction<LobbyChat>,
        boost::statechart::custom_reaction<LobbyHostAbort>,
        boost::statechart::custom_reaction<LobbyNonHostExit>,
        boost::statechart::custom_reaction<StartMPGame>
    > reactions;

    MPLobby(my_context c);
    ~MPLobby();

    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const JoinGame& msg);
    boost::statechart::result react(const LobbyUpdate& msg);
    boost::statechart::result react(const LobbyChat& msg);
    boost::statechart::result react(const LobbyHostAbort& msg);
    boost::statechart::result react(const LobbyNonHostExit& msg);
    boost::statechart::result react(const StartMPGame& msg);

    boost::shared_ptr<MultiplayerLobbyData> m_lobby_data;
    std::vector<PlayerSaveGameData>         m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>   m_server_save_game_data;

    SERVER_ACCESSOR
};


/** The server state in which a new single-player game has been initiated, and the server is waiting for all players to
    join. */
struct WaitingForSPGameJoiners : boost::statechart::state<WaitingForSPGameJoiners, ServerFSM>
{
    typedef boost::statechart::state<WaitingForSPGameJoiners, ServerFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        boost::statechart::custom_reaction<JoinGame>,
        boost::statechart::custom_reaction<CheckStartConditions>
    > reactions;

    WaitingForSPGameJoiners(my_context c);
    ~WaitingForSPGameJoiners();

    boost::statechart::result react(const JoinGame& msg);
    boost::statechart::result react(const CheckStartConditions& u);

    boost::shared_ptr<SinglePlayerSetupData> m_setup_data;
    std::vector<PlayerSaveGameData>          m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>    m_server_save_game_data;
    std::set<std::string>                    m_expected_ai_player_names;
    int                                      m_num_expected_players;

    SERVER_ACCESSOR
};


/** The server state in which a multiplayer game has been initiated, and the server is waiting for all players to
    join. */
struct WaitingForMPGameJoiners : boost::statechart::state<WaitingForMPGameJoiners, ServerFSM>
{
    typedef boost::statechart::state<WaitingForMPGameJoiners, ServerFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>,
        boost::statechart::custom_reaction<JoinGame>
    > reactions;

    WaitingForMPGameJoiners(my_context c);
    ~WaitingForMPGameJoiners();

    boost::statechart::result react(const JoinGame& msg);

    boost::shared_ptr<MultiplayerLobbyData> m_lobby_data;
    std::vector<PlayerSaveGameData>         m_player_save_game_data;
    boost::shared_ptr<ServerSaveGameData>   m_server_save_game_data;
    std::set<std::string>                   m_expected_ai_player_names;
    int                                     m_num_expected_players;

    SERVER_ACCESSOR
};


/** The server state in which a game has been starts, and is actually being played. */
struct PlayingGame : boost::statechart::simple_state<PlayingGame, ServerFSM, WaitingForTurnEnd>
{
    typedef boost::statechart::simple_state<PlayingGame, ServerFSM, WaitingForTurnEnd> Base;

    typedef boost::mpl::list<
        boost::statechart::in_state_reaction<Disconnection, ServerFSM, &ServerFSM::HandleNonLobbyDisconnection>
    > reactions;

    PlayingGame();
    ~PlayingGame();

    SERVER_ACCESSOR
};


/** The substate of PlayingGame in which players are playing their turns and the server is waiting for all players to
    finish their moves, after which the server will process the turn. */
struct WaitingForTurnEnd : boost::statechart::simple_state<WaitingForTurnEnd, PlayingGame, WaitingForTurnEndIdle>
{
    typedef boost::statechart::simple_state<WaitingForTurnEnd, PlayingGame, WaitingForTurnEndIdle> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostSPGame>,
        boost::statechart::custom_reaction<TurnOrders>,
        boost::statechart::custom_reaction<RequestObjectID>,
        boost::statechart::custom_reaction<RequestDesignID>,
        boost::statechart::custom_reaction<PlayerChat>
    > reactions;

    WaitingForTurnEnd();
    ~WaitingForTurnEnd();

    boost::statechart::result react(const HostSPGame& msg);
    boost::statechart::result react(const TurnOrders& msg);
    boost::statechart::result react(const RequestObjectID& msg);
    boost::statechart::result react(const RequestDesignID& msg);
    boost::statechart::result react(const PlayerChat& msg);

    std::string m_save_filename;
    System* m_combat_location;

    SERVER_ACCESSOR
};


/** The default substate of WaitingForTurnEnd. */
struct WaitingForTurnEndIdle : boost::statechart::simple_state<WaitingForTurnEndIdle, WaitingForTurnEnd>
{
    typedef boost::statechart::simple_state<WaitingForTurnEndIdle, WaitingForTurnEnd> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<ResolveCombat>,
        boost::statechart::custom_reaction<SaveGameRequest>
    > reactions;

    WaitingForTurnEndIdle();
    ~WaitingForTurnEndIdle();

    boost::statechart::result react(const ResolveCombat& r);
    boost::statechart::result react(const SaveGameRequest& msg);

    SERVER_ACCESSOR
};


/** The substate of WaitingForTurnEnd in which a player has initiated a save and the server is waiting for all
    players to send their save data, after which the server will actually preform the save. */
struct WaitingForSaveData : boost::statechart::state<WaitingForSaveData, WaitingForTurnEnd>
{
    typedef boost::statechart::state<WaitingForSaveData, WaitingForTurnEnd> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<ClientSaveData>,
        boost::statechart::deferral<SaveGameRequest>,
        boost::statechart::deferral<HostSPGame>,
        boost::statechart::deferral<TurnOrders>,
        boost::statechart::deferral<PlayerChat>
    > reactions;

    WaitingForSaveData(my_context c);
    ~WaitingForSaveData();

    boost::statechart::result react(const ClientSaveData& msg);

    std::set<int>                   m_needed_reponses;
    std::set<int>                   m_players_responded;
    std::vector<PlayerSaveGameData> m_player_save_game_data;

    SERVER_ACCESSOR
};

struct ResolvingCombat : boost::statechart::state<ResolvingCombat, WaitingForTurnEnd>
{
    typedef boost::statechart::state<ResolvingCombat, WaitingForTurnEnd> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<CombatComplete>
    > reactions;

    ResolvingCombat(my_context c);
    ~ResolvingCombat();

    boost::statechart::result react(const CombatComplete& msg);

    SERVER_ACCESSOR
};

#undef SERVER_ACCESSOR

#endif
