// -*- C++ -*-
#ifndef _EnumParser_h_
#define _EnumParser_h_

#include "Lexer.h"

#include <boost/spirit/include/qi.hpp>


namespace parse {

    template <typename E>
    struct enum_parser_rule
    {
        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            E (),
            parse::skipper_type
        > type;
    };

    /** Returns a reference to the parser for the enum \a E. */
    template <typename E>
    typename enum_parser_rule<E>::type& enum_parser();

    template <>
    enum_parser_rule<PlanetSize>::type& enum_parser<PlanetSize>();

    template <>
    enum_parser_rule<PlanetType>::type& enum_parser<PlanetType>();

    template <>
    enum_parser_rule<PlanetEnvironment>::type& enum_parser<PlanetEnvironment>();

    template <>
    enum_parser_rule<UniverseObjectType>::type& enum_parser<UniverseObjectType>();

    template <>
    enum_parser_rule<StarType>::type& enum_parser<StarType>();

    template <>
    enum_parser_rule<EmpireAffiliationType>::type& enum_parser<EmpireAffiliationType>();

    template <>
    enum_parser_rule<UnlockableItemType>::type& enum_parser<UnlockableItemType>();

    template <>
    enum_parser_rule<TechType>::type& enum_parser<TechType>();

    template <>
    enum_parser_rule<ShipSlotType>::type& enum_parser<ShipSlotType>();

    template <>
    enum_parser_rule<ShipPartClass>::type& enum_parser<ShipPartClass>();

    template <>
    enum_parser_rule<CombatFighterType>::type& enum_parser<CombatFighterType>();

    template <>
    enum_parser_rule<CaptureResult>::type& enum_parser<CaptureResult>();

    template <>
    enum_parser_rule<ValueRef::StatisticType>::type& enum_parser<ValueRef::StatisticType>();

    enum_parser_rule<MeterType>::type& non_ship_part_meter_type_enum();

    enum_parser_rule<MeterType>::type& ship_part_meter_type_enum();

    enum_parser_rule<MeterType>::type& set_non_ship_part_meter_type_enum();

    enum_parser_rule<MeterType>::type& set_ship_part_meter_type_enum();

}

#endif
