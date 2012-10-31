#include "EffectParserImpl.h"


namespace parse {
    namespace detail {
        effect_parser_rule effect_parser;
    }

    effect_parser_rule& effect_parser() {
        static bool once = true;
        if (once) {
            detail::effect_parser
                %=   detail::effect_parser_1()
                |    detail::effect_parser_2()
                |    detail::effect_parser_3()
                |    detail::effect_parser_4()
                ;
            detail::effect_parser.name("Effect");
#if DEBUG_EFFECT_PARSERS
            debug(detail::effect_parser);
#endif
            once = false;
        }
        return detail::effect_parser;
    }
}

