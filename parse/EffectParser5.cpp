#include "EffectParser5.h"

#include "ConditionParserImpl.h"
#include "../universe/Effect.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    effect_parser_rules_5::effect_parser_rules_5(const parse::lexer& tok,
                                                 const effect_parser_grammar& effect_parser,
                                                 Labeller& label,
                                                 const condition_parser_grammar& condition_parser) :
        effect_parser_rules_5::base_type(start, "effect_parser_rules_5"),
        one_or_more_effects(effect_parser)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        using phoenix::new_;
        using phoenix::construct;

        conditional
            =   ( omit_[tok.If_]
                  >   label(tok.Condition_)   >   condition_parser
                  >   label(tok.Effects_) > one_or_more_effects
                  >   -(label(tok.Else_)  > one_or_more_effects)
                ) [ _val = construct_movable_(new_<Effect::Conditional>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_vector_(_2, _pass),
                    deconstruct_movable_vector_(_3, _pass))) ]
            ;

        start
            =   conditional
            ;

        conditional.name("Conditional");

#if DEBUG_EFFECT_PARSERS
        debug(conditional);
#endif
    }
} }
