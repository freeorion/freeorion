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
    m_client(human_client),
    m_next_waiting_for_data_mode(WAITING_FOR_NEW_TURN)
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
        Client().Register(Client().m_ui->GetCombatWnd());
    else {
        Client().Register(Client().m_ui->GetIntroScreen());
        Client().Remove(Client().m_ui->GetMessageWnd());
        Client().Remove(Client().m_ui->GetPlayerListWnd());
    }
}

IntroMenu::~IntroMenu() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~IntroMenu";
    if (GetOptionsDB().Get<bool>("tech-demo"))
        Client().Remove(Client().m_ui->GetCombatWnd());

    Client().Remove(Client().m_ui->GetIntroScreen());
}

boost::statechart::result IntroMenu::react(const HostSPGameRequested& a) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) IntroMenu.HostSPGameRequested";
    context<HumanClientFSM>().m_next_waiting_for_data_mode = a.m_waiting_for_data_mode;
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

    Client().m_networking.SetPlayerID(msg.m_message.ReceivingPlayer());
    Client().m_networking.SetHostPlayerID(msg.m_message.ReceivingPlayer());

    Client().m_ui->GetMapWnd()->Sanitize();
    Client().m_ui->GetMessageWnd()->Show();
    Client().m_ui->GetPlayerListWnd()->Show();
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
        //Client().m_ui->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
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

    Client().m_networking.SetPlayerID(msg.m_message.ReceivingPlayer());
    Client().m_networking.SetHostPlayerID(msg.m_message.ReceivingPlayer());

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
        //Client().m_ui->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
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

    Client().m_networking.SetPlayerID(msg.m_message.ReceivingPlayer());

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
        //Client().m_ui->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
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

    Client().Register(Client().m_ui->GetMultiPlayerLobbyWnd());
}

MPLobby::~MPLobby() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~MPLobby";

    Client().Remove(Client().m_ui->GetMultiPlayerLobbyWnd());
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
    Client().m_networking.SetHostPlayerID(host_id);

    Client().m_ui->GetMultiPlayerLobbyWnd()->Refresh();

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyUpdate";
    MultiplayerLobbyData lobby_data;
    ExtractMessageData(msg.m_message, lobby_data);
    Client().m_ui->GetMultiPlayerLobbyWnd()->LobbyUpdate(lobby_data);
    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyChat& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.LobbyChat";
    Client().m_ui->GetMultiPlayerLobbyWnd()->ChatMessage(msg.m_message.SendingPlayer(), msg.m_message.Text());
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
    // transitioning into PlayingGame (and thus WaitingForTurnData)
    post_event(msg);

    if (Client().m_ui->GetMultiPlayerLobbyWnd()->LoadGameSelected())
        context<HumanClientFSM>().m_next_waiting_for_data_mode = WAITING_FOR_LOADED_GAME;
    else
        context<HumanClientFSM>().m_next_waiting_for_data_mode = WAITING_FOR_NEW_GAME;

    Client().m_ui->GetMapWnd()->Sanitize();
    Client().m_ui->GetMessageWnd()->Show();
    Client().m_ui->GetPlayerListWnd()->Show();

    return transit<PlayingGame>();
}

boost::statechart::result MPLobby::react(const Error& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) MPLobby.Error";
    std::string problem;
    bool fatal;
    ExtractMessageData(msg.m_message, problem, fatal);

    Logger().errorStream() << "MPLobby::react(const Error& msg) error: " << problem;
    ClientUI::MessageBox(UserString(problem), true);

    if (fatal) {
        //Client().m_ui->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
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
    Client().Remove(Client().m_ui->GetMapWnd());
    Client().Networking().SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    Client().Networking().SetPlayerID(Networking::INVALID_PLAYER_ID);
}

boost::statechart::result PlayingGame::react(const HostID& msg) {
    const int initial_host_id = Client().m_networking.HostPlayerID();
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
    Client().m_networking.SetHostPlayerID(host_id);

    if (initial_host_id != host_id)
        Logger().debugStream() << "PlayingGame::react(const HostID& msg) New Host ID: " << host_id;

    return discard_event();
}

boost::statechart::result PlayingGame::react(const PlayerChat& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingGame.PlayerChat";
    const std::string& text = msg.m_message.Text();
    int sending_player_id = msg.m_message.SendingPlayer();
    int recipient_player_id = msg.m_message.ReceivingPlayer();

    Client().m_ui->GetMessageWnd()->HandlePlayerChatMessage(text, sending_player_id, recipient_player_id);

    return discard_event();
}

boost::statechart::result PlayingGame::react(const Disconnection& d)
{
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

    Client().m_ui->GetMessageWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    Client().m_ui->GetPlayerListWnd()->HandlePlayerStatusUpdate(status, about_player_id);
    // TODO: tell the map wnd or something else as well?

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

    Client().m_ui->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");

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
        Client().m_ui->GetMessageWnd()->HandleGameStatusUpdate(UserString("RETURN_TO_INTRO") + "\n");
        return transit<IntroMenu>();
    } else {
        return discard_event();
    }
}


////////////////////////////////////////////////////////////
// WaitingForTurnData
////////////////////////////////////////////////////////////
WaitingForTurnData::WaitingForTurnData(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData";

    Client().Register(Client().m_ui->GetMessageWnd());
    Client().Register(Client().m_ui->GetPlayerListWnd());

    if (context<HumanClientFSM>().m_next_waiting_for_data_mode = WAITING_FOR_NEW_TURN)
        Client().m_ui->GetMapWnd()->EnableOrderIssuing(false);
}

WaitingForTurnData::~WaitingForTurnData() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~WaitingForTurnData";
    context<HumanClientFSM>().m_next_waiting_for_data_mode = WAITING_FOR_NEW_TURN;
}

boost::statechart::result WaitingForTurnData::react(const TurnProgress& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.TurnProgress";

    Message::TurnProgressPhase phase_id;
    ExtractMessageData(msg.m_message, phase_id);
    Client().m_ui->GetMessageWnd()->HandleTurnPhaseUpdate(phase_id);

    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const TurnUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.TurnUpdate";

    ExtractMessageData(msg.m_message,   Client().EmpireID(),    Client().CurrentTurnRef(),
                       Empires(),       GetUniverse(),          GetSpeciesManager(),
                       Client().m_player_info);

    // if I am the host, do autosave
    for (std::map<int, PlayerInfo>::const_iterator it = Client().m_player_info.begin(); it != Client().m_player_info.end(); ++it)
        if (it->second.host && it->first == Client().PlayerID())
            Client().Autosave();

    return transit<PlayingTurn>();
}

boost::statechart::result WaitingForTurnData::react(const TurnPartialUpdate& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.TurnPartialUpdate";

    ExtractMessageData(msg.m_message,   Client().EmpireID(),    GetUniverse());

    Client().m_ui->GetMapWnd()->MidTurnUpdate();

    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const CombatStart& msg) {
    // HACK! I get some long and inscrutable error message if
    // WaitingForTurnData doesn't have a CombatStart handler, even though
    // WaitingForTurnDataImpl actually handles this message.
    assert(!"Function WaitingForTurnData.CombatStart should never be called!");
    return discard_event();
}

boost::statechart::result WaitingForTurnData::react(const GameStart& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.GameStart";
    bool loaded_game_data;
    bool ui_data_available;
    SaveGameUIData ui_data;
    bool save_state_string_available;
    std::string save_state_string; // ignored - used by AI but not by human client
    OrderSet orders;

    ExtractMessageData(msg.m_message,               Client().m_single_player_game,      Client().EmpireIDRef(),
                       Client().CurrentTurnRef(),   Empires(),                          GetUniverse(),
                       GetSpeciesManager(),         Client().m_player_info,             orders,
                       loaded_game_data,            ui_data_available,                  ui_data,
                       save_state_string_available, save_state_string);

    Client().StartGame();
    std::swap(Client().Orders(), orders); // bring back orders planned in the current turn, they will be applied later, after some basic turn initialization
    if (loaded_game_data && ui_data_available)
        Client().m_ui->RestoreFromSaveData(ui_data);

    // if I am the host, do autosave
    if (Client().Networking().PlayerIsHost(Client().PlayerID()))
            Client().Autosave();

    return transit<PlayingTurn>();
}

boost::statechart::result WaitingForTurnData::react(const SaveGame& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnData.SaveGame";
    Client().HandleSaveGameDataRequest();
    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForTurnDataIdle
////////////////////////////////////////////////////////////
WaitingForTurnDataIdle::WaitingForTurnDataIdle(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION)
        Logger().debugStream() << "(HumanClientFSM) WaitingForTurnDataIdle";
}

WaitingForTurnDataIdle::~WaitingForTurnDataIdle() {
    if (TRACE_EXECUTION)
        Logger().debugStream() << "(HumanClientFSM) ~WaitingForTurnDataIdle";
}

boost::statechart::result WaitingForTurnDataIdle::react(const CombatStart& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) WaitingForTurnDataIdle.CombatStart";
    post_event(msg); // Re-post this event, so that it is the first thing the ResolvingCombat state sees.
    return transit<ResolvingCombat>();
}


////////////////////////////////////////////////////////////
// PlayingTurn
////////////////////////////////////////////////////////////
PlayingTurn::PlayingTurn(my_context ctx) :
    Base(ctx)
{
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn";
    Client().Register(Client().m_ui->GetMapWnd());
    Client().m_ui->GetMapWnd()->InitTurn();
    // TODO: reselect last fleet if stored in save game ui data?
    Client().m_ui->GetMessageWnd()->HandleGameStatusUpdate(
        boost::io::str(FlexibleFormat(UserString("TURN_BEGIN")) % CurrentTurn()) + "\n");
    Client().m_ui->GetPlayerListWnd()->Refresh();
    Client().m_ui->GetMapWnd()->EnableOrderIssuing(Client().EmpireID() != ALL_EMPIRES);

    // if not controlling an empire, the player (observer) can't do anything
    // other than waiting for more turn updates.  Turn updates received when not
    // in WaitingForTurnData state will be ignored, and the Turn button is
    // disabled.  So, posting TurnEnded here has the effect of automatically
    // keeping observers in the WaitingForTurnData state so they can receive
    // updates from the server.
    if (Client().EmpireID() == ALL_EMPIRES)
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

boost::statechart::result PlayingTurn::react(const TurnEnded& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) PlayingTurn.TurnEnded";
    return transit<WaitingForTurnData>();
}

boost::statechart::result PlayingTurn::react(const AutoAdvanceFirstTurn& d) {
    Client().m_ui->GetMapWnd()->m_turn_update->ClickedSignal();
    return discard_event();
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
    Client().m_ui->GetMapWnd()->Hide();
}

ResolvingCombat::~ResolvingCombat() {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ~ResolvingCombat";
    Client().m_ui->GetMapWnd()->Show();
    FreeCombatData(m_previous_combat_data.get());
    FreeCombatData(m_combat_data.get());
}

boost::statechart::result ResolvingCombat::react(const CombatStart& msg) {
    if (TRACE_EXECUTION) Logger().debugStream() << "(HumanClientFSM) ResolvingCombat.CombatStart";
    std::vector<CombatSetupGroup> setup_groups;
    Universe::ShipDesignMap foreign_designs;
    ExtractMessageData(msg.m_message, *m_combat_data, setup_groups, foreign_designs);
    for (Universe::ShipDesignMap::const_iterator it = foreign_designs.begin();
         it != foreign_designs.end();
         ++it) {
        GetUniverse().InsertShipDesignID(it->second, it->first);
    }
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
    return transit<WaitingForTurnDataIdle>();
}
