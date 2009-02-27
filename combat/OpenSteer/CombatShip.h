// -*- C++ -*-
#ifndef COMBAT_SHIP_H
#define COMBAT_SHIP_H

#include "PathingEngineFwd.h"

#include "SimpleVehicle.h"

#include <boost/enable_shared_from_this.hpp>

#include <list>


class PathingEngine;

class CombatShip :
    public OpenSteer::SimpleVehicle,
    public boost::enable_shared_from_this<CombatShip>
{
public:
    /** The missions available to ships.  The notion of "weakest" is
        intentionally left fuzzy.  The weakest target is one that is likely to
        die quickly, and also one that is unlikely to kill the ship.  Some
        relative of scaling these two factors should be employed to come up with
        a single weakness value. */
    enum MissionType {
        /** No mission set.  This mission is treated as
            ATTACK_SHIPS_NEAREST_FIRST. */
        NONE,

        /** Moves to a speficic location.  Enemy presence will be ignored. */
        MOVE_TO,

        /** Attack a specific fighter group or vessel.  The ship will close to
            within its farthest-reaching weapon's range of its target. */
        ATTACK_THIS_STANDOFF,

        /** Attack a specific fighter group or vessel.  The ship will close to
            within its closest-reaching non-PD weapon's range of its target. */
        ATTACK_THIS,

        /** Defend a specific fighter group or vessel, by attacking its
            attackers in order from weakest to strongest.  Defenders with
            sufficient PD will place themselves between the defendee and known
            hostile LR sources. */
        DEFEND_THIS,

        /** Moves to a specific location.  Movement will be interrupted to
            engage non-fighter targets as soon as they are found. */
        PATROL_TO,

        /** Attack ships without further direction, in order of weakest to
            strongest.  The ship will close to within its farthest-reaching
            weapon's range of its target. */
        ATTACK_SHIPS_WEAKEST_FIRST_STANDOFF,

        /** Attack ships without further direction, in order of weakest to
            strongest.  The ship will close to within its farthest-reaching
            weapon's range of its target. */
        ATTACK_SHIPS_NEAREST_FIRST_STANDOFF,

        /** Attack ships without further direction, in order of weakest to
            strongest.  The ship will close to within its closest-reaching
            non-PD weapon's range of its target. */
        ATTACK_SHIPS_WEAKEST_FIRST,

        /** Attack ships without further direction, in order of nearest to
            farthest.  The ship will close to within its closest-reaching non-PD
            weapon's range of its target. */
        ATTACK_SHIPS_NEAREST_FIRST,

        /** Enter starlane.  The ship is immobilized for a few combat turns
            until the ship can enter the starlane.  The ship can still attack
            and be attacked.  This mission is only valid when the ship is inside
            a starlane entry point -- if it isn't, the mission is cancelled. */
        ENTER_STARLANE
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

    CombatShip(int empire_id, const OpenSteer::Vec3& position,
               float anti_fighter_strength,
               PathingEngine& pathing_engine);
    ~CombatShip();

    float AntiFighterStrength() const;

    virtual void update(const float /*current_time*/, const float elapsed_time);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity, const float elapsedTime);

    // TODO: for testing only
    void Draw();

private:
    float MaxWeaponRange() const;
    float MinNonPDWeaponRange() const;

    void Init(const OpenSteer::Vec3& position_);
    void RemoveMission();
    void UpdateMissionQueue();
    OpenSteer::Vec3 Steer();
    CombatObjectPtr WeakestAttacker(const CombatObjectPtr& attackee);
    CombatShipPtr WeakestHostileShip();

    ProximityDBToken* m_proximity_token;
    int m_empire_id;
    OpenSteer::Vec3 m_last_steer;

    std::list<Mission> m_mission_queue;
    float m_mission_weight;
    OpenSteer::Vec3 m_mission_destination; // Only the X and Y values should be nonzero.
    CombatObjectPtr m_mission_subtarget;

    PathingEngine& m_pathing_engine;
    
    // TODO: This should be computed from the ship design in the final version
    // of this class.
    float m_anti_fighter_strength;

    // TODO: Temporary only!
    bool m_instrument;
    MissionType m_last_mission;
};

#endif
