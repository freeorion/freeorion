#include "Enums.h"

#include <cassert>

namespace {
    const std::string EMPTY_STRING = "";
    const std::string PATH_BINARY_STR = "PATH_BINARY";
    const std::string PATH_RESOURCE_STR = "PATH_RESOURCE";
    const std::string PATH_DATA_ROOT_STR = "PATH_DATA_ROOT";
    const std::string PATH_DATA_USER_STR = "PATH_DATA_USER";
    const std::string PATH_CONFIG_STR = "PATH_CONFIG";
    const std::string PATH_SAVE_STR = "PATH_SAVE";
    const std::string PATH_TEMP_STR = "PATH_TEMP";
    const std::string PATH_PYTHON_STR = "PATH_PYTHON";
    const std::string PATH_INVALID_STR = "PATH_INVALID";
}

MeterType ResourceToMeter(ResourceType type) {
    switch (type) {
    case RE_INDUSTRY:   return METER_INDUSTRY;
    case RE_RESEARCH:   return METER_RESEARCH;
    case RE_TRADE:      return METER_TRADE;
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
    case RE_TRADE:      return METER_TARGET_TRADE;
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
    case METER_TRADE:       return RE_TRADE;
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
        {METER_TRADE,           METER_TARGET_TRADE},
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

const std::string& PathTypeToString(PathType path_type) {
    switch (path_type) {
        case PATH_BINARY:       return PATH_BINARY_STR;
        case PATH_RESOURCE:     return PATH_RESOURCE_STR;
        case PATH_PYTHON:       return PATH_PYTHON_STR;
        case PATH_DATA_ROOT:    return PATH_DATA_ROOT_STR;
        case PATH_DATA_USER:    return PATH_DATA_USER_STR;
        case PATH_CONFIG:       return PATH_CONFIG_STR;
        case PATH_SAVE:         return PATH_SAVE_STR;
        case PATH_TEMP:         return PATH_TEMP_STR;
        case PATH_INVALID:      return PATH_INVALID_STR;
        default:                return EMPTY_STRING;
    }
}

const std::vector<std::string>& PathTypeStrings() {
    static std::vector<std::string> path_type_list;
    if (path_type_list.empty()) {
        for (auto path_type = PathType(0); path_type < PATH_INVALID; path_type = PathType(path_type + 1)) {
            // PATH_PYTHON is only valid for FREEORION_WIN32 or FREEORION_MACOSX
#if defined(FREEORION_LINUX)
            if (path_type == PATH_PYTHON)
                continue;
#endif
            path_type_list.push_back(PathTypeToString(path_type));
        }
    }
    return path_type_list;
}
