#include "EffectParserImpl.h"
#include "ConditionParserImpl.h"

#include "Label.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace {
    struct effect_parser_rules_5 {
        effect_parser_rules_5() {
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
                        >   parse::label(Condition_token)   >   parse::detail::condition_parser [ _a = _1 ]
                        >   parse::label(Effects_token)
                        >   (
                                '[' > +parse::effect_parser() [ push_back(_b, _1) ] > ']'
                            |   parse::effect_parser() [ push_back(_b, _1) ]
                            )
                        > -(parse::label(Else_token)
                        >   (
                                '[' > +parse::effect_parser() [ push_back(_c, _1) ] > ']'
                            |   parse::effect_parser() [ push_back(_c, _1) ]
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                Condition::ConditionBase*,
                std::vector<Effect::EffectBase*>,
                std::vector<Effect::EffectBase*>
            >,
            parse::skipper_type
        > conditional_rule;

        conditional_rule            conditional;
        parse::effect_parser_rule   start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_5() {
        static effect_parser_rules_5 retval;
        return retval.start;
    }
} }
