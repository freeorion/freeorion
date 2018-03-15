#include <boost/test/unit_test.hpp>

#include "ClientAppFixture.h"
#include "universe/Enums.h"
#include "util/Directories.h"
#include "util/Process.h"

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

BOOST_FIXTURE_TEST_SUITE(TestGame, ClientAppFixture)

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
    HostSPGame(num_AIs);

    BOOST_TEST_MESSAGE("Waiting game to start...");

    start_time = boost::posix_time::microsec_clock::local_time();
    while (!m_game_started) {
        BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
    }

    for (const auto& player : Players()) {
        if (player.second.client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            m_ai_players.insert(player.first);
    }

    BOOST_REQUIRE(m_ai_players.size() == num_AIs);

    m_ai_waiting = m_ai_players;

    BOOST_TEST_MESSAGE("Game started. Waiting AI for turns...");

    start_time = boost::posix_time::microsec_clock::local_time();
    while (! m_ai_waiting.empty()) {
        BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
    }

    SendTurnOrders();

    m_ai_waiting = m_ai_players;

    BOOST_TEST_MESSAGE("Turn done. Waiting AI for turns...");

    start_time = boost::posix_time::microsec_clock::local_time();
    while (! m_ai_waiting.empty()) {
        BOOST_REQUIRE(ProcessMessages(start_time, MAX_WAITING_SEC));
    }

    BOOST_TEST_MESSAGE("Terminating server...");

    BOOST_REQUIRE(server.Terminate());

    BOOST_TEST_MESSAGE("Server terminated");
}

BOOST_AUTO_TEST_SUITE_END()

