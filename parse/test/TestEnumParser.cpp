#include <boost/test/unit_test.hpp>

#include "EnumParser.h"

struct EnumParserFixture {
    template <class Type>
    bool parse(std::string phrase, Type& result) {
        typename parse::enum_parser_rule<Type>::type& rule = parse::enum_parser<Type>();
        const parse::lexer& lexer = lexer.instance();
        boost::spirit::qi::in_state_type in_state;
        boost::spirit::qi::_1_type _1;

        std::string::const_iterator begin_phrase = phrase.begin();
        std::string::const_iterator end_phrase = phrase.end();

        return boost::spirit::qi::phrase_parse(
            lexer.begin(begin_phrase, end_phrase),
            lexer.end(),
            rule[boost::phoenix::ref(result) = _1],
            in_state("WS")[lexer.self]
        );
    }

    bool parse_nonship_meter(std::string phrase, MeterType& result) {
        parse::enum_parser_rule<MeterType>::type& rule = parse::non_ship_part_meter_type_enum();
        const parse::lexer& lexer = lexer.instance();
        boost::spirit::qi::in_state_type in_state;
        boost::spirit::qi::_1_type _1;

        std::string::const_iterator begin_phrase = phrase.begin();
        std::string::const_iterator end_phrase = phrase.end();

        return boost::spirit::qi::phrase_parse(
            lexer.begin(begin_phrase, end_phrase),
            lexer.end(),
            rule[boost::phoenix::ref(result) = _1],
            in_state("WS")[lexer.self]
        );
    }

    bool set_parse_nonship_meter(std::string phrase, MeterType& result) {
        parse::enum_parser_rule<MeterType>::type& rule = parse::set_non_ship_part_meter_type_enum();
        const parse::lexer& lexer = lexer.instance();
        boost::spirit::qi::in_state_type in_state;
        boost::spirit::qi::_1_type _1;

        std::string::const_iterator begin_phrase = phrase.begin();
        std::string::const_iterator end_phrase = phrase.end();

        return boost::spirit::qi::phrase_parse(
            lexer.begin(begin_phrase, end_phrase),
            lexer.end(),
            rule[boost::phoenix::ref(result) = _1],
            in_state("WS")[lexer.self]
        );
    }

    bool parse_ship_meter(std::string phrase, MeterType& result) {
        parse::enum_parser_rule<MeterType>::type& rule = parse::ship_part_meter_type_enum();
        const parse::lexer& lexer = lexer.instance();
        boost::spirit::qi::in_state_type in_state;
        boost::spirit::qi::_1_type _1;

        std::string::const_iterator begin_phrase = phrase.begin();
        std::string::const_iterator end_phrase = phrase.end();

        return boost::spirit::qi::phrase_parse(
            lexer.begin(begin_phrase, end_phrase),
            lexer.end(),
            rule[boost::phoenix::ref(result) = _1],
            in_state("WS")[lexer.self]
        );
    }

    bool set_parse_ship_meter(std::string phrase, MeterType& result) {
        typename parse::enum_parser_rule<MeterType>::type& rule = parse::set_ship_part_meter_type_enum();
        const parse::lexer& lexer = lexer.instance();
        boost::spirit::qi::in_state_type in_state;
        boost::spirit::qi::_1_type _1;

        std::string::const_iterator begin_phrase = phrase.begin();
        std::string::const_iterator end_phrase = phrase.end();

        return boost::spirit::qi::phrase_parse(
            lexer.begin(begin_phrase, end_phrase),
            lexer.end(),
            rule[boost::phoenix::ref(result) = _1],
            in_state("WS")[lexer.self]
        );
    }
};

BOOST_FIXTURE_TEST_SUITE(EnumParser, EnumParserFixture)

BOOST_AUTO_TEST_CASE(CaptureResultParser) {
    CaptureResult result;

    // XXX: No enum number value to validate enum coverage.

    BOOST_CHECK(parse("Capture", result));
    BOOST_CHECK(result == CR_CAPTURE);

    BOOST_CHECK(parse("Destroy", result));
    BOOST_CHECK(result == CR_DESTROY);

    BOOST_CHECK(parse("Retain", result));
    BOOST_CHECK(result == CR_RETAIN);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_CAPTURE_RESULT;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_CAPTURE_RESULT);
}

BOOST_AUTO_TEST_CASE(CombatFighterTypeParser) {
    CombatFighterType result;

    BOOST_CHECK(parse("Interceptor", result));
    BOOST_CHECK(result == INTERCEPTOR);

    BOOST_CHECK(parse("Bomber", result));
    BOOST_CHECK(result == BOMBER);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_COMBAT_FIGHTER_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_COMBAT_FIGHTER_TYPE);
}

BOOST_AUTO_TEST_CASE(EmpireAffiliationTypeParser) {
    EmpireAffiliationType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_AFFIL_TYPES == 4, "Untested enumeration value.");

    BOOST_CHECK(parse("TheEmpire", result));
    BOOST_CHECK(result == AFFIL_SELF);

    BOOST_CHECK(parse("EnemyOf", result));
    BOOST_CHECK(result == AFFIL_ENEMY);

    BOOST_CHECK(parse("AllyOf", result));
    BOOST_CHECK(result == AFFIL_ALLY);

    BOOST_CHECK(parse("AnyEmpire", result));
    BOOST_CHECK(result == AFFIL_ANY);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_EMPIRE_AFFIL_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_EMPIRE_AFFIL_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_AFFIL_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_AFFIL_TYPES);
}

BOOST_AUTO_TEST_CASE(NonShipPartMeterTypeParser) {
    MeterType result;

    // XXX: METER_SIZE not handled, still used?
    // XXX: HAPPINESS meters not handled, still used?
    // XXX: REBEL_TROOPS meters not handled, still used?
    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    BOOST_CHECK(parse_nonship_meter("TargetPopulation", result));
    BOOST_CHECK(result == METER_TARGET_POPULATION);
    BOOST_CHECK(set_parse_nonship_meter("SetTargetPopulation", result));
    BOOST_CHECK(result == METER_TARGET_POPULATION);

    BOOST_CHECK(parse_nonship_meter("TargetIndustry", result));
    BOOST_CHECK(result == METER_TARGET_INDUSTRY);
    BOOST_CHECK(set_parse_nonship_meter("SetTargetIndustry", result));
    BOOST_CHECK(result == METER_TARGET_INDUSTRY);

    BOOST_CHECK(parse_nonship_meter("TargetResearch", result));
    BOOST_CHECK(result == METER_TARGET_RESEARCH);
    BOOST_CHECK(set_parse_nonship_meter("SetTargetResearch", result));
    BOOST_CHECK(result == METER_TARGET_RESEARCH);

    BOOST_CHECK(parse_nonship_meter("TargetTrade", result));
    BOOST_CHECK(result == METER_TARGET_TRADE);
    BOOST_CHECK(set_parse_nonship_meter("SetTargetTrade", result));
    BOOST_CHECK(result == METER_TARGET_TRADE);

    BOOST_CHECK(parse_nonship_meter("TargetConstruction", result));
    BOOST_CHECK(result == METER_TARGET_CONSTRUCTION);
    BOOST_CHECK(set_parse_nonship_meter("SetTargetConstruction", result));
    BOOST_CHECK(result == METER_TARGET_CONSTRUCTION);

    BOOST_CHECK(parse_nonship_meter("MaxFuel", result));
    BOOST_CHECK(result == METER_MAX_FUEL);
    BOOST_CHECK(set_parse_nonship_meter("SetMaxFuel", result));
    BOOST_CHECK(result == METER_MAX_FUEL);

    BOOST_CHECK(parse_nonship_meter("MaxShield", result));
    BOOST_CHECK(result == METER_MAX_SHIELD);
    BOOST_CHECK(set_parse_nonship_meter("SetMaxShield", result));
    BOOST_CHECK(result == METER_MAX_SHIELD);

    BOOST_CHECK(parse_nonship_meter("MaxStructure", result));
    BOOST_CHECK(result == METER_MAX_STRUCTURE);
    BOOST_CHECK(set_parse_nonship_meter("SetMaxStructure", result));
    BOOST_CHECK(result == METER_MAX_STRUCTURE);

    BOOST_CHECK(parse_nonship_meter("MaxDefense", result));
    BOOST_CHECK(result == METER_MAX_DEFENSE);
    BOOST_CHECK(set_parse_nonship_meter("SetMaxDefense", result));
    BOOST_CHECK(result == METER_MAX_DEFENSE);

    BOOST_CHECK(parse_nonship_meter("MaxTroops", result));
    BOOST_CHECK(result == METER_MAX_TROOPS);
    BOOST_CHECK(set_parse_nonship_meter("SetMaxTroops", result));
    BOOST_CHECK(result == METER_MAX_TROOPS);

    BOOST_CHECK(parse_nonship_meter("Population", result));
    BOOST_CHECK(result == METER_POPULATION);
    BOOST_CHECK(set_parse_nonship_meter("SetPopulation", result));
    BOOST_CHECK(result == METER_POPULATION);

    BOOST_CHECK(parse_nonship_meter("Industry", result));
    BOOST_CHECK(result == METER_INDUSTRY);
    BOOST_CHECK(set_parse_nonship_meter("SetIndustry", result));
    BOOST_CHECK(result == METER_INDUSTRY);

    BOOST_CHECK(parse_nonship_meter("Research", result));
    BOOST_CHECK(result == METER_RESEARCH);
    BOOST_CHECK(set_parse_nonship_meter("SetResearch", result));
    BOOST_CHECK(result == METER_RESEARCH);

    BOOST_CHECK(parse_nonship_meter("Trade", result));
    BOOST_CHECK(result == METER_TRADE);
    BOOST_CHECK(set_parse_nonship_meter("SetTrade", result));
    BOOST_CHECK(result == METER_TRADE);

    BOOST_CHECK(parse_nonship_meter("Construction", result));
    BOOST_CHECK(result == METER_CONSTRUCTION);
    BOOST_CHECK(set_parse_nonship_meter("SetConstruction", result));
    BOOST_CHECK(result == METER_CONSTRUCTION);

    BOOST_CHECK(parse_nonship_meter("Fuel", result));
    BOOST_CHECK(result == METER_FUEL);
    BOOST_CHECK(set_parse_nonship_meter("SetFuel", result));
    BOOST_CHECK(result == METER_FUEL);

    BOOST_CHECK(parse_nonship_meter("Shield", result));
    BOOST_CHECK(result == METER_SHIELD);
    BOOST_CHECK(set_parse_nonship_meter("SetShield", result));
    BOOST_CHECK(result == METER_SHIELD);

    BOOST_CHECK(parse_nonship_meter("Structure", result));
    BOOST_CHECK(result == METER_STRUCTURE);
    BOOST_CHECK(set_parse_nonship_meter("SetStructure", result));
    BOOST_CHECK(result == METER_STRUCTURE);

    BOOST_CHECK(parse_nonship_meter("Defense", result));
    BOOST_CHECK(result == METER_DEFENSE);
    BOOST_CHECK(set_parse_nonship_meter("SetDefense", result));
    BOOST_CHECK(result == METER_DEFENSE);

    BOOST_CHECK(parse_nonship_meter("Troops", result));
    BOOST_CHECK(result == METER_TROOPS);
    BOOST_CHECK(set_parse_nonship_meter("SetTroops", result));
    BOOST_CHECK(result == METER_TROOPS);

    BOOST_CHECK(parse_nonship_meter("Supply", result));
    BOOST_CHECK(result == METER_SUPPLY);
    BOOST_CHECK(set_parse_nonship_meter("SetSupply", result));
    BOOST_CHECK(result == METER_SUPPLY);

    BOOST_CHECK(parse_nonship_meter("Stealth", result));
    BOOST_CHECK(result == METER_STEALTH);
    BOOST_CHECK(set_parse_nonship_meter("SetStealth", result));
    BOOST_CHECK(result == METER_STEALTH);

    BOOST_CHECK(parse_nonship_meter("Detection", result));
    BOOST_CHECK(result == METER_DETECTION);
    BOOST_CHECK(set_parse_nonship_meter("SetDetection", result));
    BOOST_CHECK(result == METER_DETECTION);

    BOOST_CHECK(parse_nonship_meter("BattleSpeed", result));
    BOOST_CHECK(result == METER_BATTLE_SPEED);
    BOOST_CHECK(set_parse_nonship_meter("SetBattleSpeed", result));
    BOOST_CHECK(result == METER_BATTLE_SPEED);

    BOOST_CHECK(parse_nonship_meter("StarlaneSpeed", result));
    BOOST_CHECK(result == METER_STARLANE_SPEED);
    BOOST_CHECK(set_parse_nonship_meter("SetStarlaneSpeed", result));
    BOOST_CHECK(result == METER_STARLANE_SPEED);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_METER_TYPE;
    BOOST_CHECK(!parse_nonship_meter("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_METER_TYPE);
    result = INVALID_METER_TYPE;
    BOOST_CHECK(!set_parse_nonship_meter("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_METER_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_METER_TYPES;
    BOOST_CHECK(!parse_nonship_meter("DoesNotExist", result));
    BOOST_CHECK(result == NUM_METER_TYPES);
    result = NUM_METER_TYPES;
    BOOST_CHECK(!set_parse_nonship_meter("DoesNotExist", result));
    BOOST_CHECK(result == NUM_METER_TYPES);
}

BOOST_AUTO_TEST_CASE(PlanetEnvironmentParser) {
    PlanetEnvironment result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_ENVIRONMENTS == 5, "Untested enumeration value.");

    BOOST_CHECK(parse("Uninhabitable", result));
    BOOST_CHECK(result == PE_UNINHABITABLE);

    BOOST_CHECK(parse("Hostile", result));
    BOOST_CHECK(result == PE_HOSTILE);

    BOOST_CHECK(parse("Poor", result));
    BOOST_CHECK(result == PE_POOR);

    BOOST_CHECK(parse("Adequate", result));
    BOOST_CHECK(result == PE_ADEQUATE);

    BOOST_CHECK(parse("Good", result));
    BOOST_CHECK(result == PE_GOOD);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_PLANET_ENVIRONMENT;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_PLANET_ENVIRONMENT);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_PLANET_ENVIRONMENTS;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_PLANET_ENVIRONMENTS);
}

BOOST_AUTO_TEST_CASE(PlanetSizeParser) {
    PlanetSize result;

    // Literal is number of tests, not number of enums.
    // XXX: SZ_NOWORLD has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_SIZES == 7 + 1, "Untested enumeration value.");

    BOOST_CHECK(parse("Tiny", result));
    BOOST_CHECK(result == SZ_TINY);

    BOOST_CHECK(parse("Small", result));
    BOOST_CHECK(result == SZ_SMALL);

    BOOST_CHECK(parse("Medium", result));
    BOOST_CHECK(result == SZ_MEDIUM);

    BOOST_CHECK(parse("Large", result));
    BOOST_CHECK(result == SZ_LARGE);

    BOOST_CHECK(parse("Huge", result));
    BOOST_CHECK(result == SZ_HUGE);

    BOOST_CHECK(parse("Asteroids", result));
    BOOST_CHECK(result == SZ_ASTEROIDS);

    BOOST_CHECK(parse("GasGiant", result));
    BOOST_CHECK(result == SZ_GASGIANT);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_PLANET_SIZE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_PLANET_SIZE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_PLANET_SIZES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_PLANET_SIZES);
}

BOOST_AUTO_TEST_CASE(PlanetTypeParser) {
    PlanetType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_PLANET_TYPES == 11, "Untested enumeration value.");

    BOOST_CHECK(parse("Swamp", result));
    BOOST_CHECK(result == PT_SWAMP);

    BOOST_CHECK(parse("Toxic", result));
    BOOST_CHECK(result == PT_TOXIC);

    BOOST_CHECK(parse("Inferno", result));
    BOOST_CHECK(result == PT_INFERNO);

    BOOST_CHECK(parse("Radiated", result));
    BOOST_CHECK(result == PT_RADIATED);

    BOOST_CHECK(parse("Barren", result));
    BOOST_CHECK(result == PT_BARREN);

    BOOST_CHECK(parse("Tundra", result));
    BOOST_CHECK(result == PT_TUNDRA);

    BOOST_CHECK(parse("Desert", result));
    BOOST_CHECK(result == PT_DESERT);

    BOOST_CHECK(parse("Terran", result));
    BOOST_CHECK(result == PT_TERRAN);

    BOOST_CHECK(parse("Ocean", result));
    BOOST_CHECK(result == PT_OCEAN);

    BOOST_CHECK(parse("Asteroids", result));
    BOOST_CHECK(result == PT_ASTEROIDS);

    BOOST_CHECK(parse("GasGiant", result));
    BOOST_CHECK(result == PT_GASGIANT);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_PLANET_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_PLANET_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_PLANET_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_PLANET_TYPES);
}

BOOST_AUTO_TEST_CASE(ShipPartsClassParser) {
    ShipPartClass result;

    // Literal is number of tests, not number of enums.
    // XXX: PC_GENERAL has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(NUM_SHIP_PART_CLASSES == 13 + 1, "Untested enumeration value.");

    BOOST_CHECK(parse("ShortRange", result));
    BOOST_CHECK(result == PC_SHORT_RANGE);

    BOOST_CHECK(parse("Missiles", result));
    BOOST_CHECK(result == PC_MISSILES);

    BOOST_CHECK(parse("Fighters", result));
    BOOST_CHECK(result == PC_FIGHTERS);

    BOOST_CHECK(parse("PointDefense", result));
    BOOST_CHECK(result == PC_POINT_DEFENSE);

    BOOST_CHECK(parse("Shield", result));
    BOOST_CHECK(result == PC_SHIELD);

    BOOST_CHECK(parse("Armour", result));
    BOOST_CHECK(result == PC_ARMOUR);

    BOOST_CHECK(parse("Troops", result));
    BOOST_CHECK(result == PC_TROOPS);

    BOOST_CHECK(parse("Detection", result));
    BOOST_CHECK(result == PC_DETECTION);

    BOOST_CHECK(parse("Stealth", result));
    BOOST_CHECK(result == PC_STEALTH);

    BOOST_CHECK(parse("Fuel", result));
    BOOST_CHECK(result == PC_FUEL);

    BOOST_CHECK(parse("Colony", result));
    BOOST_CHECK(result == PC_COLONY);

    BOOST_CHECK(parse("BattleSpeed", result));
    BOOST_CHECK(result == PC_BATTLE_SPEED);

    BOOST_CHECK(parse("StarlaneSpeed", result));
    BOOST_CHECK(result == PC_STARLANE_SPEED);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_SHIP_PART_CLASS;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_SHIP_PART_CLASS);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_SHIP_PART_CLASSES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_SHIP_PART_CLASSES);
}

BOOST_AUTO_TEST_CASE(ShipPartMeterTypeParser) {
    MeterType result;

    // XXX: METER_SIZE not handled, still used?
    // XXX: No enum number value to validate enum coverage.
    //      Maybe the Meter enum should be split.

    BOOST_CHECK(parse_ship_meter("Damage", result));
    BOOST_CHECK(result == METER_DAMAGE);
    BOOST_CHECK(set_parse_ship_meter("SetDamage", result));
    BOOST_CHECK(result == METER_DAMAGE);

    BOOST_CHECK(parse_ship_meter("ROF", result));
    BOOST_CHECK(result == METER_ROF);
    BOOST_CHECK(set_parse_ship_meter("SetROF", result));
    BOOST_CHECK(result == METER_ROF);

    BOOST_CHECK(parse_ship_meter("Range", result));
    BOOST_CHECK(result == METER_RANGE);
    BOOST_CHECK(set_parse_ship_meter("SetRange", result));
    BOOST_CHECK(result == METER_RANGE);

    BOOST_CHECK(parse_ship_meter("Speed", result));
    BOOST_CHECK(result == METER_SPEED);
    BOOST_CHECK(set_parse_ship_meter("SetSpeed", result));
    BOOST_CHECK(result == METER_SPEED);

    BOOST_CHECK(parse_ship_meter("Capacity", result));
    BOOST_CHECK(result == METER_CAPACITY);
    BOOST_CHECK(set_parse_ship_meter("SetCapacity", result));
    BOOST_CHECK(result == METER_CAPACITY);

    BOOST_CHECK(parse_ship_meter("AntiShipDamage", result));
    BOOST_CHECK(result == METER_ANTI_SHIP_DAMAGE);
    BOOST_CHECK(set_parse_ship_meter("SetAntiShipDamage", result));
    BOOST_CHECK(result == METER_ANTI_SHIP_DAMAGE);

    BOOST_CHECK(parse_ship_meter("AntiFighterDamage", result));
    BOOST_CHECK(result == METER_ANTI_FIGHTER_DAMAGE);
    BOOST_CHECK(set_parse_ship_meter("SetAntiFighterDamage", result));
    BOOST_CHECK(result == METER_ANTI_FIGHTER_DAMAGE);

    BOOST_CHECK(parse_ship_meter("LaunchRate", result));
    BOOST_CHECK(result == METER_LAUNCH_RATE);
    BOOST_CHECK(set_parse_ship_meter("SetLaunchRate", result));
    BOOST_CHECK(result == METER_LAUNCH_RATE);

    BOOST_CHECK(parse_ship_meter("FighterWeaponRange", result));
    BOOST_CHECK(result == METER_FIGHTER_WEAPON_RANGE);
    BOOST_CHECK(set_parse_ship_meter("SetFighterWeaponRange", result));
    BOOST_CHECK(result == METER_FIGHTER_WEAPON_RANGE);
}

BOOST_AUTO_TEST_CASE(ShipSlotTypeParser) {
    ShipSlotType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_SHIP_SLOT_TYPES == 2, "Untested enumeration value.");

    BOOST_CHECK(parse("External", result));
    BOOST_CHECK(result == SL_EXTERNAL);

    BOOST_CHECK(parse("Internal", result));
    BOOST_CHECK(result == SL_INTERNAL);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_SHIP_SLOT_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_SHIP_SLOT_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_SHIP_SLOT_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_SHIP_SLOT_TYPES);
}

BOOST_AUTO_TEST_CASE(StatisticTypeParser) {
    ValueRef::StatisticType result;

    // XXX: No enum number value to validate enum coverage.

    BOOST_CHECK(parse("Count", result));
    BOOST_CHECK(result == ValueRef::COUNT);

    BOOST_CHECK(parse("Sum", result));
    BOOST_CHECK(result == ValueRef::SUM);

    BOOST_CHECK(parse("Mean", result));
    BOOST_CHECK(result == ValueRef::MEAN);

    BOOST_CHECK(parse("Rms", result));
    BOOST_CHECK(result == ValueRef::RMS);

    BOOST_CHECK(parse("Mode", result));
    BOOST_CHECK(result == ValueRef::MODE);

    BOOST_CHECK(parse("Max", result));
    BOOST_CHECK(result == ValueRef::MAX);

    BOOST_CHECK(parse("Min", result));
    BOOST_CHECK(result == ValueRef::MIN);

    BOOST_CHECK(parse("Spread", result));
    BOOST_CHECK(result == ValueRef::SPREAD);

    BOOST_CHECK(parse("StDev", result));
    BOOST_CHECK(result == ValueRef::STDEV);

    BOOST_CHECK(parse("Product", result));
    BOOST_CHECK(result == ValueRef::PRODUCT);

    // XXX: is not modifying result the correct behaviour?
    result = ValueRef::INVALID_STATISTIC_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == ValueRef::INVALID_STATISTIC_TYPE);
}

BOOST_AUTO_TEST_CASE(StarTypeParser) {
    StarType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_STAR_TYPES == 8, "Untested enumeration value.");

    BOOST_CHECK(parse("Blue", result));
    BOOST_CHECK(result == STAR_BLUE);

    BOOST_CHECK(parse("White", result));
    BOOST_CHECK(result == STAR_WHITE);

    BOOST_CHECK(parse("Yellow", result));
    BOOST_CHECK(result == STAR_YELLOW);

    BOOST_CHECK(parse("Orange", result));
    BOOST_CHECK(result == STAR_ORANGE);

    BOOST_CHECK(parse("Red", result));
    BOOST_CHECK(result == STAR_RED);

    BOOST_CHECK(parse("Neutron", result));
    BOOST_CHECK(result == STAR_NEUTRON);

    BOOST_CHECK(parse("BlackHole", result));
    BOOST_CHECK(result == STAR_BLACK);

    BOOST_CHECK(parse("NoStar", result));
    BOOST_CHECK(result == STAR_NONE);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_STAR_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_STAR_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_STAR_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_STAR_TYPES);
}

BOOST_AUTO_TEST_CASE(TechTypeParser) {
    TechType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_TECH_TYPES == 3, "Untested enumeration value.");

    BOOST_CHECK(parse("Theory", result));
    BOOST_CHECK(result == TT_THEORY);

    BOOST_CHECK(parse("Application", result));
    BOOST_CHECK(result == TT_APPLICATION);

    BOOST_CHECK(parse("Refinement", result));
    BOOST_CHECK(result == TT_REFINEMENT);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_TECH_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_TECH_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_TECH_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_TECH_TYPES);
}

BOOST_AUTO_TEST_CASE(UnlockableItemTypeParser)
{
    UnlockableItemType result;

    // Literal is number of tests, not number of enums.
    BOOST_REQUIRE_MESSAGE(NUM_UNLOCKABLE_ITEM_TYPES == 5, "Untested enumeration value.");

    BOOST_CHECK(parse("Building", result));
    BOOST_CHECK(result == UIT_BUILDING);

    BOOST_CHECK(parse("ShipPart", result));
    BOOST_CHECK(result == UIT_SHIP_PART);

    BOOST_CHECK(parse("ShipHull", result));
    BOOST_CHECK(result == UIT_SHIP_HULL);

    BOOST_CHECK(parse("ShipDesign", result));
    BOOST_CHECK(result == UIT_SHIP_DESIGN);

    BOOST_CHECK(parse("Tech", result));
    BOOST_CHECK(result == UIT_TECH);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_UNLOCKABLE_ITEM_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_UNLOCKABLE_ITEM_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_UNLOCKABLE_ITEM_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_UNLOCKABLE_ITEM_TYPES);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeParser)
{
    UniverseObjectType result;

    // Literal is number of tests, not number of enums.
    // XXX: OBJ_FIELD has no token, so no test, +1
    BOOST_REQUIRE_MESSAGE(NUM_OBJ_TYPES == 7 + 1, "Untested enumeration value.");

    BOOST_CHECK(parse("Building", result));
    BOOST_CHECK(result == OBJ_BUILDING);

    BOOST_CHECK(parse("Ship", result));
    BOOST_CHECK(result == OBJ_SHIP);

    BOOST_CHECK(parse("Fleet ", result));
    BOOST_CHECK(result == OBJ_FLEET );

    BOOST_CHECK(parse("Planet", result));
    BOOST_CHECK(result == OBJ_PLANET);

    BOOST_CHECK(parse("PopulationCenter", result));
    BOOST_CHECK(result == OBJ_POP_CENTER);

    BOOST_CHECK(parse("ProductionCenter", result));
    BOOST_CHECK(result == OBJ_PROD_CENTER);

    BOOST_CHECK(parse("System", result));
    BOOST_CHECK(result == OBJ_SYSTEM);

    // XXX: is not modifying result the correct behaviour?
    result = INVALID_UNIVERSE_OBJECT_TYPE;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == INVALID_UNIVERSE_OBJECT_TYPE);

    // XXX: is not modifying result the correct behaviour?
    result = NUM_OBJ_TYPES;
    BOOST_CHECK(!parse("DoesNotExist", result));
    BOOST_CHECK(result == NUM_OBJ_TYPES);
}

BOOST_AUTO_TEST_SUITE_END()
