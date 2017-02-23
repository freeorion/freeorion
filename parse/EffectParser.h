#ifndef _EffectParser_h_
#define _EffectParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include <boost/spirit/include/qi.hpp>

namespace Effect {
    class EffectBase;
}

namespace parse {
    typedef detail::rule<
        // TODO: Change this type to Effect::Base in the FO code.
        Effect::EffectBase* ()
    > effect_parser_rule;

    /** Returns a const reference to the Effect parser. */
    effect_parser_rule& effect_parser();
}

#endif
