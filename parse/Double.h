// -*- C++ -*-
#ifndef _Double_h_
#define _Double_h_

#include "Lexer.h"

#include <boost/spirit/include/qi.hpp>

namespace parse {

    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        double (),
        parse::skipper_type
    > double_parser_rule;

    extern double_parser_rule double_;

}

#endif
