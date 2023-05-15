#include "ServerApp.h"

#include <ctime>
#include <stdexcept>
#include <thread>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/functional/hash.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "SaveLoad.h"
#include "ServerFSM.h"
#include "UniverseGenerator.h"
#include "../combat/CombatEvents.h"
#include "../combat/CombatLogManager.h"
#include "../combat/CombatSystem.h"
#include "../Empire/Empire.h"
#include "../parse/Parse.h"
#include "../parse/PythonParser.h"
#include "../universe/Building.h"
#include "../universe/Condition.h"
#include "../universe/Fleet.h"
#include "../universe/FleetPlan.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Ship.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Tech.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/LoggerWithOptionsDB.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/OrderSet.h"
#include "../util/Pending.h"
#include "../util/Random.h"
#include "../util/SaveGamePreviewUtils.h"
#include "../util/ScopedTimer.h"
#include "../util/SitRepEntry.h"
#include "../util/ThreadPool.h"
#include "../util/Version.h"

namespace fs = boost::filesystem;

namespace {
    DeclareThreadSafeLogger(effects);
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

void Seed(unsigned int seed);


////////////////////////////////////////////////
// ServerApp
////////////////////////////////////////////////
ServerApp::ServerApp() :
    IApp(),
    m_signals(m_io_context, SIGINT, SIGTERM),
    m_timer(m_io_context),
    m_networking(m_io_context,
                 boost::bind(&ServerApp::HandleNonPlayerMessage, this, boost::placeholders::_1, boost::placeholders::_2),
                 boost::bind(&ServerApp::HandleMessage, this, boost::placeholders::_1, boost::placeholders::_2),
                 boost::bind(&ServerApp::PlayerDisconnected, this, boost::placeholders::_1)),
    m_fsm(new ServerFSM(*this)),
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

    m_galaxy_setup_data.seed =           GetOptionsDB().Get<std::string>("setup.seed");
    m_galaxy_setup_data.size =           GetOptionsDB().Get<int>("setup.star.count");
    m_galaxy_setup_data.shape =          GetOptionsDB().Get<Shape>("setup.galaxy.shape");
    m_galaxy_setup_data.age =            GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.galaxy.age");
    m_galaxy_setup_data.starlane_freq =  GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.starlane.frequency");
    m_galaxy_setup_data.planet_density = GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.planet.density");
    m_galaxy_setup_data.specials_freq =  GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.specials.frequency");
    m_galaxy_setup_data.monster_freq =   GetOptionsDB().Get<GalaxySetupOptionMonsterFreq>("setup.monster.frequency");
    m_galaxy_setup_data.native_freq =    GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.native.frequency");
    m_galaxy_setup_data.ai_aggr =        GetOptionsDB().Get<Aggression>("setup.ai.aggression");
    m_galaxy_setup_data.game_uid =       GetOptionsDB().Get<std::string>("setup.game.uid");

    // Initialize Python before FSM initialization
    // to be able use it for parsing
    InitializePython();
    if (!m_python_server.IsPythonRunning())
        throw std::runtime_error("Python not initialized");

    if (GetOptionsDB().Get<int>("network.server.python.asyncio-interval") > 0) {
        m_timer.expires_after(std::chrono::seconds(GetOptionsDB().Get<int>("network.server.python.asyncio-interval")));
        m_timer.async_wait(boost::bind(&ServerApp::AsyncIOTimedoutHandler,
                                       this,
                                       boost::asio::placeholders::error));
    }

    // Start parsing content before FSM initialization
    // to have data initialized before autostart execution
    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    StartBackgroundParsing(PythonParser(m_python_server, GetResourceDir() / "scripting"), std::move(barrier));
    barrier_future.wait();

    m_fsm->initiate();

    namespace ph = boost::placeholders;

    m_empires.DiplomaticStatusChangedSignal.connect(
        boost::bind(&ServerApp::HandleDiplomaticStatusChange, this, ph::_1, ph::_2));
    m_empires.DiplomaticMessageChangedSignal.connect(
        boost::bind(&ServerApp::HandleDiplomaticMessageChange,this, ph::_1, ph::_2));

    m_signals.async_wait(boost::bind(&ServerApp::SignalHandler, this, ph::_1, ph::_2));
}

ServerApp::~ServerApp() {
    DebugLogger() << "ServerApp::~ServerApp";

    // Calling Py_Finalize here causes segfault when m_python_server destructing with its python
    // object fields

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

void ServerApp::StartBackgroundParsing(const PythonParser& python, std::promise<void>&& barrier) {
    IApp::StartBackgroundParsing(python, std::move(barrier));
    const auto& rdir = GetResourceDir();

    if (fs::exists(rdir / "scripting/starting_unlocks/items.inf"))
        m_universe.SetInitiallyUnlockedItems(Pending::StartAsyncParsing(parse::items, rdir / "scripting/starting_unlocks/items.inf"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/starting_unlocks/items.inf").string();

    if (fs::exists(rdir / "scripting/starting_unlocks/buildings.inf"))
        m_universe.SetInitiallyUnlockedBuildings(Pending::StartAsyncParsing(parse::starting_buildings, rdir / "scripting/starting_unlocks/buildings.inf"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/starting_unlocks/buildings.inf").string();

    if (fs::exists(rdir / "scripting/starting_unlocks/fleets.inf"))
        m_universe.SetInitiallyUnlockedFleetPlans(Pending::StartAsyncParsing(parse::fleet_plans, rdir / "scripting/starting_unlocks/fleets.inf"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/starting_unlocks/fleets.inf").string();

    if (fs::exists(rdir / "scripting/monster_fleets.inf"))
        m_universe.SetMonsterFleetPlans(Pending::StartAsyncParsing(parse::monster_fleet_plans, rdir / "scripting/monster_fleets.inf"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/monster_fleets.inf").string();

    if (fs::exists(rdir / "scripting/empire_statistics"))
        m_universe.SetEmpireStats(Pending::StartAsyncParsing(parse::statistics, rdir / "scripting/empire_statistics"));
    else
        ErrorLogger() << "Background parse path doesn't exist: " << (rdir / "scripting/empire_statistics").string();
}

void ServerApp::CreateAIClients(const std::vector<PlayerSetupData>& player_setup_data, int max_aggression) {
    DebugLogger() << "ServerApp::CreateAIClients: " << player_setup_data.size() << " player (maybe not all AIs) at max aggression: " << max_aggression;
    // check if AI clients are needed for given setup data
    bool need_AIs = false;
    for (const PlayerSetupData& psd : player_setup_data) {
        if (psd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            need_AIs = true;
            break;
        }
    }
    if (need_AIs)
        m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::STARTING_AIS));


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
    std::size_t player_pos = args.size()-1;
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

    if (GetOptionsDB().Get<bool>("testing")) {
        args.push_back("--testing");
#ifdef FREEORION_LINUX
        // Dirty hack to output log to console.
        args.push_back("--log-file");
        args.push_back("/proc/self/fd/1");
#endif
    }

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
    std::string ai_log_dir = GetOptionsDB().Get<std::string>("ai-log-dir");
    if (!ai_log_dir.empty()) {
        args.push_back("--ai-log-dir");
        args.push_back(ai_log_dir);
        DebugLogger() << "ai-log-dir set to '" << ai_log_dir << "'";
    } else {
        DebugLogger() << "ai-log-dir not set.";
    }

    // for each AI client player, create a new AI client process
    for (const PlayerSetupData& psd : player_setup_data) {
        if (psd.client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            continue;

        // check that AIs have a name, as they will be sorted later based on it
        std::string player_name = psd.player_name;
        if (player_name.empty()) {
            ErrorLogger() << "ServerApp::CreateAIClients can't create a player with no name.";
            return;
        }

        args[player_pos] = player_name;
        m_ai_client_processes.insert_or_assign(player_name, Process(AI_CLIENT_EXE, args));

        DebugLogger() << "done starting AI " << player_name;
    }

    // set initial AI process priority to low
    SetAIsProcessPriorityToLow(true);
}

Empire* ServerApp::GetEmpire(int id)
{ return m_empires.GetEmpire(id).get(); }

std::string ServerApp::GetVisibleObjectName(const UniverseObject& object)
{ return object.Name(); }

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

void ServerApp::AsyncIOTimedoutHandler(const boost::system::error_code& error) {
    if (error) {
        DebugLogger() << "Turn timed out cancelled";
        return;
    }

    bool success = false;
    try {
        success = m_python_server.AsyncIOTick();
    } catch (const boost::python::error_already_set&) {
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

    if (success) {
        if (GetOptionsDB().Get<int>("network.server.python.asyncio-interval") > 0) {
            m_timer.expires_after(std::chrono::seconds(GetOptionsDB().Get<int>("network.server.python.asyncio-interval")));
            m_timer.async_wait(boost::bind(&ServerApp::AsyncIOTimedoutHandler,
                                             this,
                                             boost::asio::placeholders::error));
        }
    } else {
        ErrorLogger() << "Python scripted authentication failed.";
        ServerApp::GetApp()->Networking().SendMessageAll(ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"),
                                                                      false));
    }
}

void ServerApp::UpdateEmpireTurnReceived(int empire_id, int turn, bool success) {
    if (success) {
        if (auto empire = m_empires.GetEmpire(empire_id)) {
            empire->SetLastTurnReceived(turn);
        }
    }
}

void ServerApp::CleanupAIs() {
    if (m_ai_client_processes.empty() && m_networking.empty())
        return;

    DebugLogger() << "ServerApp::CleanupAIs() telling AIs game is ending";

    bool ai_connection_lingering = false;
    try {
        for (PlayerConnectionPtr player : m_networking) {
            if (player->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
                player->SendMessage(EndGameMessage(Message::EndGameReason::PLAYER_DISCONNECT));
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
        for (auto& process : m_ai_client_processes)
        { process.second.Kill(); }
    } catch (...) {
        ErrorLogger() << "ServerApp::CleanupAIs() exception while killing processes";
    }

    m_ai_client_processes.clear();
}

void ServerApp::SetAIsProcessPriorityToLow(bool set_to_low) {
    for (auto& process : m_ai_client_processes) {
        if(!(process.second.SetLowPriority(set_to_low))) {
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

    //DebugLogger() << "ServerApp::HandleMessage type " << msg.Type();
    m_networking.UpdateCookie(player_connection->Cookie()); // update cookie expire date

    switch (msg.Type()) {
    case Message::MessageType::HOST_SP_GAME:             m_fsm->process_event(HostSPGame(msg, player_connection));       break;
    case Message::MessageType::START_MP_GAME:            m_fsm->process_event(StartMPGame(msg, player_connection));      break;
    case Message::MessageType::LOBBY_UPDATE:             m_fsm->process_event(LobbyUpdate(msg, player_connection));      break;
    case Message::MessageType::SAVE_GAME_INITIATE:       m_fsm->process_event(SaveGameRequest(msg, player_connection));  break;
    case Message::MessageType::TURN_ORDERS:              m_fsm->process_event(TurnOrders(msg, player_connection));       break;
    case Message::MessageType::TURN_PARTIAL_ORDERS:      m_fsm->process_event(TurnPartialOrders(msg, player_connection));break;
    case Message::MessageType::UNREADY:                  m_fsm->process_event(RevokeReadiness(msg, player_connection));  break;
    case Message::MessageType::PLAYER_CHAT:              m_fsm->process_event(PlayerChat(msg, player_connection));       break;
    case Message::MessageType::DIPLOMACY:                m_fsm->process_event(Diplomacy(msg, player_connection));        break;
    case Message::MessageType::MODERATOR_ACTION:         m_fsm->process_event(ModeratorAct(msg, player_connection));     break;
    case Message::MessageType::ELIMINATE_SELF:           m_fsm->process_event(EliminateSelf(msg, player_connection));    break;
    case Message::MessageType::AUTO_TURN:                m_fsm->process_event(AutoTurn(msg, player_connection));         break;
    case Message::MessageType::REVERT_ORDERS:            m_fsm->process_event(RevertOrders(msg, player_connection));     break;

    case Message::MessageType::ERROR_MSG:
    case Message::MessageType::DEBUG:                    break;

    case Message::MessageType::SHUT_DOWN_SERVER:         HandleShutdownMessage(msg, player_connection);  break;
    case Message::MessageType::AI_END_GAME_ACK:          m_fsm->process_event(LeaveGame(msg, player_connection));        break;

    case Message::MessageType::REQUEST_SAVE_PREVIEWS:    UpdateSavePreviews(msg, player_connection); break;
    case Message::MessageType::REQUEST_COMBAT_LOGS:      m_fsm->process_event(RequestCombatLogs(msg, player_connection));break;
    case Message::MessageType::LOGGER_CONFIG:            HandleLoggerConfig(msg, player_connection); break;

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
        if ((*players_it)->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            DebugLogger() << "Forwarding logging thresholds to AI " << (*players_it)->PlayerID();
            (*players_it)->SendMessage(relay_options_message);
        }
    }
}

void ServerApp::HandleNonPlayerMessage(const Message& msg, PlayerConnectionPtr player_connection) {
    switch (msg.Type()) {
    case Message::MessageType::HOST_SP_GAME:  m_fsm->process_event(HostSPGame(msg, player_connection));   break;
    case Message::MessageType::HOST_MP_GAME:  m_fsm->process_event(HostMPGame(msg, player_connection));   break;
    case Message::MessageType::JOIN_GAME:     m_fsm->process_event(JoinGame(msg, player_connection));     break;
    case Message::MessageType::AUTH_RESPONSE: m_fsm->process_event(AuthResponse(msg, player_connection)); break;
    case Message::MessageType::ERROR_MSG:     m_fsm->process_event(Error(msg, player_connection));        break;
    case Message::MessageType::DEBUG:         break;
    default:
        if ((m_networking.size() == 1) &&
            (player_connection->IsLocalConnection()) &&
            (msg.Type() == Message::MessageType::SHUT_DOWN_SERVER))
        {
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
        if (player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER ||
            player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
            player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
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
    const auto& player_setup_data = single_player_setup_data.players;
    NewGameInitConcurrentWithJoiners(single_player_setup_data, player_setup_data);
}

bool ServerApp::VerifySPGameAIs(const SinglePlayerSetupData& single_player_setup_data) {
    const auto& player_setup_data = single_player_setup_data.players;
    return NewGameInitVerifyJoiners(player_setup_data);
}

void ServerApp::NewMPGameInit(const MultiplayerLobbyData& multiplayer_lobby_data) {
    // associate player IDs with player setup data by matching player IDs when
    // available (human) and names (for AI clients which didn't have an ID
    // before now because the lobby data was set up without connected/established
    // clients for the AIs.
    const auto& player_setup_data = multiplayer_lobby_data.players;
    std::vector<PlayerSetupData> psds;

    for (const auto& entry : player_setup_data) {
        const PlayerSetupData& psd = entry.second;
        if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER ||
            psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
            psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
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
                    PlayerSetupData new_psd = psd;
                    new_psd.player_id = player_id;
                    psds.push_back(std::move(new_psd));
                    found_matched_id_connection = true;
                    break;
                }
            }
            if (!found_matched_id_connection) {
                if (player_id != Networking::INVALID_PLAYER_ID) {
                    ErrorLogger() << "ServerApp::NewMPGameInit couldn't find player setup data for human player with id: " << player_id << " player name: " << psd.player_name;
                } else {
                    // There is no player currently connected for the current setup data. A player
                    // may connect later, at which time they may be assigned to this data or the
                    // corresponding empire.
                    psds.push_back(psd);
                }
            }

        } else if (psd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            // All AI player setup data, as determined from their client type, is
            // assigned to player IDs of established AI players with the appropriate names

            // find player connection with same name as this player setup data
            bool found_matched_name_connection = false;
            const std::string& player_name = psd.player_name;
            for (auto established_player_it = m_networking.established_begin();
                 established_player_it != m_networking.established_end(); ++established_player_it)
            {
                const PlayerConnectionPtr player_connection = *established_player_it;
                if (player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER &&
                    player_connection->PlayerName() == player_name)
                {
                    // assign name-matched AI client's player setup data to appropriate AI connection
                    int player_id = player_connection->PlayerID();
                    PlayerSetupData new_psd = psd;
                    new_psd.player_id = player_id;
                    psds.push_back(std::move(new_psd));
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

    NewGameInitConcurrentWithJoiners(multiplayer_lobby_data, psds);
    if (NewGameInitVerifyJoiners(psds))
        SendNewGameStartMessages();
}

namespace {
    void UpdateEmpireSupply(ScriptingContext& context, SupplyManager& supply, bool precombat) {
        // Determine initial supply distribution and exchanging and resource pools for empires
        for ([[maybe_unused]] auto& [ignored_id, empire] : context.Empires()) {
            (void)ignored_id; // quiet unused variable warning
            if (empire->Eliminated())
                continue;   // skip eliminated empires.  presumably this shouldn't be an issue when initializing a new game, but apparently I thought this was worth checking for...

            // determine which systems can propagate fleet and resource (same for both)
            empire->UpdateSupplyUnobstructedSystems(context, precombat); // TODO: pass empire ID to use for known objects lookup?
            // set range systems can propagate fleet and resourse supply (separately)
            empire->UpdateSystemSupplyRanges(context.ContextUniverse());
        }

        supply.Update(context); // must call after updating supply ranges for all empires

        const unsigned int num_threads = static_cast<unsigned int>(std::max(1, EffectsProcessingThreads()));
        boost::asio::thread_pool thread_pool(num_threads);

        for ([[maybe_unused]] auto& [ignored_id, empire] : context.Empires()) {
            (void)ignored_id; // quiet unused variable warning
            if (empire->Eliminated())
                continue;
            boost::asio::post(thread_pool, [&context, empire{empire}]() {
                // determine population centers and resource centers of empire, tells resource pools
                // the centers and groups of systems that can share resources (note that being able to
                // share resources doesn't mean a system produces resources)
                empire->InitResourcePools(context.ContextObjects(), context.supply);

                // determine how much of each resources is available in each resource sharing group
                empire->UpdateResourcePools(context);
            });
        }

        thread_pool.join();
    }
}

void ServerApp::NewGameInitConcurrentWithJoiners(
    const GalaxySetupData& galaxy_setup_data,
    const std::vector<PlayerSetupData>& player_setup_data_in)
{
    DebugLogger() << "ServerApp::NewGameInitConcurrentWithJoiners";

    m_galaxy_setup_data = galaxy_setup_data;

    // set game rules for server based on those specified in setup data
    GetGameRules().SetFromStrings(m_galaxy_setup_data.GetGameRules());

    // validate some connection info / determine which players need empires created
    std::map<int, PlayerSetupData> active_empire_id_setup_data;
    int next_empire_id = 1;
    for (const auto& psd : player_setup_data_in) {
        if (!psd.player_name.empty()
            && (psd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER
                || psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER))
        {
            active_empire_id_setup_data[next_empire_id++] = psd;
        }
    }

    if (active_empire_id_setup_data.empty()) {
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
    m_turn_expired = false;

    // create universe and empires for players
    DebugLogger() << "ServerApp::NewGameInitConcurrentWithJoiners: Creating Universe";
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::GENERATING_UNIVERSE));


    // m_current_turn set above so that every UniverseObject created before game
    // starts will have m_created_on_turn BEFORE_FIRST_TURN
    GenerateUniverse(active_empire_id_setup_data);


    // after all game initialization stuff has been created, set current turn to 0 and apply only GenerateSitRep Effects
    // so that a set of SitReps intended as the player's initial greeting will be segregated
    m_current_turn = 0;
    ScriptingContext context{m_universe, m_empires, m_galaxy_setup_data,
                             m_species_manager, m_supply_manager};
    m_universe.ApplyGenerateSitRepEffects(context);

    //can set current turn to 1 for start of game
    m_current_turn = 1;

    // record empires for each active player. Note: active_empire_id_setup_data
    // contains only data of players who control an empire; observers and
    // moderators are not included.
    for (auto& [empire_id, psd] : active_empire_id_setup_data) {
        if (psd.player_id != Networking::INVALID_PLAYER_ID)
            m_player_empire_ids[psd.player_id] = empire_id;

        // add empires to turn processing
        if (auto empire = m_empires.GetEmpire(empire_id)) {
            AddEmpireTurn(empire_id, PlayerSaveGameData(psd.player_name, empire_id,
                                                        nullptr, nullptr, std::string(),
                                                        psd.client_type));
            empire->SetReady(false);
        }
    }

    // update visibility information to ensure data sent out is up-to-date
    DebugLogger() << "ServerApp::NewGameInitConcurrentWithJoiners: Updating first-turn Empire stuff";
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns(context.current_turn);

    // initialize empire owned object counters
    for (auto& entry : m_empires)
        entry.second->UpdateOwnedObjectCounters(m_universe);

    UpdateEmpireSupply(context, m_supply_manager, false);
    m_universe.UpdateStatRecords(context);
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
        if (psd.client_type == Networking::ClientType::INVALID_CLIENT_TYPE) {
            ErrorLogger() << "Player with id " << psd.player_id << " has invalid client type";
            continue;
        }

        player_id_setup_data[psd.player_id] = psd;

        if (m_networking.HostPlayerID() == psd.player_id)
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
    if (m_networking.NumEstablishedPlayers() != player_id_setup_data.size() && !IsHostless()) {
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
        if (psd.client_type != client_type) {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners found inconsistent client type between player connection (" << client_type << ") and player setup data (" << psd.client_type << ")";
            return false;
        }
        if (psd.player_name != player_connection->PlayerName()) {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners found inconsistent player names: " << psd.player_name << " and " << player_connection->PlayerName();
            return false;
        }
        if (player_connection->PlayerName().empty()) {
            ErrorLogger() << "ServerApp::NewGameInitVerifyJoiners found player connection with empty name!";
            return false;
        }

        if (!(client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER
              || client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER
              || client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER
              || client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR))
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
    for (auto player_connection_it = m_networking.established_begin();  // can't easily use range for loop due to non-standard begin and end
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        int empire_id = PlayerEmpireID(player_id);
        bool use_binary_serialization = player_connection->IsBinarySerializationUsed();
        player_connection->SendMessage(GameStartMessage(m_single_player_game,    empire_id,
                                                        m_current_turn,          m_empires,
                                                        m_universe,              m_species_manager,
                                                        GetCombatLogManager(),   m_supply_manager,
                                                        player_info_map,         m_galaxy_setup_data,
                                                        use_binary_serialization,!player_connection->IsLocalConnection()));
    }
}

void ServerApp::LoadSPGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                               std::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    // Need to determine which data in player_save_game_data should be assigned to which established player
    std::vector<std::pair<int, int>> player_id_to_save_game_data_index;


    // assign all saved game data to a player ID
    for (int i = 0; i < static_cast<int>(player_save_game_data.size()); ++i) {
        const PlayerSaveGameData& psgd = player_save_game_data[i];
        if (psgd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
            // In a single player game, the host player is always the human player, so
            // this is just a matter of finding which entry in player_save_game_data was
            // a human player, and assigning that saved player data to the host player ID
            player_id_to_save_game_data_index.emplace_back(m_networking.HostPlayerID(), i);

        } else if (psgd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            // All saved AI player data, as determined from their client type, is
            // assigned to player IDs of established AI players

            // cycle to find next established AI player
            for (auto established_it = m_networking.established_begin(); established_it != m_networking.established_end(); ++established_it)
            {
                const PlayerConnectionPtr player_connection = *established_it;
                if (player_connection->GetClientType() != Networking::ClientType::CLIENT_TYPE_AI_PLAYER
                    || player_connection->PlayerName() != psgd.name)
                    continue;

                int player_id = player_connection->PlayerID();
                player_id_to_save_game_data_index.emplace_back(player_id, i);
                break;
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
            TraceLogger() << "Added relative path " << rel_path << " in " << subdir
                          << " to save preview directories";
            list.push_back(std::move(rel_path));
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
    logs.reserve(ids.size());
    for (auto it = ids.begin(); it != ids.end(); ++it) {
        auto log = GetCombatLogManager().GetLog(*it);
        if (!log) {
            ErrorLogger() << "UpdateCombatLogs can't fetch log with id = "<< *it << " ... skipping.";
            continue;
        }
        logs.emplace_back(*it, *log);
    }

    // Return them to the client
    DebugLogger() << "UpdateCombatLogs returning " << logs.size()
                  << " logs to player " << player_connection->PlayerID();

    try {
        bool use_binary_serialization = player_connection->IsBinarySerializationUsed();
        player_connection->SendMessage(DispatchCombatLogsMessage(logs, use_binary_serialization,
                                                                 !player_connection->IsLocalConnection()));
    } catch (const std::exception& e) {
        ErrorLogger() << "caught exception sending combat logs message: " << e.what();
        std::vector<std::pair<int, const CombatLog>> empty_logs;
        player_connection->SendMessage(DispatchCombatLogsMessage(empty_logs, false,
                                                                 !player_connection->IsLocalConnection()));
    }
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
    } catch (const boost::python::error_already_set&) {
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
                                std::array<uint8_t, 4> text_color,
                                const boost::posix_time::ptime& timestamp)
{
    ChatHistoryEntity chat{player_name, text, timestamp, text_color};
    m_chat_history.push_back(chat);

    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonChatDir());
        // Call the Python load_history function
        success = m_python_server.PutChatHistoryEntity(chat);
    } catch (const boost::python::error_already_set&) {
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
        ServerApp::GetApp()->Networking().SendMessageAll(
            ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"), false));
    }
}

void ServerApp::ExpireTurn() {
    InfoLogger() << "Turn was set to expired";
    m_turn_expired = true;
}

bool ServerApp::IsTurnExpired() const
{ return m_turn_expired; }

bool ServerApp::IsHaveWinner() const {
    for (const auto& empire : m_empires) {
        if (empire.second->Won())
            return true;
    }
    return false;
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
        if (player_connection->GetClientType() != Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
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
            if (psgd.empire_id == empire_id)
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
        if (psd.save_game_empire_id == ALL_EMPIRES) {
            ErrorLogger() << "ServerApp::LoadMPGameInit got player setup data for human player "
                                    << "with no empire assigned...";
            return;
        }

        // safety check: id-matched player is connected
        bool consistent_human_player_connected = HumanPlayerWithIdConnected(sn, setup_data_player_id);
        if (!consistent_human_player_connected)
            return;   // error message logged in HumanPlayerWithIdConnected

        // determine and store save game data index for this player
        int index = VectorIndexForPlayerSaveGameDataForEmpireID(player_save_game_data, psd.save_game_empire_id);
        if (index != -1) {
            player_id_to_save_game_data_index.push_back({setup_data_player_id, index});
        } else {
            ErrorLogger() << "ServerApp::LoadMPGameInit couldn't find save game data for "
                                   << "human player with assigned empire id: " << psd.save_game_empire_id;
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
                player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
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
        if (psd.save_game_empire_id == ALL_EMPIRES) {
            ErrorLogger() << "ServerApp::LoadMPGameInit got player setup data for AI player "
                          << "with no empire assigned...";
            return;
        }

        // get ID of name-matched AI player
        int player_id = AIPlayerIDWithName(sn, psd.player_name);
        if (player_id == Networking::INVALID_PLAYER_ID) {
            ErrorLogger() << "ServerApp::LoadMPGameInit couldn't find expected AI player with name " << psd.player_name;
            return;
        }

        DebugLogger() << "ServerApp::LoadMPGameInit matched player named " << psd.player_name
                      << " to setup data player id " << player_id
                      << " with setup data empire id " << psd.save_game_empire_id;

        // determine and store save game data index for this player
        int index = VectorIndexForPlayerSaveGameDataForEmpireID(player_save_game_data, psd.save_game_empire_id);
        if (index != -1) {
            player_id_to_save_game_data_index.push_back({player_id, index});
        } else {
            ErrorLogger() << "ServerApp::LoadMPGameInit couldn't find save game data for "
                          << "human player with assigned empire id: " << psd.save_game_empire_id;
        }
    }
}

void ServerApp::LoadMPGameInit(const MultiplayerLobbyData& lobby_data,
                               const std::vector<PlayerSaveGameData>& player_save_game_data,
                               std::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    // Need to determine which data in player_save_game_data should be assigned to which established player
    std::vector<std::pair<int, int>> player_id_to_save_game_data_index;

    const auto& player_setup_data = lobby_data.players;

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

        if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
            int setup_data_player_id = entry.first;
            GetSaveGameDataIndexForHumanPlayer(player_id_to_save_game_data_index, psd,
                                               setup_data_player_id, player_save_game_data,
                                               m_networking);

        } else if (psd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
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
        if (client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER &&
            client_type != Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
        {
            ErrorLogger() << "ServerApp::LoadGameInit found player connection with unsupported client type.";
        }
        if (player_connection->PlayerName().empty())
            ErrorLogger() << "ServerApp::LoadGameInit found player connection with empty name!";
    }


    // clear previous game player state info
    m_turn_sequence.clear();
    m_player_empire_ids.clear();


    // restore server state info from save
    m_current_turn = server_save_game_data->current_turn;

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
            empire_id = psgd.empire_id;               // can't use GetPlayerEmpireID here because m_player_empire_ids hasn't been set up yet.
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
        if (auto empire = m_empires.GetEmpire(empire_id))
            empire->SetAuthenticated(player_connection->IsAuthenticated());
    }

    for (const auto& psgd : player_save_game_data) {
        int empire_id = psgd.empire_id;
        // add empires to turn processing, and restore saved orders and UI data or save state data
        if (auto empire = m_empires.GetEmpire(empire_id)) {
            if (!empire->Eliminated())
                AddEmpireTurn(empire_id, psgd);
        } else {
            ErrorLogger() << "ServerApp::LoadGameInit couldn't find empire with id " << empire_id << " to add to turn processing";
        }
    }


    // the Universe's system graphs for each empire aren't stored when saving
    // so need to be reinitialized when loading based on the gamestate
    m_universe.InitializeSystemGraph(m_empires, m_universe.Objects());
    m_universe.UpdateEmpireVisibilityFilteredSystemGraphsWithOwnObjectMaps(m_empires);

    ScriptingContext context{m_universe, m_empires, m_galaxy_setup_data, m_species_manager,m_supply_manager};
    UpdateEmpireSupply(context, m_supply_manager, true);  // precombat supply update

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
        if (!psgd.orders)
            psgd.orders.reset(new OrderSet());    // need an empty order set pointed to for serialization in case no data is loaded but the game start message wants orders to send

        // get empire ID for player. safety check on it.
        int empire_id = PlayerEmpireID(player_id);
        if (empire_id != psgd.empire_id)
            ErrorLogger() << "LoadGameInit got inconsistent empire ids between player save game data and result of PlayerEmpireID";

        // Revoke readiness only for online players so they can redo orders for the current turn.
        // Without doing it, server would immediatly advance the turn because saves are made when
        // all players sent orders and became ready.
        RevokeEmpireTurnReadyness(empire_id);

        // restore saved orders.  these will be re-executed on client and
        // re-sent to the server (after possibly modification) by clients
        // when they end their turn
        auto orders = psgd.orders;

        bool use_binary_serialization = player_connection->IsBinarySerializationUsed();

        if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            // get save state string
            const std::string* sss = nullptr;
            if (!psgd.save_state_string.empty())
                sss = &psgd.save_state_string;

            player_connection->SendMessage(GameStartMessage(m_single_player_game, empire_id,
                                                            m_current_turn, m_empires, m_universe,
                                                            m_species_manager, GetCombatLogManager(),
                                                            m_supply_manager, player_info_map, *orders, sss,
                                                            m_galaxy_setup_data, use_binary_serialization,
                                                            !player_connection->IsLocalConnection()));

        } else if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
            player_connection->SendMessage(GameStartMessage(m_single_player_game, empire_id,
                                                            m_current_turn, m_empires, m_universe,
                                                            m_species_manager, GetCombatLogManager(),
                                                            m_supply_manager, player_info_map, *orders,
                                                            psgd.ui_data.get(), m_galaxy_setup_data,
                                                            use_binary_serialization,
                                                            !player_connection->IsLocalConnection()));

        } else if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
                   client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
        {

            player_connection->SendMessage(GameStartMessage(m_single_player_game, ALL_EMPIRES,
                                                            m_current_turn, m_empires, m_universe,
                                                            m_species_manager, GetCombatLogManager(),
                                                            m_supply_manager, player_info_map,
                                                            m_galaxy_setup_data, use_binary_serialization,
                                                            !player_connection->IsLocalConnection()));
        } else {
            ErrorLogger() << "ServerApp::CommonGameInit unsupported client type: skipping game start message.";
        }
    }
}

void ServerApp::GenerateUniverse(std::map<int, PlayerSetupData>& player_setup_data) {
    // Set game UID. Needs to be done first so we can use ClockSeed to
    // prevent reproducible UIDs.
    ClockSeed();
    if (GetOptionsDB().Get<std::string>("setup.game.uid").empty())
        m_galaxy_setup_data.SetGameUID(boost::uuids::to_string(boost::uuids::random_generator()()));

    // Initialize RNG with provided seed to get reproducible universes
    int seed = 0;
    try {
        seed = boost::lexical_cast<unsigned int>(m_galaxy_setup_data.seed);
    } catch (...) {
        try {
            boost::hash<std::string> string_hash;
            std::size_t h = string_hash(m_galaxy_setup_data.seed);
            seed = static_cast<unsigned int>(h);
        } catch (...) {}
    }
    if (m_galaxy_setup_data.GetSeed().empty() || m_galaxy_setup_data.GetSeed() == "RANDOM") {
        //ClockSeed();
        // replicate ClockSeed code here so can log the seed used
        boost::posix_time::ptime ltime = boost::posix_time::microsec_clock::local_time();
        std::string new_seed = boost::posix_time::to_simple_string(ltime);
        boost::hash<std::string> string_hash;
        std::size_t h = string_hash(new_seed);
        DebugLogger() << "GenerateUniverse using clock for seed:" << new_seed;
        seed = static_cast<unsigned int>(h);
        // store seed in galaxy setup data
        m_galaxy_setup_data.SetSeed(std::to_string(seed));
    }
    Seed(seed);
    DebugLogger() << "GenerateUniverse with seed: " << seed;

    // Reset the universe object for a new universe
    m_universe.Clear();
    m_species_manager.ClearSpeciesHomeworlds();

    // Reset the object id manager for the new empires.
    std::vector<int> empire_ids(player_setup_data.size());
    std::transform(player_setup_data.begin(), player_setup_data.end(), empire_ids.begin(),
                   [](const auto& ii) { return ii.first; });
    m_universe.ResetAllIDAllocation(empire_ids);

    // Add predefined ship designs to universe
    GetPredefinedShipDesignManager().AddShipDesignsToUniverse(m_universe);
    // Initialize empire objects for each player
    InitEmpires(player_setup_data, m_empires);

    bool success{false};
    try {
        // Set Python current work directory to directory containing
        // the universe generation Python scripts
        m_python_server.SetCurrentDir(GetPythonUniverseGeneratorDir());
        // Call the main Python universe generator function
        success = m_python_server.CreateUniverse(player_setup_data);
    } catch (const boost::python::error_already_set&) {
        success = false;
        m_python_server.HandleErrorAlreadySet();
        if (!m_python_server.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Exiting.";
            m_fsm->process_event(ShutdownServer());
        }
    }

    if (!success)
        ServerApp::GetApp()->Networking().SendMessageAll(
            ErrorMessage(UserStringNop("SERVER_UNIVERSE_GENERATION_ERRORS"), false));

    for (auto& empire : m_empires) {
        empire.second->ApplyNewTechs(m_universe, m_current_turn);
        empire.second->ApplyPolicies(m_universe, m_current_turn);
    }

    DebugLogger() << "Applying first turn effects and updating meters";

    ScriptingContext context{m_universe, m_empires, m_galaxy_setup_data, m_species_manager, m_supply_manager};

    // Apply effects for 1st turn.
    m_universe.ApplyAllEffectsAndUpdateMeters(context, false);

    TraceLogger(effects) << "After First turn meter effect applying: " << m_universe.Objects().Dump();
    // Set active meters to targets or maxes after first meter effects application
    m_universe.BackPropagateObjectMeters();
    SetActiveMetersToTargetMaxCurrentValues(m_universe.Objects());
    m_universe.UpdateMeterEstimates(context);
    m_universe.BackPropagateObjectMeters();
    SetActiveMetersToTargetMaxCurrentValues(m_universe.Objects());
    m_universe.BackPropagateObjectMeters();

    TraceLogger(effects) << "After First active set to target/max: " << m_universe.Objects().Dump();

    m_universe.BackPropagateObjectMeters();
    m_empires.BackPropagateMeters();

    DebugLogger() << "Re-applying first turn meter effects and updating meters";

    // Re-apply meter effects, so that results depending on meter values can be
    // re-checked after initial setting of those meter values
    m_universe.ApplyMeterEffectsAndUpdateMeters(context, false);
    // Re-set active meters to targets after re-application of effects
    SetActiveMetersToTargetMaxCurrentValues(m_universe.Objects());
    // Set the population of unowned planets to a random fraction of their target values.
    SetNativePopulationValues(m_universe.Objects());

    m_universe.BackPropagateObjectMeters();
    m_empires.BackPropagateMeters();

    TraceLogger() << "!!!!!!!!!!!!!!!!!!! After setting active meters to targets";
    TraceLogger() << m_universe.Objects().Dump();

    m_universe.UpdateEmpireObjectVisibilities(m_empires);
}

void ServerApp::ExecuteScriptedTurnEvents() {
    bool success(false);
    try {
        m_python_server.SetCurrentDir(GetPythonTurnEventsDir());
        // Call the main Python turn events function
        success = m_python_server.ExecuteTurnEvents();
    } catch (const boost::python::error_already_set&) {
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
        if (client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER && client_type != Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
            ErrorLogger() << "ServerApp::GetPlayerInfoMap found player connection with unsupported client type.";
        }
        if (player_connection->PlayerName().empty()) {
            ErrorLogger() << "ServerApp::GetPlayerInfoMap found player connection with empty name!";
        }

        // assemble player info for all players
        player_info_map[player_id] = PlayerInfo{player_connection->PlayerName(),
                                                empire_id,
                                                player_connection->GetClientType(),
                                                m_networking.PlayerIsHost(player_connection->PlayerID())};
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
    return ((player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) &&
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

bool ServerApp::IsAuthRequiredOrFillRoles(const std::string& player_name, const std::string& ip_address, Networking::AuthRoles& roles) {
    bool result = false;
    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonAuthDir());
        // Call the main Python turn events function
        success = m_python_server.IsRequireAuthOrReturnRoles(player_name, ip_address, result, roles);
        if (GetOptionsDB().Get<bool>("network.server.allow-observers")) {
            roles.SetRole(Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER, true);
        }
    } catch (const boost::python::error_already_set&) {
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

bool ServerApp::IsAuthSuccessAndFillRoles(const std::string& player_name,
                                          const std::string& auth, Networking::AuthRoles& roles)
{
    bool result = false;
    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonAuthDir());
        // Call the main Python turn events function
        success = m_python_server.IsSuccessAuthAndReturnRoles(player_name, auth, result, roles);
        if (GetOptionsDB().Get<bool>("network.server.allow-observers")) {
            roles.SetRole(Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER, true);
        }
    } catch (const boost::python::error_already_set&) {
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

std::vector<PlayerSetupData> ServerApp::FillListPlayers() {
    std::vector<PlayerSetupData> result;
    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonAuthDir());
        success = m_python_server.FillListPlayers(result);
    } catch (const boost::python::error_already_set&) {
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
        ErrorLogger() << "Python scripted player list failed.";
        ServerApp::GetApp()->Networking().SendMessageAll(
            ErrorMessage(UserStringNop("SERVER_TURN_EVENTS_ERRORS"), false));
    }
    return result;
}

void ServerApp::AddObserverPlayerIntoGame(const PlayerConnectionPtr& player_connection) {
    std::map<int, PlayerInfo> player_info_map = GetPlayerInfoMap();

    Networking::ClientType client_type = player_connection->GetClientType();
    bool use_binary_serialization = player_connection->IsBinarySerializationUsed();

    if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
        client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
    {
        // simply sends GAME_START message so established player will known he is in the game now
        player_connection->SendMessage(GameStartMessage(m_single_player_game, ALL_EMPIRES,
                                                        m_current_turn, m_empires, m_universe,
                                                        m_species_manager, GetCombatLogManager(),
                                                        m_supply_manager, player_info_map,
                                                        m_galaxy_setup_data, use_binary_serialization,
                                                        !player_connection->IsLocalConnection()));
    } else {
        ErrorLogger() << "ServerApp::CommonGameInit unsupported client type: skipping game start message.";
    }
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

    // test if there other human or disconnected players in the game
    bool other_human_player = false;
    for (auto& empires : m_empires) {
        if (!empires.second->Eliminated() &&
            empire_id != empires.second->EmpireID())
        {
            Networking::ClientType other_client_type = GetEmpireClientType(empires.second->EmpireID());
            if (other_client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER ||
                other_client_type == Networking::ClientType::INVALID_CLIENT_TYPE)
            {
                other_human_player = true;
                break;
            }
        }
    }
    if (!other_human_player) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CONCEDE_LAST_HUMAN_PLAYER"), false));
        return false;
    }

    auto empire = m_empires.GetEmpire(empire_id);
    if (!empire) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_NONPLAYER_CANNOT_CONCEDE"), false));
        return false;
    }

    auto is_owned = [empire_id](const UniverseObject* obj) { return obj->OwnedBy(empire_id); };

    // test for colonies count
    auto planets = m_universe.Objects().findRaw <Planet>(is_owned);
    if (planets.size() > static_cast<std::size_t>(GetGameRules().Get<int>("RULE_CONCEDE_COLONIES_THRESHOLD"))) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CONCEDE_EXCEED_COLONIES"), false));
        return false;
    }

    // empire elimination
    empire->Eliminate(m_empires, m_current_turn);
    const auto empire_ids{m_empires.EmpireIDs()};

    // destroy owned ships
    for (auto* obj : m_universe.Objects().findRaw<Ship>(is_owned)) {
        obj->SetOwner(ALL_EMPIRES);
        m_universe.RecursiveDestroy(obj->ID(), empire_ids);
    }
    // destroy owned buildings
    for (auto* obj : m_universe.Objects().findRaw<Building>(is_owned)) {
        obj->SetOwner(ALL_EMPIRES);
        m_universe.RecursiveDestroy(obj->ID(), empire_ids);
    }
    // unclaim owned planets
    for (auto* planet : planets)
        planet->Reset(m_universe.Objects());

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
        player_ctn->SendMessage(PlayerStatusMessage(Message::PlayerStatus::WAITING, empire_id));
    }

    return true;
}

void ServerApp::DropPlayerEmpireLink(int player_id)
{ m_player_empire_ids.erase(player_id); }

int ServerApp::AddPlayerIntoGame(const PlayerConnectionPtr& player_connection, int target_empire_id) {
    std::shared_ptr<Empire> empire;
    int empire_id = ALL_EMPIRES;
    auto delegation = GetPlayerDelegation(player_connection->PlayerName());
    if (GetOptionsDB().Get<bool>("network.server.take-over-ai")) {
        for (auto& e : m_empires) {
            if (!e.second->Eliminated() &&
                GetEmpireClientType(e.first) == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            {
                delegation.push_back(e.second->PlayerName());
            }
        }
    }
    if (target_empire_id == ALL_EMPIRES) {
        // search empire by player name
        for (auto& [loop_empire_id, loop_empire] : m_empires) {
            if (loop_empire->PlayerName() == player_connection->PlayerName()) {
                empire_id = loop_empire_id;
                empire = loop_empire;
                break;
            }
        }
        // Assign player to empire if he doesn't have own empire and delegates signle
        if (delegation.size() == 1 && empire == nullptr) {
            for (auto& [loop_empire_id, loop_empire] : m_empires) {
                if (loop_empire->PlayerName() == delegation.front()) {
                    empire_id = loop_empire_id;
                    empire = loop_empire;
                    break;
                }
            }
        }
        if (!delegation.empty()) {
            DebugLogger() << "ServerApp::AddPlayerIntoGame(...): Player should choose between delegates.";
            return ALL_EMPIRES;
        }
    } else {
        // use provided empire and test if it's player himself or one of delegated
        empire_id = target_empire_id;
        empire = m_empires.GetEmpire(target_empire_id);
        if (!empire)
            return ALL_EMPIRES;

        if (empire->PlayerName() != player_connection->PlayerName()) {
            bool matched = false;
            for (const auto& delegated : delegation) {
                if (empire->PlayerName() == delegated) {
                    matched = true;
                    break;
                }
            }
            if (!matched)
                return ALL_EMPIRES;
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

    int previous_player_id = EmpirePlayerID(empire_id);

    // make a link to new connection
    m_player_empire_ids[player_connection->PlayerID()] = empire_id;
    empire->SetAuthenticated(player_connection->IsAuthenticated());

    // drop previous connection to that empire
    if (previous_player_id != Networking::INVALID_PLAYER_ID && previous_player_id != player_connection->PlayerID()) {
        WarnLogger() << "ServerApp::AddPlayerIntoGame empire " << empire_id
                     << " previous player " << previous_player_id
                     << " was kicked.";
        DropPlayerEmpireLink(previous_player_id);
        auto previous_it = m_networking.GetPlayer(previous_player_id);
        if (previous_it != m_networking.established_end()) {
            const Networking::ClientType previous_client_type = (*previous_it)->GetClientType();
            const std::string previous_player_name = (*previous_it)->PlayerName();
            m_networking.Disconnect(previous_player_id);
            if (previous_client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
                // change empire's player so after reload the player still could connect
                // to the empire
                empire->SetPlayerName(player_connection->PlayerName());
                // kill unneeded AI process
                auto it = m_ai_client_processes.find(previous_player_name);
                if (it != m_ai_client_processes.end()) {
                    it->second.Kill();
                    m_ai_client_processes.erase(it);
                }
            }
        }
    }

    InfoLogger() << "ServerApp::AddPlayerIntoGame empire " << empire_id << " connected to " << player_connection->PlayerID();

    const OrderSet dummy;
    const OrderSet& orders = orders_it->second && orders_it->second->orders ? *(orders_it->second->orders) : dummy;
    const SaveGameUIData* ui_data = orders_it->second ? orders_it->second->ui_data.get() : nullptr;

    if (GetOptionsDB().Get<bool>("network.server.drop-empire-ready")) {
        // drop ready status
        empire->SetReady(false);
        m_networking.SendMessageAll(PlayerStatusMessage(Message::PlayerStatus::PLAYING_TURN,
                                                        empire_id));
    }

    auto player_info_map = GetPlayerInfoMap();
    bool use_binary_serialization = player_connection->IsBinarySerializationUsed();

    player_connection->SendMessage(GameStartMessage(
        m_single_player_game, empire_id,
        m_current_turn, m_empires, m_universe,
        m_species_manager, GetCombatLogManager(),
        m_supply_manager, player_info_map, orders,
        ui_data,
        m_galaxy_setup_data,
        use_binary_serialization,
        !player_connection->IsLocalConnection()));

    return empire_id;
}

std::vector<std::string> ServerApp::GetPlayerDelegation(const std::string& player_name) {
    std::vector<std::string> result;
    bool success = false;
    try {
        m_python_server.SetCurrentDir(GetPythonAuthDir());
        // Call the auth provider function get_player_delegation
        success = m_python_server.GetPlayerDelegation(player_name, result);
    } catch (const boost::python::error_already_set&) {
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
    return {result.begin(), result.end()};
}

bool ServerApp::IsHostless() const
{ return GetOptionsDB().Get<bool>("hostless"); }

const boost::circular_buffer<ChatHistoryEntity>& ServerApp::GetChatHistory() const
{ return m_chat_history; }

std::vector<PlayerSaveGameData> ServerApp::GetPlayerSaveGameData() const {
    std::vector<PlayerSaveGameData> player_save_game_data;
    for (const auto& [empire_id, save_data] : m_turn_sequence) {
        DebugLogger() << "ServerApp::GetPlayerSaveGameData() Empire " << empire_id
                      << " type: " << to_string(save_data->client_type)
                      << " save_game_data state string size: " << save_data->save_state_string.size()
                      << " UI data?: " << save_data->ui_data;
        if (save_data)
            player_save_game_data.push_back(*save_data);
    }
    return player_save_game_data;
}

Networking::ClientType ServerApp::GetEmpireClientType(int empire_id) const
{ return GetPlayerClientType(ServerApp::EmpirePlayerID(empire_id)); }

Networking::ClientType ServerApp::GetPlayerClientType(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return Networking::ClientType::INVALID_CLIENT_TYPE;

    const auto it = m_networking.GetPlayer(player_id);
    if (it == m_networking.established_end())
        return Networking::ClientType::INVALID_CLIENT_TYPE;
    const auto& player_connection = *it;
    return player_connection->GetClientType();
}

int ServerApp::EffectsProcessingThreads() const
{ return GetOptionsDB().Get<int>("effects.server.threads"); }

void ServerApp::AddEmpireTurn(int empire_id, const PlayerSaveGameData& psgd)
{ m_turn_sequence[empire_id] = std::make_unique<PlayerSaveGameData>(psgd); }

void ServerApp::RemoveEmpireTurn(int empire_id)
{ m_turn_sequence.erase(empire_id); }

void ServerApp::ClearEmpireTurnOrders(int empire_id) {
    for (auto& [stored_empire_id, save_game_data] : m_turn_sequence) {
        if (empire_id != ALL_EMPIRES && stored_empire_id != empire_id)
            continue; // all empires, unless a single one was specified
        if (save_game_data) {
            // reset only orders
            // left UI data and AI state intact
            save_game_data->orders.reset();
        }
    }
}

void ServerApp::SetEmpireSaveGameData(int empire_id, std::unique_ptr<PlayerSaveGameData>&& save_game_data)
{ m_turn_sequence[empire_id] = std::move(save_game_data); }

void ServerApp::UpdatePartialOrders(int empire_id, const OrderSet& added,
                                    const std::set<int>& deleted)
{
    const auto& psgd = m_turn_sequence[empire_id];
    if (psgd) {
        if (psgd->orders) {
            for (int del_id : deleted)
                 psgd->orders->erase(del_id);
            for (auto& add_set : added)
                 psgd->orders->insert(add_set);
        } else {
            psgd->orders = std::make_shared<OrderSet>(added);
        }
    }
}

void ServerApp::RevokeEmpireTurnReadyness(int empire_id) {
    if (auto empire = m_empires.GetEmpire(empire_id))
        empire->SetReady(false);
}

bool ServerApp::AllOrdersReceived() {
    // debug output
    DebugLogger() << "ServerApp::AllOrdersReceived for turn: " << m_current_turn
                  << (m_turn_expired ? " (expired)" : "");
    bool all_orders_received = true;
    for (const auto& [empire_id, save_data] : m_turn_sequence) {
        bool empire_orders_received = true;
        const auto empire = m_empires.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << " ... invalid empire id in turn sequence: "<< empire_id;
            continue;
        } else if (empire->Eliminated()) {
            ErrorLogger() << " ... eliminated empire in turn sequence: " << empire_id;
            continue;
        } else if (!empire->Ready()) {
            DebugLogger() << " ... not ready empire id: " << empire_id;
            empire_orders_received = false;
        } else if (!save_data) {
            DebugLogger() << " ... no orders from empire id: " << empire_id;
            empire_orders_received = false;
        } else if (!save_data->orders) {
            DebugLogger() << " ... no orders from empire id: " << empire_id;
            empire_orders_received = false;
        } else {
            DebugLogger() << " ... have orders from empire id: " << empire_id;
        }
        if (!empire_orders_received) {
            if (GetEmpireClientType(empire_id) != Networking::ClientType::CLIENT_TYPE_AI_PLAYER
                && m_turn_expired)
            {
                DebugLogger() << " ...... turn expired for empire id: " << empire_id;
            } else {
                all_orders_received = false;
            }
        }
    }
    return all_orders_received;
}

namespace {
    /** Returns true if \a empire has been eliminated by the applicable
      * definition of elimination.  As of this writing, elimination means
      * having no ships and no population on planets. */
    bool EmpireEliminated(int empire_id, const ObjectMap& objects) {
        // are there any populated planets? if so, not eliminated
        // are there any ships? if so, not eliminated
        return !objects.check_if_any<Planet>([empire_id](const auto* p)
                                             { return p->OwnedBy(empire_id) && p->Populated(); }) &&
               !objects.check_if_any<Ship>([empire_id](const auto* s)
                                           { return s->OwnedBy(empire_id); });
    }

    void GetEmpireFleetsAtSystem(std::map<int, std::set<int>>& empire_fleets, int system_id,
                                 const ObjectMap& objects)
    {
        empire_fleets.clear();
        auto system = objects.get<System>(system_id);
        if (!system)
            return;
        for (auto* fleet : objects.findRaw<const Fleet>(system->FleetIDs()))
            empire_fleets[fleet->Owner()].emplace(fleet->ID());
    }

    void GetEmpirePlanetsAtSystem(std::map<int, std::set<int>>& empire_planets, int system_id,
                                  const ObjectMap& objects)
    {
        empire_planets.clear();
        auto* system = objects.getRaw<System>(system_id);
        if (!system)
            return;
        for (auto* planet : objects.findRaw<const Planet>(system->PlanetIDs())) {
            if (!planet->Unowned())
                empire_planets[planet->Owner()].emplace(planet->ID());
            else if (planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f)
                empire_planets[ALL_EMPIRES].emplace(planet->ID());
        }
    }

    void GetFleetsVisibleToEmpireAtSystem(std::set<int>& visible_fleets, int empire_id, int system_id,
                                          const ScriptingContext& context)
    {
        const ObjectMap& objects{context.ContextObjects()};

        visible_fleets.clear();
        auto* system = objects.getRaw<System>(system_id);
        if (!system)
            return; // no such system
        const auto& fleet_ids = system->FleetIDs();
        if (fleet_ids.empty())
            return; // no fleets to be seen
        if (empire_id != ALL_EMPIRES && !context.GetEmpire(empire_id))
            return; // no such empire

        TraceLogger(combat) << "\t** GetFleetsVisibleToEmpire " << empire_id << " at system " << system->Name();
        // for visible fleets by an empire, check visibility of fleets by that empire
        if (empire_id != ALL_EMPIRES) {
            for (const auto* fleet : objects.findRaw<Fleet>(fleet_ids)) {
                if (!fleet)
                    continue;
                if (fleet->OwnedBy(empire_id))
                    continue;   // don't care about fleets owned by the same empire for determining combat conditions
                Visibility fleet_vis = context.ContextVis(fleet->ID(), empire_id);
                TraceLogger(combat) << "\t\tfleet (" << fleet->ID() << ") has visibility rank " << fleet_vis;
                if (fleet_vis >= Visibility::VIS_BASIC_VISIBILITY)
                    visible_fleets.emplace(fleet->ID());
            }
            return;
        }


        // now considering only fleets visible to monsters


        // get best monster detection strength here.  Use monster detection meters for this...
        float monster_detection_strength_here = 0.0f;
        for (const auto* ship : objects.findRaw<Ship>(system->ShipIDs())) {
            if (!ship || !ship->Unowned())  // only want unowned / monster ships
                continue;
            if (ship->GetMeter(MeterType::METER_DETECTION)->Initial() > monster_detection_strength_here)
                monster_detection_strength_here = ship->GetMeter(MeterType::METER_DETECTION)->Initial();
        }

        // test each ship in each fleet for visibility by best monster detection here
        for (const auto* fleet : objects.findRaw<Fleet>(fleet_ids)) {
            if (!fleet)
                continue;
            if (fleet->Unowned()) {
                visible_fleets.emplace(fleet->ID());   // fleet is monster, so can be sen by monsters
                continue;
            }

            for (const auto* ship : objects.findRaw<Ship>(fleet->ShipIDs())) {
                if (!ship)
                    continue;
                // if a ship is low enough stealth, its fleet can be seen by monsters
                if (monster_detection_strength_here >= ship->GetMeter(MeterType::METER_STEALTH)->Initial()) {
                    visible_fleets.emplace(fleet->ID());
                    break;  // fleet is seen, so don't need to check any more ships in it
                }
            }
        }
    }

    void GetPlanetsVisibleToEmpireAtSystem(std::set<int>& visible_planets, int empire_id, int system_id,
                                           const ScriptingContext& context)
    {
        const auto& objects{context.ContextObjects()};

        visible_planets.clear();
        auto* system = objects.getRaw<System>(system_id);
        if (!system)
            return; // no such system
        const auto& planet_ids = system->PlanetIDs();
        if (planet_ids.empty())
            return; // no planets to be seen
        if (empire_id != ALL_EMPIRES && !context.GetEmpire(empire_id))
            return; // no such empire

        TraceLogger(combat) << "\t** GetPlanetsVisibleToEmpire " << empire_id
                            << " at system " << system->Name();
        // for visible planets by an empire, check visibility of planet by that empire
        if (empire_id != ALL_EMPIRES) {
            for (int planet_id : planet_ids) {
                // include planets visible to empire
                Visibility planet_vis = context.ContextVis(planet_id, empire_id);
                TraceLogger(combat) << "\t\tplanet (" << planet_id << ") has visibility rank " << planet_vis;
                if (planet_vis <= Visibility::VIS_BASIC_VISIBILITY)
                    continue;
                // skip planets that have no owner and that are unpopulated; don't matter for combat conditions test
                auto* planet = objects.getRaw<Planet>(planet_id);
                if (planet->Unowned() && planet->GetMeter(MeterType::METER_POPULATION)->Initial() <= 0.0f)
                    continue;
                visible_planets.insert(planet->ID());
            }
            return;
        }


        // now considering only planets visible to monsters


        // get best monster detection strength here.  Use monster detection meters for this...
        float monster_detection_strength_here = 0.0f;
        for (auto* ship : objects.findRaw<const Ship>(system->ShipIDs())) {
            if (!ship->Unowned())  // only want unowned / monster ships
                continue;
            if (ship->GetMeter(MeterType::METER_DETECTION)->Initial() > monster_detection_strength_here)
                monster_detection_strength_here = ship->GetMeter(MeterType::METER_DETECTION)->Initial();
        }

        // test each planet for visibility by best monster detection here
        for (auto* planet : objects.findRaw<const Planet>(system->PlanetIDs())) {
            if (planet->Unowned())
                continue;       // only want empire-owned planets; unowned planets visible to monsters don't matter for combat conditions test
            // if a planet is low enough stealth, it can be seen by monsters
            if (monster_detection_strength_here >= planet->GetMeter(MeterType::METER_STEALTH)->Initial())
                visible_planets.insert(planet->ID());
        }
    }

    /** Returns true iff there is an appropriate combination of objects in the
      * system with id \a system_id for a combat to occur. */
    bool CombatConditionsInSystem(int system_id, const ScriptingContext& context) {
        const auto& objects{context.ContextObjects()};

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
        GetEmpireFleetsAtSystem(empire_fleets_here, system_id, objects);
        if (empire_fleets_here.empty())
            return false;

        auto* this_system = objects.getRaw<System>(system_id);
        DebugLogger(combat) << "CombatConditionsInSystem() for system (" << system_id << ") " << this_system->Name();
        // which empires have aggressive ships here? (including monsters as id ALL_EMPIRES)
        std::set<int> empires_with_aggressive_fleets_here;
        for (auto& empire_fleets : empire_fleets_here) {
            int empire_id = empire_fleets.first;
            for (const auto* fleet : objects.findRaw<Fleet>(empire_fleets.second)) {
                if (!fleet)
                    continue;
                // an unarmed Monster will not trigger combat
                if (  (fleet->Aggressive() || fleet->Unowned())  &&
                      (fleet->CanDamageShips(context) || !fleet->Unowned())  )
                {
                    if (!empires_with_aggressive_fleets_here.contains(empire_id))
                        DebugLogger(combat) << "\t Empire " << empire_id << " has at least one aggressive fleet present";
                    empires_with_aggressive_fleets_here.emplace(empire_id);
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
        GetEmpirePlanetsAtSystem(empire_planets_here, system_id, objects);
        if (empire_planets_here.empty() && empire_fleets_here.size() <= 1) {
            DebugLogger(combat) << "\t Only one combatant present: no combat.";
            return false;
        }

        // all empires with something here
        std::set<int> empires_here;
        for (auto& empire_fleets : empire_fleets_here)
            empires_here.emplace(empire_fleets.first);
        for (auto& empire_planets : empire_planets_here)
            empires_here.emplace(empire_planets.first);

        // what combinations of present empires are at war?
        std::map<int, std::set<int>> empires_here_at_war;  // for each empire, what other empires here is it at war with?
        for (auto emp1_it = empires_here.begin();
             emp1_it != empires_here.end(); ++emp1_it)
        {
            auto emp2_it = emp1_it;
            ++emp2_it;
            for (; emp2_it != empires_here.end(); ++emp2_it) {
                if (*emp1_it == ALL_EMPIRES || *emp2_it == ALL_EMPIRES ||
                    context.ContextDiploStatus(*emp1_it, *emp2_it) == DiplomaticStatus::DIPLO_WAR)
                {
                    empires_here_at_war[*emp1_it].emplace(*emp2_it);
                    empires_here_at_war[*emp2_it].emplace(*emp1_it);
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
            GetPlanetsVisibleToEmpireAtSystem(aggressive_empire_visible_planets, aggressive_empire_id,
                                              system_id, context);

            // is any planet owned by an empire at war with aggressive empire?
            for (const auto& planet : objects.find<Planet>(aggressive_empire_visible_planets)) {
                if (!planet)
                    continue;
                int visible_planet_empire_id = planet->Owner();

                if (aggressive_empire_id != visible_planet_empire_id &&
                    at_war_with_empire_ids.contains(visible_planet_empire_id))
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
            GetFleetsVisibleToEmpireAtSystem(aggressive_empire_visible_fleets, aggressive_empire_id,
                                             system_id, context);

            // is any fleet owned by an empire at war with aggressive empire?
            for (const auto* fleet : objects.findRaw<Fleet>(aggressive_empire_visible_fleets)) {
                if (!fleet)
                    continue;
                int visible_fleet_empire_id = fleet->Owner();

                if (aggressive_empire_id != visible_fleet_empire_id &&
                    at_war_with_empire_ids.contains(visible_fleet_empire_id))
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
    std::vector<CombatInfo> AssembleSystemCombatInfo(ScriptingContext& context) {
        std::vector<CombatInfo> combats;
        combats.reserve(context.ContextObjects().size());
        // for each system, find if a combat will occur in it, and if so, assemble
        // necessary information about that combat in combats
        for (const auto& sys : context.ContextObjects().allRaw<System>()) {
            if (CombatConditionsInSystem(sys->ID(), context))
                combats.emplace_back(sys->ID(), context.current_turn, context.ContextUniverse(),
                                     context.Empires(), context.diplo_statuses,
                                     context.galaxy_setup_data, context.species,
                                     context.supply);
        }
        return combats;
    }

    /** Back project meter values of objects in combat info, so that changes to
      * meter values from combat aren't lost when resetting meters during meter
      * updating after combat. */
    void BackProjectSystemCombatInfoObjectMeters(std::vector<CombatInfo>& combats) {
        for (CombatInfo& combat : combats) {
            for (auto* object : combat.objects.allRaw())
                object->BackPropagateMeters();
        }
    }

    /** Takes contents of CombatInfo struct and puts it into the universe.
      * Used to store results of combat in main universe. */
    void DisseminateSystemCombatInfo(const std::vector<CombatInfo>& combats, Universe& universe,
                                     const EmpireManager& empires)
    {
        // As of this writing, pointers to objects are inserted into the combat
        // ObjectMap, and these pointers still refer to the main gamestate
        // objects. Thus, copying the objects in the main gamestate are already
        // modified by the combat, and don't need to be copied from the combat
        // ObjectMap. However, the changes to object visibility during combat
        // are stored separately, and do need to be copied back to the main
        // gamestate. Standard visibility updating will then transfer the
        // modified objects / combat results to empires' known gamestate
        // ObjectMaps.
        const auto& empire_ids = empires.EmpireIDs();

        for (const CombatInfo& combat_info : combats) {
            // update visibilities from combat, in case anything was revealed
            // by shooting during combat
            for (const auto& empire_vis : combat_info.empire_object_visibility) {
                for (const auto& object_vis : empire_vis.second) {
                    if (object_vis.first < 0)
                        continue;   // temporary fighter IDs
                    universe.SetEmpireObjectVisibility(empire_vis.first, object_vis.first, object_vis.second);  // does not lower visibility
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
            for (const auto& [empire_id, obj_ids] : combat_info.destroyed_object_knowers) {
                for (int object_id : obj_ids) {
                    //DebugLogger() << "Setting knowledge of destroyed object " << object_id
                    //                       << " for empire " << empire_id;
                    universe.SetEmpireKnowledgeOfDestroyedObject(object_id, empire_id);

                    // record if empire should be informed of potential fleet
                    // destruction (which is checked later)
                    if (const auto* ship = universe.Objects().getRaw<Ship>(object_id)) {
                        if (ship->FleetID() != INVALID_OBJECT_ID)
                            empires_to_update_of_fleet_destruction[ship->FleetID()].emplace(empire_id);
                    }
                }
            }


            // destroy, in main universe, objects that were destroyed in combat,
            // and any associated objects that should now logically also be
            // destroyed
            std::set<int> all_destroyed_object_ids;
            for (int destroyed_object_id : combat_info.destroyed_object_ids) {
                auto dest_obj_ids = universe.RecursiveDestroy(destroyed_object_id, empire_ids);
                all_destroyed_object_ids.insert(dest_obj_ids.begin(), dest_obj_ids.end());
            }


            // after recursive object destruction, fleets might have been
            // destroyed. If so, need to also update empires knowledge of this
            for (const auto& fleet_empires : empires_to_update_of_fleet_destruction) {
                int fleet_id = fleet_empires.first;
                if (!all_destroyed_object_ids.contains(fleet_id))
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
            if (auto system = universe.Objects().get<System>(combat_info.system_id)) {
                // ensure all participants get updates on system.  this ensures
                // that an empire who lose all objects in the system still
                // knows about a change in system ownership
                for (int empire_id : combat_info.empire_ids)
                    universe.EmpireKnownObjects(empire_id).CopyObject(system, ALL_EMPIRES, universe);
            }
        }
    }

    /** Creates sitreps for all empires involved in a combat. */
    void CreateCombatSitReps(std::vector<CombatInfo>& combats) {
        CombatLogManager& log_manager = GetCombatLogManager();

        for (CombatInfo& combat_info : combats) {
            // add combat log entry
            int log_id = log_manager.AddNewLog(CombatLog{combat_info});

            // basic "combat occured" sitreps
            for (int empire_id : combat_info.empire_ids) {
                if (auto empire{combat_info.GetEmpire(empire_id)})
                    empire->AddSitRepEntry(CreateCombatSitRep(
                        combat_info.system_id, log_id, EnemyId(empire_id, combat_info.empire_ids),
                        combat_info.turn));
            }

            // sitreps about destroyed objects
            for (auto& [knowing_empire_id, known_destroyed_object_ids] :
                 combat_info.destroyed_object_knowers)
            {
                if (auto empire{combat_info.GetEmpire(knowing_empire_id)}) {
                    for (int dest_obj_id : known_destroyed_object_ids) {
                        if (auto* obj = combat_info.objects.getRaw(dest_obj_id))
                            empire->AddSitRepEntry(CreateCombatDestroyedObjectSitRep(
                                obj, combat_info.system_id, knowing_empire_id, combat_info.turn));
                    }
                }
            }

            // sitreps about damaged objects
            for (int damaged_object_id : combat_info.damaged_object_ids) {
                //DebugLogger() << "Checking object " << damaged_object_id << " for damaged sitrep";
                // is object destroyed? If so, don't need a damage sitrep
                if (combat_info.destroyed_object_ids.contains(damaged_object_id)) {
                    //DebugLogger() << " ... Object is destroyed and doesn't need a sitrep.";
                    continue;
                }
                const auto* obj = combat_info.objects.getRaw(damaged_object_id);
                if (!obj) {
                    ErrorLogger() << "CreateCombatSitreps couldn't find damaged object with id: " << damaged_object_id;
                    continue;
                }

                // which empires know about this object?
                for (auto& [viewing_empire_id, empire_known_objects] : combat_info.empire_object_visibility) {
                    // does this empire know about this object?
                    auto damaged_obj_it = empire_known_objects.find(damaged_object_id);
                    if (damaged_obj_it == empire_known_objects.end())
                        continue;
                    if (damaged_obj_it->second < Visibility::VIS_PARTIAL_VISIBILITY)
                        continue;

                    if (auto empire = combat_info.GetEmpire(viewing_empire_id))
                        empire->AddSitRepEntry(CreateCombatDamagedObjectSitRep(
                            obj, combat_info.system_id, viewing_empire_id, combat_info.turn));
                }
            }
        }
    }

    /** De-nests sub-events into a single layer list of events */
    std::vector<ConstCombatEventPtr> FlattenEvents(const std::vector<CombatEventPtr>& events1) {
        std::vector<ConstCombatEventPtr> flat_events{events1.begin(), events1.end()}; // copy top-level input events

        // copy nested sub-events of top-level events
        for (const auto& event1 : events1) {                        // can't modify the input events pointers
            auto events2{event1->SubEvents(ALL_EMPIRES)};           // makes movable pointers to event1's sub-events

            for (auto& event2 : events2) {
                auto events3{event2->SubEvents(ALL_EMPIRES)};       // makes movable pointers to event2's sub-events
                flat_events.push_back(std::move(event2));           // can move the pointers to events2 = event1's sub-events

                for (auto& event3 : events3) {
                    auto events4{event3->SubEvents(ALL_EMPIRES)};   // makes movable pointers to event3's sub-events
                    flat_events.push_back(std::move(event3));       // can move the pointers to events3 = event2's sub-events

                    for (auto& event4 : events4)
                        flat_events.push_back(std::move(event4));   // can move the pointers to events4 = event3's sub-events
                }
            }
        }

        return flat_events;
    }

    /** Records info in Empires about what they destroyed or had destroyed during combat. */
    void UpdateEmpireCombatDestructionInfo(const std::vector<CombatInfo>& combats,
                                           ScriptingContext& context)
    {
        for (const CombatInfo& combat_info : combats) {
            const auto& combat_objects = combat_info.objects;

            std::vector<WeaponFireEvent::ConstWeaponFireEventPtr> events_that_killed;
            for (auto& event : FlattenEvents(combat_info.combat_events)) { // TODO: could do the filtering in the call function and avoid some moves later...
                auto fire_event = std::dynamic_pointer_cast<const WeaponFireEvent>(std::move(event));
                if (fire_event && combat_info.destroyed_object_ids.contains(fire_event->target_id)) {
                    TraceLogger() << "Kill event: " << fire_event->DebugString(context);
                    events_that_killed.push_back(std::move(fire_event));
                }
            }
            DebugLogger() << "Combat combat_info system: " << combat_info.system_id
                          << "  Total Kill Events: " << events_that_killed.size();



            // If a ship was attacked multiple times during a combat in which it dies, it will get
            // processed multiple times here.  The below set will keep it from being logged as
            // multiple destroyed ships for its owner.
            std::unordered_set<int> already_logged__target_ships;
            std::unordered_map<int, int> empire_destroyed_ship_ids;

            for (const auto& attack_event : events_that_killed) {
                auto attacker = combat_objects.get(attack_event->attacker_id);
                if (!attacker)
                    continue;
                const int attacker_empire_id = attacker->Owner();
                auto attacker_empire = context.GetEmpire(attacker_empire_id);

                auto* target_ship = combat_objects.getRaw<Ship>(attack_event->target_id);
                if (!target_ship)
                    continue;
                const int target_empire_id = target_ship->Owner();
                auto target_empire = context.GetEmpire(target_empire_id);

                const auto attacker_object_type = attacker->ObjectType();

                DebugLogger() << "Attacker " << to_string(attacker_object_type)
                              << " " << attacker->Name() << " (id: " << attacker->ID()
                              << "  empire: " << std::to_string(attacker_empire_id)
                              << ")  attacks " << target_ship->Name()
                              << " (id: " << target_ship->ID()
                              << "  empire: " << std::to_string(target_empire_id)
                              << "  species: " << target_ship->SpeciesName() << ")";

                if (attacker_empire)
                    attacker_empire->RecordShipShotDown(*target_ship);

                if (target_empire) {
                    if (already_logged__target_ships.contains(attack_event->target_id))
                        continue;
                    already_logged__target_ships.insert(attack_event->target_id);
                    target_empire->RecordShipLost(*target_ship);
                }
            }
        }
    }

    /** Records info in Empires about where they invaded. */
    void UpdateEmpireInvasionInfo(const std::map<int, std::map<int, double>>& planet_empire_invasion_troops,
                                  EmpireManager& empires, const ObjectMap& objects)
    {
        for (auto& [planet_id, empire_troops] : planet_empire_invasion_troops) {
            const auto* planet = objects.getRaw<Planet>(planet_id);
            if (!planet || planet->SpeciesName().empty())
                continue;

            for (auto& [invader_empire_id, troops] : empire_troops) {
                (void)troops; // quiet warning
                const auto invader_empire = empires.GetEmpire(invader_empire_id);
                if (!invader_empire)
                    continue;
                invader_empire->RecordPlanetInvaded(*planet);
            }
        }
    }

    /** Does colonization, with safety checks */
    bool ColonizePlanet(int ship_id, int planet_id, ScriptingContext& context,
                        const std::vector<int>& empire_ids)
    {
        auto& objects = context.ContextObjects();
        auto& universe = context.ContextUniverse();

        auto* ship = objects.getRaw<Ship>(ship_id);
        if (!ship) {
            ErrorLogger() << "ColonizePlanet couldn't get ship with id " << ship_id;
            return false;
        }
        auto* planet = objects.getRaw<Planet>(planet_id);
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

        const float colonist_capacity = ship->ColonyCapacity(universe);

        if (colonist_capacity > 0.0f &&
            planet->EnvironmentForSpecies(context, species_name) < PlanetEnvironment::PE_HOSTILE)
        {
            ErrorLogger() << "ColonizePlanet nonzero colonist capacity and planet that ship's species can't colonize";
            return false;
        }

        // get empire to give ownership of planet to
        if (ship->Unowned()) {
            ErrorLogger() << "ColonizePlanet couldn't get an empire to colonize with";
            return false;
        }
        const int empire_id = ship->Owner();

        // all checks passed.  proceed with colonization.

        // colonize planet by calling Planet class Colonize member function
        // do this BEFORE destroying the ship, since species_name is a const reference to Ship::m_species_name
        if (!planet->Colonize(empire_id, species_name, colonist_capacity, context)) {
            ErrorLogger() << "ColonizePlanet: couldn't colonize planet";
            return false;
        }

        auto* system = objects.getRaw<System>(ship->SystemID());

        // destroy colonizing ship, and its fleet if now empty
        if (auto* fleet = objects.getRaw<Fleet>(ship->FleetID())) {
            fleet->RemoveShips({ship->ID()});
            if (fleet->Empty()) {
                if (system)
                    system->Remove(fleet->ID());
                universe.Destroy(fleet->ID(), empire_ids);
            }
        }

        if (system)
            system->Remove(ship->ID());
        universe.RecursiveDestroy(ship->ID(), empire_ids); // does not count as a loss of a ship for the species / empire

        return true;
    }

    /** Determines which ships ordered to colonize planet succeed, does
      * appropriate colonization, and cleans up after colonization orders.
      * Returns the IDs of planets that were colonized and IDs of ships that
      * colonized. */
    [[nodiscard]] std::pair<std::vector<int>, std::vector<int>> HandleColonization(ScriptingContext& context) {
        Universe& universe = context.ContextUniverse();
        ObjectMap& objects = context.ContextObjects();
        const auto empire_ids{context.EmpireIDs()};

        // collect, for each planet, what ships have been ordered to colonize it
        std::map<int, std::map<int, std::set<int>>> planet_empire_colonization_ship_ids; // map from planet ID to map from empire ID to set of ship IDs

        for (auto* ship : objects.allRaw<Ship>()) {
            if (ship->Unowned())
                continue;
            const int owner_empire_id = ship->Owner();
            const int ship_id = ship->ID();
            if (ship_id == INVALID_OBJECT_ID)
                continue;

            const int colonize_planet_id = ship->OrderedColonizePlanet();
            if (colonize_planet_id == INVALID_OBJECT_ID)
                continue;

            ship->SetColonizePlanet(INVALID_OBJECT_ID); // reset so failed colonization doesn't leave ship with hanging colonization order set

            auto* planet = objects.getRaw<Planet>(colonize_planet_id);
            if (!planet)
                continue;

            if (ship->SystemID() != planet->SystemID() || ship->SystemID() == INVALID_OBJECT_ID)
                continue;

            planet->ResetIsAboutToBeColonized();

            planet_empire_colonization_ship_ids[colonize_planet_id][owner_empire_id].insert(ship_id);
        }


        std::vector<int> colonized_planet_ids;
        colonized_planet_ids.reserve(planet_empire_colonization_ship_ids.size());
        std::vector<int> colonizing_ship_ids;
        colonizing_ship_ids.reserve(planet_empire_colonization_ship_ids.size()); // possibly an underestimate

        // execute colonization except when:
        // 1) an enemy empire has armed aggressive ships in the system
        // 2) multiple empires try to colonize a planet on the same turn
        for (const auto& [planet_id, empires_ships_colonizing] : planet_empire_colonization_ship_ids) {
            const auto* const planet = objects.getRaw<Planet>(planet_id);
            if (!planet) {
                ErrorLogger() << "HandleColonization couldn't get planet with id " << planet_id;
                continue;
            }
            const int system_id = planet->SystemID();
            const auto* const system = objects.getRaw<System>(system_id);
            if (!system) {
                ErrorLogger() << "HandleColonization couldn't get system with id " << system_id;
                continue;
            }

            // can't colonize if multiple empires attempting to do so on same turn
            if (empires_ships_colonizing.size() != 1) {
                // generate sitreps for every empire that tried to colonize this planet
                for (const auto& [empire_id, colonizing_ships] : empires_ships_colonizing) {
                    const auto empire = context.GetEmpire(empire_id);
                    if (!empire) {
                        ErrorLogger() << "HandleColonization couldn't get empire with id " << empire_id;
                        continue;
                    }

                    const auto is_visible =
                        [empire_it{context.empire_object_vis.find(empire_id)},
                         end_it{context.empire_object_vis.end()}](const int obj_id) -> bool
                    {
                        if (empire_it == end_it)
                            return false;
                        const auto obj_it = empire_it->second.find(obj_id);
                        if (obj_it == empire_it->second.end())
                            return false;
                        return obj_it->second >= Visibility::VIS_BASIC_VISIBILITY;
                    };

                    for (const int ship_id : colonizing_ships) {
                        bool created_empire_specific_message = false;


                        // check other ships colonizing here...
                        for (const auto& [other_empire_id, other_empire_colonizing_ships] : empires_ships_colonizing) {
                            for (const auto other_ship_id : other_empire_colonizing_ships) {
                                if (!is_visible(other_ship_id))
                                    continue;
                                if (other_ship_id == ship_id)
                                    continue;
                                empire->AddSitRepEntry(CreatePlanetEstablishFailedVisibleOtherSitRep(
                                    planet_id, ship_id, other_empire_id, context.current_turn));
                                created_empire_specific_message = true;
                                break;
                            }
                        }
                        // can this empire see another ship that attempted to colonize the same planet?
                        // SITREP_PLANET_ESTABLISH_FAILED_VISIBLE
                        //Ship %ship% failed to establish a colony or outpost on %planet% because the %empire% also attempted to establish on the same planet.
                        //SITREP_PLANET_ESTABLISH_FAILED_VISIBLE_LABEL

                        // no, just issue generic message
                        if (!created_empire_specific_message)
                            empire->AddSitRepEntry(CreatePlanetEstablishFailedSitRep(planet_id, ship_id, context.current_turn));
                    }
                }
                continue;
            }
            const int colonizing_empire_id = empires_ships_colonizing.begin()->first;
            auto empire = context.GetEmpire(colonizing_empire_id);

            const auto& empire_ships_colonizing = empires_ships_colonizing.begin()->second;
            if (empire_ships_colonizing.empty())
                continue;
            const int colonizing_ship_id = *empire_ships_colonizing.begin();

            // find which empires have obstructive armed ships in system
            std::set<int> empires_with_armed_ships_in_system;
            for (auto* fleet : objects.findRaw<const Fleet>(system->FleetIDs())) {
                if (fleet->Obstructive() && fleet->CanDamageShips(context))
                    empires_with_armed_ships_in_system.insert(fleet->Owner());  // may include ALL_EMPIRES, which is fine; this makes monsters prevent colonization
            }

            // are any of the empires with armed ships in the system enemies of the colonzing empire?
            bool colonize_blocked = false;
            for (int armed_ship_empire_id : empires_with_armed_ships_in_system) {
                if (armed_ship_empire_id == colonizing_empire_id)
                    continue;
                if (armed_ship_empire_id == ALL_EMPIRES ||
                    context.ContextDiploStatus(colonizing_empire_id, armed_ship_empire_id) == DiplomaticStatus::DIPLO_WAR)
                {
                    if (empire)
                        empire->AddSitRepEntry(CreatePlanetEstablishFailedArmedSitRep(
                            planet_id, colonizing_ship_id, armed_ship_empire_id, context.current_turn));
                    colonize_blocked = true;
                    break;
                }
            }

            if (colonize_blocked)
                continue;

            // before actual colonization, which deletes the colony ship, store ship info for later use with sitrep generation
            auto* ship = objects.getRaw<Ship>(colonizing_ship_id);
            if (!ship)
                ErrorLogger() << "HandleColonization couldn't get ship with id " << colonizing_ship_id;
            const auto& species_name = ship ? ship->SpeciesName() : "";
            float colonist_capacity = ship ? ship->ColonyCapacity(universe) : 0.0f;

            // do colonization
            if (!ColonizePlanet(colonizing_ship_id, planet_id, context, empire_ids))
                continue;   // skip sitrep if colonization failed

            // record successful colonization
            colonized_planet_ids.push_back(planet_id);
            colonizing_ship_ids.push_back(colonizing_ship_id);

            // sitrep about colonization / outposting
            if (!empire) {
                ErrorLogger() << "HandleColonization couldn't get empire with id " << colonizing_empire_id;
            } else {
                if (species_name.empty() || colonist_capacity <= 0.0f)
                    empire->AddSitRepEntry(CreatePlanetOutpostedSitRep(planet_id, context.current_turn));
                else
                    empire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet_id, species_name, context.current_turn));
            }
        }

        return {colonized_planet_ids, colonizing_ship_ids};
    }

    /** Determines which ships ordered to invade planets, does invasion and
      * ground combat resolution. Returns IDs of planets that had ground combat
      * occur on them and of ships that invaded a planet. */
    [[nodiscard]] std::pair<std::vector<int>, std::vector<int>> HandleInvasion(ScriptingContext& context) {
        std::map<int, std::map<int, double>> planet_empire_troops;  // map from planet ID to map from empire ID to pair consisting of set of ship IDs and amount of troops empires have at planet
        std::vector<Ship*> invade_ships;
        Universe& universe = context.ContextUniverse();
        ObjectMap& objects = context.ContextObjects();
        EmpireManager& empires = context.Empires();
        const auto& empire_ids = context.EmpireIDs();

        // collect ships that are invading and the troops they carry
        for (auto* ship : objects.findRaw<Ship>([&universe](const Ship& s) {
                                                    return s.SystemID() != INVALID_OBJECT_ID &&
                                                        s.OrderedInvadePlanet() != INVALID_OBJECT_ID &&
                                                        s.HasTroops(universe);
                                                }))
        {
            invade_ships.push_back(ship);

            auto* planet = objects.getRaw<Planet>(ship->OrderedInvadePlanet());
            if (!planet)
                continue;
            planet->ResetIsAboutToBeInvaded();

            if (ship->SystemID() != planet->SystemID())
                continue;
            if (ship->SystemID() == INVALID_OBJECT_ID)
                continue;

            // how many troops are invading?
            planet_empire_troops[ship->OrderedInvadePlanet()][ship->Owner()] += ship->TroopCapacity(universe);

            DebugLogger() << "HandleInvasion has accounted for "<< ship->TroopCapacity(universe)
                          << " troops to invade " << planet->Name()
                          << " and is destroying ship " << ship->ID()
                          << " named " << ship->Name();
        }

        std::vector<int> invading_ship_ids;
        invading_ship_ids.reserve(invade_ships.size());
        std::transform(invade_ships.begin(), invade_ships.end(), std::back_inserter(invading_ship_ids),
                       [](const auto* ship) { return ship->ID(); });

        // delete ships that invaded something
        for (auto* ship : invade_ships) {
            auto* system = objects.getRaw<System>(ship->SystemID());

            // destroy invading ships and their fleets if now empty
            if (auto* fleet = objects.getRaw<Fleet>(ship->FleetID())) {
                fleet->RemoveShips({ship->ID()});
                if (fleet->Empty()) {
                    if (system)
                        system->Remove(fleet->ID());
                    universe.Destroy(fleet->ID(), empire_ids);
                }
            }
            if (system)
                system->Remove(ship->ID());

            universe.RecursiveDestroy(ship->ID(), empire_ids); // does not count as ship loss for empire/species
        }

        // store invasion info in empires
        UpdateEmpireInvasionInfo(planet_empire_troops, empires, objects);


        // check each planet invading or other troops, such as due to empire troops,
        // native troops, or rebel troops
        for (const auto* planet : objects.allRaw<Planet>()) {
            planet_empire_troops[planet->ID()].merge(planet->EmpireGroundCombatForces());
            //auto empire_forces = planet->EmpireGroundCombatForces();
            //if (!empire_forces.empty())
            //    planet_empire_troops[planet->ID()].insert(empire_forces.begin(), empire_forces.end());
        }

        std::vector<int> ground_combat_planet_ids;
        ground_combat_planet_ids.reserve(planet_empire_troops.size());

        // process each planet's ground combats
        for (auto& [planet_id, empires_troops] : planet_empire_troops) {
            if (empires_troops.empty())
                continue;

            auto planet = objects.get<Planet>(planet_id);
            std::set<int> all_involved_empires;
            for (auto& [empire_id, ignored]: empires_troops) {
                (void)ignored; // quiet warning
                if (empire_id != ALL_EMPIRES)
                    all_involved_empires.insert(empire_id);
            }
            int planet_initial_owner_id = planet->Owner();
            if (planet_initial_owner_id != ALL_EMPIRES)
                all_involved_empires.insert(planet_initial_owner_id);


            if (empires_troops.size() == 1) {
                int empire_with_troops_id = empires_troops.begin()->first;
                if (planet->Unowned() && empire_with_troops_id == ALL_EMPIRES)
                    continue;   // if troops are neutral and planet is unowned, not a combat.
                if (planet->OwnedBy(empire_with_troops_id))
                    continue;   // if troops all belong to planet owner, not a combat.

            } else {
                DebugLogger() << "Ground combat troops on " << planet->Name() << " :";
                for (const auto& [empire_with_troops_id, empire_troop_level] : empires_troops)
                    DebugLogger() << " ... empire: " << empire_with_troops_id << " : " << empire_troop_level;
                Planet::ResolveGroundCombat(empires_troops, empires.GetDiplomaticStatuses());
                ground_combat_planet_ids.push_back(planet_id);
            }

            for (int empire_id : all_involved_empires) {
                if (auto empire = empires.GetEmpire(empire_id))
                    empire->AddSitRepEntry(CreateGroundCombatSitRep(
                        planet_id, EnemyId(empire_id, all_involved_empires), context.current_turn));
            }


            // process post-combat cleanup...
            if (empires_troops.size() == 1) {
                int victor_id = empires_troops.begin()->first;
                // single empire was victorious.  conquer planet if appropriate...
                // if planet is unowned and victor is an empire, or if planet is
                // owned by an empire that is not the victor, conquer it
                if ((victor_id != ALL_EMPIRES) && (planet->Unowned() || !planet->OwnedBy(victor_id))) {
                    planet->Conquer(victor_id, context);

                    // create planet conquered sitrep for all involved empires
                    for (int empire_id : all_involved_empires) {
                        if (auto empire = empires.GetEmpire(empire_id))
                            empire->AddSitRepEntry(CreatePlanetCapturedSitRep(planet_id, victor_id, context.current_turn));
                    }

                    DebugLogger() << "Empire conquers planet";
                    for (auto& [empire_with_post_battle_troops_id, troop_count] : empires_troops)
                        DebugLogger() << " empire: " << empire_with_post_battle_troops_id << ": " << troop_count;


                } else if (!planet->Unowned() && victor_id == ALL_EMPIRES) {
                    int previous_owner_id = planet->Owner();
                    planet->Conquer(ALL_EMPIRES, context);
                    DebugLogger() << "Independents conquer planet";
                    for (const auto& empire_troops : empires_troops)
                        DebugLogger() << " empire: " << empire_troops.first << ": " << empire_troops.second;

                    for (int empire_id : all_involved_empires) {
                        if (auto empire = empires.GetEmpire(empire_id))
                            empire->AddSitRepEntry(CreatePlanetRebelledSitRep(planet_id, previous_owner_id, context.current_turn));
                    }

                } else {
                    // defender held the planet
                    DebugLogger() << "Defender holds planet";
                    for (auto& [empire_with_post_battle_troops_id, troop_count] : empires_troops)
                        DebugLogger() << " empire: " << empire_with_post_battle_troops_id << ": " << troop_count;
                }

                // regardless of whether battle resulted in conquering, it did
                // likely affect the troop numbers on the planet
                if (Meter* meter = planet->GetMeter(MeterType::METER_TROOPS))
                    meter->SetCurrent(empires_troops.begin()->second);  // new troops on planet is remainder after battle

            } else {
                // no troops left?
                if (Meter* meter = planet->GetMeter(MeterType::METER_TROOPS))
                    meter->SetCurrent(0.0f);
                if (Meter* meter = planet->GetMeter(MeterType::METER_REBEL_TROOPS))
                    meter->SetCurrent(0.0f);
            }

            planet->BackPropagateMeters();

            // knowledge update to ensure previous owner of planet knows who owns it now?
            if (planet_initial_owner_id != ALL_EMPIRES && planet_initial_owner_id != planet->Owner()) {
                // get empire's knowledge of object
                ObjectMap& empire_latest_known_objects = universe.EmpireKnownObjects(planet_initial_owner_id);
                empire_latest_known_objects.CopyObject(std::move(planet), planet_initial_owner_id, universe);
            }
        }

        return {ground_combat_planet_ids, invading_ship_ids};
    }

    /** Determines which fleets or planets ordered given to other empires,
      * and sets their new ownership. Returns the IDs of anything gifted. */
    template <typename IDsT>
    std::vector<int> HandleGifting(EmpireManager& empires, ObjectMap& objects, int current_turn,
                                   const IDsT& invaded_planet_ids, const IDsT& invading_ship_ids,
                                   const IDsT& colonizing_ship_ids)
    {
        // determine system IDs where empires can receive gifts
        std::map<int, std::set<int>> empire_receiving_locations;
        auto owned_planet = [](const Planet& p) { return !p.Unowned(); };
        for (const auto* planet : objects.findRaw<const Planet>(owned_planet))
            empire_receiving_locations[planet->SystemID()].insert(planet->Owner());
        auto owned_ship_in_system = [](const Ship& s) { return !s.Unowned() && s.SystemID() != INVALID_OBJECT_ID; };
        for (const auto* ship : objects.findRaw<const Ship>(owned_ship_in_system))
            empire_receiving_locations[ship->SystemID()].insert(ship->Owner());


        // collect fleets ordered to be given and their ships
        std::map<int, std::vector<Fleet*>> empire_gifted_fleets; // indexed by recipient empire id
        std::map<int, std::vector<Ship*>> empire_gifted_ships;
        auto owned_given_stationary_fleet = [&empire_receiving_locations](const Fleet& f) {
            if (f.Unowned() ||
                f.OrderedGivenToEmpire() == ALL_EMPIRES ||
                f.OwnedBy(f.OrderedGivenToEmpire()) ||
                !f.TravelRoute().empty() ||
                f.SystemID() == INVALID_OBJECT_ID)
            { return false; }
            auto it = empire_receiving_locations.find(f.SystemID());
            return it != empire_receiving_locations.end() &&
                it->second.contains(f.OrderedGivenToEmpire());
        };
        auto not_invading_not_colonizing_ship = [&invading_ship_ids, &colonizing_ship_ids](const Ship& s) {
            return std::none_of(invading_ship_ids.begin(), invading_ship_ids.end(),
                                [sid{s.ID()}] (const auto iid) { return iid == sid; }) &&
                std::none_of(colonizing_ship_ids.begin(), colonizing_ship_ids.end(),
                             [sid{s.ID()}] (const auto cid) { return cid == sid; });
        };
        for (auto* fleet : objects.findRaw<Fleet>(owned_given_stationary_fleet)) {
            const auto recipient_empire_id = fleet->OrderedGivenToEmpire();
            empire_gifted_fleets[recipient_empire_id].push_back(fleet);
            for (auto* ship : objects.findRaw<Ship>(fleet->ShipIDs())) {
                if (ship && not_invading_not_colonizing_ship(*ship))
                    empire_gifted_ships[recipient_empire_id].push_back(ship);
            }
        }
        for (auto& fleet : objects.all<Fleet>())
            fleet->ClearGiveToEmpire(); // in case things fail, to avoid potential inconsistent state


        // collect planets ordered to be given but that aren't being invaded,
        // and buildings on them
        std::map<int, std::vector<Planet*>> empire_gifted_planets; // indexed by recipient empire id
        std::map<int, std::vector<Building*>> empire_gifted_buildings;
        auto owned_given_not_invaded_planet =
            [&invaded_planet_ids, &empire_receiving_locations](const Planet& p)
        {
            if (p.Unowned() ||
                p.OrderedGivenToEmpire() == ALL_EMPIRES ||
                p.OwnedBy(p.OrderedGivenToEmpire()) ||
                p.SystemID() == INVALID_OBJECT_ID ||
                std::any_of(invaded_planet_ids.begin(), invaded_planet_ids.end(),
                            [pid{p.ID()}](const auto iid) { return iid == pid; }))
            { return false; }

            auto it = empire_receiving_locations.find(p.SystemID());
            return it != empire_receiving_locations.end() &&
                it->second.contains(p.OrderedGivenToEmpire());
        };
        for (auto* planet : objects.findRaw<Planet>(owned_given_not_invaded_planet)) {
            const auto recipient_empire_id = planet->OrderedGivenToEmpire();
            planet->ClearGiveToEmpire(); // in case things fail, to avoid potential inconsistent state
            empire_gifted_planets[recipient_empire_id].push_back(planet);
            for (auto* building : objects.findRaw<Building>(planet->BuildingIDs())) {
                if (building)
                    empire_gifted_buildings[recipient_empire_id].push_back(building);
            }
        }


        // storage for list of all gifted objects
        std::vector<int> gifted_object_ids;
        auto do_giving = [&gifted_object_ids, &empires, current_turn](auto& recipients_objs) {
            for (auto& [recipient_empire_id, objs] : recipients_objs) {
                for (auto* gifted_obj : objs) {
                    [[maybe_unused]] const auto initial_owner_empire_id = gifted_obj->Owner();
                    const auto gifted_obj_id = gifted_obj->ID();
                    gifted_object_ids.push_back(gifted_obj_id);

                    gifted_obj->SetOwner(recipient_empire_id);

                    using ObjsT = std::decay_t<decltype(*gifted_obj)>;
                    static_assert(!std::is_same_v<ObjsT, UniverseObject>);
                    static_assert(std::is_same_v<ObjsT, Planet> || std::is_same_v<ObjsT, Building> ||
                                  std::is_same_v<ObjsT, Fleet> || std::is_same_v<ObjsT, Ship>);

                    if constexpr (std::is_same_v<ObjsT, Planet>) {
                        if (auto empire = empires.GetEmpire(recipient_empire_id))
                            empire->AddSitRepEntry(CreatePlanetGiftedSitRep(gifted_obj_id, initial_owner_empire_id,
                                                                            current_turn));

                    } else if constexpr (std::is_same_v<ObjsT, Fleet>) {
                        if (auto empire = empires.GetEmpire(recipient_empire_id))
                            empire->AddSitRepEntry(CreateFleetGiftedSitRep(gifted_obj_id, initial_owner_empire_id,
                                                                           current_turn));

                    } else if constexpr (std::is_same_v<ObjsT, Ship>) {
                        gifted_obj->SetOrderedScrapped(false);
                        gifted_obj->ClearColonizePlanet();
                        gifted_obj->ClearInvadePlanet();
                        gifted_obj->ClearBombardPlanet();

                    } else if constexpr (std::is_same_v<ObjsT, Building>) {
                        gifted_obj->SetOrderedScrapped(false);
                    }

                    Empire::ConquerProductionQueueItemsAtLocation(gifted_obj_id, recipient_empire_id, empires);
                }
            }
        };

        do_giving(empire_gifted_fleets);
        do_giving(empire_gifted_ships);
        do_giving(empire_gifted_planets);
        do_giving(empire_gifted_buildings);

        return gifted_object_ids;
    }

    /** Destroys suitable objects that have been ordered scrapped.*/
    template <typename IDsT>
    void HandleScrapping(Universe& universe, EmpireManager& empires,
                         const IDsT& invading_ship_ids, const IDsT& invaded_planet_ids,
                         const IDsT& colonizing_ship_ids, const IDsT& colonized_planet_ids,
                         const IDsT& gifted_ids)
    {
        ObjectMap& objects{universe.Objects()};
        const auto& empire_ids = empires.EmpireIDs();

        // only scap ships that aren't being gifted and that aren't invading or colonizing this turn
        const auto scrapped_ships = objects.findRaw<Ship>(
            [&invading_ship_ids, &colonizing_ship_ids, &gifted_ids](const Ship* s) {
                return s->OrderedScrapped() &&
                    std::none_of(gifted_ids.begin(), gifted_ids.end(),
                                 [sid{s->ID()}](const auto gid) { return gid == sid; }) &&
                    std::none_of(invading_ship_ids.begin(), invading_ship_ids.end(),
                                 [sid{s->ID()}](const auto iid) { return iid == sid; }) &&
                    std::none_of(colonizing_ship_ids.begin(), colonizing_ship_ids.end(),
                                 [sid{s->ID()}](const auto cid) { return cid == sid; });
        });

        for (const auto* ship : scrapped_ships) {
            DebugLogger() << "... ship: " << ship->ID() << " ordered scrapped";
            const auto ship_id = ship->ID();
            const auto fleet_id = ship->FleetID();
            const auto sys_id = ship->SystemID();

            auto* system = objects.getRaw<System>(sys_id);
            if (system)
                system->Remove(ship_id);

            if (auto* fleet = objects.getRaw<Fleet>(fleet_id)) {
                fleet->RemoveShips({ship_id});
                if (fleet->Empty()) {
                    if (system)
                        system->Remove(fleet_id);
                    universe.Destroy(fleet_id, empire_ids);
                }
            }

            // record scrapping in empire stats
            auto scrapping_empire = empires.GetEmpire(ship->Owner());
            if (scrapping_empire)
                scrapping_empire->RecordShipScrapped(*ship);

            //scrapped_object_ids.push_back(ship->ID());
            universe.Destroy(ship_id, empire_ids);
        }

        auto scrapped_buildings = objects.findRaw<Building>(
            [&invaded_planet_ids, &gifted_ids](const Building* b) {
                return b->OrderedScrapped() &&
                    std::none_of(gifted_ids.begin(), gifted_ids.end(),
                                 [bid{b->ID()}](const auto gid) { return gid == bid; }) &&
                    std::none_of(invaded_planet_ids.begin(), invaded_planet_ids.end(),
                                 [pid{b->PlanetID()}](const auto iid) { return iid == pid; });
        });
        for (auto* building : scrapped_buildings) {
            if (auto* planet = objects.getRaw<Planet>(building->PlanetID()))
                planet->RemoveBuilding(building->ID());

            if (auto* system = objects.getRaw<System>(building->SystemID()))
                system->Remove(building->ID());

            // record scrapping in empire stats
            auto scrapping_empire = empires.GetEmpire(building->Owner());
            if (scrapping_empire)
                scrapping_empire->RecordBuildingScrapped(*building);

            //scrapped_object_ids.push_back(building->ID());
            universe.Destroy(building->ID(), empire_ids);
        }
    }

    /** Removes bombardment state info from objects. Actual effects of
      * bombardment are handled during */
    void CleanUpBombardmentStateInfo(ObjectMap& objects) {
        for (auto* ship : objects.allRaw<Ship>())
            ship->ClearBombardPlanet();
        for (auto* planet : objects.allRaw<Planet>()) {
            if (planet->IsAboutToBeBombarded()) {
                //DebugLogger() << "CleanUpBombardmentStateInfo: " << planet->Name() << " was about to be bombarded";
                planet->ResetIsAboutToBeBombarded();
            }
        }
    }

    /** Causes ResourceCenters (Planets) to update their focus records */
    void UpdateResourceCenterFocusHistoryInfo(ObjectMap& objects) {
        for (auto* planet : objects.allRaw<Planet>())
            planet->UpdateFocusHistory();
    }

    /** Check validity of adopted policies, overwrite initial adopted
      * policies with those currently adopted, update adopted turns counters. */
    void UpdateEmpirePolicies(EmpireManager& empires, int current_turn,
                              bool update_cumulative_adoption_time = false)
    {
        for ([[maybe_unused]] auto& [empire_id, empire] : empires) {
            (void)empire_id;    // quieting unused variable warning
            empire->UpdatePolicies(update_cumulative_adoption_time, current_turn);
        }
    }

    /** Deletes empty fleets. */
    void CleanEmptyFleets(ScriptingContext& context) {
        std::vector<Fleet*> empty_fleets;
        Universe& universe{context.ContextUniverse()};
        ObjectMap& objects{context.ContextObjects()};
        const auto& empire_ids = context.EmpireIDs();

        for (auto* fleet : objects.allRaw<Fleet>()) {
            if (fleet->Empty())
                empty_fleets.push_back(fleet);
        }

        for (auto& fleet : empty_fleets) {
            if (auto* sys = objects.getRaw<System>(fleet->SystemID()))
                sys->Remove(fleet->ID());

            universe.RecursiveDestroy(fleet->ID(), empire_ids);
        }
    }
}

void ServerApp::PreCombatProcessTurns() {
    ScopedTimer timer("ServerApp::PreCombatProcessTurns");

    m_universe.ResetAllObjectMeters(false, true);   // revert current meter values to initial values prior to update after incrementing turn number during previous post-combat turn processing.
    m_universe.UpdateEmpireVisibilityFilteredSystemGraphsWithOwnObjectMaps(m_empires);

    DebugLogger() << "ServerApp::ProcessTurns executing orders";

    // inform players of order execution
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::PROCESSING_ORDERS));

    // clear bombardment state before executing orders, so result after is only
    // determined by what orders set.
    CleanUpBombardmentStateInfo(m_universe.Objects());

    ScriptingContext context{m_universe, m_empires, m_galaxy_setup_data, m_species_manager, m_supply_manager};

    // execute orders
    for (auto& [orders_empire_id, save_game_data] : m_turn_sequence) {
        if (!save_game_data) {
            DebugLogger() << "No SaveGameData for empire " << orders_empire_id;
            continue;
        }
        if (!save_game_data->orders) {
            DebugLogger() << "No OrderSet for empire " << orders_empire_id;
            continue;
        }
        DebugLogger() << "<<= Executing Orders for empire " << orders_empire_id << " =>>";
        save_game_data->orders->ApplyOrders(context);
    }

    // clean up orders, which are no longer needed
    ClearEmpireTurnOrders();
    // TODO: CHECK THIS: was in ClearEmpireTurnOrders... needed?
    for (auto& [empire_id, empire] : m_empires) {
        (void)empire_id;
        empire->AutoTurnSetReady();
    }

    // update focus history info
    UpdateResourceCenterFocusHistoryInfo(context.ContextObjects());

    // validate adopted policies, and update Empire Policy history
    // actual policy adoption and influence consumption occurrs during order
    // execution above
    UpdateEmpirePolicies(m_empires, context.current_turn, false);

    // clean up empty fleets that empires didn't order deleted
    CleanEmptyFleets(context);

    // update production queues after order execution
    for (auto& entry : m_empires) {
        if (entry.second->Eliminated())
            continue;   // skip eliminated empires
        entry.second->UpdateProductionQueue(context);
    }

    // player notifications
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::COLONIZE_AND_SCRAP));

    DebugLogger() << "ServerApp::ProcessTurns colonization";
    auto [colonized_planet_ids, colonizing_ship_ids] = HandleColonization(context);

    DebugLogger() << "ServerApp::ProcessTurns invasion";
    auto [invaded_planet_ids, invading_ship_ids] = HandleInvasion(context);

    DebugLogger() << "ServerApp::ProcessTurns gifting";
    auto gifted_ids = HandleGifting(m_empires, m_universe.Objects(), context.current_turn,
                                    invaded_planet_ids, invading_ship_ids, colonizing_ship_ids);

    DebugLogger() << "ServerApp::ProcessTurns scrapping";
    HandleScrapping(m_universe, m_empires, invading_ship_ids, invaded_planet_ids,
                    colonizing_ship_ids, colonized_planet_ids, gifted_ids);


    DebugLogger() << "ServerApp::ProcessTurns movement";
    // player notifications
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::FLEET_MOVEMENT));

    // Update system-obstruction after orders, colonization, invasion, gifting, scrapping
    for (auto& entry : m_empires) {
        auto& empire = entry.second;
        if (empire->Eliminated())
            continue;
        empire->UpdateSupplyUnobstructedSystems(context, true);
    }


    // fleet movement
    auto fleets = m_universe.Objects().allRaw<Fleet>();
    for (auto* fleet : fleets) {
        if (fleet)
            fleet->ClearArrivalFlag();
    }
    // first move unowned fleets, or an empire fleet landing on them could wrongly
    // blockade them before they move
    for (auto* fleet : fleets) {
        if (fleet && fleet->Unowned())
            fleet->MovementPhase(context);
    }
    for (auto* fleet : fleets) {
        // save for possible SitRep generation after moving...
        if (fleet && !fleet->Unowned())
            fleet->MovementPhase(context);
    }

    // post-movement visibility update
    m_universe.UpdateEmpireObjectVisibilities(m_empires);
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns(context.current_turn);
    m_universe.UpdateEmpireStaleObjectKnowledge(m_empires);

    // SitReps for fleets having arrived at destinations
    for (auto* fleet : fleets) {
        // save for possible SitRep generation after moving...
        if (!fleet || !fleet->ArrivedThisTurn())
            continue;
        // sitreps for all empires that can see fleet at new location
        for (auto& [empire_id, empire] : m_empires) {
            if (fleet->GetVisibility(empire_id, m_universe) >= Visibility::VIS_BASIC_VISIBILITY)
                empire->AddSitRepEntry(
                    CreateFleetArrivedAtDestinationSitRep(fleet->SystemID(), fleet->ID(), empire_id, context));
        }
    }

    // indicate that the clients are waiting for their new Universes
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::DOWNLOADING));

    // send partial turn updates to all players after orders and movement
    // exclude those without empire and who are not Observer or Moderator
    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        const auto& player = *player_it;
        int empire_id = PlayerEmpireID(player->PlayerID());
        if (m_empires.GetEmpire(empire_id) ||
            player->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR ||
            player->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER)
        {
            bool use_binary_serialization = player->IsBinarySerializationUsed();
            player->SendMessage(TurnPartialUpdateMessage(PlayerEmpireID(player->PlayerID()),
                                                         m_universe, use_binary_serialization,
                                                         !player->IsLocalConnection()));
        }
    }
}

void ServerApp::ProcessCombats() {
    ScopedTimer timer("ServerApp::ProcessCombats");
    DebugLogger() << "ServerApp::ProcessCombats";
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::COMBAT));

    ScriptingContext context{m_universe, m_empires, m_galaxy_setup_data, m_species_manager, m_supply_manager};

    // collect data about locations where combat is to occur:
    // map from system ID to CombatInfo for that system
    auto combats = AssembleSystemCombatInfo(context);

    // loop through assembled combat infos, handling each combat to update the
    // various systems' CombatInfo structs
    for (CombatInfo& combat_info : combats) {
        auto combat_system = combat_info.GetSystem();
        if (combat_system)
            combat_system->SetLastTurnBattleHere(context.current_turn);

        DebugLogger(combat) << "Processing combat at " << (combat_system ? combat_system->Name() : "(No System id: " + std::to_string(combat_info.system_id) + ")");
        TraceLogger(combat) << combat_info.objects.Dump();

        AutoResolveCombat(combat_info);
    }

    BackProjectSystemCombatInfoObjectMeters(combats);

    UpdateEmpireCombatDestructionInfo(combats, context);

    DisseminateSystemCombatInfo(combats, m_universe, m_empires);
    // update visibilities with any new info gleaned during combat
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns(context.current_turn);
    // update stale object info based on any mid- combat glimpses
    // before visibility is totally recalculated in the post combat processing
    m_universe.UpdateEmpireStaleObjectKnowledge(m_empires);

    CreateCombatSitReps(combats);
}

void ServerApp::UpdateMonsterTravelRestrictions() {
    const ScriptingContext context{m_universe, m_empires, m_galaxy_setup_data,
                                   m_species_manager, m_supply_manager};

    for (auto const* system : m_universe.Objects().allRaw<System>()) {
        bool unrestricted_monsters_present = false;
        bool empires_present = false;
        bool unrestricted_empires_present = false;
        std::vector<Fleet*> monsters;
        for (auto* fleet : m_universe.Objects().findRaw<Fleet>(system->FleetIDs())) {
            // will not require visibility for empires to block clearing of monster travel restrictions
            // unrestricted lane access (i.e, (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for
            // order of arrival -- if an enemy has unrestricted lane access and you don't, they must have arrived
            // before you, or be in cahoots with someone who did.
            bool unrestricted = ((fleet->ArrivalStarlane() == system->ID())
                                 && fleet->Obstructive()
                                 && fleet->CanDamageShips(context));
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
            for (auto* monster_fleet : monsters)
                monster_fleet->SetArrivalStarlane(INVALID_OBJECT_ID);
        }

        // Break monster blockade after combat.
        if (empires_present && unrestricted_monsters_present) {
            for (auto* monster_fleet : monsters)
                monster_fleet->SetArrivalStarlane(INVALID_OBJECT_ID);
        }
    }
}

void ServerApp::PostCombatProcessTurns() {
    ScopedTimer timer("ServerApp::PostCombatProcessTurns");

    ScriptingContext context{m_universe, m_empires, m_galaxy_setup_data,
                             m_species_manager, m_supply_manager};

    // post-combat visibility update
    m_universe.UpdateEmpireObjectVisibilities(m_empires);
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns(context.current_turn);


    // check for loss of empire capitals
    for (auto& [empire_id, empire] : m_empires) {
        int capital_id = empire->CapitalID();
        if (auto capital = m_universe.Objects().get(capital_id)) {
            if (!capital->OwnedBy(empire_id))
                empire->SetCapitalID(INVALID_OBJECT_ID, m_universe.Objects());
        } else {
            empire->SetCapitalID(INVALID_OBJECT_ID, m_universe.Objects());
        }
    }
    m_empires.RefreshCapitalIDs();


    // process production and growth phase

    // notify players that production and growth is being processed
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::EMPIRE_PRODUCTION));
    DebugLogger() << "ServerApp::PostCombatProcessTurns effects and meter updates";

    TraceLogger(effects) << "!!!!!!! BEFORE TURN PROCESSING EFFECTS APPLICATION";
    TraceLogger(effects) << m_universe.Objects().Dump();

    // execute all effects and update meters prior to production, research, etc.
    if (GetGameRules().Get<bool>("RULE_RESEED_PRNG_SERVER")) {
        static boost::hash<std::string> pcpt_string_hash;
        Seed(static_cast<unsigned int>(context.current_turn) + pcpt_string_hash(m_galaxy_setup_data.seed));
    }
    m_universe.ApplyAllEffectsAndUpdateMeters(context, false);

    // regenerate system connectivity graph after executing effects, which may
    // have added or removed starlanes.
    m_universe.InitializeSystemGraph(m_empires, m_universe.Objects());
    m_universe.UpdateEmpireVisibilityFilteredSystemGraphsWithOwnObjectMaps(m_empires);

    TraceLogger(effects) << "!!!!!!! AFTER TURN PROCESSING EFFECTS APPLICATION";
    TraceLogger(effects) << m_universe.Objects().Dump();

    DebugLogger() << "ServerApp::PostCombatProcessTurns empire resources updates";

    // now that we've had combat and applied Effects, update visibilities again, prior
    //  to updating system obstructions below.
    m_universe.UpdateEmpireObjectVisibilities(m_empires);
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns(context.current_turn);

    UpdateEmpireSupply(context, m_supply_manager, false);

    // Update fleet travel restrictions (monsters and empire fleets)
    UpdateMonsterTravelRestrictions();
    for (auto& [empire_id, empire] : m_empires) {
        if (!empire->Eliminated()) {
            empire->UpdatePreservedLanes();
            empire->UpdateUnobstructedFleets(
                m_universe.Objects(), m_universe.EmpireKnownDestroyedObjectIDs(empire_id)); // must be done after *all* noneliminated empires have updated their unobstructed systems
        }
    }

    TraceLogger(effects) << "!!!!!!! AFTER UPDATING RESOURCE POOLS AND SUPPLY STUFF";
    TraceLogger(effects) << m_universe.Objects().Dump();

    DebugLogger() << "ServerApp::PostCombatProcessTurns queue progress checking";

    // Consume distributed resources to planets and on queues, create new
    // objects for completed production and give techs to empires that have
    // researched them
    for ([[maybe_unused]] auto& [empire_id, empire] : m_empires) {
        (void)empire_id;    // unused variable warning
        if (empire->Eliminated())
            continue;   // skip eliminated empires

        for (const auto& tech : empire->CheckResearchProgress(context))
            empire->AddNewlyResearchedTechToGrantAtStartOfNextTurn(tech);
        empire->CheckProductionProgress(context);
        empire->CheckInfluenceProgress();
    }

    TraceLogger(effects) << "!!!!!!! AFTER CHECKING QUEUE AND RESOURCE PROGRESS";
    TraceLogger(effects) << m_universe.Objects().Dump();

    // execute turn events implemented as Python scripts
    ExecuteScriptedTurnEvents();

    // Execute meter-related effects on objects created this turn, so that new
    // UniverseObjects will have effects applied to them this turn, allowing
    // (for example) ships to have max fuel meters greater than 0 on the turn
    // they are created.
    m_universe.ApplyMeterEffectsAndUpdateMeters(context, false);

    TraceLogger(effects) << "!!!!!!! AFTER UPDATING METERS OF ALL OBJECTS";
    TraceLogger(effects) << m_universe.Objects().Dump();

    // Planet depopulation, some in-C++ meter modifications
    for (const auto& obj : m_universe.Objects().all()) {
        obj->PopGrowthProductionResearchPhase(context);
        obj->ClampMeters();  // ensures no meters are over MAX.  probably redundant with ClampMeters() in Universe::ApplyMeterEffectsAndUpdateMeters()
    }

    TraceLogger(effects) << "!!!!!!!!!!!!!!!!!!!!!!AFTER GROWTH AND CLAMPING";
    TraceLogger(effects) << m_universe.Objects().Dump();

    // store initial values of meters for this turn.
    m_universe.BackPropagateObjectMeters();
    m_empires.BackPropagateMeters();

    // check for loss of empire capitals
    for (auto& [empire_id, empire] : m_empires) {
        int capital_id = empire->CapitalID();
        if (auto capital = m_universe.Objects().get(capital_id)) {
            if (!capital->OwnedBy(empire_id))
                empire->SetCapitalID(INVALID_OBJECT_ID, m_universe.Objects());
        } else {
            empire->SetCapitalID(INVALID_OBJECT_ID, m_universe.Objects());
        }
    }
    m_empires.RefreshCapitalIDs();


    // store any changes in objects from various progress functions
    // before updating visibility again, so that if the
    // visibility update removes an empires ability to detect an object, the
    // empire will still know the latest state on the
    // turn when the empire did have detection ability for the object
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns(context.current_turn);

    // post-production and meter-effects visibility update
    m_universe.UpdateEmpireObjectVisibilities(m_empires);

    m_universe.UpdateEmpireStaleObjectKnowledge(m_empires);

    // update empire-visibility filtered graphs after visiblity update
    m_universe.UpdateEmpireVisibilityFilteredSystemGraphsWithOwnObjectMaps(m_empires);

    TraceLogger(effects) << "!!!!!!!!!!!!!!!!!!!!!!AFTER TURN PROCESSING POP GROWTH PRODCUTION RESEARCH";
    TraceLogger(effects) << m_universe.Objects().Dump();

    // this has to be here for the sitreps it creates to be in the right turn
    CheckForEmpireElimination();

    // update current turn number so that following visibility updates and info
    // sent to players will have updated turn associated with them
    context.current_turn = ++m_current_turn;
    DebugLogger() << "ServerApp::PostCombatProcessTurns Turn number incremented to " << m_current_turn;


    // new turn visibility update
    m_universe.UpdateEmpireObjectVisibilities(m_empires);


    DebugLogger() << "ServerApp::PostCombatProcessTurns applying Newly Added Techs";
    // apply new techs
    for ([[maybe_unused]] auto& [ignored_id, empire] : m_empires) {
        (void)ignored_id;   // quiet unused variable warning
        if (empire && !empire->Eliminated()) {
            empire->ApplyNewTechs(m_universe, m_current_turn);
            empire->ApplyPolicies(m_universe, m_current_turn);
        }
    }


    // do another policy update before final meter update to be consistent with what clients calculate...
    UpdateEmpirePolicies(m_empires, context.current_turn, true);


    TraceLogger(effects) << "ServerApp::PostCombatProcessTurns Before Final Meter Estimate Update: ";
    TraceLogger(effects) << m_universe.Objects().Dump();

    // redo meter estimates to hopefully be consistent with what happens in clients
    m_universe.UpdateMeterEstimates(context, false);

    TraceLogger(effects) << "ServerApp::PostCombatProcessTurns After Final Meter Estimate Update: ";
    TraceLogger(effects) << m_universe.Objects().Dump();


    // Re-determine supply distribution and exchanging and resource pools for empires
    UpdateEmpireSupply(context, m_supply_manager, true);

    // copy latest visible gamestate to each empire's known object state
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns(context.current_turn);


    // misc. other updates and records
    m_universe.UpdateStatRecords(context);
    for ([[maybe_unused]] auto& [ignored_empire_id, empire] : m_empires) {
        (void)ignored_empire_id;    // quiet unused variable warning
        empire->UpdateOwnedObjectCounters(m_universe);
    }
    m_species_manager.UpdatePopulationCounter(m_universe.Objects());


    // indicate that the clients are waiting for their new gamestate
    m_networking.SendMessageAll(TurnProgressMessage(Message::TurnProgressPhase::DOWNLOADING));


    // compile map of PlayerInfo, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        const int player_id = player->PlayerID();
        players[player_id] = PlayerInfo{player->PlayerName(),
                                        PlayerEmpireID(player_id),
                                        player->GetClientType(),
                                        m_networking.PlayerIsHost(player_id)};
    }
    // TEST
    auto server_players = this->GetPlayerInfoMap(); 
    if (server_players.size() != players.size())
        WarnLogger() << "PostCombatProcessTurns constructed players has different size than server players";
    for (auto it1 = server_players.begin(), it2 = players.begin(); it1 != server_players.end(); ++it1, ++it2) {
        if (it1->first != it2->first)
            WarnLogger() << "PostCombatProcessTurns constructed player info id " << it1->first
                         << " differs from server info id " << it2->first;
        if (it1->second != it2->second) {
            WarnLogger() << "PostCombatProcessTurns constructed player info differs from server player info:\n" <<
                it1->second.name << " ? " << it2->second.name << "\n" <<
                it1->second.empire_id << " ? " << it2->second.empire_id << "\n" <<
                it1->second.client_type << " ? " << it2->second.client_type << "\n" <<
                it1->second.host << " ? " << it2->second.host;
        }
    }
    // END TEST

    m_universe.ObfuscateIDGenerator();

    DebugLogger() << "ServerApp::PostCombatProcessTurns Sending turn updates to players";
    // send new-turn updates to all players
    // exclude those without empire and who are not Observer or Moderator
    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        const int empire_id = PlayerEmpireID(player->PlayerID());
        const auto empire = m_empires.GetEmpire(empire_id);
        if (empire ||
            player->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR ||
            player->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER)
        {
            const bool use_binary_serialization = player->IsBinarySerializationUsed();
            player->SendMessage(TurnUpdateMessage(empire_id,                m_current_turn,
                                                  m_empires,                m_universe,
                                                  m_species_manager,        GetCombatLogManager(),
                                                  m_supply_manager,         players,
                                                  use_binary_serialization, !player->IsLocalConnection()));
        }
    }
    m_turn_expired = false;
    DebugLogger() << "ServerApp::PostCombatProcessTurns done";
}

void ServerApp::CheckForEmpireElimination() {
    std::set<std::shared_ptr<Empire>> surviving_empires;
    std::set<std::shared_ptr<Empire>> non_eliminated_non_ai_controlled_empires;
    for (auto& [empire_id, empire] : m_empires) {
        if (empire->Eliminated()) {
            continue;   // don't double-eliminate an empire
        } else if (EmpireEliminated(empire_id, m_universe.Objects())) {
            empire->Eliminate(m_empires, m_current_turn);
            RemoveEmpireTurn(empire_id);
            const int player_id = EmpirePlayerID(empire_id);
            DebugLogger() << "ServerApp::CheckForEmpireElimination empire #" << empire_id << " " << empire->Name()
                          << " of player #" << player_id << " eliminated";
            auto player_it = m_networking.GetPlayer(player_id);
            if (player_it != m_networking.established_end() &&
                (*player_it)->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            {
                auto it = m_ai_client_processes.find((*player_it)->PlayerName());
                if (it != m_ai_client_processes.end()) {
                    it->second.Kill();
                    m_ai_client_processes.erase(it);
                }
            }
        } else {
            surviving_empires.insert(empire);
            // empires could be controlled only by connected AI client, connected human client, or
            // disconnected human client.
            // Disconnected AI client controls non-eliminated empire is an error.
            if (GetEmpireClientType(empire->EmpireID()) != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
                non_eliminated_non_ai_controlled_empires.insert(empire);
        }
    }

    if (surviving_empires.size() == 1) { // last man standing
        auto& only_empire = *surviving_empires.begin();
        only_empire->Win(UserStringNop("VICTORY_ALL_ENEMIES_ELIMINATED"), m_empires.GetEmpires(), m_current_turn);
    } else if (!m_single_player_game &&
               static_cast<int>(non_eliminated_non_ai_controlled_empires.size()) <= GetGameRules().Get<int>("RULE_THRESHOLD_HUMAN_PLAYER_WIN"))
    {
        // human victory threshold
        if (GetGameRules().Get<bool>("RULE_ONLY_ALLIANCE_WIN")) {
            for (auto emp1_it = non_eliminated_non_ai_controlled_empires.begin();
                 emp1_it != non_eliminated_non_ai_controlled_empires.end(); ++emp1_it)
            {
                auto emp2_it = emp1_it;
                ++emp2_it;
                for (; emp2_it != non_eliminated_non_ai_controlled_empires.end(); ++emp2_it) {
                    const auto status = m_empires.GetDiplomaticStatus((*emp1_it)->EmpireID(), (*emp2_it)->EmpireID());
                    // if diplomacy forbidden then allow peace status
                    if (status == DiplomaticStatus::DIPLO_WAR || (GetGameRules().Get<std::string>("RULE_DIPLOMACY") != UserStringNop("RULE_DIPLOMACY_FORBIDDEN_FOR_ALL") && status == DiplomaticStatus::DIPLO_PEACE))
                        return;
                }
            }
        }

        for (auto& empire : non_eliminated_non_ai_controlled_empires)
            empire->Win(UserStringNop("VICTORY_FEW_HUMANS_ALIVE"), m_empires.GetEmpires(), m_current_turn);
    }
}

void ServerApp::HandleDiplomaticStatusChange(int empire1_id, int empire2_id) {
    DiplomaticStatus status = m_empires.GetDiplomaticStatus(empire1_id, empire2_id);
    DiplomaticStatusUpdateInfo update(empire1_id, empire2_id, status);

    for (auto player_it = m_networking.established_begin();
         player_it != m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        player->SendMessage(DiplomaticStatusMessage(update));
    }
}

void ServerApp::HandleDiplomaticMessageChange(int empire1_id, int empire2_id) {
    const DiplomaticMessage& message = m_empires.GetDiplomaticMessage(empire1_id, empire2_id);
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
