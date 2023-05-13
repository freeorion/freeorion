#include "Enums.h"


namespace {
    static constexpr std::array<std::pair<MeterType, MeterType>, 13> assoc_meters = {{
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
        {MeterType::METER_STOCKPILE,    MeterType::METER_MAX_STOCKPILE}}};
}

const std::array<std::pair<MeterType, MeterType>, 13>& AssociatedMeterTypes()
{ return assoc_meters; }

MeterType AssociatedMeterType(MeterType meter_type) {
    const auto mt_pair_it = std::find_if(assoc_meters.begin(), assoc_meters.end(),
                                         [&meter_type](const auto& mm) { return meter_type == mm.first; });
    return (mt_pair_it != assoc_meters.end()) ? mt_pair_it->second : MeterType::INVALID_METER_TYPE;
}
