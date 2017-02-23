#ifndef _ConditionParser_h_
#define _ConditionParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include <boost/spirit/include/qi.hpp>

namespace Condition {
    struct ConditionBase;
}

namespace parse {
    typedef detail::rule<
        Condition::ConditionBase* ()
    > condition_parser_rule;

    /** Returns a const reference to the Condition parser. */
    condition_parser_rule& condition_parser();
}

#endif
