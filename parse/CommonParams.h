// -*- C++ -*-
#ifndef _Common_Params_h_
#define _Common_Params_h_

#include "Lexer.h"
#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

namespace parse { namespace detail {

    typedef qi::rule<
        token_iterator,
        bool (),
        skipper_type
    > producible_rule;
    const producible_rule& producible_parser();
} }

#endif
