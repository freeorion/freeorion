#ifndef _HumanClientFSM_h_
#define _HumanClientFSM_h_

#include "../ClientFSMEvents.h"
#include "../../util/Logger.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deferral.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>

#include <chrono>

// Human client-specific events not already defined in ClientFSMEvents.h

/** Indicates that the "Start Game" button was clicked in the MP Lobby UI, in
  * host player mode. */
struct StartMPGameClicked : boost::statechart::event<StartMPGameClicked> {};

// Indicates that the "Cancel" button was clicked in the MP Lobby UI.
struct CancelMPGameClicked : boost::statechart::event<CancelMPGameClicked> {};

// Indicates that an SP-host request was sent to the server.
struct HostSPGameRequested : boost::statechart::event<HostSPGameRequested> {};

// Indicates that a MP-host request was sent to the server.
struct HostMPGameRequested : boost::statechart::event<HostMPGameRequested> {};

// Indicates that a MP-join request was sent to the server.
struct JoinMPGameRequested : boost::statechart::event<JoinMPGameRequested> {};

// Indicates that the player's turn has been sent to the server.
struct TurnEnded : boost::statechart::event<TurnEnded> {};

// Posted to advance the turn, including when auto-advancing the first turn
struct AdvanceTurn : boost::statechart::event<AdvanceTurn> {};

/** The set of events used by the QuittingGame state. */
class Process;
struct StartQuittingGame : boost::statechart::event<StartQuittingGame> {
    StartQuittingGame(Process& server_, std::function<void()>&& after_server_shutdown_action_) :
        m_server(server_), m_after_server_shutdown_action(std::move(after_server_shutdown_action_))
    {}

    Process& m_server;
    /// An action to be completed after disconnecting from the server
    std::function<void()> m_after_server_shutdown_action;
};

struct ShutdownServer : boost::statechart::event<ShutdownServer> {};
struct WaitForDisconnect : boost::statechart::event<WaitForDisconnect> {};
struct TerminateServer : boost::statechart::event<TerminateServer> {};

/** This is a Boost.Preprocessor list of all the events above.  As new events
  * are added above, they should be added to this list as well. */
#define HUMAN_CLIENT_FSM_EVENTS                                 \
    (StartMPGameClicked)                                        \
    (CancelMPGameClicked)                                       \
    (HostSPGameRequested)                                       \
    (HostMPGameRequested)                                       \
    (JoinMPGameRequested)                                       \
    (TurnEnded)                                                 \
    (StartQuittingGame)                                         \
    (ShutdownServer)                                            \
    (WaitForDisconnect)                                         \
    (TerminateServer)

// Top-level human client states
struct IntroMenu;
struct PlayingGame;
struct WaitingForSPHostAck;
struct WaitingForMPHostAck;
struct WaitingForMPJoinAck;
struct MPLobby;
struct QuittingGame;

// Substates of PlayingGame
struct WaitingForGameStart;
struct WaitingForTurnData;
struct PlayingTurn;

class HumanClientApp;
class IntroScreen;
class MultiPlayerLobbyWnd;

#define CLIENT_ACCESSOR private: HumanClientApp& Client() { return context<HumanClientFSM>().m_client; }


/** The finite state machine that represents the human client's operation. */
struct HumanClientFSM : boost::statechart::state_machine<HumanClientFSM, IntroMenu> {
    typedef boost::statechart::state_machine<HumanClientFSM, IntroMenu> Base;

    HumanClientFSM(HumanClientApp &human_client);

    void unconsumed_event(const boost::statechart::event_base &event);

    using Base::post_event;

    HumanClientApp&    m_client;
};


/** The human client's initial state. */
struct IntroMenu : boost::statechart::state<IntroMenu, HumanClientFSM> {
    typedef boost::statechart::state<IntroMenu, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostSPGameRequested>,
        boost::statechart::custom_reaction<HostMPGameRequested>,
        boost::statechart::custom_reaction<JoinMPGameRequested>,
        boost::statechart::custom_reaction<StartQuittingGame>,
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<EndGame>
    > reactions;

    IntroMenu(my_context ctx);
    ~IntroMenu();

    boost::statechart::result react(const HostSPGameRequested& a);
    boost::statechart::result react(const HostMPGameRequested& a);
    boost::statechart::result react(const JoinMPGameRequested& a);
    boost::statechart::result react(const StartQuittingGame& msg);
    boost::statechart::result react(const EndGame&);
    boost::statechart::result react(const Disconnection&);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to host a single
  * player game and is waiting for the server to acknowledge the request. */
struct WaitingForSPHostAck : boost::statechart::simple_state<WaitingForSPHostAck, HumanClientFSM> {
    typedef boost::statechart::simple_state<WaitingForSPHostAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostSPGame>,
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<StartQuittingGame>,
        boost::statechart::custom_reaction<Error>,
        boost::statechart::custom_reaction<CheckSum>
    > reactions;

    WaitingForSPHostAck();
    ~WaitingForSPHostAck();

    boost::statechart::result react(const HostSPGame& a);
    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const StartQuittingGame& msg);
    boost::statechart::result react(const Error& msg);
    boost::statechart::result react(const CheckSum& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to host a
  * multiplayer game and is waiting for the server to acknowledge the request. */
struct WaitingForMPHostAck : boost::statechart::simple_state<WaitingForMPHostAck, HumanClientFSM> {
    typedef boost::statechart::simple_state<WaitingForMPHostAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostMPGame>,
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<StartQuittingGame>,
        boost::statechart::custom_reaction<Error>,
        boost::statechart::custom_reaction<CheckSum>
    > reactions;

    WaitingForMPHostAck();
    ~WaitingForMPHostAck();

    boost::statechart::result react(const HostMPGame& a);
    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const StartQuittingGame& msg);
    boost::statechart::result react(const Error& msg);
    boost::statechart::result react(const CheckSum& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to join a
  * single-player game and is waiting for the server to acknowledge the
  * player's join. */
struct WaitingForMPJoinAck : boost::statechart::simple_state<WaitingForMPJoinAck, HumanClientFSM> {
    typedef boost::statechart::simple_state<WaitingForMPJoinAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<JoinGame>,
        boost::statechart::custom_reaction<AuthRequest>,
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<StartQuittingGame>,
        boost::statechart::custom_reaction<CancelMPGameClicked>,
        boost::statechart::custom_reaction<Error>
    > reactions;

    WaitingForMPJoinAck();
    ~WaitingForMPJoinAck();

    boost::statechart::result react(const JoinGame& a);
    boost::statechart::result react(const AuthRequest& a);
    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const StartQuittingGame& msg);
    boost::statechart::result react(const CancelMPGameClicked& msg);
    boost::statechart::result react(const Error& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which the multiplayer lobby is active. */
struct MPLobby : boost::statechart::state<MPLobby, HumanClientFSM> {
    typedef boost::statechart::state<MPLobby, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<HostID>,
        boost::statechart::custom_reaction<LobbyUpdate>,
        boost::statechart::custom_reaction<PlayerChat>,
        boost::statechart::custom_reaction<CancelMPGameClicked>,
        boost::statechart::custom_reaction<StartMPGameClicked>,
        boost::statechart::custom_reaction<GameStart>,
        boost::statechart::custom_reaction<StartQuittingGame>,
        boost::statechart::custom_reaction<Error>,
        boost::statechart::custom_reaction<CheckSum>,
        boost::statechart::custom_reaction<ChatHistory>,
        boost::statechart::custom_reaction<PlayerStatus>,
        boost::statechart::custom_reaction<SaveGameComplete>,
        boost::statechart::custom_reaction<TurnProgress>
    > reactions;

    MPLobby(my_context ctx);
    ~MPLobby();

    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const HostID& msg);
    boost::statechart::result react(const LobbyUpdate& msg);
    boost::statechart::result react(const PlayerChat& msg);
    boost::statechart::result react(const CancelMPGameClicked& a);
    boost::statechart::result react(const StartMPGameClicked& a);
    boost::statechart::result react(const GameStart& msg);
    boost::statechart::result react(const StartQuittingGame& msg);
    boost::statechart::result react(const Error& msg);
    boost::statechart::result react(const CheckSum& msg);
    boost::statechart::result react(const ChatHistory& msg);
    boost::statechart::result react(const PlayerStatus& msg);
    boost::statechart::result react(const SaveGameComplete& msg);
    boost::statechart::result react(const TurnProgress& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which a game has been started. */
struct PlayingGame : boost::statechart::state<PlayingGame, HumanClientFSM, WaitingForGameStart> {
    typedef boost::statechart::state<PlayingGame, HumanClientFSM, WaitingForGameStart> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostID>,
        boost::statechart::custom_reaction<PlayerChat>,
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<PlayerStatus>,
        boost::statechart::custom_reaction<Diplomacy>,
        boost::statechart::custom_reaction<DiplomaticStatusUpdate>,
        boost::statechart::custom_reaction<EndGame>,
        boost::statechart::custom_reaction<StartQuittingGame>,
        boost::statechart::custom_reaction<Error>,
        boost::statechart::custom_reaction<TurnProgress>,
        boost::statechart::custom_reaction<TurnPartialUpdate>,
        boost::statechart::custom_reaction<LobbyUpdate>
    > reactions;

    PlayingGame(my_context ctx);
    ~PlayingGame();

    boost::statechart::result react(const HostID& msg);
    boost::statechart::result react(const PlayerChat& msg);
    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const PlayerStatus& msg);
    boost::statechart::result react(const Diplomacy& d);
    boost::statechart::result react(const DiplomaticStatusUpdate& u);
    boost::statechart::result react(const EndGame& msg);
    boost::statechart::result react(const StartQuittingGame& msg);
    boost::statechart::result react(const Error& msg);
    boost::statechart::result react(const TurnProgress& msg);
    boost::statechart::result react(const TurnPartialUpdate& msg);
    boost::statechart::result react(const LobbyUpdate& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which a game has been started but the start of
  * game message hasn't been received yet from the server. */
struct WaitingForGameStart : boost::statechart::state<WaitingForGameStart, PlayingGame> {
    typedef boost::statechart::state<WaitingForGameStart, PlayingGame> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<GameStart>
    > reactions;

    WaitingForGameStart(my_context ctx);
    ~WaitingForGameStart();

    boost::statechart::result react(const GameStart& msg);

    CLIENT_ACCESSOR
};


/** The substate of PlayingGame in which a game is about to start, or the
  * player is waiting for turn resolution and a new turn. */
struct WaitingForTurnData : boost::statechart::state<WaitingForTurnData, PlayingGame> {
    typedef boost::statechart::state<WaitingForTurnData, PlayingGame> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<SaveGameComplete>,
        boost::statechart::custom_reaction<TurnUpdate>,
        boost::statechart::custom_reaction<TurnRevoked>,
        boost::statechart::custom_reaction<DispatchCombatLogs>
    > reactions;

    WaitingForTurnData(my_context ctx);
    ~WaitingForTurnData();

    boost::statechart::result react(const SaveGameComplete& d);
    boost::statechart::result react(const TurnUpdate& msg);
    boost::statechart::result react(const TurnRevoked& msg);
    boost::statechart::result react(const DispatchCombatLogs& msg);

    CLIENT_ACCESSOR
};


/** The substate of PlayingGame in which the player is actively playing a turn. */
struct PlayingTurn : boost::statechart::state<PlayingTurn, PlayingGame> {
    typedef boost::statechart::state<PlayingTurn, PlayingGame> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<SaveGameComplete>,
        boost::statechart::custom_reaction<AdvanceTurn>,
        boost::statechart::custom_reaction<TurnUpdate>,
        boost::statechart::custom_reaction<TurnEnded>,
        boost::statechart::custom_reaction<PlayerStatus>,
        boost::statechart::custom_reaction<DispatchCombatLogs>
    > reactions;

    PlayingTurn(my_context ctx);
    ~PlayingTurn();

    boost::statechart::result react(const SaveGameComplete& d);
    boost::statechart::result react(const AdvanceTurn& d);
    boost::statechart::result react(const TurnUpdate& msg);
    boost::statechart::result react(const TurnEnded& d);
    boost::statechart::result react(const PlayerStatus& msg);
    boost::statechart::result react(const DispatchCombatLogs& msg);

    CLIENT_ACCESSOR
};


/** The state for the orderly shutdown of the server. */
struct QuittingGame : boost::statechart::state<QuittingGame, HumanClientFSM> {
    using Clock = std::chrono::steady_clock;

    using reactions = boost::mpl::list<
        boost::statechart::custom_reaction<StartQuittingGame>,
        boost::statechart::custom_reaction<ShutdownServer>,
        boost::statechart::custom_reaction<WaitForDisconnect>,
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<TerminateServer>
    >;

    QuittingGame(my_context ctx);
    ~QuittingGame();

    boost::statechart::result react(const StartQuittingGame& d);
    boost::statechart::result react(const ShutdownServer& msg);
    boost::statechart::result react(const WaitForDisconnect& msg);
    boost::statechart::result react(const Disconnection& msg);
    boost::statechart::result react(const TerminateServer& msg);

    std::chrono::steady_clock::time_point m_start_time;

    Process* m_server_process = nullptr;
    /// An action to be completed after disconnecting from the server
    std::function<void()> m_after_server_shutdown_action = std::function<void()>();

    CLIENT_ACCESSOR
};


#undef CLIENT_ACCESSOR

#endif
