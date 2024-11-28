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
        BOOST_CHECK_MESSAGE(calculated == expected, env << " expected " << expected << " was " << calculated);
    } else {
        BOOST_WARN_MESSAGE(calculated == expected, env << " expected " << expected << " was " << calculated);
    }
}

/**
 * - Enforces buildings checksum test if FO_CHECKSUM_BUILDING is set.
 * - Enforces encyclopedia checksum test if FO_CHECKSUM_ENCYCLOPEDIA is set.
 * - Enforces fields checksum test if FO_CHECKSUM_FIELD is set.
 * - Enforces policies checksum test if FO_CHECKSUM_POLICY is set.
 * - Enforces ship hulls checksum test if FO_CHECKSUM_SHIP_HULL is set.
 * - Enforces ship parts checksum test if FO_CHECKSUM_SHIP_PART is set.
 * - Enforces predefined ship designs checksum test if FO_CHECKSUM_SHIP_DESIGN is set.
 * - Enforces species checksum test if FO_CHECKSUM_SPECIES is set.
 * - Enforces specials checksum test if FO_CHECKSUM_SPECIALS is set.
 * - Enforces techs checksum test if FO_CHECKSUM_TECH is set.
 * - Enforces named value reference checksum test if FO_CHECKSUM_NAMED_VALUEREF is set.
 * - Setting each of these environment variables to an integer value will test that value against the corresponding parsed content checksum.
 */

BOOST_AUTO_TEST_CASE(compare_checksum) {
    auto checksums = CheckSumContent(m_context.species);

    TestCheckSumFromEnv("FO_CHECKSUM_BUILDING", 6401719, checksums["BuildingTypeManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_ENCYCLOPEDIA", 1125744, checksums["Encyclopedia"]);
    TestCheckSumFromEnv("FO_CHECKSUM_FIELD", 5633696, checksums["FieldTypeManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_POLICY", 5356786, checksums["PolicyManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SHIP_HULL", 8986067, checksums["ShipHullManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SHIP_PART", 3729274, checksums["ShipPartManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SHIP_DESIGN", 878449, checksums["PredefinedShipDesignManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SPECIES", 4345855, checksums["SpeciesManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SPECIALS", 3672180, checksums["SpecialsManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_TECH", 1230631, checksums["TechManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_NAMED_VALUEREF", 1833808, checksums["NamedValueRefManager"]);
}

BOOST_AUTO_TEST_SUITE_END()

