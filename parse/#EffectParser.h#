#ifndef _EffectParser_h_
#define _EffectParser_h_

#include "ConditionParser.h"
#include "MovableEnvelope.h"

#include <boost/spirit/include/qi.hpp>

namespace Effect {
    class EffectBase;
    class EffectsGroup;
}

namespace parse { namespace detail {
    using effect_payload        = MovableEnvelope<Effect::EffectBase>;
    using effect_signature      = effect_payload ();
    using effect_parser_rule    = rule<effect_signature>;
    using effect_parser_grammar = grammar<effect_signature>;

    } //end namespace detail
} //end namespace parse

namespace parse {
    struct effects_parser_grammar : public detail::effect_parser_grammar {
        effects_parser_grammar(const lexer& tok,
                               detail::Labeller& label,
                               const detail::condition_parser_grammar& condition_parser,
                               const detail::value_ref_grammar<std::string>& string_grammar);
        ~effects_parser_grammar();

        detail::effect_parser_rule start;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

    using effects_group_payload = std::vector<parse::detail::MovableEnvelope<Effect::EffectsGroup>>;
    using effects_group_signature = effects_group_payload ();
    using effects_group_rule = detail::rule<effects_group_signature>;

    struct effects_group_grammar : public detail::grammar<effects_group_signature> {
        effects_group_grammar(const lexer& tok,
                              detail::Labeller& label,
                              const detail::condition_parser_grammar& condition_parser,
                              const detail::value_ref_grammar<std::string>& string_grammar);

        typedef detail::rule<
            parse::detail::MovableEnvelope<Effect::EffectsGroup> (),
            boost::spirit::qi::locals<
                parse::detail::condition_payload,
                parse::detail::condition_payload,
                std::string,
                std::string,
                int,
                std::string
            >
        > effect_group_rule;

        effects_parser_grammar effects_grammar;
        detail::single_or_bracketed_repeat<effects_parser_grammar> one_or_more_effects;
        effect_group_rule effects_group;
        detail::single_or_bracketed_repeat<effect_group_rule> one_or_more_groups;
        effects_group_rule start;
    };

}

#endif
