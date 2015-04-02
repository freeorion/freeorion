#include "ValueRefParserImpl.h"

namespace parse {
    struct int_complex_parser_rules {
        int_complex_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_val_type _val;
            using phoenix::construct;
            using phoenix::new_;

            const parse::lexer& tok =                                                   parse::lexer::instance();
            const int_rule& int_value_ref =                                             int_simple();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =   parse::value_ref_parser<std::string>();

            building_types_owned
                =   (
                            tok.BuildingTypesOwned_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token) >     string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            building_types_produced
                =   (
                            tok.BuildingTypesProduced_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token) >     string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            building_types_scrapped
                =   (
                            tok.BuildingTypesScrapped_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            empire_ships_destroyed
                =   (
                            tok.EmpireShipsDestroyed_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Empire_token) >   int_value_ref [ _c = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            jumps_between
                =   (
                            tok.JumpsBetween_ [ _a = construct<std::string>(_1) ]
                        >   parse::label(Object_token) >   (int_value_ref [ _b = _1 ] | int_var_statistic() [ _b = _1 ])
                        >   parse::label(Object_token) >   (int_value_ref [ _c = _1 ] | int_var_statistic() [ _c = _1 ])
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            //jumps_between_by_empire_supply
            //    =   (
            //                tok.JumpsBetweenByEmpireSupplyConnections_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Object_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Object_token) >>   int_value_ref [ _c = _1 ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _f = _1 ]
            //        ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
            //    ;

            outposts_owned
                =   (
                            tok.OutpostsOwned_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            parts_in_ship_design
                =   (
                            tok.PartsInShipDesign_[ _a = construct<std::string>(_1) ]
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                        > ( parse::label(Design_token) >   int_value_ref [ _b = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            part_class_in_ship_design
                =   (
                            tok.PartClassInShipDesign_  [ _a = construct<std::string>(_1) ]
                        //> ( parse::label(Class_token) >>
                        //    as_string [ parse::enum_parser<ShipPartClass>() ]
                        //    [ _d = new_<ValueRef::Constant<std::string> >(_1) ]
                        //  )
                        > ( parse::label(Class_token) >
                            ( tok.ShortRange_       | tok.Missiles_     | tok.Fighters_
                            | tok.PointDefense_     | tok.Shield_       | tok.Armour_
                            | tok.Troops_           | tok.Detection_    | tok.Stealth_
                            | tok.Fuel_             | tok.Colony_       | tok.Speed_
                            | tok.General_          | tok.Bombard_      | tok.Research_
                            | tok.Industry_         | tok.Trade_        | tok.ProductionLocation_
                            ) [ _d = new_<ValueRef::Constant<std::string> >(_1) ]
                          )
                        > ( parse::label(Design_token) >   int_value_ref [ _b = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            ship_designs_destroyed
                =   (
                            tok.ShipDesignsDestroyed_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Design_token) >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            ship_designs_lost
                =   (
                            tok.ShipDesignsLost_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Design_token) >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            ship_designs_owned
                =   (
                            tok.ShipDesignsOwned_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Design_token) >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            ship_designs_produced
                =   (
                            tok.ShipDesignsProduced_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Design_token) >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            ship_designs_scrapped
                =   (
                            tok.ShipDesignsScrapped_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Design_token) >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_colonies_owned
                =  (
                            tok.SpeciesColoniesOwned_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_planets_bombed
                =  (
                            tok.SpeciesPlanetsBombed_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_planets_depoped
                =   (
                            tok.SpeciesPlanetsDepoped_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_planets_invaded
                =   (
                            tok.SpeciesPlanetsInvaded_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_ships_destroyed
                =   (
                            tok.SpeciesShipsDestroyed_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_ships_lost
                =   (
                            tok.SpeciesShipsLost_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_ships_owned
                =   (
                            tok.SpeciesShipsOwned_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_ships_produced
                =   (
                            tok.SpeciesShipsProduced_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            species_ships_scrapped
                =   (
                            tok.SpeciesShipsScrapped_ [ _a = construct<std::string>(_1) ]
                        >-( parse::label(Empire_token) >   int_value_ref [ _b = _1 ] )
                        >-( parse::label(Name_token)   >   string_value_ref [ _d = _1 ] )
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _f, _d, _e) ]
                ;

            start
                %=   building_types_owned
                |    building_types_produced
                |    building_types_scrapped
                |    empire_ships_destroyed
                |    jumps_between
                //|    jumps_between_by_empire_supply
                |    outposts_owned
                |    parts_in_ship_design
                |    part_class_in_ship_design
                |    ship_designs_destroyed
                |    ship_designs_lost
                |    ship_designs_owned
                |    ship_designs_produced
                |    ship_designs_scrapped
                |    species_colonies_owned
                |    species_planets_bombed
                |    species_planets_depoped
                |    species_planets_invaded
                |    species_ships_destroyed
                |    species_ships_lost
                |    species_ships_owned
                |    species_ships_produced
                |    species_ships_scrapped
                ;

            building_types_owned.name("BuildingTypesOwned");
            building_types_produced.name("BuildingTypesProduced");
            building_types_scrapped.name("BuildingTypesScrapped");
            empire_ships_destroyed.name("EmpireShipsDestroyed");
            jumps_between.name("JumpsBetween");
            //jumps_between_by_empire_supply.name("JumpsBetweenByEmpireSupplyConnections");
            outposts_owned.name("OutpostsOwned");
            parts_in_ship_design.name("PartsInShipDesign");
            part_class_in_ship_design.name("PartClassInShipDesign");
            ship_designs_destroyed.name("ShipDesignsDestroyed");
            ship_designs_lost.name("ShipDesignsLost");
            ship_designs_owned.name("ShipDesignsOwned");
            ship_designs_produced.name("ShipDesignsProduced");
            ship_designs_scrapped.name("ShipDesignsScrapped");
            species_colonies_owned.name("SpeciesColoniesOwned");
            species_planets_bombed.name("SpeciesPlanetsBombed");
            species_planets_depoped.name("SpeciesPlanetsDepoped");
            species_planets_invaded.name("SpeciesPlanetsInvaded");
            species_ships_destroyed.name("SpeciesShipsDestroyed");
            species_ships_lost.name("SpeciesShipsLost");
            species_ships_owned.name("SpeciesShipsOwned");
            species_ships_produced.name("SpeciesShipsProduced");
            species_ships_scrapped.name("SpeciesShipsScrapped");

#if DEBUG_INT_COMPLEX_PARSERS
            debug(building_types_owned);
            debug(building_types_produced);
            debug(building_types_scrapped);
            debug(empire_ships_destroyed);
            debug(jumps_between);
            //debug(jumps_between_by_empire_supply);
            debug(outposts_owned);
            debug(parts_in_ship_design);
            debug(part_class_in_ship_design);
            debug(ship_designs_destroyed);
            debug(ship_designs_lost);
            debug(ship_designs_owned);
            debug(ship_designs_produced);
            debug(ship_designs_scrapped);
            debug(species_colonies_owned);
            debug(species_planets_bombed);
            debug(species_planets_depoped);
            debug(species_planets_invaded);
            debug(species_ships_destroyed);
            debug(species_ships_lost);
            debug(species_ships_owned);
            debug(species_ships_produced);
            debug(species_ships_scrapped);

#endif
        }

        complex_variable_rule<int>::type    building_types_owned;
        complex_variable_rule<int>::type    building_types_produced;
        complex_variable_rule<int>::type    building_types_scrapped;
        complex_variable_rule<int>::type    empire_ships_destroyed;
        complex_variable_rule<int>::type    jumps_between;
        //complex_variable_rule<int>::type    jumps_between_by_empire_supply;
        complex_variable_rule<int>::type    outposts_owned;
        complex_variable_rule<int>::type    parts_in_ship_design;
        complex_variable_rule<int>::type    part_class_in_ship_design;
        complex_variable_rule<int>::type    ship_designs_destroyed;
        complex_variable_rule<int>::type    ship_designs_lost;
        complex_variable_rule<int>::type    ship_designs_owned;
        complex_variable_rule<int>::type    ship_designs_produced;
        complex_variable_rule<int>::type    ship_designs_scrapped;
        complex_variable_rule<int>::type    species_colonies_owned;
        complex_variable_rule<int>::type    species_planets_bombed;
        complex_variable_rule<int>::type    species_planets_depoped;
        complex_variable_rule<int>::type    species_planets_invaded;
        complex_variable_rule<int>::type    species_ships_destroyed;
        complex_variable_rule<int>::type    species_ships_lost;
        complex_variable_rule<int>::type    species_ships_owned;
        complex_variable_rule<int>::type    species_ships_produced;
        complex_variable_rule<int>::type    species_ships_scrapped;
        complex_variable_rule<int>::type    start;
    };

    namespace detail {
        int_complex_parser_rules int_complex_parser;
    }
}

const complex_variable_rule<int>::type& int_var_complex()
{ return parse::detail::int_complex_parser.start; }
