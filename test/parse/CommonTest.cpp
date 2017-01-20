#include "CommonTest.h"

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#endif

#include <boost/preprocessor.hpp>

std::ostream& std::operator << (std::ostream& stream, const std::type_info& type) {
#ifdef __GNUG__
    int status = -4;
    char* res = abi::__cxa_demangle(type.name(), nullptr, nullptr, &status);
    stream << ((status == 0) ? res : type.name());
    free(res);
#else
    stream << type.name();
#endif
    return stream;
}

std::ostream& std::operator << (std::ostream& stream, const ValueRef::OpType& type) {
#define VALUE_REF_BRANCH(unused,data,elem) \
    if(type == ValueRef::elem) stream << BOOST_PP_STRINGIZE(elem);

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

void print_expectation_failure(const boost::spirit::qi::expectation_failure<parse::token_iterator>& ex) {
    std::stringstream result;
    result << "Expected a " << ex.what_ << ", found instead \"" << std::string(ex.first->matched().begin(), ex.first->matched().end()) << "\"";
    throw std::runtime_error(result.str());
}
