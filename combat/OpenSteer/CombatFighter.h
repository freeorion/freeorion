// -*- C++ -*-
#ifndef _CombatFighter_h_
#define _CombatFighter_h_

#include "PathingEngineFwd.h"

#include "CombatObject.h"
#include "../../universe/ShipDesign.h"
#include "../CombatOrder.h"

#include <boost/enable_shared_from_this.hpp>

#include <list>


class PathingEngine;

class CombatFighterFormation
{
public:
    typedef std::list<CombatFighterPtr>::iterator iterator;
    typedef std::list<CombatFighterPtr>::const_iterator const_iterator;

    ~CombatFighterFormation();

    const CombatFighter& Leader() const;
    OpenSteer::Vec3 Centroid() const;
    bool empty() const;
    std::size_t size() const;
    const_iterator begin() const;
    const_iterator end() const;

    CombatFighter& Leader();
    void SetLeader(const CombatFighterPtr& fighter);
    void push_back(const CombatFighterPtr& fighter);
    void erase(const CombatFighterPtr& fighter);
    void erase(CombatFighter* fighter);
    iterator begin();
    iterator end();

private:
    CombatFighterFormation();
    explicit CombatFighterFormation(PathingEngine& pathing_engine);

    CombatFighterPtr m_leader;
    std::list<CombatFighterPtr> m_members;
    PathingEngine* m_pathing_engine;

    friend class PathingEngine;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        {
            ar  & BOOST_SERIALIZATION_NVP(m_leader)
                & BOOST_SERIALIZATION_NVP(m_members)
                & BOOST_SERIALIZATION_NVP(m_pathing_engine);
        }
};

class CombatFighter :
    public CombatObject,
    public boost::enable_shared_from_this<CombatFighter>
{
public:
    static const int FORMATION_SIZE = 5;

    ~CombatFighter();

    virtual float maxForce() const;
    virtual float maxSpeed() const;
    int ID() const;
    const FighterStats& Stats() const;
    const FighterMission& CurrentMission() const;
    virtual double Health() const;

    virtual void update(const float /*current_time*/, const float elapsed_time);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity, const float elapsedTime);

    CombatFighterFormationPtr Formation();

    void EnterSpace();
    void AppendMission(const FighterMission& mission);
    void ClearMissions();
    void ExitSpace();

    virtual void Damage(double d);

private:
    CombatFighter();
    CombatFighter(CombatObjectPtr base, const FighterStats& stats, int empire_id,
                  int fighter_id, PathingEngine& pathing_engine,
                  const CombatFighterFormationPtr& formation,
                  int formation_position);
    CombatFighter(CombatObjectPtr base, const FighterStats& stats, int empire_id,
                  int fighter_id, PathingEngine& pathing_engine,
                  const CombatFighterFormationPtr& formation);

    void PushMission(const FighterMission& mission);
    OpenSteer::Vec3 GlobalFormationPosition();
    void RemoveMission();
    void UpdateMissionQueue();
    void FireAtHostiles();
    OpenSteer::Vec3 Steer();
    CombatObjectPtr WeakestAttacker(const CombatObjectPtr& attackee);
    CombatShipPtr WeakestHostileShip();

    ProximityDBToken* m_proximity_token;
    bool m_leader;
    FighterStats m_stats;
    int m_empire_id;
    int m_id;
    OpenSteer::Vec3 m_last_steer;

    std::list<FighterMission> m_mission_queue;
    float m_mission_weight;
    OpenSteer::Vec3 m_mission_destination; // Only the X and Y values should be nonzero.
    CombatObjectWeakPtr m_mission_subtarget;
    CombatObjectWeakPtr m_base;

    int m_formation_position;
    CombatFighterFormationPtr m_formation;
    OpenSteer::Vec3 m_out_of_formation;

    double m_health;

    PathingEngine* m_pathing_engine;

    // TODO: Temporary only!
    bool m_instrument;
    FighterMission::Type m_last_mission;

    friend class PathingEngine;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        {
            ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
                & BOOST_SERIALIZATION_NVP(m_proximity_token)
                & BOOST_SERIALIZATION_NVP(m_leader)
                & BOOST_SERIALIZATION_NVP(m_stats)
                & BOOST_SERIALIZATION_NVP(m_empire_id)
                & BOOST_SERIALIZATION_NVP(m_id)
                & BOOST_SERIALIZATION_NVP(m_last_steer)
                & BOOST_SERIALIZATION_NVP(m_mission_queue)
                & BOOST_SERIALIZATION_NVP(m_mission_weight)
                & BOOST_SERIALIZATION_NVP(m_mission_destination)
                & BOOST_SERIALIZATION_NVP(m_mission_subtarget)
                & BOOST_SERIALIZATION_NVP(m_base)
                & BOOST_SERIALIZATION_NVP(m_formation_position)
                & BOOST_SERIALIZATION_NVP(m_formation)
                & BOOST_SERIALIZATION_NVP(m_out_of_formation)
                & BOOST_SERIALIZATION_NVP(m_pathing_engine)
                & BOOST_SERIALIZATION_NVP(m_instrument)
                & BOOST_SERIALIZATION_NVP(m_last_mission);
        }
};

#endif
