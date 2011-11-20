#include "ConditionParserImpl.h"


namespace parse {
    namespace detail {
        condition_parser_rule condition_parser;
    }

    condition_parser_rule& condition_parser()
    {
        static bool once = true;
        if (once) {
            detail::condition_parser
                %=   detail::condition_parser_1()
                |    detail::condition_parser_2()
                |    detail::condition_parser_3()
                ;
            detail::condition_parser.name("Condition");
#if DEBUG_CONDITION_PARSERS
            debug(detail::condition_parser);
#endif
            once = false;
        }
        return detail::condition_parser;
    }
}
