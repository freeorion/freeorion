#include "ValueRefParser.h"

#include "MovableEnvelope.h"
#include "../universe/ValueRef.h"
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi_as.hpp>

namespace parse {
    int_complex_parser_grammar::int_complex_parser_grammar(
        const parse::lexer& tok,
        detail::Labeller& labeller,
        const int_arithmetic_rules& _int_arith_rules,
        const detail::value_ref_grammar<std::string>& string_grammar
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
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_val_type _val;
        qi::_pass_type _pass;
        const boost::phoenix::function<detail::construct_movable> construct_movable_;
        const boost::phoenix::function<detail::deconstruct_movable> deconstruct_movable_;

        game_rule
            = ( tok.GameRule_
                > labeller.rule(Name_token) >     string_grammar
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, nullptr, nullptr, nullptr, deconstruct_movable_(_2, _pass), nullptr)) ]
            ;
         empire_name_ref
            =   (
                    (   tok.BuildingTypesOwned_
                    |   tok.BuildingTypesProduced_
                    |   tok.BuildingTypesScrapped_
                    |   tok.SpeciesColoniesOwned_
                    |   tok.SpeciesPlanetsBombed_
                    |   tok.SpeciesPlanetsDepoped_
                    |   tok.SpeciesPlanetsInvaded_
                    |   tok.SpeciesShipsDestroyed_
                    |   tok.SpeciesShipsLost_
                    |   tok.SpeciesShipsOwned_
                    |   tok.SpeciesShipsProduced_
                    |   tok.SpeciesShipsScrapped_
                    |   tok.TurnTechResearched_
                    )
                >  -(   labeller.rule(Empire_token) >   int_rules.expr)
                >  -(   labeller.rule(Name_token) >     string_grammar)
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), nullptr, nullptr, deconstruct_movable_(_3, _pass), nullptr)) ]
            ;

        empire_ships_destroyed
            =   (
                tok.EmpireShipsDestroyed_
                >-( labeller.rule(Empire_token) >   int_rules.expr )
                >-( labeller.rule(Empire_token) >   int_rules.expr )
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), deconstruct_movable_(_3, _pass), nullptr, nullptr, nullptr)) ]
            ;

        jumps_between
            = (tok.JumpsBetween_
               > labeller.rule(Object_token)
               > ( int_rules.expr
                   // "cast" the ValueRef::Statistic<int> into
                   // ValueRef::ValueRefBase<int> so the alternative contains a
                   // single type
                   | qi::as<parse::detail::MovableEnvelope<ValueRef::ValueRefBase<int>>>()[int_rules.statistic_expr])
               > labeller.rule(Object_token)
               > (int_rules.expr
                  | qi::as<parse::detail::MovableEnvelope<ValueRef::ValueRefBase<int>>>()[int_rules.statistic_expr])
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), deconstruct_movable_(_3, _pass), nullptr, nullptr, nullptr)) ]
            ;

        //jumps_between_by_empire_supply
        //    =   (
        //                tok.JumpsBetweenByEmpireSupplyConnections_ [ _a = construct<std::string>(_1) ]
        //            >   labeller.rule(Object_token) >>   int_rules.expr [ _b = _1 ]
        //            >   labeller.rule(Object_token) >>   int_rules.expr [ _c = _1 ]
        //            >   labeller.rule(Empire_token) >>   int_rules.expr [ _f = _1 ]
        //        ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_a, deconstruct_movable_(_b, _pass), deconstruct_movable_(_c, _pass), deconstruct_movable_(_f, _pass), deconstruct_movable_(_d, _pass), deconstruct_movable_(_e, _pass))) ]
        //    ;

        outposts_owned
            =   (
                tok.OutpostsOwned_
                >-( labeller.rule(Empire_token) >   int_rules.expr )
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), nullptr, nullptr, nullptr, nullptr)) ]
            ;

        parts_in_ship_design
            =   (
                tok.PartsInShipDesign_
                >-( labeller.rule(Name_token)   >   string_grammar )
                > ( labeller.rule(Design_token) >   int_rules.expr )
            ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_3, _pass), nullptr, nullptr, deconstruct_movable_(_2, _pass), nullptr)) ]
            ;

        part_class_in_ship_design
            =   (
                tok.PartOfClassInShipDesign_
                //> ( labeller.rule(Class_token) >>
                //    as_string [ ship_part_class_enum ]
                //    [ _d = construct_movable_(new_<ValueRef::Constant<std::string>>(_1)) ]
                //  )
                > ( labeller.rule(Class_token) >
                    ( tok.ShortRange_       | tok.FighterBay_   | tok.FighterWeapon_
                      | tok.Shield_           | tok.Armour_
                      | tok.Troops_           | tok.Detection_    | tok.Stealth_
                      | tok.Fuel_             | tok.Colony_       | tok.Speed_
                      | tok.General_          | tok.Bombard_      | tok.Research_
                      | tok.Industry_         | tok.Trade_        | tok.ProductionLocation_
                    )
                  )
                > ( labeller.rule(Design_token) >   int_rules.expr)
            ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_3, _pass), nullptr, nullptr, deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<std::string>>(_2)), _pass), nullptr)) ]
            ;

        ship_parts_owned_by_name
            =   (
                tok.ShipPartsOwned_
                > -( labeller.rule(Empire_token) > int_rules.expr )
                > -( labeller.rule(Name_token)   > string_grammar )
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), nullptr, nullptr, deconstruct_movable_(_3, _pass), nullptr)) ]
            ;

        ship_parts_owned_by_class
            =   (
                tok.ShipPartsOwned_
                > -( labeller.rule(Empire_token) > int_rules.expr )
                > labeller.rule(Class_token)  >> ship_part_class_enum
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<int>>(_3)), _pass), nullptr, nullptr, nullptr)) ]
            ;

        empire_design_ref
            =   (
                    (   tok.ShipDesignsDestroyed_
                    |   tok.ShipDesignsLost_
                    |   tok.ShipDesignsOwned_
                    |   tok.ShipDesignsProduced_
                    |   tok.ShipDesignsScrapped_
                    )
                >  -(   labeller.rule(Empire_token) > int_rules.expr )
                >  -(   labeller.rule(Design_token) > string_grammar )
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), nullptr, nullptr, deconstruct_movable_(_3, _pass), nullptr)) ]
            ;

        slots_in_hull
            =   (
                tok.SlotsInHull_
                >   labeller.rule(Name_token) >      string_grammar
            ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, nullptr, nullptr, nullptr, deconstruct_movable_(_2, _pass), nullptr)) ]
            ;

        slots_in_ship_design
            =   (
                tok.SlotsInShipDesign_
                >   labeller.rule(Design_token) >    int_rules.expr
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_2, _pass), nullptr, nullptr, nullptr, nullptr)) ]
            ;

        special_added_on_turn
            =   (
                tok.SpecialAddedOnTurn_
                >-( labeller.rule(Name_token)   >   string_grammar )
                >-( labeller.rule(Object_token) >   int_rules.expr )
            ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<int>>(_1, deconstruct_movable_(_3, _pass), nullptr, nullptr, deconstruct_movable_(_2, _pass), nullptr)) ]
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
            |   ship_parts_owned_by_name
            |   ship_parts_owned_by_class
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
        ship_parts_owned_by_name.name("ShipPartsOwnedByName");
        ship_parts_owned_by_class.name("ShipPartsOwnedByClass");
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
        debug(ship_parts_owned_by_name);
        debug(ship_parts_owned_by_class);
        debug(empire_design_ref);
        debug(slots_in_hull);
        debug(slots_in_ship_design);
        debug(special_added_on_turn);
#endif
    }
}
