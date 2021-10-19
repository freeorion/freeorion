#include "ValueRefPythonParser.h"

#include <stdexcept>

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/ValueRefs.h"
#include "../universe/NamedValueRefManager.h"
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


namespace {
    template<typename T>
    value_ref_wrapper<T> insert_named_lookup_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();

        return value_ref_wrapper<T>(std::make_shared<ValueRef::NamedRef<T>>(name, true));
    }
}

void RegisterGlobalsValueRefs(boost::python::dict& globals) {
    globals["NamedRealLookup"] = boost::python::raw_function(insert_named_lookup_<double>);
    globals["Value"] = value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE));
    globals["CurrentTurn"] = value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, "CurrentTurn"));
}

