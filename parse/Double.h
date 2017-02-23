#ifndef _Double_h_
#define _Double_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include <boost/spirit/include/qi.hpp>

namespace parse {
    typedef detail::rule<
        double ()
    > double_parser_rule;

    extern double_parser_rule double_;
}

#endif
