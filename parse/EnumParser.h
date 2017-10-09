#ifndef _EnumParser_h_
#define _EnumParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>


namespace parse {
    detail::enum_rule<EmpireAffiliationType>& empire_affiliation_type_enum();

    detail::enum_rule<UnlockableItemType>& unlockable_item_type_enum();

    detail::enum_rule<ShipSlotType>& ship_slot_type_enum();

    detail::enum_rule<ShipPartClass>& ship_part_class_enum();

    detail::enum_rule<CaptureResult>& capture_result_enum();

    detail::enum_rule<ValueRef::StatisticType>& statistic_type_enum();

    detail::enum_rule<MeterType>& non_ship_part_meter_type_enum();

    detail::enum_rule<MeterType>& ship_part_meter_type_enum();

    detail::enum_rule<MeterType>& set_non_ship_part_meter_type_enum();

    detail::enum_rule<MeterType>& set_ship_part_meter_type_enum();
}

#endif
