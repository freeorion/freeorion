#include <boost/test/unit_test.hpp>

#include "ClientAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestChecksum, ClientAppFixture)

void TestCheckSumFromEnv(const char* env, unsigned int def, unsigned int expected) {
    bool force = false;
    unsigned int value = def;

    if (const char *env_value = std::getenv(env)) {
        force = true;
        try {
            value = boost::lexical_cast<unsigned int>(env_value);
        } catch (...) {
            // ignore
        }
    }
    if (force) {
        BOOST_REQUIRE_MESSAGE(expected == value, env << " expected " << expected << " was " << value);
    } else {
        BOOST_WARN_MESSAGE(expected == value, env << " expected " << expected << " was " << value);
    }
}

/**
 * - Enforces named value reference checksum test if FO_CHECKSUM_NAMED_VALUEREF is set.
 * - Enforces techs checksum test if FO_CHECKSUM_TECH is set.
 * - Setting each of these environment variables to an integer value will test that value against the corresponding parsed content checksum.
 */

BOOST_AUTO_TEST_CASE(compare_checksum) {
    auto checksums = CheckSumContent();

    TestCheckSumFromEnv("FO_CHECKSUM_NAMED_VALUEREF", 42021, checksums["NamedValueRefManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_TECH", 9095847, checksums["TechManager"]);
}

BOOST_AUTO_TEST_SUITE_END()

