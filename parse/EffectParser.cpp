#include "EffectParserImpl.h"


namespace parse {
    effects_parser_grammar::effects_parser_grammar(
        const parse::lexer& tok) :
        effects_parser_grammar::base_type(start, "effects_parser_grammar"),
        effect_parser_1(tok),
        effect_parser_2(tok),
        effect_parser_3(tok),
        effect_parser_4(tok, *this),
        effect_parser_5(*this)
    {
        start
            = effect_parser_1
            | effect_parser_2
            | effect_parser_3
            | effect_parser_4
            | effect_parser_5
            ;
        start.name("Effect");
    }

}

