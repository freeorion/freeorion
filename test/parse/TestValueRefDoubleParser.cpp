#include <boost/test/unit_test.hpp>

#include "parse/ValueRefParser.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefDoubleFixture: boost::unit_test::test_observer {
    ValueRefDoubleFixture():
        result(0) {
        boost::unit_test::framework::register_observer(*this);
    }

    ~ValueRefDoubleFixture() {
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

    void printTree(const ValueRef::ValueRefBase<double>* root, int depth) {
        if(depth > 10) {
            std::cout << "Tree print overflow" << std::endl;
        }

        if(const ValueRef::Constant<double>* value = dynamic_cast<const ValueRef::Constant<double>*>(root)) {
            std::cout << std::string(depth * 2, ' ') << value->Value() << std::endl;
        }

        if(const ValueRef::Operation<double>* operation = dynamic_cast<const ValueRef::Operation<double>*>(root)) {
            std::cout << std::string(depth * 2, ' ') << operation->GetOpType() << std::endl;
            printTree(operation->LHS(), depth + 1);
            printTree(operation->RHS(), depth + 1);
        }
    }

    bool parse(std::string phrase, ValueRef::ValueRefBase<double>*& result) {
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
            double_rules[boost::phoenix::ref(result) = _1] > eoi,
            in_state("WS")[lexer.self]
        );

        return matched && begin == end;
    }

    typedef std::pair<ValueRef::ReferenceType, std::string> ReferenceType;
    typedef std::pair<ValueRef::StatisticType, std::string> StatisticType;

    static const std::array<ReferenceType, 4> referenceTypes;
    static const std::array<StatisticType, 9> statisticTypes;
    static const std::array<std::string, 3> containerTypes;
    static const std::array<std::string, 36> attributes;

    ValueRef::ValueRefBase<double>* result;
    const ValueRef::Operation<double>* operation1;
    const ValueRef::Operation<double>* operation2;
    const ValueRef::Operation<double>* operation3;
    const ValueRef::Operation<double>* operation4;
    const ValueRef::Operation<double>* operation5;
    const ValueRef::Operation<double>* operation6;
    const ValueRef::Constant<double>* value;
    const ValueRef::Statistic<double>* statistic;
    const ValueRef::Variable<double>* variable;
};

const std::array<ValueRefDoubleFixture::ReferenceType, 4> ValueRefDoubleFixture::referenceTypes = {{
    std::make_pair(ValueRef::SOURCE_REFERENCE, "Source"),
    std::make_pair(ValueRef::EFFECT_TARGET_REFERENCE, "Target"),
    std::make_pair(ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE, "LocalCandidate"),
    std::make_pair(ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE, "RootCandidate")
}};

const std::array<ValueRefDoubleFixture::StatisticType, 9> ValueRefDoubleFixture::statisticTypes = {{
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

const std::array<std::string, 3> ValueRefDoubleFixture::containerTypes = {{
    "Fleet",
    "Planet",
    "System"
}};

const std::array<std::string, 36> ValueRefDoubleFixture::attributes = {{
    "Industry",
    "TargetIndustry",
    "Research",
    "TargetResearch",
    "Trade",
    "TargetTrade",
    "Construction",
    "TargetConstruction",
    "Population",
    "TargetPopulation",
    "TargetHappiness",
    "Happiness",
    "MaxFuel",
    "Fuel",
    "MaxShield",
    "Shield",
    "MaxDefense",
    "Defense",
    "MaxTroops",
    "Troops",
    "RebelTroops",
    "MaxStructure",
    "Structure",
    "Supply",
    "Stealth",
    "Detection",
    "BattleSpeed",
    "StarlaneSpeed",
    "TradeStockpile",
    "DistanceToSource",
    "X",
    "Y",
    "SizeAsDouble",
    "HabitableSize",
    "NextTurnPopGrowth",
    "Size",
    "DistanceFromOriginalType"
}};


BOOST_FIXTURE_TEST_SUITE(TestValueRefDoubleParser, ValueRefDoubleFixture)

BOOST_AUTO_TEST_CASE(DoubleLiteralParserInteger) {
    BOOST_CHECK(parse("7309", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<double>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 7309.0);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserNegativeInteger) {
    BOOST_CHECK(parse("-1343", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side parameters.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1343.0);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserReal) {
    BOOST_CHECK(parse("14.234", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<double>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 14.234);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserNegativeReal) {
    BOOST_CHECK(parse("-13.7143", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side parameters.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 13.7143);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserRealFractionOnly) {
    BOOST_CHECK(parse(".234", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<double>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), .234);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserNegativeRealFractionOnly) {
    BOOST_CHECK(parse("-.143", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side parameters.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), .143);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserBracketedInteger) {
    BOOST_CHECK(parse("(595)", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<double>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 595.0);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserNegativeBracketedInteger) {
    BOOST_CHECK(parse("(-1532)", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side parameters.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1532);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserBracketedReal) {
    BOOST_CHECK(parse("((143.97))", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*result));
    value = dynamic_cast<const ValueRef::Constant<double>*>(result);
    BOOST_CHECK_EQUAL(value->Value(), 143.97);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserNegativeBracketedReal) {
    BOOST_CHECK(parse("(-(6754.20))", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::NEGATE);

    // XXX: Unary operations have no right hand side or left hand side parameters.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 6754.2);
}

BOOST_AUTO_TEST_CASE(DoubleLiteralParserSineOperation) {
    // XXX: sin not documented, radians or degree as input?
    BOOST_CHECK(parse("sin(90.0)", result));

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::SINE);

    // XXX: Unary operations have no right hand side or left hand side parameters.
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 90.0);
}

// XXX value_ref_rule<double> throws an expectation_failure, the enum rule does not. is this intended?
BOOST_AUTO_TEST_CASE(DoubleLiteralParserMalformed) {
    BOOST_CHECK_THROW(parse("-", result), std::runtime_error);
    BOOST_CHECK(!result);

    // XXX: Is a trailing dot a valid real number?
    BOOST_CHECK_THROW(parse("5.", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("(.20", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("(-", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("((", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("((1.001", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("((1)", result), std::runtime_error);
    BOOST_CHECK(!result);

    BOOST_CHECK_THROW(parse("(1.5243)))", result), std::runtime_error);
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
  -1.2+2.7

Expected AST:
  PLUS
    NEGATE
      1.2
    2.7
*/
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser1) {
    BOOST_CHECK(parse("-1.2+2.7", result));

    // (-1.2) + 2.7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // -1.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::NEGATE);

    // 1.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1.2);

    // 2.7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2.7);
}

/*
Term:
  -1.1+2.8-8+5.2

Expected AST:
  PLUS
    MINUS
      PLUS
        NEGATE
          1.1
        2.8
      8
    5.2
*/
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser2) {
    BOOST_CHECK(parse("-1.1+2.8-8+5.2", result));

    // (-1.1+2.8-8) + 5.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // (-1.1+2.8) - 8
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::MINUS);

    // (-1.1) + 2.8
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::PLUS);

    // -1.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation3->LHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<double>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::NEGATE);

    // 1.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation4->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1.1);

    // 2.8
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation3->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2.8);

    // 8
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation2->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 8.0);

    // 5.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 5.2);
}

/*
Term:
  4.8*3.1+2.04

Expected AST:
  PLUS
    TIMES
      4.8
      3.1
    2.04
*/
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser3) {
    BOOST_CHECK(parse("4.8*3.1+2.04", result));

    // (4.8*3.1) + 2.04
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // 4.8 * 3.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::TIMES);

    // 4.8
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 4.8);

    // 3.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation2->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 3.1);

    // 2.04
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2.04);
}

/*
Term:
  -1.5+2.9/-7.4

Expected AST:
  PLUS
    NEGATE
      1.5
    DIVIDE
      2.9
      NEGATE
        7.4
*/
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser4) {
    BOOST_CHECK(parse("-1.5+2.9/-7.4", result));

    // (-1.5) + (2.9/-7.4)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // -1.5
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::NEGATE);

    // 1.5
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1.5);

    // 2.9 / (-7.4)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->RHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::DIVIDE);

    // 2.9
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation3->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 2.9);

    // -7.4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation3->RHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<double>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::NEGATE);

    // 7.4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation4->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 7.4);
}

/*
Term:
  -1.2/3.7+-6.2*7.6

Expected AST:
  PLUS
    DIVIDE
      NEGATE
        1.2
      3.7
    TIMES
      NEGATE
        6.2
      7.6
*/
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser5) {
    BOOST_CHECK(parse("-1.2/3.7+-6.2*7.6", result));

    // (-1.2/3.7) + (-6.2*7.6)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // (-1.2) / 3.7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::DIVIDE);

    // -1.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::NEGATE);

    // 1.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation3->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1.2);

    // 3.7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation2->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 3.7);

    // (-6.2) * 7.6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->RHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::TIMES);

    // -6.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation4->LHS()));
    operation5 = dynamic_cast<const ValueRef::Operation<double>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(operation5->GetOpType(), ValueRef::NEGATE);

    // 6.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation5->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation5->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 6.2);

    // 7.6
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation4->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation4->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 7.6);
}

/*
Term:
  1.1+.4*-4.1/6.2*(9-.1)

Expected AST:
  PLUS
    1.1
    TIMES
      DIVIDE
        TIMES
          0.4
          NEGATE
            4.1
        6.2
      MINUS
        9
        0.1
*/
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser6) {
    BOOST_CHECK(parse("1.1+.4*-4.1/6.2*(9-.1)", result));

    // 1.1 + (.4*-4.1/6.2*(9-.1))
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::PLUS);

    // 1.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1.1);

    // (.4*-4.1/6.2) * ((9-.1))
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->RHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::TIMES);

    // (.4*-4.1) / 6.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::DIVIDE);

    // .4 * (-4.1)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation3->LHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<double>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::TIMES);

    // .4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation4->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(value->Value(), .4);

    // -4.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation4->RHS()));
    operation5 = dynamic_cast<const ValueRef::Operation<double>*>(operation4->RHS());
    BOOST_CHECK_EQUAL(operation5->GetOpType(), ValueRef::NEGATE);

    // 4.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation5->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation5->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 4.1);

    // 6.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation3->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 6.2);

    // 9 - .1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation2->RHS()));
    operation6 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(operation6->GetOpType(), ValueRef::MINUS);

    // 9
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation6->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation6->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 9.0);

    // .1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation6->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation6->RHS());
    BOOST_CHECK_EQUAL(value->Value(), .1);
}

/*
Term:
  -2^5--(3^-4)

Expected AST:
  MINUS
    POWER
      NEGATE
        2
      5
    NEGATE
      POWER
        3
        NEGATE
          4
*/
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser7) {
    BOOST_CHECK(parse("-2^5--(3^-4)", result));

    // (-2^5) - (-(3^-4))
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*result));
    operation1 = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK_EQUAL(operation1->GetOpType(), ValueRef::MINUS);

    // (-2) ^ 5
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->LHS()));
    operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->LHS());
    BOOST_CHECK_EQUAL(operation2->GetOpType(), ValueRef::EXPONENTIATE);

    // -(2)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::NEGATE);

    // 2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation3->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 2);

    // 5
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation2->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 5);

    // -(3^-4)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation1->RHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::NEGATE);

    // 3 ^ (-4)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation4->LHS()));
    operation5 = dynamic_cast<const ValueRef::Operation<double>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(operation5->GetOpType(), ValueRef::EXPONENTIATE);

    // 3
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation5->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation5->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 3);

    // -(4)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation5->RHS()));
    operation6 = dynamic_cast<const ValueRef::Operation<double>*>(operation5->RHS());
    BOOST_CHECK_EQUAL(operation6->GetOpType(), ValueRef::NEGATE);

    // 4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation6->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation6->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 4);
}

BOOST_AUTO_TEST_CASE(DoubleArithmeticParserMalformed) {
    BOOST_CHECK_THROW(parse("1.1 +", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("-1. +", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("-5. 2.1", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("5.2 *", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("* 5.11", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("7.67 / * 5.47", result), std::runtime_error);
    BOOST_CHECK(!result);
    delete result;
    result = 0;

    BOOST_CHECK_THROW(parse("7.84 - 5.2 * .3 / - + .22", result), std::runtime_error);
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(DoubleVariableParserCurrentTurn) {
    BOOST_CHECK(parse("CurrentTurn", result));
    std::string property[] = { "CurrentTurn" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::StaticCast<int, double>), typeid(*result));
    variable = dynamic_cast<const ValueRef::StaticCast<int, double>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::NON_OBJECT_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(DoubleVariableParserValue) {
    BOOST_CHECK(parse("Value", result));
    std::string property[] = { "Value" };

    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Variable<double>), typeid(*result));
    variable = dynamic_cast<const ValueRef::Variable<double>*>(result);
    BOOST_CHECK_EQUAL(variable->GetReferenceType(), ValueRef::EFFECT_TARGET_REFERENCE);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        variable->PropertyName().begin(), variable->PropertyName().end(),
        property, property + 1
    );
}

BOOST_AUTO_TEST_CASE(DoubleVariableParserTypeless) {
    for (const ReferenceType& reference : referenceTypes) {
        for (const std::string& attribute : attributes) {
            std::string phrase = reference.second + "." + attribute;
            BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse: \"" + phrase + "\"");
            std::string property[] = {
                attribute
            };

            BOOST_CHECK_EQUAL(typeid(ValueRef::Variable<double>), typeid(*result));
            if(variable = dynamic_cast<const ValueRef::Variable<double>*>(result)) {
                BOOST_CHECK_EQUAL(variable->GetReferenceType(), reference.first);
                BOOST_CHECK_EQUAL_COLLECTIONS(
                    variable->PropertyName().begin(), variable->PropertyName().end(),
                    property, property + 1
                );
            }

            delete result;
            result = 0;
        }
    }
}

BOOST_AUTO_TEST_CASE(DoubleVariableParserTyped) {
    for (const ReferenceType& reference : referenceTypes) {
        for (const std::string& type : containerTypes) {
            for (const std::string& attribute : attributes) {
                std::string phrase = reference.second + "." + type + "." + attribute;
                BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse: \"" + phrase + "\"");
                std::string property[] = {
                    type,
                    attribute
                };

                BOOST_CHECK_EQUAL(typeid(ValueRef::Variable<double>), typeid(*result));
                if(variable = dynamic_cast<const ValueRef::Variable<double>*>(result)) {
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
}

// XXX: Statistic COUNT, UNIQUE_COUNT and IF not tested.

BOOST_AUTO_TEST_CASE(DoubleStatisticParserTypeless) {
    for (const StatisticType& statisticType : statisticTypes) {
        for (const std::string& attribute : attributes) {
            std::string property[] = { attribute };

            std::string phrase = "Statistic " + statisticType.second + " Value = " + attribute + " Condition = All";

            BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse \"" + phrase + "\"");

            BOOST_CHECK_EQUAL(typeid(ValueRef::Statistic<double>), typeid(*result));
            if(statistic = dynamic_cast<const ValueRef::Statistic<double>*>(result)) {
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

BOOST_AUTO_TEST_CASE(DoubleStatisticParserTyped) {
    for (const StatisticType& statisticType : statisticTypes) {
        for (const std::string& containerType : containerTypes) {
            for (const std::string& attribute : attributes) {
                std::string property[] = {
                    containerType,
                    attribute
                };

                std::string phrase = "Statistic " + statisticType.second + " Value = " + containerType + "." + attribute + " Condition = All";

                BOOST_CHECK_MESSAGE(parse(phrase, result), "Failed to parse \"" + phrase + "\"");

                BOOST_CHECK_EQUAL(typeid(ValueRef::Statistic<double>), typeid(*result));
                if(statistic = dynamic_cast<const ValueRef::Statistic<double>*>(result)) {
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
