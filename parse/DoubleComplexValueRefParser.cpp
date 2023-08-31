#include "ValueRefParser.h"

#include "MovableEnvelope.h"
#include "../universe/ValueRefs.h"
#include "EnumParser.h"
#include <boost/phoenix.hpp>

namespace parse {

    std::string MeterToNameWrapper(MeterType meter) {
        return std::string{ValueRef::MeterToName(meter)};
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(std::string, MeterToName_, MeterToNameWrapper, 1)

    double_complex_parser_grammar::double_complex_parser_grammar(
        const lexer& tok,
        detail::Labeller& label,
        const detail::condition_parser_grammar& condition_parser,
        const detail::value_ref_grammar<std::string>& string_grammar
    ) :
        double_complex_parser_grammar::base_type(start, "double_complex_parser_grammar"),
        simple_int_rules(tok),
        ship_part_meter_type_enum(tok)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;
        using phoenix::new_;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;
        qi::_val_type _val;
        qi::_pass_type _pass;
        qi::omit_type omit_;

        const boost::phoenix::function<detail::construct_movable> construct_movable_;
        const boost::phoenix::function<detail::deconstruct_movable> deconstruct_movable_;
        const detail::value_ref_rule<int>& simple_int = simple_int_rules.simple;

        name_property_rule
            = ((    tok.GameRule_
                |   tok.HullFuel_
                |   tok.HullStructure_
                |   tok.HullStealth_
                |   tok.HullSpeed_
                |   tok.PartCapacity_
                |   tok.PartSecondaryStat_
               ) >   label(tok.name_) > string_grammar
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                        _1,
                        nullptr,
                        nullptr,
                        nullptr,
                        deconstruct_movable_(_2, _pass),
                        nullptr)) ]
            ;

        id_empire_location_rule
            = (     tok.ShipDesignCost_
                 >  label(tok.design_) > simple_int
                 >  label(tok.empire_) > simple_int
                 >  label(tok.location_) > simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass),
                deconstruct_movable_(_4, _pass),
                nullptr, nullptr)) ]
            ;

        name_empire_location_rule
            = (     tok.BuildingTypeCost_
                 >  label(tok.type_) > string_grammar
                 >  label(tok.empire_) > simple_int
                 >  label(tok.location_) > simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_3, _pass),
                deconstruct_movable_(_4, _pass),
                nullptr,
                deconstruct_movable_(_2, _pass),
                nullptr)) ]
            ;

        empire_meter_value
            = (     tok.EmpireMeterValue_
                 >  label(tok.empire_) > simple_int
                 >  label(tok.meter_) > tok.string
              ) [_val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                nullptr,
                nullptr,
                deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<std::string>>(_3)), _pass),
                nullptr)) ]
            ;

        empire_stockpile
            = ( tok.EmpireStockpile_
            >   label(tok.empire_) > simple_int
            >   label(tok.resource_) > ( tok.Industry_ | tok.Influence_ )
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                nullptr,
                nullptr,
                deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<std::string>>(_3)), _pass),
                nullptr)) ]
            ;

        direct_distance
            = (     tok.DirectDistanceBetween_
                 >  label(tok.object_) > simple_int
                 >  label(tok.object_) > simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass),
                nullptr, nullptr, nullptr)) ]
            ;

        // in shortest_path would have liked to be able to use
        //            >   label(tok.object_) >   (simple_int [ _b = _1 ] | int_rules.statistic_expr [ _b = _1 ])
        //            >   label(tok.object_) >   (simple_int [ _c = _1 ] | int_rules.statistic_expr [ _c = _1 ])
        // but getting crashes upon program start, presumably due to initialization order problems

        shortest_path
            =   (
                        tok.ShortestPath_
                    >   label(tok.object_) > simple_int
                    >   label(tok.object_) > simple_int
                ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass),
                nullptr, nullptr, nullptr)) ]
            ;

        species_content_opinion
            = (     tok.SpeciesContentOpinion_
                >   label(tok.species_) > string_grammar
                >   label(tok.name_) > string_grammar
              )
              [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                nullptr,
                nullptr,
                nullptr,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass))) ]
            ;

        species_empire_opinion
            = (
                (   tok.SpeciesEmpireOpinion_ | tok.SpeciesEmpireTargetOpinion_ )
                >   label(tok.species_) > string_grammar
                >   label(tok.empire_) > simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                deconstruct_movable_(_3, _pass),
                nullptr,
                nullptr,
                deconstruct_movable_(_2, _pass),
                nullptr)) ]
            ;

        species_species_opinion
            = (
                (   tok.SpeciesSpeciesOpinion_ | tok.SpeciesSpeciesTargetOpinion_ )
                >   label(tok.species_) > string_grammar
                >   label(tok.species_) > string_grammar
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,
                nullptr,
                nullptr,
                nullptr,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass))) ]
            ;

        unwrapped_part_meter
            = (     tok.ShipPartMeter_
                >   label(tok.part_)    >   string_grammar
                >   label(tok.meter_)   >   ship_part_meter_type_enum
                >   label(tok.object_)  >   simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                    _1,                                     // variable_name
                    deconstruct_movable_(_4, _pass),        // int_ref1
                    nullptr,                                // int_ref2
                    nullptr,                                // int_ref3
                    deconstruct_movable_(_2, _pass),        // string_ref1
                    deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<std::string>>(MeterToName_(_3))), _pass),   // string_ref2
                    false                                   // return_immediate_value
                  ))]
            ;

        value_wrapped_part_meter
            = (    (omit_[tok.Value_] >> '(')
                >   tok.ShipPartMeter_
                >   label(tok.part_)    >   string_grammar
                >   label(tok.meter_)   >   ship_part_meter_type_enum
                >   label(tok.object_)  >   simple_int
                >>  ')'
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                    _1,                                     // variable_name
                    deconstruct_movable_(_4, _pass),        // int_ref1
                    nullptr,                                // int_ref2
                    nullptr,                                // int_ref3
                    deconstruct_movable_(_2, _pass),        // string_ref1
                    deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<std::string>>(MeterToName_(_3))), _pass),   // string_ref2
                    true                                    // return_immediate_value
                  ))]
            ;

        special_capacity
            = (     tok.SpecialCapacity_
                >   label(tok.name_) > string_grammar
                >>  label(tok.object_)
                >   simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<double>>(
                _1,                                 // variable_name
                deconstruct_movable_(_3, _pass),    // int_ref1
                nullptr,                            // int_ref2
                nullptr,                            // int_ref3
                deconstruct_movable_(_2, _pass),    // string_ref1
                nullptr ))]                         // string_ref2
            ;

        start
            %=  name_property_rule
            |   id_empire_location_rule
            |   name_empire_location_rule
            |   empire_meter_value
            |   empire_stockpile
            |   direct_distance
            |   shortest_path
            |   species_content_opinion
            |   species_empire_opinion
            |   species_species_opinion
            |   unwrapped_part_meter
            |   value_wrapped_part_meter
            |   special_capacity
            ;

        name_property_rule.name("GameRule; Hull Fuel, Stealth, Structure, or Speed; or PartCapacity or PartSecondaryStat");
        id_empire_location_rule.name("ShipDesignCost");
        name_empire_location_rule.name("BuildingTypeCost");
        empire_meter_value.name("EmpireMeterValue");
        empire_stockpile.name("EmpireStockpile");
        direct_distance.name("DirectDistanceBetween");
        shortest_path.name("ShortestPathBetween");
        species_content_opinion.name("SpeciesContentOpinion");
        species_empire_opinion.name("SpeciesEmpireOpinion");
        species_species_opinion.name("SpeciesSpeciesOpinion");
        special_capacity.name("SpecialCapacity");
        unwrapped_part_meter.name("ShipPartMeter");
        value_wrapped_part_meter.name("ShipPartMeter (immediate value)");

#if DEBUG_DOUBLE_COMPLEX_PARSERS
        debug(name_property_rule);
        debug(id_empire_location_rule);
        debug(name_empire_location_rule);
        debug(empire_meter_value);
        debug(empire_stockpile);
        debug(direct_distance);
        debug(shortest_path);
        debug(species_empire_opinion);
        debug(species_species_opinion);
        debug(special_capacity);
        debug(unwrapped_part_meter);
        debug(value_wrapped_part_meter);
#endif
    }
}
