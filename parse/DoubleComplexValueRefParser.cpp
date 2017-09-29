#include "ValueRefParserImpl.h"


namespace parse {
    struct double_complex_parser_rules {
        double_complex_parser_rules() {
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

            static const std::string TOK_SPECIES_EMPIRE_OPINION{"SpeciesEmpireOpinion"};
            static const std::string TOK_SPECIES_SPECIES_OPINION{"SpeciesSpeciesOpinion"};

            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_rule<int>& simple_int = int_simple();

            name_property_rule
                = (     tok.GameRule_           [ _a = construct<std::string>(_1) ]
                    |   tok.HullFuel_           [ _a = construct<std::string>(_1) ]
                    |   tok.HullStructure_      [ _a = construct<std::string>(_1) ]
                    |   tok.HullStealth_        [ _a = construct<std::string>(_1) ]
                    |   tok.HullSpeed_          [ _a = construct<std::string>(_1) ]
                    |   tok.PartCapacity_       [ _a = construct<std::string>(_1) ]
                    |   tok.PartSecondaryStat_  [ _a = construct<std::string>(_1) ]
                  ) >   detail::label(Name_token) > parse::string_value_ref() [ _d = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
                ;

            empire_meter_value
                = (     tok.EmpireMeterValue_ [ _a = construct<std::string>(_1)]
                     >  parse::detail::label(Empire_token) > simple_int[ _b = _1]
                     >  parse::detail::label(Meter_token) > tok.string[ _d = new_<ValueRef::Constant<std::string>>(_1)]
                  )     [_val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e)]
                ;

            direct_distance
                = (     tok.DirectDistanceBetween_ [ _a = construct<std::string>(_1) ]
                     >  parse::detail::label(Object_token) > simple_int [ _b = _1 ]
                     >  parse::detail::label(Object_token) > simple_int [ _c = _1 ]
                  )     [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
                ;

            // in shortest_path would have liked to be able to use 
            //            >   parse::detail::label(Object_token) >   (simple_int [ _b = _1 ] | int_var_statistic() [ _b = _1 ])
            //            >   parse::detail::label(Object_token) >   (simple_int [ _c = _1 ] | int_var_statistic() [ _c = _1 ])
            // but getting crashes upon program start, presumably due to initialization order problems

            shortest_path
                =   (
                            tok.ShortestPath_ [ _a = construct<std::string>(_1) ]
                        >   parse::detail::label(Object_token) > simple_int [ _b = _1 ]
                        >   parse::detail::label(Object_token) > simple_int [ _c = _1 ]
                    )       [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
                ;

            species_empire_opinion
                = (
                    ((  tok.SpeciesOpinion_ [ _a = construct<std::string>(TOK_SPECIES_EMPIRE_OPINION) ]
                       >  parse::detail::label(Species_token) > parse::string_value_ref() [ _d = _1 ]
                     )
                    >> parse::detail::label(Empire_token)
                    )  >  simple_int [ _b = _1 ]
                  )     [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
                ;

            species_species_opinion
                = (
                    ((   tok.SpeciesOpinion_ [ _a = construct<std::string>(TOK_SPECIES_SPECIES_OPINION) ]
                      >  parse::detail::label(Species_token) >  parse::string_value_ref() [ _d = _1 ]
                     )
                    >> parse::detail::label(Species_token)
                    ) >  parse::string_value_ref() [ _e = _1 ]
                  )     [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
                ;

            special_capacity
                = (
                    ((  tok.SpecialCapacity_ [ _a = construct<std::string>(_1) ]
                       >  parse::detail::label(Name_token) > parse::string_value_ref() [ _d = _1 ]
                     )
                    >> parse::detail::label(Object_token)
                    )  >  simple_int [ _b = _1 ]
                  )     [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
                ;

            start
                %=  name_property_rule
                |   empire_meter_value
                |   direct_distance
                |   shortest_path
                |   species_empire_opinion
                |   species_species_opinion
                |   special_capacity
                ;

            name_property_rule.name("GameRule; Hull Fuel, Stealth, Structure, or Speed; or PartCapacity");
            empire_meter_value.name("EmpireMeterValue");
            direct_distance.name("DirectDistanceBetween");
            shortest_path.name("ShortestPathBetween");
            species_empire_opinion.name("SpeciesOpinion (of empire)");
            species_species_opinion.name("SpeciesOpinion (of species)");
            special_capacity.name("SpecialCapacity");

#if DEBUG_DOUBLE_COMPLEX_PARSERS
            debug(name_property_rule);
            debug(empire_meter_value);
            debug(direct_distance);
            debug(shortest_path);
            debug(species_empire_opinion);
            debug(species_species_opinion);
            debug(special_capacity);
#endif
        }

        complex_variable_rule<double> name_property_rule;
        complex_variable_rule<double> empire_meter_value;
        complex_variable_rule<double> direct_distance;
        complex_variable_rule<double> shortest_path;
        complex_variable_rule<double> species_empire_opinion;
        complex_variable_rule<double> species_species_opinion;
        complex_variable_rule<double> special_capacity;
        complex_variable_rule<double> start;
    };

    namespace detail {
        double_complex_parser_rules double_complex_parser;
    }
}

const complex_variable_rule<double>& double_var_complex()
{ return parse::detail::double_complex_parser.start; }
