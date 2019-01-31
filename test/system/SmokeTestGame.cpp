#include <boost/test/unit_test.hpp>

#include "ClientAppFixture.h"
#include "Empire/Empire.h"
#include "universe/Enums.h"
#include "util/Directories.h"
#include "util/Process.h"
#include "util/SitRepEntry.h"

namespace {
    std::string ServerClientExe() {
#ifdef FREEORION_WIN32
        return PathToString(GetBinDir() / "freeoriond.exe");
#else
        return (GetBinDir() / "freeoriond").string();
#endif
    }

    constexpr static int MAX_WAITING_SEC = 120;
}

#ifdef FREEORION_MACOSX
#include <stdlib.h>
#endif

BOOST_FIXTURE_TEST_SUITE(SmokeTestGame, ClientAppFixture)

/**
 * - Do start a server with as a host mock player on localhost.
 * - Expect successfully connection to localhost server.
 * - Do launch singleplayer game with `FO_TEST_GAME_AIS` AIs (by default 2).
 * - Expect singleplayer game is correctly lauched.
 * - Do make empty turns until turn number reach `FO_TEST_GAME_TURNS` or mock player will be
 *   eliminated or someone will win.
 * - Expect turns're processing without error.
 * - Do save game after each turn if `FO_TEST_GAME_SAVE` was set.
 * - Expect saving is processing correctly.
 * - Do shut down server.
 * - Expect that server and AIs were shut down.
 */

BOOST_AUTO_TEST_CASE(host_server) {

    BOOST_REQUIRE(!PingLocalHostServer());

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
    args.push_back("--testing");

#ifdef FREEORION_LINUX
    // Dirty hack to output log to console.
    args.push_back("--log-file");
    args.push_back("/proc/self/fd/1");
#endif

    Process server = Process(SERVER_CLIENT_EXE, args);

    BOOST_REQUIRE(ConnectToLocalHostServer());

    BOOST_TEST_MESSAGE("Connected to server");

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));

    BOOST_TEST_MESSAGE("First messages processed. Starting game...");

    unsigned int num_AIs = 2;
    int num_turns = 2;
    bool save_game = true;

    const char *env_num_AIs = std::getenv("FO_TEST_GAME_AIS");
    if (env_num_AIs) {
        try {
            num_AIs = boost::lexical_cast<unsigned int>(env_num_AIs);
        } catch (...) {
            // ignore
        }
    }

    const char *env_num_turns = std::getenv("FO_TEST_GAME_TURNS");
    if (env_num_turns) {
        try {
            num_turns = boost::lexical_cast<int>(env_num_turns);
        } catch (...) {
            // ignore
        }
    }

    const char *env_save_game = std::getenv("FO_TEST_GAME_SAVE");
    if (env_save_game) {
        try {
            save_game = boost::lexical_cast<int>(env_save_game) != 0;
        } catch (...) {
            // ignore
        }
    }

    HostSPGame(num_AIs);

    BOOST_TEST_MESSAGE("Waiting game to start...");

    start_time = boost::posix_time::microsec_clock::local_time();
    while (!m_game_started) {
        BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
    }

    BOOST_REQUIRE_EQUAL(m_ai_empires.size(), num_AIs);

    BOOST_TEST_MESSAGE("Game started. Waiting AI for turns...");

    start_time = boost::posix_time::microsec_clock::local_time();
    while (! m_ai_waiting.empty()) {
        BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
    }

    SaveGameUIData ui_data;

    while (m_current_turn <= num_turns) {
        SendPartialOrders();

        StartTurn(ui_data);

        m_turn_done = false;
        m_ai_waiting = m_ai_empires;

        BOOST_TEST_MESSAGE("Turn done. Waiting server for update...");
        start_time = boost::posix_time::microsec_clock::local_time();
        while (!m_turn_done) {
            BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
        }

        // output sitreps
        const Empire* my_empire = m_empires.GetEmpire(m_empire_id);
        BOOST_REQUIRE(my_empire != nullptr);
        for (auto sitrep_it = my_empire->SitRepBegin(); sitrep_it != my_empire->SitRepEnd(); ++sitrep_it) {
            if (sitrep_it->GetTurn() == m_current_turn) {
                BOOST_TEST_MESSAGE("Sitrep: " << sitrep_it->Dump());
            }
        }

        if (my_empire->Eliminated()) {
            BOOST_TEST_MESSAGE("Test player empire was eliminated.");
            break;
        }
        bool have_winner = false;
        for (auto empire : m_empires) {
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
        while (! m_ai_waiting.empty()) {
            BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
        }

        if (save_game) {
            BOOST_TEST_MESSAGE("Save game");

            SaveGame();
            start_time = boost::posix_time::microsec_clock::local_time();
            while (! m_save_completed) {
                BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
            }
        }
    }

    BOOST_TEST_MESSAGE("Terminating server...");

    BOOST_REQUIRE(server.Terminate());

    BOOST_TEST_MESSAGE("Server terminated");
}

BOOST_AUTO_TEST_SUITE_END()

