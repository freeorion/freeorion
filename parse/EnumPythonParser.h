#ifndef _EnumPythonParser_h
#define _EnumPythonParser_h

template<typename E>
struct enum_wrapper {
    enum_wrapper(E value_) : value(value_) { }

    E value;
};

#endif // _EnumPythonParser_h
