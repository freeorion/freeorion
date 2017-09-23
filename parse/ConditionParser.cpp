#include "ConditionParserImpl.h"


namespace parse {
    namespace detail {
        condition_parser_rule condition_parser;
    }

    condition_parser_rule& condition_parser() {
        static bool once = true;
        if (once) {
            once = false;
            detail::condition_parser
                %=   detail::condition_parser_1()
                |    detail::condition_parser_2()
                |    detail::condition_parser_3()
                |    detail::condition_parser_4()
                |    detail::condition_parser_5()
                |    detail::condition_parser_6()
                |    detail::condition_parser_7()
                ;
            detail::condition_parser.name("Condition");
#if DEBUG_CONDITION_PARSERS
            debug(detail::condition_parser);
#endif
        }

        return detail::condition_parser;
    }

    conditions_parser_grammar::conditions_parser_grammar(
        const parse::lexer& tok,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        conditions_parser_grammar::base_type(start, "conditions_parser_grammar"),
        condition_parser_1(tok, *this, string_grammar),
        condition_parser_2(tok, *this, string_grammar),
        condition_parser_3(tok, *this, string_grammar),
        condition_parser_4(tok, *this, string_grammar),
        condition_parser_5(tok, *this, string_grammar),
        condition_parser_6(tok, string_grammar),
        condition_parser_7(tok, *this, string_grammar)
    {
        start
            = condition_parser_1
            | condition_parser_2
            | condition_parser_3
            | condition_parser_4
            | condition_parser_5
            | condition_parser_6
            | condition_parser_7
            ;
        start.name("Condition");
    }

}
