#include <boost/test/unit_test.hpp>

#include "client/ClientApp.h"
#include "network/ClientNetworking.h"
#include "util/Process.h"
#include "util/Directories.h"

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
                HandleMessage(msg);
                to_process = true;
            } else {
                if(to_process) {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
                    to_process = false;
                } else {
                    return true;
                }
            }
        }
    }

    void HandleMessage(Message& msg) {
        switch (msg.Type()) {
        default:
            BOOST_TEST_MESSAGE(msg.Type());
            break;
        }
    }
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

    BOOST_TEST_MESSAGE("First messages processed");
}

BOOST_AUTO_TEST_SUITE_END()

