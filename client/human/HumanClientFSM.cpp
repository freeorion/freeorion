#include "HumanClientFSM.h"

#include "HumanClientApp.h"
#include "../../Empire/Empire.h"
#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../universe/Universe.h"
#include "../../network/Networking.h"
#include "../../network/ClientNetworking.h"
#include "../../util/i18n.h"
#include "../util/GameRules.h"
#include "../../util/OptionsDB.h"
#include "../../UI/ChatWnd.h"
#include "../../UI/PlayerListWnd.h"
#include "../../UI/IntroScreen.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../UI/PasswordEnterWnd.h"
#include "../../UI/MapWnd.h"

#include <boost/format.hpp>
#include <boost/uuid/uuid_io.hpp>
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
    DeclareThreadSafeLogger(FSM);
}

////////////////////////////////////////////////////////////
// HumanClientFSM
////////////////////////////////////////////////////////////
HumanClientFSM::HumanClientFSM(HumanClientApp &human_client) :
    m_client(human_client)
{}

void HumanClientFSM::unconsumed_event(const boost::statechart::event_base &event) {
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

    if (terminated()) {
        ErrorLogger(FSM) << "A " << most_derived_message_type_str << " event was passed to "
            "the HumanClientFSM.  The FSM has terminated.  The event is being ignored.";
        return;
    }

    std::stringstream ss;
    ss << "[";
    for (auto leaf_state_it = state_begin(); leaf_state_it != state_end();) {
        // The following use of typeid assumes that
        // BOOST_STATECHART_USE_NATIVE_RTTI is defined
        const auto& leaf_state = *leaf_state_it;
        ss << typeid(leaf_state).name();
        ++leaf_state_it;
        if (leaf_state_it != state_end())
            ss << ", ";
    }
    ss << "]";

    ErrorLogger(FSM) << "A " << most_derived_message_type_str
                     << " event was not handled by any of these states : "
                     << ss.str() << ".  It is being ignored.";
}


////////////////////////////////////////////////////////////
// IntroMenu
////////////////////////////////////////////////////////////
IntroMenu::IntroMenu(my_context ctx) :
    Base(ctx)
{
    TraceLogger(FSM) << "(HumanClientFSM) IntroMenu";
    Client().GetClientUI().ShowIntroScreen();
}

IntroMenu::~IntroMenu() {
    TraceLogger(FSM) << "(HumanClientFSM) ~IntroMenu";
}

boost::statechart::result IntroMenu::react(const HostSPGameRequested& a) {
    TraceLogger(FSM) << "(HumanClientFSM) IntroMenu.HostSPGameRequested";
    Client().Remove(Client().GetClientUI().GetIntroScreen());
    return transit<WaitingForSPHostAck>();
}

boost::statechart::result IntroMenu::react(const HostMPGameRequested& a) {
    TraceLogger(FSM) << "(HumanClientFSM) IntroMenu.HostMPGameRequested";
    Client().Remove(Client().GetClientUI().GetIntroScreen());
    return transit<WaitingForMPHostAck>();
}

boost::statechart::result IntroMenu::react(const JoinMPGameRequested& a) {
    TraceLogger(FSM) << "(HumanClientFSM) IntroMenu.JoinMPGameRequested";
    Client().Remove(Client().GetClientUI().GetIntroScreen());
    return transit<WaitingForMPJoinAck>();
}

boost::statechart::result IntroMenu::react(const StartQuittingGame& e) {
    TraceLogger(FSM) << "(HumanClientFSM) Quit or reset to main menu.";
    post_event(e);
    return transit<QuittingGame>();
}

boost::statechart::result IntroMenu::react(const EndGame&) {
    TraceLogger(FSM) << "(HumanClientFSM) IntroMenu ignoring EndGame.";
    return discard_event();
}

boost::statechart::result IntroMenu::react(const Disconnection&) {
    TraceLogger(FSM) << "(HumanClientFSM) IntroMenu ignoring disconnection.";
    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForSPHostAck
////////////////////////////////////////////////////////////
WaitingForSPHostAck::WaitingForSPHostAck() :
    Base()
{ TraceLogger(FSM) << "(HumanClientFSM) WaitingForSPHostAck"; }

WaitingForSPHostAck::~WaitingForSPHostAck()
{ TraceLogger(FSM) << "(HumanClientFSM) ~WaitingForSPHostAck"; }

boost::statechart::result WaitingForSPHostAck::react(const HostSPGame& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForSPHostAck.HostSPGame";

    try {
        int host_id = boost::lexical_cast<int>(msg.m_message.Text());

        Client().Networking().SetPlayerID(host_id);
        Client().Networking().SetHostPlayerID(host_id);

        // Logging configuration can only be sent after receiving host id.
        Client().SendLoggingConfigToServer();

        Client().GetClientUI().GetMapWnd()->Sanitize();

        return transit<PlayingGame>();
    } catch (const boost::bad_lexical_cast& ex) {
        ErrorLogger(FSM) << "WaitingForSPHostAck::react(const HostSPGame& msg) Host id " << msg.m_message.Text() << " is not a number: " << ex.what();
        return transit<IntroMenu>();
    }
}

boost::statechart::result WaitingForSPHostAck::react(const Disconnection& d) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result WaitingForSPHostAck::react(const Error& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForSPHostAck.Error";
    std::string problem;
    bool fatal;
    int player_id;
    ExtractErrorMessageData(msg.m_message, player_id, problem, fatal);

    ErrorLogger(FSM) << "WaitingForSPHostAck::react(const Error& msg) error: " << problem;

    //Note: transit<> frees this pointer so Client() must be called before.
    HumanClientApp& client = Client();

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro(true);
        ClientUI::MessageBox(UserString(problem), true);
        client.GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
    }
    return retval;
}

boost::statechart::result WaitingForSPHostAck::react(const StartQuittingGame& e) {
    TraceLogger(FSM) << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}

boost::statechart::result WaitingForSPHostAck::react(const CheckSum& e) {
    TraceLogger(FSM) << "(HumanClientFSM) CheckSum.";
    Client().VerifyCheckSum(e.m_message);
    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForMPHostAck
////////////////////////////////////////////////////////////
WaitingForMPHostAck::WaitingForMPHostAck() :
    Base()
{ TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPHostAck"; }

WaitingForMPHostAck::~WaitingForMPHostAck()
{ TraceLogger(FSM) << "(HumanClientFSM) ~WaitingForMPHostAck"; }

boost::statechart::result WaitingForMPHostAck::react(const HostMPGame& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPHostAck.HostMPGame";

    try {
        int host_id = boost::lexical_cast<int>(msg.m_message.Text());

        Client().Networking().SetPlayerID(host_id);
        Client().Networking().SetHostPlayerID(host_id);

        // Logging configuration can only be sent after receiving host id.
        Client().SendLoggingConfigToServer();

        return transit<MPLobby>();
    } catch (const boost::bad_lexical_cast& ex) {
        ErrorLogger(FSM) << "WaitingForMPHostAck::react(const HostMPGame& msg) Host id " << msg.m_message.Text() << " is not a number: " << ex.what();
        return transit<IntroMenu>();
    }
}

boost::statechart::result WaitingForMPHostAck::react(const Disconnection& d) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result WaitingForMPHostAck::react(const Error& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPHostAck.Error";
    std::string problem;
    bool fatal;
    int player_id;
    ExtractErrorMessageData(msg.m_message, player_id, problem, fatal);

    ErrorLogger(FSM) << "WaitingForMPHostAck::react(const Error& msg) error: " << problem;

    //Note: transit<> frees this pointer so Client() must be called before.
    HumanClientApp& client = Client();

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro(true);
        ClientUI::MessageBox(UserString(problem), true);
        client.GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
    }
    return retval;
}

boost::statechart::result WaitingForMPHostAck::react(const StartQuittingGame& e) {
    TraceLogger(FSM) << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}

boost::statechart::result WaitingForMPHostAck::react(const CheckSum& e) {
    TraceLogger(FSM) << "(HumanClientFSM) CheckSum.";
    Client().VerifyCheckSum(e.m_message);
    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForMPJoinAck
////////////////////////////////////////////////////////////
WaitingForMPJoinAck::WaitingForMPJoinAck() :
    Base()
{ TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPJoinAck"; }

WaitingForMPJoinAck::~WaitingForMPJoinAck()
{ TraceLogger(FSM) << "(HumanClientFSM) ~WaitingForMPJoinAck"; }

boost::statechart::result WaitingForMPJoinAck::react(const JoinGame& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPJoinAck.JoinGame";

    try {
        int player_id;
        boost::uuids::uuid cookie;
        ExtractJoinAckMessageData(msg.m_message, player_id, cookie);

        if (!cookie.is_nil()) {
            try {
                std::string cookie_option = "network.server.cookie." + Client().Networking().Destination();
                GetOptionsDB().Remove(cookie_option);
                GetOptionsDB().Add(cookie_option, "OPTIONS_DB_SERVER_COOKIE", boost::uuids::to_string(cookie));
                GetOptionsDB().Commit();
            } catch(const std::exception& err) {
                WarnLogger() << "Cann't save cookie for server " << Client().Networking().Destination() << ": "
                             << err.what();
                // ignore
            }
        }

        Client().Networking().SetPlayerID(player_id);

        return transit<MPLobby>();
    } catch (const boost::bad_lexical_cast& ex) {
        ErrorLogger(FSM) << "WaitingForMPJoinAck::react(const JoinGame& msg) Host id " << msg.m_message.Text() << " is not a number: " << ex.what();
        return transit<IntroMenu>();
    }
}

boost::statechart::result WaitingForMPJoinAck::react(const AuthRequest& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPJoinAck.AuthRequest";

    std::string player_name;
    std::string auth;
    ExtractAuthRequestMessageData(msg.m_message, player_name, auth);

    auto password_dialog = Client().GetClientUI().GetPasswordEnterWnd();
    password_dialog->SetPlayerName(player_name);
    Client().Register(password_dialog);

    return discard_event();
}

boost::statechart::result WaitingForMPJoinAck::react(const Disconnection& d) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result WaitingForMPJoinAck::react(const Error& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPJoinAck.Error";
    std::string problem;
    bool fatal;
    int player_id;
    ExtractErrorMessageData(msg.m_message, player_id, problem, fatal);

    ErrorLogger(FSM) << "WaitingForMPJoinAck::react(const Error& msg) error: " << problem;

    //Note: transit<> frees this pointer so Client() must be called before.
    HumanClientApp& client = Client();

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro(true);
        ClientUI::MessageBox(UserString(problem), true);
        client.GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
    }

    return retval;
}

boost::statechart::result WaitingForMPJoinAck::react(const StartQuittingGame& e) {
    TraceLogger(FSM) << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}

boost::statechart::result WaitingForMPJoinAck::react(const CancelMPGameClicked& a) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForMPJoinAck.CancelMPGameClicked";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    return retval;
}

////////////////////////////////////////////////////////////
// MPLobby
////////////////////////////////////////////////////////////
MPLobby::MPLobby(my_context ctx) :
    Base(ctx)
{
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby";

    const auto& wnd = Client().GetClientUI().GetMultiPlayerLobbyWnd();
    Client().Register(wnd);
    wnd->CleanupChat();
}

MPLobby::~MPLobby() {
    TraceLogger(FSM) << "(HumanClientFSM) ~MPLobby";
}

boost::statechart::result MPLobby::react(const Disconnection& d) {
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result MPLobby::react(const HostID& msg) {
    const std::string& text = msg.m_message.Text();
    int host_id = Networking::INVALID_PLAYER_ID;
    try {
        host_id = boost::lexical_cast<int>(text);
    } catch (const boost::bad_lexical_cast& ex) {
        ErrorLogger(FSM) << "MPLobby::react(const HostID& msg) could not convert \"" << text << "\" to host id";
    }

    Client().Networking().SetHostPlayerID(host_id);

    // Logging configuration can only be sent after receiving host id.
    Client().SendLoggingConfigToServer();

    Client().GetClientUI().GetMultiPlayerLobbyWnd()->Refresh();

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyUpdate& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby.LobbyUpdate";
    MultiplayerLobbyData lobby_data;
    ExtractLobbyUpdateMessageData(msg.m_message, lobby_data);
    Client().GetClientUI().GetMultiPlayerLobbyWnd()->LobbyUpdate(lobby_data);
    return discard_event();
}

boost::statechart::result MPLobby::react(const PlayerChat& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby.PlayerChat";

    int player_id;
    boost::posix_time::ptime timestamp;
    std::string data;
    ExtractServerPlayerChatMessageData(msg.m_message, player_id, timestamp, data);

    Client().GetClientUI().GetMultiPlayerLobbyWnd()->ChatMessage(player_id, timestamp, data);
    return discard_event();
}

boost::statechart::result MPLobby::react(const CancelMPGameClicked& a)
{
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby.CancelMPGameClicked";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    return retval;
}

boost::statechart::result MPLobby::react(const StartMPGameClicked& a) {
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby.StartMPGameClicked";

    if (Client().Networking().PlayerIsHost(Client().Networking().PlayerID()))
        Client().Networking().SendMessage(StartMPGameMessage());
    else
        ErrorLogger(FSM) << "MPLobby::react received start MP game event but this client is not the host.  Ignoring";

    return discard_event(); // wait for server response GameStart message to leave this state...
}

boost::statechart::result MPLobby::react(const GameStart& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby.GameStart";

    // need to re-post the game start message to be re-handled after
    // transitioning into WaitingForGameStart
    post_event(msg);

    Client().GetClientUI().GetMapWnd()->Sanitize();
    Client().Remove(Client().GetClientUI().GetMultiPlayerLobbyWnd());
    return transit<WaitingForGameStart>();
}

boost::statechart::result MPLobby::react(const Error& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) MPLobby.Error";
    std::string problem;
    bool fatal;
    int player_id;
    ExtractErrorMessageData(msg.m_message, player_id, problem, fatal);

    ErrorLogger(FSM) << "MPLobby::react(const Error& msg) error: " << problem;

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal) {
        Client().ResetToIntro(true);
        ClientUI::MessageBox(UserString(problem), true);
    }

    return retval;
}

boost::statechart::result MPLobby::react(const StartQuittingGame& e) {
    TraceLogger(FSM) << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}

boost::statechart::result MPLobby::react(const CheckSum& e) {
    TraceLogger(FSM) << "(HumanClientFSM) CheckSum.";
    Client().VerifyCheckSum(e.m_message);
    return discard_event();
}

boost::statechart::result MPLobby::react(const ChatHistory& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) ChatHistory.";

    std::vector<ChatHistoryEntity> chat_history;
    ExtractChatHistoryMessage(msg.m_message, chat_history);

    const auto& wnd = Client().GetClientUI().GetMultiPlayerLobbyWnd();
    for (const auto& elem : chat_history)
        wnd->ChatMessage(elem.m_text,
                         elem.m_player_name,
                         elem.m_player_name.empty() ? Client().GetClientUI().TextColor() : elem.m_text_color,
                         elem.m_timestamp);

    return discard_event();
}

boost::statechart::result MPLobby::react(const PlayerStatus& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayerStatus.";
    // ToDo: show it in player ready status

    return discard_event();
}

boost::statechart::result MPLobby::react(const SaveGameComplete& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) SaveGameComplete.";
    // ignore it

    return discard_event();
}

boost::statechart::result MPLobby::react(const TurnProgress& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) TurnProgress.";

    Message::TurnProgressPhase phase_id;
    ExtractTurnProgressMessageData(msg.m_message, phase_id);
    const auto& wnd = Client().GetClientUI().GetMultiPlayerLobbyWnd();
    wnd->TurnPhaseUpdate(phase_id);

    return discard_event();
}


////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame(my_context ctx) :
    Base(ctx)
{
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame";

    Client().Register(Client().GetClientUI().GetMapWnd());
    Client().GetClientUI().GetMapWnd()->Show();
    Client().GetClientUI().GetMessageWnd()->Clear();
}

PlayingGame::~PlayingGame() {
    TraceLogger(FSM) << "(HumanClientFSM) ~PlayingGame";
}

boost::statechart::result PlayingGame::react(const HostID& msg) {
    const int initial_host_id = Client().Networking().HostPlayerID();
    const std::string& text = msg.m_message.Text();
    int host_id = Networking::INVALID_PLAYER_ID;
    try {
        host_id = boost::lexical_cast<int>(text);
    } catch (const boost::bad_lexical_cast& ex) {
        ErrorLogger(FSM) << "PlayingGame::react(const HostID& msg) could not convert \"" << text << "\" to host id";
    }

    Client().Networking().SetHostPlayerID(host_id);

    // Logging configuration can only be sent after receiving host id.
    Client().SendLoggingConfigToServer();

    if (initial_host_id != host_id)
        DebugLogger(FSM) << "PlayingGame::react(const HostID& msg) New Host ID: " << host_id;

    return discard_event();
}

boost::statechart::result PlayingGame::react(const PlayerChat& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.PlayerChat: " << msg.m_message.Text();
    std::string text;
    int sending_player_id;
    boost::posix_time::ptime timestamp;
    ExtractServerPlayerChatMessageData(msg.m_message, sending_player_id, timestamp, text);

    std::string player_name{UserString("PLAYER") + " " + std::to_string(sending_player_id)};
    GG::Clr text_color{Client().GetClientUI().TextColor()};
    if (sending_player_id != Networking::INVALID_PLAYER_ID) {
        auto players = Client().Players();
        auto player_it = players.find(sending_player_id);
        if (player_it != players.end()) {
            player_name = player_it->second.name;
            if (auto empire = GetEmpire(player_it->second.empire_id))
                text_color = empire->Color();
        }
    } else {
        // It's a server message. Don't set player name.
        player_name = "";
    }

    Client().GetClientUI().GetMessageWnd()->HandlePlayerChatMessage(text, player_name, text_color, timestamp, Client().PlayerID());

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Disconnection& d) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.Disconnection";

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return retval;
}

boost::statechart::result PlayingGame::react(const PlayerStatus& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.PlayerStatus";
    int ignore_about_player_id;
    Message::PlayerStatus status;
    int about_empire_id;
    ExtractPlayerStatusMessageData(msg.m_message, ignore_about_player_id, status, about_empire_id);

    Client().SetEmpireStatus(about_empire_id, status);
    Client().GetClientUI().GetPlayerListWnd()->HandleEmpireStatusUpdate(status, about_empire_id);
    // TODO: tell the map wnd or something else as well?

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Diplomacy& d) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.Diplomacy";

    DiplomaticMessage diplo_message;
    ExtractDiplomacyMessageData(d.m_message, diplo_message);
    Empires().SetDiplomaticMessage(diplo_message);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const DiplomaticStatusUpdate& u) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.DiplomaticStatusUpdate";

    DiplomaticStatusUpdateInfo diplo_update;
    ExtractDiplomaticStatusMessageData(u.m_message, diplo_update);
    Empires().SetDiplomaticStatus(diplo_update.empire1_id, diplo_update.empire2_id, diplo_update.diplo_status);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const EndGame& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.EndGame";
    Message::EndGameReason reason;
    std::string reason_player_name;
    ExtractEndGameMessageData(msg.m_message, reason, reason_player_name);
    std::string reason_message;
    bool error = false;
    switch (reason) {
    case Message::LOCAL_CLIENT_DISCONNECT:
        reason_message = UserString("SERVER_LOST");
        break;
    case Message::PLAYER_DISCONNECT:
        reason_message = boost::io::str(FlexibleFormat(UserString("PLAYER_DISCONNECTED")) % reason_player_name);
        error = true;
        break;
    }

    // See reaction_transition_note.
    auto retval = discard_event();
    Client().ResetToIntro(true);
    ClientUI::MessageBox(reason_message, error);
    return retval;
}

boost::statechart::result PlayingGame::react(const StartQuittingGame& e) {
    TraceLogger(FSM) << "(HumanClientFSM) Quit or reset to main menu.";

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    post_event(e);
    return transit<QuittingGame>();
}

boost::statechart::result PlayingGame::react(const Error& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.Error";
    std::string problem;
    bool fatal;
    int player_id;
    ExtractErrorMessageData(msg.m_message, player_id, problem, fatal);

    ErrorLogger(FSM) << "PlayingGame::react(const Error& msg) error: "
                  << problem << "\nProblem is" << (fatal ? "fatal" : "non-fatal");

    // See reaction_transition_note.
    auto retval = discard_event();
    if (fatal)
        Client().ResetToIntro(true);

    ClientUI::MessageBox(UserString(problem), fatal);

    return retval;
}

boost::statechart::result PlayingGame::react(const TurnProgress& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.TurnProgress";

    Message::TurnProgressPhase phase_id;
    ExtractTurnProgressMessageData(msg.m_message, phase_id);
    Client().HandleTurnPhaseUpdate(phase_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const TurnPartialUpdate& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.TurnPartialUpdate";

    ExtractTurnPartialUpdateMessageData(msg.m_message,   Client().EmpireID(),    GetUniverse());

    Client().GetClientUI().GetMapWnd()->MidTurnUpdate();

    return discard_event();
}

boost::statechart::result PlayingGame::react(const LobbyUpdate& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.LobbyUpdate";

    // need to re-post the lobby update message to be re-handled after
    // transitioning into MPLobby
    post_event(msg);

    Client().ResetClientData(true);
    Client().GetClientUI().ShowMultiPlayerLobbyWnd();

    return transit<MPLobby>();
}


////////////////////////////////////////////////////////////
// WaitingForGameStart
////////////////////////////////////////////////////////////
WaitingForGameStart::WaitingForGameStart(my_context ctx) :
    Base(ctx)
{
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForGameStart";

    Client().Register(Client().GetClientUI().GetMessageWnd());
    Client().Register(Client().GetClientUI().GetPlayerListWnd());

    Client().GetClientUI().GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForGameStart::~WaitingForGameStart()
{ TraceLogger(FSM) << "(HumanClientFSM) ~WaitingForGameStart"; }

boost::statechart::result WaitingForGameStart::react(const GameStart& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForGameStart.GameStart";

    bool loaded_game_data;
    bool ui_data_available;
    SaveGameUIData ui_data;
    bool save_state_string_available;
    std::string save_state_string; // ignored - used by AI but not by human client
    OrderSet orders;
    bool single_player_game = false;
    int empire_id = ALL_EMPIRES;
    int current_turn = INVALID_GAME_TURN;
    Client().EmpireStatus().clear();
    Client().Orders().Reset();

    ExtractGameStartMessageData(msg.m_message,       single_player_game,             empire_id,
                                current_turn,        Empires(),                      GetUniverse(),
                                GetSpeciesManager(), GetCombatLogManager(),          GetSupplyManager(),
                                Client().Players(),  Client().Orders(),              loaded_game_data,
                                ui_data_available,   ui_data,                        save_state_string_available,
                                save_state_string,   Client().GetGalaxySetupData());

    DebugLogger(FSM) << "Extracted GameStart message for turn: " << current_turn << " with empire: " << empire_id;

    Client().SetSinglePlayerGame(single_player_game);
    Client().SetEmpireID(empire_id);
    Client().SetCurrentTurn(current_turn);

    GetGameRules().SetFromStrings(Client().GetGalaxySetupData().GetGameRules());

    bool is_new_game = !(loaded_game_data && ui_data_available);
    Client().StartGame(is_new_game);

    if (!is_new_game)
        Client().GetClientUI().RestoreFromSaveData(ui_data);

    // if I am the host on the first turn, do an autosave.
    if (is_new_game && Client().Networking().PlayerIsHost(Client().PlayerID()))
        Client().Autosave();

    Client().GetClientUI().GetPlayerListWnd()->Refresh();

    return transit<PlayingTurn>();
}


////////////////////////////////////////////////////////////
// WaitingForTurnData
////////////////////////////////////////////////////////////
WaitingForTurnData::WaitingForTurnData(my_context ctx) :
    Base(ctx)
{
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForTurnData";
    Client().GetClientUI().GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForTurnData::~WaitingForTurnData()
{ TraceLogger(FSM) << "(HumanClientFSM) ~WaitingForTurnData"; }

boost::statechart::result WaitingForTurnData::react(const SaveGameComplete& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) WaitingForTurnData.SaveGameComplete";

    std::string save_filename;
    int bytes_written;
    ExtractServerSaveGameCompleteMessageData(msg.m_message, save_filename, bytes_written);

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("SERVER_SAVE_COMPLETE")) % save_filename % bytes_written) + "\n");

    Client().SaveGameCompleted();

    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const TurnUpdate& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.TurnUpdate";

    int current_turn = INVALID_GAME_TURN;

    try {
        ExtractTurnUpdateMessageData(msg.m_message,           Client().EmpireID(),    current_turn,
                                     Empires(),               GetUniverse(),          GetSpeciesManager(),
                                     GetCombatLogManager(),   GetSupplyManager(),     Client().Players());
    } catch (...) {
        Client().GetClientUI().GetMessageWnd()->HandleLogMessage(UserString("ERROR_PROCESSING_SERVER_MESSAGE") + "\n");
        return discard_event();
    }

    DebugLogger(FSM) << "Extracted TurnUpdate message for turn: " << current_turn;

    Client().SetCurrentTurn(current_turn);

    // if I am the host, do autosave
    if (Client().Networking().PlayerIsHost(Client().PlayerID()))
        Client().Autosave();

    Client().HandleTurnUpdate();

    Client().GetClientUI().GetPlayerListWnd()->Refresh();

    return transit<PlayingTurn>();
}

boost::statechart::result WaitingForTurnData::react(const TurnRevoked& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.TurnRevoked";

    // Allow player to change orders
    return transit<PlayingTurn>();
}

boost::statechart::result WaitingForTurnData::react(const DispatchCombatLogs& msg) {
    DebugLogger(FSM) << "(PlayerFSM) WaitingForTurnData::DispatchCombatLogs message received";
    Client().UpdateCombatLogs(msg.m_message);
    return discard_event();
}


////////////////////////////////////////////////////////////
// PlayingTurn
////////////////////////////////////////////////////////////
PlayingTurn::PlayingTurn(my_context ctx) :
    Base(ctx)
{
    TraceLogger(FSM) << "(HumanClientFSM) PlayingTurn";
    Client().Register(Client().GetClientUI().GetMapWnd());
    Client().GetClientUI().GetMapWnd()->InitTurn();
    Client().GetClientUI().GetMapWnd()->RegisterWindows(); // only useful at game start but InitTurn() takes a long time, don't want to display windows before content is ready.  could go in WaitingForGameStart dtor but what if it is given e.g. an error reaction?
    // TODO: reselect last fleet if stored in save game ui data?
    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("TURN_BEGIN")) % CurrentTurn()) + "\n");
    Client().GetClientUI().GetPlayerListWnd()->HandleEmpireStatusUpdate(Message::PLAYING_TURN, Client().EmpireID());

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

        // if no auto turns left, and supposed to quit then autosave
        // which will lead to a quit when the save completes
        if (Client().AutoTurnsLeft() <= 0 && GetOptionsDB().Get<bool>("auto-quit"))
            Client().Autosave();

        // if there are still auto turns left, advance the turn automatically,
        // and decrease the auto turn counter
        if (Client().AutoTurnsLeft() > 0) {
            post_event(AdvanceTurn());
            Client().DecAutoTurns();
        }
    }
}

PlayingTurn::~PlayingTurn()
{ TraceLogger(FSM) << "(HumanClientFSM) ~PlayingTurn"; }

boost::statechart::result PlayingTurn::react(const SaveGameComplete& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingTurn.SaveGameComplete";

    std::string save_filename;
    int bytes_written;
    ExtractServerSaveGameCompleteMessageData(msg.m_message, save_filename, bytes_written);

    Client().GetClientUI().GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("SERVER_SAVE_COMPLETE")) % save_filename % bytes_written) + "\n");

    Client().SaveGameCompleted();

    // auto quit save has completed, close the app
    if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER
        && Client().AutoTurnsLeft() <= 0
        && GetOptionsDB().Get<bool>("auto-quit"))
    {
        DebugLogger(FSM) << "auto-quit save completed, ending game.";
        Client().ExitApp(0);
    }

    return discard_event();
}

boost::statechart::result PlayingTurn::react(const AdvanceTurn& d) {
    SaveGameUIData ui_data;
    Client().GetClientUI().GetSaveGameUIData(ui_data);
    Client().StartTurn(ui_data);
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const TurnUpdate& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingTurn.TurnUpdate";

     Client().GetClientUI().GetMessageWnd()->HandleLogMessage(UserString("ERROR_EARLY_TURN_UPDATE") + "\n");

    // need to re-post the game start message to be re-handled after
    // transitioning into WaitingForTurnData
    post_event(msg);

    return transit<WaitingForTurnData>();
}

boost::statechart::result PlayingTurn::react(const TurnEnded& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingTurn.TurnEnded";
    return transit<WaitingForTurnData>();
}

boost::statechart::result PlayingTurn::react(const PlayerStatus& msg) {
    TraceLogger(FSM) << "(HumanClientFSM) PlayingTurn.PlayerStatus";
    int ignore_about_player_id;
    Message::PlayerStatus status;
    int about_empire_id;
    ExtractPlayerStatusMessageData(msg.m_message, ignore_about_player_id, status, about_empire_id);

    Client().SetEmpireStatus(about_empire_id, status);
    Client().GetClientUI().GetPlayerListWnd()->HandleEmpireStatusUpdate(status, about_empire_id);

    if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR &&
        Client().GetClientUI().GetMapWnd()->AutoEndTurnEnabled())
    {
        // check status of all empires: are they all done their turns?
        bool all_participants_waiting = true;
        const auto& empire_status = Client().EmpireStatus();
        for (auto& entry : Client().Empires()) {
            int empire_id = entry.first;

            auto status_it = empire_status.find(empire_id);
            if (status_it == empire_status.end()) {
                all_participants_waiting = false;
                break;
            }
            if (status_it->second != Message::WAITING) {
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
    DebugLogger(FSM) << "(PlayerFSM) PlayingGame::DispatchCombatLogs message received";
    Client().UpdateCombatLogs(msg.m_message);
    return discard_event();
}


////////////////////////////////////////////////////////////
// QuittingGame
////////////////////////////////////////////////////////////
/** The QuittingGame state expects to start with a StartQuittingGame message posted. */
QuittingGame::QuittingGame(my_context c) :
    my_base(c),
    m_start_time(Clock::now())
{
    // Quit the game by sending a shutdown message to the server and waiting for
    // the disconnection event.  Free the server if it starts an orderly
    // shutdown, otherwise kill it.

    TraceLogger(FSM) << "(Host) QuittingGame";
}

QuittingGame::~QuittingGame()
{ TraceLogger(FSM) << "(HumanClientFSM) ~QuittingGame"; }

boost::statechart::result QuittingGame::react(const StartQuittingGame& u) {
    TraceLogger(FSM) << "(HumanClientFSM) QuittingGame";

    m_server_process = &u.m_server;
    m_after_server_shutdown_action = u.m_after_server_shutdown_action;

    post_event(ShutdownServer());
    return discard_event();
}

boost::statechart::result QuittingGame::react(const ShutdownServer& u) {
    TraceLogger(FSM) << "(HumanClientFSM) QuittingGame.ShutdownServer";

    if (!m_server_process) {
        ErrorLogger(FSM) << "m_server_process is nullptr";
        post_event(TerminateServer());
        return discard_event();
    }

    if (m_server_process->Empty()) {
        if (Client().Networking().IsTxConnected()) {
            WarnLogger(FSM) << "Disconnecting from server that is already killed.";
            Client().Networking().DisconnectFromServer();
        }
        post_event(TerminateServer());
        return discard_event();
    }

    if (Client().Networking().IsTxConnected()) {
        DebugLogger(FSM) << "Sending server shutdown message.";
        Client().Networking().SendMessage(ShutdownServerMessage());

        post_event(WaitForDisconnect());

    } else {
        post_event(TerminateServer());
    }
    return discard_event();
}

const auto QUITTING_TIMEOUT          = std::chrono::milliseconds(5000);
const auto QUITTING_POLLING_INTERVAL = std::chrono::milliseconds(10);
boost::statechart::result QuittingGame::react(const WaitForDisconnect& u) {
    TraceLogger(FSM) << "(HumanClientFSM) QuittingGame.WaitForDisconnect";

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
    TraceLogger(FSM) << "(HumanClientFSM) PlayingGame.Disconnection";

    if (m_server_process) {
        // Treat disconnection as acknowledgement of shutdown and free the
        // process to allow orderly shutdown.
        m_server_process->Free();
    } else {
        ErrorLogger(FSM) << "m_server_process is nullptr";
    }

    post_event(TerminateServer());
    return discard_event();
}

boost::statechart::result QuittingGame::react(const TerminateServer& u) {
    TraceLogger(FSM) << "(HumanClientFSM) QuittingGame.TerminateServer";

    if (m_server_process && !m_server_process->Empty()) {
        DebugLogger(FSM) << "QuittingGame terminated server process.";
        m_server_process->RequestTermination();
    }

    m_after_server_shutdown_action();

    // If m_after_server_shutdown_action() exits the app, this line will never be reached
    return transit<IntroMenu>();
}
