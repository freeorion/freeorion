#include "EffectParser5.h"

#include "ConditionParserImpl.h"
#include "../universe/Effect.h"

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
        using phoenix::new_;
        using phoenix::push_back;

        conditional
            =   (       tok.If_
                        >   labeller.rule(Condition_token)   >   condition_parser [ _a = _1 ]
                        >   labeller.rule(Effects_token)
                        >   (
                            ('[' > +effect_parser [ push_back(_b, _1) ] > ']')
                            |    effect_parser [ push_back(_b, _1) ]
                        )
                        > -(labeller.rule(Else_token)
                            >   (
                                ('[' > +effect_parser [ push_back(_c, _1) ] > ']')
                                |    effect_parser [ push_back(_c, _1) ]
                            )
                           )
                ) [ _val = new_<Effect::Conditional>(_a, _b, _c) ]
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
