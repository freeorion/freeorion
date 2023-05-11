#include <boost/test/unit_test.hpp>

#include "parse/EnumParser.h"
#include "parse/ValueRefParser.h"
#include "parse/EnumValueRefRules.h"
#include "parse/ConditionParserImpl.h"
#include "universe/BuildingType.h"
#include "universe/Enums.h"
#include "universe/Planet.h"
#include "universe/ShipHull.h"
#include "universe/ShipPart.h"
#include "universe/Species.h"
#include "universe/System.h"
#include "universe/UnlockableItem.h"

#include <type_traits>

namespace {
    // File scope lexer since: 1) initializing the lexer deterministic finite
    // automaton (DFA) is slow; 2) the tests are single threaded and
    // non-concurrent.
    const parse::lexer lexer;
}

struct EnumParserFixture {
    template <typename Grammar> requires (std::is_constructible_v<Grammar, const parse::lexer&>)
    Grammar& make_grammar(const parse::lexer& lexer)
    {
        static Grammar grammar(lexer);
        return grammar;
    }

    template <typename Grammar>
        requires (std::is_constructible_v<Grammar, const parse::lexer&, parse::detail::Labeller&>)
    Grammar& make_grammar(const parse::lexer& lexer)
    {
        parse::detail::Labeller label;
        static Grammar grammar(lexer, label);
        return grammar;
    }

    template <typename Grammar>
        requires (std::is_constructible_v<Grammar,
                                          const parse::lexer&, parse::detail::Labeller&,
                                          const parse::detail::condition_parser_grammar&>)
    Grammar& make_grammar(const parse::lexer& lexer)
    {
        parse::detail::Labeller label;
        parse::conditions_parser_grammar cond(lexer, label);
        static Grammar grammar(lexer, label, cond);
        return grammar;
    }

    template <typename Type, typename Grammar>
    bool parse_core(std::string phrase, Type& result, const parse::lexer& lexer, Grammar& grammar) {
        boost::spirit::qi::in_state_type in_state;
        boost::spirit::qi::eoi_type eoi;
        boost::spirit::qi::_1_type _1;

        std::string::const_iterator begin_phrase = phrase.begin();
        std::string::const_iterator end_phrase = phrase.end();

        auto begin = lexer.begin(begin_phrase, end_phrase);
        auto end   = lexer.end();

        bool matched = boost::spirit::qi::phrase_parse(
            begin, end,
            grammar[boost::phoenix::ref(result) = _1] > eoi,
            in_state("WS")[lexer.self]
        );

        return matched && begin == end;
    }

    template <typename Type, typename Grammar>
    bool parse(std::string phrase, Type& result) {
        Grammar& grammar = make_grammar<Grammar>(lexer);
        return parse_core(phrase, result, lexer, grammar);
    }

    template <typename Type, typename Rules>
    bool parse_enum_expr(std::string phrase, Type& result) {
        Rules& rules = make_grammar<Rules>(lexer);
        return parse_core(phrase, result, lexer, rules.enum_expr);
    }
};

BOOST_FIXTURE_TEST_SUITE(TestEnumParser, EnumParserFixture)

#define CHECK_ENUM_AND_RESULT(string, expected, result_type, grammar_type) \
    {                                                                   \
        result_type result;                                             \
        auto pass = parse<result_type, grammar_type>(string, result);   \
        BOOST_CHECK(pass);                                              \
        BOOST_CHECK(result == expected);                                \
    }

#define CHECK_ENUM_EXPR_AND_RESULT(string, expected, result_type, grammar_type) \
    {                                                                   \
        result_type result;                                             \
        auto pass = parse_enum_expr<result_type, grammar_type>(string, result); \
        BOOST_CHECK(pass);                                              \
        BOOST_CHECK(result == expected);                                \
    }

#define CHECK_FAILED_ENUM(result_type, grammar_type)                    \
    {                                                                   \
        result_type result;                                             \
        auto pass = parse<result_type, grammar_type>("ThisEnumerationDoesNotExist", result); \
        BOOST_CHECK(!pass);                                             \
    }

#define CHECK_FAILED_ENUM_EXPR(result_type, grammar_type)               \
    {                                                                   \
        result_type result;                                             \
        auto pass = parse_enum_expr<result_type, grammar_type>("ThisEnumerationDoesNotExist", result); \
        BOOST_CHECK(!pass);                                             \
    }

BOOST_AUTO_TEST_CASE(CaptureResultParser) {
    // XXX: No enum number value to validate enum coverage.
    CHECK_ENUM_AND_RESULT("Capture", CaptureResult::CR_CAPTURE, CaptureResult, parse::capture_result_enum_grammar);
    CHECK_ENUM_AND_RESULT("Destroy", CaptureResult::CR_DESTROY, CaptureResult, parse::capture_result_enum_grammar);
    CHECK_ENUM_AND_RESULT("Retain", CaptureResult::CR_RETAIN, CaptureResult, parse::capture_result_enum_grammar);
    CHECK_FAILED_ENUM(CaptureResult, parse::capture_result_enum_grammar);
}

BOOST_AUTO_TEST_CASE(EmpireAffiliationTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(int(EmpireAffiliationType::NUM_AFFIL_TYPES) == 8, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("TheEmpire", EmpireAffiliationType::AFFIL_SELF, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("EnemyOf", EmpireAffiliationType::AFFIL_ENEMY, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("PeaceWith", EmpireAffiliationType::AFFIL_PEACE, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("AllyOf", EmpireAffiliationType::AFFIL_ALLY, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("AnyEmpire", EmpireAffiliationType::AFFIL_ANY, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("None", EmpireAffiliationType::AFFIL_NONE, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("CanSee", EmpireAffiliationType::AFFIL_CAN_SEE, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("Human", EmpireAffiliationType::AFFIL_HUMAN, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_FAILED_ENUM(EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
}

BOOST_AUTO_TEST_CASE(NonShipPartMeterTypeParser) {
    // XXX: METER_SIZE not handled, still used?
    // XXX: HAPPINESS meters not handled, still used?
    // XXX: REBEL_TROOPS meters not handled, still used?
    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    CHECK_ENUM_AND_RESULT("TargetPopulation", MeterType::METER_TARGET_POPULATION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetPopulation", MeterType::METER_TARGET_POPULATION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetIndustry", MeterType::METER_TARGET_INDUSTRY, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetIndustry", MeterType::METER_TARGET_INDUSTRY, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetResearch", MeterType::METER_TARGET_RESEARCH, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetResearch", MeterType::METER_TARGET_RESEARCH, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetInfluence", MeterType::METER_TARGET_INFLUENCE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetInfluence", MeterType::METER_TARGET_INFLUENCE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetConstruction", MeterType::METER_TARGET_CONSTRUCTION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetConstruction", MeterType::METER_TARGET_CONSTRUCTION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxFuel", MeterType::METER_MAX_FUEL, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxFuel", MeterType::METER_MAX_FUEL, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxShield", MeterType::METER_MAX_SHIELD, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxShield", MeterType::METER_MAX_SHIELD, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxStructure", MeterType::METER_MAX_STRUCTURE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxStructure", MeterType::METER_MAX_STRUCTURE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxDefense", MeterType::METER_MAX_DEFENSE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxDefense", MeterType::METER_MAX_DEFENSE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxTroops", MeterType::METER_MAX_TROOPS, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxTroops", MeterType::METER_MAX_TROOPS, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Population", MeterType::METER_POPULATION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetPopulation", MeterType::METER_POPULATION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Industry", MeterType::METER_INDUSTRY, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetIndustry", MeterType::METER_INDUSTRY, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Research", MeterType::METER_RESEARCH, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetResearch", MeterType::METER_RESEARCH, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Influence", MeterType::METER_INFLUENCE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetInfluence", MeterType::METER_INFLUENCE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Construction", MeterType::METER_CONSTRUCTION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetConstruction", MeterType::METER_CONSTRUCTION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Fuel", MeterType::METER_FUEL, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetFuel", MeterType::METER_FUEL, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Shield", MeterType::METER_SHIELD, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetShield", MeterType::METER_SHIELD, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Structure", MeterType::METER_STRUCTURE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetStructure", MeterType::METER_STRUCTURE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Defense", MeterType::METER_DEFENSE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetDefense", MeterType::METER_DEFENSE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Troops", MeterType::METER_TROOPS, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTroops", MeterType::METER_TROOPS, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Supply", MeterType::METER_SUPPLY, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetSupply", MeterType::METER_SUPPLY, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Stealth", MeterType::METER_STEALTH, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetStealth", MeterType::METER_STEALTH, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Detection", MeterType::METER_DETECTION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetDetection", MeterType::METER_DETECTION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Speed", MeterType::METER_SPEED, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetSpeed", MeterType::METER_SPEED, MeterType, parse::set_non_ship_part_meter_enum_grammar);

    CHECK_FAILED_ENUM(MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_FAILED_ENUM(MeterType, parse::set_non_ship_part_meter_enum_grammar);
}

BOOST_AUTO_TEST_CASE(PlanetEnvironmentEnumParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(int(PlanetEnvironment::NUM_PLANET_ENVIRONMENTS) == 5, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Uninhabitable", PlanetEnvironment::PE_UNINHABITABLE, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Hostile", PlanetEnvironment::PE_HOSTILE, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Poor", PlanetEnvironment::PE_POOR, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Adequate", PlanetEnvironment::PE_ADEQUATE, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Good", PlanetEnvironment::PE_GOOD, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_FAILED_ENUM_EXPR(PlanetEnvironment, parse::detail::planet_environment_parser_rules);
}

BOOST_AUTO_TEST_CASE(PlanetSizeParser) {
    // Literal is number of tests, not number of enums.
    // XXX: SZ_NOWORLD has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(int(PlanetSize::NUM_PLANET_SIZES) == 7 + 1, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Tiny", PlanetSize::SZ_TINY, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Small", PlanetSize::SZ_SMALL, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Medium", PlanetSize::SZ_MEDIUM, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Large", PlanetSize::SZ_LARGE, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Huge", PlanetSize::SZ_HUGE, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Asteroids", PlanetSize::SZ_ASTEROIDS, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("GasGiant", PlanetSize::SZ_GASGIANT, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_FAILED_ENUM_EXPR(PlanetSize, parse::detail::planet_size_parser_rules);
}

BOOST_AUTO_TEST_CASE(PlanetTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(int(PlanetType::NUM_PLANET_TYPES) == 11, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Swamp", PlanetType::PT_SWAMP, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Toxic", PlanetType::PT_TOXIC, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Inferno", PlanetType::PT_INFERNO, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Radiated", PlanetType::PT_RADIATED, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Barren", PlanetType::PT_BARREN, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Tundra", PlanetType::PT_TUNDRA, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Desert", PlanetType::PT_DESERT, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Terran", PlanetType::PT_TERRAN, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Ocean", PlanetType::PT_OCEAN, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Asteroids", PlanetType::PT_ASTEROIDS, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("GasGiant", PlanetType::PT_GASGIANT, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_FAILED_ENUM_EXPR(PlanetType, parse::detail::planet_type_parser_rules);
}

BOOST_AUTO_TEST_CASE(ShipPartsClassParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(int(ShipPartClass::NUM_SHIP_PART_CLASSES) == 17, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("ShortRange", ShipPartClass::PC_DIRECT_WEAPON, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("FighterBay", ShipPartClass::PC_FIGHTER_BAY, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("FighterHangar", ShipPartClass::PC_FIGHTER_HANGAR, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Shield", ShipPartClass::PC_SHIELD, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Armour", ShipPartClass::PC_ARMOUR, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Troops", ShipPartClass::PC_TROOPS, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Detection", ShipPartClass::PC_DETECTION, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Stealth", ShipPartClass::PC_STEALTH, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Fuel", ShipPartClass::PC_FUEL, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Colony", ShipPartClass::PC_COLONY, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Speed", ShipPartClass::PC_SPEED, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("General", ShipPartClass::PC_GENERAL, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Bombard", ShipPartClass::PC_BOMBARD, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Industry", ShipPartClass::PC_INDUSTRY, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Research", ShipPartClass::PC_RESEARCH, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Influence", ShipPartClass::PC_INFLUENCE, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("ProductionLocation", ShipPartClass::PC_PRODUCTION_LOCATION, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_FAILED_ENUM(ShipPartClass, parse::ship_part_class_enum_grammar);
}

BOOST_AUTO_TEST_CASE(ShipPartMeterTypeParser) {
    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    CHECK_ENUM_AND_RESULT("Damage", MeterType::METER_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetDamage", MeterType::METER_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Capacity", MeterType::METER_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetCapacity", MeterType::METER_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxDamage", MeterType::METER_MAX_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxDamage", MeterType::METER_MAX_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxCapacity", MeterType::METER_MAX_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxCapacity", MeterType::METER_MAX_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetSecondaryStat", MeterType::METER_SECONDARY_STAT, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxSecondaryStat", MeterType::METER_MAX_SECONDARY_STAT, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_FAILED_ENUM(MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_FAILED_ENUM(MeterType, parse::set_ship_part_meter_enum_grammar);
}

BOOST_AUTO_TEST_CASE(ShipSlotTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(int(ShipSlotType::NUM_SHIP_SLOT_TYPES) == 3, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("External", ShipSlotType::SL_EXTERNAL, ShipSlotType, parse::ship_slot_enum_grammar);
    CHECK_ENUM_AND_RESULT("Internal", ShipSlotType::SL_INTERNAL, ShipSlotType, parse::ship_slot_enum_grammar);
    CHECK_ENUM_AND_RESULT("Core", ShipSlotType::SL_CORE, ShipSlotType, parse::ship_slot_enum_grammar);
    CHECK_FAILED_ENUM(ShipSlotType, parse::ship_slot_enum_grammar);
}

BOOST_AUTO_TEST_CASE(StatisticTypeParser) {
    // XXX: No enum number value to validate enum coverage.

    CHECK_ENUM_AND_RESULT("Count", ValueRef::StatisticType::COUNT, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Sum", ValueRef::StatisticType::SUM, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Mean", ValueRef::StatisticType::MEAN, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("RMS", ValueRef::StatisticType::RMS, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Mode", ValueRef::StatisticType::MODE, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Max", ValueRef::StatisticType::MAX, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Min", ValueRef::StatisticType::MIN, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Spread", ValueRef::StatisticType::SPREAD, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("StDev", ValueRef::StatisticType::STDEV, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Product", ValueRef::StatisticType::PRODUCT, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_FAILED_ENUM(ValueRef::StatisticType, parse::statistic_enum_grammar);
}

BOOST_AUTO_TEST_CASE(StarTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(int(StarType::NUM_STAR_TYPES) == 8, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Blue", StarType::STAR_BLUE, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("White", StarType::STAR_WHITE, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Yellow", StarType::STAR_YELLOW, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Orange", StarType::STAR_ORANGE, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Red", StarType::STAR_RED, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Neutron", StarType::STAR_NEUTRON, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("BlackHole", StarType::STAR_BLACK, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("NoStar", StarType::STAR_NONE, StarType, parse::detail::star_type_parser_rules);
    CHECK_FAILED_ENUM_EXPR(StarType, parse::detail::star_type_parser_rules);
}

BOOST_AUTO_TEST_CASE(UnlockableItemTypeParser)
{
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(int(UnlockableItemType::NUM_UNLOCKABLE_ITEM_TYPES) == 6, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("Building", UnlockableItemType::UIT_BUILDING, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("ShipPart", UnlockableItemType::UIT_SHIP_PART, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("ShipHull", UnlockableItemType::UIT_SHIP_HULL, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("ShipDesign", UnlockableItemType::UIT_SHIP_DESIGN, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("Tech", UnlockableItemType::UIT_TECH, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("Policy", UnlockableItemType::UIT_POLICY, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_FAILED_ENUM(UnlockableItemType, parse::unlockable_item_enum_grammar);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeParser)
{
    // Literal is number of tests, not number of enums.
    // XXX: OBJ_FIGHTER has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(int(UniverseObjectType::NUM_OBJ_TYPES) == 6 + 1, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Building", UniverseObjectType::OBJ_BUILDING, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Ship", UniverseObjectType::OBJ_SHIP, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Fleet ", UniverseObjectType::OBJ_FLEET , UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Planet", UniverseObjectType::OBJ_PLANET, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("System", UniverseObjectType::OBJ_SYSTEM, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Field", UniverseObjectType::OBJ_FIELD, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_FAILED_ENUM_EXPR(UniverseObjectType, parse::detail::universe_object_type_parser_rules);
}

BOOST_AUTO_TEST_SUITE_END()
