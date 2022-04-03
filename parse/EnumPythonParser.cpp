#include "EnumPythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Enums.h"
#include "../Empire/ResourcePool.h"

void RegisterGlobalsEnums(boost::python::dict& globals) {
    globals["EnemyOf"] = enum_wrapper<EmpireAffiliationType>(EmpireAffiliationType::AFFIL_ENEMY);
    globals["Influence"] = enum_wrapper<ResourceType>(ResourceType::RE_INFLUENCE);
}

