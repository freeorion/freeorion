#include "EffectParser.h"
#include "EffectParserImpl.h"

#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>


namespace parse {
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

    struct effects_group_rules {
        effects_group_rules() :
            effects_grammar(parse::lexer::instance())
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
            qi::_g_type _g;
            qi::_val_type _val;
            qi::lit_type lit;
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            effects_group
                =   tok.EffectsGroup_
                > -(parse::detail::label(Description_token)      > tok.string [ _g = _1 ])
                >   parse::detail::label(Scope_token)            > parse::detail::condition_parser [ _a = _1 ]
                > -(parse::detail::label(Activation_token)       > parse::detail::condition_parser [ _b = _1 ])
                > -(parse::detail::label(StackingGroup_token)    > tok.string [ _c = _1 ])
                > -(parse::detail::label(AccountingLabel_token)  > tok.string [ _e = _1 ])
                > ((parse::detail::label(Priority_token)         > tok.int_ [ _f = _1 ]) | eps [ _f = 100 ])
                >   parse::detail::label(Effects_token)
                >   (
                            ('[' > +effects_grammar [ push_back(_d, _1) ] > ']')
                        |    effects_grammar [ push_back(_d, _1) ]
                    )
                    [ _val = new_<Effect::EffectsGroup>(_a, _b, _d, _e, _c, _f, _g) ]
                ;

            start
                =    ('[' > +effects_group [ push_back(_val, construct<std::shared_ptr<Effect::EffectsGroup>>(_1)) ] > ']')
                |     effects_group [ push_back(_val, construct<std::shared_ptr<Effect::EffectsGroup>>(_1)) ]
                ;

            effects_group.name("EffectsGroup");
            start.name("EffectsGroups");

#if DEBUG_PARSERS
            debug(effects_group);
            debug(start);
#endif
        }

        typedef parse::detail::rule<
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
        > effects_group_rule;

        parse::effects_parser_grammar effects_grammar;
        effects_group_rule effects_group;
        parse::detail::effects_group_rule start;
    };

    namespace detail {
    effects_group_rule& effects_group_parser() {
        static effects_group_rules rules;
        return rules.start;
    }
    }

}

