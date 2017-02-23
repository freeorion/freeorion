#ifndef _Int_h_
#define _Int_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include <boost/spirit/include/qi.hpp>

namespace parse {
    typedef detail::rule<
        int ()
    > int_parser_rule;

    extern int_parser_rule int_;
}

#endif
