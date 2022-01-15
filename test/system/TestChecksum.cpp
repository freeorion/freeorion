#include <boost/test/unit_test.hpp>

#include "ClientAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestChecksum, ClientAppFixture)

void TestCheckSumFromEnv(const char* env, unsigned int def, unsigned int calculated) {
    bool force = false;
    unsigned int expected = def;

    if (const char *env_value = std::getenv(env)) {
        force = true;
        try {
            expected = boost::lexical_cast<unsigned int>(env_value);
        } catch (...) {
            // ignore
        }
    }
    if (force) {
        BOOST_REQUIRE_MESSAGE(calculated == expected, env << " expected " << expected << " was " << calculated);
    } else {
        BOOST_WARN_MESSAGE(calculated == expected, env << " expected " << expected << " was " << calculated);
    }
}

/**
 * - Enforces named value reference checksum test if FO_CHECKSUM_NAMED_VALUEREF is set.
 * - Enforces techs checksum test if FO_CHECKSUM_TECH is set.
 * - Setting each of these environment variables to an integer value will test that value against the corresponding parsed content checksum.
 */

BOOST_AUTO_TEST_CASE(compare_checksum) {
    auto checksums = CheckSumContent();

    TestCheckSumFromEnv("FO_CHECKSUM_NAMED_VALUEREF", 2351423, checksums["NamedValueRefManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_TECH", 5454078, checksums["TechManager"]);
}

BOOST_AUTO_TEST_SUITE_END()

