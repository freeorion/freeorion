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

    enum_rule<PlanetSize>& planet_size_enum();

    enum_rule<PlanetType>& planet_type_enum();

    enum_rule<PlanetEnvironment>& planet_environment_enum();

    enum_rule<UniverseObjectType>& universe_object_type_enum();

    enum_rule<StarType>& star_type_enum();

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
