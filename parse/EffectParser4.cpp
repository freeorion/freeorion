#include "EffectParser4.h"

#include "ValueRefParser.h"
#include "EnumValueRefRules.h"
#include "../universe/Effect.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    effect_parser_rules_4::effect_parser_rules_4(
        const parse::lexer& tok,
        const effect_parser_grammar& effect_parser,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_4::base_type(start, "effect_parser_rules_4"),
        int_rules(tok, labeller, condition_parser, string_grammar),
        double_rules(tok, labeller, condition_parser, string_grammar),
        star_type_rules(tok, labeller, condition_parser),
        planet_type_rules(tok, labeller, condition_parser),
        planet_size_rules(tok, labeller, condition_parser),
        one_or_more_effects(effect_parser)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;
        qi::_5_type _5;
        qi::_6_type _6;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        using phoenix::new_;
        using phoenix::construct;
        using phoenix::push_back;

        create_planet
            =   (       omit_[tok.CreatePlanet_]
                        >   labeller.rule(Type_token)        >   planet_type_rules.expr
                        >   labeller.rule(PlanetSize_token)  >   planet_size_rules.expr
                        > -(labeller.rule(Name_token)        >   string_grammar      )
                        > -(labeller.rule(Effects_token) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreatePlanet>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_vector_(_4, _pass))) ]
            ;

        create_building
            =   (       omit_[tok.CreateBuilding_]
                        >   labeller.rule(Type_token)        >   string_grammar
                        > -(labeller.rule(Name_token)        >   string_grammar )
                        > -(labeller.rule(Effects_token) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateBuilding>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_vector_(_3, _pass))) ]
            ;

        create_ship_1
            =   ((       omit_[tok.CreateShip_]
                         >>  labeller.rule(DesignID_token)
                 )  >   int_rules.expr
                 > -(labeller.rule(Empire_token)      >   int_rules.expr    )
                 > -(labeller.rule(Species_token)     >   string_grammar )
                 > -(labeller.rule(Name_token)        >   string_grammar )
                 > -(labeller.rule(Effects_token)     >   one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateShip>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass),
                    deconstruct_movable_vector_(_5, _pass))) ]
            ;

        create_ship_2
            =   ((       omit_[tok.CreateShip_]
                         >>  labeller.rule(DesignName_token)  >>  string_grammar
                 )
                 > -(labeller.rule(Empire_token)      >   int_rules.expr    )
                 > -(labeller.rule(Species_token)     >   string_grammar )
                 > -(labeller.rule(Name_token)        >   string_grammar )
                 > -(labeller.rule(Effects_token)     >   one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateShip>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass),
                    deconstruct_movable_vector_(_5, _pass))) ]
            ;

        create_field_1
            =   ((       omit_[tok.CreateField_]
                         >>  labeller.rule(Type_token)    >>  string_grammar
                         >>  labeller.rule(Size_token)
                 )
                 >   double_rules.expr
                 > -(labeller.rule(Name_token)    >   string_grammar )
                 > -(labeller.rule(Effects_token) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateField>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_vector_(_4, _pass))) ]
            ;

        create_field_2
            =   ((       omit_[tok.CreateField_]
                         >>  labeller.rule(Type_token)    >>  string_grammar
                         >>  labeller.rule(X_token)
                 )
                 >   double_rules.expr
                 >   labeller.rule(Y_token)       >   double_rules.expr
                 >   labeller.rule(Size_token)    >   double_rules.expr
                 > -(labeller.rule(Name_token)    >   string_grammar )
                 > -(labeller.rule(Effects_token) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateField>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass),
                    deconstruct_movable_(_5, _pass),
                    deconstruct_movable_vector_(_6, _pass))) ]
            ;

        create_system_1
            =   ((       omit_[tok.CreateSystem_]
                         >>  labeller.rule(Type_token)
                 )
                 >   star_type_rules.expr 
                 >   labeller.rule(X_token)       >   double_rules.expr    
                 >   labeller.rule(Y_token)       >   double_rules.expr    
                 > -(labeller.rule(Name_token)    >   string_grammar    )
                 > -(labeller.rule(Effects_token) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateSystem>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass),
                    deconstruct_movable_vector_(_5, _pass))) ]
            ;

        create_system_2
            =   ((       omit_[tok.CreateSystem_]
                         >>  labeller.rule(X_token)
                 )
                 >   double_rules.expr 
                 >   labeller.rule(Y_token)       >   double_rules.expr 
                 > -(labeller.rule(Name_token)    >   string_grammar )
                 > -(labeller.rule(Effects_token) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateSystem>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_vector_(_4, _pass))) ]
            ;

        start
            %=  create_planet
            |   create_building
            |   create_ship_1
            |   create_ship_2
            |   create_field_1
            |   create_field_2
            |   create_system_1
            |   create_system_2
            ;

        create_planet.name("CreatePlanet");
        create_building.name("CreateBuilding");
        create_ship_1.name("CreateShip");
        create_ship_2.name("CreateShip");
        create_field_1.name("CreateField");
        create_field_2.name("CreateField");
        create_system_1.name("CreateSystem");
        create_system_2.name("CreateSystem");

#if DEBUG_EFFECT_PARSERS
        debug(create_planet);
        debug(create_building);
        debug(create_ship_1);
        debug(create_ship_2);
        debug(create_field_1);
        debug(create_field_2);
        debug(create_system_1);
        debug(create_system_2);
#endif
    }

} }
