#include "ClientAppFixture.h"

#include "combat/CombatLogManager.h"
#include "client/ClientNetworking.h"
#include "universe/Species.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/i18n.h"
#include "util/Version.h"
#include "util/PythonCommon.h"
#include "parse/PythonParser.h"

#include <boost/format.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/thread/thread.hpp>
#include <boost/test/unit_test.hpp>

ClientAppFixture::ClientAppFixture() :
    m_cookie(boost::uuids::nil_uuid())
{
    InitDirs(boost::unit_test::framework::master_test_suite().argv[0]);

#ifdef FREEORION_LINUX
    // Dirty hack to output log to console.
    InitLoggingSystem("/proc/self/fd/1", "Test");
#else
    InitLoggingSystem((GetUserDataDir() / "test.log").string(), "Test");
#endif
    //InitLoggingOptionsDBSystem();

    static constexpr auto server_path_option = "misc.server-local-binary.path";
    static constexpr auto server_filename =
#ifdef FREEORION_WIN32
        "freeoriond.exe";
#else
        "freeoriond";
#endif
    if (!GetOptionsDB().OptionExists(server_path_option)) {
        auto server_path = PathToString(GetBinDir() / server_filename);
        GetOptionsDB().Add<std::string>(server_path_option, UserStringNop("OPTIONS_DB_FREEORIOND_PATH"), std::move(server_path));
    }

    InfoLogger() << FreeOrionVersionString();
    DebugLogger() << "Test client initialized";

    GetOptionsDB().Set<std::string>("resource.path", PathToString(GetBinDir() / "default"));

    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    std::thread background([this] (auto b) {
        DebugLogger() << "Started background parser thread";
        PythonCommon python;
        python.Initialize();
        StartBackgroundParsing(PythonParser(python, GetResourceDir() / "scripting"), std::move(b));
    }, std::move(barrier));
    background.detach();
    barrier_future.wait();
}

int ClientAppFixture::EffectsProcessingThreads() const
{ return 1; }

bool ClientAppFixture::PingLocalHostServer()
{ return m_networking->PingLocalHostServer(std::chrono::milliseconds(100)); }

bool ClientAppFixture::ConnectToLocalHostServer()
{ return m_networking->ConnectToLocalHostServer(); }

bool ClientAppFixture::ConnectToServer(const std::string& ip_address)
{ return m_networking->ConnectToServer(ip_address); }

void ClientAppFixture::DisconnectFromServer()
{ return m_networking->DisconnectFromServer(); }

void ClientAppFixture::HostSPGame(unsigned int num_AIs) {
    auto game_rules = GetGameRules().GetRulesAsStrings();

    SinglePlayerSetupData setup_data;
    setup_data.new_game = true;
    setup_data.filename.clear();  // not used for new game

    // GalaxySetupData
    setup_data.SetSeed("TestSeed1");
    setup_data.size =             100;
    setup_data.shape =            Shape::SPIRAL_4;
    setup_data.age =              GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    setup_data.starlane_freq =    GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    setup_data.planet_density =   GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    setup_data.specials_freq =    GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    setup_data.monster_freq =     GalaxySetupOptionMonsterFreq::MONSTER_SETUP_MEDIUM;
    setup_data.native_freq =      GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    setup_data.ai_aggr =          Aggression::MANIACAL;
    setup_data.game_rules =       game_rules;

    // SinglePlayerSetupData contains a map of PlayerSetupData, for
    // the human and AI players.  Need to compile this information
    // from the specified human options and number of requested AIs

    // Human player setup data
    PlayerSetupData human_player_setup_data;
    human_player_setup_data.player_name = "TestPlayer";
    human_player_setup_data.empire_name = "TestEmpire";
    human_player_setup_data.empire_color = {0, 255, 0, 255};

    human_player_setup_data.starting_species_name = "SP_HUMAN";
    human_player_setup_data.save_game_empire_id = ALL_EMPIRES; // not used for new games
    human_player_setup_data.client_type = Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER;

    // add to setup data players
    setup_data.players.push_back(human_player_setup_data);

    // AI player setup data.  One entry for each requested AI

    for (unsigned int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
        PlayerSetupData ai_setup_data;

        ai_setup_data.player_name = "AI_" + std::to_string(ai_i);
        ai_setup_data.empire_name.clear();                // leave blank, to be set by server in Universe::GenerateEmpires
        ai_setup_data.empire_color = {0, 0, 0, 0};        // to be set by server
        ai_setup_data.starting_species_name.clear();      // leave blank, to be set by server
        ai_setup_data.save_game_empire_id = ALL_EMPIRES;  // not used for new games
        ai_setup_data.client_type = Networking::ClientType::CLIENT_TYPE_AI_PLAYER;

        setup_data.players.push_back(ai_setup_data);
    }

    m_networking->SendMessage(HostSPGameMessage(setup_data, DependencyVersions()));
}

void ClientAppFixture::JoinGame() {
    m_lobby_updated = false;
    m_networking->SendMessage(JoinGameMessage("TestPlayer",
                                              Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER,
                                              {},
                                              m_cookie));
}

bool ClientAppFixture::ProcessMessages(const boost::posix_time::ptime& start_time, int max_seconds) {
    bool to_process = true;
    BOOST_TEST_MESSAGE("Processing Messages for at most " << max_seconds << " s");
    while ((boost::posix_time::microsec_clock::local_time() - start_time).total_seconds() < max_seconds) {
        if (!m_networking->IsConnected()) {
            ErrorLogger() << "Disconnected";
            BOOST_TEST_MESSAGE("Disconnected!");
            return false;
        }
        auto opt_msg = m_networking->GetMessage();
        if (opt_msg) {
            Message msg = *opt_msg;
            if (!HandleMessage(msg)) {
                BOOST_TEST_MESSAGE("failed to handle message!");
                return false;
            }
            to_process = true;
        } else {
            if (to_process) {
                BOOST_TEST_MESSAGE("... waiting 1000 ms for messages...");
                boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                to_process = false;
            } else {
                return true;
            }
        }
    }
    ErrorLogger() << "Timeout";
    return false; // return on timeout
}

bool ClientAppFixture::HandleMessage(Message& msg) {
    InfoLogger() << "Handle message " << msg.Type();
    BOOST_TEST_MESSAGE("Handling message: " << to_string(msg.Type()));

    switch (msg.Type()) {
    case Message::MessageType::CHECKSUM: {
        bool result = VerifyCheckSum(msg);
        if (!result) {
            ErrorLogger() << "Wrong checksum";
            BOOST_TEST_MESSAGE("Message had wrong checksum");
        }
        return result;
    }
    case Message::MessageType::SET_AUTH_ROLES:
        ExtractSetAuthorizationRolesMessage(msg, m_networking->AuthorizationRoles());
        return true;
    case Message::MessageType::HOST_SP_GAME:
        try {
            int host_id = boost::lexical_cast<int>(msg.Text());
            m_networking->SetPlayerID(host_id);
            m_networking->SetHostPlayerID(host_id);
            return true;
        } catch (const boost::bad_lexical_cast& ex) {
            ErrorLogger() << "Host id " << msg.Text() << " is not a number: " << ex.what();
            BOOST_TEST_MESSAGE("Couldn't cast host ID: " << msg.Text());
            return false;
        }
    case Message::MessageType::TURN_PROGRESS: {
        Message::TurnProgressPhase phase_id;
        ExtractTurnProgressMessageData(msg, phase_id);
        InfoLogger() << "Turn progress: " << phase_id;
        return true;
    }
    case Message::MessageType::GAME_START: {
        bool single_player_game;     // ignored
        bool loaded_game_data;       // ignored
        bool ui_data_available;      // ignored
        SaveGameUIData ui_data;      // ignored
        bool state_string_available; // ignored
        std::string save_state_string;

        ExtractGameStartMessageData(msg,                     single_player_game,     m_empire_id,
                                    m_current_turn,          m_empires,              m_universe,
                                    m_species_manager,       GetCombatLogManager(),  m_supply_manager,
                                    m_player_info,           m_orders,               loaded_game_data,
                                    ui_data_available,       ui_data,                state_string_available,
                                    save_state_string,       m_galaxy_setup_data);
        m_context.current_turn = m_current_turn;

        InfoLogger() << "Extracted GameStart message for turn: " << m_current_turn << " with empire: " << m_empire_id;

        m_ai_empires.clear();
        for (const auto& empire : m_empires) {
            if (GetEmpireClientType(empire.first) == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
                m_ai_empires.insert(empire.first);
        }
        m_ai_waiting = m_ai_empires;

        m_game_started = true;
        return true;
    }
    case Message::MessageType::DIPLOMATIC_STATUS:
    case Message::MessageType::PLAYER_CHAT:
    case Message::MessageType::CHAT_HISTORY:
    case Message::MessageType::TURN_TIMEOUT:
    case Message::MessageType::PLAYER_INFO:
        BOOST_TEST_MESSAGE("...ignored message...");
        return true; // ignore
    case Message::MessageType::PLAYER_STATUS: {
        int about_empire_id;
        Message::PlayerStatus status;
        ExtractPlayerStatusMessageData(msg, status, about_empire_id);
        SetEmpireStatus(about_empire_id, status);
        if (status == Message::PlayerStatus::WAITING)
            m_ai_waiting.erase(about_empire_id);
        BOOST_TEST_MESSAGE("Updated empire " << about_empire_id << " status: " << to_string(status));
        return true;
    }
    case Message::MessageType::TURN_PARTIAL_UPDATE: {
        ExtractTurnPartialUpdateMessageData(msg, m_empire_id, m_universe);
        BOOST_TEST_MESSAGE("Partial turn update unpacked");
        return true;
    }
    case Message::MessageType::TURN_UPDATE: {
        ExtractTurnUpdateMessageData(msg,                   m_empire_id,      m_current_turn,
                                     m_empires,             m_universe,       m_species_manager,
                                     GetCombatLogManager(), m_supply_manager, m_player_info);
        m_context.current_turn = m_current_turn;
        m_turn_done = true;
        BOOST_TEST_MESSAGE("Full turn update unpacked");
        return true;
    }
    case Message::MessageType::SAVE_GAME_COMPLETE:
        m_save_completed = true;
        return true;
    case Message::MessageType::JOIN_GAME: {
        int player_id = Networking::INVALID_PLAYER_ID;
        ExtractJoinAckMessageData(msg, player_id, m_cookie);
        m_networking->SetPlayerID(player_id);
        return true;
    }
    case Message::MessageType::HOST_ID: {
        int host_id = Networking::INVALID_PLAYER_ID;
        try {
            host_id = boost::lexical_cast<int>(msg.Text());
            m_networking->SetHostPlayerID(host_id);
            BOOST_TEST_MESSAGE("Set Host Player ID to: " << host_id);
        } catch (const boost::bad_lexical_cast&) {
            ErrorLogger() << "HOST_ID: Could not convert \"" << msg.Text() << "\" to host id";
            BOOST_TEST_MESSAGE("Couldn't get host ID: " << msg.Text());
            return false;
        }
        return true;
    }
    case Message::MessageType::LOBBY_UPDATE:
        m_lobby_updated = true;
        ExtractLobbyUpdateMessageData(msg, m_lobby_data);
        BOOST_TEST_MESSAGE("Lobby Updated");
        return true;
    case Message::MessageType::ERROR_MSG: {
            int player_id = Networking::INVALID_PLAYER_ID;
            std::string problem_key, unlocalized_info;
            bool fatal = false;
            ExtractErrorMessageData(msg, player_id, problem_key, unlocalized_info, fatal);
            ErrorLogger() << "Catch " << (fatal ? "fatal " : "") << "error " << problem_key << " from player " << player_id;
            BOOST_TEST_MESSAGE("Received " << (fatal ? "fatal " : "") << " error message: " << problem_key);
        }
        return false;
    default:
        ErrorLogger() << "Unknown message type: " << msg.Type();
        BOOST_TEST_MESSAGE("Unhandled unknown message type!");
        return false;
    }
}

void ClientAppFixture::SaveGame() {
    std::string save_filename = boost::io::str(boost::format("FreeOrionTestGame_%04d_%s%s")
                                               % m_current_turn % FilenameTimestamp() % SP_SAVE_FILE_EXTENSION);
    boost::filesystem::path save_dir_path(GetSaveDir() / "test");
    boost::filesystem::path save_path(save_dir_path / save_filename);
    if (!exists(save_dir_path))
        boost::filesystem::create_directories(save_dir_path);

    auto path_string = PathToString(save_path);
    m_save_completed = false;
    m_networking->SendMessage(HostSaveGameInitiateMessage(path_string));

}

void ClientAppFixture::UpdateLobby() {
    m_lobby_updated = false;
    m_networking->SendMessage(LobbyUpdateMessage(m_lobby_data));
}

unsigned int ClientAppFixture::GetLobbyAICount() const {
    unsigned int res = 0;
    for (const auto& plr: m_lobby_data.players) {
        if (plr.second.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            ++ res;
    }
    return res;
}

