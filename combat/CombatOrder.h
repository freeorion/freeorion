// -*- C++ -*-
#ifndef _CombatOrder_h_
#define _CombatOrder_h_

#include "OpenSteer/PathingEngineFwd.h"
#include "OpenSteer/Vec3.h"

#include <boost/cast.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>


struct ShipMission
{
    /** The missions available to ships.  The notion of "weakest" is
        intentionally left fuzzy.  The weakest target is one that is likely to
        die quickly, and also one that is unlikely to kill the ship.  Some
        relative scaling of these two factors should be employed to come up
        with a single weakness value. */
    enum Type {
        /** No mission set.  This mission is treated as
            ATTACK_SHIPS_NEAREST_FIRST. */
        NONE,

        /** Moves to a speficic location.  Enemy presence will be ignored. */
        MOVE_TO,

        /** Attack a specific fighter group or vessel.  The ship will close to
            within its farthest-reaching weapon's range of its target. */
        ATTACK_THIS_STANDOFF,

        /** Attack a specific fighter group or vessel.  The ship will close to
            within its closest-reaching non-PD weapon's range of its
            target. */
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
            farthest.  The ship will close to within its closest-reaching
            non-PD weapon's range of its target. */
        ATTACK_SHIPS_NEAREST_FIRST,

        /** Enter starlane.  The ship is immobilized for a few combat turns
            until the ship can enter the starlane.  The ship can still attack
            and be attacked.  This mission is only valid when the ship is
            inside a starlane entry point -- if it isn't, the mission is
            cancelled. */
        ENTER_STARLANE
    };

    ShipMission();
    explicit ShipMission(Type type);
    ShipMission(Type type, const OpenSteer::Vec3& destination);
    ShipMission(Type type, const CombatObjectPtr& target);

    Type m_type;
    OpenSteer::Vec3 m_destination;
    CombatObjectWeakPtr m_target;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct FighterMission
{
    /** The missions available to fighters.  The notion of "weakest" is
        intentionally left fuzzy.  The weakest target is one that is likely to
        die quickly, and also one that is unlikely to kill the fighter.  Some
        relative scaling of these two factors should be employed to come up
        with a single weakness value. */
    enum Type {
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

        /** Attack fighters without further direction, in order of nearest to
            farthest bombers, then nearest to farthest interceptors. */
        ATTACK_FIGHTERS_BOMBERS_FIRST,

        /** Attack fighters without further direction, in order of nearest to
            farthest interceptors, then nearest to farthest bombers. */
        ATTACK_FIGHTERS_INTERCEPTORS_FIRST,

        /** Attack ships without further direction, in order of weakest to
            strongest. */
        ATTACK_SHIPS_WEAKEST_FIRST,

        /** Attack ships without further direction, in order of nearest to
            farthest. */
        ATTACK_SHIPS_NEAREST_FIRST,

        /** Return to the fighter's base/carrier.  Enemy presence will not
            affect movement. */
        RETURN_TO_BASE
    };

    FighterMission();
    explicit FighterMission(Type type);
    FighterMission(Type type, const OpenSteer::Vec3& destination);
    FighterMission(Type type, const CombatObjectPtr& target);

    Type m_type;
    OpenSteer::Vec3 m_destination;
    CombatObjectWeakPtr m_target;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/////////////////////////////////////////////////////
// CombatOrder
/////////////////////////////////////////////////////
class CombatOrder
{
public:
    enum OrderType
    {
        SHIP_ORDER,
        FIGHTER_ORDER,
        SETUP_PLACEMENT_ORDER
        // TODO: Bases and planetary defenses will probably need to be
        // represented differently.
    };

    /** \name Structors */ //@{
    CombatOrder();
    CombatOrder(int id, const ShipMission& ship_mission, bool append);
    CombatOrder(int id, const FighterMission& fighter_mission, bool append);
    CombatOrder(int id, const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction);
    //@}

    /** \name Accessors */ //@{
    OrderType Type() const;
    int ID() const;
    const ShipMission& GetShipMission() const;
    const FighterMission& GetFighterMission() const;
    const std::pair<OpenSteer::Vec3, OpenSteer::Vec3>& GetPositionAndDirection() const;
    bool Append() const;
    //@}

private:
    OrderType m_order_type;
    int m_id;
    ShipMission m_ship_mission;
    FighterMission m_fighter_mission;
    std::pair<OpenSteer::Vec3, OpenSteer::Vec3> m_position_and_direction;
    bool m_append;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/////////////////////////////////////////////////////
// CombatOrderSet
/////////////////////////////////////////////////////
typedef std::vector<CombatOrder> CombatOrderSet;

#endif
