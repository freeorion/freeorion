// -*- C++ -*-
#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include "../universe/Universe.h"

/** Contains information about the state of a combat before or after the combat
  * occurs. */
struct CombatInfo
{
    CombatInfo();
    ~CombatInfo();

    int                         system_id;              ///< ID of system where combat is occurring (could be INVALID_OBJECT_ID ?)
    std::set<int>               empire_ids;             ///< IDs of empires involved in combat
    ObjectMap                   objects;                ///< actual state of objects relevant to combat
    std::map<int, ObjectMap>    empire_known_objects;   ///< each empire's latest known state of objects relevant to combat 
};

/** Resolves a battle. */
void ResolveCombat(CombatInfo& combat_info);

#endif // _CombatSystem_h_

