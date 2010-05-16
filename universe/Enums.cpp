#include "Enums.h"

#include <cassert>


FocusType MeterToFocus(MeterType type)
{
    switch (type) {
    case METER_FARMING:
    case METER_TARGET_FARMING:
        return FOCUS_FARMING;
        break;
    case METER_INDUSTRY:
    case METER_TARGET_INDUSTRY:
        return FOCUS_INDUSTRY;
        break;
    case METER_MINING:
    case METER_TARGET_MINING:
        return FOCUS_MINING;
        break;
    case METER_RESEARCH:
    case METER_TARGET_RESEARCH:
        return FOCUS_RESEARCH;
        break;
    case METER_TRADE:
    case METER_TARGET_TRADE:
        return FOCUS_TRADE;
        break;
    default:
        assert(0);
        return INVALID_FOCUS_TYPE;
        break;
    }
}

MeterType ResourceToMeter(ResourceType type)
{
    switch (type) {
    case RE_FOOD:       return METER_FARMING;
    case RE_INDUSTRY:   return METER_INDUSTRY;
    case RE_MINERALS:   return METER_MINING;
    case RE_RESEARCH:   return METER_RESEARCH;
    case RE_TRADE:      return METER_TRADE;
    default:
        assert(0);
        return INVALID_METER_TYPE;
        break;
    }
}

ResourceType MeterToResource(MeterType type)
{
    switch (type) {
    case METER_FARMING:     return RE_FOOD;
    case METER_INDUSTRY:    return RE_INDUSTRY;
    case METER_MINING:      return RE_MINERALS;
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
    case METER_HEALTH:      return METER_TARGET_HEALTH;         break;
    case METER_FARMING:     return METER_TARGET_FARMING;        break;
    case METER_INDUSTRY:    return METER_TARGET_INDUSTRY;       break;
    case METER_RESEARCH:    return METER_TARGET_RESEARCH;       break;
    case METER_TRADE:       return METER_TARGET_TRADE;          break;
    case METER_MINING:      return METER_TARGET_MINING;         break;
    case METER_CONSTRUCTION:return METER_TARGET_CONSTRUCTION;   break;
    case METER_FUEL:        return METER_MAX_FUEL;              break;
    case METER_SHIELD:      return METER_MAX_SHIELD;            break;
    case METER_STRUCTURE:   return METER_MAX_STRUCTURE;         break;
    case METER_DEFENSE:     return METER_MAX_DEFENSE;           break;
    default:                return INVALID_METER_TYPE;          break;
    }
}