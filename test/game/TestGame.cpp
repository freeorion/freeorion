#include <boost/test/unit_test.hpp>

#include "client/ClientApp.h"
#include "network/ClientNetworking.h"

struct GameFixture : public ClientApp {
    int EffectsProcessingThreads() const
    { return GetOptionsDB().Get<int>("effects.ai.threads"); }
};

BOOST_FIXTURE_TEST_SUITE(TestGame, GameFixture)

BOOST_AUTO_TEST_CASE(host_server) {
    if (m_networking->PingLocalHostServer(std::chrono::milliseconds(100)))
        BOOST_FAIL("Local server already running");

}

BOOST_AUTO_TEST_SUITE_END()

