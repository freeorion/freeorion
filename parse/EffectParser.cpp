#include "EffectParser.h"
#include "EffectParserImpl.h"

#include "ConditionParserImpl.h"

#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace parse {
    effects_parser_grammar::effects_parser_grammar(
        const parse::lexer& tok, detail::Labeller& labeller) :
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

    effects_group_grammar::effects_group_grammar(const lexer& tok, detail::Labeller& labeller) :
        effects_group_grammar::base_type(start, "effects_group_grammar"),
        effects_grammar(tok, labeller)
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

            effects_group
                =   tok.EffectsGroup_
                > -(labeller.rule(Description_token)      > tok.string [ _g = _1 ])
                >   labeller.rule(Scope_token)            > parse::detail::condition_parser [ _a = _1 ]
                > -(labeller.rule(Activation_token)       > parse::detail::condition_parser [ _b = _1 ])
                > -(labeller.rule(StackingGroup_token)    > tok.string [ _c = _1 ])
                > -(labeller.rule(AccountingLabel_token)  > tok.string [ _e = _1 ])
                > ((labeller.rule(Priority_token)         > tok.int_ [ _f = _1 ]) | eps [ _f = 100 ])
                >   labeller.rule(Effects_token)
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
}
