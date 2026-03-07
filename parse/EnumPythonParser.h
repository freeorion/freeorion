#ifndef _EnumPythonParser_h
#define _EnumPythonParser_h

#include <functional>

#include "ValueRefPythonParser.h"

namespace boost::python {
    class dict;
}

template<typename E>
struct enum_wrapper {
    enum_wrapper(E value_) : value(value_) { }

    const E value;

    auto operator<=>(const enum_wrapper<E>&) const = default;
};

template<typename E>
struct std::hash<enum_wrapper<E>> {
    std::size_t operator()(const enum_wrapper<E>& e) const noexcept {
        return std::hash<E>{}(e.value);
    }
};

template<typename E>
std::unique_ptr<ValueRef::ValueRef<E>> pyobject_to_vref_enum(const boost::python::object& obj) {
    auto arg = boost::python::extract<value_ref_wrapper<E>>(obj);
    if (arg.check()) {
        return ValueRef::CloneUnique(arg().value_ref);
    }
    return std::make_unique<ValueRef::Constant<E>>(boost::python::extract<enum_wrapper<E>>(obj)().value);
}

void RegisterGlobalsEnums(boost::python::dict& globals);

#endif // _EnumPythonParser_h
