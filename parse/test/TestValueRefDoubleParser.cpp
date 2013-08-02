#include <boost/test/unit_test.hpp>

#include "ValueRefParser.h"
#include "universe/ValueRef.h"

#define CHECK_IS_TYPE_PTR(TYPE, VALUE) \
    BOOST_CHECK_MESSAGE(VALUE && typeid(TYPE) == typeid(*VALUE), \
        "check typeid(" #TYPE ") == typeid(" #VALUE ") failed: " #VALUE " was " << (VALUE ? typeid(*VALUE).name() : "(null)"))

#define REQUIRE_IS_TYPE_PTR(TYPE, VALUE) \
    BOOST_REQUIRE_MESSAGE(VALUE && typeid(TYPE) == typeid(*VALUE), \
        "check typeid(" #TYPE ") == typeid(" #VALUE ") failed: " #VALUE " was " << (VALUE ? typeid(*VALUE).name() : "(null)"))

struct ValueRefDoubleFixture {
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
};

// XXX value_ref_parser_rule<double> throws an expectation_failure, the enum parser does not. is this intended?

BOOST_FIXTURE_TEST_SUITE(ValueRefDoubleParser, ValueRefDoubleFixture)

BOOST_AUTO_TEST_CASE(DoubleLiteralParser) {
    ValueRef::ValueRefBase<double>* result;

    BOOST_CHECK(parse("7309", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == 7309.0);
    delete result;
    result = 0;

    BOOST_CHECK(parse("-1343", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == -1343.0);
    delete result;
    result = 0;

    BOOST_CHECK(parse("14.234", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == 14.234);
    delete result;
    result = 0;

    BOOST_CHECK(parse("-13.7143", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == -13.7143);
    delete result;
    result = 0;

    BOOST_CHECK(parse(".234", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == .234);
    delete result;
    result = 0;

    BOOST_CHECK(parse("-.143", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == -.143);
    delete result;
    result = 0;

    BOOST_CHECK(parse("(595)", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == 595.0);
    delete result;
    result = 0;

    BOOST_CHECK(parse("(-1532)", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == -1532.0);
    delete result;
    result = 0;

    BOOST_CHECK(parse("((143.97))", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == 143.97);
    delete result;
    result = 0;

    BOOST_CHECK(parse("((-6754.20))", result));
    CHECK_IS_TYPE_PTR(ValueRef::Constant<double>, result);
    if(dynamic_cast<ValueRef::Constant<double>*>(result))
        BOOST_CHECK(dynamic_cast<ValueRef::Constant<double>*>(result)->Value() == -6754.20);
    delete result;
    result = 0;

    // errornous input
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
//   +      #
//   |\     #
//   | \    #
//   |  \   #
//   |   \  #
// -1.2 2.7 #
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser1) {
    ValueRef::ValueRefBase<double>* result;

    BOOST_CHECK(parse("-1.2+2.7", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, result);
    ValueRef::Operation<double>* operation = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation->LHS())->Value() == -1.2);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation->RHS())->Value() == 2.7);

    delete result;
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
//  |  \    #
//  |   \   #
// -1.1 2.8 #
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser2) {
    ValueRef::ValueRefBase<double>* result;

    BOOST_CHECK(parse("-1.1+2.8-8+5.2", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, result);
    const ValueRef::Operation<double>* operation  = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // (-1.1+2.8-8) + 5.2
    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation->RHS())->Value() == 5.2);

    // (-1.1+2.8) - 8
    const ValueRef::Operation<double>* operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation->LHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::MINUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation2->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS())->Value() == 8.0);

    // -1.1 + 2.8
    const ValueRef::Operation<double>* operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->LHS());
    BOOST_CHECK(operation3->GetOpType() == ValueRef::PLUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation3->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation3->LHS())->Value() == -1.1);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation3->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation3->RHS())->Value() == 2.8);

    delete result;
}

// Term:
// 4.8*3.1+2.04
// Expected AST:
// +        #
// |\       #
// * 2.04   #
// |\       #
// | \      #
// |  \     #
// |   \    #
// 4.8 3.1  #
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser3) {
    ValueRef::ValueRefBase<double>* result;

    BOOST_CHECK(parse("4.8*3.1+2.04", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, result);
    const ValueRef::Operation<double>* operation = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // (4.8*3.1) + 2.04
    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation->RHS())->Value() == 2.04);

    // 4.8 * 3.1
    const ValueRef::Operation<double>* operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation->LHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation2->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation2->LHS())->Value() == 4.8);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS())->Value() == 3.1);

    delete result;
}

// Term:
// -1.5+2.9/-7.4
// Expected AST:
//  +              #
//  |\             #
//  | \            #
//  |  \           #
//  |   \          #
// -1.5 (/)        #
//       | \       #
//       |  \      #
//       |   \     #
//       |    \    #
//       2.9 -7.4  #
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser4) {
    ValueRef::ValueRefBase<double>* result;

    BOOST_CHECK(parse("-1.5+2.9/-7.4", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, result);
    const ValueRef::Operation<double>* operation = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // -1.5 + (2.9/-7.4)
    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation->LHS())->Value() == -1.5);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation->RHS());

    // 2.9 / -7.4
    const ValueRef::Operation<double>* operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation->RHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::DIVIDE);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation2->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation2->LHS())->Value() == 2.9);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS())->Value() == -7.4);

    delete result;
}

// Term:
// -1.2/3.7+-6.2*7.6
// Expected AST:
//         +          #
//        /  \        #
//       /    \       #
//      /      \      #
//    (/)       *     #
//    /\        /\    #
//   /  \      /  \   #
// -1.2 3.7 -6.2 7.6  #
BOOST_AUTO_TEST_CASE(DoubleArithmeticParser5) {
    ValueRef::ValueRefBase<double>* result;

    BOOST_CHECK(parse("-1.2/3.7+-6.2*7.6", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, result);
    const ValueRef::Operation<double>* operation = dynamic_cast<ValueRef::Operation<double>*>(result);
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    // (-1.2/3.7) + (-6.2*7.6)
    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation->RHS());

    // -1.2 / 3.7
    const ValueRef::Operation<double>* operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation->LHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::DIVIDE);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation2->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation2->LHS())->Value() == -1.2);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation2->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation2->RHS())->Value() == 3.7);

    // -6.2 * 7.6
    const ValueRef::Operation<double>* operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation->RHS());
    BOOST_CHECK(operation3->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation3->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation3->LHS())->Value() == -6.2);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation3->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation3->RHS())->Value() == 7.6);

    delete result;
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
    ValueRef::ValueRefBase<double>* result;

    BOOST_CHECK(parse("1.1+.4*-4.1/6.2*(9-.1)", result));

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, result);
    const ValueRef::Operation<double>* operation = dynamic_cast<ValueRef::Operation<double>*>(result);

    // 1.1 + (.4*-4.1/6.2*(9-.1))
    BOOST_CHECK(operation->GetOpType() == ValueRef::PLUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation->LHS())->Value() == -1.1);

    // (.4*-4.1/6.2) * ((9-.1))
    const ValueRef::Operation<double>* operation2 = dynamic_cast<const ValueRef::Operation<double>*>(operation->RHS());
    BOOST_CHECK(operation2->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation2->LHS());

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation2->RHS());

    // .4 * (-4.1/6.2)
    const ValueRef::Operation<double>* operation3 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->LHS());
    BOOST_CHECK(operation3->GetOpType() == ValueRef::TIMES);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation3->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation3->LHS())->Value() == .4);

    REQUIRE_IS_TYPE_PTR(ValueRef::Operation<double>, operation3->RHS());

    // -4.1 / 6.2
    const ValueRef::Operation<double>* operation4 = dynamic_cast<const ValueRef::Operation<double>*>(operation3->RHS());
    BOOST_CHECK(operation4->GetOpType() == ValueRef::DIVIDE);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation4->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation4->LHS())->Value() == -4.1);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation4->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation4->RHS())->Value() == 6.2);

    // 9 - .1
    const ValueRef::Operation<double>* operation5 = dynamic_cast<const ValueRef::Operation<double>*>(operation2->RHS());
    BOOST_CHECK(operation5->GetOpType() == ValueRef::MINUS);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation5->LHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation5->LHS())->Value() == 9.0);

    REQUIRE_IS_TYPE_PTR(ValueRef::Constant<double>, operation5->RHS());
    BOOST_CHECK(dynamic_cast<const ValueRef::Constant<double>*>(operation5->RHS())->Value() == .1);

    delete result;
}

BOOST_AUTO_TEST_CASE(DoubleArithmeticParserMalformed) {
    ValueRef::ValueRefBase<double>* result = 0;

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
