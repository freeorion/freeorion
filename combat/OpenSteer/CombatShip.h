// -*- C++ -*-
#ifndef _CombatShip_h_
#define _CombatShip_h_

#include "PathingEngineFwd.h"

#include "CombatObject.h"
#include "../CombatOrder.h"

#include <boost/enable_shared_from_this.hpp>

#include <list>
#include <set>


class Ship;

class CombatShip :
    public CombatObject,
    public boost::enable_shared_from_this<CombatShip>
{
public:
    CombatShip(int empire_id, Ship* ship, const OpenSteer::Vec3& position,
               const OpenSteer::Vec3& direction, PathingEngine& pathing_engine);
    ~CombatShip();

    float AntiFighterStrength() const;
    Ship* GetShip() const;
    const ShipMission& CurrentMission() const;
    virtual double Health() const;

    void LaunchFighters();
    void RecoverFighters(const CombatFighterFormationPtr& formation);
    void AppendMission(const ShipMission& mission);
    void ClearMissions();
    void AppendFighterMission(const FighterMission& mission);
    void ClearFighterMissions();

    virtual void update(const float /*current_time*/, const float elapsed_time);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity, const float elapsedTime);

    virtual void Damage(double d);

private:
    CombatShip();

    float MaxWeaponRange() const;
    float MinNonPDWeaponRange() const;

    void Init(const OpenSteer::Vec3& position_, const OpenSteer::Vec3& direction);
    void PushMission(const ShipMission& mission);
    void RemoveMission();
    void UpdateMissionQueue();
    void FireAtHostiles();
    OpenSteer::Vec3 Steer();
    CombatObjectPtr WeakestAttacker(const CombatObjectPtr& attackee);
    CombatShipPtr WeakestHostileShip();

    ProximityDBToken* m_proximity_token;
    int m_empire_id;
    Ship* m_ship;
    OpenSteer::Vec3 m_last_steer;

    std::list<ShipMission> m_mission_queue;
    float m_mission_weight;
    OpenSteer::Vec3 m_mission_destination; // Only the X and Y values should be nonzero.
    CombatObjectWeakPtr m_mission_subtarget;

    PathingEngine* m_pathing_engine;

    // TODO: This should be computed from the ship design in the final version
    // of this class.
    float m_anti_fighter_strength;

    std::set<CombatFighterFormationPtr> m_formations;
    std::set<CombatFighterFormationPtr> m_unlaunched_formations;
    std::set<CombatFighterFormationPtr> m_launched_formations;

    // TODO: Temporary only!
    bool m_instrument;
    ShipMission::Type m_last_mission;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        {
            ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
                & BOOST_SERIALIZATION_NVP(m_proximity_token)
                & BOOST_SERIALIZATION_NVP(m_empire_id)
                & BOOST_SERIALIZATION_NVP(m_ship)
                & BOOST_SERIALIZATION_NVP(m_last_steer)
                & BOOST_SERIALIZATION_NVP(m_mission_queue)
                & BOOST_SERIALIZATION_NVP(m_mission_weight)
                & BOOST_SERIALIZATION_NVP(m_mission_destination)
                & BOOST_SERIALIZATION_NVP(m_mission_subtarget)
                & BOOST_SERIALIZATION_NVP(m_pathing_engine)
                & BOOST_SERIALIZATION_NVP(m_anti_fighter_strength)
                & BOOST_SERIALIZATION_NVP(m_formations)
                & BOOST_SERIALIZATION_NVP(m_unlaunched_formations)
                & BOOST_SERIALIZATION_NVP(m_launched_formations)
                & BOOST_SERIALIZATION_NVP(m_instrument)
                & BOOST_SERIALIZATION_NVP(m_last_mission);
        }
};

#endif
