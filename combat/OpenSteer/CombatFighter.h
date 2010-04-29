// -*- C++ -*-
#ifndef _CombatFighter_h_
#define _CombatFighter_h_

#include "PathingEngineFwd.h"

#include "CombatObject.h"
#include "../../universe/ShipDesign.h"
#include "../CombatOrder.h"

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
    double Damage(double d);
    void push_back(const CombatFighterPtr& fighter);
    void erase(const CombatFighterPtr& fighter);
    void erase(CombatFighter* fighter);
    iterator begin();
    iterator end();

private:
    CombatFighterFormation();
    explicit CombatFighterFormation(PathingEngine& pathing_engine);
    void SetLeader(const CombatFighterPtr& fighter);

    CombatFighterPtr m_leader;
    std::list<CombatFighterPtr> m_members;
    PathingEngine* m_pathing_engine;

    friend class PathingEngine;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class CombatFighter :
    public CombatObject
{
public:
    static const std::size_t FORMATION_SIZE;

    CombatFighter(CombatObjectPtr base, const PartType& part, int empire_id,
                  PathingEngine& pathing_engine);
    ~CombatFighter();

    virtual float maxForce() const;
    virtual float maxSpeed() const;
    int ID() const;
    bool IsLeader() const;
    const FighterStats& Stats() const;
    const std::string& PartName() const;
    const FighterMission& CurrentMission() const;
    virtual double HealthAndShield() const;
    virtual double Health() const;
    virtual double FractionalHealth() const;
    virtual double AntiFighterStrength() const;
    virtual double AntiShipStrength(CombatShipPtr target = CombatShipPtr()) const;
    virtual bool IsFighter() const;
    virtual bool IsShip() const;
    virtual int Owner() const;

    virtual void update(const float elapsed_time, bool force);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity,
                                      const float elapsedTime);

    CombatFighterFormationPtr Formation();

    void EnterSpace();
    void AppendMission(const FighterMission& mission);
    void ClearMissions();
    void ExitSpace();

    virtual void Damage(double d, DamageSource source);
    virtual void Damage(const CombatFighterPtr& source);
    virtual void TurnStarted(unsigned int number);
    virtual void SignalDestroyed();

    void SetWeakPtr(const CombatFighterPtr& ptr);
    CombatFighterPtr shared_from_this();

private:
    CombatFighter();
    CombatFighter(CombatObjectPtr base, int empire_id, PathingEngine& pathing_engine);

    void Init(const PartType& part);
    void DamageImpl(double d);
    void SetFormation(const CombatFighterFormationPtr& formation);
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
    std::string m_part_name;
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

    unsigned int m_last_queue_update_turn;
    unsigned int m_last_fired_turn;
    unsigned int m_turn;

    FighterStats m_stats;

    PathingEngine* m_pathing_engine;

    // TODO: Temporary only!
    bool m_instrument;
    FighterMission::Type m_last_mission;

    boost::weak_ptr<CombatFighter> m_weak_ptr;

    friend class PathingEngine;
    friend double CombatFighterFormation::Damage(double);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif
