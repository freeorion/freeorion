#include <boost/test/unit_test.hpp>

#include "ValueRefParser.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefIntFixture {
    ValueRefIntFixture():
        result(0) {
    }

    ~ValueRefIntFixture() {
        delete result;
    }

bool parse(std::string phrase, ValueRef::ValueRefBase<int>*& result) {
    parse::value_ref_parser_rule<int>::type& rule = parse::value_ref_parser<int>();
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

    typedef std::pair<ValueRef::ReferenceType, std::string> ReferenceType;
    typedef std::pair<ValueRef::StatisticType, std::string> StatisticType;

    static const boost::array<ReferenceType, 4>  referenceTypes;
    static const boost::array<StatisticType, 9>  statisticTypes;
    static const boost::array<std::string, 3>  containerTypes;
    static const boost::array<std::string, 13> attributes;

    ValueRef::ValueRefBase<int>* result;
    const ValueRef::Operation<int>* operation1;
    const ValueRef::Operation<int>* operation2;
    const ValueRef::Operation<int>* operation3;
    const ValueRef::Operation<int>* operation4;
    const ValueRef::Operation<int>* operation5;
    const ValueRef::Operation<int>* operation6;
    const ValueRef::Constant<int>* value;
    const ValueRef::Statistic<int>* statistic;
    const ValueRef::Variable<int>* variable;
};

const boost::array<ValueRefIntFixture::ReferenceType, 4>  ValueRefIntFixture::referenceTypes = {{
    std::make_pair(ValueRef::SOURCE_REFERENCE, "Source"),
    std::make_pair(ValueRef::EFFECT_TARGET_REFERENCE, "Target"),
    std::make_pair(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE, "LocalCandidate"),
    std::make_pair(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE, "RootCandidate")
}};


const boost::array<ValueRefIntFixture::StatisticType, 9> ValueRefIntFixture::statisticTypes = {{
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

const boost::array<std::string, 3> ValueRefIntFixture::containerTypes = {{
    "Fleet",
    "Planet",
    "System"
}};

const boost::array<std::string, 13> ValueRefIntFixture::attributes = {{
    "Age",
    "CreationTurn",
    "DesignID",
    "FinalDestinationID",
    "FleetID",
    "ID",
    "NextSystemID",
    "NumShips"
    "Owner",
    "PlanetID",
    "PreviousSystemID",
    "ProducedByEmpireID",
    "SystemID"
}};

BOOST_FIXTURE_TEST_SUITE(ValueRefIntParser, ValueRefIntFixture)

BOOST_AUTO_TEST_CASE(IntLiteralParserInteger) {
    BOOST_CHECK(parse("7309", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<int>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 7309);
}

BOOST_AUTO_TEST_CASE(IntLiteralParserNegativeInteger) {
    BOOST_CHECK(parse("-1343", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1343);
}

// XXX: What is the desired real to int casting behaviour (to nearest int, floor, ...)
BOOST_AUTO_TEST_CASE(IntLiteralParserReal) {
    BOOST_CHECK(parse("14.234", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<int>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 14);
}

BOOST_AUTO_TEST_CASE(IntLiteralParserNegativeReal) {
    BOOST_CHECK(parse("-13.7143", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 13);
}

BOOST_AUTO_TEST_CASE(IntLiteralParserBracketedInteger) {
    BOOST_CHECK(parse("(595)", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<int>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 595);
}

BOOST_AUTO_TEST_CASE(IntLiteralParserNegativeBracketedInteger) {
    BOOST_CHECK(parse("(-1532)", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1532);
}

BOOST_AUTO_TEST_CASE(IntLiteralParserDoubleBracketedInteger) {
    BOOST_CHECK(parse("((143))", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<int>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 143);
}

BOOST_AUTO_TEST_CASE(IntLiteralParserNegativeBracketedReal) {
    BOOST_CHECK(parse("(-(6754.20))", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 6754);
}

// XXX value_ref_parser_rule<int> throws an expectation_failure, the enum parser does not. is this intended?
BOOST_AUTO_TEST_CASE(IntLiteralParserErrornousInput) {
    BOOST_CHECK(!parse("-", result));
    BOOST_CHECK(!parse("(1", result));
    BOOST_CHECK(!parse("(-", result));
    BOOST_CHECK(!parse("((", result));
    BOOST_CHECK(!parse("((1", result));
    BOOST_CHECK(!parse("((1)", result));
    BOOST_CHECK(!parse("(1)))", result));
}

// Multiple operations should be evaluated in a certain order.
// The priorities are in decending order:
//
// ()
// */
// +-
//
// Priorities of the same order are evaluated left to right.
// XXX: Operation Type priorities are not documented.

// Term:
// -1+2
// Expected AST:
// +    #
// |\   #
// | \  #
// -  2 #
// |    #
// 1    #
BOOST_AUTO_TEST_CASE(IntArithmeticParser1) {
    BOOST_CHECK(parse("-1+2", result));

    // (-1) + 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // -1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::NEGATE);

    // 1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);

    // 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);

}

// Term:
// -1+2-8+5
// Expected AST:
// +   #
// |\  #
// - 5 #
// |\  #
// + 8 #
// |\  #
// - 2 #
// |   #
// 1
BOOST_AUTO_TEST_CASE(IntArithmeticParser2) {
    BOOST_CHECK(parse("-1+2-8+5", result));

    // (-1+2-8) + 5
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // (-1+2) - 8
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::MINUS);

    // (-1) + 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::PLUS);

    // -1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation3->LHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<int>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::NEGATE);

    // 1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation4->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation4->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 1);

    // 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation3->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);

    // 8
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation2->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 8);

    // 5
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 5);
}

// Term:
// 4*3+2
// Expected AST:
// +    #
// |\   #
// * 2  #
// |\   #
// 4 3  #
BOOST_AUTO_TEST_CASE(IntArithmeticParser3) {
    BOOST_CHECK(parse("4*3+2", result));

    // (4*3) + 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // 4 * 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::TIMES);

    // 4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 4);

    // 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation2->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 3);

    // 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);
}

// Term:
// -1+2/-7
// Expected AST:
//  +        #
//  |\       #
// -1 (/)    #
//    | \    #
//    2 -7   #
BOOST_AUTO_TEST_CASE(IntArithmeticParser4) {
    BOOST_CHECK(parse("-1+2/-7", result));

    // -1 + (2/-7)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // -1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::NEGATE);

    // 1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1);

    // 2 / -7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->RHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::DIVIDE);

    // 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation3->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);

    // -7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation3->RHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<int>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::NEGATE);

    // 7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation4->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 7);
}

// Term:
// -1/3+-6*7
// Expected AST:
//    +        #
//   / \       #
// (/)  *      #
// /\   | \    #
// - 3  -  7   #
// |    |      #
// 1    6      #
BOOST_AUTO_TEST_CASE(IntArithmeticParser5) {
    BOOST_CHECK(parse("-1/3+-6*7", result));

    // (-1/3) + (-6*7)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // (-1) / 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::DIVIDE);

    // -1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::NEGATE);

    // 1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation3->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1);

    // 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation2->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 3);

    // (-6) * 7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->RHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::TIMES);

    // -6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation4->LHS()));
    operation5 = dynamic_cast<const ValueRef::Operation<int>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(operation5->GetOpType(), ValueRef::NEGATE);

    // 6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation5->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation5->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 6);

    // 7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation4->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation4->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 7);
}

// Term:
// 1+3*-4/6*(9-1)
// Expected AST:
//     +      #
//    /\      #
//   1  *     # 
//    / |     #
//  (/) -     #
//  /\  |\    #
// *  6 9 1   #
// | \        #
// 3  -       #
//    |       #
//    4       #
//
//
BOOST_AUTO_TEST_CASE(IntArithmeticParser6) {
    BOOST_CHECK(parse("1+3*-4*6*(9-1)", result));

    // 1 + (3*-4/6*(9-1))
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // 1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1);

    // (3*-4/6) * ((9-1))
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->RHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::TIMES);

    // 3 * (-4/6)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::TIMES);

    // 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation3->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 3);

    // (-4) / 6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation3->RHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<int>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::DIVIDE);

    // -4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation4->LHS()));
    operation5 = dynamic_cast<const ValueRef::Operation<int>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(operation5->GetOpType(), ValueRef::NEGATE);

    // 4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation5->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation5->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 4);

    // 6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation4->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation4->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 6);

    // 9 - 1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation2->RHS()));
    operation6 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(operation6->GetOpType(), ValueRef::MINUS);

    // 9
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation6->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation6->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 9);

    // 1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation6->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation6->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 1);
}

BOOST_AUTO_TEST_CASE(IntArithmeticParserMalformed) {
    BOOST_CHECK(!parse("1 +", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("-1 +", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("-5 2", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("5 + - - 2", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("5 *", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("* 5", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("7 / * 5", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("7 - 5 * 3 / - + 2", result));
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(IntVariableParserCurrentTurn) {
    BOOST_CHECK(parse("CurrentTurn", result));
    adobe::name_t property[] = { adobe::name_t("CurrentTurn") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<int>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<int>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(IntVariableParserValue) {
    BOOST_CHECK(parse("Value", result));
    adobe::name_t property[] = { adobe::name_t("Value") };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<int>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<int>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(IntVariableParserTypeless) {
    BOOST_FOREACH(const ReferenceType& reference, referenceTypes) {
        BOOST_FOREACH(const std::string& attribute, attributes) {
            std::string phrase = reference.second + "." + attribute;
            BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse: \"" + phrase + "\"");
            adobe::name_t property[] = {
                adobe::name_t(reference.second.c_str()),
                adobe::name_t(attribute.c_str())
            };

            BOOST_CHECK_EQUAL(typeid(ValueRef::Variable<int>), typeid(*result));
            if(variable = dynamic_cast<const ValueRef::Variable<int>*>(result)) {
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

BOOST_AUTO_TEST_CASE(IntVariableParserTyped) {
    BOOST_FOREACH(const ReferenceType& reference, referenceTypes) {
        BOOST_FOREACH(const std::string& type, containerTypes) {
            BOOST_FOREACH(const std::string& attribute, attributes) {
                std::string phrase = reference.second + "." + type + "." + attribute;
                BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse: \"" + phrase + "\"");
                adobe::name_t property[] = {
                    adobe::name_t(reference.second.c_str()),
                    adobe::name_t(type.c_str()),
                    adobe::name_t(attribute.c_str())
                };

                BOOST_CHECK_EQUAL(typeid(ValueRef::Variable<int>), typeid(*result));
                if(variable = dynamic_cast<const ValueRef::Variable<int>*>(result)) {
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

// Term:
// 2*Source.NumShips-Target.NumShips
// Expected AST:
//     -                #
//    /\                #
//   *  Target.NumShips #
//  / \                 #
// 2   Source.NumShips  #
//
BOOST_AUTO_TEST_CASE(IntArithmeticVariableParser) {
    BOOST_CHECK(parse("2*Source.NumShips-Target.NumShips", result));

    // (2*Source.NumShips) - Target.NumShips
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::MINUS);

    // 2 * Source.NumShips
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::TIMES);

    // 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);

    // Source.NumShips
    {
        adobe::name_t property[] = {
            adobe::name_t("Source"),
            adobe::name_t("NumShips")
        };
        BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<int>), typeid(*operation2->RHS()));
        variable = dynamic_cast<const ValueRef::Variable<int>*>(operation2->RHS());
        BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::SOURCE_REFERENCE);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            variable->PropertyName().begin(), variable->PropertyName().end(),
            property, property + 2
        );
    }

    // Target.NumShips
    {
        adobe::name_t property[] = {
            adobe::name_t("Target"),
            adobe::name_t("NumShips")
        };
        BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<int>), typeid(*operation1->RHS()));
        variable = dynamic_cast<const ValueRef::Variable<int>*>(operation1->RHS());
        BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            variable->PropertyName().begin(), variable->PropertyName().end(),
            property, property + 2
        );
    }
}

BOOST_AUTO_TEST_CASE(IntVariableParserMalformed) {
    BOOST_CHECK(!parse("Source", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Source .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Source . System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Source . System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target . System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target . System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Source . Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Source . Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target . Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target . Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Source . Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Source . Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target . Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Target . Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate . System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate . System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate . System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate . System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate . Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate . Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate . Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate . Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate . Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("LocalCandidate . Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate . Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RootCandidate . Fleet .", result));
    BOOST_CHECK(!result);
}

// XXX: Statistic COUNT, UNIQUE_COUNT and IF not tested.

BOOST_AUTO_TEST_CASE(IntStatisticParserTypeless) {
    BOOST_FOREACH(const StatisticType& statisticType, statisticTypes) {
        BOOST_FOREACH(const std::string& attribute, attributes) {
            adobe::name_t property[] = { adobe::name_t(attribute.c_str()) };

            boost::array<std::string, 4> phrases = {{
                // long variant
                statisticType.first + " Property = " + attribute + " Condition = All",
                // Check variant with missing "Condition =" keyword.
                statisticType.first + " Property = " + attribute + " All",
                // Check variant with missing "Property =" keyword.
                statisticType.first + " " + attribute + " Condition = All",
                // Check short variant
                statisticType.first + " " + attribute + " All"
            }};

            BOOST_FOREACH(const std::string& phrase, phrases) {
                BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse \"" + phrase + "\"");

                BOOST_CHECK_EQUAL(typeid(ValueRef::Statistic<int>), typeid(*result));
                if(statistic = dynamic_cast<const ValueRef::Statistic<int>*>(result)) {
                    BOOST_CHECK_EQUAL(statistic->GetStatisticType(), statisticType.first);
                    BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
                    BOOST_CHECK_EQUAL_COLLECTIONS(
                        statistic->PropertyName().begin(), statistic->PropertyName().end(),
                        property, property + 1
                    );
                    BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->SamplingCondition())));
                }

                delete result;
                result = 0;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(IntStatisticParserTyped) {
    BOOST_FOREACH(const StatisticType& statisticType, statisticTypes) {
        BOOST_FOREACH(const std::string& containerType, containerTypes) {
            BOOST_FOREACH(const std::string& attribute, attributes) {
                adobe::name_t property[] = {
                    adobe::name_t(containerType.c_str()),
                    adobe::name_t(attribute.c_str())
                };

                boost::array<std::string, 4> phrases = {{
                    // long variant
                    statisticType.first + " Property = " + containerType + "." + attribute + " Condition = All",
                    // Check variant with missing "Condition =" keyword.
                    statisticType.first + " Property = " + containerType + "." + attribute + " All",
                    // Check variant with missing "Property =" keyword.
                    statisticType.first + " " + containerType + "." + attribute + " Condition = All",
                    // Check short variant
                    statisticType.first + " " + containerType + "." + attribute + " All"
                }};

                BOOST_FOREACH(const std::string& phrase, phrases) {
                    BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse \"" + phrase + "\"");

                    BOOST_CHECK_EQUAL(typeid(ValueRef::Statistic<int>), typeid(*result));
                    if(statistic = dynamic_cast<const ValueRef::Statistic<int>*>(result)) {
                        BOOST_CHECK_EQUAL(statistic->GetStatisticType(), statisticType.first);
                        BOOST_CHECK_EQUAL(statistic->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
                        BOOST_CHECK_EQUAL_COLLECTIONS(
                            statistic->PropertyName().begin(), statistic->PropertyName().end(),
                            property, property + 2
                        );
                        BOOST_CHECK_EQUAL(typeid(Condition::All), typeid(*(statistic->SamplingCondition())));
                    }

                    delete result;
                    result = 0;
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(IntStatisticParserMalformed) {
    BOOST_CHECK(!parse("Number", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Number Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Number Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Sum NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mean NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("RMS NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Mode NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Max NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Min NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Spread NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("StDev NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Fleet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Planet . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System .", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product System . NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Owner", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Owner Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Owner Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product ID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product ID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product ID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product CreationTurn", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product CreationTurn Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product CreationTurn Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Age", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Age Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Age Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product ProducedByEmpireID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product ProducedByEmpireID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product ProducedByEmpireID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product DesignID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product DesignID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product DesignID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product FleetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product FleetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product FleetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product PlanetID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product PlanetID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product PlanetID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product SystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product SystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product SystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product FinalDestinationID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product FinalDestinationID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product FinalDestinationID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product NextSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product NextSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product NextSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product PreviousSystemID", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product PreviousSystemID Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product PreviousSystemID Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product Property = NumShips Condition =", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product NumShips", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product NumShips Condition", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("Product NumShips Condition =", result));
    BOOST_CHECK(!result);

}

BOOST_AUTO_TEST_SUITE_END()
