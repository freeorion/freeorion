#include <boost/test/unit_test.hpp>

#include "parse/Parse.h"
#include "parse/PythonParser.h"
#include "universe/BuildingType.h"
#include "universe/Conditions.h"
#include "universe/Effects.h"
#include "universe/Planet.h"
#include "universe/Tech.h"
#include "universe/UnlockableItem.h"
#include "universe/ValueRefs.h"
#include "universe/NamedValueRefManager.h"
#include "util/i18n.h"
#include "util/CheckSums.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/Pending.h"
#include "util/PythonCommon.h"

#include "ParserAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestDefaultPythonParser, ParserAppFixture)

/**
 * Checks count of techs and tech categories in real scripts
 * FO_CHECKSUM_TECH_NAME determines tech name to be check for FO_CHECKSUM_TECH_VALUE checksum
 */

BOOST_AUTO_TEST_CASE(parse_techs_full) {
    PythonParser parser(m_python, m_default_scripting_dir);

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, m_default_scripting_dir / "macros");

    auto techs_p = Pending::ParseSynchronously(parse::techs<TechManager::TechParseTuple>, parser, m_default_scripting_dir / "techs");
    auto [techs, tech_categories, categories_seen] = *Pending::WaitForPendingUnlocked(std::move(techs_p));

    BOOST_CHECK(!techs.empty());
    BOOST_CHECK(!tech_categories.empty());
    BOOST_CHECK(!categories_seen.empty());

    BOOST_REQUIRE_EQUAL(209, techs.size());
    BOOST_REQUIRE_EQUAL(9, tech_categories.size());
    BOOST_REQUIRE_EQUAL(9, categories_seen.size());

    if (const char* tech_name = std::getenv("FO_CHECKSUM_TECH_NAME")) {
        const auto tech_it = techs.find(tech_name);
        BOOST_REQUIRE(techs.end() != tech_it);
        BOOST_REQUIRE_EQUAL(tech_name, tech_it->second.Name());

        BOOST_TEST_MESSAGE("Dump " << tech_name << ":");
        BOOST_TEST_MESSAGE(tech_it->second.Dump(0));

        if (const char *tech_checksum_str = std::getenv("FO_CHECKSUM_TECH_VALUE")) {
            unsigned int tech_checksum = boost::lexical_cast<unsigned int>(tech_checksum_str);
            unsigned int value{0};
            CheckSums::CheckSumCombine(value, *tech_it);
            BOOST_REQUIRE_EQUAL(tech_checksum, value);
        }
    }
}

/**
 * Checks count of species and species census ordering in real scripts
 * FO_CHECKSUM_SPECIES_NAME determines species name to be check for FO_CHECKSUM_SPECIES_VALUE checksum
 */

BOOST_AUTO_TEST_CASE(parse_species_full) {
    PythonParser parser(m_python, m_default_scripting_dir);

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, m_default_scripting_dir / "macros");

    auto species_p = Pending::ParseSynchronously(parse::species, parser, m_default_scripting_dir / "species");
    const auto [species, ordering] = *Pending::WaitForPendingUnlocked(std::move(species_p));

    BOOST_CHECK(!ordering.empty());
    BOOST_CHECK(!species.empty());

    BOOST_REQUIRE_EQUAL(7, ordering.size());
    BOOST_REQUIRE_EQUAL(50, species.size());

    for (const auto& s : species) {
        for (const auto& effects : s.second.Effects()) {
            BOOST_REQUIRE_MESSAGE(effects.Scope(), s.second.Name());
        }
        BOOST_CHECK_MESSAGE(UserStringExists(s.second.Name()), s.second.Name());
        for (const auto& l : s.second.Likes()) {
            BOOST_CHECK_MESSAGE(UserStringExists(l), l);
        }
        for (const auto& l : s.second.Dislikes()) {
            BOOST_CHECK_MESSAGE(UserStringExists(l), l);
        }
    }

    if (const char *species_name = std::getenv("FO_CHECKSUM_SPECIES_NAME")) {
        const auto species_it = species.find(species_name);
        BOOST_REQUIRE(species.end() != species_it);
        BOOST_REQUIRE_EQUAL(species_name, species_it->second.Name());

        BOOST_TEST_MESSAGE("Dump " << species_name << ":");
        BOOST_TEST_MESSAGE(species_it->second.Dump(0));

        if (const char *species_checksum_str = std::getenv("FO_CHECKSUM_SPECIES_VALUE")) {
            uint32_t species_checksum = boost::lexical_cast<uint32_t>(species_checksum_str);
            uint32_t value{0};
            CheckSums::CheckSumCombine(value, species_it->second);
            BOOST_REQUIRE_EQUAL(species_checksum, value);
        }
    }
}

/**
 * Checks count of buildings in real scripts
 * FO_CHECKSUM_BUILDINGS_NAME determines building name to be check for FO_CHECKSUM_BUILDINGS_VALUE checksum
 */

BOOST_AUTO_TEST_CASE(parse_buildings_full) {
    PythonParser parser(m_python, m_default_scripting_dir);

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, m_default_scripting_dir / "macros");

    auto buildings_p = Pending::ParseSynchronously(parse::buildings, parser, m_default_scripting_dir / "buildings");
    const auto buildings = *Pending::WaitForPendingUnlocked(std::move(buildings_p));

    BOOST_CHECK(!buildings.empty());

    BOOST_REQUIRE_EQUAL(108, buildings.size());

    if (const char *buildings_name = std::getenv("FO_CHECKSUM_BUILDINGS_NAME")) {
        const auto buildings_it = buildings.find(buildings_name);
        BOOST_REQUIRE(buildings.end() != buildings_it);
        BOOST_REQUIRE_EQUAL(buildings_name, buildings_it->second->Name());

        BOOST_TEST_MESSAGE("Dump " << buildings_name << ":");
        BOOST_TEST_MESSAGE(buildings_it->second->Dump(0));

        if (const char *buildings_checksum_str = std::getenv("FO_CHECKSUM_BUILDINGS_VALUE")) {
            uint32_t buildings_checksum = boost::lexical_cast<uint32_t>(buildings_checksum_str);
            uint32_t value{0};
            CheckSums::CheckSumCombine(value, buildings_it->second);
            BOOST_REQUIRE_EQUAL(buildings_checksum, value);
        }
    }
}

/**
 * Checks count of empire statistics in real scripts
 * FO_CHECKSUM_EMPIRE_STATISTIC_NAME determines building name to be check for FO_CHECKSUM_EMPIRE_STATISTIC_VALUE checksum
 */

BOOST_AUTO_TEST_CASE(parse_empire_statistics_full) {
    PythonParser parser(m_python, m_default_scripting_dir);

    auto named_values = Pending::ParseSynchronously(parse::named_value_refs, m_default_scripting_dir / "macros");

    auto empire_statistics_p = Pending::ParseSynchronously(parse::statistics, parser, m_default_scripting_dir / "empire_statistics");

    const auto empire_statistics = *Pending::WaitForPendingUnlocked(std::move(empire_statistics_p));

    BOOST_CHECK(!empire_statistics.empty());

    BOOST_REQUIRE_EQUAL(22, empire_statistics.size());

    if (const char *empire_statistic_name = std::getenv("FO_CHECKSUM_EMPIRE_STATISTIC_NAME")) {
        const auto empire_statistic_it = empire_statistics.find(empire_statistic_name);
        BOOST_REQUIRE(empire_statistics.end() != empire_statistic_it);

        BOOST_TEST_MESSAGE("Dump " << empire_statistic_name << ":");
        BOOST_TEST_MESSAGE(empire_statistic_it->second->Dump(0));

        if (const char *empire_statistic_checksum_str = std::getenv("FO_CHECKSUM_EMPIRE_STATISTIC_VALUE")) {
            uint32_t empire_statistic_checksum = boost::lexical_cast<uint32_t>(empire_statistic_checksum_str);
            uint32_t value{0};
            CheckSums::CheckSumCombine(value, empire_statistic_it->second);
            BOOST_REQUIRE_EQUAL(empire_statistic_checksum, value);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

