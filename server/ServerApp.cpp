#include "ServerApp.h"

#include "SaveLoad.h"
#include "ServerFSM.h"
#include "../combat/CombatSystem.h"
#include "../combat/CombatEvents.h"
#include "../combat/CombatLogManager.h"
#include "../parse/Parse.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Special.h"
#include "../universe/System.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../universe/UniverseGenerator.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/LoggerWithOptionsDB.h"
#include "../util/GameRules.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/OrderSet.h"
#include "../util/Pending.h"
#include "../util/SaveGamePreviewUtils.h"
#include "../util/SitRepEntry.h"
#include "../util/ScopedTimer.h"
#include "../util/Version.h"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#include <ctime>
#include <thread>

namespace {
    DeclareThreadSafeLogger(effects);
}

namespace fs = boost::filesystem;

void Seed(unsigned int seed);

namespace {
    DeclareThreadSafeLogger(combat);

    //If there's only one other empire, return their ID:
    int EnemyId(int empire_id, const std::set<int> &empire_ids) {
        if (empire_ids.size() == 2) {
            for (int enemy_id : empire_ids) {
                if (enemy_id != empire_id)
                    return enemy_id;
            }
        }
        return ALL_EMPIRES;
    }
};


////////////////////////////////////////////////
// ServerApp
////////////////////////////////////////////////
ServerApp::ServerApp() :
    IApp(),
    m_signals(m_io_context, SIGINT, SIGTERM),
    m_networking(m_io_context,
                 boost::bind(&ServerApp::HandleNonPlayerMessage, this, _1, _2),
                 boost::bind(&ServerApp::HandleMessage, this, _1, _2),
                 boost::bind(&ServerApp::PlayerDisconnected, this, _1)),
    m_fsm(new ServerFSM(*this)),
    m_current_turn(INVALID_GAME_TURN),
    m_single_player_game(false),
    m_chat_history(1000)
{
    // Force the log file if requested.
    if (GetOptionsDB().Get<std::string>("log-file").empty()) {
        const std::string SERVER_LOG_FILENAME((GetUserDataDir() / "freeoriond.log").string());
        GetOptionsDB().Set("log-file", SERVER_LOG_FILENAME);
    }
    // Force the log threshold if requested.
    auto force_log_level = GetOptionsDB().Get<std::string>("log-level");
    if (!force_log_level.empty())
        OverrideAllLoggersThresholds(to_LogLevel(force_log_level));

    InitLoggingSystem(GetOptionsDB().Get<std::string>("log-file"), "Server");
    InitLoggingOptionsDBSystem();

    InfoLogger() << FreeOrionVersionString();
    LogDependencyVersions();

    // Start parsing content before FSM initialization
    // to have data initialized before autostart execution
    StartBackgroundParsing();

    m_fsm->initiate();

    Empires().DiplomaticStatusChangedSignal.connect(
        boost::bind(&ServerApp::HandleDiplomaticStatusChange, this, _1, _2));
    Empires().DiplomaticMessageChangedSignal.connect(
        boost::bind(&ServerApp::HandleDiplomaticMessageChange,this, _1, _2));

    m_signals.async_wait(boost::bind(&ServerApp::SignalHandler, this, _1, _2));
}

ServerApp::~ServerApp() {
    DebugLogger() << "ServerApp::~ServerApp";
    m_python_server.Finalize();
    CleanupAIs();
    delete m_fsm;
    DebugLogger() << "Server exited cleanly.";
}

void ServerApp::operator()()
{ Run(); }

void ServerApp::SignalHandler(const boost::system::error_code& error, int signal_number) {
    if (error)
        ErrorLogger() << "Exiting due to OS error (" << error.value() << ") " << error.message();
    m_fsm->process_event(ShutdownServer());
}

namespace {
    std::string AIClientExe()
    {
#ifdef FREEORION_WIN32
        return (GetBinDir() / "freeorionca.exe").string();
#else
        return (GetBinDir() / "freeorionca").string();
#endif
    }
}

#ifdef FREEORION_MACOSX
#include <stdlib.h>
#endif

void ServerApp::StartBackgroundParsing() {
    IApp::StartBackgroundParsing();
    const auto& rdir = GetResourceDir();
    m_universe.SetInitiallyUnlockedItems(
        Pending::StartParsing(parse::items, rdir / "scripting/starting_unlocks/items.inf"));
    m_universe.SetInitiallyUnlockedBuildings(
        Pending::StartParsing(parse::starting_buildings, rdir / "scripting/starting_unlocks/buildings.inf"));
    m_universe.SetInitiallyUnlockedFleetPlans(
        Pending::StartParsing(parse::fleet_plans, rdir / "scripting/starting_unlocks/fleets.inf"));
    m_universe.SetMonsterFleetPlans(
        Pending::StartParsing(parse::monster_fleet_plans, rdir / "scripting/monster_fleets.inf"));
    m_universe.SetEmpireStats(
        Pending::StartParsing(parse::statistics, rdir / "scripting/empire_statistics"));
}

void ServerApp::CreateAIClients(const std::vector<PlayerSetupData>& player_setup_data, int max_aggression) {
    DebugLogger() << "ServerApp::CreateAIClients: " << player_setup_data.size() << " player (maybe not all AIs) at max aggression: " << max_aggression;
    // check if AI clients are needed for given setup data
    bool need_AIs = false;
    for (const PlayerSetupData& psd : player_setup_data) {
        if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            need_AIs = true;
            break;
        }
    }
    if (need_AIs)
        m_networking.SendMessageAll(TurnProgressMessage(Message::STARTING_AIS));


    // disconnect any old AI clients
    CleanupAIs();

    if (!need_AIs)
        return;

#ifdef FREEORION_MACOSX
    // On OSX set environment variable DYLD_LIBRARY_PATH to python framework folder
    // bundled with app, so the dynamic linker uses the bundled python library.
    // Otherwise the dynamic linker will look for a correct python lib in system
    // paths, and if it can't find it, throw an error and terminate!
    // Setting environment variable here, spawned child processes will inherit it.
    setenv("DYLD_LIBRARY_PATH", GetPythonHome().string().c_str(), 1);
#endif

    // binary / executable to run for AI clients
    const std::string AI_CLIENT_EXE = AIClientExe();


    // TODO: add other command line args to AI client invocation as needed
    std::vector<std::string> args, arg;
    args.push_back("\"" + AI_CLIENT_EXE + "\"");
    args.push_back("place_holder");
    size_t player_pos = args.size()-1;
    std::stringstream max_aggr_str;
    max_aggr_str << max_aggression;
    args.push_back(max_aggr_str.str());
    args.push_back("--resource.path");
    args.push_back("\"" + GetOptionsDB().Get<std::string>("resource.path") + "\"");

    auto force_log_level = GetOptionsDB().Get<std::string>("log-level");
    if (!force_log_level.empty()) {
        args.push_back("--log-level");
        args.push_back(GetOptionsDB().Get<std::string>("log-level"));
    }

#ifdef FREEORION_LINUX
    if (GetOptionsDB().Get<bool>("testing")) {
        // Dirty hack to output log to console.
        args.push_back("--log-file");
        args.push_back("/proc/self/fd/1");
    }
#endif

    args.push_back("--ai-path");
    args.push_back(GetOptionsDB().Get<std::string>("ai-path"));
    DebugLogger() << "starting AIs with " << AI_CLIENT_EXE ;
    DebugLogger() << "ai-aggression set to " << max_aggression;
    DebugLogger() << "ai-path set to '" << GetOptionsDB().Get<std::string>("ai-path") << "'";
    std::string ai_config = GetOptionsDB().Get<std::string>("ai-config");
    if (!ai_config.empty()) {
        args.push_back("--ai-config");
        args.push_back(ai_config);
        DebugLogger() << "ai-config set to '" << ai_config << "'";
    } else {
        DebugLogger() << "ai-config not set.";
    }

    // for each AI client player, create a new AI client process
    for (const PlayerSetupData& psd : player_setup_data) {
        if (psd.m_client_type != Networking::CLIENT_TYPE_AI_PLAYER)
            continue;

        // check that AIs have a name, as they will be sorted later based on it
        std::string player_name = psd.m_player_name;
        if (player_name.empty()) {
            ErrorLogger() << "ServerApp::CreateAIClients can't create a player with no name.";
            return;
        }

        args[player_pos] = player_name;
        m_ai_client_processes.push_back(Process(AI_CLIENT_EXE, args));

        DebugLogger() << "done starting AI " << player_name;
    }

    // set initial AI process priority to low
    SetAIsProcessPriorityToLow(true);
}

ServerApp* ServerApp::GetApp()
{ return static_cast<ServerApp*>(s_app); }

Universe& ServerApp::GetUniverse()
{ return m_universe; }

EmpireManager& ServerApp::Empires()
{ return m_empires; }

Empire* ServerApp::GetEmpire(int id)
{ return m_empires.GetEmpire(id); }

SupplyManager& ServerApp::GetSupplyManager()
{ return m_supply_manager; }

std::shared_ptr<UniverseObject> ServerApp::GetUniverseObject(int object_id)
{ return m_universe.Objects().Object(object_id); }

ObjectMap& ServerApp::EmpireKnownObjects(int empire_id)
{ return m_universe.EmpireKnownObjects(empire_id); }

std::shared_ptr<UniverseObject> ServerApp::EmpireKnownObject(int object_id, int empire_id)
{ return m_universe.EmpireKnownObjects(empire_id).Object(object_id); }

ServerNetworking& ServerApp::Networking()
{ return m_networking; }

std::string ServerApp::GetVisibleObjectName(std::shared_ptr<const UniverseObject> object) {
    if (!object) {
        ErrorLogger() << "ServerApp::GetVisibleObjectName(): expected non null object pointer.";
        return std::string();
    }

    return object->Name();
}

void ServerApp::Run() {
    DebugLogger() << "FreeOrion server waiting for network events";
    try {
        while (1) {
            if (m_io_context.run_one())
                m_networking.HandleNextEvent();
            else
                break;
        }
    } catch (const NormalExitException&)
    {}
}

void ServerApp::InitializePython() {
    if (m_python_server.IsPythonRunning())
        return;

    if (m_python_server.Initialize())
        return;

    ErrorLogger() << "Server's python interpreter failed to initialize.";
}

void ServerApp::CleanupAIs() {
    if (m_ai_client_processes.empty() && m_networking.empty())
        return;

    DebugLogger() << "ServerApp::CleanupAIs() telling AIs game is ending";

    bool ai_connection_lingering = false;
    try {
        for (PlayerConnectionPtr player : m_networking) {
            if (player->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER) {
                player->SendMessage(EndGameMessage(Message::PLAYER_DISCONNECT));
                ai_connection_lingering = true;
            }
        }
    } catch (...) {
        ErrorLogger() << "ServerApp::CleanupAIs() exception while sending end game messages";
    }

    if (ai_connection_lingering) {
        // time for AIs to react?
        DebugLogger() << "ServerApp::CleanupAIs() waiting 1 second for AI processes to clean up...";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    DebugLogger() << "ServerApp::CleanupAIs() killing " << m_ai_client_processes.size() << " AI clients.";
    try {
        for (Process& process : m_ai_client_processes)
        { process.Kill(); }
    } catch (...) {
        ErrorLogger() << "ServerApp::CleanupAIs() exception while killing processes";
    }

    try {
        m_ai_client_processes.clear();
    } catch (...) {
        ErrorLogger() << "ServerApp::CleanupAIs() exception while clearing client processes";
    }
}

void ServerApp::SetAIsProcessPriorityToLow(bool set_to_low) {
    for (Process& process : m_ai_client_processes) {
        if(!(process.SetLowPriority(set_to_low))) {
            if (set_to_low)
                ErrorLogger() << "ServerApp::SetAIsProcessPriorityToLow : failed to lower priority for AI process";
            else
#ifdef FREEORION_WIN32
                ErrorLogger() << "ServerApp::SetAIsProcessPriorityToLow : failed to raise priority for AI process";
#else
                ErrorLogger() << "ServerApp::SetAIsProcessPriorityToLow : cannot raise priority for AI process, requires superuser privileges on this system";
#endif
        }
    }
}

void ServerApp::HandleMessage(const Message& msg, PlayerConnectionPtr player_connection) {

    //DebugLogger() << "ServerApp::HandleMessage type " << boost::lexical_cast<std::string>(msg.Type());
    m_networking.UpdateCookie(player_connection->Cookie()); // update cookie expire date

    switch (msg.Type()) {
    case Message::HOST_SP_GAME:             m_fsm->process_event(HostSPGame(msg, player_connection));       break;
    case Message::START_MP_GAME:            m_fsm->process_event(StartMPGame(msg, player_connection));      break;
    case Message::LOBBY_UPDATE:             m_fsm->process_event(LobbyUpdate(msg, player_connection));      break;
    case Message::SAVE_GAME_INITIATE:       m_fsm->process_event(SaveGameRequest(msg, player_connection));  break;
    case Message::TURN_ORDERS:              m_fsm->process_event(TurnOrders(msg, player_connection));       break;
    case Message::TURN_PARTIAL_ORDERS:      m_fsm->process_event(TurnPartialOrders(msg, player_connection));break;
    case Message::UNREADY:                  m_fsm->process_event(RevokeReadiness(msg, player_connection));  break;
    case Message::PLAYER_CHAT:              m_fsm->process_event(PlayerChat(msg, player_connection));       break;
    case Message::DIPLOMACY:                m_fsm->process_event(Diplomacy(msg, player_connection));        break;
    case Message::MODERATOR_ACTION:         m_fsm->process_event(ModeratorAct(msg, player_connection));     break;
    case Message::ELIMINATE_SELF:           m_fsm->process_event(EliminateSelf(msg, player_connection));    break;

    case Message::ERROR_MSG:
    case Message::DEBUG:                    break;

    case Message::SHUT_DOWN_SERVER:         HandleShutdownMessage(msg, player_connection);  break;
    case Message::AI_END_GAME_ACK:          m_fsm->process_event(LeaveGame(msg, player_connection));        break;

    case Message::REQUEST_SAVE_PREVIEWS:    UpdateSavePreviews(msg, player_connection); break;
    case Message::REQUEST_COMBAT_LOGS:      m_fsm->process_event(RequestCombatLogs(msg, player_connection));break;
    case Message::LOGGER_CONFIG:            HandleLoggerConfig(msg, player_connection); break;

    default:
        ErrorLogger() << "ServerApp::HandleMessage : Received an unknown message type \"" << msg.Type() << "\".  Terminating connection.";
        m_networking.Disconnect(player_connection);
        break;
    }
}

void ServerApp::HandleShutdownMessage(const Message& msg, PlayerConnectionPtr player_connection) {
    int player_id = player_connection->PlayerID();
    bool is_host = m_networking.PlayerIsHost(player_id);
    if (!is_host) {
        DebugLogger() << "ServerApp::HandleShutdownMessage rejecting shut down message from non-host player";
        return;
    }
    DebugLogger() << "ServerApp::HandleShutdownMessage shutting down";
    m_fsm->process_event(ShutdownServer());
}

void ServerApp::HandleLoggerConfig(const Message& msg, PlayerConnectionPtr player_connection) {
    int player_id = player_connection->PlayerID();
    bool is_host = m_networking.PlayerIsHost(player_id);
    if (!is_host && m_networking.HostPlayerID() != Networking::INVALID_PLAYER_ID) {
        WarnLogger() << "ServerApp::HandleLoggerConfig rejecting message from non-host player id = " << player_id;
        return;
    }

    DebugLogger() << "Handling logging config message from the host.";
    std::set<std::tuple<std::string, std::string, LogLevel>> options;
    ExtractLoggerConfigMessageData(msg, options);

    SetLoggerThresholds(options);

    // Forward the message to all the AIs
    const auto relay_options_message = LoggerConfigMessage(Networking::INVALID_PLAYER_ID, options);
    for (auto players_it = m_networking.established_begin();
         players_it != m_networking.established_end(); ++players_it)
    {
        if ((*players_it)->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER) {
            DebugLogger() << "Forwarding logging thresholds to AI " << (*players_it)->PlayerID();
            (*players_it)->SendMessage(relay_options_message);
        }
    }
}

void ServerApp::HandleNonPlayerMessage(const Message& msg, PlayerConnectionPtr player_connection) {
    switch (msg.Type()) {
    case Message::HOST_SP_GAME:  m_fsm->process_event(HostSPGame(msg, player_connection));   break;
    case Message::HOST_MP_GAME:  m_fsm->process_event(HostMPGame(msg, player_connection));   break;
    case Message::JOIN_GAME:     m_fsm->process_event(JoinGame(msg, player_connection));     break;
    case Message::AUTH_RESPONSE: m_fsm->process_event(AuthResponse(msg, player_connection)); break;
    case Message::ERROR_MSG:     m_fsm->process_event(Error(msg, player_connection));        break;
    case Message::DEBUG:         break;
    default:
        if ((m_networking.size() == 1) && (player_connection->IsLocalConnection()) && (msg.Type() == Message::SHUT_DOWN_SERVER)) {
            DebugLogger() << "ServerApp::HandleNonPlayerMessage received Message::SHUT_DOWN_SERVER from the sole "
                          << "connected player, who is local and so the request is being honored; server shutting down.";
            m_fsm->process_event(ShutdownServer());
        } else {
            ErrorLogger() << "ServerApp::HandleNonPlayerMessage : Received an invalid message type \""
                                            << msg.Type() << "\" for a non-player Message.  Terminating connection.";
            m_networking.Disconnect(player_connection);
            break;
        }
    }
}

void ServerApp::PlayerDisconnected(PlayerConnectionPtr player_connection)
{ m_fsm->process_event(Disconnection(player_connection)); }

void ServerApp::ShutdownTimedoutHandler(boost::system::error_code error) {
    if (error)
        DebugLogger() << "Shutdown timed out cancelled";

    DebugLogger() << "Shutdown timed out.  Disconnecting remaining clients.";
    m_fsm->process_event(DisconnectClients());
}

void ServerApp::SelectNewHost() {
    int new_host_id = Networking::INVALID_PLAYER_ID;
    int old_host_id = m_networking.HostPlayerID();

    DebugLogger() << "ServerApp::SelectNewHost old host id: " << old_host_id;

    // scan through players for a human to host
    for (auto players_it = m_networking.established_begin();
         players_it != m_networking.established_end(); ++players_it)
    {
        PlayerConnectionPtr player_connection = *players_it;
        if (player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
            player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        { new_host_id = player_connection->PlayerID(); }
    }

    if (new_host_id == Networking::INVALID_PLAYER_ID) {
        // couldn't find a host... abort
        DebugLogger() << "ServerApp::SelectNewHost : Host disconnected and couldn't find a replacement.";
        m_networking.SendMessageAll(ErrorMessage(UserStringNop("SERVER_UNABLE_TO_SELECT_HOST"), false));
    }

    // set new host ID
    m_networking.SetHostPlayerID(new_host_id);

    // inform players.
    for (PlayerConnectionPtr player : m_networking) {
        if (player->PlayerID() != old_host_id)
            player->SendMessage(HostIDMessage(new_host_id));
    }
}

void ServerApp::NewSPGameInit(const SinglePlayerSetupData& single_player_setup_data) {
    // associate player IDs with player setup data.  the player connection with
    // id == m_networking.HostPlayerID() should be the human player in
    // PlayerSetupData.  AI player connections are assigned one of the remaining
    // PlayerSetupData entries that is for an AI player.
    const auto& player_setup_data = single_player_setup_data.m_players;
    NewGameInitConcurrentWithJoiners(single_player_setup_data, player_setup_data);
}

bool ServerApp::VerifySPGameAIs(const SinglePlayerSetupData& single_player_setup_data) {
    const auto& player_setup_data = single_player_setup_data.m_players;
    return NewGameInitVerifyJoiners(player_setup_data);
}

void ServerApp::NewMPGameInit(const MultiplayerLobbyData& multiplayer_lobby_data) {
    // associate player IDs with player setup data by matching player IDs when
    // available (human) and names (for AI clients which didn't have an ID
    // before now because the lobby data was set up without connected/established
    // clients for the AIs.
    std::map<int, PlayerSetupData> player_id_setup_data;
    const auto& player_setup_data = multiplayer_lobby_data.m_players;

    for (const auto& entry : player_setup_data) {
        const PlayerSetupData& psd = entry.second;
        if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
            psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        {
            // Human players have consistent IDs, so these can be easily
            // matched between established player connections and setup data.

            // find player connection with same ID as this player setup data
            bool found_matched_id_connection = false;
            int player_id = entry.first;
            for (auto established_player_it = m_networking.established_begin();
                 established_player_it != m_networking.established_end(); ++established_player_it)
            {
                const PlayerConnectionPtr player_connection = *established_player_it;
                if (player_connection->PlayerID() == player_id)
                {
                    player_id_setup_data[player_id] = psd;
                    found_matched_id_connection = true;
                    break;
                }
            }
            if (!found_matched_id_connection)
                ErrorLogger() << "ServerApp::NewMPGameInit couldn't find player setup data for human player with id: " << player_id;

        } else if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            // All AI player setup data, as determined from their client type, is
            // assigned to player IDs of established AI players with the appropriate names

            // find player connection with same name as this player setup data
            bool found_matched_name_connection = false;
            const std::string& player_name = psd.m_player_name;
            for (auto established_player_it = m_networking.established_begin();
                 established_player_it != m_networking.established_end(); ++established_player_it)
            {
                const PlayerConnectionPtr player_connection = *established_player_it;
                if (player_connection->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER &&
                    player_connection->PlayerName() == player_name)
                {
                    // assign name-matched AI client's player setup data to appropriate AI connection
                    int player_id = player_connection->PlayerID();
                    player_id_setup_data[player_id] = psd;
                    found_matched_name_connection = true;
                    break;
                }
            }
            if (!found_matched_name_connection)
                ErrorLogger() << "ServerApp::NewMPGameInit couldn't find player setup data for AI player with name: " << player_name;

        } else {
            // do nothing for any other player type, until another player type
            // is implemented.  human observers don't need to be put into the
            // map of id to player setup data, as they don't need empires to be
            // created for them.
            ErrorLogger() << "ServerApp::NewMPGameInit skipping unsupported client type in player setup data";
        }
    }

    std::vector<PlayerSetupData> psds;
    for (auto& id_and_psd : player_id_setup_data) {
        id_and_psd.second.m_player_id = id_and_psd.first;
        psds.push_back(id_and_psd.second);
    }

    NewGameInitConcurrentWithJoiners(multiplayer_lobby_data, psds);
    if (NewGameInitVerifyJoiners(psds))
        SendNewGameStartMessages();
}

void UpdateEmpireSupply(bool precombat=false) {
    EmpireManager& empires = Empires();

    // Determine initial supply distribution and exchanging and resource pools for empires
    for (auto& entry : empires) {
        Empire* empire = entry.second;
        if (empire->Eliminated())
            continue;   // skip eliminated empires.  presumably this shouldn't be an issue when initializing a new game, but apparently I thought this was worth checking for...

        empire->UpdateSupplyUnobstructedSystems(precombat);  // determines which systems can propagate fleet and resource (same for both)
        empire->UpdateSystemSupplyRanges();         // sets range systems can propagate fleet and resourse supply (separately)
    }

    GetSupplyManager().Update();

    for (auto& entry : empires) {
        Empire* empire = entry.second;
        if (empire->Eliminated())
            continue;

        empire->InitResourcePools();                // determines population centers and resource centers of empire, tells resource pools the centers and groups of systems that can share resources (note that being able to share resources doesn't mean a system produces resources)
        empire->UpdateResourcePools();              // determines how much of each resources is available in each resource sharing group
    }
}

void ServerApp::NewGameInitConcurrentWithJoiners(
    const GalaxySetupData& galaxy_setup_data,
    const std::vector<PlayerSetupData>& player_setup_data)
{
    DebugLogger() << "ServerApp::NewGameInitConcurrentWithJoiners";

    m_galaxy_setup_data = galaxy_setup_data;

    // set game rules for server based on those specified in setup data
    GetGameRules().SetFromStrings(m_galaxy_setup_data.GetGameRules());

    // validate some connection info / determine which players need empires created
    std::map<int, PlayerSetupData> active_players_id_setup_data;
    for (const auto& psd : player_setup_data) {
        if (!psd.m_player_name.empty()
            && (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER
                || psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER))
        {
            active_players_id_setup_data[psd.m_player_id] = psd;
        }
    }

    if (active_players_id_setup_data.empty()) {
        ErrorLogger() << "ServerApp::NewGameInitConcurrentWithJoiners found no active players!";
        m_networking.SendMessageAll(ErrorMessage(UserStringNop("SERVER_FOUND_NO_ACTIVE_PLAYERS"), true));
        return;
    }

    // clear previous game player state info
    m_turn_sequence.clear();
    m_player_empire_ids.clear();
    m_empires.Clear();

    // set server state info for new game
    m_current_turn = BEFORE_FIRST_TURN;

    // create universe and empires for players
    DebugLogger() << "ServerApp::NewGameInitConcurrentWithJoiners: Creating Universe";
    m_networking.SendMessageAll(TurnProgressMessage(Message::GENERATING_UNIVERSE));


    // m_current_turn set above so that every UniverseObject created before game
    // starts will have m_created_on_turn BEFORE_FIRST_TURN
    GenerateUniverse(active_players_id_setup_data);


    // after all game initialization stuff has been created, set current turn to 0 and apply only GenerateSitRep Effects
    // so that a set of SitReps intended as the player's initial greeting will be segregated
    m_current_turn = 0;
    m_universe.ApplyGenerateSitRepEffects();

    //can set current turn to 1 for start of game
    m_current_turn = 1;

    // record empires for each active player: ID of empire and player should
    // be the same when creating a new game. Note: active_players_id_setup_data
    // contains only ids of players who control an empire; observers and
    // moderators are not included.
    for (const auto& player_id_and_setup : active_players_id_setup_data) {
        int player_id = player_id_and_setup.first;
        m_player_empire_ids[player_id] = player_id;

        // add empires to turn processing
        int empire_id = PlayerEmpireID(player_id);
        if (GetEmpire(empire_id))
            AddEmpireTurn(empire_id, PlayerSaveGameData(player_id_and_setup.second.m_player_name, empire_id,
                                                        nullptr, nullptr, std::string(),
                                                        player_id_and_setup.second.m_client_type,
                                                        false));
    }

    // update visibility information to ensure data sent out is up-to-date
    DebugLogger() << "ServerApp::NewGameInitConcurrentWithJoiners: Updating first-turn Empire stuff";
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();

    // initialize empire owned object counters
    EmpireManager& empires = Empires();
    for (auto& entry : empires)
        entry.second->UpdateOwnedObjectCounters();

    UpdateEmpireSupply();
    m_universe.UpdateStatRecords();
}

bool ServerApp::NewGameInitVerifyJoiners(const std::vector<PlayerSetupData>& player_setup_data) {
    DebugLogger() << "ServerApp::NewGameInitVerifyJoiners";

    // associate player IDs with player setup data.  the player connection with
    // id == m_networking.HostPlayerID() should be the human player in
    // PlayerSetupData.  AI player connections are assigned one of the remaining
    // PlayerSetupData entries that is for an AI player.

    std::map<int, PlayerSetupData> player_id_setup_data;
    bool host_in_player_id_setup_data = false;

    for (const auto& psd : player_setup_data) {
        if (psd.m_client_type == Networking::INVALID_CLIENT_TYPE) {
            ErrorLogger() << "Player with id " << psd.m_player_id << " has invalid client type";
            continue;
        }

        player_id_setup_data[psd.m_player_id] = psd;

        if (m_networking.HostPlayerID() == psd.m_player_id)
            host_in_player_id_setup_data = true;
    }

    // ensure some reasonable inputs
    if (player_id_setup_data.empty()) {
        ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners passed empty player_id_setup_data.  Aborting";
        m_networking.SendMessageAll(ErrorMessage(UserStringNop("SERVER_FOUND_NO_ACTIVE_PLAYERS"), true));
        return false;
    }

    if (!host_in_player_id_setup_data && !IsHostless()) {
        ErrorLogger() << "NewGameInitVerifyJoiners : Host id " << m_networking.HostPlayerID()
                      << " is not a valid player id.";
        return false;
    }

    // ensure number of players connected and for which data are provided are consistent
    if (m_networking.NumEstablishedPlayers() != player_id_setup_data.size()) {
        ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners has " << m_networking.NumEstablishedPlayers()
                      << " established players but " << player_id_setup_data.size() << " players in setup data.";
        return false;
    }

    // validate some connection info / determine which players need empires created
    for (auto player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        Networking::ClientType client_type = player_connection->GetClientType();
        int player_id = player_connection->PlayerID();

        auto player_id_setup_data_it = player_id_setup_data.find(player_id);
        if (player_id_setup_data_it == player_id_setup_data.end()) {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners couldn't find player setup data for player with ID " << player_id;
            return false;
        }
        const PlayerSetupData& psd = player_id_setup_data_it->second;
        if (psd.m_client_type != client_type) {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners found inconsistent client type between player connection (" << client_type << ") and player setup data (" << psd.m_client_type << ")";
            return false;
        }
        if (psd.m_player_name != player_connection->PlayerName()) {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners found inconsistent player names: " << psd.m_player_name << " and " << player_connection->PlayerName();
            return false;
        }
        if (player_connection->PlayerName().empty()) {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners found player connection with empty name!";
            return false;
        }

        if (!(client_type == Networking::CLIENT_TYPE_AI_PLAYER
              || client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER
              || client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER
              || client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR))
        {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners found player connection with unsupported client type.";
        }
    }
    return true;
}

void ServerApp::SendNewGameStartMessages() {
    std::map<int, PlayerInfo> player_info_map = GetPlayerInfoMap();

    // send new game start messages
    DebugLogger() << "SendGameStartMessages: Sending GameStartMessages to players";
    for (auto player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        int empire_id = PlayerEmpireID(player_id);
        bool use_binary_serialization = player_connection->IsBinarySerializationUsed();
        player_connection->SendMessage(GameStartMessage(m_single_player_game,    empire_id,
                                                        m_current_turn,          m_empires,
                                                        m_universe,              GetSpeciesManager(),
                                                        GetCombatLogManager(),   GetSupplyManager(),
                                                        player_info_map,         m_galaxy_setup_data,
                                                        use_binary_serialization));
    }
}

void ServerApp::LoadSPGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                               std::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    // Need to determine which data in player_save_game_data should be assigned to which established player
    std::vector<std::pair<int, int>> player_id_to_save_game_data_index;

    auto established_player_it = m_networking.established_begin();

    // assign all saved game data to a player ID
    for (int i = 0; i < static_cast<int>(player_save_game_data.size()); ++i) {
        const PlayerSaveGameData& psgd = player_save_game_data[i];
        if (psgd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            // In a single player game, the host player is always the human player, so
            // this is just a matter of finding which entry in player_save_game_data was
            // a human player, and assigning that saved player data to the host player ID
            player_id_to_save_game_data_index.push_back({m_networking.HostPlayerID(), i});

        } else if (psgd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            // All saved AI player data, as determined from their client type, is
            // assigned to player IDs of established AI players

            // cycle to find next established AI player
            while (established_player_it != m_networking.established_end()) {
                const PlayerConnectionPtr player_connection = *established_player_it;
                ++established_player_it;
                // if player is an AI, assign it to this
                if (player_connection->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER) {
                    int player_id = player_connection->PlayerID();
                    player_id_to_save_game_data_index.push_back({player_id, i});
                    break;
                }
            }
        } else {
            // do nothing for any other player type, until another player type is implemented
            ErrorLogger() << "ServerApp::LoadSPGameInit skipping unsupported client type in player save game data";
        }
    }

    LoadGameInit(player_save_game_data, player_id_to_save_game_data_index, server_save_game_data);
}

namespace {
    /** Check that \p path is a file or directory in the server save
    directory. */
    bool IsInServerSaveDir(const fs::path& path) {
        if (!fs::exists(path))
            return false;

        return IsInDir(GetServerSaveDir(),
                       (fs::is_regular_file(path) ? path.parent_path() : path));
    }

    /// Generates information on the subdirectories of \p directory
    std::vector<std::string> ListSaveSubdirectories(const fs::path& directory) {
        std::vector<std::string> list;
        if (!fs::is_directory(directory))
            return list;

        auto server_dir_str = PathToString(fs::canonical(GetServerSaveDir()));

        // Adds \p subdir to the list
        auto add_to_list = [&list, &server_dir_str](const fs::path& subdir) {
            auto subdir_str = PathToString(fs::canonical(subdir));
            auto rel_path = subdir_str.substr(server_dir_str.length());
            list.push_back(rel_path);
            TraceLogger() << "Added relative path " << rel_path << " in " << subdir
                          << " to save preview directories";
        };

        // Add parent dir if still within server_dir_str
        auto parent = directory / "..";
        if (IsInServerSaveDir(parent))
            add_to_list(parent);

        // Add all directories to list
        fs::directory_iterator end;
        for (fs::directory_iterator it(fs::canonical(directory)); it != end; ++it) {
            if (!fs::is_directory(it->path()) || !IsInServerSaveDir(it->path()))
                continue;
            add_to_list(it->path());
        }
        return list;
    }
}

void ServerApp::UpdateSavePreviews(const Message& msg,
                                   PlayerConnectionPtr player_connection)
{
    // Only relative paths are allowed to prevent client from list arbitrary
    // directories, or knowing the absolute path of the server save directory.
    std::string relative_directory_name;
    ExtractRequestSavePreviewsMessageData(msg, relative_directory_name);

    DebugLogger() << "ServerApp::UpdateSavePreviews: Preview request for sub directory: " << relative_directory_name;

    fs::path directory = GetServerSaveDir() / FilenameToPath(relative_directory_name);
    // Do not allow a relative path to explore outside the save directory.
    bool contains_dot_dot = relative_directory_name.find("..") != std::string::npos;
    if (contains_dot_dot || !IsInServerSaveDir(directory)) {
        directory = GetServerSaveDir();
        ErrorLogger() << "ServerApp::UpdateSavePreviews: Tried to load previews from "
                      << relative_directory_name
                      << " which is outside the allowed save directory. Defaulted to the save directory, "
                      << directory;
        relative_directory_name = ".";
    }

    PreviewInformation preview_information;
    preview_information.folder = std::move(relative_directory_name);
    preview_information.subdirectories = ListSaveSubdirectories(directory);
    LoadSaveGamePreviews(
        directory,
        m_single_player_game? SP_SAVE_FILE_EXTENSION : MP_SAVE_FILE_EXTENSION,
        preview_information.previews);

    DebugLogger() << "ServerApp::UpdateSavePreviews: Sending " << preview_information.previews.size()
                  << " previews in response.";

    player_connection->SendMessage(DispatchSavePreviewsMessage(preview_information));
}

void ServerApp::UpdateCombatLogs(const Message& msg, PlayerConnectionPtr player_connection) {
    std::vector<int> ids;
    ExtractRequestCombatLogsMessageData(msg, ids);

    // Compose a vector of the requested ids and logs
    std::vector<std::pair<int, const CombatLog>> logs;
    for (auto it = ids.begin(); it != ids.end(); ++it) {
        boost::optional<const CombatLog&> log = GetCombatLogManager().GetLog(*it);
        if (!log) {
            ErrorLogger() << "UpdateCombatLogs can't fetch log with id = "<< *it << " ... skipping.";
            continue;
        }
        logs.push_back({*it, *log});
    }

    // Return them to the client
    DebugLogger() << "UpdateCombatLogs returning " << logs.size()
                  << " logs to player " << player_connection->PlayerID();
    player_connection->SendMessage(DispatchCombatLogsMessage(logs));
}

void ServerApp::LoadChatHistory() {
    // don't load history if it was already loaded
    if (!m_chat_history.empty())
        return;

    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonChatDir());
        // Call the Python load_history function
        success = m_python_server.LoadChatHistory(m_chat_history);
    } catch (const boost::python::error_already_set& err) {
        success = false;
        m_python_server.HandleErrorAlreadySet();
        if (!m_python_server.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python_server.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                m_fsm->process_event(ShutdownServer());
            }
        }
    }

    if (!success) {
        ErrorLogger() << "Python scripted chat failed.";
        ServerApp::GetApp()->Networking().SendMessageAll(ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"),
                                                                      false));
    }
}

void ServerApp::PushChatMessage(const std::string& text,
                                const std::string& player_name,
                                GG::Clr text_color,
                                const boost::posix_time::ptime& timestamp)
{
    ChatHistoryEntity chat;
    chat.m_timestamp = timestamp;
    chat.m_player_name = player_name;
    chat.m_text_color = text_color;
    chat.m_text = text;
    m_chat_history.push_back(chat);

    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonChatDir());
        // Call the Python load_history function
        success = m_python_server.PutChatHistoryEntity(chat);
    } catch (const boost::python::error_already_set& err) {
        success = false;
        m_python_server.HandleErrorAlreadySet();
        if (!m_python_server.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python_server.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                m_fsm->process_event(ShutdownServer());
            }
        }
    }

    if (!success) {
        ErrorLogger() << "Python scripted chat failed.";
        ServerApp::GetApp()->Networking().SendMessageAll(ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"),
                                                                      false));
    }
}

namespace {
    /** Verifies that a human player is connected with the indicated \a id. */
    bool HumanPlayerWithIdConnected(const ServerNetworking& sn, int id) {
        // make sure there is a human player connected with the player id
        // matching what this PlayerSetupData say
        auto established_player_it = sn.GetPlayer(id);
        if (established_player_it == sn.established_end()) {
            ErrorLogger() << "ServerApp::LoadMPGameInit couldn't find player connection for "
                          << "human player setup data with player id: " << id;
            return false;
        }
        const PlayerConnectionPtr player_connection = *established_player_it;
        if (player_connection->GetClientType() != Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            ErrorLogger() << "ServerApp::LoadMPGameInit found player connection of wrong type "
                          << "for human player setup data with player id: " << id;
            return false;
        }
        return true;
    }

    /** Returns index into vector parameter that matches parameter empire id. */
    int VectorIndexForPlayerSaveGameDataForEmpireID(const std::vector<PlayerSaveGameData>& player_save_game_data,
                                                    int empire_id)
    {
        if (empire_id == ALL_EMPIRES)
            return -1;
        // find save game data vector index that has requested empire id
        for (int i = 0; i < static_cast<int>(player_save_game_data.size()); ++i) {
            const PlayerSaveGameData& psgd = player_save_game_data.at(i);
            if (psgd.m_empire_id == empire_id)
                return i;
        }
        return -1;
    }

    /** Adds entry to \a player_id_to_save_game_data_index after validation. */
    void GetSaveGameDataIndexForHumanPlayer(std::vector<std::pair<int, int>>& player_id_to_save_game_data_index,
                                            const PlayerSetupData& psd, int setup_data_player_id,
                                            const std::vector<PlayerSaveGameData>& player_save_game_data,
                                            const ServerNetworking& sn)
    {
        // safety check: setup data has valid empire assigned
        if (psd.m_save_game_empire_id == ALL_EMPIRES) {
            ErrorLogger() << "ServerApp::LoadMPGameInit got player setup data for human player "
                                    << "with no empire assigned...";
            return;
        }

        // safety check: id-matched player is connected
        bool consistent_human_player_connected = HumanPlayerWithIdConnected(sn, setup_data_player_id);
        if (!consistent_human_player_connected)
            return;   // error message logged in HumanPlayerWithIdConnected

        // determine and store save game data index for this player
        int index = VectorIndexForPlayerSaveGameDataForEmpireID(player_save_game_data, psd.m_save_game_empire_id);
        if (index != -1) {
            player_id_to_save_game_data_index.push_back({setup_data_player_id, index});
        } else {
            ErrorLogger() << "ServerApp::LoadMPGameInit couldn't find save game data for "
                                   << "human player with assigned empire id: " << psd.m_save_game_empire_id;
        }
    }

    /** Returns ID of AI player with the indicated \a player_name. */
    int AIPlayerIDWithName(const ServerNetworking& sn, const std::string& player_name) {
        if (player_name.empty())
            return Networking::INVALID_PLAYER_ID;

        for (auto established_player_it = sn.established_begin();
             established_player_it != sn.established_end(); ++established_player_it)
        {
            const PlayerConnectionPtr player_connection = *established_player_it;
            if (player_connection->PlayerName() == player_name &&
                player_connection->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER)
            { return player_connection->PlayerID(); }
        }
        return Networking::INVALID_PLAYER_ID;
    }

    /** Adds entry to \a player_id_to_save_game_data_index after validation. */
    void GetSaveGameDataIndexForAIPlayer(std::vector<std::pair<int, int>>& player_id_to_save_game_data_index,
                                         const PlayerSetupData& psd,
                                         const std::vector<PlayerSaveGameData>& player_save_game_data,
                                         const ServerNetworking& sn)
    {
        // For AI players, the multplayer setup data does not specify a
        // player ID because the AI processes aren't run until after the
        // game settings are confirmed and the game started in the UI,
        // and thus the AI clients don't connect and get assigned player
        // ids until after the lobby setup is done.
        //
        // In order to assign save game data to players (ie. determine
        // the save game data vector index for each player id), need to
        // match another property in the setup data: the AI player names.
        //
        // So: attempt to find player connections that have the same name
        // as is listed in the player setup data for AI players.

        // safety check: setup data has valid empire assigned
        if (psd.m_save_game_empire_id == ALL_EMPIRES) {
            ErrorLogger() << "ServerApp::LoadMPGameInit got player setup data for AI player "
                                    << "with no empire assigned...";
            return;
        }

        // get ID of name-matched AI player
        int player_id = AIPlayerIDWithName(sn, psd.m_player_name);
        if (player_id == Networking::INVALID_PLAYER_ID) {
            ErrorLogger() << "ServerApp::LoadMPGameInit couldn't find expected AI player with name " << psd.m_player_name;
            return;
        }

        DebugLogger() << "ServerApp::LoadMPGameInit matched player named " << psd.m_player_name
                               << " to setup data player id " << player_id
                               << " with setup data empire id " << psd.m_save_game_empire_id;

        // determine and store save game data index for this player
        int index = VectorIndexForPlayerSaveGameDataForEmpireID(player_save_game_data, psd.m_save_game_empire_id);
        if (index != -1) {
            player_id_to_save_game_data_index.push_back({player_id, index});
        } else {
            ErrorLogger() << "ServerApp::LoadMPGameInit couldn't find save game data for "
                                   << "human player with assigned empire id: " << psd.m_save_game_empire_id;
        }
    }
}

void ServerApp::LoadMPGameInit(const MultiplayerLobbyData& lobby_data,
                               const std::vector<PlayerSaveGameData>& player_save_game_data,
                               std::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    // Need to determine which data in player_save_game_data should be assigned to which established player
    std::vector<std::pair<int, int>> player_id_to_save_game_data_index;

    const auto& player_setup_data = lobby_data.m_players;

    // * Multiplayer lobby data has a map from player ID to PlayerSetupData.
    // * PlayerSetupData contains an empire ID that the player will be controlling.
    // * PlayerSaveGameData in a vector contain empire ID members.
    // * LoadGameInit (called below) need an index in the PlayerSaveGameData vector
    //   for each player ID
    // => Need to find which index into the PlayerSaveGameData vector has the right
    //    empire id for each player id.


    // for every player setup data entry that represents an empire in the game,
    // assign saved game data to the player ID of an established human or AI player
    for (const auto& entry : player_setup_data) {
        const PlayerSetupData& psd = entry.second;

        if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            int setup_data_player_id = entry.first;
            GetSaveGameDataIndexForHumanPlayer(player_id_to_save_game_data_index, psd,
                                               setup_data_player_id, player_save_game_data,
                                               m_networking);

        } else if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            // AI clients have no player id in setup data (even though humans do)
            GetSaveGameDataIndexForAIPlayer(player_id_to_save_game_data_index, psd,
                                            player_save_game_data, m_networking);

        }
        // do nothing for any other player type, until another player type
        // is implemented.  human observers and moderators don't need to be
        // put into the map of id to player setup data, as they don't need
        // empires to be created for them.
    }

    LoadGameInit(player_save_game_data, player_id_to_save_game_data_index, server_save_game_data);
}

void ServerApp::LoadGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                             const std::vector<std::pair<int, int>>& player_id_to_save_game_data_index,
                             std::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    DebugLogger() << "ServerApp::LoadGameInit";

    // ensure some reasonable inputs
    if (player_save_game_data.empty()) {
        ErrorLogger() << "ServerApp::LoadGameInit passed empty player save game data.  Aborting";
        m_networking.SendMessageAll(ErrorMessage(UserStringNop("SERVER_FOUND_NO_ACTIVE_PLAYERS"), true));
        return;
    }

    // ensure number of players connected and for which data are provided are consistent
    if (player_id_to_save_game_data_index.size() != player_save_game_data.size())
        ErrorLogger() << "ServerApp::LoadGameInit passed index mapping and player save game data are of different sizes...";

    if (m_networking.NumEstablishedPlayers() != player_save_game_data.size())
        ErrorLogger() << "ServerApp::LoadGameInit has " << m_networking.NumEstablishedPlayers()
                      << " established players but " << player_save_game_data.size()
                      << " entries in player save game data.  Could be ok... so not aborting, but might crash";


    // set game rules for server based on those specified in setup data
    GetGameRules().SetFromStrings(m_galaxy_setup_data.GetGameRules());


    // validate some connection info
    for (auto player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        Networking::ClientType client_type = player_connection->GetClientType();
        if (client_type != Networking::CLIENT_TYPE_AI_PLAYER &&
            client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER)
        {
            ErrorLogger() << "ServerApp::LoadGameInit found player connection with unsupported client type.";
        }
        if (player_connection->PlayerName().empty()) {
            ErrorLogger() << "ServerApp::LoadGameInit found player connection with empty name!";
        }
    }


    // clear previous game player state info
    m_turn_sequence.clear();
    m_player_empire_ids.clear();


    // restore server state info from save
    m_current_turn = server_save_game_data->m_current_turn;

    std::map<int, PlayerSaveGameData> player_id_save_game_data;

    // add empires to turn processing and record empires for each player
    for (auto player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        if (!player_connection->IsEstablished()) {
            ErrorLogger() << "LoadGameInit got player from connection";
            continue;
        }

        int player_id = player_connection->PlayerID();

        // get index into save game data for this player id
        int player_save_game_data_index = -1;   // default invalid index
        for (const std::pair<int, int>& entry : player_id_to_save_game_data_index) {
            int index_player_id = entry.first;
            if (player_id != index_player_id)
                continue;
            player_save_game_data_index = entry.second;
            break;
        }
        if (player_save_game_data_index == -1) {
            DebugLogger() << "No save game data index for player with id " << player_id;
            continue;
        }


        // get the player's saved game data
        int empire_id = ALL_EMPIRES;
        try {
            const PlayerSaveGameData& psgd = player_save_game_data.at(player_save_game_data_index);
            empire_id = psgd.m_empire_id;               // can't use GetPlayerEmpireID here because m_player_empire_ids hasn't been set up yet.
            player_id_save_game_data[player_id] = psgd; // store by player ID for easier access later
        } catch (...) {
            ErrorLogger() << "ServerApp::LoadGameInit couldn't find save game data with index " << player_save_game_data_index;
            continue;
        }


        // record player id to empire id mapping in loaded game.  Player IDs
        // and empire IDs are not necessarily the same when loading a game as
        // the player controlling a particular empire might have a different
        // player ID than when the game was first created
        m_player_empire_ids[player_id] = empire_id;

        // set actual authentication status
        if (Empire* empire = GetEmpire(empire_id)) {
            empire->SetAuthenticated(player_connection->IsAuthenticated());
        }
    }

    for (const auto& psgd : player_save_game_data) {
        int empire_id = psgd.m_empire_id;
        // add empires to turn processing, and restore saved orders and UI data or save state data
        if (Empire* empire = GetEmpire(empire_id)) {
            if (!empire->Eliminated())
                AddEmpireTurn(empire_id, psgd);
        } else {
            ErrorLogger() << "ServerApp::LoadGameInit couldn't find empire with id " << empire_id << " to add to turn processing";
        }
    }


    // the Universe's system graphs for each empire aren't stored when saving
    // so need to be reinitialized when loading based on the gamestate
    m_universe.InitializeSystemGraph();

    UpdateEmpireSupply(true);  // precombat type supply update

    std::map<int, PlayerInfo> player_info_map = GetPlayerInfoMap();

    // assemble player state information, and send game start messages
    DebugLogger() << "ServerApp::CommonGameInit: Sending GameStartMessages to players";

    for (auto player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        Networking::ClientType client_type = player_connection->GetClientType();

        // attempt to find saved state data for this player.
        PlayerSaveGameData psgd;
        auto save_data_it = player_id_save_game_data.find(player_id);
        if (save_data_it != player_id_save_game_data.end()) {
            psgd = save_data_it->second;
        }
        if (!psgd.m_orders) {
            psgd.m_orders.reset(new OrderSet());    // need an empty order set pointed to for serialization in case no data is loaded but the game start message wants orders to send
        }

        // get empire ID for player. safety check on it.
        int empire_id = PlayerEmpireID(player_id);
        if (empire_id != psgd.m_empire_id) {
            ErrorLogger() << "LoadGameInit got inconsistent empire ids between player save game data and result of PlayerEmpireID";
        }

        // Revoke readiness only for online players so they can redo orders for the current turn.
        // Without doing it, server would immediatly advance the turn because saves are made when
        // all players sent orders and became ready.
        RevokeEmpireTurnReadyness(empire_id);

        // restore saved orders.  these will be re-executed on client and
        // re-sent to the server (after possibly modification) by clients
        // when they end their turn
        auto orders = psgd.m_orders;

        bool use_binary_serialization = player_connection->IsBinarySerializationUsed();

        if (client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            // get save state string
            const std::string* sss = nullptr;
            if (!psgd.m_save_state_string.empty())
                sss = &psgd.m_save_state_string;

            player_connection->SendMessage(GameStartMessage(m_single_player_game, empire_id,
                                                            m_current_turn, m_empires, m_universe,
                                                            GetSpeciesManager(), GetCombatLogManager(),
                                                            GetSupplyManager(), player_info_map, *orders, sss,
                                                            m_galaxy_setup_data, use_binary_serialization));

        } else if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            player_connection->SendMessage(GameStartMessage(m_single_player_game, empire_id,
                                                            m_current_turn, m_empires, m_universe,
                                                            GetSpeciesManager(), GetCombatLogManager(),
                                                            GetSupplyManager(),  player_info_map, *orders,
                                                            psgd.m_ui_data.get(), m_galaxy_setup_data,
                                                            use_binary_serialization));

        } else if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                   client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        {

            player_connection->SendMessage(GameStartMessage(m_single_player_game, ALL_EMPIRES,
                                                            m_current_turn, m_empires, m_universe,
                                                            GetSpeciesManager(), GetCombatLogManager(),
                                                            GetSupplyManager(), player_info_map,
                                                            m_galaxy_setup_data, use_binary_serialization));
        } else {
            ErrorLogger() << "ServerApp::CommonGameInit unsupported client type: skipping game start message.";
        }
    }

    for (auto player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        // send other empires' statuses
        for (const auto& empire : Empires()) {
            auto other_orders_it = m_turn_sequence.find(empire.first);
            bool ready = other_orders_it == m_turn_sequence.end() ||
                    (other_orders_it->second && other_orders_it->second->m_ready);
            (*player_connection_it)->SendMessage(PlayerStatusMessage(EmpirePlayerID(empire.first),
                                                                     ready ? Message::WAITING : Message::PLAYING_TURN,
                                                                     empire.first));
        }
    }
}

void ServerApp::GenerateUniverse(std::map<int, PlayerSetupData>& player_setup_data) {
    Universe& universe = GetUniverse();

    // Set game UID. Needs to be done first so we can use ClockSeed to
    // prevent reproducible UIDs.
    ClockSeed();
    GetGalaxySetupData().SetGameUID(boost::uuids::to_string(boost::uuids::random_generator()()));

    // Initialize RNG with provided seed to get reproducible universes
    int seed = 0;
    try {
        seed = boost::lexical_cast<unsigned int>(GetGalaxySetupData().m_seed);
    } catch (...) {
        try {
            boost::hash<std::string> string_hash;
            std::size_t h = string_hash(GetGalaxySetupData().m_seed);
            seed = static_cast<unsigned int>(h);
        } catch (...) {}
    }
    if (GetGalaxySetupData().GetSeed().empty() || GetGalaxySetupData().GetSeed() == "RANDOM") {
        //ClockSeed();
        // replicate ClockSeed code here so can log the seed used
        boost::posix_time::ptime ltime = boost::posix_time::microsec_clock::local_time();
        std::string new_seed = boost::posix_time::to_simple_string(ltime);
        boost::hash<std::string> string_hash;
        std::size_t h = string_hash(new_seed);
        DebugLogger() << "GenerateUniverse using clock for seed:" << new_seed;
        seed = static_cast<unsigned int>(h);
        // store seed in galaxy setup data
        ServerApp::GetApp()->GetGalaxySetupData().SetSeed(std::to_string(seed));
    }
    Seed(seed);
    DebugLogger() << "GenerateUniverse with seed: " << seed;

    // Reset the universe object for a new universe
    universe.Clear();
    GetSpeciesManager().ClearSpeciesHomeworlds();

    // Reset the object id manager for the new empires.
    std::vector<int> empire_ids(player_setup_data.size());
    std::transform(player_setup_data.begin(), player_setup_data.end(), empire_ids.begin(),
                   [](const std::pair<int,PlayerSetupData> ii) { return ii.first; });
    universe.ResetAllIDAllocation(empire_ids);

    // Add predefined ship designs to universe
    GetPredefinedShipDesignManager().AddShipDesignsToUniverse();
    // Initialize empire objects for each player
    InitEmpires(player_setup_data);

    bool success(false);
    try {
        // Set Python current work directory to directory containing
        // the universe generation Python scripts
        m_python_server.SetCurrentDir(GetPythonUniverseGeneratorDir());
        // Call the main Python universe generator function
        success = m_python_server.CreateUniverse(player_setup_data);
    } catch (const boost::python::error_already_set& err) {
        success = false;
        m_python_server.HandleErrorAlreadySet();
        if (!m_python_server.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Exiting.";
            m_fsm->process_event(ShutdownServer());
        }
    }

    if (!success)
        ServerApp::GetApp()->Networking().SendMessageAll(ErrorMessage(UserStringNop("SERVER_UNIVERSE_GENERATION_ERRORS"), false));


    DebugLogger() << "Applying first turn effects and updating meters";

    // Apply effects for 1st turn.
    universe.ApplyAllEffectsAndUpdateMeters(false);

    TraceLogger(effects) << "After First turn meter effect applying: " << universe.Objects().Dump();
    // Set active meters to targets or maxes after first meter effects application
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());

    universe.UpdateMeterEstimates();
    universe.BackPropagateObjectMeters();
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());
    universe.BackPropagateObjectMeters();

    TraceLogger(effects) << "After First active set to target/max: " << universe.Objects().Dump();

    universe.BackPropagateObjectMeters();
    Empires().BackPropagateMeters();

    DebugLogger() << "Re-applying first turn meter effects and updating meters";

    // Re-apply meter effects, so that results depending on meter values can be
    // re-checked after initial setting of those meter values
    universe.ApplyMeterEffectsAndUpdateMeters(false);
    // Re-set active meters to targets after re-application of effects
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());
    // Set the population of unowned planets to a random fraction of their target values.
    SetNativePopulationValues(universe.Objects());

    universe.BackPropagateObjectMeters();
    Empires().BackPropagateMeters();

    TraceLogger() << "!!!!!!!!!!!!!!!!!!! After setting active meters to targets";
    TraceLogger() << universe.Objects().Dump();

    universe.UpdateEmpireObjectVisibilities();
}

void ServerApp::ExecuteScriptedTurnEvents() {
    bool success(false);
    try {
        m_python_server.SetCurrentDir(GetPythonTurnEventsDir());
        // Call the main Python turn events function
        success = m_python_server.ExecuteTurnEvents();
    } catch (const boost::python::error_already_set& err) {
        success = false;
        m_python_server.HandleErrorAlreadySet();
        if (!m_python_server.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python_server.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                m_fsm->process_event(ShutdownServer());
            }
        }
    }

    if (!success) {
        ErrorLogger() << "Python scripted turn events failed.";
        ServerApp::GetApp()->Networking().SendMessageAll(ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"), false));
    }
}

std::map<int, PlayerInfo> ServerApp::GetPlayerInfoMap() const {
    // compile information about players to send out to other players at start of game.
    DebugLogger() << "ServerApp::GetPlayerInfoMap: Compiling PlayerInfo for each player";
    std::map<int, PlayerInfo> player_info_map;
    for (auto player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();

        int empire_id = PlayerEmpireID(player_id);
        if (empire_id == ALL_EMPIRES)
            ErrorLogger() << "ServerApp::GetPlayerInfoMap: couldn't find an empire for player with id " << player_id;


        // validate some connection info
        Networking::ClientType client_type = player_connection->GetClientType();
        if (client_type != Networking::CLIENT_TYPE_AI_PLAYER && client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            ErrorLogger() << "ServerApp::GetPlayerInfoMap found player connection with unsupported client type.";
        }
        if (player_connection->PlayerName().empty()) {
            ErrorLogger() << "ServerApp::GetPlayerInfoMap found player connection with empty name!";
        }

        // assemble player info for all players
        player_info_map[player_id] = PlayerInfo(player_connection->PlayerName(),
                                                empire_id,
                                                player_connection->GetClientType(),
                                                m_networking.PlayerIsHost(player_connection->PlayerID()));
    }
    return player_info_map;
}

int ServerApp::PlayerEmpireID(int player_id) const {
    auto it = m_player_empire_ids.find(player_id);
    if (it != m_player_empire_ids.end())
        return it->second;
    else
        return ALL_EMPIRES;
}

int ServerApp::EmpirePlayerID(int empire_id) const {
    for (const auto& entry : m_player_empire_ids)
        if (entry.second == empire_id)
            return entry.first;
    return Networking::INVALID_PLAYER_ID;
}

bool ServerApp::IsLocalHumanPlayer(int player_id) {
    auto it = m_networking.GetPlayer(player_id);
    if (it == m_networking.established_end()) {
        ErrorLogger() << "ServerApp::IsLocalHumanPlayer : could not get player connection for player id " << player_id;
        return false;
    }

    PlayerConnectionPtr player_connection = *it;
    return ((player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER) &&
            player_connection->IsLocalConnection());
}

bool ServerApp::IsAvailableName(const std::string& player_name) const {
    if (player_name.empty())
        return false;
    for (auto it = m_networking.established_begin();
         it != m_networking.established_end(); ++it)
    {
        if ((*it)->PlayerName() == player_name)
            return false;
    }
    // check if some name reserved with cookie
    return m_networking.IsAvailableNameInCookies(player_name);
}

bool ServerApp::IsAuthRequiredOrFillRoles(const std::string& player_name, Networking::AuthRoles& roles) {
    bool result = false;
    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonAuthDir());
        // Call the main Python turn events function
        success = m_python_server.IsRequireAuthOrReturnRoles(player_name, result, roles);
    } catch (const boost::python::error_already_set& err) {
        success = false;
        m_python_server.HandleErrorAlreadySet();
        if (!m_python_server.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python_server.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                m_fsm->process_event(ShutdownServer());
            }
        }
    }

    if (!success) {
        ErrorLogger() << "Python scripted authentication failed.";
        ServerApp::GetApp()->Networking().SendMessageAll(ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"),
                                                                      false));
    }
    return result;
}

bool ServerApp::IsAuthSuccessAndFillRoles(const std::string& player_name, const std::string& auth, Networking::AuthRoles& roles) {
    bool result = false;
    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonAuthDir());
        // Call the main Python turn events function
        success = m_python_server.IsSuccessAuthAndReturnRoles(player_name, auth, result, roles);
    } catch (const boost::python::error_already_set& err) {
        success = false;
        m_python_server.HandleErrorAlreadySet();
        if (!m_python_server.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python_server.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                m_fsm->process_event(ShutdownServer());
            }
        }
    }

    if (!success) {
        ErrorLogger() << "Python scripted authentication failed.";
        ServerApp::GetApp()->Networking().SendMessageAll(ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"),
                                                                      false));
    }
    return result;
}

void ServerApp::AddObserverPlayerIntoGame(const PlayerConnectionPtr& player_connection) {
    std::map<int, PlayerInfo> player_info_map = GetPlayerInfoMap();

    Networking::ClientType client_type = player_connection->GetClientType();
    bool use_binary_serialization = player_connection->IsBinarySerializationUsed();

    if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
        client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
    {
        // simply sends GAME_START message so established player will known he is in the game now
        player_connection->SendMessage(GameStartMessage(m_single_player_game, ALL_EMPIRES,
                                                        m_current_turn, m_empires, m_universe,
                                                        GetSpeciesManager(), GetCombatLogManager(),
                                                        GetSupplyManager(), player_info_map,
                                                        m_galaxy_setup_data, use_binary_serialization));

        // send other empires' statuses
        for (const auto& empire : Empires()) {
            auto other_orders_it = m_turn_sequence.find(empire.first);
            bool ready = other_orders_it == m_turn_sequence.end() ||
                    (other_orders_it->second && other_orders_it->second->m_ready);
            player_connection->SendMessage(PlayerStatusMessage(EmpirePlayerID(empire.first),
                                                               ready ? Message::WAITING : Message::PLAYING_TURN,
                                                               empire.first));
        }
    } else {
        ErrorLogger() << "ServerApp::CommonGameInit unsupported client type: skipping game start message.";
    }

    // TODO: notify other players
}

bool ServerApp::EliminatePlayer(const PlayerConnectionPtr& player_connection) {
    if (!GetGameRules().Get<bool>("RULE_ALLOW_CONCEDE")) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CONCEDE_DISABLED"), false));
        return false;
    }

    int player_id = player_connection->PlayerID();
    int empire_id = PlayerEmpireID(player_id);
    if (empire_id == ALL_EMPIRES) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_NONPLAYER_CANNOT_CONCEDE"), false));
        return false;
    }

    // test if there other human players in the game
    bool other_human_player = false;
    for (auto& empires : Empires()) {
        if (!empires.second->Eliminated() &&
            empire_id != empires.second->EmpireID() &&
            GetEmpireClientType(empires.second->EmpireID()) == Networking::CLIENT_TYPE_HUMAN_PLAYER)
        {
            other_human_player = true;
            break;
        }
    }
    if (!other_human_player) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CONCEDE_LAST_HUMAN_PLAYER"), false));
        return false;
    }

    Empire* empire = GetEmpire(empire_id);
    if (!empire) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_NONPLAYER_CANNOT_CONCEDE"), false));
        return false;
    }

    // test for colonies count
    std::vector<int> planet_ids = Objects().FindObjectIDs(OwnedVisitor<Planet>(empire_id));
    if (planet_ids.size() > static_cast<size_t>(GetGameRules().Get<int>("RULE_CONCEDE_COLONIES_THRESHOLD"))) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CONCEDE_EXCEED_COLONIES"), false));
        return false;
    }

    // empire elimination
    empire->Eliminate();

    // unclaim ships and systems
    for (int planet_id : planet_ids) {
        auto planet = GetPlanet(planet_id);
        if (planet)
            planet->Reset();
    }
    for (auto& obj : Objects().FindObjects(OwnedVisitor<Ship>(empire_id))) {
        obj->SetOwner(ALL_EMPIRES);
        GetUniverse().RecursiveDestroy(obj->ID());
    }

    // Don't wait for turn
    RemoveEmpireTurn(empire_id);

    // break link between player and empire
    m_player_empire_ids.erase(player_id);

    // notify other player that this empire finished orders
    // so them don't think player of eliminated empire is making its turn too long
    for (auto player_it = m_networking.established_begin();
        player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player_ctn = *player_it;
        player_ctn->SendMessage(PlayerStatusMessage(player_id,
                                                    Message::WAITING,
                                                    empire_id));
    }

    return true;
}

void ServerApp::DropPlayerEmpireLink(int player_id)
{ m_player_empire_ids.erase(player_id); }

int ServerApp::AddPlayerIntoGame(const PlayerConnectionPtr& player_connection) {
    Empire* empire = nullptr;
    int empire_id = ALL_EMPIRES;
    // search empire by player name
    for (auto e : Empires()) {
        if (e.second->PlayerName() == player_connection->PlayerName()) {
            empire_id = e.first;
            empire = e.second;
            break;
        }
    }

    if (empire_id == ALL_EMPIRES || empire == nullptr)
        return ALL_EMPIRES;

    if (empire->Eliminated())
        return ALL_EMPIRES;

    auto orders_it = m_turn_sequence.find(empire_id);
    if (orders_it == m_turn_sequence.end()) {
        WarnLogger() << "ServerApp::AddPlayerIntoGame empire " << empire_id
                     << " for \"" << player_connection->PlayerName()
                     << "\" doesn't wait for orders";
        return ALL_EMPIRES;
    }

    // make a link to new connection
    m_player_empire_ids[player_connection->PlayerID()] = empire_id;
    empire->SetAuthenticated(player_connection->IsAuthenticated());

    const OrderSet dummy;
    const OrderSet& orders = orders_it->second && orders_it->second->m_orders ? *(orders_it->second->m_orders) : dummy;
    const SaveGameUIData* ui_data = orders_it->second ? orders_it->second->m_ui_data.get() : nullptr;

    // drop ready status
    if (orders_it->second)
        orders_it->second->m_ready = false;

    auto player_info_map = GetPlayerInfoMap();
    bool use_binary_serialization = player_connection->IsBinarySerializationUsed();

    player_connection->SendMessage(GameStartMessage(
        m_single_player_game, empire_id,
        m_current_turn, m_empires, m_universe,
        GetSpeciesManager(), GetCombatLogManager(),
        GetSupplyManager(),  player_info_map, orders,
        ui_data,
        m_galaxy_setup_data,
        use_binary_serialization));

    // send other empires' statuses
    for (const auto& empire : Empires()) {
        auto other_orders_it = m_turn_sequence.find(empire.first);
        bool ready = other_orders_it == m_turn_sequence.end() ||
                (other_orders_it->second && other_orders_it->second->m_ready);
        player_connection->SendMessage(PlayerStatusMessage(EmpirePlayerID(empire.first),
                                                           ready ? Message::WAITING : Message::PLAYING_TURN,
                                                           empire.first));
    }

    return empire_id;
}

bool ServerApp::IsHostless() const
{ return GetOptionsDB().Get<bool>("hostless"); }

const boost::circular_buffer<ChatHistoryEntity>& ServerApp::GetChatHistory() const
{ return m_chat_history; }

std::vector<PlayerSaveGameData> ServerApp::GetPlayerSaveGameData() const {
    std::vector<PlayerSaveGameData> player_save_game_data;
    for (const auto& m_save_data : m_turn_sequence) {
        DebugLogger() << "ServerApp::GetPlayerSaveGameData() Empire " << m_save_data.first
                      << " save_game_data " << m_save_data.second.get();
        if (m_save_data.second) {
            player_save_game_data.push_back(*m_save_data.second);
        }
    }
    return player_save_game_data;
}

Networking::ClientType ServerApp::GetEmpireClientType(int empire_id) const
{ return GetPlayerClientType(ServerApp::EmpirePlayerID(empire_id)); }

Networking::ClientType ServerApp::GetPlayerClientType(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return Networking::INVALID_CLIENT_TYPE;

   auto it = m_networking.GetPlayer(player_id);
    if (it == m_networking.established_end())
        return Networking::INVALID_CLIENT_TYPE;
    PlayerConnectionPtr player_connection = *it;
    return player_connection->GetClientType();
}

int ServerApp::EffectsProcessingThreads() const
{ return GetOptionsDB().Get<int>("effects.server.threads"); }

void ServerApp::AddEmpireTurn(int empire_id, const PlayerSaveGameData& psgd)
{ m_turn_sequence[empire_id] = boost::make_unique<PlayerSaveGameData>(psgd); }

void ServerApp::RemoveEmpireTurn(int empire_id)
{ m_turn_sequence.erase(empire_id); }

void ServerApp::ClearEmpireTurnOrders() {
    for (auto& order : m_turn_sequence) {
        if (order.second) {
            // reset only orders
            // left UI data and AI state intact
            order.second->m_orders.reset();
            order.second->m_ready = false;
        }
    }
}

void ServerApp::SetEmpireSaveGameData(int empire_id, std::unique_ptr<PlayerSaveGameData>&& save_game_data)
{ m_turn_sequence[empire_id] = std::move(save_game_data); }

void ServerApp::UpdatePartialOrders(int empire_id, const OrderSet& added, const std::set<int>& deleted) {
    const auto& psgd = m_turn_sequence[empire_id];
    if (psgd) {
        if (psgd->m_orders) {
            for (int id : deleted)
                 psgd->m_orders->erase(id);
            for (auto it : added)
                 psgd->m_orders->insert(it);
        } else {
            psgd->m_orders = std::make_shared<OrderSet>(added);
        }
    }
}

void ServerApp::RevokeEmpireTurnReadyness(int empire_id)
{
    const auto& psgd = m_turn_sequence[empire_id];
    if (psgd)
        psgd->m_ready = false;
}

bool ServerApp::AllOrdersReceived() {
    // debug output
    DebugLogger() << "ServerApp::AllOrdersReceived for turn: " << m_current_turn;
    bool all_orders_received = true;
    for (const auto& empire_orders : m_turn_sequence) {
        if (!empire_orders.second) {
            DebugLogger() << " ... no save data from empire id: " << empire_orders.first;
            all_orders_received = false;
        } else if (!empire_orders.second->m_orders) {
            DebugLogger() << " ... no orders from empire id: " << empire_orders.first;
            all_orders_received = false;
        } else if (!empire_orders.second->m_ready) {
            DebugLogger() << " ... not ready empire id: " << empire_orders.first;
            all_orders_received = false;
        } else {
            DebugLogger() << " ... have orders from empire id: " << empire_orders.first;
        }
    }
    return all_orders_received;
}

namespace {
    /** Returns true if \a empire has been eliminated by the applicable
      * definition of elimination.  As of this writing, elimination means
      * having no ships and no planets. */
    bool EmpireEliminated(int empire_id) {
          return (Objects().FindObjects(OwnedVisitor<Planet>(empire_id)).empty() &&    // no planets
                  Objects().FindObjects(OwnedVisitor<Ship>(empire_id)).empty());      // no ship
      }

    void GetEmpireFleetsAtSystem(std::map<int, std::set<int>>& empire_fleets, int system_id) {
        empire_fleets.clear();
        auto system = GetSystem(system_id);
        if (!system)
            return;
        for (auto& fleet : Objects().FindObjects<const Fleet>(system->FleetIDs())) {
            empire_fleets[fleet->Owner()].insert(fleet->ID());
        }
    }

    void GetEmpirePlanetsAtSystem(std::map<int, std::set<int>>& empire_planets, int system_id) {
        empire_planets.clear();
        auto system = GetSystem(system_id);
        if (!system)
            return;
        for (auto& planet : Objects().FindObjects<const Planet>(system->PlanetIDs())) {
            if (!planet->Unowned())
                empire_planets[planet->Owner()].insert(planet->ID());
            else if (planet->InitialMeterValue(METER_POPULATION) > 0.0f)
                empire_planets[ALL_EMPIRES].insert(planet->ID());
        }
    }

    void GetFleetsVisibleToEmpireAtSystem(std::set<int>& visible_fleets,
                                          int empire_id, int system_id)
    {
        visible_fleets.clear();
        auto system = GetSystem(system_id);
        if (!system)
            return; // no such system
        const auto& fleet_ids = system->FleetIDs();
        if (fleet_ids.empty())
            return; // no fleets to be seen
        if (empire_id != ALL_EMPIRES && !GetEmpire(empire_id))
            return; // no such empire

        TraceLogger(combat) << "\t** GetFleetsVisibleToEmpire " << empire_id << " at system " << system->Name();
        // for visible fleets by an empire, check visibility of fleets by that empire
        if (empire_id != ALL_EMPIRES) {
            for (int fleet_id : fleet_ids) {
                auto fleet = GetFleet(fleet_id);
                if (!fleet)
                    continue;
                if (fleet->OwnedBy(empire_id))
                    continue;   // don't care about fleets owned by the same empire for determining combat conditions
                Visibility fleet_vis = GetUniverse().GetObjectVisibilityByEmpire(fleet->ID(), empire_id);
                TraceLogger(combat) << "\t\tfleet (" << fleet_id << ") has visibility rank " << fleet_vis;
                if (fleet_vis >= VIS_BASIC_VISIBILITY)
                    visible_fleets.insert(fleet->ID());
            }
            return;
        }


        // now considering only fleets visible to monsters


        // get best monster detection strength here.  Use monster detection meters for this...
        float monster_detection_strength_here = 0.0f;
        for (int ship_id : system->ShipIDs()) {
            auto ship = GetShip(ship_id);
            if (!ship || !ship->Unowned())  // only want unowned / monster ships
                continue;
            if (ship->InitialMeterValue(METER_DETECTION) > monster_detection_strength_here)
                monster_detection_strength_here = ship->InitialMeterValue(METER_DETECTION);
        }

        // test each ship in each fleet for visibility by best monster detection here
        for (int fleet_id : fleet_ids) {
            auto fleet = GetFleet(fleet_id);
            if (!fleet)
                continue;
            if (fleet->Unowned()) {
                visible_fleets.insert(fleet->ID());   // fleet is monster, so can be sen by monsters
                continue;
            }

            for (int ship_id : fleet->ShipIDs()) {
                auto ship = GetShip(ship_id);
                if (!ship)
                    continue;
                // if a ship is low enough stealth, its fleet can be seen by monsters
                if (monster_detection_strength_here >= ship->InitialMeterValue(METER_STEALTH)) {
                    visible_fleets.insert(fleet->ID());
                    break;  // fleet is seen, so don't need to check any more ships in it
                }
            }
        }
    }

    void GetPlanetsVisibleToEmpireAtSystem(std::set<int>& visible_planets,
                                           int empire_id, int system_id)
    {
        visible_planets.clear();
        auto system = GetSystem(system_id);
        if (!system)
            return; // no such system
        const auto& planet_ids = system->PlanetIDs();
        if (planet_ids.empty())
            return; // no planets to be seen
        if (empire_id != ALL_EMPIRES && !GetEmpire(empire_id))
            return; // no such empire

        TraceLogger(combat) << "\t** GetPlanetsVisibleToEmpire " << empire_id << " at system " << system->Name();
        // for visible planets by an empire, check visibility of planet by that empire
        if (empire_id != ALL_EMPIRES) {
            for (int planet_id : planet_ids) {
                // include planets visible to empire
                Visibility planet_vis = GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id);
                TraceLogger(combat) << "\t\tplanet (" << planet_id << ") has visibility rank " << planet_vis;
                if (planet_vis <= VIS_BASIC_VISIBILITY)
                    continue;
                // skip planets that have no owner and that are unpopulated; don't matter for combat conditions test
                auto planet = GetPlanet(planet_id);
                if (planet->Unowned() && planet->InitialMeterValue(METER_POPULATION) <= 0.0f)
                    continue;
                visible_planets.insert(planet->ID());
            }
            return;
        }


        // now considering only planets visible to monsters


        // get best monster detection strength here.  Use monster detection meters for this...
        float monster_detection_strength_here = 0.0f;
        for (auto& ship : Objects().FindObjects<const Ship>(system->ShipIDs())) {
            if (!ship->Unowned())  // only want unowned / monster ships
                continue;
            if (ship->InitialMeterValue(METER_DETECTION) > monster_detection_strength_here)
                monster_detection_strength_here = ship->InitialMeterValue(METER_DETECTION);
        }

        // test each planet for visibility by best monster detection here
        for (auto& planet : Objects().FindObjects<const Planet>(system->PlanetIDs())) {
            if (planet->Unowned())
                continue;       // only want empire-owned planets; unowned planets visible to monsters don't matter for combat conditions test
            // if a planet is low enough stealth, it can be seen by monsters
            if (monster_detection_strength_here >= planet->InitialMeterValue(METER_STEALTH))
                visible_planets.insert(planet->ID());
        }
    }

    /** Returns true iff there is an appropriate combination of objects in the
      * system with id \a system_id for a combat to occur. */
    bool CombatConditionsInSystem(int system_id) {
        // combats occur if:
        // 1) empires A and B are at war, and
        // 2) a) empires A and B both have fleets in a system, or
        // 2) b) empire A has a fleet and empire B has a planet in a system
        // 3) empire A can see the fleet or planet of empire B
        // 4) empire A's fleet is set to aggressive
        // 5) empire A's fleet has at least one armed ship <-- only enforced if empire A is 'monster'
        //
        // monster ships are treated as owned by an empire at war with all other empires (may be passive or aggressive)
        // native planets are treated as owned by an empire at war with all other empires

        // what empires have fleets here? (including monsters as id ALL_EMPIRES)
        std::map<int, std::set<int>> empire_fleets_here;
        GetEmpireFleetsAtSystem(empire_fleets_here, system_id);
        if (empire_fleets_here.empty())
            return false;

        auto this_system = GetSystem(system_id);
        DebugLogger(combat) << "CombatConditionsInSystem() for system (" << system_id << ") " << this_system->Name();
        // which empires have aggressive ships here? (including monsters as id ALL_EMPIRES)
        std::set<int> empires_with_aggressive_fleets_here;
        for (auto& empire_fleets : empire_fleets_here) {
            int empire_id = empire_fleets.first;
            for (int fleet_id : empire_fleets.second) {
                auto fleet = GetFleet(fleet_id);
                if (!fleet)
                    continue;
                // an unarmed Monster will not trigger combat
                if (  (fleet->Aggressive() || fleet->Unowned())  &&
                      (fleet->HasArmedShips() || fleet->HasFighterShips() || !fleet->Unowned())  )
                {
                    if (!empires_with_aggressive_fleets_here.count(empire_id))
                        DebugLogger(combat) << "\t Empire " << empire_id << " has at least one aggressive fleet present";
                    empires_with_aggressive_fleets_here.insert(empire_id);
                    break;
                }
            }
        }
        if (empires_with_aggressive_fleets_here.empty()) {
            DebugLogger(combat) << "\t All fleets present are either passive or both unowned and unarmed: no combat.";
            return false;
        }

        // what empires have planets here?  Unowned planets are included for
        // ALL_EMPIRES if they have population > 0
        std::map<int, std::set<int>> empire_planets_here;
        GetEmpirePlanetsAtSystem(empire_planets_here, system_id);
        if (empire_planets_here.empty() && empire_fleets_here.size() <= 1) {
            DebugLogger(combat) << "\t Only one combatant present: no combat.";
            return false;
        }

        // all empires with something here
        std::set<int> empires_here;
        for (auto& empire_fleets : empire_fleets_here)
        { empires_here.insert(empire_fleets.first); }
        for (auto& empire_planets : empire_planets_here)
        { empires_here.insert(empire_planets.first); }

        // what combinations of present empires are at war?
        std::map<int, std::set<int>> empires_here_at_war;  // for each empire, what other empires here is it at war with?
        for (auto emp1_it = empires_here.begin();
             emp1_it != empires_here.end(); ++emp1_it)
        {
            auto emp2_it = emp1_it;
            ++emp2_it;
            for (; emp2_it != empires_here.end(); ++emp2_it) {
                if (*emp1_it == ALL_EMPIRES || *emp2_it == ALL_EMPIRES ||
                    Empires().GetDiplomaticStatus(*emp1_it, *emp2_it) == DIPLO_WAR)
                {
                    empires_here_at_war[*emp1_it].insert(*emp2_it);
                    empires_here_at_war[*emp2_it].insert(*emp1_it);
                }
            }
        }
        if (empires_here_at_war.empty()) {
            DebugLogger(combat) << "\t No warring combatants present: no combat.";
            return false;
        }

        // is an empire with an aggressive fleet here able to see a planet of an
        // empire it is at war with here?
        for (int aggressive_empire_id : empires_with_aggressive_fleets_here) {
            // what empires is the aggressive empire at war with?
            const auto& at_war_with_empire_ids = empires_here_at_war[aggressive_empire_id];

            // what planets can the aggressive empire see?
            std::set<int> aggressive_empire_visible_planets;
            GetPlanetsVisibleToEmpireAtSystem(aggressive_empire_visible_planets, aggressive_empire_id, system_id);

            // is any planet owned by an empire at war with aggressive empire?
            for (int planet_id : aggressive_empire_visible_planets) {
                auto planet = GetPlanet(planet_id);
                if (!planet)
                    continue;
                int visible_planet_empire_id = planet->Owner();

                if (aggressive_empire_id != visible_planet_empire_id &&
                    at_war_with_empire_ids.count(visible_planet_empire_id))
                {
                    DebugLogger(combat) << "\t Empire " << aggressive_empire_id << " sees target planet " << planet->Name();
                    return true;  // an aggressive empire can see a planet onwned by an empire it is at war with
                }
            }
        }

        // is an empire with an aggressive fleet here able to see a fleet or a
        // planet of an empire it is at war with here?
        for (int aggressive_empire_id : empires_with_aggressive_fleets_here) {
            // what empires is the aggressive empire at war with?
            const auto& at_war_with_empire_ids = empires_here_at_war[aggressive_empire_id];
            if (at_war_with_empire_ids.empty())
                continue;

            // what fleets can the aggressive empire see?
            std::set<int> aggressive_empire_visible_fleets;
            GetFleetsVisibleToEmpireAtSystem(aggressive_empire_visible_fleets, aggressive_empire_id, system_id);

            // is any fleet owned by an empire at war with aggressive empire?
            for (int fleet_id : aggressive_empire_visible_fleets) {
                auto fleet = GetFleet(fleet_id);
                if (!fleet)
                    continue;
                int visible_fleet_empire_id = fleet->Owner();

                if (aggressive_empire_id != visible_fleet_empire_id &&
                    at_war_with_empire_ids.count(visible_fleet_empire_id))
                {
                    DebugLogger(combat) << "\t Empire " << aggressive_empire_id << " sees target fleet " << fleet->Name();
                    return true;  // an aggressive empire can see a fleet onwned by an empire it is at war with
                }
            }
        }

        DebugLogger(combat) << "\t No aggressive fleet can see a target: no combat.";
        return false;   // no possible conditions for combat were found
    }

    /** Clears and refills \a combats with CombatInfo structs for
      * every system where a combat should occur this turn. */
    void AssembleSystemCombatInfo(std::vector<CombatInfo>& combats) {
        combats.clear();
        // for each system, find if a combat will occur in it, and if so, assemble
        // necessary information about that combat in combats
        for (int sys_id : GetUniverse().Objects().FindObjectIDs<System>()) {
            if (CombatConditionsInSystem(sys_id)) {
                combats.push_back(CombatInfo(sys_id, CurrentTurn()));
            }
        }
    }

    /** Back project meter values of objects in combat info, so that changes to
      * meter values from combat aren't lost when resetting meters during meter
      * updating after combat. */
    void BackProjectSystemCombatInfoObjectMeters(std::vector<CombatInfo>& combats) {
        for (CombatInfo& combat : combats) {
            for (const auto& object : combat.objects)
                object->BackPropagateMeters();
        }
    }

    /** Takes contents of CombatInfo struct and puts it into the universe.
      * Used to store results of combat in main universe. */
    void DisseminateSystemCombatInfo(const std::vector<CombatInfo>& combats) {
        Universe& universe = GetUniverse();

        // as of this writing, pointers to objects are inserted into combat
        // ObjectMap, and these pointers refer to the main gamestate objects
        // therefore, copying the combat result state back into the main
        // gamestate object map isn't necessary, as these objects have already
        // been updated by the combat processing. similarly, standard
        // visibility updating will transfer the results to empires' known
        // gamestate ObjectMaps.
        for (const CombatInfo& combat_info : combats) {
            // update visibilities from combat, in case anything was revealed
            // by shooting during combat
            for (const auto& empire_vis : combat_info.empire_object_visibility) {
                for (const auto& object_vis : empire_vis.second) {
                    if (object_vis.first < 0)
                        continue;   // temporary fighter IDs
                    if (object_vis.second > GetUniverse().GetObjectVisibilityByEmpire(object_vis.first, empire_vis.first))
                        universe.SetEmpireObjectVisibility(empire_vis.first, object_vis.first, object_vis.second);
                }
            }


            // indexed by fleet id, which empire ids to inform that a fleet is destroyed
            std::map<int, std::set<int>> empires_to_update_of_fleet_destruction;

            // update which empires know objects are destroyed.  this may
            // duplicate the destroyed object knowledge that is set when the
            // object is destroyed with Universe::Destroy, but there may also
            // be empires in this battle that otherwise couldn't see the object
            // as determined for galaxy map purposes, but which do know it has
            // been destroyed from having observed it during the battle.
            for (const auto& dok : combat_info.destroyed_object_knowers) {
                int empire_id = dok.first;

                for (int object_id : dok.second) {
                    //DebugLogger() << "Setting knowledge of destroyed object " << object_id
                    //                       << " for empire " << empire_id;
                    universe.SetEmpireKnowledgeOfDestroyedObject(object_id, empire_id);

                    // record if empire should be informed of potential fleet
                    // destruction (which is checked later)
                    if (auto ship = GetShip(object_id)) {
                        if (ship->FleetID() != INVALID_OBJECT_ID)
                            empires_to_update_of_fleet_destruction[ship->FleetID()].insert(empire_id);
                    }
                }
            }


            // destroy, in main universe, objects that were destroyed in combat,
            // and any associated objects that should now logically also be
            // destroyed
            std::set<int> all_destroyed_object_ids;
            for (int destroyed_object_id : combat_info.destroyed_object_ids) {
                auto dest_obj_ids = universe.RecursiveDestroy(destroyed_object_id);
                all_destroyed_object_ids.insert(dest_obj_ids.begin(), dest_obj_ids.end());
            }


            // after recursive object destruction, fleets might have been
            // destroyed. If so, need to also update empires knowledge of this
            for (const auto& fleet_empires : empires_to_update_of_fleet_destruction) {
                int fleet_id = fleet_empires.first;
                if (!all_destroyed_object_ids.count(fleet_id))
                    continue;   // fleet wasn't destroyed
                // inform empires
                for (int empire_id : fleet_empires.second) {
                    //DebugLogger() << "Setting knowledge of destroyed object " << fleet_id
                    //                       << " for empire " << empire_id;
                    universe.SetEmpireKnowledgeOfDestroyedObject(fleet_id, empire_id);
                }
            }


            // update system ownership after combat.  may be necessary if the
            // combat caused planets to change ownership.
            if (auto system = GetSystem(combat_info.system_id)) {
                // ensure all participants get updates on system.  this ensures
                // that an empire who lose all objects in the system still
                // knows about a change in system ownership
                for (int empire_id : combat_info.empire_ids)
                { universe.EmpireKnownObjects(empire_id).CopyObject(system, ALL_EMPIRES); }
            }
        }
    }

    /** Creates sitreps for all empires involved in a combat. */
    void CreateCombatSitReps(const std::vector<CombatInfo>& combats) {
        CombatLogManager& log_manager = GetCombatLogManager();

        for (const CombatInfo& combat_info : combats) {
            // add combat log entry
            int log_id = log_manager.AddNewLog(CombatLog(combat_info));

            // basic "combat occured" sitreps
            const std::set<int>& empire_ids = combat_info.empire_ids;
            for (int empire_id : empire_ids) {
                if (Empire* empire = GetEmpire(empire_id))
                    empire->AddSitRepEntry(CreateCombatSitRep(combat_info.system_id, log_id, EnemyId(empire_id, empire_ids)));
            }

            // sitreps about destroyed objects
            for (const auto& empire_kdos : combat_info.destroyed_object_knowers) {
                int empire_id = empire_kdos.first;
                Empire* empire = GetEmpire(empire_id);
                if (!empire)
                    continue;

                for (int dest_obj_id : empire_kdos.second) {
                    //DebugLogger() << "Creating destroyed object sitrep for empire " << empire_id << " and object " << dest_obj_id;
                    //if (auto obj = GetEmpireKnownObject(dest_obj_id, empire_id)) {
                    //    DebugLogger() << "Object known to empire: " << obj->Dump();
                    //} else {
                    //    DebugLogger() << "Object not known to empire";
                    //}
                    empire->AddSitRepEntry(CreateCombatDestroyedObjectSitRep(dest_obj_id, combat_info.system_id,
                                                                             empire_id));
                }
            }

            // sitreps about damaged objects
            for (int damaged_object_id : combat_info.damaged_object_ids) {
                //DebugLogger() << "Checking object " << damaged_object_id << " for damaged sitrep";
                // is object destroyed? If so, don't need a damage sitrep
                if (combat_info.destroyed_object_ids.count(damaged_object_id))
                    //DebugLogger() << "Object is destroyed and doesn't need a sitrep.";
                    continue;
                // which empires know about this object?
                for (const auto& empire_ok : combat_info.empire_object_visibility) {
                    // does this empire know about this object?
                    const auto& empire_known_objects = empire_ok.second;
                    if (!empire_known_objects.count(damaged_object_id))
                        continue;
                    int empire_id = empire_ok.first;
                    if (auto empire = GetEmpire(empire_id))
                        empire->AddSitRepEntry(CreateCombatDamagedObjectSitRep(
                            damaged_object_id, combat_info.system_id, empire_id));
                }
            }
        }
    }

    /** Records info in Empires about what they destroyed or had destroyed during combat. */
    void UpdateEmpireCombatDestructionInfo(const std::vector<CombatInfo>& combats) {
        for (const CombatInfo& combat_info : combats) {
            std::vector<WeaponFireEvent::ConstWeaponFireEventPtr> events_that_killed;
            for (CombatEventPtr event : combat_info.combat_events) {
                auto maybe_attacker = std::dynamic_pointer_cast<WeaponsPlatformEvent>(event);
                if (maybe_attacker) {
                    auto sub_events = maybe_attacker->SubEvents(maybe_attacker->attacker_owner_id);
                    for (auto weapon_event : sub_events) {
                        auto maybe_fire_event = std::dynamic_pointer_cast<const WeaponFireEvent>(weapon_event);
                        if (maybe_fire_event
                                && combat_info.destroyed_object_ids.count(maybe_fire_event->target_id))
                            events_that_killed.push_back(maybe_fire_event);
                    }
                }

                auto maybe_fire_event = std::dynamic_pointer_cast<const WeaponFireEvent>(event);
                if (maybe_fire_event
                        && combat_info.destroyed_object_ids.count(maybe_fire_event->target_id))
                    events_that_killed.push_back(maybe_fire_event);
            }

            // If a ship was attacked multiple times during a combat in which it dies, it will get
            // processed multiple times here.  The below set will keep it from being logged as
            // multiple destroyed ships for its owner.
            // TODO: fix similar issue for overlogging on attacker side
            std::set<int> already_logged__target_ships;
            for (const auto& attack_event : events_that_killed) {
                auto attacker = GetUniverseObject(attack_event->attacker_id);
                if (!attacker)
                    continue;
                int attacker_empire_id = attacker->Owner();
                Empire* attacker_empire = GetEmpire(attacker_empire_id);

                auto target_ship = GetShip(attack_event->target_id);
                if (!target_ship)
                    continue;
                int target_empire_id = target_ship->Owner();
                int target_design_id = target_ship->DesignID();
                const std::string& target_species_name = target_ship->SpeciesName();
                Empire* target_empire = GetEmpire(target_empire_id);

                std::map<int, int>::iterator map_it;
                std::map<std::string, int>::iterator species_it;

                if (attacker_empire) {
                    // record destruction of an empire's ship by attacker empire
                    map_it = attacker_empire->EmpireShipsDestroyed().find(target_empire_id);
                    if (map_it == attacker_empire->EmpireShipsDestroyed().end())
                        attacker_empire->EmpireShipsDestroyed()[target_empire_id] = 1;
                    else
                        map_it->second++;

                    // record destruction of a design by attacker empire
                    map_it = attacker_empire->ShipDesignsDestroyed().find(target_design_id);
                    if (map_it == attacker_empire->ShipDesignsDestroyed().end())
                        attacker_empire->ShipDesignsDestroyed()[target_design_id] = 1;
                    else
                        map_it->second++;

                    // record destruction of ship with a species on it by attacker empire
                    species_it = attacker_empire->SpeciesShipsDestroyed().find(target_species_name);
                    if (species_it == attacker_empire->SpeciesShipsDestroyed().end())
                        attacker_empire->SpeciesShipsDestroyed()[target_species_name] = 1;
                    else
                        species_it->second++;
                }

                if (target_empire) {
                    if (already_logged__target_ships.count(attack_event->target_id))
                        continue;
                    already_logged__target_ships.insert(attack_event->target_id);
                    // record destruction of a ship with a species on it owned by defender empire
                    species_it = target_empire->SpeciesShipsLost().find(target_species_name);
                    if (species_it == target_empire->SpeciesShipsLost().end())
                        target_empire->SpeciesShipsLost()[target_species_name] = 1;
                    else
                        species_it->second++;

                    // record destruction of with a design owned by defender empire
                    map_it = target_empire->ShipDesignsLost().find(target_design_id);
                    if (map_it == target_empire->ShipDesignsLost().end())
                        target_empire->ShipDesignsLost()[target_design_id] = 1;
                    else
                        map_it->second++;
                }
            }
        }
    }

    /** Records info in Empires about where they invaded. */
    void UpdateEmpireInvasionInfo(const std::map<int, std::map<int, double>>& planet_empire_invasion_troops) {
        for (const auto& planet_empire_troops : planet_empire_invasion_troops) {
            int planet_id = planet_empire_troops.first;
            auto planet = GetPlanet(planet_id);
            if (!planet)
                continue;
            const auto& planet_species = planet->SpeciesName();
            if (planet_species.empty())
                continue;

            for (const auto& empire_troops : planet_empire_troops.second) {
                Empire* invader_empire = GetEmpire(empire_troops.first);
                if (!invader_empire)
                    continue;

                auto species_it = invader_empire->SpeciesPlanetsInvaded().find(planet_species);
                if (species_it == invader_empire->SpeciesPlanetsInvaded().end())
                    invader_empire->SpeciesPlanetsInvaded()[planet_species] = 1;
                else
                    species_it->second++;
            }
        }
    }

    /** Does colonization, with safety checks */
    bool ColonizePlanet(int ship_id, int planet_id) {
        auto ship = GetShip(ship_id);
        if (!ship) {
            ErrorLogger() << "ColonizePlanet couldn't get ship with id " << ship_id;
            return false;
        }
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "ColonizePlanet couldn't get planet with id " << planet_id;
            return false;
        }

        // get species to colonize with: species of ship
        const auto& species_name = ship->SpeciesName();
        if (species_name.empty()) {
            ErrorLogger() << "ColonizePlanet ship has no species";
            return false;
        }

        float colonist_capacity = ship->ColonyCapacity();

        if (colonist_capacity > 0.0f && planet->EnvironmentForSpecies(ship->SpeciesName()) < PE_HOSTILE) {
            ErrorLogger() << "ColonizePlanet nonzero colonist capacity and planet that ship's species can't colonize";
            return false;
        }

        // get empire to give ownership of planet to
        if (ship->Unowned()) {
            ErrorLogger() << "ColonizePlanet couldn't get an empire to colonize with";
            return false;
        }
        int empire_id = ship->Owner();

        // all checks passed.  proceed with colonization.

        // colonize planet by calling Planet class Colonize member function
        // do this BEFORE destroying the ship, since species_name is a const reference to Ship::m_species_name
        if (!planet->Colonize(empire_id, species_name, colonist_capacity)) {
            ErrorLogger() << "ColonizePlanet: couldn't colonize planet";
            return false;
        }

        auto system = GetSystem(ship->SystemID());

        // destroy colonizing ship, and its fleet if now empty
        auto fleet = GetFleet(ship->FleetID());
        if (fleet) {
            fleet->RemoveShips({ship->ID()});
            if (fleet->Empty()) {
                if (system)
                    system->Remove(fleet->ID());
                GetUniverse().Destroy(fleet->ID());
            }
        }

        if (system)
            system->Remove(ship->ID());
        GetUniverse().RecursiveDestroy(ship->ID()); // does not count as a loss of a ship for the species / empire

        return true;
    }

    /** Determines which ships ordered to colonize planet succeed, does
      * appropriate colonization, and cleans up after colonization orders */
    void HandleColonization() {
        // collect, for each planet, what ships have been ordered to colonize it
        std::map<int, std::map<int, std::set<int>>> planet_empire_colonization_ship_ids; // map from planet ID to map from empire ID to set of ship IDs

        for (auto& ship : GetUniverse().Objects().FindObjects<Ship>()) {
            if (ship->Unowned())
                continue;
            int owner_empire_id = ship->Owner();
            int ship_id = ship->ID();
            if (ship_id == INVALID_OBJECT_ID)
                continue;

            int colonize_planet_id = ship->OrderedColonizePlanet();
            if (colonize_planet_id == INVALID_OBJECT_ID)
                continue;

            ship->SetColonizePlanet(INVALID_OBJECT_ID); // reset so failed colonization doesn't leave ship with hanging colonization order set

            auto planet = GetPlanet(colonize_planet_id);
            if (!planet)
                continue;

            if (ship->SystemID() != planet->SystemID() || ship->SystemID() == INVALID_OBJECT_ID)
                continue;

            planet->ResetIsAboutToBeColonized();

            planet_empire_colonization_ship_ids[colonize_planet_id][owner_empire_id].insert(ship_id);
        }


        std::vector<int> newly_colonize_planet_ids;

        // execute colonization except when:
        // 1) an enemy empire has armed aggressive ships in the system
        // 2) multiple empires try to colonize a planet on the same turn
        for (auto& planet_colonization : planet_empire_colonization_ship_ids) {
            // can't colonize if multiple empires attempting to do so on same turn
            auto& empires_ships_colonizing = planet_colonization.second;
            if (empires_ships_colonizing.size() != 1)
                continue;
            int colonizing_empire_id = empires_ships_colonizing.begin()->first;

            const auto& empire_ships_colonizing = empires_ships_colonizing.begin()->second;
            if (empire_ships_colonizing.empty())
                continue;
            int colonizing_ship_id = *empire_ships_colonizing.begin();

            int planet_id = planet_colonization.first;
            auto planet = GetPlanet(planet_id);
            if (!planet) {
                ErrorLogger() << "HandleColonization couldn't get planet with id " << planet_id;
                continue;
            }
            int system_id = planet->SystemID();
            auto system = GetSystem(system_id);
            if (!system) {
                ErrorLogger() << "HandleColonization couldn't get system with id " << system_id;
                continue;
            }

            // find which empires have aggressive armed ships in system
            std::set<int> empires_with_armed_ships_in_system;
            for (auto& fleet : Objects().FindObjects<const Fleet>(system->FleetIDs())) {
                if (fleet->Aggressive() && (fleet->HasArmedShips() || fleet->HasFighterShips()))
                    empires_with_armed_ships_in_system.insert(fleet->Owner());  // may include ALL_EMPIRES, which is fine; this makes monsters prevent colonization
            }

            // are any of the empires with armed ships in the system enemies of the colonzing empire?
            bool colonize_blocked = false;
            for (int armed_ship_empire_id : empires_with_armed_ships_in_system) {
                if (armed_ship_empire_id == colonizing_empire_id)
                    continue;
                if (armed_ship_empire_id == ALL_EMPIRES ||
                    Empires().GetDiplomaticStatus(colonizing_empire_id, armed_ship_empire_id) == DIPLO_WAR)
                {
                    colonize_blocked = true;
                    break;
                }
            }

            if (colonize_blocked)
                continue;

            // before actual colonization, which deletes the colony ship, store ship info for later use with sitrep generation
            auto ship = GetShip(colonizing_ship_id);
            if (!ship)
                ErrorLogger() << "HandleColonization couldn't get ship with id " << colonizing_ship_id;
            const auto& species_name = ship ? ship->SpeciesName() : "";
            float colonist_capacity = ship ? ship->ColonyCapacity() : 0.0f;


            // do colonization
            if (!ColonizePlanet(colonizing_ship_id, planet_id))
                continue;   // skip sitrep if colonization failed

            // record successful colonization
            newly_colonize_planet_ids.push_back(planet_id);

            // sitrep about colonization / outposting
            Empire* empire = GetEmpire(colonizing_empire_id);
            if (!empire) {
                ErrorLogger() << "HandleColonization couldn't get empire with id " << colonizing_empire_id;
            } else {
                if (species_name.empty() || colonist_capacity <= 0.0f)
                    empire->AddSitRepEntry(CreatePlanetOutpostedSitRep(planet_id));
                else
                    empire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet_id, species_name));
            }
        }
    }

    /** Given initial set of ground forces on planet, determine ground forces on
      * planet after a turn of ground combat. */
    void ResolveGroundCombat(std::map<int, double>& empires_troops) {
        if (empires_troops.empty() || empires_troops.size() == 1)
            return;

        std::multimap<double, int> inverted_empires_troops;
        for (const auto& entry : empires_troops)
            inverted_empires_troops.insert({entry.second, entry.first});

        // everyone but victor loses all troops.  victor's troops remaining are
        // what the victor started with minus what the second-largest troop
        // amount was
        auto victor_it = inverted_empires_troops.rbegin();
        auto next_it = victor_it;
        ++next_it;
        int victor_id = victor_it->second;
        double victor_troops = victor_it->first - next_it->first;

        empires_troops.clear();
        empires_troops[victor_id] = victor_troops;
    }

    /** Determines which ships ordered to invade planets, does invasion and
      * ground combat resolution */
    void HandleInvasion() {
        std::map<int, std::map<int, double>> planet_empire_troops;  // map from planet ID to map from empire ID to pair consisting of set of ship IDs and amount of troops empires have at planet

        // assemble invasion forces from each invasion ship
        for (auto& ship : Objects().FindObjects<Ship>()) {
            if (!ship->HasTroops())     // can't invade without troops
                continue;
            if (ship->SystemID() == INVALID_OBJECT_ID)
                continue;

            int invade_planet_id = ship->OrderedInvadePlanet();
            if (invade_planet_id == INVALID_OBJECT_ID)
                continue;
            auto planet = GetPlanet(invade_planet_id);
            if (!planet)
                continue;
            planet->ResetIsAboutToBeInvaded();

            if (ship->SystemID() != planet->SystemID())
                continue;
            if (ship->SystemID() == INVALID_OBJECT_ID)
                continue;

            // how many troops are invading?
            planet_empire_troops[invade_planet_id][ship->Owner()] += ship->TroopCapacity();

            auto system = GetSystem(ship->SystemID());

            // destroy invading ships and their fleets if now empty
            auto fleet = GetFleet(ship->FleetID());
            if (fleet) {
                fleet->RemoveShips({ship->ID()});
                if (fleet->Empty()) {
                    if (system)
                        system->Remove(fleet->ID());
                    GetUniverse().Destroy(fleet->ID());
                }
            }
            if (system)
                system->Remove(ship->ID());

            DebugLogger() << "HandleInvasion has accounted for "<< ship->TroopCapacity()
                          << " troops to invade " << planet->Name()
                          << " and is destroying ship " << ship->ID()
                          << " named " << ship->Name();

            GetUniverse().RecursiveDestroy(ship->ID()); // does not count as ship loss for empire/species
        }

        // store invasion info in empires
        UpdateEmpireInvasionInfo(planet_empire_troops);

        // check each planet for other troops, such as due to empire troops, native troops, or rebel troops
        for (auto& planet : Objects().FindObjects<Planet>()) {
            if (!planet) {
                ErrorLogger() << "HandleInvasion couldn't get planet";
                continue;
            }
            if (planet->InitialMeterValue(METER_TROOPS) > 0.0f) {
                // empires may have garrisons on planets
                planet_empire_troops[planet->ID()][planet->Owner()] += planet->InitialMeterValue(METER_TROOPS) + 0.0001;    // small bonus to ensure ties are won by initial owner
            }
            if (!planet->Unowned() && planet->InitialMeterValue(METER_REBEL_TROOPS) > 0.0f) {
                // rebels may be present on empire-owned planets
                planet_empire_troops[planet->ID()][ALL_EMPIRES] += planet->InitialMeterValue(METER_REBEL_TROOPS);
            }
        }

        // process each planet's ground combats
        for (auto& planet_combat : planet_empire_troops) {
            int planet_id = planet_combat.first;
            auto planet = GetPlanet(planet_id);
            std::set<int> all_involved_empires;
            int planet_initial_owner_id = planet->Owner();

            auto& empires_troops = planet_combat.second;
            if (empires_troops.empty())
                continue;
            else if (empires_troops.size() == 1) {
                int empire_with_troops_id = empires_troops.begin()->first;
                if (planet->Unowned() && empire_with_troops_id == ALL_EMPIRES)
                    continue;

                if (planet->OwnedBy(empire_with_troops_id)) {
                    continue;   // if troops all belong to planet owner, not a combat.

                } else {
                    //DebugLogger() << "Ground combat on " << planet->Name() << " was unopposed";
                    if (planet_initial_owner_id != ALL_EMPIRES)
                        all_involved_empires.insert(planet_initial_owner_id);
                    if (empire_with_troops_id != ALL_EMPIRES)
                        all_involved_empires.insert(empire_with_troops_id);
                }
            } else {
                DebugLogger() << "Ground combat troops on " << planet->Name() << " :";
                for (const auto& empire_troops : empires_troops)
                { DebugLogger() << " ... empire: " << empire_troops.first << " : " << empire_troops.second; }

                // create sitreps for all empires involved in battle
                for (const auto& empire_troops : empires_troops) {
                    if (empire_troops.first != ALL_EMPIRES)
                        all_involved_empires.insert(empire_troops.first);
                }

                ResolveGroundCombat(empires_troops);
            }

            for (int empire_id : all_involved_empires) {
                if (Empire* empire = GetEmpire(empire_id))
                    empire->AddSitRepEntry(CreateGroundCombatSitRep(planet_id, EnemyId(empire_id, all_involved_empires)));
            }

            // who won?
            if (empires_troops.size() == 1) {
                int victor_id = empires_troops.begin()->first;
                // single empire was victorious.  conquer planet if appropriate...
                // if planet is unowned and victor is an empire, or if planet is
                // owned by an empire that is not the victor, conquer it
                if ((planet->Unowned() && victor_id != ALL_EMPIRES) ||
                    (!planet->Unowned() && !planet->OwnedBy(victor_id)))
                {
                    planet->Conquer(victor_id);

                    // create planet conquered sitrep for all involved empires
                    for (int empire_id : all_involved_empires) {
                        if (Empire* empire = GetEmpire(empire_id))
                            empire->AddSitRepEntry(CreatePlanetCapturedSitRep(planet_id, victor_id));
                    }

                    DebugLogger() << "Empire conquers planet";
                    for (const auto& empire_troops : empires_troops)
                    { DebugLogger() << " empire: " << empire_troops.first << ": " << empire_troops.second; }


                } else if (!planet->Unowned() && victor_id == ALL_EMPIRES) {
                    planet->Conquer(ALL_EMPIRES);
                    DebugLogger() << "Independents conquer planet";
                    for (const auto& empire_troops : empires_troops)
                    { DebugLogger() << " empire: " << empire_troops.first << ": " << empire_troops.second; }

                    // TODO: planet lost to rebels sitrep
                } else {
                    // defender held theh planet
                    DebugLogger() << "Defender holds planet";
                    for (const auto& empire_troops : empires_troops)
                    { DebugLogger() << " empire: " << empire_troops.first << ": " << empire_troops.second; }
                }

                // regardless of whether battle resulted in conquering, it did
                // likely affect the troop numbers on the planet
                if (Meter* meter = planet->GetMeter(METER_TROOPS))
                    meter->SetCurrent(empires_troops.begin()->second);  // new troops on planet is remainder after battle

            } else {
                // no troops left?
                if (Meter* meter = planet->GetMeter(METER_TROOPS))
                    meter->SetCurrent(0.0);
                if (Meter* meter = planet->GetMeter(METER_REBEL_TROOPS))
                    meter->SetCurrent(0.0);
            }

            planet->BackPropagateMeters();

            // knowledge update to ensure previous owner of planet knows who owns it now?
            if (planet_initial_owner_id != ALL_EMPIRES && planet_initial_owner_id != planet->Owner()) {
                // get empire's knowledge of object
                ObjectMap& empire_latest_known_objects = EmpireKnownObjects(planet_initial_owner_id);
                empire_latest_known_objects.CopyObject(planet, planet_initial_owner_id);
            }
        }
    }

    /** Determines which fleets or planets ordered given to other empires,
      * and sets their new ownership */
    void HandleGifting() {
        std::map<int, std::vector<std::shared_ptr<UniverseObject>>> empire_gifted_objects;

        // collect fleets ordered to be given
        for (auto& fleet : GetUniverse().Objects().FindObjects<Fleet>()) {
            int ordered_given_to_empire_id = fleet->OrderedGivenToEmpire();
            if (ordered_given_to_empire_id == ALL_EMPIRES)
                continue;
            fleet->ClearGiveToEmpire(); // in case things fail, to avoid potential inconsistent state

            if (fleet->Unowned()
                || fleet->OwnedBy(ordered_given_to_empire_id)
                || !fleet->TravelRoute().empty())
            { continue; }

            empire_gifted_objects[ordered_given_to_empire_id].push_back(fleet);
        }

        // collect planets ordered to be given
        for (auto& planet : GetUniverse().Objects().FindObjects<Planet>()) {
            int ordered_given_to_empire_id = planet->OrderedGivenToEmpire();
            if (ordered_given_to_empire_id == ALL_EMPIRES)
                continue;
            planet->ClearGiveToEmpire(); // in case things fail, to avoid potential inconsistent state

            if (planet->Unowned() || planet->OwnedBy(ordered_given_to_empire_id))
            { continue; }

            empire_gifted_objects[ordered_given_to_empire_id].push_back(planet);
        }

        // further filter ordered given objects and do giving if appropriate
        for (auto& gifted_objects : empire_gifted_objects) {
            int recipient_empire_id = gifted_objects.first;
            std::map<int, bool> systems_contain_recipient_empire_owned_objects;

            // for each recipient empire, process objects it is being gifted
            for (auto& gifted_obj : gifted_objects.second) {
                int initial_owner_empire_id = gifted_obj->Owner();


                // gifted object must be in a system
                if (gifted_obj->SystemID() == INVALID_OBJECT_ID)
                    continue;
                auto system = GetSystem(gifted_obj->SystemID());
                if (!system)
                    continue;

                // the recipient must have an owned object in the same system
                bool can_receive_here = false;

                // is reception ability for this location cached?
                auto sys_it = systems_contain_recipient_empire_owned_objects.find(system->ID());
                if (sys_it != systems_contain_recipient_empire_owned_objects.end()) {
                    can_receive_here = sys_it->second;

                } else {
                    // not cached, so scan for objects
                    for (auto& system_obj : Objects().FindObjects<const UniverseObject>(system->ObjectIDs())) {
                        if (system_obj->OwnedBy(recipient_empire_id)) {
                            can_receive_here = true;
                            systems_contain_recipient_empire_owned_objects[system->ID()] = true;
                            break;
                        }
                    }
                    if (!can_receive_here)
                        systems_contain_recipient_empire_owned_objects[system->ID()] = false;
                }
                if (!can_receive_here)
                    continue;

                // recipient empire can receive objects at this system, so do transfer
                for (auto& contained_obj : Objects().FindObjects<UniverseObject>(gifted_obj->ContainedObjectIDs())) {
                    if (contained_obj->OwnedBy(initial_owner_empire_id))
                        contained_obj->SetOwner(recipient_empire_id);
                }
                gifted_obj->SetOwner(recipient_empire_id);
            }
        }
    }

    /** Destroys suitable objects that have been ordered scrapped.*/
    void HandleScrapping() {
        //// debug
        //for (auto& ship : Objects().FindObjects<Ship>()) {
        //    if (!ship->OrderedScrapped())
        //        continue;
        //    DebugLogger() << "... ship: " << ship->ID() << " ordered scrapped";
        //}
        //// end debug

        for (auto& ship : Objects().FindObjects<Ship>()) {
            if (!ship->OrderedScrapped())
                continue;

            DebugLogger() << "... ship: " << ship->ID() << " ordered scrapped";

            auto system = GetSystem(ship->SystemID());
            if (system)
                system->Remove(ship->ID());

            auto fleet = GetFleet(ship->FleetID());
            if (fleet) {
                fleet->RemoveShips({ship->ID()});
                if (fleet->Empty()) {
                    //scrapped_object_ids.push_back(fleet->ID());
                    system->Remove(fleet->ID());
                    GetUniverse().Destroy(fleet->ID());
                }
            }

            // record scrapping in empire stats
            Empire* scrapping_empire = GetEmpire(ship->Owner());
            if (scrapping_empire) {
                auto& designs_scrapped = scrapping_empire->ShipDesignsScrapped();
                if (designs_scrapped.count(ship->DesignID()))
                    designs_scrapped[ship->DesignID()]++;
                else
                    designs_scrapped[ship->DesignID()] = 1;

                auto& species_ships_scrapped = scrapping_empire->SpeciesShipsScrapped();
                if (species_ships_scrapped.count(ship->SpeciesName()))
                    species_ships_scrapped[ship->SpeciesName()]++;
                else
                    species_ships_scrapped[ship->SpeciesName()] = 1;
            }

            //scrapped_object_ids.push_back(ship->ID());
            GetUniverse().Destroy(ship->ID());
        }

        for (auto& building : Objects().FindObjects<Building>()) {
            if (!building->OrderedScrapped())
                continue;

            if (auto planet = GetPlanet(building->PlanetID()))
                planet->RemoveBuilding(building->ID());

            if (auto system = GetSystem(building->SystemID()))
                system->Remove(building->ID());

            // record scrapping in empire stats
            Empire* scrapping_empire = GetEmpire(building->Owner());
            if (scrapping_empire) {
                auto& buildings_scrapped = scrapping_empire->BuildingTypesScrapped();
                if (buildings_scrapped.count(building->BuildingTypeName()))
                    buildings_scrapped[building->BuildingTypeName()]++;
                else
                    buildings_scrapped[building->BuildingTypeName()] = 1;
            }

            //scrapped_object_ids.push_back(building->ID());
            GetUniverse().Destroy(building->ID());
        }
    }

    /** Removes bombardment state info from objects. Actual effects of
      * bombardment are handled during */
    void CleanUpBombardmentStateInfo() {
        for (auto& ship : GetUniverse().Objects().FindObjects<Ship>())
        { ship->ClearBombardPlanet(); }
        for (auto& planet : GetUniverse().Objects().FindObjects<Planet>()) {
            if (planet->IsAboutToBeBombarded()) {
                //DebugLogger() << "CleanUpBombardmentStateInfo: " << planet->Name() << " was about to be bombarded";
                planet->ResetIsAboutToBeBombarded();
            }
        }
    }

    /** Causes ResourceCenters (Planets) to update their focus records */
    void UpdateResourceCenterFocusHistoryInfo() {
        for (auto& planet : GetUniverse().Objects().FindObjects<Planet>()) {
            planet->UpdateFocusHistory();
        }
    }

    /** Deletes empty fleets. */
    void CleanEmptyFleets() {
        for (auto& fleet : Objects().FindObjects<Fleet>()) {
            if (!fleet->Empty())
                continue;

            auto sys = GetSystem(fleet->SystemID());
            if (sys)
                sys->Remove(fleet->ID());

            GetUniverse().RecursiveDestroy(fleet->ID());
        }
    }
}

void ServerApp::PreCombatProcessTurns() {
    ScopedTimer timer("ServerApp::PreCombatProcessTurns", true);
    ObjectMap& objects = m_universe.Objects();

    m_universe.ResetAllObjectMeters(false, true);   // revert current meter values to initial values prior to update after incrementing turn number during previous post-combat turn processing.

    m_universe.UpdateEmpireVisibilityFilteredSystemGraphs();


    DebugLogger() << "ServerApp::ProcessTurns executing orders";

    // inform players of order execution
    m_networking.SendMessageAll(TurnProgressMessage(Message::PROCESSING_ORDERS));

    // clear bombardment state before executing orders, so result after is only
    // determined by what orders set.
    CleanUpBombardmentStateInfo();

    // execute orders
    for (const auto& empire_orders : m_turn_sequence) {
        auto& save_game_data = empire_orders.second;
        if (!save_game_data) {
            DebugLogger() << "No SaveGameData for empire " << empire_orders.first;
            continue;
        }
        if (!save_game_data->m_orders) {
            DebugLogger() << "No OrderSet for empire " << empire_orders.first;
            continue;
        }
        for (const auto& id_and_order : *save_game_data->m_orders)
            id_and_order.second->Execute();
    }

    // clean up orders, which are no longer needed
    ClearEmpireTurnOrders();

    // update ResourceCenter focus history info
    UpdateResourceCenterFocusHistoryInfo();

    // clean up empty fleets that empires didn't order deleted
    CleanEmptyFleets();

    // update production queues after order execution
    for (auto& entry : Empires()) {
        if (entry.second->Eliminated())
            continue;   // skip eliminated empires
        entry.second->UpdateProductionQueue();
    }

    // player notifications
    m_networking.SendMessageAll(TurnProgressMessage(Message::COLONIZE_AND_SCRAP));

    DebugLogger() << "ServerApp::ProcessTurns colonization";
    HandleColonization();

    DebugLogger() << "ServerApp::ProcessTurns invasion";
    HandleInvasion();

    DebugLogger() << "ServerApp::ProcessTurns gifting";
    HandleGifting();

    DebugLogger() << "ServerApp::ProcessTurns scrapping";
    HandleScrapping();


    DebugLogger() << "ServerApp::ProcessTurns movement";
    // process movement phase

    // player notifications
    m_networking.SendMessageAll(TurnProgressMessage(Message::FLEET_MOVEMENT));


    // fleet movement
    auto fleets = objects.FindObjects<Fleet>();
    for (auto& fleet : fleets) {
        if (fleet)
            fleet->ClearArrivalFlag();
    }
    // first move unowned fleets, or an empire fleet landing on them could wrongly
    // blockade them before they move
    for (auto& fleet : fleets) {
        // save for possible SitRep generation after moving...
        if (fleet && fleet->Unowned())
            fleet->MovementPhase();
    }
    for (auto& fleet : fleets) {
        // save for possible SitRep generation after moving...
        if (fleet && !fleet->Unowned())
            fleet->MovementPhase();
    }

    // post-movement visibility update
    m_universe.UpdateEmpireObjectVisibilities();
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();
    m_universe.UpdateEmpireStaleObjectKnowledge();

    // SitRep for fleets having arrived at destinations
    for (auto& fleet : fleets) {
        // save for possible SitRep generation after moving...
        if (!fleet || !fleet->ArrivedThisTurn())
            continue;
        // sitreps for all empires that can see fleet at new location
        for (auto& entry : Empires()) {
            if (fleet->GetVisibility(entry.first) >= VIS_BASIC_VISIBILITY)
                entry.second->AddSitRepEntry(
                    CreateFleetArrivedAtDestinationSitRep(fleet->SystemID(), fleet->ID(),
                                                          entry.first));
        }
    }

    // indicate that the clients are waiting for their new Universes
    m_networking.SendMessageAll(TurnProgressMessage(Message::DOWNLOADING));

    // send partial turn updates to all players after orders and movement
    // exclude those without empire and who are not Observer or Moderator
    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        int empire_id = PlayerEmpireID(player->PlayerID());
        const Empire* empire = GetEmpire(empire_id);
        if (empire ||
            player->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR ||
            player->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
        {
            bool use_binary_serialization = player->IsBinarySerializationUsed();
            player->SendMessage(TurnPartialUpdateMessage(PlayerEmpireID(player->PlayerID()),
                                                         m_universe, use_binary_serialization));
        }
    }
}

void ServerApp::ProcessCombats() {
    ScopedTimer timer("ServerApp::ProcessCombats", true);
    DebugLogger() << "ServerApp::ProcessCombats";
    m_networking.SendMessageAll(TurnProgressMessage(Message::COMBAT));

    std::vector<CombatInfo> combats;   // map from system ID to CombatInfo for that system


    // collect data about locations where combat is to occur
    AssembleSystemCombatInfo(combats);

    // loop through assembled combat infos, handling each combat to update the
    // various systems' CombatInfo structs
    for (CombatInfo& combat_info : combats) {
        if (auto system = combat_info.GetSystem())
            system->SetLastTurnBattleHere(CurrentTurn());

        auto combat_system = combat_info.GetSystem();
        DebugLogger(combat) << "Processing combat at " << (combat_system ? combat_system->Name() : "(No System)");
        TraceLogger(combat) << combat_info.objects.Dump();

        AutoResolveCombat(combat_info);
    }

    BackProjectSystemCombatInfoObjectMeters(combats);

    UpdateEmpireCombatDestructionInfo(combats);

    DisseminateSystemCombatInfo(combats);
    // update visibilities with any new info gleaned during combat
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();
    // update stale object info based on any mid- combat glimpses
    // before visibility is totally recalculated in the post combat processing
    m_universe.UpdateEmpireStaleObjectKnowledge();

    CreateCombatSitReps(combats);
}

void ServerApp::UpdateMonsterTravelRestrictions() {
    for (auto const &maybe_system : m_universe.Objects().ExistingSystems()) {
        auto system = std::dynamic_pointer_cast<const System>(maybe_system.second);
        if (!system) {
            ErrorLogger() << "Non System object in ExistingSystems with id = " << maybe_system.second->ID();
            continue;
        }

        bool unrestricted_monsters_present = false;
        bool empires_present = false;
        bool unrestricted_empires_present = false;
        std::vector<std::shared_ptr<Fleet>> monsters;
        for (auto maybe_fleet : m_universe.Objects().FindObjects(system->FleetIDs())) {
            auto fleet = std::dynamic_pointer_cast<Fleet>(maybe_fleet);
            if (!fleet) {
                ErrorLogger() << "Non Fleet object in system(" << system->ID()
                              << ") fleets with id = " << maybe_fleet->ID();
                continue;
            }
            // will not require visibility for empires to block clearing of monster travel restrictions
            // unrestricted lane access (i.e, (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for
            // order of arrival -- if an enemy has unrestricted lane access and you don't, they must have arrived
            // before you, or be in cahoots with someone who did.
            bool unrestricted = ((fleet->ArrivalStarlane() == system->ID())
                                 && fleet->Aggressive()
                                 && (fleet->HasArmedShips() || fleet->HasFighterShips()));
            if (fleet->Unowned()) {
                monsters.push_back(fleet);
                if (unrestricted)
                    unrestricted_monsters_present = true;
            } else {
                empires_present = true;
                if (unrestricted)
                    unrestricted_empires_present = true;
            }
        }

        // Prevent monsters from leaving any empire blockade.
        if (unrestricted_empires_present) {
            for (auto &monster_fleet : monsters) {
                monster_fleet->SetArrivalStarlane(INVALID_OBJECT_ID);
            }
        }

        // Break monster blockade after combat.
        if (empires_present && unrestricted_monsters_present) {
            for (auto &monster_fleet : monsters) {
                monster_fleet->SetArrivalStarlane(INVALID_OBJECT_ID);
            }
        }
    }
}

void ServerApp::PostCombatProcessTurns() {
    ScopedTimer timer("ServerApp::PostCombatProcessTurns", true);

    EmpireManager& empires = Empires();
    ObjectMap& objects = m_universe.Objects();

    // post-combat visibility update
    m_universe.UpdateEmpireObjectVisibilities();
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();


    // check for loss of empire capitals
    for (auto& entry : empires) {
        int capital_id = entry.second->CapitalID();
        if (auto capital = GetUniverseObject(capital_id)) {
            if (!capital->OwnedBy(entry.first))
                entry.second->SetCapitalID(INVALID_OBJECT_ID);
        } else {
            entry.second->SetCapitalID(INVALID_OBJECT_ID);
        }
    }


    // process production and growth phase

    // notify players that production and growth is being processed
    m_networking.SendMessageAll(TurnProgressMessage(Message::EMPIRE_PRODUCTION));
    DebugLogger() << "ServerApp::PostCombatProcessTurns effects and meter updates";

    TraceLogger(effects) << "!!!!!!! BEFORE TURN PROCESSING EFFECTS APPLICATION";
    TraceLogger(effects) << objects.Dump();

    // execute all effects and update meters prior to production, research, etc.
    if (GetGameRules().Get<bool>("RULE_RESEED_PRNG_SERVER")) {
        static boost::hash<std::string> pcpt_string_hash;
        Seed(static_cast<unsigned int>(CurrentTurn()) + pcpt_string_hash(m_galaxy_setup_data.m_seed));
    }
    m_universe.ApplyAllEffectsAndUpdateMeters(false);

    // regenerate system connectivity graph after executing effects, which may
    // have added or removed starlanes.
    m_universe.InitializeSystemGraph();

    TraceLogger(effects) << "!!!!!!! AFTER TURN PROCESSING EFFECTS APPLICATION";
    TraceLogger(effects) << objects.Dump();

    DebugLogger() << "ServerApp::PostCombatProcessTurns empire resources updates";

    // now that we've had combat and applied Effects, update visibilities again, prior
    //  to updating system obstructions below.
    m_universe.UpdateEmpireObjectVisibilities();
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();

    UpdateEmpireSupply();

    // Update fleet travel restrictions (monsters and empire fleets)
    UpdateMonsterTravelRestrictions();
    for (auto& entry : empires) {
        if (!entry.second->Eliminated()) {
            Empire* empire = entry.second;
            empire->UpdatePreservedLanes();
            empire->UpdateUnobstructedFleets();     // must be done after *all* noneliminated empires have updated their unobstructed systems
        }
    }

    TraceLogger(effects) << "!!!!!!! AFTER UPDATING RESOURCE POOLS AND SUPPLY STUFF";
    TraceLogger(effects) << objects.Dump();

    DebugLogger() << "ServerApp::PostCombatProcessTurns queue progress checking";

    // Consume distributed resources to planets and on queues, create new
    // objects for completed production and give techs to empires that have
    // researched them
    for (auto& entry : empires) {
        Empire* empire = entry.second;
        if (empire->Eliminated())
            continue;   // skip eliminated empires

        empire->CheckResearchProgress();
        empire->CheckProductionProgress();
        empire->CheckTradeSocialProgress();
    }

    TraceLogger(effects) << "!!!!!!! AFTER CHECKING QUEUE AND RESOURCE PROGRESS";
    TraceLogger(effects) << objects.Dump();

    // execute turn events implemented as Python scripts
    ExecuteScriptedTurnEvents();

    // Execute meter-related effects on objects created this turn, so that new
    // UniverseObjects will have effects applied to them this turn, allowing
    // (for example) ships to have max fuel meters greater than 0 on the turn
    // they are created.
    m_universe.ApplyMeterEffectsAndUpdateMeters(false);

    TraceLogger(effects) << "!!!!!!! AFTER UPDATING METERS OF ALL OBJECTS";
    TraceLogger(effects) << objects.Dump();

    // Planet depopulation, some in-C++ meter modifications
    for (const auto& obj : objects) {
        obj->PopGrowthProductionResearchPhase();
        obj->ClampMeters();  // ensures no meters are over MAX.  probably redundant with ClampMeters() in Universe::ApplyMeterEffectsAndUpdateMeters()
    }

    TraceLogger(effects) << "!!!!!!!!!!!!!!!!!!!!!!AFTER GROWTH AND CLAMPING";
    TraceLogger(effects) << objects.Dump();

    // store initial values of meters for this turn.
    m_universe.BackPropagateObjectMeters();
    empires.BackPropagateMeters();


    // check for loss of empire capitals
    for (auto& entry : empires) {
        int capital_id = entry.second->CapitalID();
        if (auto capital = GetUniverseObject(capital_id)) {
            if (!capital->OwnedBy(entry.first))
                entry.second->SetCapitalID(INVALID_OBJECT_ID);
        } else {
            entry.second->SetCapitalID(INVALID_OBJECT_ID);
        }
    }


    // store any changes in objects from various progress functions
    // before updating visibility again, so that if the
    // visibility update removes an empires ability to detect an object, the
    // empire will still know the latest state on the
    // turn when the empire did have detection ability for the object
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();

    // post-production and meter-effects visibility update
    m_universe.UpdateEmpireObjectVisibilities();

    m_universe.UpdateEmpireStaleObjectKnowledge();

    // update empire-visibility filtered graphs after visiblity update
    m_universe.UpdateEmpireVisibilityFilteredSystemGraphs();

    TraceLogger(effects) << "!!!!!!!!!!!!!!!!!!!!!!AFTER TURN PROCESSING POP GROWTH PRODCUTION RESEARCH";
    TraceLogger(effects) << objects.Dump();

    // this has to be here for the sitreps it creates to be in the right turn
    CheckForEmpireElimination();

    // update current turn number so that following visibility updates and info
    // sent to players will have updated turn associated with them
    ++m_current_turn;
    DebugLogger() << "ServerApp::PostCombatProcessTurns Turn number incremented to " << m_current_turn;


    // new turn visibility update
    m_universe.UpdateEmpireObjectVisibilities();


    TraceLogger(effects) << "ServerApp::PostCombatProcessTurns Before Final Meter Estimate Update: ";
    TraceLogger(effects) << objects.Dump();

    // redo meter estimates to hopefully be consistent with what happens in clients
    m_universe.UpdateMeterEstimates(false);

    TraceLogger(effects) << "ServerApp::PostCombatProcessTurns After Final Meter Estimate Update: ";
    TraceLogger(effects) << objects.Dump();


    // Re-determine supply distribution and exchanging and resource pools for empires
    UpdateEmpireSupply(true);

    // copy latest visible gamestate to each empire's known object state
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();


    // misc. other updates and records
    m_universe.UpdateStatRecords();
    for (auto& entry : empires)
        entry.second->UpdateOwnedObjectCounters();
    GetSpeciesManager().UpdatePopulationCounter();


    // indicate that the clients are waiting for their new gamestate
    m_networking.SendMessageAll(TurnProgressMessage(Message::DOWNLOADING));


    // compile map of PlayerInfo, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        int player_id = player->PlayerID();
        players[player_id] = PlayerInfo(player->PlayerName(),
                                        PlayerEmpireID(player_id),
                                        player->GetClientType(),
                                        m_networking.PlayerIsHost(player_id));
    }

    m_universe.ObfuscateIDGenerator();

    DebugLogger() << "ServerApp::PostCombatProcessTurns Sending turn updates to players";
    // send new-turn updates to all players
    // exclude those without empire and who are not Observer or Moderator
    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        int empire_id = PlayerEmpireID(player->PlayerID());
        const Empire* empire = GetEmpire(empire_id);
        if (empire ||
            player->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR ||
            player->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
        {
            bool use_binary_serialization = player->IsBinarySerializationUsed();
            player->SendMessage(TurnUpdateMessage(empire_id, m_current_turn,
                                                  m_empires,                          m_universe,
                                                  GetSpeciesManager(),                GetCombatLogManager(),
                                                  GetSupplyManager(),                 players,
                                                  use_binary_serialization));
        }
    }
    DebugLogger() << "ServerApp::PostCombatProcessTurns done";
}

void ServerApp::CheckForEmpireElimination() {
    std::set<Empire*> surviving_empires;
    std::set<Empire*> surviving_human_empires;
    for (auto& entry : Empires()) {
        if (entry.second->Eliminated())
            continue;   // don't double-eliminate an empire
        else if (EmpireEliminated(entry.first))
            entry.second->Eliminate();
        else {
            surviving_empires.insert(entry.second);
            if (GetEmpireClientType(entry.second->EmpireID()) == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                surviving_human_empires.insert(entry.second);
        }
    }

    if (surviving_empires.size() == 1) // last man standing
        (*surviving_empires.begin())->Win(UserStringNop("VICTORY_ALL_ENEMIES_ELIMINATED"));
    else if (!m_single_player_game &&
             static_cast<int>(surviving_human_empires.size()) <= GetGameRules().Get<int>("RULE_THRESHOLD_HUMAN_PLAYER_WIN"))
    {
        // human victory threshold
        if (GetGameRules().Get<bool>("RULE_ONLY_ALLIANCE_WIN")) {
            for (auto emp1_it = surviving_human_empires.begin();
                 emp1_it != surviving_human_empires.end(); ++emp1_it)
            {
                auto emp2_it = emp1_it;
                ++emp2_it;
                for (; emp2_it != surviving_human_empires.end(); ++emp2_it) {
                    if (Empires().GetDiplomaticStatus((*emp1_it)->EmpireID(), (*emp2_it)->EmpireID()) != DIPLO_ALLIED)
                        return;
                }
            }
        }

        for (auto& empire : surviving_human_empires) {
            empire->Win(UserStringNop("VICTORY_FEW_HUMANS_ALIVE"));
        }
    }
}

void ServerApp::HandleDiplomaticStatusChange(int empire1_id, int empire2_id) {
    DiplomaticStatus status = Empires().GetDiplomaticStatus(empire1_id, empire2_id);
    DiplomaticStatusUpdateInfo update(empire1_id, empire2_id, status);

    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        player->SendMessage(DiplomaticStatusMessage(update));
    }
}

void ServerApp::HandleDiplomaticMessageChange(int empire1_id, int empire2_id) {
    const DiplomaticMessage& message = Empires().GetDiplomaticMessage(empire1_id, empire2_id);
    // get players corresponding to empires in message
    int player1_id = EmpirePlayerID(empire1_id);
    int player2_id = EmpirePlayerID(empire2_id);
    if (player1_id == Networking::INVALID_PLAYER_ID || player2_id == Networking::INVALID_PLAYER_ID)
        return;

    auto player1_it = m_networking.GetPlayer(player1_id);
    if (player1_it != m_networking.established_end())
        (*player1_it)->SendMessage(DiplomacyMessage(message));
    auto player2_it = m_networking.GetPlayer(player2_id);
    if (player2_it != m_networking.established_end())
        (*player2_it)->SendMessage(DiplomacyMessage(message));
}
