#include <boost/test/unit_test.hpp>

#include "parse/EnumParser.h"
#include "parse/ValueRefParser.h"
#include "parse/EnumValueRefRules.h"
#include "parse/ConditionParserImpl.h"
#include "universe/Enums.h"

#include <type_traits>

namespace {
    // File scope lexer since: 1) initializing the lexer deterministic finite
    // automaton (DFA) is slow; 2) the tests are single threaded and
    // non-concurrent.
    const parse::lexer lexer;
}

struct EnumParserFixture {
    template <typename Grammar>
    Grammar& make_grammar(
        const parse::lexer& lexer,
        typename std::enable_if<std::is_constructible<Grammar, const parse::lexer&>::value, std::nullptr_t>::type = nullptr)
    {
        static Grammar grammar(lexer);
        return grammar;
    }

    template <typename Grammar>
    Grammar& make_grammar(
        const parse::lexer& lexer,
        typename std::enable_if<std::is_constructible<Grammar, const parse::lexer&, parse::detail::Labeller&>::value, std::nullptr_t>::type = nullptr)
    {
        parse::detail::Labeller label;
        static Grammar grammar(lexer, label);
        return grammar;
    }

    template <typename Grammar>
    Grammar& make_grammar(
        const parse::lexer& lexer,
        typename std::enable_if<std::is_constructible<Grammar, const parse::lexer&, parse::detail::Labeller&, const parse::detail::condition_parser_grammar&>::value, std::nullptr_t>::type = nullptr)
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
    CHECK_ENUM_AND_RESULT("Capture", CR_CAPTURE, CaptureResult, parse::capture_result_enum_grammar);
    CHECK_ENUM_AND_RESULT("Destroy", CR_DESTROY, CaptureResult, parse::capture_result_enum_grammar);
    CHECK_ENUM_AND_RESULT("Retain", CR_RETAIN, CaptureResult, parse::capture_result_enum_grammar);
    CHECK_FAILED_ENUM(CaptureResult, parse::capture_result_enum_grammar);
}

BOOST_AUTO_TEST_CASE(EmpireAffiliationTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_AFFIL_TYPES == 7, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("TheEmpire", AFFIL_SELF, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("EnemyOf", AFFIL_ENEMY, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("AllyOf", AFFIL_ALLY, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("AnyEmpire", AFFIL_ANY, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("None", AFFIL_NONE, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("CanSee", AFFIL_CAN_SEE, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_ENUM_AND_RESULT("HUMAN", AFFIL_HUMAN, EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
    CHECK_FAILED_ENUM(EmpireAffiliationType, parse::empire_affiliation_enum_grammar);
}

BOOST_AUTO_TEST_CASE(NonShipPartMeterTypeParser) {
    // XXX: METER_SIZE not handled, still used?
    // XXX: HAPPINESS meters not handled, still used?
    // XXX: REBEL_TROOPS meters not handled, still used?
    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    CHECK_ENUM_AND_RESULT("TargetPopulation", METER_TARGET_POPULATION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetPopulation", METER_TARGET_POPULATION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetIndustry", METER_TARGET_INDUSTRY, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetIndustry", METER_TARGET_INDUSTRY, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetResearch", METER_TARGET_RESEARCH, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetResearch", METER_TARGET_RESEARCH, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetTrade", METER_TARGET_TRADE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetTrade", METER_TARGET_TRADE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("TargetConstruction", METER_TARGET_CONSTRUCTION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTargetConstruction", METER_TARGET_CONSTRUCTION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxFuel", METER_MAX_FUEL, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxFuel", METER_MAX_FUEL, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxShield", METER_MAX_SHIELD, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxShield", METER_MAX_SHIELD, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxStructure", METER_MAX_STRUCTURE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxStructure", METER_MAX_STRUCTURE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxDefense", METER_MAX_DEFENSE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxDefense", METER_MAX_DEFENSE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxTroops", METER_MAX_TROOPS, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxTroops", METER_MAX_TROOPS, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Population", METER_POPULATION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetPopulation", METER_POPULATION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Industry", METER_INDUSTRY, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetIndustry", METER_INDUSTRY, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Research", METER_RESEARCH, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetResearch", METER_RESEARCH, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Trade", METER_TRADE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTrade", METER_TRADE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Construction", METER_CONSTRUCTION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetConstruction", METER_CONSTRUCTION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Fuel", METER_FUEL, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetFuel", METER_FUEL, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Shield", METER_SHIELD, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetShield", METER_SHIELD, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Structure", METER_STRUCTURE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetStructure", METER_STRUCTURE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Defense", METER_DEFENSE, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetDefense", METER_DEFENSE, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Troops", METER_TROOPS, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetTroops", METER_TROOPS, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Supply", METER_SUPPLY, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetSupply", METER_SUPPLY, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Stealth", METER_STEALTH, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetStealth", METER_STEALTH, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Detection", METER_DETECTION, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetDetection", METER_DETECTION, MeterType, parse::set_non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Speed", METER_SPEED, MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetSpeed", METER_SPEED, MeterType, parse::set_non_ship_part_meter_enum_grammar);

    CHECK_FAILED_ENUM(MeterType, parse::non_ship_part_meter_enum_grammar);
    CHECK_FAILED_ENUM(MeterType, parse::set_non_ship_part_meter_enum_grammar);
}

BOOST_AUTO_TEST_CASE(PlanetEnvironmentEnumParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_ENVIRONMENTS == 5, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Uninhabitable", PE_UNINHABITABLE, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Hostile", PE_HOSTILE, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Poor", PE_POOR, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Adequate", PE_ADEQUATE, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Good", PE_GOOD, PlanetEnvironment, parse::detail::planet_environment_parser_rules);
    CHECK_FAILED_ENUM_EXPR(PlanetEnvironment, parse::detail::planet_environment_parser_rules);
}

BOOST_AUTO_TEST_CASE(PlanetSizeParser) {
    // Literal is number of tests, not number of enums.
    // XXX: SZ_NOWORLD has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_SIZES == 7 + 1, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Tiny", SZ_TINY, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Small", SZ_SMALL, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Medium", SZ_MEDIUM, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Large", SZ_LARGE, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Huge", SZ_HUGE, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Asteroids", SZ_ASTEROIDS, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("GasGiant", SZ_GASGIANT, PlanetSize, parse::detail::planet_size_parser_rules);
    CHECK_FAILED_ENUM_EXPR(PlanetSize, parse::detail::planet_size_parser_rules);
}

BOOST_AUTO_TEST_CASE(PlanetTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_TYPES == 11, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Swamp", PT_SWAMP, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Toxic", PT_TOXIC, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Inferno", PT_INFERNO, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Radiated", PT_RADIATED, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Barren", PT_BARREN, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Tundra", PT_TUNDRA, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Desert", PT_DESERT, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Terran", PT_TERRAN, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Ocean", PT_OCEAN, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Asteroids", PT_ASTEROIDS, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("GasGiant", PT_GASGIANT, PlanetType, parse::detail::planet_type_parser_rules);
    CHECK_FAILED_ENUM_EXPR(PlanetType, parse::detail::planet_type_parser_rules);
}

BOOST_AUTO_TEST_CASE(ShipPartsClassParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_SHIP_PART_CLASSES == 17, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("ShortRange", PC_DIRECT_WEAPON, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("FighterBay", PC_FIGHTER_BAY, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("FighterHangar", PC_FIGHTER_HANGAR, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Shield", PC_SHIELD, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Armour", PC_ARMOUR, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Troops", PC_TROOPS, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Detection", PC_DETECTION, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Stealth", PC_STEALTH, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Fuel", PC_FUEL, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Colony", PC_COLONY, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Speed", PC_SPEED, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("General", PC_GENERAL, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Bombard", PC_BOMBARD, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Industry", PC_INDUSTRY, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Research", PC_RESEARCH, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("Trade", PC_TRADE, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_ENUM_AND_RESULT("ProductionLocation", PC_PRODUCTION_LOCATION, ShipPartClass, parse::ship_part_class_enum_grammar);
    CHECK_FAILED_ENUM(ShipPartClass, parse::ship_part_class_enum_grammar);
}

BOOST_AUTO_TEST_CASE(ShipPartMeterTypeParser) {
    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    CHECK_ENUM_AND_RESULT("Damage", METER_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetDamage", METER_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("Capacity", METER_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetCapacity", METER_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxDamage", METER_MAX_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxDamage", METER_MAX_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("MaxCapacity", METER_MAX_CAPACITY, MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxCapacity", METER_MAX_CAPACITY, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetSecondaryStat", METER_SECONDARY_STAT, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_ENUM_AND_RESULT("SetMaxSecondaryStat", METER_MAX_SECONDARY_STAT, MeterType, parse::set_ship_part_meter_enum_grammar);
    CHECK_FAILED_ENUM(MeterType, parse::ship_part_meter_enum_grammar);
    CHECK_FAILED_ENUM(MeterType, parse::set_ship_part_meter_enum_grammar);
}

BOOST_AUTO_TEST_CASE(ShipSlotTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_SHIP_SLOT_TYPES == 3, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("External", SL_EXTERNAL, ShipSlotType, parse::ship_slot_enum_grammar);
    CHECK_ENUM_AND_RESULT("Internal", SL_INTERNAL, ShipSlotType, parse::ship_slot_enum_grammar);
    CHECK_ENUM_AND_RESULT("Core", SL_CORE, ShipSlotType, parse::ship_slot_enum_grammar);
    CHECK_FAILED_ENUM(ShipSlotType, parse::ship_slot_enum_grammar);
}

BOOST_AUTO_TEST_CASE(StatisticTypeParser) {
    // XXX: No enum number value to validate enum coverage.

    CHECK_ENUM_AND_RESULT("Count", ValueRef::COUNT, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Sum", ValueRef::SUM, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Mean", ValueRef::MEAN, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Rms", ValueRef::RMS, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Mode", ValueRef::MODE, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Max", ValueRef::MAX, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Min", ValueRef::MIN, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Spread", ValueRef::SPREAD, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("StDev", ValueRef::STDEV, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_ENUM_AND_RESULT("Product", ValueRef::PRODUCT, ValueRef::StatisticType, parse::statistic_enum_grammar);
    CHECK_FAILED_ENUM(ValueRef::StatisticType, parse::statistic_enum_grammar);
}

BOOST_AUTO_TEST_CASE(StarTypeParser) {
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_STAR_TYPES == 8, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Blue", STAR_BLUE, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("White", STAR_WHITE, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Yellow", STAR_YELLOW, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Orange", STAR_ORANGE, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Red", STAR_RED, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Neutron", STAR_NEUTRON, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("BlackHole", STAR_BLACK, StarType, parse::detail::star_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("NoStar", STAR_NONE, StarType, parse::detail::star_type_parser_rules);
    CHECK_FAILED_ENUM_EXPR(StarType, parse::detail::star_type_parser_rules);
}

BOOST_AUTO_TEST_CASE(UnlockableItemTypeParser)
{
    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_UNLOCKABLE_ITEM_TYPES == 5, "Untested enumeration value.");

    CHECK_ENUM_AND_RESULT("Building", UIT_BUILDING, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("ShipPart", UIT_SHIP_PART, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("ShipHull", UIT_SHIP_HULL, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("ShipDesign", UIT_SHIP_DESIGN, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_ENUM_AND_RESULT("Tech", UIT_TECH, UnlockableItemType, parse::unlockable_item_enum_grammar);
    CHECK_FAILED_ENUM(UnlockableItemType, parse::unlockable_item_enum_grammar);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeParser)
{
    // Literal is number of tests, not number of enums.
    // XXX: OBJ_FIGHTER has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(NUM_OBJ_TYPES == 8 + 1, "Untested enumeration value.");

    CHECK_ENUM_EXPR_AND_RESULT("Building", OBJ_BUILDING, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Ship", OBJ_SHIP, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Fleet ", OBJ_FLEET , UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Planet", OBJ_PLANET, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("PopulationCenter", OBJ_POP_CENTER, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("ProductionCenter", OBJ_PROD_CENTER, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("System", OBJ_SYSTEM, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_ENUM_EXPR_AND_RESULT("Field", OBJ_FIELD, UniverseObjectType, parse::detail::universe_object_type_parser_rules);
    CHECK_FAILED_ENUM_EXPR(UniverseObjectType, parse::detail::universe_object_type_parser_rules);
}

BOOST_AUTO_TEST_SUITE_END()
