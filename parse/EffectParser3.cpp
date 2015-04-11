#include "EffectParserImpl.h"

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace {
    struct effect_parser_rules_3 {
        effect_parser_rules_3() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_val_type _val;
            using phoenix::new_;

            const parse::lexer& tok =                                                   parse::lexer::instance();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =        parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =   parse::value_ref_parser<std::string>();
            const parse::value_ref_parser_rule<StarType>::type& star_type_value_ref =   parse::value_ref_parser<StarType>();

            move_to
                =    tok.MoveTo_
                >    parse::label(Destination_token) > parse::detail::condition_parser [ _val = new_<Effect::MoveTo>(_1) ]
                ;

            move_in_orbit
                =    tok.MoveInOrbit_
                >    parse::label(Speed_token) >  double_value_ref[ _a = _1 ]
                >   (
                        parse::label(Focus_token) >  parse::detail::condition_parser [ _val = new_<Effect::MoveInOrbit>(_a, _1) ]
                    |
                        (
                            parse::label(X_token)     >  double_value_ref [ _b = _1 ]
                        >   parse::label(Y_token)     >  double_value_ref [ _val = new_<Effect::MoveInOrbit>(_a, _b, _1) ]
                        )
                    )
                ;

            move_towards
                =    tok.MoveTowards_
                >    parse::label(Speed_token) > double_value_ref[ _a = _1 ]
                >    (
                        parse::label(Target_token) >  parse::detail::condition_parser [ _val = new_<Effect::MoveTowards>(_a, _1) ]
                     |
                        (
                            parse::label(X_token)     > double_value_ref [ _b = _1 ]
                        >   parse::label(Y_token)     > double_value_ref [ _val = new_<Effect::MoveTowards>(_a, _b, _1) ]
                        )
                    )
                ;

            set_destination
                =    tok.SetDestination_
                >    parse::label(Destination_token) > parse::detail::condition_parser [ _val = new_<Effect::SetDestination>(_1) ]
                ;

            set_aggression
                =   tok.SetAggressive_  [ _val = new_<Effect::SetAggression>(true) ]
                |   tok.SetPassive_     [ _val = new_<Effect::SetAggression>(false) ]
                ;

            destroy
                =    tok.Destroy_ [ _val = new_<Effect::Destroy>() ]
                ;

            victory
                =    tok.Victory_
                >    parse::label(Reason_token) > tok.string [ _val = new_<Effect::Victory>(_1) ]
                ;

            add_special_1
                =    tok.AddSpecial_
                >    parse::label(Name_token) > string_value_ref [ _val = new_<Effect::AddSpecial>(_1) ]
                ;

            add_special_2
                =    tok.AddSpecial_
                >>   parse::label(Name_token) >> string_value_ref [ _c = _1 ]
                >>   parse::label(Capacity_token) > double_value_ref [ _val = new_<Effect::AddSpecial>(_c, _1) ]
                ;

            remove_special
                =    tok.RemoveSpecial_
                >    parse::label(Name_token) > string_value_ref [ _val = new_<Effect::RemoveSpecial>(_1) ]
                ;

            add_starlanes
                =    tok.AddStarlanes_
                >    parse::label(Endpoint_token) > parse::detail::condition_parser [ _val = new_<Effect::AddStarlanes>(_1) ]
                ;

            remove_starlanes
                =    tok.RemoveStarlanes_
                >    parse::label(Endpoint_token) > parse::detail::condition_parser [ _val = new_<Effect::RemoveStarlanes>(_1) ]
                ;

            set_star_type
                =    tok.SetStarType_
                >    parse::label(Type_token) > star_type_value_ref [ _val = new_<Effect::SetStarType>(_1) ]
                ;

            set_texture
                =    tok.SetTexture_
                >    parse::label(Name_token) > tok.string [ _val = new_<Effect::SetTexture>(_1) ]
                ;

            start
                %=   move_to
                |    move_in_orbit
                |    move_towards
                |    set_destination
                |    set_aggression
                |    destroy
                |    victory
                |    add_special_2
                |    add_special_1
                |    remove_special
                |    add_starlanes
                |    remove_starlanes
                |    set_star_type
                |    set_texture
                ;

            move_to.name("MoveTo");
            move_in_orbit.name("MoveInOrbit");
            move_towards.name("MoveTowards");
            set_destination.name("SetDestination");
            set_aggression.name("SetAggression");
            destroy.name("Destroy");
            victory.name("Victory");
            add_special_1.name("AddSpecial");
            add_special_2.name("AddSpecial");
            remove_special.name("RemoveSpecial");
            add_starlanes.name("AddStarlanes");
            remove_starlanes.name("RemoveStarlanes");
            set_star_type.name("SetStarType");
            set_texture.name("SetTexture");

#if DEBUG_EFFECT_PARSERS
            debug(move_to);
            debug(move_in_orbit);
            debug(move_towards)
            debug(set_destination);
            debug(set_aggression);
            debug(destroy);
            debug(victory);
            debug(add_special_1);
            debug(add_special_2);
            debug(remove_special);
            debug(add_starlanes);
            debug(remove_starlanes);
            debug(set_star_type);
            debug(set_texture);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*
            >,
            parse::skipper_type
        > doubles_string_rule;

        parse::effect_parser_rule           move_to;
        doubles_string_rule                 move_in_orbit;
        doubles_string_rule                 move_towards;
        parse::effect_parser_rule           set_destination;
        parse::effect_parser_rule           set_aggression;
        parse::effect_parser_rule           destroy;
        parse::effect_parser_rule           victory;
        parse::effect_parser_rule           add_special_1;
        doubles_string_rule                 add_special_2;
        parse::effect_parser_rule           remove_special;
        parse::effect_parser_rule           add_starlanes;
        parse::effect_parser_rule           remove_starlanes;
        parse::effect_parser_rule           set_star_type;
        parse::effect_parser_rule           set_texture;
        parse::effect_parser_rule           start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_3() {
        static effect_parser_rules_3 retval;
        return retval.start;
    }
} }
