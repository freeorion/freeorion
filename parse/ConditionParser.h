#ifndef _ConditionParser_h_
#define _ConditionParser_h_

#include "Lexer.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "EnumParser.h"
#include "MovableEnvelope.h"

#include <boost/spirit/include/qi.hpp>

namespace Condition {
    enum SortingMethod : int;
    enum ComparisonType : int;
    enum ContentType : int;
    struct ConditionBase;
}

namespace parse { namespace detail {
    using condition_payload        = MovableEnvelope<Condition::ConditionBase>;
    using condition_signature      = condition_payload ();
    using condition_parser_rule    = rule<condition_signature>;
    using condition_parser_grammar = grammar<condition_signature>;
}}

namespace parse {
    struct conditions_parser_grammar : public detail::condition_parser_grammar {
        conditions_parser_grammar(
            const parse::lexer& tok,
            detail::Labeller& label);
        ~conditions_parser_grammar();

        detail::condition_parser_rule start;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}

#endif
