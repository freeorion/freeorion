#include "EnumPythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Enums.h"
#include "../universe/Species.h"
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
}

