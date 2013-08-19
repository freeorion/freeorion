#include <boost/test/unit_test.hpp>

#include "ValueRefParser.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefIntFixture: boost::unit_test::test_observer {
    ValueRefIntFixture():
        result(0) {
        boost::unit_test::framework::register_observer(*this);
    }

    ~ValueRefIntFixture() {
        boost::unit_test::framework::deregister_observer(*this);
        delete result;
    }

    void assertion_result(bool passed) {
        if(!passed && result) {
            printTree(result, 0);

            delete result;
            result = 0;
        }
    }

    void printTree(const ValueRef::ValueRefBase<int>* root, int depth) {
        if(depth > 10) {
            std::cout << "Tree print overflow" << std::endl;
        }

        if(const ValueRef::Constant<int>* value = dynamic_cast<const ValueRef::Constant<int>*>(root)) {
            std::cout << std::string(depth * 2, ' ') << value->Value() << std::endl;
        }

        if(const ValueRef::Operation<int>* operation = dynamic_cast<const ValueRef::Operation<int>*>(root)) {
            std::cout << std::string(depth * 2, ' ') << operation->GetOpType() << std::endl;
            printTree(operation->LHS(), depth + 1);
            printTree(operation->RHS(), depth + 1);
        }
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
    "NumShips",
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

BOOST_AUTO_TEST_CASE(DoubleLiteralParserSineOperation) {
    // XXX: sin not documented, radians or degree as input?
    BOOST_CHECK(parse("sin(90.0)", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::SINE);

    // XXX: Unary operations have no right hand side or left hand side parameters.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 90);
}

// XXX value_ref_parser_rule<int> throws an expectation_failure, the enum parser does not. is this intended?
BOOST_AUTO_TEST_CASE(IntLiteralParserMalformed) {
    BOOST_CHECK_THROW(parse("-", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("(1", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("(-", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("((", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("((1", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("((1)", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("(1)))", result), std::runtime_error);
    BOOST_CHECK(!result);
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

/*
Term:
  -1+2

Expected AST:
  PLUS
    NEGATE
      1
    2
*/
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
    BOOST_CHECK_EQUAL(value->Value(), 1);

    // 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);

}

/*
Term:
  -1+2-8+5

Expected AST:
  PLUS
    MINUS
      PLUS
        NEGATE
          1
        2
      8
    5
*/
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
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation4->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation4->LHS());
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

/*
Term:
  4*3+2

Expected AST:
  PLUS
    TIMES
      4
      3
    2
*/
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

/*
Term:
  -1+2/-7

Expected AST:
  PLUS
    NEGATE
      1
    DIVIDE
      2
      NEGATE
        7
*/
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
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation3->LHS());
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

/*
Term:
  -1/3+-6*7

Expected AST:
  PLUS
    DIVIDE
      NEGATE
        1
      3
    TIMES
      NEGATE
        6
      7
*/
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

/*
Term:
  1+3*-4/6*(9-1)

Expected AST:
  PLUS
    1
    TIMES
      DIVIDE
        TIMES
          3
          NEGATE
            4
        6
      MINUS
        9
        1
*/
BOOST_AUTO_TEST_CASE(IntArithmeticParser6) {
    BOOST_CHECK(parse("1+3*-4/6*(9-1)", result));

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

    // (3*-4) / 6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::DIVIDE);

    // 3 * (-4)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation3->LHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<int>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::TIMES);

    // 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation4->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 3);

    // -4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<int>), typeid(*operation4->RHS()));
    operation5 = dynamic_cast<const ValueRef::Operation<int>*>(operation4->RHS());
    BOOST_CHECK_EQUAL(operation5->GetOpType(), ValueRef::NEGATE);

    // 4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation5->LHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation5->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 4);

    // 6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<int>), typeid(*operation3->RHS()));
    value = dynamic_cast<const ValueRef::Constant<int>*>(operation3->RHS());
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
    BOOST_CHECK_THROW(parse("1 +", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("-1 +", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("-5 2", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("5 *", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("* 5", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("7 / * 5", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("7 - 5 * 3 / - + 2", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;
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
    // All phrases are missing the attribute value.
    BOOST_FOREACH(const ReferenceType& reference, referenceTypes) {
        // eg: "Target"
        BOOST_CHECK_THROW(parse(reference.second, result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "LocalCandidate."
        BOOST_CHECK_THROW(parse(reference.second + ".", result), std::runtime_error);
        BOOST_CHECK(!result);

        BOOST_FOREACH(const std::string& containerType, containerTypes) {
            // eg: "RootCandidate.Planet"
            BOOST_CHECK_THROW(parse(reference.second + "." + containerType, result), std::runtime_error);
            BOOST_CHECK(!result);

            // eg: "Target.Fleet."
            BOOST_CHECK_THROW(parse(reference.second + "." + containerType + ".", result), std::runtime_error);
            BOOST_CHECK(!result);
        }
    }
}

// XXX: Statistic COUNT, UNIQUE_COUNT and IF not tested.

BOOST_AUTO_TEST_CASE(IntStatisticParserTypeless) {
    BOOST_FOREACH(const StatisticType& statisticType, statisticTypes) {
        BOOST_FOREACH(const std::string& attribute, attributes) {
            adobe::name_t property[] = { adobe::name_t(attribute.c_str()) };

            boost::array<std::string, 4> phrases = {{
                // long variant
                statisticType.second + " Property = " + attribute + " Condition = All",
                // Check variant with missing "Condition =" keyword.
                statisticType.second + " Property = " + attribute + " All",
                // Check variant with missing "Property =" keyword.
                statisticType.second + " " + attribute + " Condition = All",
                // Check short variant
                statisticType.second + " " + attribute + " All"
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
                    statisticType.second + " Property = " + containerType + "." + attribute + " Condition = All",
                    // Check variant with missing "Condition =" keyword.
                    statisticType.second + " Property = " + containerType + "." + attribute + " All",
                    // Check variant with missing "Property =" keyword.
                    statisticType.second + " " + containerType + "." + attribute + " Condition = All",
                    // Check short variant
                    statisticType.second + " " + containerType + "." + attribute + " All"
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
    BOOST_FOREACH(const StatisticType& statisticType, statisticTypes) {
        // eg: "Number"
        BOOST_CHECK_THROW(parse(statisticType.second, result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "Mean Condition"
        BOOST_CHECK_THROW(parse(statisticType.second + " Condition", result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "RMS Condition ="
        BOOST_CHECK_THROW(parse(statisticType.second + " Condition =", result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "Mean Property"
        BOOST_CHECK_THROW(parse(statisticType.second + " Property", result), std::runtime_error);
        BOOST_CHECK(!result);

        // eg: "RMS Property ="
        BOOST_CHECK_THROW(parse(statisticType.second + " Property =", result), std::runtime_error);
        BOOST_CHECK(!result);

        BOOST_FOREACH(const std::string& attribute, attributes) {
            // missing or incomplete condition
            // eg: "Mean Owner"
            BOOST_CHECK_THROW(parse(statisticType.second + " " + attribute, result), std::runtime_error);
            BOOST_CHECK(!result);

            // eg: "Mean Property = Owner"
            BOOST_CHECK_THROW(parse(statisticType.second + " Property = " + attribute, result), std::runtime_error);
            BOOST_CHECK(!result);

            // eg: "Mean Property = Owner Condition"
            BOOST_CHECK_THROW(parse(statisticType.second + " Property = " + attribute + " Condition", result), std::runtime_error);
            BOOST_CHECK(!result);

            // eg: "Mean Property = Owner Condition ="
            BOOST_CHECK_THROW(parse(statisticType.second + " Property = " + attribute + " Condition =", result), std::runtime_error);
            BOOST_CHECK(!result);

            BOOST_FOREACH(const std::string& containerType, containerTypes) {
                // eg: "Mean Fleet.Owner"
                BOOST_CHECK_THROW(parse(statisticType.second + " " + containerType + "." + attribute, result), std::runtime_error);
                BOOST_CHECK(!result);

                // eg: "Mean Property = Planet.Owner"
                BOOST_CHECK_THROW(parse(statisticType.second + " Property = " + containerType + "." + attribute, result), std::runtime_error);
                BOOST_CHECK(!result);

                // eg: "Mean Property = Fleet.Owner Condition"
                BOOST_CHECK_THROW(parse(statisticType.second + " Property = " + containerType + "." + attribute + " Condition", result), std::runtime_error);
                BOOST_CHECK(!result);

                // eg: "Mean Property = Planet.Owner Condition ="
                BOOST_CHECK_THROW(parse(statisticType.second + " Property = " + containerType + "." + attribute + " Condition =", result), std::runtime_error);
                BOOST_CHECK(!result);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
