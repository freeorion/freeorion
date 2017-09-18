#include "EffectParserImpl.h"

#include "ParseImpl.h"
#include "ConditionParserImpl.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace {
    struct effect_parser_rules_5 {
        effect_parser_rules_5(const parse::effect_parser_rule& effect_parser) {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::push_back;

            const parse::lexer& tok =   parse::lexer::instance();

            conditional
                =   (       tok.If_
                        >   parse::detail::label(Condition_token)   >   parse::detail::condition_parser [ _a = _1 ]
                        >   parse::detail::label(Effects_token)
                        >   (
                                ('[' > +effect_parser [ push_back(_b, _1) ] > ']')
                            |    effect_parser [ push_back(_b, _1) ]
                            )
                        > -(parse::detail::label(Else_token)
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

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            qi::locals<
                Condition::ConditionBase*,
                std::vector<Effect::EffectBase*>,
                std::vector<Effect::EffectBase*>
            >
        > conditional_rule;

        conditional_rule            conditional;
        parse::effect_parser_rule   start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_5(const effect_parser_rule& effect_parser) {
        static effect_parser_rules_5 retval(effect_parser);
        return retval.start;
    }
} }
