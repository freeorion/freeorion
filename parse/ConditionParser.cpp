#include "ConditionParserImpl.h"

#include "ConditionParser1.h"
#include "ConditionParser2.h"
#include "ConditionParser3.h"
#include "ConditionParser4.h"
#include "ConditionParser5.h"
#include "ConditionParser6.h"
#include "ConditionParser7.h"

#include "../universe/Condition.h"

//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

namespace parse {
    struct conditions_parser_grammar::Impl {
        Impl(conditions_parser_grammar& conditions_parser_grammar,
             const parse::lexer& tok,
             detail::Labeller& label
            ) :
            string_grammar(tok, label, conditions_parser_grammar),
            condition_parser_1(tok, label, conditions_parser_grammar, string_grammar),
            condition_parser_2(tok, label, conditions_parser_grammar, string_grammar),
            condition_parser_3(tok, label, conditions_parser_grammar, string_grammar),
            condition_parser_4(tok, label, conditions_parser_grammar, string_grammar),
            condition_parser_5(tok, label, conditions_parser_grammar, string_grammar),
            condition_parser_6(tok, label, conditions_parser_grammar, string_grammar),
            condition_parser_7(tok, label, conditions_parser_grammar, string_grammar)
        {}

        const parse::string_parser_grammar string_grammar;
        detail::condition_parser_rules_1 condition_parser_1;
        detail::condition_parser_rules_2 condition_parser_2;
        detail::condition_parser_rules_3 condition_parser_3;
        detail::condition_parser_rules_4 condition_parser_4;
        detail::condition_parser_rules_5 condition_parser_5;
        detail::condition_parser_rules_6 condition_parser_6;
        detail::condition_parser_rules_7 condition_parser_7;
    };

    conditions_parser_grammar::conditions_parser_grammar(
        const parse::lexer& tok,
        detail::Labeller& label
    ) :
        conditions_parser_grammar::base_type(start, "conditions_parser_grammar"),
        m_impl(boost::make_unique<conditions_parser_grammar::Impl>(*this, tok, label))
    {
        start
            = m_impl->condition_parser_1
            | m_impl->condition_parser_2
            | m_impl->condition_parser_3
            | m_impl->condition_parser_4
            | m_impl->condition_parser_5
            | m_impl->condition_parser_6
            | m_impl->condition_parser_7
            ;
        start.name("Condition");
    }

    conditions_parser_grammar::~conditions_parser_grammar()
    {}
}
