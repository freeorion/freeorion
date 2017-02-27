#include <boost/test/unit_test.hpp>

#include "parse/ValueRefParser.h"
#include "universe/Enums.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefUniverseObjectTypeFixture {
    ValueRefUniverseObjectTypeFixture():
        result(0),
        operation1(0),
        operation2(0),
        operation3(0),
        operation4(0),
        operation5(0),
        operation6(0),
        value(0),
        statistic(0),
        variable(0)       
    {}

    ~ValueRefUniverseObjectTypeFixture() {
        delete result;
    }

    bool parse(std::string phrase, ValueRef::ValueRefBase<UniverseObjectType>*& result) {
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
            parse::detail::universe_object_type_rules().expr[boost::phoenix::ref(result) = _1] > eoi,
            in_state("WS")[lexer.self]
        );

        return matched && begin == end;
    }

    typedef std::pair<ValueRef::ReferenceType, std::string> ReferenceType;
    typedef std::pair<ValueRef::StatisticType, std::string> StatisticType;

    static const std::array<ReferenceType, 4> referenceTypes;
    static const std::array<StatisticType, 1> statisticTypes;
    static const std::array<std::string, 3> containerTypes;
    static const std::array<std::string, 1> attributes;

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

const std::array<ValueRefUniverseObjectTypeFixture::StatisticType, 1> ValueRefUniverseObjectTypeFixture::statisticTypes = {{
    std::make_pair(ValueRef::MODE,    "Mode")
}};

const std::array<ValueRefUniverseObjectTypeFixture::ReferenceType, 4> ValueRefUniverseObjectTypeFixture::referenceTypes = {{
    std::make_pair(ValueRef::SOURCE_REFERENCE, "Source"),
    std::make_pair(ValueRef::EFFECT_TARGET_REFERENCE, "Target"),
    std::make_pair(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE, "LocalCandidate"),
    std::make_pair(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE, "RootCandidate")
}};

const std::array<std::string, 3> ValueRefUniverseObjectTypeFixture::containerTypes = {{
    "Fleet",
    "Planet",
    "System"
}};

const std::array<std::string, 1> ValueRefUniverseObjectTypeFixture::attributes = {{
    "ObjectType"
}};


BOOST_FIXTURE_TEST_SUITE(TestValueRefUniverseObjectTypeParser, ValueRefUniverseObjectTypeFixture)

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
    std::string property[] = { "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::SOURCE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserTarget) {
    BOOST_CHECK(parse("Target.ObjectType", result));
    std::string property[] = { "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserLocalCandidate) {
    BOOST_CHECK(parse("LocalCandidate.ObjectType", result));
    std::string property[] = { "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserRootCandidate) {
    BOOST_CHECK(parse("RootCandidate.ObjectType", result));
    std::string property[] = { "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserFleetSource) {
    BOOST_CHECK(parse("Source.Fleet.ObjectType", result));
    std::string property[] = { "Fleet", "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::SOURCE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserSytemTarget) {
    BOOST_CHECK(parse("Target.System.ObjectType", result));
    std::string property[] = { "System", "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserLocalCandidatePlanet) {
    BOOST_CHECK(parse("LocalCandidate.Planet.ObjectType", result));
    std::string property[] = { "Planet", "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserRootCandidateFleet) {
    BOOST_CHECK(parse("RootCandidate.Fleet.ObjectType", result));
    std::string property[] = { "Fleet", "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<UniverseObjectType>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 2
    );
}

BOOST_AUTO_TEST_CASE(UniverseObjectVariableParserMalformed) {
    for (const ReferenceType& reference : referenceTypes) {
        BOOST_CHECK_THROW(parse(reference.second, result), std::runtime_error);
        BOOST_CHECK(!result);

        BOOST_CHECK_THROW(parse(reference.second + ".", result), std::runtime_error);
        BOOST_CHECK(!result);

        for (const std::string& type : containerTypes) {
            BOOST_CHECK_THROW(parse(reference.second + "." + type, result), std::runtime_error);
            BOOST_CHECK(!result);

            BOOST_CHECK_THROW(parse(reference.second + "." + type + ".", result), std::runtime_error);
            BOOST_CHECK(!result);
        }
    }
}

BOOST_AUTO_TEST_CASE(UniverseObjectStatisticParserFleet) {
    BOOST_CHECK(parse("Statistic Mode Value = Fleet.ObjectType Condition = All", result));
    std::string property[] = { "Fleet", "ObjectType" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Statistic<UniverseObjectType>), typeid(*result));
    statistic = dynamic_cast<const ValueRef::Statistic<UniverseObjectType>*>(result);
    BOOST_CHECK_EQUAL(statistic->GetStatisticType(), ValueRef::MODE);
    BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        statistic->PropertyName().begin(), statistic->PropertyName().end(),
        property, property + 2
    );
    BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->GetSamplingCondition())));
}

BOOST_AUTO_TEST_CASE(UniverseObjectStatisticParserMalformed) {
    for (const StatisticType& statisticType : statisticTypes) {
        // eg: "Statistic Number"
        BOOST_CHECK_THROW(parse("Statistic " + statisticType.second, result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "Statistic Mean Condition"
        BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Condition", result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "Statistic RMS Condition ="
        BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Condition =", result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "Statistic Mean Value"
        BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value", result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "Statistic RMS Value ="
        BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value =", result), std::runtime_error);
        BOOST_CHECK(!result);

        for (const std::string& attribute : attributes) {
            // missing or incomplete condition
            // eg: "Statistic Mean Owner"
            BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " " + attribute, result), std::runtime_error);
            BOOST_CHECK(!result);

            // eg: "Statistic Mean Value = Owner"
            BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value = " + attribute, result), std::runtime_error);
            BOOST_CHECK(!result);

            // eg: "Statistic Mean Value = Owner Condition"
            BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value = " + attribute + " Condition", result), std::runtime_error);
            BOOST_CHECK(!result);

            // eg: "Statistic Mean Value = Owner Condition ="
            BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value = " + attribute + " Condition =", result), std::runtime_error);
            BOOST_CHECK(!result);

            for (const std::string& containerType : containerTypes) {
                // eg: "Statistic Mean Fleet.Owner"
                BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " " + containerType + "." + attribute, result), std::runtime_error);
                BOOST_CHECK(!result);

                // eg: "Statistic Mean Value = Planet.Owner"
                BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value = " + containerType + "." + attribute, result), std::runtime_error);
                BOOST_CHECK(!result);

                // eg: "Statistic Mean Value = Fleet.Owner Condition"
                BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value = " + containerType + "." + attribute + " Condition", result), std::runtime_error);
                BOOST_CHECK(!result);

                // eg: "Statistic Mean Value = Planet.Owner Condition ="
                BOOST_CHECK_THROW(parse("Statistic " + statisticType.second + " Value = " + containerType + "." + attribute + " Condition =", result), std::runtime_error);
                BOOST_CHECK(!result);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
