// -*- C++ -*-
#ifndef _ServerFSM_h_
#define _ServerFSM_h_

#include <boost/mpl/list.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>


class Message;
class PlayerConnection;
class ServerApp;
typedef boost::shared_ptr<PlayerConnection> PlayerConnectionPtr;

// Non-Message events
struct Disconnection : boost::statechart::event<Disconnection>
{
    Disconnection(PlayerConnectionPtr& player_connection);

    PlayerConnectionPtr& m_player_connection;
};


//  Message events
/** The base class for all state machine events that are based on Messages. */
struct MessageEventBase
{
    /** Basic ctor. */
    MessageEventBase(const Message& message, PlayerConnectionPtr& player_connection);

    const Message&       m_message;
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
        (LoadSPGame)                            \
        (TurnOrders)                            \
        (ClientSaveData)                        \
        (EndGame)                               \
        (RequestObjectID)                       \
        (PlayerChat)


#define DECLARE_MESSAGE_EVENT(r, data, name)                            \
    struct name :                                                       \
        boost::statechart::event<name>,                                 \
        MessageEventBase                                                \
    {                                                                   \
        name(const Message& message, PlayerConnectionPtr& player_connection) : \
            MessageEventBase(message, player_connection)                \
            {}                                                          \
    };

BOOST_PP_SEQ_FOR_EACH(DECLARE_MESSAGE_EVENT, _, MESSAGE_EVENTS)

#undef DECLARE_MESSAGE_EVENT


// Top-level server states
struct Idle;
struct MPLobby;
struct WaitingForJoiners;
struct WaitingForTurnEnd;
struct WaitingForSaveData;


/** The finite state machine that represents the server's operation. */
struct ServerFSM : boost::statechart::state_machine<ServerFSM, Idle>
{
    ServerFSM(ServerApp &server);

    ServerApp& Server();

private:
    ServerApp& m_server;
};


/** The server's initial state. */
struct Idle : boost::statechart::simple_state<Idle, ServerFSM>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostMPGame>,
        boost::statechart::custom_reaction<HostSPGame>
    > reactions;

    Idle();
    ~Idle();

    boost::statechart::result react(const HostMPGame& msg);
    boost::statechart::result react(const HostSPGame& msg);
};


/** The server state in which the multiplayer lobby is active. */
struct MPLobby : boost::statechart::simple_state<MPLobby, ServerFSM>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<JoinGame>,
        boost::statechart::custom_reaction<LobbyUpdate>,
        boost::statechart::custom_reaction<LobbyChat>,
        boost::statechart::custom_reaction<LobbyHostAbort>,
        boost::statechart::custom_reaction<LobbyNonHostExit>,
        boost::statechart::custom_reaction<StartMPGame>
    > reactions;

    MPLobby();
    ~MPLobby();

    boost::statechart::result react(const Disconnection&);
    boost::statechart::result react(const JoinGame& msg);
    boost::statechart::result react(const LobbyUpdate& msg);
    boost::statechart::result react(const LobbyChat& msg);
    boost::statechart::result react(const LobbyHostAbort& msg);
    boost::statechart::result react(const LobbyNonHostExit& msg);
    boost::statechart::result react(const StartMPGame& msg);
};


/** The server state in which a game has been initiated, and the server is waiting for all palyers to join. */
struct WaitingForJoiners : boost::statechart::simple_state<WaitingForJoiners, ServerFSM>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<JoinGame>
    > reactions;

    WaitingForJoiners();
    ~WaitingForJoiners();

    boost::statechart::result react(const Disconnection&);
    boost::statechart::result react(const JoinGame& msg);
};


/** The server state in which a game is being played, and the server is waiting for all players to finish their moves,
    after which the server will process the turn.*/
struct WaitingForTurnEnd : boost::statechart::simple_state<WaitingForTurnEnd, ServerFSM>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<SaveGameRequest>,
        boost::statechart::custom_reaction<LoadSPGame>,
        boost::statechart::custom_reaction<TurnOrders>,
        boost::statechart::custom_reaction<RequestObjectID>,
        boost::statechart::custom_reaction<PlayerChat>,
        boost::statechart::custom_reaction<EndGame>
    > reactions;

    WaitingForTurnEnd();
    ~WaitingForTurnEnd();

    boost::statechart::result react(const Disconnection&);
    boost::statechart::result react(const SaveGameRequest& msg);
    boost::statechart::result react(const LoadSPGame& msg);
    boost::statechart::result react(const TurnOrders& msg);
    boost::statechart::result react(const RequestObjectID& msg);
    boost::statechart::result react(const PlayerChat& msg);
    boost::statechart::result react(const EndGame& msg);
};


/** The server state in which a game is being played, a player has initiated a save, and the server is waiting for all
    players to send their save data, after which the server will save the data. */
struct WaitingForSaveData : boost::statechart::simple_state<WaitingForSaveData, ServerFSM>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<ClientSaveData>,
        boost::statechart::custom_reaction<SaveGameRequest>,
        boost::statechart::custom_reaction<LoadSPGame>,
        boost::statechart::custom_reaction<TurnOrders>,
        boost::statechart::custom_reaction<RequestObjectID>,
        boost::statechart::custom_reaction<PlayerChat>,
        boost::statechart::custom_reaction<EndGame>
    > reactions;

    // TODO: Catch all other types of messages, but defer them. Actually, test first if it is possible to let unhandled events fall through to the WaitingForTurnEnd state.

    WaitingForSaveData();
    ~WaitingForSaveData();

    boost::statechart::result react(const Disconnection&);
    boost::statechart::result react(const ClientSaveData& msg);
    boost::statechart::result react(const SaveGameRequest& msg);
    boost::statechart::result react(const LoadSPGame& msg);
    boost::statechart::result react(const TurnOrders& msg);
    boost::statechart::result react(const RequestObjectID& msg);
    boost::statechart::result react(const PlayerChat& msg);
    boost::statechart::result react(const EndGame& msg);
};

#endif
