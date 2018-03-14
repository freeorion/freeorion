#include <boost/test/unit_test.hpp>

#include "client/ClientApp.h"
#include "network/ClientNetworking.h"
#include "universe/Enums.h"
#include "universe/Species.h"
#include "combat/CombatLogManager.h"
#include "util/GameRules.h"
#include "util/Directories.h"
#include "util/Process.h"
#include "util/Version.h"
#include "GG/GG/ClrConstants.h"

namespace {
    std::string ServerClientExe() {
#ifdef FREEORION_WIN32
        return PathToString(GetBinDir() / "freeoriond.exe");
#else
        return (GetBinDir() / "freeoriond").string();
#endif
    }
}

#ifdef FREEORION_MACOSX
#include <stdlib.h>
#endif

struct GameFixture : public ClientApp {

    GameFixture() :
        m_game_started(false)
    {
#ifdef FREEORION_LINUX
        // Dirty hack to output log to console.
        InitLoggingSystem("/proc/self/fd/1", "Test");
#else
        InitLoggingSystem((GetUserDataDir() / "test.log").string(), "Test");
#endif
        //InitLoggingOptionsDBSystem();

        InfoLogger() << FreeOrionVersionString();
        DebugLogger() << "Test client initialized";

        StartBackgroundParsing();
    }

    int EffectsProcessingThreads() const
    { return GetOptionsDB().Get<int>("effects.ai.threads"); }

    bool ProcessMessages() {
        bool to_process = true;
        while (1) {
            if (!m_networking->IsConnected())
                return false;
            auto opt_msg = m_networking->GetMessage();
            if (opt_msg) {
                Message msg = *opt_msg;
                if (! HandleMessage(msg)) {
                    return false;
                }
                to_process = true;
            } else {
                if(to_process) {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                    to_process = false;
                } else {
                    return true;
                }
            }
        }
    }

    bool HandleMessage(Message& msg) {
        InfoLogger() << "Handle message " << msg.Type();
        switch (msg.Type()) {
        case Message::CHECKSUM: {
            bool result = VerifyCheckSum(msg);
            if (!result)
                ErrorLogger() << "Wrong checksum";
            //return result;
            return true;
        }
        case Message::SET_AUTH_ROLES:
            ExtractSetAuthorizationRolesMessage(msg, m_networking->AuthorizationRoles());
            return true;
        case Message::HOST_SP_GAME:
            try {
                int host_id = boost::lexical_cast<int>(msg.Text());
                m_networking->SetPlayerID(host_id);
                m_networking->SetHostPlayerID(host_id);
                return true;
            } catch (const boost::bad_lexical_cast& ex) {
                ErrorLogger() << "Host id " << msg.Text() << " is not a number: " << ex.what();
                return false;
            }
        case Message::TURN_PROGRESS: {
            Message::TurnProgressPhase phase_id;
            ExtractTurnProgressMessageData(msg, phase_id);
            InfoLogger() << "Turn progress: " << phase_id;
            return true;
        }
        case Message::GAME_START: {
            bool single_player_game;     // ignored
            bool loaded_game_data;       // ignored
            bool ui_data_available;      // ignored
            SaveGameUIData ui_data;      // ignored
            bool state_string_available; // ignored
            std::string save_state_string;
            m_player_status.clear();

            ExtractGameStartMessageData(msg,                     single_player_game,     m_empire_id,
                                        m_current_turn,          m_empires,              m_universe,
                                        GetSpeciesManager(),     GetCombatLogManager(),  GetSupplyManager(),
                                        m_player_info,           m_orders,               loaded_game_data,
                                        ui_data_available,       ui_data,                state_string_available,
                                        save_state_string,       m_galaxy_setup_data);

            InfoLogger() << "Extracted GameStart message for turn: " << m_current_turn << " with empire: " << m_empire_id;
            m_game_started = true;
            return true;
        }
        case Message::DIPLOMATIC_STATUS:
        case Message::PLAYER_CHAT:
            return true; // ignore
        case Message::PLAYER_STATUS: {
            int about_player_id;
            Message::PlayerStatus status;
            ExtractPlayerStatusMessageData(msg, about_player_id, status);

            if (status == Message::WAITING) {
                m_ai_waiting.erase(about_player_id);
            }
            return true;
        }
        case Message::TURN_PARTIAL_UPDATE: {
            ExtractTurnPartialUpdateMessageData(msg, EmpireID(), GetUniverse());
            return true;
        }
        case Message::TURN_UPDATE: {
            int current_turn = INVALID_GAME_TURN;
            ExtractTurnUpdateMessageData(msg,                   EmpireID(),         current_turn,
                                         Empires(),             GetUniverse(),      GetSpeciesManager(),
                                         GetCombatLogManager(), GetSupplyManager(), Players());
            return true;
        }
        default:
            ErrorLogger() << "Unknown message type: " << msg.Type();
            return false;
        }
    }

    bool          m_game_started; ///< Is server started the game?
    std::set<int> m_ai_players; ///< Ids of AI players in game.
    std::set<int> m_ai_waiting; ///< Ids of AI players not yet send orders.
};

BOOST_FIXTURE_TEST_SUITE(TestGame, GameFixture)

BOOST_AUTO_TEST_CASE(host_server) {
    if (m_networking->PingLocalHostServer(std::chrono::milliseconds(100)))
        BOOST_FAIL("Local server already running");

    std::string SERVER_CLIENT_EXE = ServerClientExe();

    BOOST_TEST_MESSAGE(SERVER_CLIENT_EXE);

#ifdef FREEORION_MACOSX
    // On OSX set environment variable DYLD_LIBRARY_PATH to python framework folder
    // bundled with app, so the dynamic linker uses the bundled python library.
    // Otherwise the dynamic linker will look for a correct python lib in system
    // paths, and if it can't find it, throw an error and terminate!
    // Setting environment variable here, spawned child processes will inherit it.
    setenv("DYLD_LIBRARY_PATH", GetPythonHome().string().c_str(), 1);
#endif

    std::vector<std::string> args;
    args.push_back("\"" + SERVER_CLIENT_EXE + "\"");
    args.push_back("--singleplayer");

#ifdef FREEORION_LINUX
    // Dirty hack to output log to console.
    args.push_back("--log-file");
    args.push_back("/proc/self/fd/1");
#endif

    Process server = Process(SERVER_CLIENT_EXE, args);

    BOOST_REQUIRE(m_networking->ConnectToLocalHostServer());

    BOOST_TEST_MESSAGE("Connected to server");

    BOOST_REQUIRE(ProcessMessages());

    BOOST_TEST_MESSAGE("First messages processed. Starting game...");

    std::vector<std::pair<std::string, std::string>> game_rules = GetGameRules().GetRulesAsStrings();

    SinglePlayerSetupData setup_data;
    setup_data.m_new_game = true;
    setup_data.m_filename.clear();  // not used for new game

    // get values stored in options from previous time game was run or
    // from just having run GalaxySetupWnd

    // GalaxySetupData
    setup_data.SetSeed("TestSeed1");
    setup_data.m_size =             100;
    setup_data.m_shape =            Shape::SPIRAL_4;
    setup_data.m_age =              GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_starlane_freq =    GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_planet_density =   GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_specials_freq =    GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_monster_freq =     GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_native_freq =      GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_ai_aggr =          Aggression::MANIACAL;
    setup_data.m_game_rules =       game_rules;

    // SinglePlayerSetupData contains a map of PlayerSetupData, for
    // the human and AI players.  Need to compile this information
    // from the specified human options and number of requested AIs

    // Human player setup data
    PlayerSetupData human_player_setup_data;
    human_player_setup_data.m_player_name = "TestPlayer";
    human_player_setup_data.m_empire_name = "TestEmpire";
    human_player_setup_data.m_empire_color = GG::CLR_GREEN;

    human_player_setup_data.m_starting_species_name = "SP_HUMAN";
    human_player_setup_data.m_save_game_empire_id = ALL_EMPIRES; // not used for new games
    human_player_setup_data.m_client_type = Networking::CLIENT_TYPE_HUMAN_PLAYER;

    // add to setup data players
    setup_data.m_players.push_back(human_player_setup_data);

    // AI player setup data.  One entry for each requested AI
    unsigned int num_AIs = 2;
    for (unsigned int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
        PlayerSetupData ai_setup_data;

        ai_setup_data.m_player_name = "AI_" + std::to_string(ai_i);
        ai_setup_data.m_empire_name.clear();                // leave blank, to be set by server in Universe::GenerateEmpires
        ai_setup_data.m_empire_color = GG::CLR_ZERO;        // to be set by server
        ai_setup_data.m_starting_species_name.clear();      // leave blank, to be set by server
        ai_setup_data.m_save_game_empire_id = ALL_EMPIRES;  // not used for new games
        ai_setup_data.m_client_type = Networking::CLIENT_TYPE_AI_PLAYER;

        setup_data.m_players.push_back(ai_setup_data);
    }

    m_networking->SendMessage(HostSPGameMessage(setup_data));

    BOOST_TEST_MESSAGE("Waiting game to start...");

    while (!m_game_started) {
        BOOST_REQUIRE(ProcessMessages());
        BOOST_TEST_MESSAGE("Processed messages");
    }

    for (const auto& player : Players()) {
        if (player.second.client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            m_ai_players.insert(player.first);
    }

    BOOST_REQUIRE(m_ai_players.size() == num_AIs);

    m_ai_waiting = m_ai_players;

    BOOST_TEST_MESSAGE("Game started. Waiting AI for turns...");

    while (! m_ai_waiting.empty()) {
        BOOST_REQUIRE(ProcessMessages());
        BOOST_TEST_MESSAGE("Processed messages");
    }

    m_networking->SendMessage(TurnOrdersMessage(OrderSet()));

    m_ai_waiting = m_ai_players;

    BOOST_TEST_MESSAGE("Turn done. Waiting AI for turns...");

    while (! m_ai_waiting.empty()) {
        BOOST_REQUIRE(ProcessMessages());
        BOOST_TEST_MESSAGE("Processed messages");
    }

    BOOST_TEST_MESSAGE("Terminating server...");

    BOOST_REQUIRE(server.Terminate());

    BOOST_TEST_MESSAGE("Server terminated");
}

BOOST_AUTO_TEST_SUITE_END()

