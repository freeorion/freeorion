#include "EffectParser3.h"

#include "ValueRefParserImpl.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    effect_parser_rules_3::effect_parser_rules_3(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_3::base_type(start, "effect_parser_rules_3"),
        double_rules(tok, labeller, condition_parser, string_grammar),
        star_type_rules(tok, labeller, condition_parser)
    {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_val_type _val;
        using phoenix::new_;

        move_to
            =    tok.MoveTo_
            >    labeller.rule(Destination_token) > condition_parser [ _val = new_<Effect::MoveTo>(_1) ]
            ;

        move_in_orbit
            =    tok.MoveInOrbit_
            >    labeller.rule(Speed_token) >  double_rules.expr [ _a = _1 ]
            >   (
                (labeller.rule(Focus_token) >  condition_parser [ _val = new_<Effect::MoveInOrbit>(_a, _1) ])
                |
                (
                    labeller.rule(X_token)     >  double_rules.expr [ _b = _1 ]
                    >   labeller.rule(Y_token)     >  double_rules.expr [ _val = new_<Effect::MoveInOrbit>(_a, _b, _1) ]
                )
            )
            ;

        move_towards
            =    tok.MoveTowards_
            >    labeller.rule(Speed_token) > double_rules.expr [ _a = _1 ]
            >    (
                (labeller.rule(Target_token) >  condition_parser [ _val = new_<Effect::MoveTowards>(_a, _1) ])
                |
                (
                    labeller.rule(X_token)     > double_rules.expr [ _b = _1 ]
                    >   labeller.rule(Y_token)     > double_rules.expr [ _val = new_<Effect::MoveTowards>(_a, _b, _1) ]
                )
            )
            ;

        set_destination
            =    tok.SetDestination_
            >    labeller.rule(Destination_token) > condition_parser [ _val = new_<Effect::SetDestination>(_1) ]
            ;

        set_aggression
            =   tok.SetAggressive_  [ _val = new_<Effect::SetAggression>(true) ]
            |   tok.SetPassive_     [ _val = new_<Effect::SetAggression>(false) ]
            ;

        destroy
            =    tok.Destroy_ [ _val = new_<Effect::Destroy>() ]
            ;

        noop
            =    tok.NoOp_ [ _val = new_<Effect::NoOp>() ]
            ;

        victory
            =    tok.Victory_
            >    labeller.rule(Reason_token) > tok.string [ _val = new_<Effect::Victory>(_1) ]
            ;

        add_special_1
            =   tok.AddSpecial_
            >   labeller.rule(Name_token) > string_grammar [ _val = new_<Effect::AddSpecial>(_1) ]
            ;

        add_special_2
            =  ((tok.AddSpecial_ | tok.SetSpecialCapacity_)
                >>  labeller.rule(Name_token) >> string_grammar [ _c = _1 ]
                >> (labeller.rule(Capacity_token) | labeller.rule(Value_token))
               )
            >   double_rules.expr [ _val = new_<Effect::AddSpecial>(_c, _1) ]
            ;

        remove_special
            =   tok.RemoveSpecial_
            >   labeller.rule(Name_token) > string_grammar [ _val = new_<Effect::RemoveSpecial>(_1) ]
            ;

        add_starlanes
            =   tok.AddStarlanes_
            >   labeller.rule(Endpoint_token) > condition_parser [ _val = new_<Effect::AddStarlanes>(_1) ]
            ;

        remove_starlanes
            =   tok.RemoveStarlanes_
            >   labeller.rule(Endpoint_token) > condition_parser [ _val = new_<Effect::RemoveStarlanes>(_1) ]
            ;

        set_star_type
            =   tok.SetStarType_
            >   labeller.rule(Type_token) > star_type_rules.expr [ _val = new_<Effect::SetStarType>(_1) ]
            ;

        set_texture
            =   tok.SetTexture_
            >   labeller.rule(Name_token) > tok.string [ _val = new_<Effect::SetTexture>(_1) ]
            ;

        start
            %=  move_to
            |   move_in_orbit
            |   move_towards
            |   set_destination
            |   set_aggression
            |   destroy
            |   noop
            |   victory
            |   add_special_2
            |   add_special_1
            |   remove_special
            |   add_starlanes
            |   remove_starlanes
            |   set_star_type
            |   set_texture
            ;

        move_to.name("MoveTo");
        move_in_orbit.name("MoveInOrbit");
        move_towards.name("MoveTowards");
        set_destination.name("SetDestination");
        set_aggression.name("SetAggression");
        destroy.name("Destroy");
        noop.name("NoOp");
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
        debug(noop);
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
} }
