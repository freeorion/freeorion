// -*- C++ -*-
#ifndef _EffectParser_h_
#define _EffectParser_h_

#include "Lexer.h"

#include <boost/spirit/include/qi.hpp>


namespace Effect {

    class EffectBase;

}

namespace parse {

    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        Effect::EffectBase* (), // TODO: Change this type to Effect::Base in the FO code.
        parse::skipper_type
    > effect_parser_rule;

    /** Returns a const reference to the Effect parser. */
    effect_parser_rule& effect_parser();

}

#endif
