#include "HumanClientFSM.h"

#include "HumanClientApp.h"
#include "../../Empire/Empire.h"
#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../network/Networking.h"
#include "../../network/ClientNetworking.h"
#include "../../util/i18n.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../UI/ChatWnd.h"
#include "../../UI/PlayerListWnd.h"
#include "../../UI/IntroScreen.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../UI/MapWnd.h"

#include <boost/format.hpp>
#include <thread>

/** \page statechart_notes Notes on Boost Statechart transitions

    \section reaction_transition_note Transistion Ordering and Reaction return values.

    Transitions, transit<> or discard_event(), from states must occur from the current state.
    Transitioning from a state a second time causes indeterminate behavior and may crash.
    Transitioning from a state that is not the current state causes indeterminate behavior and may crash.

    Reactions handle events in states.  They return a statechart::result from a transit<> or discard_event().

    In a reaction the state is transitioned when transit<> or discard_event() is called, not when
    the reaction returns.  If a function is called within the reaction that may internally cause a
    transition, then the transistion out of the reaction's initial state must happen before the
    function is called.  Save the return value and return it at the end of the reaction.

    This code is correct:
    \code
        boost::statechart::result SomeState::react(const SomeEvent& a) {

            // Transition before calling other code that may also transition
            auto retval = transit<SomeOtherState>();

            // Or the transit could be a discard event
            // auto retval = discard_event();

            // Call functions that might also transition.
            Client().ResetToIntro();
            ClientUI::MessageBox("Some message", true);

            // Return the state chart result from the only transition in this reaction.
            return retval;
        }
    \endcode
    discard_event() happens before any transitions inside ResetToIntro() or the MessageBox.

    Two constructions that may cause problems are:

        - MessageBox blocks execution locally and starts an EventPump that handles events that may
          transit<> to a new state which makes a local transition lexically after the MessageBox
          potentially fatal.

        - Some client functions (ResetToIntro()) process events and change the state machine, so a
          transition after the function is undefined and potentially fatal.

 */


class CombatLogManager;
CombatLogManager&   GetCombatLogManager();

namespace {
    const bool TRACE_EXECUTION = true;
}

////////////////////////////////////////////////////////////
bool TraceHumanClientFSMExecution()
{ return TRACE_EXECUTION; }


////////////////////////////////////////////////////////////
// HumanClientFSM
////////////////////////////////////////////////////////////
HumanClientFSM::HumanClientFSM(HumanClientApp &human_client) :
    m_client(human_client)
{}

void HumanClientFSM::unconsumed_event(const boost::statechart::event_base &event)
{
    std::string most_derived_message_type_str = "[ERROR: Unknown Event]";
    const boost::statechart::event_base* event_ptr = &event;
    if (dynamic_cast<const Disconnection*>(event_ptr))
        most_derived_message_type_str = "Disconnection";
#define EVENT_CASE(r, data, name)                                       \
    else if (dynamic_cast<const name*>(event_ptr))                      \
        most_derived_message_type_str = BOOST_PP_STRINGIZE(name);
    BOOST_PP_SEQ_FOR_EACH(EVENT_CASE, _, HUMAN_CLIENT_FSM_EVENTS)
    BOOST_PP_SEQ_FOR_EACH(EVENT_CASE, _, MESSAGE_EVENTS)
#undef EVENT_CASE
    ErrorLogger() << "HumanClientFSM : A " << most_derived_message_type_str << " event was passed to "
        "the HumanClientFSM.  This event is illegal in the FSM's current state.  It is being ignored.";
}


////////////////////////////////////////////////////////////
// IntroMenu
////////////////////////////////////////////////////////////
IntroMenu::IntroMenu(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) IntroMenu";
    Client().GetClientUI().ShowIntroScreen();
}

IntroMenu::~IntroMenu() {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~IntroMenu";
}

boost::statechart::result IntroMenu::react(const HostSPGameRequested& a) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) IntroMenu.HostSPGameRequested";
    Client().Remove(Client().GetClientUI().GetIntroScreen());
    return transit<WaitingForSPHostAck>();
}

boost::statechart::result IntroMenu::react(const HostMPGameRequested& a) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) IntroMenu.HostMPGameRequested";
    Client().Remove(Client().GetClientUI().GetIntroScreen());
    return transit<WaitingForMPHostAck>();
}

boost::statechart::result IntroMenu::react(const JoinMPGameRequested& a) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) IntroMenu.JoinMPGameRequested";
    Client().Remove(Client().GetClientUI().GetIntroScreen());
    return transit<WaitingForMPJoinAck>();
}

boost::statechart::result IntroMenu::react(const StartQuittingGame& e) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) Quit or reset to main menu.";
    post_event(e);
    return transit<QuittingGame>();
}

////////////////////////////////////////////////////////////
// WaitingForSPHostAck
////////////////////////////////////////////////////////////
WaitingForSPHostAck::WaitingForSPHostAck() :
    Base()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForSPHostAck"; }

WaitingForSPHostAck::~WaitingForSPHostAck()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~WaitingForSPHostAck"; }

boost::statechart::result WaitingForSPHostAck::react(const HostSPGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForSPHostAck.HostSPGame";

    Client().Networking().SetPlayerID(msg.m_message.ReceivingPlayer());
    Client().Networking().SetHostPlayerID(msg.m_message.ReceivingPlayer());

    Client().GetClientUI().GetMapWnd()->Sanitize();

    return transit<PlayingGame>();
}

boost::statechart::result WaitingForSPHostAck::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro();
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result WaitingForSPHostAck::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForSPHostAck.Error";
    std::string problem;
    bool fatal;
    ExtractErrorMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "WaitingForSPHostAck::react(const Error& msg) error: " << problem;

    //Note: transit<> frees this pointer so Client() must be called before.
    HumanClientApp& client = Client();

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro();
        ClientUI::MessageBox(UserString(problem), true);
        client.GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
    }
    return retval;
}

boost::statechart::result WaitingForSPHostAck::react(const StartQuittingGame& e) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}


////////////////////////////////////////////////////////////
// WaitingForMPHostAck
////////////////////////////////////////////////////////////
WaitingForMPHostAck::WaitingForMPHostAck() :
    Base()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPHostAck"; }

WaitingForMPHostAck::~WaitingForMPHostAck()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~WaitingForMPHostAck"; }

boost::statechart::result WaitingForMPHostAck::react(const HostMPGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPHostAck.HostMPGame";

    Client().Networking().SetPlayerID(msg.m_message.ReceivingPlayer());
    Client().Networking().SetHostPlayerID(msg.m_message.ReceivingPlayer());

    return transit<MPLobby>();
}

boost::statechart::result WaitingForMPHostAck::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro();
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result WaitingForMPHostAck::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPHostAck.Error";
    std::string problem;
    bool fatal;
    ExtractErrorMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "WaitingForMPHostAck::react(const Error& msg) error: " << problem;

    //Note: transit<> frees this pointer so Client() must be called before.
    HumanClientApp& client = Client();

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro();
        ClientUI::MessageBox(UserString(problem), true);
        client.GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
    }
    return retval;
}

boost::statechart::result WaitingForMPHostAck::react(const StartQuittingGame& e) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}


////////////////////////////////////////////////////////////
// WaitingForMPJoinAck
////////////////////////////////////////////////////////////
WaitingForMPJoinAck::WaitingForMPJoinAck() :
    Base()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPJoinAck"; }

WaitingForMPJoinAck::~WaitingForMPJoinAck()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~WaitingForMPJoinAck"; }

boost::statechart::result WaitingForMPJoinAck::react(const JoinGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPJoinAck.JoinGame";

    Client().Networking().SetPlayerID(msg.m_message.ReceivingPlayer());

    return transit<MPLobby>();
}

boost::statechart::result WaitingForMPJoinAck::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro();
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result WaitingForMPJoinAck::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPJoinAck.Error";
    std::string problem;
    bool fatal;
    ExtractErrorMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "WaitingForMPJoinAck::react(const Error& msg) error: " << problem;

    //Note: transit<> frees this pointer so Client() must be called before.
    HumanClientApp& client = Client();

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro();
        ClientUI::MessageBox(UserString(problem), true);
        client.GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
    }

    return retval;
}

boost::statechart::result WaitingForMPJoinAck::react(const StartQuittingGame& e) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}


////////////////////////////////////////////////////////////
// MPLobby
////////////////////////////////////////////////////////////
MPLobby::MPLobby(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby";

    Client().Register(Client().GetClientUI().GetMultiPlayerLobbyWnd());
}

MPLobby::~MPLobby() {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~MPLobby";
}

boost::statechart::result MPLobby::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro();
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result MPLobby::react(const HostID& msg) {
    const std::string& text = msg.m_message.Text();
    int host_id = Networking::INVALID_PLAYER_ID;
    if (text.empty()) {
        ErrorLogger() << "MPLobby::react(const HostID& msg) got empty message text?!";
    } else {
        try {
            host_id = boost::lexical_cast<int>(text);
        } catch (...) {
            ErrorLogger() << "MPLobby::react(const HostID& msg) couldn't parese message text: " << text;
        }
    }
    Client().Networking().SetHostPlayerID(host_id);

    Client().GetClientUI().GetMultiPlayerLobbyWnd()->Refresh();

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.LobbyUpdate";
    MultiplayerLobbyData lobby_data;
    ExtractLobbyUpdateMessageData(msg.m_message, lobby_data);
    Client().GetClientUI().GetMultiPlayerLobbyWnd()->LobbyUpdate(lobby_data);
    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyChat& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.LobbyChat";
    Client().GetClientUI().GetMultiPlayerLobbyWnd()->ChatMessage(msg.m_message.SendingPlayer(), msg.m_message.Text());
    return discard_event();
}

boost::statechart::result MPLobby::react(const CancelMPGameClicked& a)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.CancelMPGameClicked";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro();
    return retval;
}

boost::statechart::result MPLobby::react(const StartMPGameClicked& a) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.StartMPGameClicked";

    if (Client().Networking().PlayerIsHost(Client().Networking().PlayerID()))
        Client().Networking().SendMessage(StartMPGameMessage(Client().PlayerID()));
    else
        ErrorLogger() << "MPLobby::react received start MP game event but this client is not the host.  Ignoring";

    return discard_event(); // wait for server response GameStart message to leave this state...
}

boost::statechart::result MPLobby::react(const GameStart& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.GameStart";

    // need to re-post the game start message to be re-handled after
    // transitioning into WaitingForGameStart
    post_event(msg);

    Client().GetClientUI().GetMapWnd()->Sanitize();
    Client().Remove(Client().GetClientUI().GetMultiPlayerLobbyWnd());
    return transit<WaitingForGameStart>();
}

boost::statechart::result MPLobby::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.Error";
    std::string problem;
    bool fatal;
    ExtractErrorMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "MPLobby::react(const Error& msg) error: " << problem;

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro();
        ClientUI::MessageBox(UserString(problem), true);
    }

    return retval;
}

boost::statechart::result MPLobby::react(const StartQuittingGame& e) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}


////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame";

    Client().Register(Client().GetClientUI().GetMapWnd());
    Client().GetClientUI().GetMapWnd()->Show();
    Client().GetClientUI().GetMessageWnd()->Clear();
}

PlayingGame::~PlayingGame() {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~PlayingGame";
}

boost::statechart::result PlayingGame::react(const HostID& msg) {
    const int initial_host_id = Client().Networking().HostPlayerID();
    const std::string& text = msg.m_message.Text();
    int host_id = Networking::INVALID_PLAYER_ID;
    if (text.empty()) {
        ErrorLogger() << "PlayingGame::react(const HostID& msg) got empty message text?!";
    } else {
        try {
            host_id = boost::lexical_cast<int>(text);
        } catch (...) {
            ErrorLogger() << "PlayingGame::react(const HostID& msg) couldn't parese message text: " << text;
        }
    }
    Client().Networking().SetHostPlayerID(host_id);

    if (initial_host_id != host_id)
        DebugLogger() << "PlayingGame::react(const HostID& msg) New Host ID: " << host_id;

    return discard_event();
}

boost::statechart::result PlayingGame::react(const PlayerChat& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.PlayerChat: " << msg.m_message.Text();
    const std::string& text = msg.m_message.Text();
    int sending_player_id = msg.m_message.SendingPlayer();
    int recipient_player_id = msg.m_message.ReceivingPlayer();

    Client().GetClientUI().GetMessageWnd()->HandlePlayerChatMessage(text, sending_player_id, recipient_player_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro();
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result PlayingGame::react(const PlayerStatus& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.PlayerStatus";
    int about_player_id;
    MessagePacket::PlayerStatus status;
    ExtractPlayerStatusMessageData(msg.m_message, about_player_id, status);

    Client().SetPlayerStatus(about_player_id, status);
    Client().GetClientUI().GetMessageWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    Client().GetClientUI().GetPlayerListWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    // TODO: tell the map wnd or something else as well?

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Diplomacy& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Diplomacy";

    DiplomaticMessage diplo_message;
    ExtractDiplomacyMessageData(d.m_message, diplo_message);
    Empires().SetDiplomaticMessage(diplo_message);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const DiplomaticStatusUpdate& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.DiplomaticStatusUpdate";

    DiplomaticStatusUpdateInfo diplo_update;
    ExtractDiplomaticStatusMessageData(u.m_message, diplo_update);
    Empires().SetDiplomaticStatus(diplo_update.empire1_id, diplo_update.empire2_id, diplo_update.diplo_status);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const EndGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.EndGame";
    MessagePacket::EndGameReason reason;
    std::string reason_player_name;
    ExtractEndGameMessageData(msg.m_message, reason, reason_player_name);
    std::string reason_message;
    bool error = false;
    switch (reason) {
    case MessagePacket::LOCAL_CLIENT_DISCONNECT:
        reason_message = UserString("SERVER_LOST");
        break;
    case MessagePacket::PLAYER_DISCONNECT:
        reason_message = boost::io::str(FlexibleFormat(UserString("PLAYER_DISCONNECTED")) % reason_player_name);
        error = true;
        break;
    }

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro();
    ClientUI::MessageBox(reason_message, error);
    return retval;
}

boost::statechart::result PlayingGame::react(const StartQuittingGame& e) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}

boost::statechart::result PlayingGame::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Error";
    std::string problem;
    bool fatal;
    ExtractErrorMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "PlayingGame::react(const Error& msg) error: "
                  << problem << "\nProblem is" << (fatal ? "fatal" : "non-fatal");

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal)
        Client().ResetToIntro();

    ClientUI::MessageBox(UserString(problem), fatal);

    return retval;
}

boost::statechart::result PlayingGame::react(const TurnProgress& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.TurnProgress";

    MessagePacket::TurnProgressPhase phase_id;
    ExtractTurnProgressMessageData(msg.m_message, phase_id);
    Client().GetClientUI().GetMessageWnd()->HandleTurnPhaseUpdate(phase_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const TurnPartialUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.TurnPartialUpdate";

    ExtractTurnPartialUpdateMessageData(msg.m_message,   Client().EmpireID(),    GetUniverse());

    Client().GetClientUI().GetMapWnd()->MidTurnUpdate();

    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForGameStart
////////////////////////////////////////////////////////////
WaitingForGameStart::WaitingForGameStart(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForGameStart";

    Client().Register(Client().GetClientUI().GetMessageWnd());
    Client().Register(Client().GetClientUI().GetPlayerListWnd());

    Client().GetClientUI().GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForGameStart::~WaitingForGameStart()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~WaitingForGameStart"; }

boost::statechart::result WaitingForGameStart::react(const GameStart& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForGameStart.GameStart";

    bool loaded_game_data;
    bool ui_data_available;
    SaveGameUIData ui_data;
    bool save_state_string_available;
    std::string save_state_string; // ignored - used by AI but not by human client
    OrderSet orders;
    bool single_player_game = false;
    int empire_id = ALL_EMPIRES;
    int current_turn = INVALID_GAME_TURN;
    Client().PlayerStatus().clear();

    ExtractGameStartMessageData(msg.m_message,       single_player_game,             empire_id,
                                current_turn,        Empires(),                      GetUniverse(),
                                GetSpeciesManager(), GetCombatLogManager(),          GetSupplyManager(),
                                Client().Players(),  orders,                         loaded_game_data,
                                ui_data_available,   ui_data,                        save_state_string_available,
                                save_state_string,   Client().GetGalaxySetupData());

    DebugLogger() << "Extracted GameStart message for turn: " << current_turn << " with empire: " << empire_id;

    Client().SetSinglePlayerGame(single_player_game);
    Client().SetEmpireID(empire_id);
    Client().SetCurrentTurn(current_turn);

    Client().StartGame();
    std::swap(Client().Orders(), orders); // bring back orders planned in the current turn, they will be applied later, after some basic turn initialization
    if (loaded_game_data && ui_data_available)
        Client().GetClientUI().RestoreFromSaveData(ui_data);

    // if I am the host on the first turn, do an autosave. on later turns, will
    // have just loaded save, so don't need to autosave. might also have just
    // loaded a turn 1 autosave, but not sure how to check for that here...
    if (Client().CurrentTurn() == 1 && Client().Networking().PlayerIsHost(Client().PlayerID()))
        Client().Autosave();

    return transit<PlayingTurn>();
}


////////////////////////////////////////////////////////////
// WaitingForTurnData
////////////////////////////////////////////////////////////
WaitingForTurnData::WaitingForTurnData(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForTurnData";
    Client().GetClientUI().GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForTurnData::~WaitingForTurnData()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~WaitingForTurnData"; }

boost::statechart::result WaitingForTurnData::react(const SaveGameDataRequest& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForTurnData.SaveGameDataRequest";
    DebugLogger() << "Sending Save Game Data to Server";
    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("SERVER_SAVE_INITIATE_ACK") + "\n");
    Client().HandleSaveGameDataRequest();
    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const SaveGameComplete& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForTurnData.SaveGameComplete";

    std::string save_filename;
    int bytes_written;
    ExtractServerSaveGameCompleteMessageData(msg.m_message, save_filename, bytes_written);

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("SERVER_SAVE_COMPLETE")) % save_filename % bytes_written) + "\n");

    Client().SaveGameCompleted();

    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const TurnUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.TurnUpdate";

    int current_turn = INVALID_GAME_TURN;

    try {
        ExtractTurnUpdateMessageData(msg.m_message,           Client().EmpireID(),    current_turn,
                                     Empires(),               GetUniverse(),          GetSpeciesManager(),
                                     GetCombatLogManager(),   GetSupplyManager(),     Client().Players());
    } catch (...) {
        Client().GetClientUI().GetMessageWnd()->HandleLogMessage(UserString("ERROR_PROCESSING_SERVER_MESSAGE") + "\n");
        return discard_event();
    }

    DebugLogger() << "Extracted TurnUpdate message for turn: " << current_turn;

    Client().SetCurrentTurn(current_turn);

    // if I am the host, do autosave
    if (Client().Networking().PlayerIsHost(Client().PlayerID()))
        Client().Autosave();

    Client().HandleTurnUpdate();

    return transit<PlayingTurn>();
}

boost::statechart::result WaitingForTurnData::react(const DispatchCombatLogs& msg) {
    DebugLogger() << "(PlayerFSM) WaitingForTurnData::DispatchCombatLogs message received";
    Client().UpdateCombatLogs(msg.m_message);
    return discard_event();
}


////////////////////////////////////////////////////////////
// PlayingTurn
////////////////////////////////////////////////////////////
PlayingTurn::PlayingTurn(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn";
    Client().Register(Client().GetClientUI().GetMapWnd());
    Client().GetClientUI().GetMapWnd()->InitTurn();
    Client().GetClientUI().GetMapWnd()->RegisterWindows(); // only useful at game start but InitTurn() takes a long time, don't want to display windows before content is ready.  could go in WaitingForGameStart dtor but what if it is given e.g. an error reaction?
    // TODO: reselect last fleet if stored in save game ui data?
    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("TURN_BEGIN")) % CurrentTurn()) + "\n");
    Client().GetClientUI().GetMessageWnd()->HandlePlayerStatusUpdate(MessagePacket::PLAYING_TURN, Client().PlayerID());
    Client().GetClientUI().GetPlayerListWnd()->Refresh();
    Client().GetClientUI().GetPlayerListWnd()->HandlePlayerStatusUpdate(MessagePacket::PLAYING_TURN, Client().PlayerID());

    if (Client().GetApp()->GetClientType() != Networking::CLIENT_TYPE_HUMAN_OBSERVER)
        Client().GetClientUI().GetMapWnd()->EnableOrderIssuing(true);

    if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER) {
        // observers can't do anything but wait for the next update, and need to
        // be back in WaitingForTurnData, so posting TurnEnded here has the effect
        // of keeping observers in the WaitingForTurnData state so they can receive
        // updates from the server.
        post_event(TurnEnded());

    } else if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
        if (Client().GetClientUI().GetMapWnd()->AutoEndTurnEnabled()) {
            // if in-game-GUI auto turn advance enabled, set auto turn counter to 1
            Client().InitAutoTurns(1);
        }

        if (Client().AutoTurnsLeft() <= 0 &&
            GetOptionsDB().Get<bool>("auto-quit"))
        {
            // if no auto turns left, and supposed to quit after that, quit
            DebugLogger() << "auto-quit ending game.";
            std::cout << "auto-quit ending game." << std::endl;
            Client().ExitApp();
        }

        // if there are still auto turns left, advance the turn automatically,
        // and decrease the auto turn counter
        if (Client().AutoTurnsLeft() > 0) {
            post_event(AdvanceTurn());
            Client().DecAutoTurns();
        }
    }
}

PlayingTurn::~PlayingTurn()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~PlayingTurn"; }

boost::statechart::result PlayingTurn::react(const SaveGameDataRequest& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn.SaveGameDataRequest";
    DebugLogger() << "Sending Save Game Data to Server";
    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("SERVER_SAVE_INITIATE_ACK") + "\n");
    Client().HandleSaveGameDataRequest();
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const SaveGameComplete& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn.SaveGameComplete";

    std::string save_filename;
    int bytes_written;
    ExtractServerSaveGameCompleteMessageData(msg.m_message, save_filename, bytes_written);

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("SERVER_SAVE_COMPLETE")) % save_filename % bytes_written) + "\n");

    Client().SaveGameCompleted();
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const AdvanceTurn& d) {
    Client().StartTurn();
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const TurnUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn.TurnUpdate";

     Client().GetClientUI().GetMessageWnd()->HandleLogMessage(UserString("ERROR_EARLY_TURN_UPDATE") + "\n");

    // need to re-post the game start message to be re-handled after
    // transitioning into WaitingForTurnData
    post_event(msg);

    return transit<WaitingForTurnData>();
}

boost::statechart::result PlayingTurn::react(const TurnEnded& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn.TurnEnded";
    return transit<WaitingForTurnData>();
}

boost::statechart::result PlayingTurn::react(const PlayerStatus& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn.PlayerStatus";
    int about_player_id;
    MessagePacket::PlayerStatus status;
    ExtractPlayerStatusMessageData(msg.m_message, about_player_id, status);

    Client().SetPlayerStatus(about_player_id, status);
    Client().GetClientUI().GetMessageWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    Client().GetClientUI().GetPlayerListWnd()->HandlePlayerStatusUpdate(status, about_player_id);

    if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR &&
        Client().GetClientUI().GetMapWnd()->AutoEndTurnEnabled())
    {
        // check status of all non-mod non-obs players: are they all done their turns?
        bool all_participants_waiting = true;
        const std::map<int, MessagePacket::PlayerStatus>& player_status = Client().PlayerStatus();
        for (std::map<int, PlayerInfo>::value_type& entry : Client().Players()) {
            int player_id = entry.first;

            if (entry.second.client_type != Networking::CLIENT_TYPE_AI_PLAYER &&
                entry.second.client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER)
            { continue; }   // only active participants matter...

            std::map<int, MessagePacket::PlayerStatus>::const_iterator status_it = player_status.find(player_id);
            if (status_it == player_status.end()) {
                all_participants_waiting = false;
                break;
            }
            if (status_it->second != MessagePacket::WAITING) {
                all_participants_waiting = false;
                break;
            }
        }

        // if all participants waiting, can end turn immediately
        if (all_participants_waiting)
            post_event(AdvanceTurn());
    }

    return discard_event();
}

boost::statechart::result PlayingTurn::react(const DispatchCombatLogs& msg) {
    DebugLogger() << "(PlayerFSM) PlayingGame::DispatchCombatLogs message received";
    Client().UpdateCombatLogs(msg.m_message);
    return discard_event();
}


////////////////////////////////////////////////////////////
// QuittingGame
////////////////////////////////////////////////////////////
/** The QuittingGame state expects to start with a StartQuittingGame message posted. */
QuittingGame::QuittingGame(my_context c) :
    my_base(c),
    m_start_time(Clock::now()),
    m_reset_to_intro(true),
    m_server_process(nullptr)
{
    // Quit the game by sending a shutdown message to the server and waiting for
    // the disconnection event.  Free the server if it starts an orderly
    // shutdown, otherwise kill it.

    if (TRACE_EXECUTION) DebugLogger() << "(Host) QuittingGame";
}

QuittingGame::~QuittingGame()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~QuittingGame"; }

boost::statechart::result QuittingGame::react(const StartQuittingGame& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) QuittingGame reset to intro is " << u.m_reset_to_intro;

    m_reset_to_intro = u.m_reset_to_intro;
    m_server_process = &u.m_server;

    post_event(ShutdownServer());
    return discard_event();
}

boost::statechart::result QuittingGame::react(const ShutdownServer& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) QuittingGame.ShutdownServer";

    if (!m_server_process) {
        ErrorLogger() << "m_server_process is nullptr";
        post_event(TerminateServer());
        return discard_event();
    }

    if (m_server_process->Empty()) {
        if (Client().Networking().IsTxConnected()) {
            WarnLogger() << "Disconnecting from server that is already killed.";
            Client().Networking().DisconnectFromServer();
        }
        post_event(TerminateServer());
        return discard_event();
    }

    if (Client().Networking().IsTxConnected()) {
        DebugLogger() << "Sending server shutdown message.";
        Client().Networking().SendMessage(ShutdownServerMessage(Client().Networking().PlayerID()));

        post_event(WaitForDisconnect());

    } else {
        post_event(TerminateServer());
    }
    return discard_event();
}

const auto QUITTING_TIMEOUT          = std::chrono::milliseconds(5000);
const auto QUITTING_POLLING_INTERVAL = std::chrono::milliseconds(10);
boost::statechart::result QuittingGame::react(const WaitForDisconnect& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) QuittingGame.WaitForDisconnect";

    if (!Client().Networking().IsConnected()) {
        post_event(TerminateServer());
        return discard_event();
    }

    // Wait until the timeout for a disconnect event
    if (QUITTING_TIMEOUT > (Clock::now() - m_start_time)) {
        std::this_thread::sleep_for(QUITTING_POLLING_INTERVAL);
        post_event(WaitForDisconnect());
        return discard_event();
    }

    // Otherwise kill the connection
    Client().Networking().DisconnectFromServer();

    post_event(TerminateServer());
    return discard_event();
 }

boost::statechart::result QuittingGame::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Disconnection";

    if (m_server_process) {
        // Treat disconnection as acknowledgement of shutdown and free the
        // process to allow orderly shutdown.
        m_server_process->Free();
    } else {
        ErrorLogger() << "m_server_process is nullptr";
    }

    post_event(TerminateServer());
    return discard_event();
}

boost::statechart::result QuittingGame::react(const TerminateServer& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) QuittingGame.TerminateServer";

    if (m_server_process && !m_server_process->Empty()) {
        DebugLogger() << "QuittingGame terminated server process.";
        m_server_process->RequestTermination();
    }

    // Reset the game or quit the app as appropriate
    if (m_reset_to_intro) {
        Client().ResetClientData();
        return transit<IntroMenu>();
    } else {
        throw HumanClientApp::CleanQuit();
    }
}

