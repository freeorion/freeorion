// -*- C++ -*-
#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include <string>
#include <vector>

class Empire;
class Fleet;
class Planet;

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

