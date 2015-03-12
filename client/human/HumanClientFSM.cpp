#include "HumanClientFSM.h"

#include "HumanClientApp.h"
#include "../../Empire/Empire.h"
#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../network/Networking.h"
#include "../../util/i18n.h"
#include "../../util/MultiplayerCommon.h"
#include "../../UI/ChatWnd.h"
#include "../../UI/PlayerListWnd.h"
#include "../../UI/IntroScreen.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../UI/MapWnd.h"

#include <boost/format.hpp>

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
    Client().KillServer();

    Client().Remove(Client().GetClientUI()->GetMapWnd());
    Client().GetClientUI()->GetMapWnd()->Hide();

    Client().Register(Client().GetClientUI()->GetIntroScreen());
    Client().GetClientUI()->GetMapWnd()->HideMessages();
    Client().GetClientUI()->GetMapWnd()->HideEmpires();
}

IntroMenu::~IntroMenu() {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~IntroMenu";
}

boost::statechart::result IntroMenu::react(const HostSPGameRequested& a) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) IntroMenu.HostSPGameRequested";
    Client().Remove(Client().GetClientUI()->GetIntroScreen());
    return transit<WaitingForSPHostAck>();
}

boost::statechart::result IntroMenu::react(const HostMPGameRequested& a) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) IntroMenu.HostMPGameRequested";
    Client().Remove(Client().GetClientUI()->GetIntroScreen());
    return transit<WaitingForMPHostAck>();
}

boost::statechart::result IntroMenu::react(const JoinMPGameRequested& a) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) IntroMenu.JoinMPGameRequested";
    Client().Remove(Client().GetClientUI()->GetIntroScreen());
    return transit<WaitingForMPJoinAck>();
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

    Client().GetClientUI()->GetMapWnd()->Sanitize();

    return transit<PlayingGame>();
}

boost::statechart::result WaitingForSPHostAck::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForSPHostAck.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "WaitingForSPHostAck::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
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

boost::statechart::result WaitingForMPHostAck::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPHostAck.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "WaitingForMPHostAck::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
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

boost::statechart::result WaitingForMPJoinAck::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForMPJoinAck.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "WaitingForMPJoinAck::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
}


////////////////////////////////////////////////////////////
// MPLobby
////////////////////////////////////////////////////////////
MPLobby::MPLobby(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby";

    Client().Register(Client().GetClientUI()->GetMultiPlayerLobbyWnd());
}

MPLobby::~MPLobby() {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~MPLobby";
}

boost::statechart::result MPLobby::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.Disconnection";
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    Client().Remove(Client().GetClientUI()->GetMultiPlayerLobbyWnd());
    return transit<IntroMenu>();
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

    Client().GetClientUI()->GetMultiPlayerLobbyWnd()->Refresh();

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.LobbyUpdate";
    MultiplayerLobbyData lobby_data;
    ExtractMessageData(msg.m_message, lobby_data);
    Client().GetClientUI()->GetMultiPlayerLobbyWnd()->LobbyUpdate(lobby_data);
    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyChat& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.LobbyChat";
    Client().GetClientUI()->GetMultiPlayerLobbyWnd()->ChatMessage(msg.m_message.SendingPlayer(), msg.m_message.Text());
    return discard_event();
}

boost::statechart::result MPLobby::react(const CancelMPGameClicked& a)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.CancelMPGameClicked";
    HumanClientApp::GetApp()->Networking().DisconnectFromServer();
    Client().Remove(Client().GetClientUI()->GetMultiPlayerLobbyWnd());
    return transit<IntroMenu>();
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

    Client().GetClientUI()->GetMapWnd()->Sanitize();
    Client().Remove(Client().GetClientUI()->GetMultiPlayerLobbyWnd());
    return transit<WaitingForGameStart>();
}

boost::statechart::result MPLobby::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) MPLobby.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "MPLobby::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        Client().Remove(Client().GetClientUI()->GetMultiPlayerLobbyWnd());
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
}


////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame";

    Client().Register(Client().GetClientUI()->GetMapWnd());
    Client().GetClientUI()->GetMapWnd()->Show();
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

    Client().GetClientUI()->GetMessageWnd()->HandlePlayerChatMessage(text, sending_player_id, recipient_player_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Disconnection";
    Client().EndGame(true);
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    Client().Remove(Client().GetClientUI()->GetMapWnd());
    Client().Networking().SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    Client().Networking().SetPlayerID(Networking::INVALID_PLAYER_ID);
    return transit<IntroMenu>();
}

boost::statechart::result PlayingGame::react(const PlayerStatus& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.PlayerStatus";
    int about_player_id;
    Message::PlayerStatus status;
    ExtractMessageData(msg.m_message, about_player_id, status);

    Client().SetPlayerStatus(about_player_id, status);
    Client().GetClientUI()->GetMessageWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    Client().GetClientUI()->GetPlayerListWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    // TODO: tell the map wnd or something else as well?

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Diplomacy& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Diplomacy";

    DiplomaticMessage diplo_message;
    ExtractMessageData(d.m_message, diplo_message);
    Empires().SetDiplomaticMessage(diplo_message);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const DiplomaticStatusUpdate& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.DiplomaticStatusUpdate";

    DiplomaticStatusUpdateInfo diplo_update;
    ExtractMessageData(u.m_message, diplo_update);
    Empires().SetDiplomaticStatus(diplo_update.empire1_id, diplo_update.empire2_id, diplo_update.diplo_status);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const VictoryDefeat& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.VictoryDefeat";
    Message::VictoryOrDefeat victory_or_defeat;
    std::string reason_string;
    int empire_id;
    ExtractMessageData(msg.m_message, victory_or_defeat, reason_string, empire_id);

    const Empire* empire = GetEmpire(empire_id);
    std::string empire_name = UserString("UNKNOWN_EMPIRE");
    if (empire)
        empire_name = empire->Name();

    //ClientUI::MessageBox(boost::io::str(FlexibleFormat(UserString(reason_string)) % empire_name));
    return discard_event();
}

boost::statechart::result PlayingGame::react(const PlayerEliminated& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.PlayerEliminated";
    int empire_id;
    std::string empire_name;
    ExtractMessageData(msg.m_message, empire_id, empire_name);
    Client().EmpireEliminatedSignal(empire_id);
    // TODO: replace this with something better
    //ClientUI::MessageBox(boost::io::str(FlexibleFormat(UserString("EMPIRE_DEFEATED")) % empire_name));
    return discard_event();
}

boost::statechart::result PlayingGame::react(const EndGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.EndGame";
    Message::EndGameReason reason;
    std::string reason_player_name;
    ExtractMessageData(msg.m_message, reason, reason_player_name);
    std::string reason_message;
    bool error = false;
    switch (reason) {
    case Message::LOCAL_CLIENT_DISCONNECT:
        Client().EndGame(true);
        reason_message = UserString("SERVER_LOST");
        break;
    case Message::PLAYER_DISCONNECT:
        Client().EndGame(true);
        reason_message = boost::io::str(FlexibleFormat(UserString("PLAYER_DISCONNECTED")) % reason_player_name);
        error = true;
        break;
    case Message::YOU_ARE_ELIMINATED:
        Client().EndGame(true);
        reason_message = UserString("PLAYER_DEFEATED");
        break;
    }
    ClientUI::MessageBox(reason_message, error);
    Client().Remove(Client().GetClientUI()->GetMapWnd());
    Client().Networking().SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    Client().Networking().SetPlayerID(Networking::INVALID_PLAYER_ID);
    return transit<IntroMenu>();
}

boost::statechart::result PlayingGame::react(const ResetToIntroMenu& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.ResetToIntroMenu";

    Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
    Client().Remove(Client().GetClientUI()->GetMapWnd());
    Client().Networking().SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    Client().Networking().SetPlayerID(Networking::INVALID_PLAYER_ID);
    return transit<IntroMenu>();
}

boost::statechart::result PlayingGame::react(const Error& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    ErrorLogger() << "PlayingGame::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
        Client().Remove(Client().GetClientUI()->GetMapWnd());
        Client().Networking().SetHostPlayerID(Networking::INVALID_PLAYER_ID);
        Client().Networking().SetPlayerID(Networking::INVALID_PLAYER_ID);
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
}

boost::statechart::result PlayingGame::react(const TurnProgress& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.TurnProgress";

    Message::TurnProgressPhase phase_id;
    ExtractMessageData(msg.m_message, phase_id);
    Client().GetClientUI()->GetMessageWnd()->HandleTurnPhaseUpdate(phase_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const TurnPartialUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.TurnPartialUpdate";

    ExtractMessageData(msg.m_message,   Client().EmpireID(),    GetUniverse());

    Client().GetClientUI()->GetMapWnd()->MidTurnUpdate();

    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForGameStart
////////////////////////////////////////////////////////////
WaitingForGameStart::WaitingForGameStart(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForGameStart";

    Client().GetClientUI()->GetMapWnd()->ShowMessages();
    Client().GetClientUI()->GetMapWnd()->ShowEmpires();

    Client().GetClientUI()->GetMapWnd()->EnableOrderIssuing(false);
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

    ExtractMessageData(msg.m_message,       single_player_game,             empire_id,
                       current_turn,        Empires(),                      GetUniverse(),
                       GetSpeciesManager(), GetCombatLogManager(),          Client().Players(),
                       orders,              loaded_game_data,               ui_data_available,
                       ui_data,             save_state_string_available,    save_state_string,
                       Client().GetGalaxySetupData());

    DebugLogger() << "Extracted GameStart message for turn: " << current_turn << " with empire: " << empire_id;

    Client().SetSinglePlayerGame(single_player_game);
    Client().SetEmpireID(empire_id);
    Client().SetCurrentTurn(current_turn);

    Client().StartGame();
    std::swap(Client().Orders(), orders); // bring back orders planned in the current turn, they will be applied later, after some basic turn initialization
    if (loaded_game_data && ui_data_available)
        Client().GetClientUI()->RestoreFromSaveData(ui_data);

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
    Client().GetClientUI()->GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForTurnData::~WaitingForTurnData()
{ if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) ~WaitingForTurnData"; }

boost::statechart::result WaitingForTurnData::react(const SaveGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) WaitingForTurnData.SaveGame";
    Client().HandleSaveGameDataRequest();
    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const TurnUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingGame.TurnUpdate";

    int current_turn = INVALID_GAME_TURN;

    try {
        ExtractMessageData(msg.m_message,           Client().EmpireID(),    current_turn,
                           Empires(),               GetUniverse(),          GetSpeciesManager(),
                           GetCombatLogManager(),   Client().Players());
    } catch (...) {
        Client().GetClientUI()->GetMessageWnd()->HandleLogMessage(UserString("ERROR_PROCESSING_SERVER_MESSAGE") + "\n");
        return discard_event();
    }

    DebugLogger() << "Extracted TurnUpdate message for turn: " << current_turn;

    Client().SetCurrentTurn(current_turn);

    // if I am the host, do autosave
    if (Client().Networking().PlayerIsHost(Client().PlayerID()))
        Client().Autosave();

    return transit<PlayingTurn>();
}


////////////////////////////////////////////////////////////
// PlayingTurn
////////////////////////////////////////////////////////////
PlayingTurn::PlayingTurn(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn";
    Client().Register(Client().GetClientUI()->GetMapWnd());
    Client().GetClientUI()->GetMapWnd()->InitTurn();
    // TODO: reselect last fleet if stored in save game ui data?
    Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("TURN_BEGIN")) % CurrentTurn()) + "\n");
    Client().GetClientUI()->GetMessageWnd()->HandlePlayerStatusUpdate(Message::PLAYING_TURN, Client().PlayerID());
    Client().GetClientUI()->GetPlayerListWnd()->Refresh();
    Client().GetClientUI()->GetPlayerListWnd()->HandlePlayerStatusUpdate(Message::PLAYING_TURN, Client().PlayerID());

    if (Client().GetApp()->GetClientType() != Networking::CLIENT_TYPE_HUMAN_OBSERVER)
        Client().GetClientUI()->GetMapWnd()->EnableOrderIssuing(true);

    if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER) {
        // observers can't do anything but wait for the next update, and need to
        // be back in WaitingForTurnData, so posting TurnEnded here has the effect
        // of keeping observers in the WaitingForTurnData state so they can receive
        // updates from the server.
        post_event(TurnEnded());

    } else if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
        if (Client().GetClientUI()->GetMapWnd()->AutoEndTurnEnabled()) {
            // if in-game-GUI auto turn advance enabled, set auto turn counter to 1
            Client().InitAutoTurns(1);
        }

        if (Client().AutoTurnsLeft() <= 0 &&
            GetOptionsDB().Get<bool>("auto-quit"))
        {
            // if no auto turns left, and supposed to quit after that, quit
            DebugLogger() << "auto-quit ending game.";
            std::cout << "auto-quit ending game." << std::endl;
            Client().EndGame(true);
            throw HumanClientApp::CleanQuit();
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

boost::statechart::result PlayingTurn::react(const SaveGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn.SaveGame";
    Client().HandleSaveGameDataRequest();
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const AdvanceTurn& d) {
    Client().GetClientUI()->GetMapWnd()->m_btn_turn->LeftClickedSignal();
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const TurnUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(HumanClientFSM) PlayingTurn.TurnUpdate";

     Client().GetClientUI()->GetMessageWnd()->HandleLogMessage(UserString("ERROR_EARLY_TURN_UPDATE") + "\n");

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
    Message::PlayerStatus status;
    ExtractMessageData(msg.m_message, about_player_id, status);

    Client().SetPlayerStatus(about_player_id, status);
    Client().GetClientUI()->GetMessageWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    Client().GetClientUI()->GetPlayerListWnd()->HandlePlayerStatusUpdate(status, about_player_id);

    if (Client().GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR &&
        Client().GetClientUI()->GetMapWnd()->AutoEndTurnEnabled())
    {
        // check status of all non-mod non-obs players: are they all done their turns?
        bool all_participants_waiting = true;
        const std::map<int, Message::PlayerStatus>& player_status = Client().PlayerStatus();
        const std::map<int, PlayerInfo>& players = Client().Players();
        for (std::map<int, PlayerInfo>::const_iterator it = players.begin();
                it != players.end(); ++it)
        {
            int player_id = it->first;

            if (it->second.client_type != Networking::CLIENT_TYPE_AI_PLAYER &&
                it->second.client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER)
            { continue; }   // only active participants matter...

            std::map<int, Message::PlayerStatus>::const_iterator status_it = player_status.find(player_id);
            if (status_it == player_status.end()) {
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

