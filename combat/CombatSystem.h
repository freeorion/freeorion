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


struct CombatAssetsOwner
{
    CombatAssetsOwner(Empire* o) : owner(o) {}
    bool operator==(const CombatAssetsOwner &ca) const;
    Empire* owner;
};

struct CombatAssets : public CombatAssetsOwner
{
    CombatAssets(Empire* o) : CombatAssetsOwner(o) {}
    std::vector<Fleet*>     fleets;
    std::vector<Planet*>    planets;
};

class CombatSystem
{
public:
    /** Resolves a battle in the given system, taking into account
      * fleets and defensive bases.  Ships and fleets will either
      * be destroyed, or will retreat, as per the v0.2 requirements doc.
      * Systems may change hands as well.  */
    void ResolveCombat(int system_id, const std::vector<CombatAssets> &assets);
};

#endif // _CombatSystem_h_

