#include "EffectParserImpl.h"

#include "EffectParser1.h"
#include "EffectParser2.h"
#include "EffectParser3.h"
#include "EffectParser4.h"
#include "EffectParser5.h"

#include "../universe/Condition.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

namespace parse {

    detail::MovableEnvelope<Effect::EffectsGroup> construct_EffectsGroup(
        const detail::MovableEnvelope<Condition::ConditionBase>& scope,
        const detail::MovableEnvelope<Condition::ConditionBase>& activation,
        const std::vector<detail::effect_payload>& effects,
        const std::string& accounting_label,
        const std::string& stacking_group,
        int priority,
        const std::string& description,
        bool& pass)
    {
        return detail::MovableEnvelope<Effect::EffectsGroup>(
            boost::make_unique<Effect::EffectsGroup>(
                scope.OpenEnvelope(pass),
                activation.OpenEnvelope(pass),
                OpenEnvelopes(effects, pass),
                accounting_label,
                stacking_group,
                priority,
                description
            ));
    }
    BOOST_PHOENIX_ADAPT_FUNCTION(detail::MovableEnvelope<Effect::EffectsGroup>,
                                 construct_EffectsGroup_, construct_EffectsGroup, 8)

    /** effects_parser_grammar::Impl holds the rules for
        effects_parser_grammar. */
    struct effects_parser_grammar::Impl {
        Impl (const effects_parser_grammar& effects_parser_grammar,
            const lexer& tok,
            detail::Labeller& label,
            const detail::condition_parser_grammar& condition_parser,
            const detail::value_ref_grammar<std::string>& string_grammar
        ) :
            effect_parser_1(tok, label, condition_parser, string_grammar),
            effect_parser_2(tok, label, condition_parser, string_grammar),
            effect_parser_3(tok, label, condition_parser, string_grammar),
            effect_parser_4(tok, effects_parser_grammar, label, condition_parser, string_grammar),
            effect_parser_5(tok, effects_parser_grammar, label, condition_parser)
        {}

        detail::effect_parser_rules_1 effect_parser_1;
        detail::effect_parser_rules_2 effect_parser_2;
        detail::effect_parser_rules_3 effect_parser_3;
        detail::effect_parser_rules_4 effect_parser_4;
        detail::effect_parser_rules_5 effect_parser_5;
    };

    effects_parser_grammar::effects_parser_grammar(
        const lexer& tok,
        detail::Labeller& label,
        const detail::condition_parser_grammar& condition_parser,
        const detail::value_ref_grammar<std::string>& string_grammar
    ) :
        effects_parser_grammar::base_type(start, "effects_parser_grammar"),
        m_impl(boost::make_unique<effects_parser_grammar::Impl>(*this, tok, label, condition_parser, string_grammar))
    {
        start
            = m_impl->effect_parser_1
            | m_impl->effect_parser_2
            | m_impl->effect_parser_3
            | m_impl->effect_parser_4
            | m_impl->effect_parser_5
            ;
        start.name("Effect");
    }

    effects_parser_grammar::~effects_parser_grammar()
    {};

    effects_group_grammar::effects_group_grammar(
        const lexer& tok,
        detail::Labeller& label,
        const detail::condition_parser_grammar& condition_parser,
        const detail::value_ref_grammar<std::string>& string_grammar
    ) :
        effects_group_grammar::base_type(start, "effects_group_grammar"),
        effects_grammar(tok, label, condition_parser, string_grammar),
        one_or_more_effects(effects_grammar),
        one_or_more_groups(effects_group)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;
        using phoenix::new_;
        using phoenix::push_back;

        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_f_type _f;
        qi::_val_type _val;
        qi::lit_type lit;
        qi::eps_type eps;
        qi::_pass_type _pass;
        const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;

        effects_group
            =   tok.EffectsGroup_
            > -(label(tok.Description_)      > tok.string [ _f = _1 ])
            >   label(tok.Scope_)            > condition_parser [ _a = _1 ]
            > -(label(tok.Activation_)       > condition_parser [ _b = _1 ])
            > -(label(tok.StackingGroup_)    > tok.string [ _c = _1 ])
            > -(label(tok.AccountingLabel_)  > tok.string [ _d = _1 ])
            > ((label(tok.Priority_)         > tok.int_ [ _e = _1 ]) | eps [ _e = 100 ])
            >   label(tok.Effects_)
            >   one_or_more_effects
            [ _val = construct_EffectsGroup_(_a, _b, _1, _d, _c, _e, _f, _pass) ]
            ;

        start %=  one_or_more_groups;

        effects_group.name("EffectsGroup");
        start.name("EffectsGroups");

#if DEBUG_PARSERS
        debug(effects_group);
        debug(start);
#endif
    }
}
