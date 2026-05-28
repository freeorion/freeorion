#include "ValueRefPythonParser.h"

value_ref_wrapper<double> pow(const value_ref_wrapper<int>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(
            ValueRef::OpType::EXPONENTIATE,
            std::make_unique<ValueRef::StaticCast<int, double>>(ValueRef::CloneUnique(lhs.value_ref)),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> pow(const value_ref_wrapper<double>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(
            ValueRef::OpType::EXPONENTIATE,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> pow(double lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(
            ValueRef::OpType::EXPONENTIATE,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> pow(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(
            ValueRef::OpType::EXPONENTIATE,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator-(const value_ref_wrapper<double>& lhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(
            ValueRef::OpType::NEGATE,
            ValueRef::CloneUnique(lhs.value_ref))
    );
}

value_ref_wrapper<std::string> operator+(const value_ref_wrapper<std::string>& lhs, const std::string& rhs) {
    return value_ref_wrapper<std::string>(
        std::make_shared<ValueRef::Operation<std::string>>(
            ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<std::string>>(rhs))
    );
}

value_ref_wrapper<std::string> operator+(const std::string& lhs, const value_ref_wrapper<std::string>& rhs) {
    return value_ref_wrapper<std::string>(
        std::make_shared<ValueRef::Operation<std::string>>(
            ValueRef::OpType::PLUS,
            std::make_unique<ValueRef::Constant<std::string>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator!=(const value_ref_wrapper<PlanetType>& lhs, const value_ref_wrapper<PlanetType>& rhs) {
    return condition_wrapper(std::make_shared<Condition::ValueTest>(
        std::make_unique<ValueRef::StaticCast<PlanetType, int>>(ValueRef::CloneUnique(lhs.value_ref)),
        Condition::ComparisonType::NOT_EQUAL,
        std::make_unique<ValueRef::StaticCast<PlanetType, int>>(ValueRef::CloneUnique(rhs.value_ref))));
}

condition_wrapper operator!=(const value_ref_wrapper<PlanetSize>& lhs, const value_ref_wrapper<PlanetSize>& rhs) {
    return condition_wrapper(std::make_shared<Condition::ValueTest>(
        std::make_unique<ValueRef::StaticCast<PlanetSize, int>>(ValueRef::CloneUnique(lhs.value_ref)),
        Condition::ComparisonType::NOT_EQUAL,
        std::make_unique<ValueRef::StaticCast<PlanetSize, int>>(ValueRef::CloneUnique(rhs.value_ref))));
}

void RegisterGlobalsValueRefs(boost::python::dict& globals) {

}
