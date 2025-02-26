#ifndef _ValueRefPythonParser_h_
#define _ValueRefPythonParser_h_

#include <memory>

#include <boost/python/extract.hpp>

#include "../universe/ValueRefs.h"
#include "../universe/Conditions.h"

#include "ConditionPythonParser.h"

class PythonParser;

template<typename T>
struct value_ref_wrapper {
    value_ref_wrapper(std::shared_ptr<ValueRef::ValueRef<T>>&& ref) :
        value_ref(std::move(ref))
    {}

    value_ref_wrapper(const std::shared_ptr<ValueRef::ValueRef<T>>& ref) :
        value_ref(ref)
    {}

    value_ref_wrapper<T> call(const value_ref_wrapper<T>& var_) const {
        if (auto var = std::dynamic_pointer_cast<const ValueRef::Variable<T>>(var_.value_ref)) {
            return value_ref_wrapper<T>(std::make_shared<ValueRef::Variable<T>>(
                var->GetReferenceType(), var->PropertyName(), var->GetContainerType(),
                ValueRef::ValueToReturn::Immediate));
        } else if(var_.value_ref) {
            throw std::runtime_error(std::string("Unknown type of Value.__call__ ") + typeid(*var_.value_ref).name());
        } else {
            throw std::runtime_error("Empty value in Value.__call__");
        }
    }

    operator condition_wrapper() const {
        auto op = std::dynamic_pointer_cast<const ValueRef::Operation<T>>(value_ref);

        if (op && op->LHS() && op->RHS()) {
            Condition::ComparisonType cmp_type;
            switch (op->GetOpType()) {
                case ValueRef::OpType::COMPARE_EQUAL:
                    cmp_type = Condition::ComparisonType::EQUAL;
                    break;
                case ValueRef::OpType::COMPARE_GREATER_THAN:
                    cmp_type = Condition::ComparisonType::GREATER_THAN;
                    break;
                case ValueRef::OpType::COMPARE_GREATER_THAN_OR_EQUAL:
                    cmp_type = Condition::ComparisonType::GREATER_THAN_OR_EQUAL;
                    break;
                case ValueRef::OpType::COMPARE_LESS_THAN:
                    cmp_type = Condition::ComparisonType::LESS_THAN;
                    break;
                case ValueRef::OpType::COMPARE_LESS_THAN_OR_EQUAL:
                    cmp_type = Condition::ComparisonType::LESS_THAN_OR_EQUAL;
                    break;
                case ValueRef::OpType::COMPARE_NOT_EQUAL:
                    cmp_type = Condition::ComparisonType::NOT_EQUAL;
                    break;
                default:
                    throw std::runtime_error(std::string("Not implemented in ") + __func__ + " op type " + std::to_string(static_cast<int>(op->GetOpType())) + value_ref->Dump());
            }
            return condition_wrapper(std::make_shared<Condition::ValueTest>(
                op->LHS()->Clone(),
                cmp_type,
                op->RHS()->Clone()));
        } else {
            throw std::runtime_error(std::string("Unknown type of Value to condition ") + typeid(*value_ref).name());
        }
    }

    const std::shared_ptr<const ValueRef::ValueRef<T>> value_ref;
};

template<typename T>
std::unique_ptr<ValueRef::ValueRef<T>> pyobject_to_vref(const boost::python::object& obj) {
    auto arg = boost::python::extract<value_ref_wrapper<T>>(obj);
    if (arg.check()) {
        return ValueRef::CloneUnique(arg().value_ref);
    }
    return std::make_unique<ValueRef::Constant<T>>(boost::python::extract<T>(obj)());
}

value_ref_wrapper<double> pow(const value_ref_wrapper<int>& lhs, double rhs);
value_ref_wrapper<double> pow(const value_ref_wrapper<double>& lhs, double rhs);
value_ref_wrapper<double> pow(double lhs, const value_ref_wrapper<double>& rhs);
value_ref_wrapper<double> pow(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs);

value_ref_wrapper<double> operator*(int, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator*(const value_ref_wrapper<int>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator*(const value_ref_wrapper<double>&, const value_ref_wrapper<int>&);
value_ref_wrapper<double> operator*(const value_ref_wrapper<double>&, double);
value_ref_wrapper<double> operator*(double, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator*(double, const value_ref_wrapper<int>&);
value_ref_wrapper<double> operator*(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator/(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator/(const value_ref_wrapper<double>&, int);
value_ref_wrapper<double> operator/(const value_ref_wrapper<double>&, double);
value_ref_wrapper<double> operator+(int, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, int);
value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, double);
value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, const value_ref_wrapper<int>&);
value_ref_wrapper<double> operator+(const value_ref_wrapper<int>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator+(double, const value_ref_wrapper<int>&);
value_ref_wrapper<double> operator-(const value_ref_wrapper<double>&, double);
value_ref_wrapper<double> operator-(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator-(int, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator-(double, const value_ref_wrapper<int>&);
value_ref_wrapper<double> operator-(const value_ref_wrapper<double>&, int);
value_ref_wrapper<double> operator-(const value_ref_wrapper<double>&, const value_ref_wrapper<int>&);
value_ref_wrapper<double> operator>=(const value_ref_wrapper<double>&, int);
value_ref_wrapper<double> operator<=(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator<=(double, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator<=(const value_ref_wrapper<double>&, double);
value_ref_wrapper<double> operator>(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator>=(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator<(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator<(double, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator<(const value_ref_wrapper<double>&, double);
value_ref_wrapper<double> operator!=(const value_ref_wrapper<double>&, int);
value_ref_wrapper<double> operator-(const value_ref_wrapper<double>&);

value_ref_wrapper<int> operator*(int, const value_ref_wrapper<int>&);
value_ref_wrapper<int> operator/(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator-(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator-(int, const value_ref_wrapper<int>&);
value_ref_wrapper<int> operator+(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator+(const value_ref_wrapper<int>&, const value_ref_wrapper<int>&);
value_ref_wrapper<int> operator<(const value_ref_wrapper<int>&, const value_ref_wrapper<int>&);
value_ref_wrapper<int> operator<(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator<=(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator>(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator>=(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator>=(const value_ref_wrapper<int>&, const value_ref_wrapper<int>&);
value_ref_wrapper<int> operator==(const value_ref_wrapper<int>&, const value_ref_wrapper<int>&);
value_ref_wrapper<int> operator==(const value_ref_wrapper<int>&, int);
value_ref_wrapper<int> operator!=(const value_ref_wrapper<int>&, int);

value_ref_wrapper<std::string> operator+(const value_ref_wrapper<std::string>&, const std::string&);
value_ref_wrapper<std::string> operator+(const std::string&, const value_ref_wrapper<std::string>&);

void RegisterGlobalsValueRefs(boost::python::dict& globals, const PythonParser& parser);

#endif // _ValueRefPythonParser_h_

