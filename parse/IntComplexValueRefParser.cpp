#include "ValueRefParserImpl.h"


namespace parse {
    int_complex_parser_grammar::int_complex_parser_grammar(
        const parse::lexer& tok,
        detail::Labeller& labeller,
        const parse::int_arithmetic_rules& _int_arith_rules,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        int_complex_parser_grammar::base_type(start, "int_complex_parser_grammar"),
        int_rules(_int_arith_rules),
        ship_part_class_enum(tok)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;
        using phoenix::new_;

        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_f_type _f;
        qi::_val_type _val;

        game_rule
            =   tok.GameRule_ [ _a = construct<std::string>(_1) ]
            >   labeller.rule(Name_token) >     string_grammar [ _d = _1 ]
                [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;
         empire_name_ref
            =   (
                    (   tok.BuildingTypesOwned_     [ _a = construct<std::string>(_1) ]
                    |   tok.BuildingTypesProduced_  [ _a = construct<std::string>(_1) ]
                    |   tok.BuildingTypesScrapped_  [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesColoniesOwned_   [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesPlanetsBombed_   [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesPlanetsDepoped_  [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesPlanetsInvaded_  [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesShipsDestroyed_  [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesShipsLost_       [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesShipsOwned_      [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesShipsProduced_   [ _a = construct<std::string>(_1) ]
                    |   tok.SpeciesShipsScrapped_   [ _a = construct<std::string>(_1) ]
                    |   tok.TurnTechResearched_     [ _a = construct<std::string>(_1) ]
                    )
                >  -(   labeller.rule(Empire_token) >   int_rules.expr [ _b = _1 ] )
                >  -(   labeller.rule(Name_token) >     string_grammar [ _d = _1 ] )
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        empire_ships_destroyed
            =   (
                    tok.EmpireShipsDestroyed_ [ _a = construct<std::string>(_1) ]
                    >-( labeller.rule(Empire_token) >   int_rules.expr [ _b = _1 ] )
                    >-( labeller.rule(Empire_token) >   int_rules.expr [ _c = _1 ] )
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        jumps_between
            =   (
                    tok.JumpsBetween_ [ _a = construct<std::string>(_1) ]
                    >   labeller.rule(Object_token) >   (int_rules.expr [ _b = _1 ] | int_rules.statistic_expr [ _b = _1 ])
                    >   labeller.rule(Object_token) >   (int_rules.expr [ _c = _1 ] | int_rules.statistic_expr [ _c = _1 ])
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        //jumps_between_by_empire_supply
        //    =   (
        //                tok.JumpsBetweenByEmpireSupplyConnections_ [ _a = construct<std::string>(_1) ]
        //            >   labeller.rule(Object_token) >>   int_rules.expr [ _b = _1 ]
        //            >   labeller.rule(Object_token) >>   int_rules.expr [ _c = _1 ]
        //            >   labeller.rule(Empire_token) >>   int_rules.expr [ _f = _1 ]
        //        ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
        //    ;

        outposts_owned
            =   (
                tok.OutpostsOwned_ [ _a = construct<std::string>(_1) ]
                >-( labeller.rule(Empire_token) >   int_rules.expr [ _b = _1 ] )
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        parts_in_ship_design
            =   (
                tok.PartsInShipDesign_[ _a = construct<std::string>(_1) ]
                >-( labeller.rule(Name_token)   >   string_grammar [ _d = _1 ] )
                > ( labeller.rule(Design_token) >   int_rules.expr [ _b = _1 ] )
            ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        part_class_in_ship_design
            =   (
                tok.PartOfClassInShipDesign_  [ _a = construct<std::string>(_1) ]
                //> ( labeller.rule(Class_token) >>
                //    as_string [ ship_part_class_enum ]
                //    [ _d = new_<ValueRef::Constant<std::string>>(_1) ]
                //  )
                > ( labeller.rule(Class_token) >
                    ( tok.ShortRange_       | tok.FighterBay_   | tok.FighterWeapon_
                      | tok.Shield_           | tok.Armour_
                      | tok.Troops_           | tok.Detection_    | tok.Stealth_
                      | tok.Fuel_             | tok.Colony_       | tok.Speed_
                      | tok.General_          | tok.Bombard_      | tok.Research_
                      | tok.Industry_         | tok.Trade_        | tok.ProductionLocation_
                    ) [ _d = new_<ValueRef::Constant<std::string>>(_1) ]
                  )
                > ( labeller.rule(Design_token) >   int_rules.expr [ _b = _1 ] )
            ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        ship_parts_owned
            =   (
                tok.ShipPartsOwned_ [ _a = construct<std::string>(_1) ]
                >-( labeller.rule(Empire_token)          > int_rules.expr [ _b = _1 ] )
                >-(     ( labeller.rule(Name_token)      > string_grammar [ _d = _1 ] )
                        |   ( labeller.rule(Class_token)     >>
                              ship_part_class_enum [ _c = new_<ValueRef::Constant<int>>(_1) ]
                            )
                      )
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        empire_design_ref
            =   (
                    (   tok.ShipDesignsDestroyed_ [ _a = construct<std::string>(_1) ]
                    |   tok.ShipDesignsLost_      [ _a = construct<std::string>(_1) ]
                    |   tok.ShipDesignsOwned_     [ _a = construct<std::string>(_1) ]
                    |   tok.ShipDesignsProduced_  [ _a = construct<std::string>(_1) ]
                    |   tok.ShipDesignsScrapped_  [ _a = construct<std::string>(_1) ]
                    )
                >  -(   labeller.rule(Empire_token) > int_rules.expr [ _b = _1 ] )
                >  -(   labeller.rule(Design_token) > string_grammar [ _d = _1 ] )
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        slots_in_hull
            =   (
                tok.SlotsInHull_ [ _a = construct<std::string>(_1) ]
                >   labeller.rule(Name_token) >      string_grammar [ _d = _1 ]
            ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        slots_in_ship_design
            =   (
                tok.SlotsInShipDesign_ [ _a = construct<std::string>(_1) ]
                >   labeller.rule(Design_token) >    int_rules.expr [ _b = _1 ]
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        special_added_on_turn
            =   (
                tok.SpecialAddedOnTurn_ [ _a = construct<std::string>(_1) ]
                >-( labeller.rule(Name_token)   >   string_grammar [ _d = _1 ] )
                >-( labeller.rule(Object_token) >   int_rules.expr [ _b = _1 ] )
                ) [ _val = new_<ValueRef::ComplexVariable<int>>(_a, _b, _c, _f, _d, _e) ]
            ;

        start
            %=  game_rule
            |   empire_name_ref
            |   empire_ships_destroyed
            |   jumps_between
            //|   jumps_between_by_empire_supply
            |   outposts_owned
            |   parts_in_ship_design
            |   part_class_in_ship_design
            |   ship_parts_owned
            |   empire_design_ref
            |   slots_in_hull
            |   slots_in_ship_design
            |   special_added_on_turn
            ;

        game_rule.name("GameRule");
        empire_name_ref.name("...");
        empire_ships_destroyed.name("EmpireShipsDestroyed");
        jumps_between.name("JumpsBetween");
        //jumps_between_by_empire_supply.name("JumpsBetweenByEmpireSupplyConnections");
        outposts_owned.name("OutpostsOwned");
        parts_in_ship_design.name("PartsInShipDesign");
        part_class_in_ship_design.name("PartOfClassInShipDesign");
        ship_parts_owned.name("ShipPartsOwned");
        empire_design_ref.name("ShipDesignsDestroyed, ShipDesignsLost, ShipDesignsOwned, ShipDesignsProduced, or ShipDesignsScrapped");
        slots_in_hull.name("SlotsInHull");
        slots_in_ship_design.name("SlotsInShipDesign");
        special_added_on_turn.name("SpecialAddedOnTurn");

#if DEBUG_INT_COMPLEX_PARSERS
        debug(game_rule);
        debug(empire_name_ref);
        debug(empire_ships_destroyed);
        debug(jumps_between);
        //debug(jumps_between_by_empire_supply);
        debug(outposts_owned);
        debug(parts_in_ship_design);
        debug(part_class_in_ship_design);
        debug(ship_parts_owned);
        debug(empire_design_ref);
        debug(slots_in_hull);
        debug(slots_in_ship_design);
        debug(special_added_on_turn);
#endif
    }
}
