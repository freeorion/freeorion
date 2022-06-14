#include "EnumPythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Enums.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/ValueRef.h"
#include "../Empire/ResourcePool.h"

void RegisterGlobalsEnums(boost::python::dict& globals) {
    globals["EnemyOf"] = enum_wrapper<EmpireAffiliationType>(EmpireAffiliationType::AFFIL_ENEMY);
    globals["Influence"] = enum_wrapper<ResourceType>(ResourceType::RE_INFLUENCE);

    for (const auto& op : std::initializer_list<std::pair<const char*, ::PlanetEnvironment>>{
            {"Uninhabitable", PlanetEnvironment::PE_UNINHABITABLE},
            {"Hostile",       PlanetEnvironment::PE_HOSTILE},
            {"Poor",          PlanetEnvironment::PE_POOR},
            {"Adequate",      PlanetEnvironment::PE_ADEQUATE},
            {"Good",          PlanetEnvironment::PE_GOOD}})
    {
        globals[op.first] = enum_wrapper< ::PlanetEnvironment>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, ValueRef::StatisticType>>{
            {"CountUnique",     ValueRef::StatisticType::UNIQUE_COUNT},
            {"If",              ValueRef::StatisticType::IF},
            {"Count",           ValueRef::StatisticType::COUNT},
            {"HistogramMax",    ValueRef::StatisticType::HISTO_MAX},
            {"HistogramMin",    ValueRef::StatisticType::HISTO_MIN},
            {"HistogramSpread", ValueRef::StatisticType::HISTO_SPREAD},
            {"Sum",             ValueRef::StatisticType::SUM},
            {"Mean",            ValueRef::StatisticType::MEAN},
            {"RMS",             ValueRef::StatisticType::RMS},
            {"Mode",            ValueRef::StatisticType::MODE},
            {"Max",             ValueRef::StatisticType::MAX},
            {"Min",             ValueRef::StatisticType::MIN},
            {"Spread",          ValueRef::StatisticType::SPREAD},
            {"StDev",           ValueRef::StatisticType::STDEV},
            {"Product",         ValueRef::StatisticType::PRODUCT}})
    {
        globals[op.first] = enum_wrapper<ValueRef::StatisticType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, ::StarType>>{
            {"Blue",      StarType::STAR_BLUE},
            {"White",     StarType::STAR_WHITE},
            {"Yellow",    StarType::STAR_YELLOW},
            {"Orange",    StarType::STAR_ORANGE},
            {"Red",       StarType::STAR_RED},
            {"Neutron",   StarType::STAR_NEUTRON},
            {"BlackHole", StarType::STAR_BLACK},
            {"NoStar",    StarType::STAR_NONE}})
    {
        globals[op.first] = enum_wrapper< ::StarType>(op.second);
    }
}

