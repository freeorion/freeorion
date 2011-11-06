// -*- C++ -*-
#ifndef _ConditionParser_h_
#define _ConditionParser_h_

#include "Lexer.h"

#include <boost/spirit/include/qi.hpp>

namespace Condition {
    class ConditionBase;
}

namespace parse {

    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        Condition::ConditionBase* (),
        parse::skipper_type
    > condition_parser_rule;

    /** Returns a const reference to the Condition parser. */
    condition_parser_rule& condition_parser();

}

#endif
