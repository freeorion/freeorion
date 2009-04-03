// -*- C++ -*-
#ifndef _CombatEventListener_h_
#define _CombatEventListener_h_

#include "OpenSteer/PathingEngineFwd.h"


class CombatEventListener
{
public:
    virtual void ShipPlaced(const CombatShipPtr &ship) = 0;
    virtual void ShipFired(const CombatShipPtr &ship,
                           const CombatObjectPtr &target,
                           const std::string& part_name) = 0;
    virtual void ShipDestroyed(const CombatShipPtr &ship) = 0;
    virtual void ShipEnteredStarlane(const CombatShipPtr &ship) = 0;

    virtual void FighterLaunched(const CombatFighterPtr &fighter) = 0;
    virtual void FighterFired(const CombatFighterPtr &fighter,
                              const CombatObjectPtr &target) = 0;
    virtual void FighterDestroyed(const CombatFighterPtr &fighter) = 0;
    virtual void FighterDocked(const CombatFighterPtr &fighter) = 0;

    virtual void MissileLaunched(const MissilePtr &missile) = 0;
    virtual void MissileExploded(const MissilePtr &missile) = 0;
    virtual void MissileRemoved(const MissilePtr &missile) = 0;
};

#endif
