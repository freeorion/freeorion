#ifndef _EffectParser5_h_
#define _EffectParser5_h_

#include "EffectParserImpl.h"

 namespace parse { namespace detail {
    struct effect_parser_rules_5 : public effect_parser_grammar {
        effect_parser_rules_5(const parse::lexer& tok,
                              const effect_parser_grammar& effect_parser,
                              Labeller& labeller,
                              const condition_parser_grammar& condition_parser);

        using conditional_rule = rule<
            effect_signature,
            boost::spirit::qi::locals<
                condition_payload,
                std::vector<effect_payload>,
                std::vector<effect_payload>
                >
            >;

        single_or_bracketed_repeat<effect_parser_grammar> one_or_more_effects;
        conditional_rule conditional;
        effect_parser_rule start;
    };
    }
 }

#endif // _EffectParser5_h_
