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
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_4::base_type(start, "effect_parser_rules_4"),
        int_rules(tok, label, condition_parser, string_grammar),
        double_rules(tok, label, condition_parser, string_grammar),
        star_type_rules(tok, label, condition_parser),
        planet_type_rules(tok, label, condition_parser),
        planet_size_rules(tok, label, condition_parser),
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
                        >   label(tok.Type_)        >   planet_type_rules.expr
                        >   label(tok.PlanetSize_)  >   planet_size_rules.expr
                        > -(label(tok.Name_)        >   string_grammar      )
                        > -(label(tok.Effects_) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreatePlanet>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_vector_(_4, _pass))) ]
            ;

        create_building
            =   (       omit_[tok.CreateBuilding_]
                        >   label(tok.Type_)        >   string_grammar
                        > -(label(tok.Name_)        >   string_grammar )
                        > -(label(tok.Effects_) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateBuilding>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_vector_(_3, _pass))) ]
            ;

        create_ship_1
            =   ((       omit_[tok.CreateShip_]
                         >>  label(tok.DesignID_)
                 )  >   int_rules.expr
                 > -(label(tok.Empire_)      >   int_rules.expr    )
                 > -(label(tok.Species_)     >   string_grammar )
                 > -(label(tok.Name_)        >   string_grammar )
                 > -(label(tok.Effects_)     >   one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateShip>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass),
                    deconstruct_movable_vector_(_5, _pass))) ]
            ;

        create_ship_2
            =   ((       omit_[tok.CreateShip_]
                         >>  label(tok.DesignName_)  >>  string_grammar
                 )
                 > -(label(tok.Empire_)      >   int_rules.expr    )
                 > -(label(tok.Species_)     >   string_grammar )
                 > -(label(tok.Name_)        >   string_grammar )
                 > -(label(tok.Effects_)     >   one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateShip>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass),
                    deconstruct_movable_vector_(_5, _pass))) ]
            ;

        create_field_1
            =   ((       omit_[tok.CreateField_]
                         >>  label(tok.Type_)    >>  string_grammar
                         >>  label(tok.Size_)
                 )
                 >   double_rules.expr
                 > -(label(tok.Name_)    >   string_grammar )
                 > -(label(tok.Effects_) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateField>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_vector_(_4, _pass))) ]
            ;

        create_field_2
            =   ((       omit_[tok.CreateField_]
                         >>  label(tok.Type_)    >>  string_grammar
                         >>  label(tok.X_)
                 )
                 >   double_rules.expr
                 >   label(tok.Y_)       >   double_rules.expr
                 >   label(tok.Size_)    >   double_rules.expr
                 > -(label(tok.Name_)    >   string_grammar )
                 > -(label(tok.Effects_) > one_or_more_effects )
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
                         >>  label(tok.Type_)
                 )
                 >   star_type_rules.expr 
                 >   label(tok.X_)       >   double_rules.expr    
                 >   label(tok.Y_)       >   double_rules.expr    
                 > -(label(tok.Name_)    >   string_grammar    )
                 > -(label(tok.Effects_) > one_or_more_effects )
                ) [ _val = construct_movable_(new_<Effect::CreateSystem>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass),
                    deconstruct_movable_vector_(_5, _pass))) ]
            ;

        create_system_2
            =   ((       omit_[tok.CreateSystem_]
                         >>  label(tok.X_)
                 )
                 >   double_rules.expr 
                 >   label(tok.Y_)       >   double_rules.expr 
                 > -(label(tok.Name_)    >   string_grammar )
                 > -(label(tok.Effects_) > one_or_more_effects )
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
