#include <boost/test/unit_test.hpp>

#include "ValueRefParser.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefDoubleFixture {
    ValueRefDoubleFixture():
        result(0)
    {}

    ~ValueRefDoubleFixture() {
        delete result;
    }

    bool parse(std::string phrase, ValueRef::ValueRefBase<double>*& result) {
        parse::value_ref_parser_rule<double>::type& rule = parse::value_ref_parser<double>();
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

    ValueRef::ValueRefBase<double>* result;
    const ValueRef::Operation<double>* operation1;
    const ValueRef::Operation<double>* operation2;
    const ValueRef::Operation<double>* operation3;
    const ValueRef::Operation<double>* operation4;
    const ValueRef::Operation<double>* operation5;
    const ValueRef::Operation<double>* operation6;
    const ValueRef::Constant<double>* value;
};

BOOST_FIXTURE_TEST_SUITE(ValueRefDoubleParser, ValueRefDoubleFixture)

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

// XXX value_ref_parser_rule<double> throws an expectation_failure, the enum parser does not. is this intended?
BOOST_AUTO_TEST_CASE(DoubleLiteralParserErrornousInput) {
    BOOST_CHECK(!parse("-", result));
    BOOST_CHECK(!parse("(.20", result));
    BOOST_CHECK(!parse("(-", result));
    BOOST_CHECK(!parse("((", result));
    BOOST_CHECK(!parse("((1.001", result));
    BOOST_CHECK(!parse("((1)", result));
    BOOST_CHECK(!parse("(1.5243)))", result));
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
// -1.2+2.7
// Expected AST:
//  +     #
//  |\    #
//  | \   #
//  |  \  #
//  - 2.7 #
//  |     #
// 1.2    #
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
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>*), typeid(*operation2->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 1.2);

    // 2.7
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation1->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation1->RHS());
    BOOST_CHECK_EQUAL(value->Value(), 2.7);
}

// Term:
// -1.1+2.8-8+5.2
// Expected AST:
//  +       #
//  |\      #
//  - 5.2   #
//  |\      #
//  + 8     #
//  |\      #
//  | \     #
//  - 2.8 #
//  |
// 1.1
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
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation4->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation4->RHS());
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

// Term:
// 4.8*3.1+2.04
// Expected AST:
//  +       #
//  |\      #
//  | \     #
//  * 2.04  #
//  |\      #
//  | \     #
//  |  \    #
// 4.8 3.1  #
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

// Term:
// -1.5+2.9/-7.4
// Expected AST:
//  +          #
//  |\         #
//  - \        #
//  |  \       #
// 1.5 (/)     #
//      | \    #
//     2.9 -   #
//         |   #
//       -7.4  #
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
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation3->RHS());
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

// Term:
// -1.2/3.7+-6.2*7.6
// Expected AST:
//       +        #
//      / \       #
//     /   \      #
//   (/)    *     #
//   /\     /\    #
//  /  \   /  \   #
//  - 3.7  - 7.6  #
//  |      |      #
// 1.2    6.2     #
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

// Term:
// 1.1+.4*-4.1/6.2*(9-.1)
// Expected AST:
//     +          #
//     /\         #
//    /  \        #
//   1.1  *       # 
//       / \      #
//      /   |     #
//    (/)   -     #
//    /\    |\    #
//   /  \   | \   #
//   *  6.2 9 .1  #
//   /\           #
//  /  \          #
// .4 -4.1        #
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

    // .4 * (-4.1/6.2)
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation2->LHS()));
    operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->LHS());
    BOOST_CHECK_EQUAL(operation3->GetOpType(), ValueRef::TIMES);

    // .4
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation3->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation3->LHS());
    BOOST_CHECK_EQUAL(value->Value(), .4);

    // (-4.1) / 6.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation3->RHS()));
    operation4 = dynamic_cast<const ValueRef::Operation<double>*>(operation3->RHS());
    BOOST_CHECK_EQUAL(operation4->GetOpType(), ValueRef::DIVIDE);

    // -4.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Operation<double>), typeid(*operation4->LHS()));
    operation5 = dynamic_cast<const ValueRef::Operation<double>*>(operation4->LHS());
    BOOST_CHECK_EQUAL(operation5->GetOpType(), ValueRef::NEGATE);

    // 4.1
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation5->LHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation5->LHS());
    BOOST_CHECK_EQUAL(value->Value(), 4.1);

    // 6.2
    BOOST_REQUIRE_EQUAL(typeid(ValueRef::Constant<double>), typeid(*operation4->RHS()));
    value = dynamic_cast<const ValueRef::Constant<double>*>(operation4->RHS());
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

BOOST_AUTO_TEST_CASE(DoubleArithmeticParserMalformed) {
    // XXX: Is a trailing dot a valid real number?
    BOOST_CHECK(!parse("5.", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("1.1 +", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("-1. +", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("-5. 2.1", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("5. + - - 2.2", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("5.2 *", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("* 5.11", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("7.67 / * 5.47", result));
    BOOST_CHECK(!result);

    BOOST_CHECK(!parse("7.84 - 5.2 * .3 / - + .22", result));
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_SUITE_END()
