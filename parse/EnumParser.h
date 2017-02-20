#ifndef _EnumParser_h_
#define _EnumParser_h_

#include "Lexer.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

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

    enum_parser_rule<PlanetSize>::type& planet_size_enum();

    enum_parser_rule<PlanetType>::type& planet_type_enum();

    enum_parser_rule<PlanetEnvironment>::type& planet_environment_enum();

    enum_parser_rule<UniverseObjectType>::type& universe_object_type_enum();

    enum_parser_rule<StarType>::type& star_type_enum();

    enum_parser_rule<EmpireAffiliationType>::type& empire_affiliation_type_enum();

    enum_parser_rule<UnlockableItemType>::type& unlockable_item_type_enum();

    enum_parser_rule<ShipSlotType>::type& ship_slot_type_enum();

    enum_parser_rule<ShipPartClass>::type& ship_part_class_enum();

    enum_parser_rule<CaptureResult>::type& capture_result_enum();

    enum_parser_rule<ValueRef::StatisticType>::type& statistic_type_enum();

    enum_parser_rule<MeterType>::type& non_ship_part_meter_type_enum();

    enum_parser_rule<MeterType>::type& ship_part_meter_type_enum();

    enum_parser_rule<MeterType>::type& set_non_ship_part_meter_type_enum();

    enum_parser_rule<MeterType>::type& set_ship_part_meter_type_enum();
}

#endif
