#ifndef COMMON_HPP
#define COMMON_HPP

#include <ostream>
#include <typeinfo>

#include <boost/spirit/include/qi_expect.hpp>

#include "universe/ValueRefFwd.h"
#include "parse/Lexer.h"

namespace std {
    std::ostream& operator << (std::ostream& stream, const std::type_info& type);
    std::ostream& operator << (std::ostream& stream, const ValueRef::OpType& type);
}

void print_expectation_failure(const boost::spirit::qi::expectation_failure<parse::token_iterator>& ex);

#endif 
