// -*- C++ -*-
#ifndef _HumanClientFSM_h_
#define _HumanClientFSM_h_

#include "../ClientFSMEvents.h"
#include "../../util/AppInterface.h"

#include <boost/mpl/list.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deferral.hpp>
#include <boost/statechart/in_state_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>

#include <set>
#include <vector>


enum WaitingForDataMode {
    WAITING_FOR_NEW_GAME,
    WAITING_FOR_LOADED_GAME,
    WAITING_FOR_NEW_TURN
};

// This function returns true iff the FSM's state instrumentation should be output to the logger's debug stream.
bool TraceHumanClientFSMExecution();

// Human client-specific events not already defined in ClientFSMEvents.h

// Indicates that the "Start Game" button was clicked in the MP Lobby UI, in host player mode.
struct StartMPGameClicked : boost::statechart::event<StartMPGameClicked> {};

// Indicates that the "Cancel" button was clicked in the MP Lobby UI.
struct CancelMPGameClicked : boost::statechart::event<CancelMPGameClicked> {};

// Indicates that an SP-host request was sent to the server.
struct HostSPGameRequested : boost::statechart::event<HostSPGameRequested>
{
    HostSPGameRequested(WaitingForDataMode waiting_for_data_mode) : m_waiting_for_data_mode(waiting_for_data_mode) {}
    WaitingForDataMode m_waiting_for_data_mode;
};

// Indicates that a MP-host request was sent to the server.
struct HostMPGameRequested : boost::statechart::event<HostMPGameRequested> {};

// Indicates that a MP-join request was sent to the server.
struct JoinMPGameRequested : boost::statechart::event<JoinMPGameRequested> {};

// Indicates that the player's turn has been sent to the server.
struct TurnEnded : boost::statechart::event<TurnEnded> {};

// Indicates that a game has ended and that the state should be reset to IntroMenu.
struct ResetToIntroMenu : boost::statechart::event<ResetToIntroMenu> {};

// This is a Boost.Preprocessor list of all the events above.  As new events are added above, they should be added to
// this list as well.
#define HUMAN_CLIENT_FSM_EVENTS                                    \
    (StartMPGameClicked)                                           \
        (CancelMPGameClicked)                                      \
        (HostSPGameRequested)                                      \
        (HostMPGameRequested)                                      \
        (JoinMPGameRequested)                                      \
        (TurnEnded)                                                \
        (ResetToIntroMenu)

// Top-level human client states
struct IntroMenu;
struct PlayingGame;
struct WaitingForSPHostAck;
struct WaitingForMPHostAck;
struct WaitingForMPJoinAck;

// Substates of IntroMenu
struct IntroMenuIdle;
struct MPLobby;

// Substates of MPLobby
struct MPLobbyIdle;
struct HostMPLobby;
struct NonHostMPLobby;

// Substates of PlayingGame
struct WaitingForTurnData;
struct PlayingTurn;

// Substates of WaitingForTurnData
struct WaitingForTurnDataIdle;
struct ResolvingCombat;


class HumanClientApp;
class CombatWnd;
class IntroScreen;
class TurnProgressWnd;
class MultiplayerLobbyWnd;

#define CLIENT_ACCESSOR private: HumanClientApp& Client() { return context<HumanClientFSM>().m_client; }

// Note that empty idle states are needed for several states.  They are needed when a state has internal states (and so
// because of the requirements of Boost.StateChart, must have an initial internal state), but the state does not
// logically have an initial state that is always correct to use.  for instance, MPLobby must always be in one of its
// internal substates HostMPLobby, or NonHostMPLobby, but it is inappropriate to make either of these be the "default"
// (initial) internal state.  For this reason, a do-nothing internal state MPLobbyIdle is created instead.
#define EMPTY_IDLE_STATE(name)                                          \
    struct name##Idle : boost::statechart::simple_state<name##Idle, name> \
    {                                                                   \
        typedef boost::statechart::simple_state<name##Idle, name> Base; \
        name##Idle() {if (TraceHumanClientFSMExecution()) Logger().debugStream() << "(HumanClientFSM) " #name "Idle"; } \
        ~name##Idle() {if (TraceHumanClientFSMExecution()) Logger().debugStream() << "(HumanClientFSM) ~" #name "Idle";} \
        CLIENT_ACCESSOR                                                 \
    }


/** The finite state machine that represents the human client's operation. */
struct HumanClientFSM : boost::statechart::state_machine<HumanClientFSM, IntroMenu>
{
    typedef boost::statechart::state_machine<HumanClientFSM, IntroMenu> Base;

    HumanClientFSM(HumanClientApp &human_client);

    void unconsumed_event(const boost::statechart::event_base &event);

    HumanClientApp&    m_client;
    WaitingForDataMode m_next_waiting_for_data_mode;
};


/** The human client's initial state. */
struct IntroMenu : boost::statechart::state<IntroMenu, HumanClientFSM, IntroMenuIdle>
{
    typedef boost::statechart::state<IntroMenu, HumanClientFSM, IntroMenuIdle> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostSPGameRequested>,
        boost::statechart::custom_reaction<HostMPGameRequested>,
        boost::statechart::custom_reaction<JoinMPGameRequested>
    > reactions;

    IntroMenu(my_context ctx);
    ~IntroMenu();

    boost::statechart::result react(const HostSPGameRequested& a);
    boost::statechart::result react(const HostMPGameRequested& a);
    boost::statechart::result react(const JoinMPGameRequested& a);

    IntroScreen* m_intro_screen;

    CLIENT_ACCESSOR
};


/** The IntroMenu's initial state. */
EMPTY_IDLE_STATE(IntroMenu);


/** The human client state in which the player has requested to host a single player game and is waiting for the server
    to acknowledge the request. */
struct WaitingForSPHostAck : boost::statechart::simple_state<WaitingForSPHostAck, HumanClientFSM>
{
    typedef boost::statechart::simple_state<WaitingForSPHostAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostSPGame>
    > reactions;

    WaitingForSPHostAck();
    ~WaitingForSPHostAck();

    boost::statechart::result react(const HostSPGame& a);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to host a multiplayer game and is waiting for the server
    to acknowledge the request. */
struct WaitingForMPHostAck : boost::statechart::simple_state<WaitingForMPHostAck, HumanClientFSM>
{
    typedef boost::statechart::simple_state<WaitingForMPHostAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostMPGame>
    > reactions;

    WaitingForMPHostAck();
    ~WaitingForMPHostAck();

    boost::statechart::result react(const HostMPGame& a);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to join a single-player game and is waiting for the server
    to acknowledge the player's join. */
struct WaitingForMPJoinAck : boost::statechart::simple_state<WaitingForMPJoinAck, HumanClientFSM>
{
    typedef boost::statechart::simple_state<WaitingForMPJoinAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<JoinGame>
    > reactions;

    WaitingForMPJoinAck();
    ~WaitingForMPJoinAck();

    boost::statechart::result react(const JoinGame& a);

    CLIENT_ACCESSOR
};


/** The human client state in which the multiplayer lobby is active. */
struct MPLobby : boost::statechart::state<MPLobby, IntroMenu, MPLobbyIdle>
{
    typedef boost::statechart::state<MPLobby, IntroMenu, MPLobbyIdle> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<LobbyUpdate>,
        boost::statechart::custom_reaction<LobbyChat>,
        boost::statechart::custom_reaction<LobbyHostAbort>,
        boost::statechart::custom_reaction<LobbyNonHostExit>,
        boost::statechart::custom_reaction<GameStart>
    > reactions;

    MPLobby(my_context ctx);
    ~MPLobby();

    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const LobbyUpdate& msg);
    boost::statechart::result react(const LobbyChat& msg);
    boost::statechart::result react(const LobbyHostAbort& msg);
    boost::statechart::result react(const LobbyNonHostExit& msg);
    boost::statechart::result react(const GameStart& msg);

    MultiplayerLobbyWnd* m_lobby_wnd;

    CLIENT_ACCESSOR
};


/** The initial substate of MPLobby. */
EMPTY_IDLE_STATE(MPLobby);


/** The multiplayer lobby substate for host player mode. */
struct HostMPLobby : boost::statechart::state<HostMPLobby, MPLobby>
{
    typedef boost::statechart::state<HostMPLobby, MPLobby> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<StartMPGameClicked>,
        boost::statechart::custom_reaction<CancelMPGameClicked>
    > reactions;

    HostMPLobby(my_context ctx);
    ~HostMPLobby();

    boost::statechart::result react(const StartMPGameClicked& a);
    boost::statechart::result react(const CancelMPGameClicked& a);

    CLIENT_ACCESSOR
};


/** The multiplayer lobby substate for non-host player (joiner) mode. */
struct NonHostMPLobby : boost::statechart::state<NonHostMPLobby, MPLobby>
{
    typedef boost::statechart::state<NonHostMPLobby, MPLobby> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<CancelMPGameClicked>
    > reactions;

    NonHostMPLobby(my_context ctx);
    ~NonHostMPLobby();

    boost::statechart::result react(const CancelMPGameClicked& a);

    CLIENT_ACCESSOR
};


/** The human client state in which a game has been started, and a turn is being played. */
struct PlayingGame : boost::statechart::simple_state<PlayingGame, HumanClientFSM, WaitingForTurnData>
{
    typedef boost::statechart::simple_state<PlayingGame, HumanClientFSM, WaitingForTurnData> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<PlayerEliminated>,
        boost::statechart::custom_reaction<PlayerExit>,
        boost::statechart::custom_reaction<EndGame>,
        boost::statechart::custom_reaction<ResetToIntroMenu>
    > reactions;

    PlayingGame();
    ~PlayingGame();

    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const PlayerEliminated& msg);
    boost::statechart::result react(const PlayerExit& msg);
    boost::statechart::result react(const EndGame& msg);
    boost::statechart::result react(const ResetToIntroMenu& msg);

    CLIENT_ACCESSOR
};


/** The substate of PlayingGame in which a game is about to start, or the player is waiting for turn resolution and a
    new turn. */
struct WaitingForTurnData : boost::statechart::state<WaitingForTurnData, PlayingGame, WaitingForTurnDataIdle>
{
    typedef boost::statechart::state<WaitingForTurnData, PlayingGame, WaitingForTurnDataIdle> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<TurnProgress>,
        boost::statechart::custom_reaction<TurnUpdate>,
        boost::statechart::custom_reaction<CombatStart>,
        boost::statechart::custom_reaction<GameStart>,
        boost::statechart::deferral<PlayerEliminated>,
        boost::statechart::deferral<SaveGame>,
        boost::statechart::deferral<PlayerChat>
    > reactions;

    WaitingForTurnData(my_context ctx);
    ~WaitingForTurnData();

    boost::statechart::result react(const TurnProgress& msg);
    boost::statechart::result react(const TurnUpdate& msg);
    boost::statechart::result react(const CombatStart& msg);
    boost::statechart::result react(const GameStart& msg);

    TurnProgressWnd* m_turn_progress_wnd;

    CLIENT_ACCESSOR
};


/** The initial substate of WaitingForTurnData. */
EMPTY_IDLE_STATE(WaitingForTurnData);


/** The substate of PlayingGame in which the player is actively playing a turn. */
struct PlayingTurn : boost::statechart::state<PlayingTurn, PlayingGame>
{
    typedef boost::statechart::state<PlayingTurn, PlayingGame> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<SaveGame>,
        boost::statechart::custom_reaction<TurnEnded>,
        boost::statechart::custom_reaction<PlayerChat>
    > reactions;

    PlayingTurn(my_context ctx);
    ~PlayingTurn();

    boost::statechart::result react(const SaveGame& d);
    boost::statechart::result react(const TurnEnded& d);
    boost::statechart::result react(const PlayerChat& msg);

    CLIENT_ACCESSOR
};


/** The substate of WaitingForTurnData in which the player is resolving a combat. */
struct ResolvingCombat : boost::statechart::state<ResolvingCombat, WaitingForTurnData>
{
    typedef boost::statechart::state<ResolvingCombat, WaitingForTurnData> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<CombatStart>,
        boost::statechart::custom_reaction<CombatRoundUpdate>,
        boost::statechart::custom_reaction<CombatEnd>,
        boost::statechart::deferral<PlayerChat>
    > reactions;

    ResolvingCombat(my_context ctx);
    ~ResolvingCombat();

    boost::statechart::result react(const CombatStart& msg);
    boost::statechart::result react(const CombatRoundUpdate& msg);
    boost::statechart::result react(const CombatEnd& msg);

    CombatWnd* m_combat_wnd;

    CLIENT_ACCESSOR
};

#undef CLIENT_ACCESSOR
#undef EMPTY_IDLE_STATE

#endif
