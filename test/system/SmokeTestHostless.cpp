#include <boost/test/unit_test.hpp>

#include "ClientAppFixture.h"
#include "Empire/Empire.h"
#include "universe/Planet.h"
#include "util/Directories.h"
#include "util/Process.h"
#include "util/SitRepEntry.h"

#ifdef FREEORION_MACOSX
#include <stdlib.h>
#endif

BOOST_FIXTURE_TEST_SUITE(SmokeTestHostless, ClientAppFixture)

/**
 * - Do start a server with hostless mode if `FO_TEST_HOSTLESS_LAUNCH_SERVER` was set with save
 *   enabled if `FO_TEST_HOSTLESS_SAVE` was set.
 * - Do connect to lobby to localhost server as a Player.
 * - Expect successfully connection to localhost server.
 * - Do add `FO_TEST_HOSTLESS_AIS` AIs to lobby (by default 2).
 * - Expect AIs will be added successfully.
 * - Do make player ready.
 * - Expect game started.
 * - Do make empty turns until turn number reach `FO_TEST_HOSTLESS_TURNS` or mock player will be
 *   eliminated or someone will win.
 * - Expect turns're processing without error.
 * - Do disconnect from server.
 * - Do reconnect to server until number of played games reach `FO_TEST_HOSTLESS_GAMES`.
 * - Expect got to lobby.
 * - Expect number of AIs was preserved.
 * - Do repeat game steps.
 * - Expect all games processed correctly.
 * - Do shut down server.
 * - Expect that serve and AIs were shut down.
 */

BOOST_AUTO_TEST_CASE(hostless_server) {
    unsigned int num_AIs = 2;
    unsigned int num_games = 3;
    int num_turns = 3;
    bool save_game = true;
    bool launch_server = true;

    const char *env_num_AIs = std::getenv("FO_TEST_HOSTLESS_AIS");
    if (env_num_AIs) {
        try {
            num_AIs = boost::lexical_cast<unsigned int>(env_num_AIs);
        } catch (...) {
            // ignore
        }
    }

    const char *env_num_turns = std::getenv("FO_TEST_HOSTLESS_TURNS");
    if (env_num_turns) {
        try {
            num_turns = boost::lexical_cast<int>(env_num_turns);
        } catch (...) {
            // ignore
        }
    }

    const char *env_save_game = std::getenv("FO_TEST_HOSTLESS_SAVE");
    if (env_save_game) {
        try {
            save_game = boost::lexical_cast<int>(env_save_game) != 0;
        } catch (...) {
            // ignore
        }
    }

    const char *env_launch_server = std::getenv("FO_TEST_HOSTLESS_LAUNCH_SERVER");
    if (env_launch_server) {
        try {
            launch_server = boost::lexical_cast<int>(env_launch_server) != 0;
        } catch (...) {
            // ignore
        }
    }

    const char *env_games = std::getenv("FO_TEST_HOSTLESS_GAMES");
    if (env_games) {
        try {
            num_games = boost::lexical_cast<unsigned int>(env_games);
        } catch (...) {
            // ignore
        }
    }

    boost::optional<Process> server;
    if (launch_server) {
        BOOST_REQUIRE(!PingLocalHostServer());

        std::string SERVER_CLIENT_EXE = GetOptionsDB().Get<std::string>("misc.server-local-binary.path");

        BOOST_TEST_MESSAGE(SERVER_CLIENT_EXE);

#ifdef FREEORION_MACOSX
        // On OSX set environment variable DYLD_LIBRARY_PATH to python framework folder
        // bundled with app, so the dynamic linker uses the bundled python library.
        // Otherwise the dynamic linker will look for a correct python lib in system
        // paths, and if it can't find it, throw an error and terminate!
        // Setting environment variable here, spawned child processes will inherit it.
        setenv("DYLD_LIBRARY_PATH", GetPythonHome().string().c_str(), 1);
#endif

        std::vector<std::string> args {
            "\"" + SERVER_CLIENT_EXE + "\"",
            "--hostless",
            "--save.auto.hostless.enabled", save_game ? "1" : "0",
            "--setup.ai.player.count", "0",
            "--testing",
            "--log-level", "info",
            "--resource.path", GetOptionsDB().Get<std::string>("resource.path")
        };

#ifdef FREEORION_LINUX
        // Dirty hack to output log to console.
        args.push_back("--log-file");
        args.push_back("/proc/self/fd/1");
#endif

        server = Process(SERVER_CLIENT_EXE, args);

        BOOST_TEST_MESSAGE("Server started.");
    }

    for (unsigned int g = 0; g < num_games; ++g) {
        m_game_started = false;
        BOOST_TEST_MESSAGE("Game " << g << ". Connecting to server...");

        BOOST_REQUIRE(ConnectToServer("localhost"));

        BOOST_TEST_MESSAGE("Joining game...");
        JoinGame();
        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
        start_time = boost::posix_time::microsec_clock::local_time();
        BOOST_TEST_MESSAGE("Waiting for lobby update...");
        while (!m_lobby_updated) {
            BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
        }
        BOOST_TEST_MESSAGE("Entered to lobby");

        if (g == 0) {
            // if first game lobby should be empty
            BOOST_REQUIRE_EQUAL(GetLobbyAICount(), 0);

            BOOST_TEST_MESSAGE("Filling lobby with " << num_AIs << "AIs.");
            // fill lobby with AIs
            for (unsigned int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
                PlayerSetupData ai_plr;
                ai_plr.client_type = Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
                m_lobby_data.players.emplace_back(Networking::INVALID_PLAYER_ID, ai_plr);
                // publish changes
                UpdateLobby();
                start_time = boost::posix_time::microsec_clock::local_time();
                BOOST_TEST_MESSAGE("Waiting for lobby update after adding AI " << ai_i);
                while (!m_lobby_updated) {
                    BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
                }
            }
            BOOST_TEST_MESSAGE("Lobby filling done.");
        }
        // after filling first game there should be corrent number of AIs
        // and other game should retain number of AIs from previous game
        BOOST_REQUIRE_EQUAL(GetLobbyAICount(), num_AIs);

        // get ready
        for (auto& plr : m_lobby_data.players) {
            if (plr.first == PlayerID())
                plr.second.player_ready = true;
        }
        UpdateLobby();

        start_time = boost::posix_time::microsec_clock::local_time();
        while (!m_game_started) {
            BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
        }

        BOOST_REQUIRE_EQUAL(m_ai_empires.size(), num_AIs);

        start_time = boost::posix_time::microsec_clock::local_time();
        while (!m_ai_waiting.empty()) {
            BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
        }

        SaveGameUIData ui_data;

        while (m_current_turn <= num_turns) {
            SendPartialOrders();

            StartTurn(ui_data);

            m_turn_done = false;
            m_ai_waiting = m_ai_empires;

            BOOST_TEST_MESSAGE("Turn done. Waiting server for update... " << m_current_turn);
            start_time = boost::posix_time::microsec_clock::local_time();
            while (!m_turn_done) {
                BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
            }

            BOOST_TEST_MESSAGE("Turn updated " << m_current_turn);

            // output sitreps
            const auto my_empire = m_empires.GetEmpire(m_empire_id);
            BOOST_REQUIRE(!!my_empire);
            for (const auto& sitrep : my_empire->SitReps()) {
                if (sitrep.GetTurn() == m_current_turn) {
                    BOOST_TEST_MESSAGE("Sitrep: " << sitrep.Dump());
                }
            }

            if (m_current_turn == 2) {
                // check home planet meters
                bool found_planet = false;

                auto is_owned = [this](const UniverseObject* obj) { return obj->OwnedBy(m_empire_id); };

                for (const auto* planet : m_universe.Objects().findRaw<const Planet>(is_owned)) {
                    BOOST_REQUIRE_LT(0.0, planet->GetMeter(MeterType::METER_POPULATION)->Current());
                    BOOST_TEST_MESSAGE("Population: " << planet->GetMeter(MeterType::METER_POPULATION)->Current());
                    BOOST_REQUIRE_LT(0.0, planet->GetMeter(MeterType::METER_INDUSTRY)->Current());
                    BOOST_TEST_MESSAGE("Industry: " << planet->GetMeter(MeterType::METER_INDUSTRY)->Current());
                    BOOST_REQUIRE_LT(0.0, planet->GetMeter(MeterType::METER_RESEARCH)->Current());
                    BOOST_TEST_MESSAGE("Research: " << planet->GetMeter(MeterType::METER_RESEARCH)->Current());
                    found_planet = true;
                }
                BOOST_REQUIRE(found_planet);
            }

            if (my_empire->Eliminated()) {
                BOOST_TEST_MESSAGE("Test player empire was eliminated.");
                break;
            }
            bool have_winner = false;
            for (auto& empire : m_empires) {
                if (empire.second->Won()) {
                    have_winner = true;
                    break;
                }
            }
            if (have_winner) {
                BOOST_TEST_MESSAGE("Someone wins game.");
                break;
            }

            BOOST_TEST_MESSAGE("Waiting AI for turns...");
            start_time = boost::posix_time::microsec_clock::local_time();
            while (!m_ai_waiting.empty()) {
                BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
            }
        }

        DisconnectFromServer();

        start_time = boost::posix_time::microsec_clock::local_time();
        while (ProcessMessages(start_time, MAX_WAITING_SEC));
    }

    if (launch_server && server) {
        BOOST_WARN(server->Terminate());
    }
}

BOOST_AUTO_TEST_SUITE_END()

