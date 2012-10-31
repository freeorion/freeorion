// -*- C++ -*-
#ifndef _EffectParserImpl_h_
#define _EffectParserImpl_h_

#include "EffectParser.h"


#define DEBUG_EFFECT_PARSERS 0

namespace parse { namespace detail {

    extern effect_parser_rule effect_parser;

    const effect_parser_rule& effect_parser_1();
    const effect_parser_rule& effect_parser_2();
    const effect_parser_rule& effect_parser_3();
    const effect_parser_rule& effect_parser_4();

} }

#endif
