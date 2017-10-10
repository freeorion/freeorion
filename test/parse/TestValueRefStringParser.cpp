#include <boost/test/unit_test.hpp>

#include "parse/ValueRefParser.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefStringFixture {
    ValueRefStringFixture():
        result(0),
        value(0),
        statistic(0),
        variable(0)
    {}

    ~ValueRefStringFixture() {
        delete result;
    }

    bool parse(std::string phrase, ValueRef::ValueRefBase<std::string>*& result) {
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
            string_grammar[boost::phoenix::ref(result) = _1] > eoi,
            in_state("WS")[lexer.self]
        );

        return matched && begin == end;
    }

    typedef std::pair<ValueRef::ReferenceType, std::string> ReferenceType;
    typedef std::pair<ValueRef::StatisticType, std::string> StatisticType;

    static const std::array<ReferenceType, 4> referenceTypes;
    static const std::array<StatisticType, 9> statisticTypes;
    static const std::array<std::string, 3> containerTypes;
    static const std::array<std::string, 13> attributes;

    ValueRef::ValueRefBase<std::string>* result;
    const ValueRef::Constant<std::string>* value;
    const ValueRef::Statistic<std::string>* statistic;
    const ValueRef::Variable<std::string>* variable;
};

const std::array<ValueRefStringFixture::ReferenceType, 4> ValueRefStringFixture::referenceTypes = {{
    std::make_pair(ValueRef::SOURCE_REFERENCE, "Source"),
    std::make_pair(ValueRef::EFFECT_TARGET_REFERENCE, "Target"),
    std::make_pair(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE, "LocalCandidate"),
    std::make_pair(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE, "RootCandidate")
}};


const std::array<ValueRefStringFixture::StatisticType, 9> ValueRefStringFixture::statisticTypes = {{
    std::make_pair(ValueRef::MAX,     "Max"),
    std::make_pair(ValueRef::MEAN,    "Mean"),
    std::make_pair(ValueRef::MIN,     "Min"),
    std::make_pair(ValueRef::MODE,    "Mode"),
    std::make_pair(ValueRef::PRODUCT, "Product"),
    std::make_pair(ValueRef::RMS,     "RMS"),
    std::make_pair(ValueRef::SPREAD,  "Spread"),
    std::make_pair(ValueRef::STDEV,   "StDev"),
    std::make_pair(ValueRef::SUM,     "Sum")
}};

const std::array<std::string, 3> ValueRefStringFixture::containerTypes = {{
    "Fleet",
    "Planet",
    "System"
}};

const std::array<std::string, 13> ValueRefStringFixture::attributes = {{
    "Age",
    "CreationTurn",
    "DesignID",
    "FinalDestinationID",
    "FleetID",
    "ID",
    "NextSystemID",
    "NumShips",
    "Owner",
    "PlanetID",
    "PreviousSystemID",
    "ProducedByEmpireID",
    "SystemID"
}};


BOOST_FIXTURE_TEST_SUITE(TestValueRefStringParser, ValueRefStringFixture)

BOOST_AUTO_TEST_CASE(StringLiteralParserString) {
    BOOST_CHECK(parse("Tiny", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<std::string>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<std::string>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), "Tiny");
}

BOOST_AUTO_TEST_CASE(StringLiteralParserSpaceString) {
    BOOST_CHECK(parse("\"A little bit of text with spaces.\"", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<std::string>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<std::string>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), "A little bit of text with spaces.");
}

BOOST_AUTO_TEST_CASE(StringLiteralParserInteger) {
    BOOST_CHECK(parse("5", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<std::string>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<std::string>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), "5");
}

BOOST_AUTO_TEST_CASE(StringLiteralParserReal) {
    BOOST_CHECK(parse("7.21", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<std::string>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<std::string>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), "7.21");
}

BOOST_AUTO_TEST_CASE(StringVariableParserCurrentTurn) {
    BOOST_CHECK(parse("CurrentTurn", result));
    std::string property[] = { "CurrentTurn" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::StringCast<int>), typeid(*result));
    variable = dynamic_cast<const ValueRef::StringCast<int>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(StringLiteralParserMalformed) {
    BOOST_CHECK_THROW(parse("\"A bit of text with missing quotes, whoops", result), std::runtime_error);
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(StringVariableParserValue) {
    BOOST_CHECK(parse("Value", result));
    std::string property[] = { "Value" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<std::string>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<std::string>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(StringVariableParserTypeless) {
    for (const ReferenceType& reference : referenceTypes) {
        for (const std::string& attribute : attributes) {
            std::string phrase = reference.second + "." + attribute;
            BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse: \"" + phrase + "\"");
            std::string property[] = {
                reference.second,
                attribute
            };

            BOOST_CHECK_EQUAL(typeid(ValueRef::StringCast<int>), typeid(*result));
            if(variable = dynamic_cast<const ValueRef::StringCast<int>*>(result)) {
                BOOST_CHECK_EQUAL(variable->GetReferenceType(), reference.first);
                BOOST_CHECK_EQUAL_COLLECTIONS(
                    variable->PropertyName().begin(), variable->PropertyName().end(),
                    property, property + 2
                );
            }

            delete result;
            result = 0;
        }
    }
}

BOOST_AUTO_TEST_CASE(StringVariableParserTyped) {
    for (const ReferenceType& reference : referenceTypes) {
        for (const std::string& type : containerTypes) {
            for (const std::string& attribute : attributes) {
                std::string phrase = reference.second + "." + type + "." + attribute;
                BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse: \"" + phrase + "\"");
                std::string property[] = {
                    reference.second,
                    type,
                    attribute
                };

                BOOST_CHECK_EQUAL(typeid(ValueRef::StringCast<int>), typeid(*result));
                if(variable = dynamic_cast<const ValueRef::StringCast<int>*>(result)) {
                    BOOST_CHECK_EQUAL(variable->GetReferenceType(), reference.first);
                    BOOST_CHECK_EQUAL_COLLECTIONS(
                        variable->PropertyName().begin(), variable->PropertyName().end(),
                        property, property + 3
                    );
                }

                delete result;
                result = 0;
            }
        }
    }
}

// XXX: Statistic COUNT, UNIQUE_COUNT and IF not tested.

BOOST_AUTO_TEST_CASE(StringStatisticParserTypeless) {
    for (const StatisticType& statisticType : statisticTypes) {
        for (const std::string& attribute : attributes) {
            std::string property[] = { attribute };

            std::string phrase = "Statistic " + statisticType.second + " Value = " + attribute + " Condition = All";

            BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse \"" + phrase + "\"");

            BOOST_CHECK_EQUAL(typeid(ValueRef::Statistic<std::string>), typeid(*result));
            if(statistic = dynamic_cast<const ValueRef::Statistic<std::string>*>(result)) {
                BOOST_CHECK_EQUAL(statistic->GetStatisticType(), statisticType.first);
                BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
                BOOST_CHECK_EQUAL_COLLECTIONS(
                    statistic->PropertyName().begin(), statistic->PropertyName().end(),
                    property, property + 1
                );
                BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->GetSamplingCondition())));
            }

            delete result;
            result = 0;
        }
    }
}

BOOST_AUTO_TEST_CASE(StringStatisticParserTyped) {
    for (const StatisticType& statisticType : statisticTypes) {
        for (const std::string& containerType : containerTypes) {
            for (const std::string& attribute : attributes) {
                std::string property[] = {
                    containerType,
                    attribute
                };

                std::string phrase = "Statistic " + statisticType.second + " Value = " + containerType + "." + attribute + " Condition = All";

                BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse \"" + phrase + "\"");

                BOOST_CHECK_EQUAL(typeid(ValueRef::Statistic<std::string>), typeid(*result));
                if(statistic = dynamic_cast<const ValueRef::Statistic<std::string>*>(result)) {
                    BOOST_CHECK_EQUAL(statistic->GetStatisticType(), statisticType.first);
                    BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
                    BOOST_CHECK_EQUAL_COLLECTIONS(
                        statistic->PropertyName().begin(), statistic->PropertyName().end(),
                        property, property + 2
                    );
                    BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->GetSamplingCondition())));
                }

                delete result;
                result = 0;
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
