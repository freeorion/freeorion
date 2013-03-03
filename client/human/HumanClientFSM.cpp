#include "HumanClientFSM.h"

#include "HumanClientApp.h"
#include "../../Empire/Empire.h"
#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../network/Networking.h"
#include "../../util/MultiplayerCommon.h"
#include "../../UI/ChatWnd.h"
#include "../../UI/PlayerListWnd.h"
#include "../../UI/CombatWnd.h"
#include "../../UI/IntroScreen.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../UI/MapWnd.h"

#include <boost/format.hpp>


namespace {
    const bool TRACE_EXECUTION = true;

    void FreeCombatData(CombatData* combat_data) {
        if (!combat_data)
            return;
        delete combat_data->m_system;
        for (std::map<int, UniverseObject*>::iterator it = combat_data->m_combat_universe.begin();
             it != combat_data->m_combat_universe.end();
             ++it) {
            delete it->second;
        }
    }
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
    Logger().errorStream() << "HumanClientFSM : A " << most_derived_message_type_str << " event was passed to "
        "the HumanClientFSM.  This event is illegal in the FSM's current state.  It is being ignored.";
}


////////////////////////////////////////////////////////////
// IntroMenu
////////////////////////////////////////////////////////////
IntroMenu::IntroMenu(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) IntroMenu";
    if (GetOptionsDB().Get<bool>("tech-demo"))
        Client().Register(Client().GetClientUI()->GetCombatWnd());
    else {
        Client().Register(Client().GetClientUI()->GetIntroScreen());
        Client().Remove(Client().GetClientUI()->GetMessageWnd());
        Client().Remove(Client().GetClientUI()->GetPlayerListWnd());
    }
}

IntroMenu::~IntroMenu() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~IntroMenu";
    if (GetOptionsDB().Get<bool>("tech-demo"))
        Client().Remove(Client().GetClientUI()->GetCombatWnd());

    Client().Remove(Client().GetClientUI()->GetIntroScreen());
}

boost::statechart::result IntroMenu::react(const HostSPGameRequested& a) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) IntroMenu.HostSPGameRequested";
    return transit<WaitingForSPHostAck>();
}

boost::statechart::result IntroMenu::react(const HostMPGameRequested& a) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) IntroMenu.HostMPGameRequested";
    return transit<WaitingForMPHostAck>();
}

boost::statechart::result IntroMenu::react(const JoinMPGameRequested& a) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) IntroMenu.JoinMPGameRequested";
    return transit<WaitingForMPJoinAck>();
}


////////////////////////////////////////////////////////////
// WaitingForSPHostAck
////////////////////////////////////////////////////////////
WaitingForSPHostAck::WaitingForSPHostAck() :
    Base()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForSPHostAck"; }

WaitingForSPHostAck::~WaitingForSPHostAck()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForSPHostAck"; }

boost::statechart::result WaitingForSPHostAck::react(const HostSPGame& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForSPHostAck.HostSPGame";

    Client().Networking().SetPlayerID(msg.m_message.ReceivingPlayer());
    Client().Networking().SetHostPlayerID(msg.m_message.ReceivingPlayer());

    Client().GetClientUI()->GetMapWnd()->Sanitize();
    Client().GetClientUI()->GetMessageWnd()->Show();
    Client().GetClientUI()->GetPlayerListWnd()->Show();
    return transit<PlayingGame>();
}

boost::statechart::result WaitingForSPHostAck::react(const Error& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForSPHostAck.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    Logger().errorStream() << "WaitingForSPHostAck::react(const Error& msg) error: " << problem;
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
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPHostAck"; }

WaitingForMPHostAck::~WaitingForMPHostAck()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForMPHostAck"; }

boost::statechart::result WaitingForMPHostAck::react(const HostMPGame& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPHostAck.HostMPGame";

    Client().Networking().SetPlayerID(msg.m_message.ReceivingPlayer());
    Client().Networking().SetHostPlayerID(msg.m_message.ReceivingPlayer());

    return transit<MPLobby>();
}

boost::statechart::result WaitingForMPHostAck::react(const Error& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPHostAck.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    Logger().errorStream() << "WaitingForMPHostAck::react(const Error& msg) error: " << problem;
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
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPJoinAck"; }

WaitingForMPJoinAck::~WaitingForMPJoinAck()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForMPJoinAck"; }

boost::statechart::result WaitingForMPJoinAck::react(const JoinGame& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPJoinAck.JoinGame";

    Client().Networking().SetPlayerID(msg.m_message.ReceivingPlayer());

    return transit<MPLobby>();
}

boost::statechart::result WaitingForMPJoinAck::react(const Error& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForMPJoinAck.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    Logger().errorStream() << "WaitingForMPJoinAck::react(const Error& msg) error: " << problem;
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
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby";

    Client().Register(Client().GetClientUI()->GetMultiPlayerLobbyWnd());
}

MPLobby::~MPLobby() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~MPLobby";

    Client().Remove(Client().GetClientUI()->GetMultiPlayerLobbyWnd());
}

boost::statechart::result MPLobby::react(const Disconnection& d) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.Disconnection";
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return transit<IntroMenu>();
}

boost::statechart::result MPLobby::react(const HostID& msg) {
    const std::string& text = msg.m_message.Text();
    int host_id = Networking::INVALID_PLAYER_ID;
    if (text.empty()) {
        Logger().errorStream() << "MPLobby::react(const HostID& msg) got empty message text?!";
    } else {
        try {
            host_id = boost::lexical_cast<int>(text);
        } catch (...) {
            Logger().errorStream() << "MPLobby::react(const HostID& msg) couldn't parese message text: " << text;
        }
    }
    Client().Networking().SetHostPlayerID(host_id);

    Client().GetClientUI()->GetMultiPlayerLobbyWnd()->Refresh();

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyUpdate";
    MultiplayerLobbyData lobby_data;
    ExtractMessageData(msg.m_message, lobby_data);
    Client().GetClientUI()->GetMultiPlayerLobbyWnd()->LobbyUpdate(lobby_data);
    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyChat& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyChat";
    Client().GetClientUI()->GetMultiPlayerLobbyWnd()->ChatMessage(msg.m_message.SendingPlayer(), msg.m_message.Text());
    return discard_event();
}

boost::statechart::result MPLobby::react(const CancelMPGameClicked& a)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.CancelMPGameClicked";
    HumanClientApp::GetApp()->Networking().DisconnectFromServer();
    return transit<IntroMenu>();
}

boost::statechart::result MPLobby::react(const StartMPGameClicked& a) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.StartMPGameClicked";

    if (Client().Networking().PlayerIsHost(Client().Networking().PlayerID()))
        Client().Networking().SendMessage(StartMPGameMessage(Client().PlayerID()));
    else
        Logger().errorStream() << "MPLobby::react received start MP game event but this client is not the host.  Ignoring";

    return discard_event(); // wait for server response GameStart message to leave this state...
}

boost::statechart::result MPLobby::react(const GameStart& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.GameStart";

    // need to re-post the game start message to be re-handled after
    // transitioning into WaitingForGameStart
    post_event(msg);

    Client().GetClientUI()->GetMapWnd()->Sanitize();
    Client().GetClientUI()->GetMessageWnd()->Show();
    Client().GetClientUI()->GetPlayerListWnd()->Show();

    return transit<WaitingForGameStart>();
}

boost::statechart::result MPLobby::react(const Error& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    Logger().errorStream() << "MPLobby::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
}


////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame() :
    Base()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame"; }

PlayingGame::~PlayingGame() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~PlayingGame";
    Client().Remove(Client().GetClientUI()->GetMapWnd());
    Client().Networking().SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    Client().Networking().SetPlayerID(Networking::INVALID_PLAYER_ID);
}

boost::statechart::result PlayingGame::react(const HostID& msg) {
    const int initial_host_id = Client().Networking().HostPlayerID();
    const std::string& text = msg.m_message.Text();
    int host_id = Networking::INVALID_PLAYER_ID;
    if (text.empty()) {
        Logger().errorStream() << "PlayingGame::react(const HostID& msg) got empty message text?!";
    } else {
        try {
            host_id = boost::lexical_cast<int>(text);
        } catch (...) {
            Logger().errorStream() << "PlayingGame::react(const HostID& msg) couldn't parese message text: " << text;
        }
    }
    Client().Networking().SetHostPlayerID(host_id);

    if (initial_host_id != host_id)
        Logger().debugStream() << "PlayingGame::react(const HostID& msg) New Host ID: " << host_id;

    return discard_event();
}

boost::statechart::result PlayingGame::react(const PlayerChat& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.PlayerChat";
    const std::string& text = msg.m_message.Text();
    int sending_player_id = msg.m_message.SendingPlayer();
    int recipient_player_id = msg.m_message.ReceivingPlayer();

    Client().GetClientUI()->GetMessageWnd()->HandlePlayerChatMessage(text, sending_player_id, recipient_player_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Disconnection& d) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.Disconnection";
    Client().EndGame(true);
    ClientUI::MessageBox(UserString("SERVER_LOST"), true);
    return transit<IntroMenu>();
}

boost::statechart::result PlayingGame::react(const PlayerStatus& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.PlayerStatus";
    int about_player_id;
    Message::PlayerStatus status;
    ExtractMessageData(msg.m_message, about_player_id, status);

    Client().GetClientUI()->GetMessageWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    Client().GetClientUI()->GetPlayerListWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    // TODO: tell the map wnd or something else as well?

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Diplomacy& d) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.Diplomacy";

    DiplomaticMessage diplo_message;
    ExtractMessageData(d.m_message, diplo_message);
    Empires().SetDiplomaticMessage(diplo_message);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const DiplomaticStatusUpdate& u) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.DiplomaticStatusUpdate";

    DiplomaticStatusUpdateInfo diplo_update;
    ExtractMessageData(u.m_message, diplo_update);
    Empires().SetDiplomaticStatus(diplo_update.empire1_id, diplo_update.empire2_id, diplo_update.diplo_status);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const VictoryDefeat& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.VictoryDefeat";
    Message::VictoryOrDefeat victory_or_defeat;
    std::string reason_string;
    int empire_id;
    ExtractMessageData(msg.m_message, victory_or_defeat, reason_string, empire_id);

    const Empire* empire = Empires().Lookup(empire_id);
    std::string empire_name = UserString("UNKNOWN_EMPIRE");
    if (empire)
        empire_name = empire->Name();

    //ClientUI::MessageBox(boost::io::str(FlexibleFormat(UserString(reason_string)) % empire_name));
    return discard_event();
}

boost::statechart::result PlayingGame::react(const PlayerEliminated& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.PlayerEliminated";
    int empire_id;
    std::string empire_name;
    ExtractMessageData(msg.m_message, empire_id, empire_name);
    Client().EmpireEliminatedSignal(empire_id);
    // TODO: replace this with something better
    //ClientUI::MessageBox(boost::io::str(FlexibleFormat(UserString("EMPIRE_DEFEATED")) % empire_name));
    return discard_event();
}

boost::statechart::result PlayingGame::react(const EndGame& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.EndGame";
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
    return transit<IntroMenu>();
}

boost::statechart::result PlayingGame::react(const ResetToIntroMenu& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.ResetToIntroMenu";

    Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

    return transit<IntroMenu>();
}

boost::statechart::result PlayingGame::react(const Error& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    Logger().errorStream() << "PlayingGame::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
}

boost::statechart::result PlayingGame::react(const TurnProgress& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.TurnProgress";

    Message::TurnProgressPhase phase_id;
    ExtractMessageData(msg.m_message, phase_id);
    Client().GetClientUI()->GetMessageWnd()->HandleTurnPhaseUpdate(phase_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const TurnPartialUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.TurnPartialUpdate";

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
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForGameStart";

    Client().Register(Client().GetClientUI()->GetMessageWnd());
    Client().Register(Client().GetClientUI()->GetPlayerListWnd());
    Client().GetClientUI()->GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForGameStart::~WaitingForGameStart()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForGameStart"; }

boost::statechart::result WaitingForGameStart::react(const GameStart& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForGameStart.GameStart";

    bool loaded_game_data;
    bool ui_data_available;
    SaveGameUIData ui_data;
    bool save_state_string_available;
    std::string save_state_string; // ignored - used by AI but not by human client
    OrderSet orders;
    bool single_player_game = false;
    int empire_id = ALL_EMPIRES;
    int current_turn = INVALID_GAME_TURN;

    ExtractMessageData(msg.m_message,               single_player_game,     empire_id,
                       current_turn,                Empires(),              GetUniverse(),
                       GetSpeciesManager(),         Client().Players(),     orders,
                       loaded_game_data,            ui_data_available,      ui_data,
                       save_state_string_available, save_state_string);

    Logger().debugStream() << "Extracted GameStart message for turn: " << current_turn << " with empire: " << empire_id;

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
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData";

    Client().Register(Client().GetClientUI()->GetMessageWnd());
    Client().Register(Client().GetClientUI()->GetPlayerListWnd());
    Client().GetClientUI()->GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForTurnData::~WaitingForTurnData()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForTurnData"; }

boost::statechart::result WaitingForTurnData::react(const CombatStart& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.CombatStart";
    post_event(msg); // Re-post this event, so that it is the first thing the ResolvingCombat state sees.
    return transit<ResolvingCombat>();
}

boost::statechart::result WaitingForTurnData::react(const SaveGame& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.SaveGame";
    Client().HandleSaveGameDataRequest();
    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const TurnUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.TurnUpdate";

    int current_turn = INVALID_GAME_TURN;

    try {
        ExtractMessageData(msg.m_message,   Client().EmpireID(),    current_turn,
                           Empires(),       GetUniverse(),          GetSpeciesManager(),
                           Client().Players());
    } catch (...) {
        Client().GetClientUI()->GetMessageWnd()->HandleLogMessage(UserString("ERROR_PROCESSING_SERVER_MESSAGE") + "\n");
        return discard_event();
    }

    Logger().debugStream() << "Extracted TurnUpdate message for turn: " << current_turn;

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
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn";
    Client().Register(Client().GetClientUI()->GetMapWnd());
    Client().GetClientUI()->GetMapWnd()->InitTurn();
    // TODO: reselect last fleet if stored in save game ui data?
    Client().GetClientUI()->GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("TURN_BEGIN")) % CurrentTurn()) + "\n");
    Client().GetClientUI()->GetMessageWnd()->HandlePlayerStatusUpdate(Message::PLAYING_TURN, Client().PlayerID());
    Client().GetClientUI()->GetPlayerListWnd()->Refresh();
    Client().GetClientUI()->GetPlayerListWnd()->HandlePlayerStatusUpdate(Message::PLAYING_TURN, Client().PlayerID());
    Client().GetClientUI()->GetMapWnd()->EnableOrderIssuing(true);   // MapWnd

    // observers can't do anything but wait for the next update, and need to
    // be back in WaitingForTurnData, so posting TurnEnded here has the effect
    // of keeping observers in the WaitingForTurnData state so they can receive
    // updates from the server.
    if (Client().GetApp()->GetPlayerClientType(Client().PlayerID()) == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
        post_event(TurnEnded());

    else if (GetOptionsDB().Get<bool>("auto-advance-first-turn")) {
        static bool once = true;
        if (once) {
            post_event(AutoAdvanceFirstTurn());
            once = false;
        }
    }
}

PlayingTurn::~PlayingTurn()
{ if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~PlayingTurn"; }

boost::statechart::result PlayingTurn::react(const SaveGame& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn.SaveGame";
    Client().HandleSaveGameDataRequest();
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const AutoAdvanceFirstTurn& d) {
    Client().GetClientUI()->GetMapWnd()->m_turn_update->ClickedSignal();
    return discard_event();
}

boost::statechart::result PlayingTurn::react(const TurnUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn.TurnUpdate";

     Client().GetClientUI()->GetMessageWnd()->HandleLogMessage(UserString("ERROR_EARLY_TURN_UPDATE") + "\n");

    // need to re-post the game start message to be re-handled after
    // transitioning into WaitingForTurnData
    post_event(msg);

    return transit<WaitingForTurnData>();
}

boost::statechart::result PlayingTurn::react(const TurnEnded& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn.TurnEnded";
    return transit<WaitingForTurnData>();
}


////////////////////////////////////////////////////////////
// ResolvingCombat
////////////////////////////////////////////////////////////
ResolvingCombat::ResolvingCombat(my_context ctx) :
    Base(ctx),
    m_previous_combat_data(),
    m_combat_data(new CombatData),
    m_combat_wnd(new CombatWnd(Client().SceneManager(), Client().Camera(), Client().Viewport()))
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat";
    Client().Register(m_combat_wnd.get());
    Client().GetClientUI()->GetMapWnd()->Hide();
}

ResolvingCombat::~ResolvingCombat() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~ResolvingCombat";
    Client().GetClientUI()->GetMapWnd()->Show();
    FreeCombatData(m_previous_combat_data.get());
    FreeCombatData(m_combat_data.get());
}

boost::statechart::result ResolvingCombat::react(const CombatStart& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat.CombatStart";
    std::vector<CombatSetupGroup> setup_groups;
    Universe::ShipDesignMap foreign_designs;
    ExtractMessageData(msg.m_message, *m_combat_data, setup_groups, foreign_designs);
    for (Universe::ShipDesignMap::const_iterator it = foreign_designs.begin();
         it != foreign_designs.end(); ++it)
    { GetUniverse().InsertShipDesignID(it->second, it->first); }
    m_combat_wnd->InitCombat(*m_combat_data, setup_groups);
    return discard_event();
}

boost::statechart::result ResolvingCombat::react(const CombatRoundUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat.CombatRoundUpdate";
    if (m_previous_combat_data.get()) {
        FreeCombatData(m_previous_combat_data.get());
        m_previous_combat_data.release();
    }
    m_previous_combat_data = m_combat_data;
    m_combat_data.reset(new CombatData);
    ExtractMessageData(msg.m_message, *m_combat_data);
    m_combat_wnd->CombatTurnUpdate(*m_combat_data);
    return discard_event();
}

boost::statechart::result ResolvingCombat::react(const CombatEnd& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat.CombatEnd";
    return transit<WaitingForTurnData>();
}
