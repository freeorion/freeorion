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
                                                 Labeller& labeller,
                                                 const condition_parser_grammar& condition_parser) :
        effect_parser_rules_5::base_type(start, "effect_parser_rules_5")
    {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        using phoenix::new_;
        using phoenix::construct;

        conditional
            =   (       tok.If_
                        >   labeller.rule(Condition_token)   >   condition_parser [ _a = _1 ]
                        >   labeller.rule(Effects_token)
                        >   (
                            ('[' > +effect_parser [ emplace_back_1_(_b, _1) ] > ']')
                            |    effect_parser [ emplace_back_1_(_b, _1) ]
                        )
                        > -(labeller.rule(Else_token)
                            >   (
                                ('[' > +effect_parser [ emplace_back_1_(_c, _1) ] > ']')
                                |    effect_parser [ emplace_back_1_(_c, _1) ]
                            )
                           )
                ) [ _val = construct_movable_(new_<Effect::Conditional>(
                    deconstruct_movable_(_a, _pass),
                    deconstruct_movable_vector_(_b, _pass),
                    deconstruct_movable_vector_(_c, _pass))) ]
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
