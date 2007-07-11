#include "HumanClientFSM.h"

#include "../../util/AppInterface.h"
#include "../../util/MultiplayerCommon.h"


namespace {
    const bool TRACE_EXECUTION = true;
}

////////////////////////////////////////////////////////////
// HumanClientFSM
////////////////////////////////////////////////////////////
HumanClientFSM::HumanClientFSM(HumanClientApp &human_client) :
    m_human_client(human_client)
{}

void HumanClientFSM::unconsumed_event(const boost::statechart::event_base &event)
{
    std::string most_derived_message_type_str = "[ERROR: Unknown Event]";
    const boost::statechart::event_base* event_ptr = &event;
    if (dynamic_cast<const Disconnection*>(event_ptr))
        most_derived_message_type_str = "Disconnection";
#define MESSAGE_EVENT_CASE(r, data, name)                               \
    else if (dynamic_cast<const name*>(event_ptr))                      \
        most_derived_message_type_str = BOOST_PP_STRINGIZE(name);
    BOOST_PP_SEQ_FOR_EACH(MESSAGE_EVENT_CASE, _, MESSAGE_EVENTS);
#undef MESSAGE_EVENT_CASE
    Logger().errorStream() << "HumanClientFSM : A " << most_derived_message_type_str << " event was passed to "
        "the HumanClientFSM.  This event is illegal in the FSM's current state.  It is being ignored.";
}

HumanClientApp& HumanClientFSM::HumanClient()
{ return m_human_client; }


////////////////////////////////////////////////////////////
// IntroMenu
////////////////////////////////////////////////////////////
IntroMenu::IntroMenu() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) IntroMenu"; }

IntroMenu::~IntroMenu()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~IntroMenu"; }


////////////////////////////////////////////////////////////
// WaitingForSPHostAck
////////////////////////////////////////////////////////////
WaitingForSPHostAck::WaitingForSPHostAck() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForSPHostAck"; }

WaitingForSPHostAck::~WaitingForSPHostAck()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForSPHostAck"; }

boost::statechart::result WaitingForSPHostAck::react(const HostSPGame& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForSPHostAck.HostSPGame";
    // TODO: Record player ID
    return transit<PlayingGame>();
}


////////////////////////////////////////////////////////////
// WaitingForMPHostAck
////////////////////////////////////////////////////////////
WaitingForMPHostAck::WaitingForMPHostAck() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPHostAck"; }

WaitingForMPHostAck::~WaitingForMPHostAck()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForMPHostAck"; }

boost::statechart::result WaitingForMPHostAck::react(const HostMPGame& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPHostAck.HostMPGame";
    // TODO: Record player ID
    return transit<HostMPLobby>();
}


////////////////////////////////////////////////////////////
// WaitingForMPJoinAck
////////////////////////////////////////////////////////////
WaitingForMPJoinAck::WaitingForMPJoinAck() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPJoinAck"; }

WaitingForMPJoinAck::~WaitingForMPJoinAck()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForMPJoinAck"; }

boost::statechart::result WaitingForMPJoinAck::react(const JoinGame& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPJoinAck.JoinGame";
    // TODO: Record player ID
    return transit<NonHostMPLobby>();
}


////////////////////////////////////////////////////////////
// MPLobby
////////////////////////////////////////////////////////////
MPLobby::MPLobby() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby"; }

MPLobby::~MPLobby()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~MPLobby"; }

boost::statechart::result MPLobby::react(const Disconnection& d)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.Disconnection";
    // TODO
    return transit<IntroMenu>();
}

boost::statechart::result MPLobby::react(const LobbyUpdate& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyUpdate";
    // TODO
    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyChat& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyChat";
    // TODO
    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyHostAbort& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyHostAbort";
    // TODO
    return transit<IntroMenu>();
}

boost::statechart::result MPLobby::react(const LobbyNonHostExit& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyNonHostExit";
    // TODO
    return discard_event();
}

boost::statechart::result MPLobby::react(const GameStart& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.GameStart";
    // TODO
    return transit<PlayingGame>();
}


////////////////////////////////////////////////////////////
// MPLobbyIdle
////////////////////////////////////////////////////////////
MPLobbyIdle::MPLobbyIdle() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobbyIdle"; }

MPLobbyIdle::~MPLobbyIdle()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~MPLobbyIdle"; }


////////////////////////////////////////////////////////////
// HostMPLobby
////////////////////////////////////////////////////////////
HostMPLobby::HostMPLobby() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) HostMPLobby"; }

HostMPLobby::~HostMPLobby()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~HostMPLobby"; }

boost::statechart::result HostMPLobby::react(const StartMPGameClicked& a)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) HostMPLobby.StartMPGameClicked";
    // TODO
    return transit<PlayingGame>();
}


////////////////////////////////////////////////////////////
// NonHostMPLobby
////////////////////////////////////////////////////////////
NonHostMPLobby::NonHostMPLobby() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) NonHostMPLobby"; }

NonHostMPLobby::~NonHostMPLobby()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~NonHostMPLobby"; }

boost::statechart::result NonHostMPLobby::react(const GameStart& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) NonHostMPLobby.GameStart";
    // TODO
    return transit<PlayingGame>();
}


////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame"; }

PlayingGame::~PlayingGame()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~PlayingGame"; }

boost::statechart::result PlayingGame::react(const Disconnection& d)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.Disconnection";
    // TODO
    return transit<IntroMenu>();
}

boost::statechart::result PlayingGame::react(const PlayerEliminated& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.PlayerEliminated";
    // TODO
    return discard_event();
}

boost::statechart::result PlayingGame::react(const PlayerExit& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.PlayerExit";
    // TODO
    return discard_event();
}

boost::statechart::result PlayingGame::react(const EndGame& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.EndGame";
    // TODO
    return transit<IntroMenu>();
}


////////////////////////////////////////////////////////////
// WaitingForTurnData
////////////////////////////////////////////////////////////
WaitingForTurnData::WaitingForTurnData() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData"; }

WaitingForTurnData::~WaitingForTurnData()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForTurnData"; }

boost::statechart::result WaitingForTurnData::react(const TurnProgress& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.TurnProgress";
    // TODO
    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const TurnUpdate& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.TurnUpdate";
    // TODO
    return terminate();
}

boost::statechart::result WaitingForTurnData::react(const LoadGame& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.LoadGame";
    // TODO
    return terminate();
}

boost::statechart::result WaitingForTurnData::react(const CombatStart& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.CombatStart";
    // TODO
    return transit<ResolvingCombat>();
}


////////////////////////////////////////////////////////////
// WaitingForTurnDataIdle
////////////////////////////////////////////////////////////
WaitingForTurnDataIdle::WaitingForTurnDataIdle() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnDataIdle"; }

WaitingForTurnDataIdle::~WaitingForTurnDataIdle()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForTurnDataIdle"; }


////////////////////////////////////////////////////////////
// PlayingTurn
////////////////////////////////////////////////////////////
PlayingTurn::PlayingTurn() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn"; }

PlayingTurn::~PlayingTurn()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~PlayingTurn"; }

boost::statechart::result PlayingTurn::react(const SaveGame& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn.SaveGame";
    // TODO
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const PlayerChat& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn.PlayerChat";
    // TODO
    return discard_event();
}


////////////////////////////////////////////////////////////
// ResolvingCombat
////////////////////////////////////////////////////////////
ResolvingCombat::ResolvingCombat() :
    Base ()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat"; }

ResolvingCombat::~ResolvingCombat()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~ResolvingCombat"; }

boost::statechart::result ResolvingCombat::react(const CombatRoundUpdate& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat.CombatRoundUpdate";
    // TODO
    return discard_event();
}

boost::statechart::result ResolvingCombat::react(const CombatEnd& msg)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat.CombatEnd";
    // TODO
    return transit<WaitingForTurnDataIdle>();
}
