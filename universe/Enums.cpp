#include "Enums.h"

#include <cassert>

MeterType FocusToMeter(FocusType type)
{
    switch (type) {
    case FOCUS_FARMING:     return METER_FARMING;
    case FOCUS_INDUSTRY:    return METER_INDUSTRY;
    case FOCUS_MINING:      return METER_MINING;
    case FOCUS_RESEARCH:    return METER_RESEARCH;
    case FOCUS_TRADE:       return METER_TRADE;
    default:
        assert(0);
        return INVALID_METER_TYPE;
        break;
    }
}


FocusType MeterToFocus(MeterType type)
{
    switch (type) {
    case METER_FARMING:     return FOCUS_FARMING;
    case METER_INDUSTRY:    return FOCUS_INDUSTRY;
    case METER_MINING:      return FOCUS_MINING;
    case METER_RESEARCH:    return FOCUS_RESEARCH;
    case METER_TRADE:       return FOCUS_TRADE;
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
