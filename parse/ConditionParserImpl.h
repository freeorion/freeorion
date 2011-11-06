// -*- C++ -*-
#ifndef _ConditionParserImpl_h_
#define _ConditionParserImpl_h_

#include "ConditionParser.h"


#define DEBUG_CONDITION_PARSERS 0

namespace parse { namespace detail {

    extern condition_parser_rule condition_parser;

    const condition_parser_rule& condition_parser_1();
    const condition_parser_rule& condition_parser_2();
    const condition_parser_rule& condition_parser_3();

} }

#endif
