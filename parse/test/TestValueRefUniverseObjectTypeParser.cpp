#include <boost/test/unit_test.hpp>

#include "ValueRefParser.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefUniverseObjectTypeFixture {
    ValueRefUniverseObjectTypeFixture():
        result(0)
    {}

    ~ValueRefUniverseObjectTypeFixture() {
        delete result;
    }

    bool parse(std::string phrase, ValueRef::ValueRefBase<UniverseObjectType>*& result) {
        parse::value_ref_parser_rule<UniverseObjectType>::type& rule = parse::value_ref_parser<UniverseObjectType>();
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

    ValueRef::ValueRefBase<UniverseObjectType>* result;
    const ValueRef::Operation<UniverseObjectType>* operation1;
    const ValueRef::Operation<UniverseObjectType>* operation2;
    const ValueRef::Operation<UniverseObjectType>* operation3;
    const ValueRef::Operation<UniverseObjectType>* operation4;
    const ValueRef::Operation<UniverseObjectType>* operation5;
    const ValueRef::Operation<UniverseObjectType>* operation6;
    const ValueRef::Constant<UniverseObjectType>* value;
};

BOOST_FIXTURE_TEST_SUITE(ValueRefUniverseObjectTypeParser, ValueRefUniverseObjectTypeFixture)

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserBuilding) {
    BOOST_CHECK(parse("Building", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_BUILDING);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserShip) {
    BOOST_CHECK(parse("Ship", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_SHIP);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserFleet) {
    BOOST_CHECK(parse("Fleet", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_FLEET);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserPlanet) {
    BOOST_CHECK(parse("Planet", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_PLANET);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserPopulationCenter) {
    BOOST_CHECK(parse("PopulationCenter", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_POP_CENTER);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserProductionCenter) {
    BOOST_CHECK(parse("ProductionCenter", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_PROD_CENTER);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserSystem) {
    BOOST_CHECK(parse("System", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_SYSTEM);
}

BOOST_AUTO_TEST_CASE(UniverseObjectTypeLiteralParserField) {
    BOOST_CHECK(parse("Field", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<UniverseObjectType>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), OBJ_FIELD);
}

BOOST_AUTO_TEST_SUITE_END()
