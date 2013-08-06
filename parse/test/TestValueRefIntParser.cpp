#include <boost/test/unit_test.hpp>

#include "ValueRefParser.h"
#include "universe/ValueRef.h"
#include "CommonTest.h"

struct ValueRefIntFixture {
    ValueRefIntFixture():
        result(0)
    {}

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

    ValueRef::ValueRefBase<int>* result;
    const ValueRef::Operation<int>* operation1;
    const ValueRef::Operation<int>* operation2;
    const ValueRef::Operation<int>* operation3;
    const ValueRef::Operation<int>* operation4;
    const ValueRef::Operation<int>* operation5;
    const ValueRef::Operation<int>* operation6;
    const ValueRef::Constant<int>* value;
};

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

BOOST_AUTO_TEST_SUITE_END()
