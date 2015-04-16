#include "Enums.h"

#include <cassert>


MeterType ResourceToMeter(ResourceType type) {
    switch (type) {
    case RE_INDUSTRY:   return METER_INDUSTRY;
    case RE_RESEARCH:   return METER_RESEARCH;
    case RE_TRADE:      return METER_TRADE;
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
    case METER_TRADE:       return RE_TRADE;
    default:
        assert(0);
        return INVALID_RESOURCE_TYPE;
        break;
    }
}

MeterType AssociatedMeterType(MeterType meter_type) {
    switch (meter_type) {
    case METER_POPULATION:  return METER_TARGET_POPULATION;     break;
    case METER_INDUSTRY:    return METER_TARGET_INDUSTRY;       break;
    case METER_RESEARCH:    return METER_TARGET_RESEARCH;       break;
    case METER_TRADE:       return METER_TARGET_TRADE;          break;
    case METER_CONSTRUCTION:return METER_TARGET_CONSTRUCTION;   break;
    case METER_HAPPINESS:   return METER_TARGET_HAPPINESS;      break;
    case METER_FUEL:        return METER_MAX_FUEL;              break;
    case METER_SHIELD:      return METER_MAX_SHIELD;            break;
    case METER_STRUCTURE:   return METER_MAX_STRUCTURE;         break;
    case METER_DEFENSE:     return METER_MAX_DEFENSE;           break;
    case METER_TROOPS:      return METER_MAX_TROOPS;            break;
    case METER_SUPPLY:      return METER_MAX_SUPPLY;            break;
    default:                return INVALID_METER_TYPE;          break;
    }
}
