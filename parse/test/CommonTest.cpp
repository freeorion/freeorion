#include "CommonTest.h"

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#endif

#include <boost/preprocessor.hpp>

std::ostream& std::operator << (std::ostream& stream, const std::type_info& type) {
#ifdef __GNUG__
    int status = -4;
    char* res = abi::__cxa_demangle(type.name(), NULL, NULL, &status);
    stream << ((status == 0) ? res : type.name());
    free(res);
#else
    stream << type.name();
#endif
    return stream;
}

std::ostream& std::operator << (std::ostream& stream, const ValueRef::OpType& type) {
#define VALUE_REF_BRANCH(unused,data,elem) \
    if(type == ValueRef::elem) stream << "ValueRef::" BOOST_PP_STRINGIZE(elem);

    BOOST_PP_SEQ_FOR_EACH(VALUE_REF_BRANCH,~,
        (PLUS)
        (MINUS)
        (TIMES)
        (DIVIDE)
        (NEGATE)
        (EXPONENTIATE)
        (ABS)
        (LOGARITHM)
        (SINE)
        (COSINE)
        (MINIMUM)
        (MAXIMUM)
        (RANDOM_UNIFORM)
    )

    return stream;
}

