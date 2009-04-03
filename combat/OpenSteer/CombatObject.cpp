#include "CombatObject.h"

#include "../CombatEventListener.h"


namespace {
    class NullListener :
        public CombatEventListener
    {
        virtual void ShipPlaced(const CombatShipPtr &ship) {}
        virtual void ShipFired(const CombatShipPtr &ship,
                               const CombatObjectPtr &target,
                               const std::string& part_name) {}
        virtual void ShipDestroyed(const CombatShipPtr &ship) {}
        virtual void ShipEnteredStarlane(const CombatShipPtr &ship) {}

        virtual void FighterLaunched(const CombatFighterPtr &fighter) {}
        virtual void FighterFired(const CombatFighterPtr &fighter,
                                  const CombatObjectPtr &target) {}
        virtual void FighterDestroyed(const CombatFighterPtr &fighter) {}
        virtual void FighterDocked(const CombatFighterPtr &fighter) {}

        virtual void MissileLaunched(const MissilePtr &missile) {}
        virtual void MissileExploded(const MissilePtr &missile) {}
        virtual void MissileRemoved(const MissilePtr &missile) {}
    } g_null_listener;
}

CombatObject::CombatObject() :
    m_listener(0)
{}

void CombatObject::SetListener(CombatEventListener& listener)
{ m_listener = &listener; }

CombatEventListener& CombatObject::Listener()
{ return m_listener ? *m_listener : g_null_listener; }
