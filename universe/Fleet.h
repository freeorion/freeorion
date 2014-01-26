// -*- C++ -*-
#ifndef _Fleet_h_
#define _Fleet_h_

#include "UniverseObject.h"
#include "ObjectMap.h"
#include "../util/Export.h"

#include <boost/serialization/version.hpp>

////////////////////////////////////////////////
// MovePathNode
////////////////////////////////////////////////
/** Contains info about a single notable point on the move path of a fleet or
  * other UniverseObject. */
struct MovePathNode {
    MovePathNode(double x_, double y_, bool turn_end_, int eta_, int id_, int lane_start_id_, int lane_end_id_, bool post_blockade_ = false) :
    x(x_), y(y_), turn_end(turn_end_), eta(eta_), object_id(id_), lane_start_id(lane_start_id_), lane_end_id(lane_end_id_), post_blockade(post_blockade_)
    {}
    double  x, y;           ///< location in Universe of node
    bool    turn_end;       ///< true if the fleet will end a turn at this point
    int     eta;            ///< estimated turns to reach this node
    int     object_id;      ///< id of object (most likely a system) located at this node, or INVALID_OBJECT_ID if there is no object here
    int     lane_start_id;  ///< id of object (most likely a system) at the start of the starlane on which this MovePathNode is located, or INVALID_OBJECT_ID if not on a starlane
    int     lane_end_id;    ///< id of object (most likely a system) at the end of the starlane on which this MovePathNode is located, or INVALID_OBJECT_ID if not on a starlane
    bool    post_blockade;  ///< estimation of whether this node is past a blockade for the subject fleet
};

/** Encapsulates data for a FreeOrion fleet.  Fleets are basically a group of
  * ships that travel together. */
class FO_COMMON_API Fleet : public UniverseObject {
public:
    /** \name Accessors */ //@{
    virtual const std::string&          TypeName() const;                   ///< returns user-readable string indicating the type of UniverseObject this is
    virtual UniverseObjectType          ObjectType() const;
    virtual std::string                 Dump() const;

    const std::set<int>&                ShipIDs() const     { return m_ships; }         ///< returns set of IDs of ships in fleet.

    virtual int                         ContainerObjectID() const;          ///< returns id of the object that directly contains this object, if any, or INVALID_OBJECT_ID if this object is not contained by any other
    virtual const std::set<int>&        ContainedObjectIDs() const;         ///< returns ids of objects directly contained within this object
    virtual bool                        Contains(int object_id) const;      ///< returns true if there is an object with id \a object_id is contained within this UniverseObject
    virtual bool                        ContainedBy(int object_id) const;   ///< returns true if there is an object with id \a object_id that contains this UniverseObject

    virtual const std::string&          PublicName(int empire_id) const;

    /** Returns the list of systems that this fleet will move through en route
      * to its destination (may be empty).  If this fleet is currently at a
      * system, that system will be the first one in the list. */
    const std::list<int>&               TravelRoute() const;

    int                                 OrderedGivenToEmpire() const    { return m_ordered_given_to_empire_id; }///< returns the ID of the empire this fleet has been ordered given to, or ALL_EMPIRES if this fleet hasn't been ordered given to an empire

    bool                                Aggressive() const  { return m_aggressive; }

    /** Returns a list of locations at which notable events will occur along the fleet's path if it follows the 
        specified route.  It is assumed in the calculation that the fleet starts its move path at its actual current
        location, however the fleet's current location will not be on the list, even if it is currently in a system. */
    std::list<MovePathNode>             MovePath(const std::list<int>& route, bool flag_blockades = false) const;
    std::list<MovePathNode>             MovePath(bool flag_blockades = false) const;              ///< Returns MovePath for fleet's current TravelRoute
    std::pair<int, int>                 ETA() const;                                            ///< Returns the number of turns which must elapse before the fleet arrives at its current final destination and the turns to the next system, respectively.
    std::pair<int, int>                 ETA(const std::list<MovePathNode>& move_path) const;    ///< Returns the number of turns which must elapse before the fleet arrives at the final destination and next system in the spepcified \a move_path
    float                               Damage() const;                     ///< Returns total amount of damage this fleet has, which is the sum of the ships' damage
    float                               Structure() const;                  ///< Returns total amount of structure this fleet has, which is the sum of the ships' structure
    float                               Shields() const;                    ///< Returns total amount of shields this fleet has, which is the sum of the ships' shields
    float                               Fuel() const;                       ///< Returns effective amount of fuel this fleet has, which is the least of the amounts of fuel that the ships have
    float                               MaxFuel() const;                    ///< Returns effective maximum amount of fuel this fleet has, which is the least of the max amounts of fuel that the ships can have
    int                                 FinalDestinationID() const          { return m_moving_to; }     ///< Returns ID of system that this fleet is moving to.
    int                                 PreviousSystemID() const            { return m_prev_system; }   ///< Returns ID of system that this fleet is moving away from as it moves to its destination.
    int                                 NextSystemID() const                { return m_next_system; }   ///< Returns ID of system that this fleet is moving to next as it moves to its destination.
    bool                                BlockadedAtSystem(int start_system_id, int dest_system_id) const; ///< returns true iff this fleet's movement would be blockaded at system.
    float                               Speed() const;                      ///< Returns speed of fleet. (Should be equal to speed of slowest ship in fleet, unless in future the calculation of fleet speed changes.)
    bool                                CanChangeDirectionEnRoute() const   { return false; }           ///< Returns true iff this fleet can change its direction while in interstellar space.
    bool                                HasMonsters() const;                ///< returns true iff this fleet contains monster ships.
    bool                                HasArmedShips() const;              ///< Returns true if there is at least one armed ship in the fleet.
    bool                                HasColonyShips() const;             ///< Returns true if there is at least one colony ship with nonzero capacity in the fleet.
    bool                                HasOutpostShips() const;            ///< Returns true if there is at least one colony ship with zero capacity in the fleet
    bool                                HasTroopShips() const;              ///< Returns true if there is at least one troop ship in the fleet.
    bool                                HasShipsOrderedScrapped() const;    ///< Returns true if there is at least one ship ordered scrapped in the fleet.
    bool                                HasShipsWithoutScrapOrders() const; ///< Returns true if there is at least one ship without any scrap orders in the fleet.
    int                                 NumShips() const                    { return m_ships.size(); }  ///< Returns number of ships in fleet.
    bool                                Empty() const                       { return m_ships.empty(); } ///< Returns true if fleet contains no ships, false otherwise.

    /** Returns true iff this fleet is moving, but the route is unknown.  This
      * is usually the case when a foreign player A's fleet is represented on
      * another player B's client, and player B has never seen one or more of
      * the systems in the fleet's route. */
    bool                                UnknownRoute() const;

    /** Returns true iff this fleet arrived at its current System this turn. */
    bool                                ArrivedThisTurn() const             { return m_arrived_this_turn; }

    /** Has two primary uses: orientation in tactical combat, and determination of starlane blockade restrictions.
     * Returns the ID of the starlane that this fleet arrived on, if it arrived into a blockade which is not yet broken.
     * If in a system and not blockaded, the value is the current system ID. The blockade intent is that you can't
     * break a blockade unless you beat the blockaders (via combat or they retreat).**/
    int                                 ArrivalStarlane() const             { return m_arrival_starlane; }

    virtual TemporaryPtr<UniverseObject>Accept(const UniverseObjectVisitor& visitor) const;
    //@}

    /** \name Mutators */ //@{
    virtual void            Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES);

    void                    SetRoute(const std::list<int>& route);          ///< sets this fleet to move through the series of systems in the list, in order
    void                    CalculateRoute() const;                         ///< sets this fleet to move through the series of systems that makes the shortest path from its current location to its current destination system

    void                    SetAggressive(bool aggressive = true);          ///< sets this fleet to be agressive (true) or passive (false)

    virtual void            MovementPhase();

    void                    AddShip(int ship_id);                           ///< adds the ship to the fleet
    void                    AddShips(const std::vector<int>& ship_ids);     ///< adds the ships to the fleet
    void                    RemoveShip(int ship_id);                        ///< removes the ship from the fleet.
    void                    RemoveShips(const std::vector<int>& ship_ids);  ///< removes the ships from the fleet.

    void                    SetNextAndPreviousSystems(int next, int prev);  ///< sets the previous and next systems for this fleet.  Useful after moving a moving fleet to a different location, so that it moves along its new local starlanes
    void                    SetArrivalStarlane(int starlane) { m_arrival_starlane = starlane; }  ///< sets the arrival starlane, used to clear blockaded status after combat
    void                    ClearArrivalFlag() { m_arrived_this_turn = false; } ///< used to clear the m_arrived_this_turn flag, prior to any fleets moving, for accurate blockade tests

    void                    SetGiveToEmpire(int empire_id);                 ///< marks fleet to be given to empire
    void                    ClearGiveToEmpire();                            ///< marks fleet not to be given to any empire

    virtual void            ResetTargetMaxUnpairedMeters();
    //@}

    /* returns a name for a fleet based on the specified \a ship_ids */
    static std::string      GenerateFleetName(const std::vector<int>& ship_ids, int new_fleet_id = INVALID_OBJECT_ID);

    static const int            ETA_NEVER;                                  ///< returned by ETA when fleet can't reach destination due to lack of route or inability to move
    static const int            ETA_UNKNOWN;                                ///< returned when ETA can't be determined
    static const int            ETA_OUT_OF_RANGE;                           ///< returned by ETA when fleet can't reach destination due to insufficient fuel capacity and lack of fleet resupply on route

protected:
    friend class Universe;
    /** \name Structors */ //@{
    Fleet() :
        UniverseObject(),
        m_moving_to(INVALID_OBJECT_ID),
        m_prev_system(INVALID_OBJECT_ID),
        m_next_system(INVALID_OBJECT_ID),
        m_aggressive(true),
        m_ordered_given_to_empire_id(ALL_EMPIRES),
        m_travel_distance(0.0),
        m_arrived_this_turn(false),
        m_arrival_starlane(INVALID_OBJECT_ID)
    {}
    Fleet(const std::string& name, double x, double y, int owner);      ///< general ctor taking name, position and owner id

    template <class T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);
    template <class T> friend void boost::checked_delete(T* x);
    ~Fleet() {}

    virtual Fleet*          Clone(int empire_id = ALL_EMPIRES) const;  ///< returns new copy of this Fleet
    //@}

private:
    ///< removes any systems on the route after the specified system
    void                    ShortenRouteToEndAtSystem(std::list<int>& travel_route, int last_system);

    std::set<int>               m_ships;
    int                         m_moving_to;

    // these two uniquely describe the starlane graph edge the fleet is on, if it it's on one
    int                         m_prev_system;  ///< the previous system in the route, if any
    int                         m_next_system;  ///< the next system in the route, if any

    bool                        m_aggressive;    ///< should this fleet attack enemies in the same system?

    int                         m_ordered_given_to_empire_id;

    /** list of systems on travel route of fleet from current position to
      * destination.  If the fleet is currently in a system, that will be the
      * first system on the list.  Otherwise, the first system on the list will
      * be the next system the fleet will reach along its path.  The list may
      * also contain a single null pointer, which indicates that the route is
      * unknown.  The list may also be empty, which indicates that it has not
      * yet been caluclated, and CalculateRoute should be called. */
    mutable std::list<int>      m_travel_route;
    mutable double              m_travel_distance;

    bool                        m_arrived_this_turn;
    int                         m_arrival_starlane; // see comment for ArrivalStarlane()

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(Fleet, 1)

#endif // _Fleet_h_
