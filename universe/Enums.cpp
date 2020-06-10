#include "Enums.h"

#include <cassert>


MeterType ResourceToMeter(ResourceType type) {
    switch (type) {
    case RE_INDUSTRY:   return METER_INDUSTRY;
    case RE_RESEARCH:   return METER_RESEARCH;
    case RE_INFLUENCE:  return METER_INFLUENCE;
    case RE_STOCKPILE:  return METER_STOCKPILE;
    default:
        assert(0);
        return INVALID_METER_TYPE;
        break;
    }
}

MeterType ResourceToTargetMeter(ResourceType type) {
    switch (type) {
    case RE_INDUSTRY:   return METER_TARGET_INDUSTRY;
    case RE_RESEARCH:   return METER_TARGET_RESEARCH;
    case RE_INFLUENCE:  return METER_TARGET_INFLUENCE;
    case RE_STOCKPILE:  return METER_MAX_STOCKPILE;
    default:
        assert(0);
        return INVALID_METER_TYPE;
        break;
    }
}

ResourceType MeterToResource(MeterType type) {
    switch (type) {
    case METER_INDUSTRY:    return RE_INDUSTRY;
    case METER_RESEARCH:    return RE_RESEARCH;
    case METER_INFLUENCE:   return RE_INFLUENCE;
    case METER_STOCKPILE:   return RE_STOCKPILE;
    default:
        assert(0);
        return INVALID_RESOURCE_TYPE;
        break;
    }
}

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
