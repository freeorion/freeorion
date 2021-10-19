#ifndef _EnumPythonParser_h
#define _EnumPythonParser_h

#include <boost/python/dict.hpp>

template<typename E>
struct enum_wrapper {
    enum_wrapper(E value_) : value(value_) { }

    E value;
};

void RegisterGlobalsEnums(boost::python::dict& globals);

#endif // _EnumPythonParser_h
