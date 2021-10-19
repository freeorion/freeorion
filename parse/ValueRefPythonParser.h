#ifndef _ValueRefPythonParser_h_
#define _ValueRefPythonParser_h_

#include <memory>

#include <boost/python/dict.hpp>

#include "../universe/ValueRefs.h"

#include "ConditionPythonParser.h"

template<typename T>
struct value_ref_wrapper {
    value_ref_wrapper(std::shared_ptr<ValueRef::ValueRef<T>>&& ref)
        : value_ref(std::move(ref))
    { }

    value_ref_wrapper(const std::shared_ptr<ValueRef::ValueRef<T>>& ref)
        : value_ref(ref)
    { }

    value_ref_wrapper<T> call(const value_ref_wrapper<T>& var_) const {
        std::shared_ptr<ValueRef::Variable<T>> var = std::dynamic_pointer_cast<ValueRef::Variable<T>>(var_.value_ref);
        if (var) {
            return value_ref_wrapper<T>(std::make_shared<ValueRef::Variable<T>>(
                                            var->GetReferenceType(),
                                            var->PropertyName(),
                                            true
                                        ));
        } else if(var_.value_ref) {
            throw std::runtime_error(std::string("Unknown type of Value.__call__ ") + typeid(*var_.value_ref).name());
        } else {
            throw std::runtime_error("Empty value in Value.__call__");
        }
    }

    std::shared_ptr<ValueRef::ValueRef<T>> value_ref;
};

value_ref_wrapper<double> operator*(int, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, int);
value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, double);
value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);
value_ref_wrapper<double> operator-(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);

condition_wrapper operator<(const value_ref_wrapper<int>&, const value_ref_wrapper<int>&);
condition_wrapper operator==(const value_ref_wrapper<int>&, const value_ref_wrapper<int>&);

void RegisterGlobalsValueRefs(boost::python::dict& globals);

#endif // _ValueRefPythonParser_h_

