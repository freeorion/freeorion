// -*- C++ -*-
#ifndef _Int_h_
#define _Int_h_

#include "Lexer.h"

#include <boost/spirit/include/qi.hpp>

namespace parse {

    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        int (),
        parse::skipper_type
    > int_parser_rule;

    extern int_parser_rule int_;

}

#endif
