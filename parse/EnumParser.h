#ifndef _EnumParser_h_
#define _EnumParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>


namespace parse {
    template <typename E>
    using enum_rule = detail::rule<
        E ()
    >;

    template <typename T>
    enum_rule<T>& enum_expr()
    {}

    template <>
    enum_rule<PlanetSize>& enum_expr<PlanetSize>();

    template <>
    enum_rule<PlanetType>& enum_expr<PlanetType>();

    template <>
    enum_rule<PlanetEnvironment>& enum_expr<PlanetEnvironment>();

    template <>
    enum_rule<UniverseObjectType>& enum_expr<UniverseObjectType>();

    template <>
    enum_rule<StarType>& enum_expr<StarType>();

    enum_rule<EmpireAffiliationType>& empire_affiliation_type_enum();

    enum_rule<UnlockableItemType>& unlockable_item_type_enum();

    enum_rule<ShipSlotType>& ship_slot_type_enum();

    enum_rule<ShipPartClass>& ship_part_class_enum();

    enum_rule<CaptureResult>& capture_result_enum();

    enum_rule<ValueRef::StatisticType>& statistic_type_enum();

    enum_rule<MeterType>& non_ship_part_meter_type_enum();

    enum_rule<MeterType>& ship_part_meter_type_enum();

    enum_rule<MeterType>& set_non_ship_part_meter_type_enum();

    enum_rule<MeterType>& set_ship_part_meter_type_enum();
}

#endif
