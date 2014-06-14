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
            qi::_val_type _val;
            using phoenix::construct;
            using phoenix::new_;

            const parse::lexer& tok =                                                   parse::lexer::instance();
            const parse::value_ref_parser_rule<int>::type& int_value_ref =              parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =   parse::value_ref_parser<std::string>();

            empire_building_types_produced
                =   (
                            tok.EmpireBuildingTypesProduced_ [ _a = construct<std::string>(_1) ]
                        >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
                        >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _d, _e) ]
                ;

            empire_building_types_scrapped
                =   (
                            tok.EmpireBuildingTypesScrapped_ [ _a = construct<std::string>(_1) ]
                        >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
                        >   parse::label(Name_token)   >>   string_value_ref [ _d = _1 ]
                    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _d, _e) ]
                ;

            //        (
            //                tok.EmpireBuildingTypesScrapped_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token)   >>   string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireEmpireShipsDestroyed_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _c = _1 ]
            //        )
            //        |(
            //                tok.EmpireShipDesignsDestroyed_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Design_token) >>   string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireShipDesignsLost_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Design_token) >>   string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireShipDesignsProduced_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Design_token) >>   string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireShipDesignsScrapped_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Design_token) >>   string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireSpeciesPlanetsBombed_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireSpeciesPlanetsDepoped_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireSpeciesPlanetsInvaded_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireSpeciesShipsDestroyed_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireSpeciesShipsLost_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireSpeciesShipsProduced_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
            //        )
            //        |(
            //                tok.EmpireSpeciesShipsScrapped_ [ _a = construct<std::string>(_1) ]
            //            >   parse::label(Empire_token) >>   int_value_ref [ _b = _1 ]
            //            >   parse::label(Name_token) >>     string_value_ref [ _d = _1 ]
            //        )
            //    ) [ _val = new_<ValueRef::ComplexVariable<int> >(_a, _b, _c, _d, _e) ]
            //;


            //empire_empire_ships_destroyed;
            //empire_ship_designs_destroyed;
            //empire_ship_designs_lost;
            //empire_ship_designs_produced;
            //empire_ship_designs_scrapped;
            //empire_species_planets_bombed;
            //empire_species_planets_depoped;
            //empire_species_planets_invaded;
            //empire_species_ships_destroyed;
            //empire_species_ships_lost;
            //empire_species_ships_produced;
            //empire_species_ships_scrapped;


            start
                %=   empire_building_types_produced
                |    empire_building_types_scrapped
                ;
                //|   empire_empire_ships_destroyed
                //|   empire_ship_designs_destroyed
                //|   empire_ship_designs_lost
                //|   empire_ship_designs_produced
                //|   empire_ship_designs_scrapped
                //|   empire_species_planets_bombed
                //|   empire_species_planets_depoped
                //|   empire_species_planets_invaded
                //|   empire_species_ships_destroyed
                //|   empire_species_ships_lost
                //|   empire_species_ships_produced
                //|   empire_species_ships_scrapped

            empire_building_types_produced.name("EmpireBuildingTypesProduced");
            empire_building_types_scrapped.name("EmpireBuildingTypesScrapped");

#if DEBUG_INT_COMPLEX_PARSERS
            debug(empire_building_types_produced);
            debug(empire_building_types_scrapped);
#endif
        }

        complex_variable_rule<int>::type    empire_building_types_produced;
        complex_variable_rule<int>::type    empire_building_types_scrapped;
        complex_variable_rule<int>::type    empire_empire_ships_destroyed;
        complex_variable_rule<int>::type    empire_ship_designs_destroyed;
        complex_variable_rule<int>::type    empire_ship_designs_lost;
        complex_variable_rule<int>::type    empire_ship_designs_produced;
        complex_variable_rule<int>::type    empire_ship_designs_scrapped;
        complex_variable_rule<int>::type    empire_species_planets_bombed;
        complex_variable_rule<int>::type    empire_species_planets_depoped;
        complex_variable_rule<int>::type    empire_species_planets_invaded;
        complex_variable_rule<int>::type    empire_species_ships_destroyed;
        complex_variable_rule<int>::type    empire_species_ships_lost;
        complex_variable_rule<int>::type    empire_species_ships_produced;
        complex_variable_rule<int>::type    empire_species_ships_scrapped;
        complex_variable_rule<int>::type    start;
    };
}

const complex_variable_rule<int>::type& int_var_complex() {
    static parse::int_complex_parser_rules retval;
    return retval.start;
}
