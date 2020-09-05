#include "Enums.h"


const std::map<MeterType, MeterType>& AssociatedMeterTypes() {
    static const std::map<MeterType, MeterType> meters = {
        {MeterType::METER_POPULATION,   MeterType::METER_TARGET_POPULATION},
        {MeterType::METER_INDUSTRY,     MeterType::METER_TARGET_INDUSTRY},
        {MeterType::METER_RESEARCH,     MeterType::METER_TARGET_RESEARCH},
        {MeterType::METER_INFLUENCE,    MeterType::METER_TARGET_INFLUENCE},
        {MeterType::METER_CONSTRUCTION, MeterType::METER_TARGET_CONSTRUCTION},
        {MeterType::METER_HAPPINESS,    MeterType::METER_TARGET_HAPPINESS},
        {MeterType::METER_FUEL,         MeterType::METER_MAX_FUEL},
        {MeterType::METER_SHIELD,       MeterType::METER_MAX_SHIELD},
        {MeterType::METER_STRUCTURE,    MeterType::METER_MAX_STRUCTURE},
        {MeterType::METER_DEFENSE,      MeterType::METER_MAX_DEFENSE},
        {MeterType::METER_TROOPS,       MeterType::METER_MAX_TROOPS},
        {MeterType::METER_SUPPLY,       MeterType::METER_MAX_SUPPLY},
        {MeterType::METER_STOCKPILE,    MeterType::METER_MAX_STOCKPILE}};
    return meters;
}

MeterType AssociatedMeterType(MeterType meter_type) {
    auto mt_pair_it = AssociatedMeterTypes().find(meter_type);
    if (mt_pair_it == AssociatedMeterTypes().end())
        return MeterType::INVALID_METER_TYPE;
    return mt_pair_it->second;
}
