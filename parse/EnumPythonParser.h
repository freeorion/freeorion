#ifndef _EnumPythonParser_h
#define _EnumPythonParser_h

#include <functional>

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

void RegisterGlobalsEnums(boost::python::dict& globals);

#endif // _EnumPythonParser_h
