// -*- C++ -*-
#ifndef _HumanClientFSM_h_
#define _HumanClientFSM_h_

#include "../ClientFSMEvents.h"
#include "../../util/Logger.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deferral.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>


/** This function returns true iff the FSM's state instrumentation should be
  * output to the logger's debug stream. */
bool TraceHumanClientFSMExecution();

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

/** Indicates that a game has ended and that the state should be reset to
  * IntroMenu. */
struct ResetToIntroMenu : boost::statechart::event<ResetToIntroMenu> {};

// Posted by PlayingTurn's ctor when --auto-advance-first-turn is in use.
struct AutoAdvanceFirstTurn : boost::statechart::event<AutoAdvanceFirstTurn> {};

/** This is a Boost.Preprocessor list of all the events above.  As new events
  * are added above, they should be added to this list as well. */
#define HUMAN_CLIENT_FSM_EVENTS                                 \
    (StartMPGameClicked)                                        \
    (CancelMPGameClicked)                                       \
    (HostSPGameRequested)                                       \
    (HostMPGameRequested)                                       \
    (JoinMPGameRequested)                                       \
    (TurnEnded)                                                 \
    (ResetToIntroMenu)

// Top-level human client states
struct IntroMenu;
struct PlayingGame;
struct WaitingForSPHostAck;
struct WaitingForMPHostAck;
struct WaitingForMPJoinAck;
struct MPLobby;

// Substates of PlayingGame
struct WaitingForGameStart;
struct WaitingForTurnData;
struct PlayingTurn;
struct ResolvingCombat;


class HumanClientApp;
class CombatWnd;
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
        boost::statechart::custom_reaction<JoinMPGameRequested>
    > reactions;

    IntroMenu(my_context ctx);
    ~IntroMenu();

    boost::statechart::result react(const HostSPGameRequested& a);
    boost::statechart::result react(const HostMPGameRequested& a);
    boost::statechart::result react(const JoinMPGameRequested& a);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to host a single
  * player game and is waiting for the server to acknowledge the request. */
struct WaitingForSPHostAck : boost::statechart::simple_state<WaitingForSPHostAck, HumanClientFSM> {
    typedef boost::statechart::simple_state<WaitingForSPHostAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostSPGame>,
        boost::statechart::custom_reaction<Error>
    > reactions;

    WaitingForSPHostAck();
    ~WaitingForSPHostAck();

    boost::statechart::result react(const HostSPGame& a);
    boost::statechart::result react(const Error& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to host a
  * multiplayer game and is waiting for the server to acknowledge the request. */
struct WaitingForMPHostAck : boost::statechart::simple_state<WaitingForMPHostAck, HumanClientFSM> {
    typedef boost::statechart::simple_state<WaitingForMPHostAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostMPGame>,
        boost::statechart::custom_reaction<Error>
    > reactions;

    WaitingForMPHostAck();
    ~WaitingForMPHostAck();

    boost::statechart::result react(const HostMPGame& a);
    boost::statechart::result react(const Error& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which the player has requested to join a
  * single-player game and is waiting for the server to acknowledge the
  * player's join. */
struct WaitingForMPJoinAck : boost::statechart::simple_state<WaitingForMPJoinAck, HumanClientFSM> {
    typedef boost::statechart::simple_state<WaitingForMPJoinAck, HumanClientFSM> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<JoinGame>,
        boost::statechart::custom_reaction<Error>
    > reactions;

    WaitingForMPJoinAck();
    ~WaitingForMPJoinAck();

    boost::statechart::result react(const JoinGame& a);
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
        boost::statechart::custom_reaction<LobbyChat>,
        boost::statechart::custom_reaction<CancelMPGameClicked>,
        boost::statechart::custom_reaction<StartMPGameClicked>,
        boost::statechart::custom_reaction<GameStart>,
        boost::statechart::custom_reaction<Error>
    > reactions;

    MPLobby(my_context ctx);
    ~MPLobby();

    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const HostID& msg);
    boost::statechart::result react(const LobbyUpdate& msg);
    boost::statechart::result react(const LobbyChat& msg);
    boost::statechart::result react(const CancelMPGameClicked& a);
    boost::statechart::result react(const StartMPGameClicked& a);
    boost::statechart::result react(const GameStart& msg);
    boost::statechart::result react(const Error& msg);

    CLIENT_ACCESSOR
};


/** The human client state in which a game has been started, and a turn is being
  * played. */
struct PlayingGame : boost::statechart::simple_state<PlayingGame, HumanClientFSM, WaitingForGameStart> {
    typedef boost::statechart::simple_state<PlayingGame, HumanClientFSM, WaitingForGameStart> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<HostID>,
        boost::statechart::custom_reaction<PlayerChat>,
        boost::statechart::custom_reaction<Disconnection>,
        boost::statechart::custom_reaction<PlayerStatus>,
        boost::statechart::custom_reaction<Diplomacy>,
        boost::statechart::custom_reaction<DiplomaticStatusUpdate>,
        boost::statechart::custom_reaction<VictoryDefeat>,
        boost::statechart::custom_reaction<PlayerEliminated>,
        boost::statechart::custom_reaction<EndGame>,
        boost::statechart::custom_reaction<ResetToIntroMenu>,
        boost::statechart::custom_reaction<Error>,
        boost::statechart::custom_reaction<TurnProgress>,
        boost::statechart::custom_reaction<TurnPartialUpdate>
    > reactions;

    PlayingGame();
    ~PlayingGame();

    boost::statechart::result react(const HostID& msg);
    boost::statechart::result react(const PlayerChat& msg);
    boost::statechart::result react(const Disconnection& d);
    boost::statechart::result react(const PlayerStatus& msg);
    boost::statechart::result react(const Diplomacy& d);
    boost::statechart::result react(const DiplomaticStatusUpdate& u);
    boost::statechart::result react(const VictoryDefeat& msg);
    boost::statechart::result react(const PlayerEliminated& msg);
    boost::statechart::result react(const EndGame& msg);
    boost::statechart::result react(const ResetToIntroMenu& msg);
    boost::statechart::result react(const Error& msg);
    boost::statechart::result react(const TurnProgress& msg);
    boost::statechart::result react(const TurnPartialUpdate& msg);

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
        boost::statechart::custom_reaction<SaveGame>,
        boost::statechart::custom_reaction<CombatStart>,
        boost::statechart::custom_reaction<TurnUpdate>
    > reactions;

    WaitingForTurnData(my_context ctx);
    ~WaitingForTurnData();

    boost::statechart::result react(const SaveGame& d);
    boost::statechart::result react(const CombatStart& msg);
    boost::statechart::result react(const TurnUpdate& msg);

    CLIENT_ACCESSOR
};


/** The substate of PlayingGame in which the player is actively playing a turn. */
struct PlayingTurn : boost::statechart::state<PlayingTurn, PlayingGame> {
    typedef boost::statechart::state<PlayingTurn, PlayingGame> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<SaveGame>,
        boost::statechart::custom_reaction<AutoAdvanceFirstTurn>,
        boost::statechart::custom_reaction<TurnUpdate>,
        boost::statechart::custom_reaction<TurnEnded>
    > reactions;

    PlayingTurn(my_context ctx);
    ~PlayingTurn();

    boost::statechart::result react(const SaveGame& d);
    boost::statechart::result react(const AutoAdvanceFirstTurn& d);
    boost::statechart::result react(const TurnUpdate& msg);
    boost::statechart::result react(const TurnEnded& d);

    CLIENT_ACCESSOR
};


/** The substate of WaitingForTurnData in which the player is resolving a combat. */
struct ResolvingCombat : boost::statechart::state<ResolvingCombat, PlayingGame> {
    typedef boost::statechart::state<ResolvingCombat, PlayingGame> Base;

    typedef boost::mpl::list<
        boost::statechart::custom_reaction<CombatStart>,
        boost::statechart::custom_reaction<CombatRoundUpdate>,
        boost::statechart::custom_reaction<CombatEnd>,
        boost::statechart::deferral<Diplomacy>
    > reactions;

    ResolvingCombat(my_context ctx);
    ~ResolvingCombat();

    boost::statechart::result react(const CombatStart& msg);
    boost::statechart::result react(const CombatRoundUpdate& msg);
    boost::statechart::result react(const CombatEnd& msg);

    std::auto_ptr<CombatData>   m_previous_combat_data;
    std::auto_ptr<CombatData>   m_combat_data;
    std::auto_ptr<CombatWnd>    m_combat_wnd;

    CLIENT_ACCESSOR
};

#undef CLIENT_ACCESSOR

#endif
