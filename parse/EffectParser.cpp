#include "EffectParserImpl.h"


namespace parse {
    namespace detail {
        effect_parser_rule effect_parser;
    }

    effect_parser_rule& effect_parser() {
        static bool once = true;
        if (once) {
            once = false;
            detail::effect_parser
                =    detail::effect_parser_1()
                |    detail::effect_parser_2()
                |    detail::effect_parser_3()
                |    detail::effect_parser_4x(detail::effect_parser)
                |    detail::effect_parser_5x(detail::effect_parser)
                ;
            detail::effect_parser.name("Effect");
#if DEBUG_EFFECT_PARSERS
            debug(detail::effect_parser);
#endif
        }
        return detail::effect_parser;
    }

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

