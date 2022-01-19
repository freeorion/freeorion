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
    auto checksums = CheckSumContent();

    TestCheckSumFromEnv("FO_CHECKSUM_BUILDING", 3444689, checksums["BuildingTypeManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_ENCYCLOPEDIA", 1078929, checksums["Encyclopedia"]);
    TestCheckSumFromEnv("FO_CHECKSUM_FIELD", 3780088, checksums["FieldTypeManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_POLICY", 1239846, checksums["PolicyManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SHIP_HULL", 9937526, checksums["ShipHullManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SHIP_PART", 9683018, checksums["ShipPartManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SHIP_DESIGN", 872567, checksums["PredefinedShipDesignManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SPECIES", 2811615, checksums["SpeciesManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_SPECIALS", 6000305, checksums["SpecialsManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_TECH", 5355498, checksums["TechManager"]);
    TestCheckSumFromEnv("FO_CHECKSUM_NAMED_VALUEREF", 2465801, checksums["NamedValueRefManager"]);
}

BOOST_AUTO_TEST_SUITE_END()

