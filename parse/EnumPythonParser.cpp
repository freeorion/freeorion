#include "EnumPythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Enums.h"

void RegisterGlobalsEnums(boost::python::dict& globals) {
    globals["EnemyOf"] = enum_wrapper<EmpireAffiliationType>(EmpireAffiliationType::AFFIL_ENEMY);
}

