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
