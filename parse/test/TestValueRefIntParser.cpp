#include <boost/test/unit_test.hpp>

#include "ValueRefParser.h"
#include "universe/ValueRef.h"

#define CHECK_IS_TYPE_PTR(TYPE, VALUE) \
    BOOST_CHECK_MESSAGE(VALUE && typeid(TYPE) == typeid(*VALUE), \
        "check typeid(" #TYPE ") == typeid(" #VALUE ") failed: " #VALUE " was " << (VALUE ? typeid(*VALUE).name() : "(null)"))

#define REQUIRE_IS_TYPE_PTR(TYPE, VALUE) \
    BOOST_REQUIRE_MESSAGE(VALUE && typeid(TYPE) == typeid(*VALUE), \
        "check typeid(" #TYPE ") == typeid(" #VALUE ") failed: " #VALUE " was " << (VALUE ? typeid(*VALUE).name() : "(null)"))

struct ValueRefIntFixture {
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
};

// XXX value_ref_parser_rule<int> throws an expectation_failure, the enum parser does not. is this intended?

BOOST_FIXTURE_TEST_SUITE(ValueRefIntParser, ValueRefIntFixture)

BOOST_AUTO_TEST_CASE(IntLiteralParser) {
    ValueRef::ValueRefBase<int>* result;

    // XXX: What is the desired real to int casting behaviour (to nearest int, floor, ...)

    BOOST_CHECK(parse("7309", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == 7309);
    delete result;
    result = 0;

    BOOST_CHECK(parse("-1343", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == -1343);
    delete result;
    result = 0;

    BOOST_CHECK(parse("14.234", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == 14);
    delete result;
    result = 0;

    BOOST_CHECK(parse("-13.7143", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == -13);
    delete result;
    result = 0;

    BOOST_CHECK(parse("(595)", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == 595);
    delete result;
    result = 0;

    BOOST_CHECK(parse("(-1532)", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == -1532);
    delete result;
    result = 0;

    BOOST_CHECK(parse("((143))", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == 143);
    delete result;
    result = 0;

    BOOST_CHECK(parse("((-6754.20))", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<int>, result);
    if(dynamic_cast<ValueRef::Constant<int>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<int>*>(result)->Value() == -6754);
    delete result;
    result = 0;

    // errornous input
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
//  +   #
//  |\  #
// -1 2 #
BOOST_AUTO_TEST_CASE(IntArithmeticParser1) {
    ValueRef::ValueRefBase<int>* result;

    BOOST_CHECK(parse("-1+2", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, result);
    ValueRef::Operation<int>* operation = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation->LHS())->Value() == -1);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation->RHS())->Value() == 2);

    delete result;
}

// Term:
// -1+2-8+5
// Expected AST:
//  +   #
//  |\  #
//  - 5 #
//  |\  #
//  + 8 #
//  |\  #
// -1 2 #
BOOST_AUTO_TEST_CASE(IntArithmeticParser2) {
    ValueRef::ValueRefBase<int>* result;

    BOOST_CHECK(parse("-1+2-8+5", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, result);
    const ValueRef::Operation<int>* operation  = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // (-1+2-8) + 5
    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation->RHS())->Value() == 5);

    // (-1+2) - 8
    const ValueRef::Operation<int>* operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation->LHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::MINUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation2->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation2->RHS())->Value() == 8);

    // -1 + 2
    const ValueRef::Operation<int>* operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->LHS());
    BOOST_CHECK(operation3->GetOpType() == ValueRef::PLUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation3->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation3->LHS())->Value() == -1);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation3->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation3->RHS())->Value() == 2);

    delete result;
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
    ValueRef::ValueRefBase<int>* result;

    BOOST_CHECK(parse("4*3+2", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, result);
    const ValueRef::Operation<int>* operation = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // (4*3) + 2
    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation->RHS())->Value() == 2);

    // 4 * 3
    const ValueRef::Operation<int>* operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation->LHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation2->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation2->LHS())->Value() == 4);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation2->RHS())->Value() == 3);

    delete result;
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
    ValueRef::ValueRefBase<int>* result;

    BOOST_CHECK(parse("-1+2/-7", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, result);
    const ValueRef::Operation<int>* operation = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // -1 + (2/-7)
    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation->LHS())->Value() == -1);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation->RHS());

    // 2 / -7
    const ValueRef::Operation<int>* operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation->RHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::DIVIDE);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation2->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation2->LHS())->Value() == 2);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation2->RHS())->Value() == -7);

    delete result;
}

// Term:
// -1/3+-6*7
// Expected AST:
//    +        #
//   / \       #
// (/)  *      #
// /\   | \    #
//-1 3 -6  7   #
BOOST_AUTO_TEST_CASE(IntArithmeticParser5) {
    ValueRef::ValueRefBase<int>* result;

    BOOST_CHECK(parse("-1/3+-6*7", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, result);
    const ValueRef::Operation<int>* operation = dynamic_cast<ValueRef::Operation<int>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // (-1/3) + (-6*7)
    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation->RHS());

    // -1 / 3
    const ValueRef::Operation<int>* operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation->LHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::DIVIDE);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation2->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation2->LHS())->Value() == -1);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation2->RHS())->Value() == 3);

    // -6 * 7
    const ValueRef::Operation<int>* operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation->RHS());
    BOOST_CHECK(operation3->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation3->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation3->LHS())->Value() == -6);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation3->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation3->RHS())->Value() == 7);

    delete result;
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
// 3 -4       #
BOOST_AUTO_TEST_CASE(IntArithmeticParser6) {
    ValueRef::ValueRefBase<int>* result;

    BOOST_CHECK(parse("1+3*-4*6*(9-1)", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, result);
    const ValueRef::Operation<int>* operation = dynamic_cast<ValueRef::Operation<int>*>(result);

    // 1 + (3*-4/6*(9-1))
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation->LHS())->Value() == -1);

    // (3*-4/6) * ((9-1))
    const ValueRef::Operation<int>* operation2 = dynamic_cast<const ValueRef::Operation<int>*>(operation->RHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation2->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation2->RHS());

    // 3 * (-4/6)
    const ValueRef::Operation<int>* operation3 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->LHS());
    BOOST_CHECK(operation3->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation3->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation3->LHS())->Value() == 3);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<int>, operation3->RHS());

    // -4 / 6
    const ValueRef::Operation<int>* operation4 = dynamic_cast<const ValueRef::Operation<int>*>(operation3->RHS());
    BOOST_CHECK(operation4->GetOpType() == ValueRef::DIVIDE);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation4->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation4->LHS())->Value() == -4);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation4->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation4->RHS())->Value() == 6);

    // 9 - 1
    const ValueRef::Operation<int>* operation5 = dynamic_cast<const ValueRef::Operation<int>*>(operation2->RHS());
    BOOST_CHECK(operation5->GetOpType() == ValueRef::MINUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation5->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation5->LHS())->Value() == 9);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<int>, operation5->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<int>*>(operation5->RHS())->Value() == 1);

    delete result;
}

BOOST_AUTO_TEST_CASE(IntArithmeticParserMalformed) {
    ValueRef::ValueRefBase<int>* result = 0;

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
