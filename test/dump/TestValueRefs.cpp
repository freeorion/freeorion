#include <boost/test/unit_test.hpp>

#include "universe/ValueRefs.h"

BOOST_AUTO_TEST_SUITE(TestValueRefs);

BOOST_AUTO_TEST_CASE(Variable) {
    BOOST_CHECK_EQUAL("GalaxyAge", ValueRef::Variable<int>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, "GalaxyAge").Dump());
}

BOOST_AUTO_TEST_CASE(ComplexVariable) {
    BOOST_CHECK_EQUAL("GameRule name = \"TEST_RULE\"", ValueRef::ComplexVariable<int>(
        "GameRule",
        nullptr,
        nullptr,
        nullptr,
        std::make_unique<ValueRef::Constant<std::string>>("TEST_RULE"),
        nullptr).Dump());
}

BOOST_AUTO_TEST_CASE(OperationCompare) {
    BOOST_CHECK_EQUAL("(4 >= 5) ? 6 : 7", ValueRef::Operation<int>(
        ValueRef::OpType::COMPARE_GREATER_THAN_OR_EQUAL,
        std::make_unique<ValueRef::Constant<int>>(4),
        std::make_unique<ValueRef::Constant<int>>(5),
        std::make_unique<ValueRef::Constant<int>>(6),
        std::make_unique<ValueRef::Constant<int>>(7)).Dump());
}

BOOST_AUTO_TEST_SUITE_END();

