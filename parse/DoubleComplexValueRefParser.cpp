#include "ValueRefParser.h"

#include "MovableEnvelope.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace parse {
    double_complex_parser_grammar::double_complex_parser_grammar(
        const lexer& tok,
        detail::Labeller& labeller,
        const detail::condition_parser_grammar& condition_parser,
        const detail::value_ref_grammar<std::string>& string_grammar
    ) :
        double_complex_parser_grammar::base_type(start, "double_complex_parser_grammar"),
        simple_int_rules(tok)
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
        qi::omit_type omit_;
        const boost::phoenix::function<detail::construct_movable> construct_movable_;
        const boost::phoenix::function<detail::deconstruct_movable> deconstruct_movable_;

        const std::string TOK_SPECIES_EMPIRE_OPINION{"SpeciesEmpireOpinion"};
        const std::string TOK_SPECIES_SPECIES_OPINION{"SpeciesSpeciesOpinion"};

        const detail::value_ref_rule<int>& simple_int = simple_int_rules.simple;

        name_property_rule
            = ((    tok.GameRule_
                |   tok.HullFuel_
                |   tok.HullStructure_
                |   tok.HullStealth_
                |   tok.HullSpeed_
                |   tok.PartCapacity_
                |   tok.PartSecondaryStat_
               ) >   labeller.rule(Name_token) > string_grammar
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                        _1,
                        nullptr,
                        nullptr,
                        nullptr,
                        deconstruct_movable_(_2, _pass),
                        nullptr)) ]
            ;

        empire_meter_value
            = (     tok.EmpireMeterValue_
                 >  labeller.rule(Empire_token) > simple_int
                 >  labeller.rule(Meter_token) > tok.string
              ) [_val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                nullptr,
                nullptr,
                deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<std::string>>(_3)), _pass),
                nullptr)) ]
            ;

        direct_distance
            = (     tok.DirectDistanceBetween_
                 >  labeller.rule(Object_token) > simple_int
                 >  labeller.rule(Object_token) > simple_int
              )     [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass),
                nullptr, nullptr, nullptr)) ]
            ;

        // in shortest_path would have liked to be able to use
        //            >   labeller.rule(Object_token) >   (simple_int [ _b = _1 ] | int_rules.statistic_expr [ _b = _1 ])
        //            >   labeller.rule(Object_token) >   (simple_int [ _c = _1 ] | int_rules.statistic_expr [ _c = _1 ])
        // but getting crashes upon program start, presumably due to initialization order problems

        shortest_path
            =   (
                        tok.ShortestPath_
                    >   labeller.rule(Object_token) > simple_int
                    >   labeller.rule(Object_token) > simple_int
                )       [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass),
                nullptr, nullptr, nullptr)) ]
            ;

        species_opinion
            = omit_[tok.SpeciesOpinion_] >  labeller.rule(Species_token) > string_grammar;


        species_empire_opinion
            = ( species_opinion
                >> (labeller.rule(Empire_token) >  simple_int)
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                construct<std::string>(TOK_SPECIES_EMPIRE_OPINION),
                deconstruct_movable_(_2, _pass),
                nullptr,
                nullptr,
                deconstruct_movable_(_1, _pass),
                nullptr)) ]
            ;

        species_species_opinion
            = ( species_opinion
                >> (labeller.rule(Species_token) >  string_grammar)
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                construct<std::string>(TOK_SPECIES_SPECIES_OPINION),
                nullptr,
                nullptr,
                nullptr,
                deconstruct_movable_(_1, _pass),
                deconstruct_movable_(_2, _pass))) ]
            ;

        special_capacity
            = ( tok.SpecialCapacity_
                >  labeller.rule(Name_token) > string_grammar
                >> labeller.rule(Object_token)
                >  simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_3, _pass),
                nullptr,
                nullptr,
                deconstruct_movable_(_2, _pass),
                nullptr ))]
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
