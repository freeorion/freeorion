#include <boost/test/unit_test.hpp>

#include "parse/EnumParser.h"
#include "parse/ValueRefParser.h"
#include "parse/ValueRefParserImpl.h"
#include "universe/Enums.h"


struct EnumParserFixture {
    template <class Type>
    bool parse(std::string phrase, Type& result, parse::enum_rule<Type>& rule) {
        const parse::lexer& lexer = lexer.instance();
        boost::spirit::qi::in_state_type in_state;
        boost::spirit::qi::eoi_type eoi;
        boost::spirit::qi::_1_type _1;

        std::string::const_iterator begin_phrase = phrase.begin();
        std::string::const_iterator end_phrase = phrase.end();

        auto begin = lexer.begin(begin_phrase, end_phrase);
        auto end   = lexer.end();

        bool matched = boost::spirit::qi::phrase_parse(
            begin, end,
            rule[boost::phoenix::ref(result) = _1] > eoi,
            in_state("WS")[lexer.self]
        );

        return matched && begin == end;
    }
};

BOOST_FIXTURE_TEST_SUITE(TestEnumParser, EnumParserFixture)

BOOST_AUTO_TEST_CASE(CaptureResultParser) {
    CaptureResult result;

    // XXX: No enum number value to validate enum coverage.

    BOOST_CHECK(parse("Capture", result, capture_result_enum));
    BOOST_CHECK(result == CR_CAPTURE);

    BOOST_CHECK(parse("Destroy", result, capture_result_enum));
    BOOST_CHECK(result == CR_DESTROY);

    BOOST_CHECK(parse("Retain", result, capture_result_enum));
    BOOST_CHECK(result == CR_RETAIN);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_CAPTURE_RESULT;
    BOOST_CHECK(!parse("DoesNotExist", result, capture_result_enum));
    BOOST_CHECK(result == INVALID_CAPTURE_RESULT);
}

BOOST_AUTO_TEST_CASE(EmpireAffiliationTypeParser) {
    EmpireAffiliationType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_AFFIL_TYPES == 7, "Untested enumeration value.");

    BOOST_CHECK(parse("TheEmpire", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == AFFIL_SELF);

    BOOST_CHECK(parse("EnemyOf", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == AFFIL_ENEMY);

    BOOST_CHECK(parse("AllyOf", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == AFFIL_ALLY);

    BOOST_CHECK(parse("AnyEmpire", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == AFFIL_ANY);

    BOOST_CHECK(parse("None", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == AFFIL_NONE);

    BOOST_CHECK(parse("CanSee", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == AFFIL_CAN_SEE);

    BOOST_CHECK(parse("HUMAN", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == AFFIL_HUMAN);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_EMPIRE_AFFIL_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == INVALID_EMPIRE_AFFIL_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_AFFIL_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, empire_affiliation_type_enum));
    BOOST_CHECK(result == NUM_AFFIL_TYPES);
}

BOOST_AUTO_TEST_CASE(NonShipPartMeterTypeParser) {
    MeterType result;

    // XXX: METER_SIZE not handled, still used?
    // XXX: HAPPINESS meters not handled, still used?
    // XXX: REBEL_TROOPS meters not handled, still used?
    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    BOOST_CHECK(parse("TargetPopulation", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_POPULATION);
    BOOST_CHECK(parse("SetTargetPopulation", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_POPULATION);

    BOOST_CHECK(parse("TargetIndustry", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_INDUSTRY);
    BOOST_CHECK(parse("SetTargetIndustry", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_INDUSTRY);

    BOOST_CHECK(parse("TargetResearch", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_RESEARCH);
    BOOST_CHECK(parse("SetTargetResearch", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_RESEARCH);

    BOOST_CHECK(parse("TargetTrade", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_TRADE);
    BOOST_CHECK(parse("SetTargetTrade", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_TRADE);

    BOOST_CHECK(parse("TargetConstruction", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_CONSTRUCTION);
    BOOST_CHECK(parse("SetTargetConstruction", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TARGET_CONSTRUCTION);

    BOOST_CHECK(parse("MaxFuel", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_FUEL);
    BOOST_CHECK(parse("SetMaxFuel", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_FUEL);

    BOOST_CHECK(parse("MaxShield", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_SHIELD);
    BOOST_CHECK(parse("SetMaxShield", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_SHIELD);

    BOOST_CHECK(parse("MaxStructure", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_STRUCTURE);
    BOOST_CHECK(parse("SetMaxStructure", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_STRUCTURE);

    BOOST_CHECK(parse("MaxDefense", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_DEFENSE);
    BOOST_CHECK(parse("SetMaxDefense", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_DEFENSE);

    BOOST_CHECK(parse("MaxTroops", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_TROOPS);
    BOOST_CHECK(parse("SetMaxTroops", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_TROOPS);

    BOOST_CHECK(parse("Population", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_POPULATION);
    BOOST_CHECK(parse("SetPopulation", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_POPULATION);

    BOOST_CHECK(parse("Industry", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_INDUSTRY);
    BOOST_CHECK(parse("SetIndustry", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_INDUSTRY);

    BOOST_CHECK(parse("Research", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_RESEARCH);
    BOOST_CHECK(parse("SetResearch", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_RESEARCH);

    BOOST_CHECK(parse("Trade", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TRADE);
    BOOST_CHECK(parse("SetTrade", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TRADE);

    BOOST_CHECK(parse("Construction", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_CONSTRUCTION);
    BOOST_CHECK(parse("SetConstruction", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_CONSTRUCTION);

    BOOST_CHECK(parse("Fuel", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_FUEL);
    BOOST_CHECK(parse("SetFuel", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_FUEL);

    BOOST_CHECK(parse("Shield", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_SHIELD);
    BOOST_CHECK(parse("SetShield", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_SHIELD);

    BOOST_CHECK(parse("Structure", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_STRUCTURE);
    BOOST_CHECK(parse("SetStructure", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_STRUCTURE);

    BOOST_CHECK(parse("Defense", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_DEFENSE);
    BOOST_CHECK(parse("SetDefense", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_DEFENSE);

    BOOST_CHECK(parse("Troops", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TROOPS);
    BOOST_CHECK(parse("SetTroops", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_TROOPS);

    BOOST_CHECK(parse("Supply", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_SUPPLY);
    BOOST_CHECK(parse("SetSupply", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_SUPPLY);

    BOOST_CHECK(parse("Stealth", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_STEALTH);
    BOOST_CHECK(parse("SetStealth", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_STEALTH);

    BOOST_CHECK(parse("Detection", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_DETECTION_RANGE);
    BOOST_CHECK(parse("SetDetection", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_DETECTION_RANGE);

    BOOST_CHECK(parse("Speed", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_SPEED);
    BOOST_CHECK(parse("SetSpeed", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_SPEED);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_METER_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == INVALID_METER_TYPE);
    result = INVALID_METER_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == INVALID_METER_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_METER_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, non_ship_part_meter_type_enum));
    BOOST_CHECK(result == NUM_METER_TYPES);
    result = NUM_METER_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, set_non_ship_part_meter_type_enum));
    BOOST_CHECK(result == NUM_METER_TYPES);
}

BOOST_AUTO_TEST_CASE(PlanetEnvironmentParser) {
    PlanetEnvironment result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_ENVIRONMENTS == 5, "Untested enumeration value.");

    BOOST_CHECK(parse("Uninhabitable", result, planet_environment_rules.enum_expr));
    BOOST_CHECK(result == PE_UNINHABITABLE);

    BOOST_CHECK(parse("Hostile", result, planet_environment_rules.enum_expr));
    BOOST_CHECK(result == PE_HOSTILE);

    BOOST_CHECK(parse("Poor", result, planet_environment_rules.enum_expr));
    BOOST_CHECK(result == PE_POOR);

    BOOST_CHECK(parse("Adequate", result, planet_environment_rules.enum_expr));
    BOOST_CHECK(result == PE_ADEQUATE);

    BOOST_CHECK(parse("Good", result, planet_environment_rules.enum_expr));
    BOOST_CHECK(result == PE_GOOD);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_PLANET_ENVIRONMENT;
    BOOST_CHECK(!parse("DoesNotExist", result, planet_environment_rules.enum_expr));
    BOOST_CHECK(result == INVALID_PLANET_ENVIRONMENT);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_PLANET_ENVIRONMENTS;
    BOOST_CHECK(!parse("DoesNotExist", result, planet_environment_rules.enum_expr));
    BOOST_CHECK(result == NUM_PLANET_ENVIRONMENTS);
}

BOOST_AUTO_TEST_CASE(PlanetSizeParser) {
    PlanetSize result;

    // Literal is number of tests, not number of enums.
    // XXX: SZ_NOWORLD has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_SIZES == 7 + 1, "Untested enumeration value.");

    BOOST_CHECK(parse("Tiny", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == SZ_TINY);

    BOOST_CHECK(parse("Small", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == SZ_SMALL);

    BOOST_CHECK(parse("Medium", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == SZ_MEDIUM);

    BOOST_CHECK(parse("Large", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == SZ_LARGE);

    BOOST_CHECK(parse("Huge", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == SZ_HUGE);

    BOOST_CHECK(parse("Asteroids", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == SZ_ASTEROIDS);

    BOOST_CHECK(parse("GasGiant", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == SZ_GASGIANT);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_PLANET_SIZE;
    BOOST_CHECK(!parse("DoesNotExist", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == INVALID_PLANET_SIZE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_PLANET_SIZES;
    BOOST_CHECK(!parse("DoesNotExist", result, planet_size_rules.enum_expr));
    BOOST_CHECK(result == NUM_PLANET_SIZES);
}

BOOST_AUTO_TEST_CASE(PlanetTypeParser) {
    PlanetType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_TYPES == 11, "Untested enumeration value.");

    BOOST_CHECK(parse("Swamp", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_SWAMP);

    BOOST_CHECK(parse("Toxic", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_TOXIC);

    BOOST_CHECK(parse("Inferno", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_INFERNO);

    BOOST_CHECK(parse("Radiated", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_RADIATED);

    BOOST_CHECK(parse("Barren", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_BARREN);

    BOOST_CHECK(parse("Tundra", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_TUNDRA);

    BOOST_CHECK(parse("Desert", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_DESERT);

    BOOST_CHECK(parse("Terran", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_TERRAN);

    BOOST_CHECK(parse("Ocean", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_OCEAN);

    BOOST_CHECK(parse("Asteroids", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_ASTEROIDS);

    BOOST_CHECK(parse("GasGiant", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == PT_GASGIANT);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_PLANET_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == INVALID_PLANET_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_PLANET_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, planet_type_rules.enum_expr));
    BOOST_CHECK(result == NUM_PLANET_TYPES);
}

BOOST_AUTO_TEST_CASE(ShipPartsClassParser) {
    ShipPartClass result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_SHIP_PART_CLASSES == 17, "Untested enumeration value.");

    BOOST_CHECK(parse("ShortRange", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_DIRECT_WEAPON);

    BOOST_CHECK(parse("FighterBay", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_FIGHTER_BAY);

    BOOST_CHECK(parse("FighterHangar", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_FIGHTER_HANGAR);

    BOOST_CHECK(parse("Shield", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_SHIELD);

    BOOST_CHECK(parse("Armour", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_ARMOUR);

    BOOST_CHECK(parse("Troops", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_TROOPS);

    BOOST_CHECK(parse("Detection", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_DETECTION);

    BOOST_CHECK(parse("Stealth", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_STEALTH);

    BOOST_CHECK(parse("Fuel", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_FUEL);

    BOOST_CHECK(parse("Colony", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_COLONY);

    BOOST_CHECK(parse("Speed", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_SPEED);

    BOOST_CHECK(parse("General", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_GENERAL);

    BOOST_CHECK(parse("Bombard", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_BOMBARD);

    BOOST_CHECK(parse("Industry", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_INDUSTRY);

    BOOST_CHECK(parse("Research", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_RESEARCH);

    BOOST_CHECK(parse("Trade", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_TRADE);

    BOOST_CHECK(parse("ProductionLocation", result, ship_part_class_enum));
    BOOST_CHECK(result == PC_PRODUCTION_LOCATION);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_SHIP_PART_CLASS;
    BOOST_CHECK(!parse("DoesNotExist", result, ship_part_class_enum));
    BOOST_CHECK(result == INVALID_SHIP_PART_CLASS);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_SHIP_PART_CLASSES;
    BOOST_CHECK(!parse("DoesNotExist", result, ship_part_class_enum));
    BOOST_CHECK(result == NUM_SHIP_PART_CLASSES);
}

BOOST_AUTO_TEST_CASE(ShipPartMeterTypeParser) {
    MeterType result;

    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    BOOST_CHECK(parse("Damage", result, ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_CAPACITY);
    BOOST_CHECK(parse("SetDamage", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_CAPACITY);

    BOOST_CHECK(parse("Capacity", result, ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_CAPACITY);
    BOOST_CHECK(parse("SetCapacity", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_CAPACITY);

    BOOST_CHECK(parse("MaxDamage", result, ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_CAPACITY);
    BOOST_CHECK(parse("SetMaxDamage", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_CAPACITY);

    BOOST_CHECK(parse("MaxCapacity", result, ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_CAPACITY);
    BOOST_CHECK(parse("SetMaxCapacity", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_CAPACITY);

    BOOST_CHECK(parse("SetSecondaryStat", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_SECONDARY_STAT);

    BOOST_CHECK(parse("SetMaxSecondaryStat", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == METER_MAX_SECONDARY_STAT);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_METER_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == INVALID_METER_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_METER_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, set_ship_part_meter_type_enum));
    BOOST_CHECK(result == NUM_METER_TYPES);
}

BOOST_AUTO_TEST_CASE(ShipSlotTypeParser) {
    ShipSlotType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_SHIP_SLOT_TYPES == 3, "Untested enumeration value.");

    BOOST_CHECK(parse("External", result, ship_slot_type_enum));
    BOOST_CHECK(result == SL_EXTERNAL);

    BOOST_CHECK(parse("Internal", result, ship_slot_type_enum));
    BOOST_CHECK(result == SL_INTERNAL);

    BOOST_CHECK(parse("Core", result, ship_slot_type_enum));
    BOOST_CHECK(result == SL_CORE);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_SHIP_SLOT_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, ship_slot_type_enum));
    BOOST_CHECK(result == INVALID_SHIP_SLOT_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_SHIP_SLOT_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, ship_slot_type_enum));
    BOOST_CHECK(result == NUM_SHIP_SLOT_TYPES);
}

BOOST_AUTO_TEST_CASE(StatisticTypeParser) {
    ValueRef::StatisticType result;

    // XXX: No enum number value to validate enum coverage.

    BOOST_CHECK(parse("Count", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::COUNT);

    BOOST_CHECK(parse("Sum", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::SUM);

    BOOST_CHECK(parse("Mean", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::MEAN);

    BOOST_CHECK(parse("Rms", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::RMS);

    BOOST_CHECK(parse("Mode", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::MODE);

    BOOST_CHECK(parse("Max", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::MAX);

    BOOST_CHECK(parse("Min", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::MIN);

    BOOST_CHECK(parse("Spread", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::SPREAD);

    BOOST_CHECK(parse("StDev", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::STDEV);

    BOOST_CHECK(parse("Product", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::PRODUCT);

    // XXX: is not modifying result the correct behaviour?
    result = ValueRef::INVALID_STATISTIC_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, statistic_type_enum));
    BOOST_CHECK(result == ValueRef::INVALID_STATISTIC_TYPE);
}

BOOST_AUTO_TEST_CASE(StarTypeParser) {
    StarType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_STAR_TYPES == 8, "Untested enumeration value.");

    BOOST_CHECK(parse("Blue", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_BLUE);

    BOOST_CHECK(parse("White", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_WHITE);

    BOOST_CHECK(parse("Yellow", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_YELLOW);

    BOOST_CHECK(parse("Orange", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_ORANGE);

    BOOST_CHECK(parse("Red", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_RED);

    BOOST_CHECK(parse("Neutron", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_NEUTRON);

    BOOST_CHECK(parse("BlackHole", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_BLACK);

    BOOST_CHECK(parse("NoStar", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == STAR_NONE);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_STAR_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == INVALID_STAR_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_STAR_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, parse::detail::star_type_rules().enum_expr));
    BOOST_CHECK(result == NUM_STAR_TYPES);
}

BOOST_AUTO_TEST_CASE(UnlockableItemTypeParser)
{
    UnlockableItemType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_UNLOCKABLE_ITEM_TYPES == 5, "Untested enumeration value.");

    BOOST_CHECK(parse("Building", result, unlockable_item_type_enum));
    BOOST_CHECK(result == UIT_BUILDING);

    BOOST_CHECK(parse("ShipPart", result, unlockable_item_type_enum));
    BOOST_CHECK(result == UIT_SHIP_PART);

    BOOST_CHECK(parse("ShipHull", result, unlockable_item_type_enum));
    BOOST_CHECK(result == UIT_SHIP_HULL);

    BOOST_CHECK(parse("ShipDesign", result, unlockable_item_type_enum));
    BOOST_CHECK(result == UIT_SHIP_DESIGN);

    BOOST_CHECK(parse("Tech", result, unlockable_item_type_enum));
    BOOST_CHECK(result == UIT_TECH);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_UNLOCKABLE_ITEM_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, unlockable_item_type_enum));
    BOOST_CHECK(result == INVALID_UNLOCKABLE_ITEM_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_UNLOCKABLE_ITEM_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, unlockable_item_type_enum));
    BOOST_CHECK(result == NUM_UNLOCKABLE_ITEM_TYPES);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeParser)
{
    UniverseObjectType result;

    // Literal is number of tests, not number of enums.
    // XXX: OBJ_FIGHTER has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(NUM_OBJ_TYPES == 8 + 1, "Untested enumeration value.");

    BOOST_CHECK(parse("Building", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_BUILDING);

    BOOST_CHECK(parse("Ship", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_SHIP);

    BOOST_CHECK(parse("Fleet ", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_FLEET );

    BOOST_CHECK(parse("Planet", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_PLANET);

    BOOST_CHECK(parse("PopulationCenter", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_POP_CENTER);

    BOOST_CHECK(parse("ProductionCenter", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_PROD_CENTER);

    BOOST_CHECK(parse("System", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_SYSTEM);

    BOOST_CHECK(parse("Field", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == OBJ_FIELD);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_UNIVERSE_OBJECT_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == INVALID_UNIVERSE_OBJECT_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_OBJ_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result, parse::detail::universe_object_type_rules().enum_expr));
    BOOST_CHECK(result == NUM_OBJ_TYPES);
}

BOOST_AUTO_TEST_SUITE_END()
