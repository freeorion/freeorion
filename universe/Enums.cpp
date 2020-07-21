#include "Enums.h"


const std::map<MeterType, MeterType>& AssociatedMeterTypes() {
    static const std::map<MeterType, MeterType> meters = {
        {METER_POPULATION,      METER_TARGET_POPULATION},
        {METER_INDUSTRY,        METER_TARGET_INDUSTRY},
        {METER_RESEARCH,        METER_TARGET_RESEARCH},
        {METER_INFLUENCE,       METER_TARGET_INFLUENCE},
        {METER_CONSTRUCTION,    METER_TARGET_CONSTRUCTION},
        {METER_HAPPINESS,       METER_TARGET_HAPPINESS},
        {METER_FUEL,            METER_MAX_FUEL},
        {METER_SHIELD,          METER_MAX_SHIELD},
        {METER_STRUCTURE,       METER_MAX_STRUCTURE},
        {METER_DEFENSE,         METER_MAX_DEFENSE},
        {METER_TROOPS,          METER_MAX_TROOPS},
        {METER_SUPPLY,          METER_MAX_SUPPLY},
        {METER_STOCKPILE,       METER_MAX_STOCKPILE}};
    return meters;
}

MeterType AssociatedMeterType(MeterType meter_type) {
    auto mt_pair_it = AssociatedMeterTypes().find(meter_type);
    if (mt_pair_it == AssociatedMeterTypes().end())
        return INVALID_METER_TYPE;
    return mt_pair_it->second;
}
