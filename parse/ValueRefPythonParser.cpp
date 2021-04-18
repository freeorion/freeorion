#include "ValueRefPythonParser.h"

#include <stdexcept>

#include "../universe/ValueRefs.h"
#include "../universe/Conditions.h"

value_ref_wrapper<double> operator*(int lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, int rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator-(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::MINUS,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

condition_wrapper operator<(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::LESS_THAN,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator==(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::EQUAL,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

