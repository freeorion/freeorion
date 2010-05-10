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