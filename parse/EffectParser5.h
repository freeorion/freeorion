#ifndef _EffectParser5_h_
#define _EffectParser5_h_

#include "EffectParserImpl.h"

 namespace parse { namespace detail {
    struct effect_parser_rules_5 : public effect_parser_grammar {
        effect_parser_rules_5(const parse::lexer& tok,
                              const effect_parser_grammar& effect_parser,
                              Labeller& label,
                              const condition_parser_grammar& condition_parser);

        single_or_bracketed_repeat<effect_parser_grammar> one_or_more_effects;
        effect_parser_rule conditional;
        effect_parser_rule start;
    };
    }
 }

#endif // _EffectParser5_h_
