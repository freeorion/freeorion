#include "ConditionParserImpl.h"


namespace parse {
    conditions_parser_grammar::conditions_parser_grammar(
        const parse::lexer& tok
    ) :
        conditions_parser_grammar::base_type(start, "conditions_parser_grammar"),
        string_grammar(tok, *this),
        condition_parser_1(tok, *this, string_grammar),
        condition_parser_2(tok, *this, string_grammar),
        condition_parser_3(tok, *this, string_grammar),
        condition_parser_4(tok, *this, string_grammar),
        condition_parser_5(tok, *this, string_grammar),
        condition_parser_6(tok, *this, string_grammar),
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
