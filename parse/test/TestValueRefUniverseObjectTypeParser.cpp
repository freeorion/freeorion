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
    const ValueRef::Statistic<UniverseObjectType>* statistic;
    const ValueRef::Variable<UniverseObjectType>* variable;
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

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserSource) {
    BOOST_CHECK(parse("Source.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("Source"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::SOURCE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserTarget) {
    BOOST_CHECK(parse("Target.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("Target"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserLocalCandidate) {
    BOOST_CHECK(parse("LocalCandidate.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("LocalCandidate"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserRootCandidate) {
    BOOST_CHECK(parse("RootCandidate.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("RootCandidate"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserFleetSource) {
    BOOST_CHECK(parse("Source.Fleet.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("Source"), adobe::name_t("Fleet"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::SOURCE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 3
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserSytemTarget) {
    BOOST_CHECK(parse("Target.System.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("Target"), adobe::name_t("System"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 3
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserLocalCandidatePlanet) {
    BOOST_CHECK(parse("LocalCandidate.Planet.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("LocalCandidate"), adobe::name_t("Planet"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 3
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserRootCandidateFleet) {
    BOOST_CHECK(parse("RootCandidate.Fleet.ObjectType", result));
    adobe::name_t property[] = { adobe::name_t("RootCandidate"), adobe::name_t("Fleet"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 3
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserErrornousInput) {
    BOOST_CHECK(!parse("Source", result));
    BOOST_CHECK(!parse("Source .", result));
    BOOST_CHECK(!parse("Target", result));
    BOOST_CHECK(!parse("Target .", result));
    BOOST_CHECK(!parse("Source . System", result));
    BOOST_CHECK(!parse("Source . System .", result));
    BOOST_CHECK(!parse("Target . System", result));
    BOOST_CHECK(!parse("Target . System .", result));
    BOOST_CHECK(!parse("Source . Planet", result));
    BOOST_CHECK(!parse("Source . Planet .", result));
    BOOST_CHECK(!parse("Target . Planet", result));
    BOOST_CHECK(!parse("Target . Planet .", result));
    BOOST_CHECK(!parse("Source . Fleet", result));
    BOOST_CHECK(!parse("Source . Fleet .", result));
    BOOST_CHECK(!parse("Target . Fleet", result));
    BOOST_CHECK(!parse("Target . Fleet .", result));
    BOOST_CHECK(!parse("LocalCandidate", result));
    BOOST_CHECK(!parse("LocalCandidate .", result));
    BOOST_CHECK(!parse("RootCandidate", result));
    BOOST_CHECK(!parse("RootCandidate .", result));
    BOOST_CHECK(!parse("LocalCandidate . System", result));
    BOOST_CHECK(!parse("LocalCandidate . System .", result));
    BOOST_CHECK(!parse("RootCandidate . System", result));
    BOOST_CHECK(!parse("RootCandidate . System .", result));
    BOOST_CHECK(!parse("LocalCandidate . Planet", result));
    BOOST_CHECK(!parse("LocalCandidate . Planet .", result));
    BOOST_CHECK(!parse("RootCandidate . Planet", result));
    BOOST_CHECK(!parse("RootCandidate . Planet .", result));
    BOOST_CHECK(!parse("LocalCandidate . Fleet", result));
    BOOST_CHECK(!parse("LocalCandidate . Fleet .", result));
    BOOST_CHECK(!parse("RootCandidate . Fleet", result));
    BOOST_CHECK(!parse("RootCandidate . Fleet .", result));
}

BOOST_AUTO_TEST_CASE(UniverseObjectStatisticParserFleet) {
    BOOST_CHECK(parse("Mode Property = Fleet.ObjectType Condition = All", result));
    adobe::name_t property[] = { adobe::name_t("Fleet"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Statistic<UniverseObjectType>), typeid(*result));
    statistic = dynamic_cast<const ValueRef::Statistic<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(statistic->GetStatisticType(), ValueRef::MODE);
    BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        statistic->PropertyName().begin(), statistic->PropertyName().end(),
        property, property + 2
    );
    BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->SamplingCondition())));
}

BOOST_AUTO_TEST_CASE(UniverseObjectStatisticParserPlanet) {
    BOOST_CHECK(parse("Mode Property = Planet.ObjectType All", result));
    adobe::name_t property[] = { adobe::name_t("Planet"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Statistic<UniverseObjectType>), typeid(*result));
    statistic = dynamic_cast<const ValueRef::Statistic<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(statistic->GetStatisticType(), ValueRef::MODE);
    BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        statistic->PropertyName().begin(), statistic->PropertyName().end(),
        property, property + 2
    );
    BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->SamplingCondition())));
}

BOOST_AUTO_TEST_CASE(UniverseObjectStatisticParserSystem) {
    BOOST_CHECK(parse("Mode System.ObjectType Condition = All", result));
    adobe::name_t property[] = { adobe::name_t("System"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Statistic<UniverseObjectType>), typeid(*result));
    statistic = dynamic_cast<const ValueRef::Statistic<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(statistic->GetStatisticType(), ValueRef::MODE);
    BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        statistic->PropertyName().begin(), statistic->PropertyName().end(),
        property, property + 2
    );
    BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->SamplingCondition())));
}

BOOST_AUTO_TEST_CASE(UniverseObjectStatisticParserFleetShort) {
    BOOST_CHECK(parse("Mode Fleet.ObjectType All", result));
    adobe::name_t property[] = { adobe::name_t("Fleet"), adobe::name_t("ObjectType") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Statistic<UniverseObjectType>), typeid(*result));
    statistic = dynamic_cast<const ValueRef::Statistic<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(statistic->GetStatisticType(), ValueRef::MODE);
    BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        statistic->PropertyName().begin(), statistic->PropertyName().end(),
        property, property + 2
    );
    BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->SamplingCondition())));
}

BOOST_AUTO_TEST_CASE(UniverseObjectStatisticParserErrornousInput) {
    BOOST_CHECK(!parse("Mode", result));
    BOOST_CHECK(!parse("Mode Property", result));
    BOOST_CHECK(!parse("Mode Property =", result));
    BOOST_CHECK(!parse("Mode Property = Fleet", result));
    BOOST_CHECK(!parse("Mode Property = Fleet .", result));
    BOOST_CHECK(!parse("Mode Property = Fleet . ObjectType", result));
    BOOST_CHECK(!parse("Mode Property = Fleet . ObjectType Condition", result));
    BOOST_CHECK(!parse("Mode Property = Fleet . ObjectType Condition =", result));
    BOOST_CHECK(!parse("Mode Fleet", result));
    BOOST_CHECK(!parse("Mode Fleet .", result));
    BOOST_CHECK(!parse("Mode Fleet . ObjectType", result));
    BOOST_CHECK(!parse("Mode Fleet . ObjectType Condition", result));
    BOOST_CHECK(!parse("Mode Fleet . ObjectType Condition =", result));
    BOOST_CHECK(!parse("Mode Property = Planet", result));
    BOOST_CHECK(!parse("Mode Property = Planet .", result));
    BOOST_CHECK(!parse("Mode Property = Planet . ObjectType", result));
    BOOST_CHECK(!parse("Mode Property = Planet . ObjectType Condition", result));
    BOOST_CHECK(!parse("Mode Property = Planet . ObjectType Condition =", result));
    BOOST_CHECK(!parse("Mode Planet", result));
    BOOST_CHECK(!parse("Mode Planet .", result));
    BOOST_CHECK(!parse("Mode Planet . ObjectType", result));
    BOOST_CHECK(!parse("Mode Planet . ObjectType Condition", result));
    BOOST_CHECK(!parse("Mode Planet . ObjectType Condition =", result));
    BOOST_CHECK(!parse("Mode Property = System", result));
    BOOST_CHECK(!parse("Mode Property = System .", result));
    BOOST_CHECK(!parse("Mode Property = System . ObjectType", result));
    BOOST_CHECK(!parse("Mode Property = System . ObjectType Condition", result));
    BOOST_CHECK(!parse("Mode Property = System . ObjectType Condition =", result));
    BOOST_CHECK(!parse("Mode System", result));
    BOOST_CHECK(!parse("Mode System .", result));
    BOOST_CHECK(!parse("Mode System . ObjectType", result));
    BOOST_CHECK(!parse("Mode System . ObjectType Condition", result));
    BOOST_CHECK(!parse("Mode System . ObjectType Condition =", result));
}

BOOST_AUTO_TEST_SUITE_END()
