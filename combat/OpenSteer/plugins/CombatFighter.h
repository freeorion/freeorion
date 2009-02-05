// -*- C++ -*-
#ifndef COMBAT_FIGHTER_H
#define COMBAT_FIGHTER_H

#include "PathingEngineFwd.h"

#include "OpenSteer/SimpleVehicle.h"

#include <boost/enable_shared_from_this.hpp>

#include <list>


class PathingEngine;

enum CombatFighterType {
    /** A fighter that is better at attacking other fighters than at
        attacking ships. */
    INTERCEPTOR,

    /** A fighter that is better at attacking ships than at attacking
        other fighters. */
    BOMBER
};

class CombatFighterFormation
{
public:
    typedef std::list<CombatFighterPtr>::const_iterator const_iterator;

    explicit CombatFighterFormation(PathingEngine& pathing_engine);
    ~CombatFighterFormation();

    const CombatFighter& Leader() const;
    OpenSteer::Vec3 Centroid() const;
    bool empty() const;
    std::size_t size() const;
    const_iterator begin() const;
    const_iterator end() const;

    void SetLeader(const CombatFighterPtr& fighter);
    void push_back(const CombatFighterPtr& fighter);
    void erase(const CombatFighterPtr& fighter);
    void erase(CombatFighter* fighter);

private:
    CombatFighterPtr m_leader;
    std::list<CombatFighterPtr> m_members;
    PathingEngine& m_pathing_engine;
};

class CombatFighter :
    public OpenSteer::SimpleVehicle,
    public boost::enable_shared_from_this<CombatFighter>
{
public:
    static const int FORMATION_SIZE = 5;

    /** The missions available to fighters.  The notion of "weakest" is
        intentionally left fuzzy.  The weakest target is one that is likely to
        die quickly, and also one that is unlikely to kill the fighter.  Some
        relative of scaling these two factors should be employed to come up with
        a single weakness value. */
    enum MissionType {
        /** No mission set.  This mission is treated as
            ATTACK_FIGHTERS_BOMBERS_FIRST by interceptors, and
            ATTACK_SHIPS_WEAKEST_FIRST by bombers. */
        NONE,

        /** Moves to a speficic location.  Enemy presence will not affect
            movement. */
        MOVE_TO,

        /** Attack a specific fighter group or vessel. */
        ATTACK_THIS,

        /** Defend a specific fighter group or vessel, by attacking its
            attackers in order from weakest to strongest. */
        DEFEND_THIS,

        /** Moves to a specific location.  Movement will be interrupted to
            engage targets as soon as they are found.  Interceptors will
            engage enemy fighters, and bombers will engage enemy ships. */
        PATROL_TO,

        /** Attack fighters without further direction, in order of nearest
            to farthest bombers, then nearest to farthest interceptors. */
        ATTACK_FIGHTERS_BOMBERS_FIRST,

        /** Attack fighters without further direction, in order of nearest
            to farthest interceptors, then nearest to farthest bombers. */
        ATTACK_FIGHTERS_INTERCEPTORS_FIRST,

        /** Attack ships without further direction, in order of weakest
            to strongest. */
        ATTACK_SHIPS_WEAKEST_FIRST,

        /** Attack ships without further direction, in order of nearest
            to farthest. */
        ATTACK_SHIPS_NEAREST_FIRST,

        /** Return to the fighter's base/carrier.  Enemy presence will not
            affect movement. */
        RETURN_TO_BASE
    };

    struct Mission
    {
        explicit Mission(MissionType type);
        Mission(MissionType type, const OpenSteer::Vec3& destination);
        Mission(MissionType type, const CombatObjectPtr& target);

        MissionType m_type;
        OpenSteer::Vec3 m_destination;
        CombatObjectWeakPtr m_target;
    };

    CombatFighter(CombatObjectPtr base, CombatFighterType type, int empire_id,
                  PathingEngine& pathing_engine, const CombatFighterFormationPtr& formation,
                  int formation_position);
    CombatFighter(CombatObjectPtr base, CombatFighterType type, int empire_id,
                  PathingEngine& pathing_engine, const CombatFighterFormationPtr& formation);
    ~CombatFighter();

    virtual float maxForce() const;
    virtual float maxSpeed() const;
    const Mission& CurrentMission() const;

    virtual void update(const float /*current_time*/, const float elapsed_time);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity, const float elapsedTime);

    // TODO: for testing only
    void Draw();

    CombatFighterFormationPtr Formation();

    void AppendMission(const Mission& mission);
    void ClearMissions();

private:
    void Init();
    OpenSteer::Vec3 GlobalFormationPosition();
    void RemoveMission();
    void UpdateMissionQueue();
    OpenSteer::Vec3 Steer();
    CombatObjectPtr WeakestAttacker(const CombatObjectPtr& attackee);
    CombatShipPtr WeakestHostileShip();

    ProximityDBToken* m_proximity_token;
    bool m_leader;
    CombatFighterType m_type;
    int m_empire_id;
    OpenSteer::Vec3 m_last_steer;

    std::list<Mission> m_mission_queue;
    float m_mission_weight;
    OpenSteer::Vec3 m_mission_destination; // Only the X and Y values should be nonzero.
    CombatObjectPtr m_mission_subtarget;
    CombatObjectWeakPtr m_base;

    int m_formation_position;
    CombatFighterFormationPtr m_formation;
    OpenSteer::Vec3 m_out_of_formation;

    PathingEngine& m_pathing_engine;

    // TODO: Temporary only!
    bool m_instrument;
    MissionType m_last_mission;
};

#endif
