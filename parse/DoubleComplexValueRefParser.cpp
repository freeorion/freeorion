#include "ValueRefParserImpl.h"


namespace parse {
    double_complex_parser_grammar::double_complex_parser_grammar(
        const parse::lexer& tok,
        parse::detail::Labeller& labeller,
        const detail::condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        double_complex_parser_grammar::base_type(start, "double_complex_parser_grammar"),
        simple_int_rules(tok)
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

        static const std::string TOK_SPECIES_EMPIRE_OPINION{"SpeciesEmpireOpinion"};
        static const std::string TOK_SPECIES_SPECIES_OPINION{"SpeciesSpeciesOpinion"};

        const parse::value_ref_rule<int>& simple_int = simple_int_rules.simple;

        name_property_rule
            = (     tok.GameRule_           [ _a = construct<std::string>(_1) ]
                |   tok.HullFuel_           [ _a = construct<std::string>(_1) ]
                |   tok.HullStructure_      [ _a = construct<std::string>(_1) ]
                |   tok.HullStealth_        [ _a = construct<std::string>(_1) ]
                |   tok.HullSpeed_          [ _a = construct<std::string>(_1) ]
                |   tok.PartCapacity_       [ _a = construct<std::string>(_1) ]
                |   tok.PartSecondaryStat_  [ _a = construct<std::string>(_1) ]
              ) >   labeller.rule(Name_token) > string_grammar [ _d = _1 ]
                [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
            ;

        empire_meter_value
            = (     tok.EmpireMeterValue_ [ _a = construct<std::string>(_1)]
                 >  labeller.rule(Empire_token) > simple_int[ _b = _1]
                 >  labeller.rule(Meter_token) > tok.string[ _d = new_<ValueRef::Constant<std::string>>(_1)]
              )     [_val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e)]
            ;

        direct_distance
            = (     tok.DirectDistanceBetween_ [ _a = construct<std::string>(_1) ]
                 >  labeller.rule(Object_token) > simple_int [ _b = _1 ]
                 >  labeller.rule(Object_token) > simple_int [ _c = _1 ]
              )     [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
            ;

        // in shortest_path would have liked to be able to use
        //            >   labeller.rule(Object_token) >   (simple_int [ _b = _1 ] | int_rules.statistic_expr [ _b = _1 ])
        //            >   labeller.rule(Object_token) >   (simple_int [ _c = _1 ] | int_rules.statistic_expr [ _c = _1 ])
        // but getting crashes upon program start, presumably due to initialization order problems

        shortest_path
            =   (
                        tok.ShortestPath_ [ _a = construct<std::string>(_1) ]
                    >   labeller.rule(Object_token) > simple_int [ _b = _1 ]
                    >   labeller.rule(Object_token) > simple_int [ _c = _1 ]
                )       [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
            ;

        species_empire_opinion
            = (
                ((  tok.SpeciesOpinion_ [ _a = construct<std::string>(TOK_SPECIES_EMPIRE_OPINION) ]
                   >  labeller.rule(Species_token) > string_grammar [ _d = _1 ]
                 )
                >> labeller.rule(Empire_token)
                )  >  simple_int [ _b = _1 ]
              )     [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
            ;

        species_species_opinion
            = (
                ((   tok.SpeciesOpinion_ [ _a = construct<std::string>(TOK_SPECIES_SPECIES_OPINION) ]
                  >  labeller.rule(Species_token) >  string_grammar [ _d = _1 ]
                 )
                >> labeller.rule(Species_token)
                ) >  string_grammar [ _e = _1 ]
              )     [ _val = new_<ValueRef::ComplexVariable<double>>(_a, _b, _c, _f, _d, _e) ]
            ;

        special_capacity
            = (
                ((  tok.SpecialCapacity_ [ _a = construct<std::string>(_1) ]
                   >  labeller.rule(Name_token) > string_grammar [ _d = _1 ]
                 )
                >> labeller.rule(Object_token)
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
}
