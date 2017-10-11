#ifndef _EffectParser_h_
#define _EffectParser_h_

#include "ConditionParser.h"

#include <boost/spirit/include/qi.hpp>

namespace Effect {
    class EffectBase;
    class EffectsGroup;
}

namespace parse { namespace detail {
    using effect_signature      = Effect::EffectBase* ();
    using effect_parser_rule    = rule<effect_signature>;
    using effect_parser_grammar = grammar<effect_signature>;

    } //end namespace detail
} //end namespace parse

namespace parse {
    struct effects_parser_grammar : public detail::effect_parser_grammar {
        effects_parser_grammar(const lexer& tok,
                               detail::Labeller& labeller,
                               const detail::condition_parser_grammar& condition_parser,
                               const value_ref_grammar<std::string>& string_grammar);
        ~effects_parser_grammar();

        detail::effect_parser_rule start;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

    using effects_group_signature = std::vector<std::shared_ptr<Effect::EffectsGroup>> ();
    using effects_group_rule = detail::rule<effects_group_signature>;

    struct effects_group_grammar : public detail::grammar<effects_group_signature> {
        effects_group_grammar(const lexer& tok,
                              detail::Labeller& labeller,
                              const detail::condition_parser_grammar& condition_parser,
                              const value_ref_grammar<std::string>& string_grammar);

        typedef detail::rule<
            Effect::EffectsGroup* (),
            boost::spirit::qi::locals<
                Condition::ConditionBase*,
                Condition::ConditionBase*,
                std::string,
                std::vector<Effect::EffectBase*>,
                std::string,
                int,
                std::string
            >
        > effect_group_rule;

        effects_parser_grammar effects_grammar;
        effect_group_rule effects_group;
        effects_group_rule start;
    };

}

#endif
