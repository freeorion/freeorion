#ifndef _ValueRefPythonParser_h_
#define _ValueRefPythonParser_h_

#include <memory>

#include "../universe/ValueRef.h"

template<typename T>
struct value_ref_wrapper {
    value_ref_wrapper(std::shared_ptr<ValueRef::ValueRef<T>>&& ref)
        : value_ref(std::move(ref))
    { }

    value_ref_wrapper(const std::shared_ptr<ValueRef::ValueRef<T>>& ref)
        : value_ref(ref)
    { }


    std::shared_ptr<ValueRef::ValueRef<T>> value_ref;
};

value_ref_wrapper<double> operator*(int, const value_ref_wrapper<double>&);

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>&, const value_ref_wrapper<double>&);

#endif // _ValueRefPythonParser_h_

