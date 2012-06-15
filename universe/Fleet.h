// -*- C++ -*-
#ifndef _Fleet_h_
#define _Fleet_h_

#include "UniverseObject.h"

////////////////////////////////////////////////
// MovePathNode
////////////////////////////////////////////////
/** Contains info about a single notable point on the move path of a fleet or other UniverseObject. */
struct MovePathNode {
    MovePathNode(double x_, double y_, bool turn_end_, int eta_, int id_, int lane_start_id_, int lane_end_id_) :
        x(x_), y(y_), turn_end(turn_end_), eta(eta_), object_id(id_), lane_start_id(lane_start_id_), lane_end_id(lane_end_id_)
    {}
    double  x, y;           ///< location in Universe of node
    bool    turn_end;       ///< true if the fleet will end a turn at this point
    int     eta;            ///< estimated turns to reach this node
    int     object_id;      ///< id of object (most likely a system) located at this node, or INVALID_OBJECT_ID if there is no object here
    int     lane_start_id;  ///< id of object (most likely a system) at the start of the starlane on which this MovePathNode is located, or INVALID_OBJECT_ID if not on a starlane
    int     lane_end_id;    ///< id of object (most likely a system) at the end of the starlane on which this MovePathNode is located, or INVALID_OBJECT_ID if not on a starlane
};

/** encapsulates data for a FreeOrion fleet.  Fleets are basically a group of ships that travel together. */
class Fleet : public UniverseObject
{
public:
    typedef std::set<int>               ShipIDSet;
    typedef ShipIDSet::iterator         iterator;                       ///< an iterator to the ships in the fleet
    typedef ShipIDSet::const_iterator   const_iterator;                 ///< a const iterator to the ships in the fleet

    /** \name Structors */ //@{
    Fleet() :
        UniverseObject(),
        m_moving_to(INVALID_OBJECT_ID),
        m_prev_system(INVALID_OBJECT_ID),
        m_next_system(INVALID_OBJECT_ID),
        m_travel_distance(0.0),
        m_arrived_this_turn(false),
        m_arrival_starlane(INVALID_OBJECT_ID)
    {}
    Fleet(const std::string& name, double x, double y, int owner);      ///< general ctor taking name, position and owner id

    virtual Fleet*                      Clone(int empire_id = ALL_EMPIRES) const;  ///< returns new copy of this Fleet
    //@}

    /** \name Accessors */ //@{
    virtual const std::string&          TypeName() const;                   ///< returns user-readable string indicating the type of UniverseObject this is
    virtual std::string                 Dump() const;

    const_iterator                      begin() const       { return m_ships.begin(); } ///< returns the begin const_iterator for the ships in the fleet
    const_iterator                      end() const         { return m_ships.end(); }   ///< returns the end const_iterator for the ships in the fleet

    const std::set<int>&                ShipIDs() const     { return m_ships; }         ///< returns set of IDs of ships in fleet.

    virtual const std::string&          PublicName(int empire_id) const;

    /** Returns the list of systems that this fleet will move through en route
      * to its destination (may be empty).  If this fleet is currently at a
      * system, that system will be the first one in the list. */
    const std::list<int>&               TravelRoute() const;

    /** Returns a list of locations at which notable events will occur along the fleet's path if it follows the 
        specified route.  It is assumed in the calculation that the fleet starts its move path at its actual current
        location, however the fleet's current location will not be on the list, even if it is currently in a system. */
    std::list<MovePathNode>             MovePath(const std::list<int>& route) const;
    std::list<MovePathNode>             MovePath() const;                   ///< Returns MovePath for fleet's current TravelRoute
    std::pair<int, int>                 ETA() const;                                            ///< Returns the number of turns which must elapse before the fleet arrives at its current final destination and the turns to the next system, respectively.
    std::pair<int, int>                 ETA(const std::list<MovePathNode>& move_path) const;    ///< Returns the number of turns which must elapse before the fleet arrives at the final destination and next system in the spepcified \a move_path
    double                              Fuel() const;                       ///< Returns effective amount of fuel this fleet has, which is the least of the amounts of fuel that the ships have
    double                              MaxFuel() const;                    ///< Returns effective maximum amount of fuel this fleet has, which is the least of the max amounts of fuel that the ships can have
    int                                 FinalDestinationID() const          { return m_moving_to; }     ///< Returns ID of system that this fleet is moving to.
    int                                 PreviousSystemID() const            { return m_prev_system; }   ///< Returns ID of system that this fleet is moving away from as it moves to its destination.
    int                                 NextSystemID() const                { return m_next_system; }   ///< Returns ID of system that this fleet is moving to next as it moves to its destination.
    double                              Speed() const;                      ///< Returns speed of fleet. (Should be equal to speed of slowest ship in fleet, unless in future the calculation of fleet speed changes.)
    bool                                CanChangeDirectionEnRoute() const   { return false; }           ///< Returns true iff this fleet can change its direction while in interstellar space.
    bool                                HasMonsters() const;                ///< returns true iff this fleet contains monster ships.
    bool                                HasArmedShips() const;              ///< Returns true if there is at least one armed ship in the fleet.
    bool                                HasColonyShips() const;             ///< Returns true if there is at least one colony ship in the fleet.
    bool                                HasTroopShips() const;              ///< Returns true if there is at least one troop ship in the fleet.
    int                                 NumShips() const                    { return m_ships.size(); }  ///< Returns number of ships in fleet.
    bool                                Empty() const                       { return m_ships.empty(); } ///< Returns true if fleet contains no ships, false otherwise.
    virtual bool                        Contains(int object_id) const;      ///< Returns true iff this Fleet contains a ship with ID \a id.
    virtual std::vector<UniverseObject*>FindObjects() const;                ///< returns objects contained within this fleet
    virtual std::vector<int>            FindObjectIDs() const;              ///< returns ids of objects contained within this fleet

    /** Returns true iff this fleet is moving, but the route is unknown.  This
      * is usually the case when a foreign player A's fleet is represented on
      * another player B's client, and player B has never seen one or more of
      * the systems in the fleet's route. */
    bool                                UnknownRoute() const;

    /** Returns true iff this fleet arrived at its current System this turn. */
    bool                                ArrivedThisTurn() const             { return m_arrived_this_turn; }

    /** Returns the ID of the starlane that this fleet arrived on.  The value
        returned is undefined if ArrivedThisTurn() does not return true. */
    int                                 ArrivalStarlane() const             { return m_arrival_starlane; }

    virtual UniverseObject*             Accept(const UniverseObjectVisitor& visitor) const;
    //@}

    /** \name Mutators */ //@{
    virtual void            Copy(const UniverseObject* copied_object, int empire_id = ALL_EMPIRES);

    void                    SetRoute(const std::list<int>& route);          ///< sets this fleet to move through the series of systems in the list, in order
    void                    CalculateRoute() const;                         ///< sets this fleet to move through the series of systems that makes the shortest path from its current location to its current destination system

    virtual void            MovementPhase();

    void                    AddShip(int ship_id);                           ///< adds the ship to the fleet
    bool                    RemoveShip(int ship);                           ///< removes the ship from the fleet. Returns false if no ship with ID \a id was found.

    iterator                begin() { return m_ships.begin(); }             ///< returns the begin iterator for the ships in the fleet
    iterator                end()   { return m_ships.end(); }               ///< returns the end iterator for the ships in the fleet

    virtual void            SetSystem(int sys);
    virtual void            MoveTo(double x, double y);
    void                    SetNextAndPreviousSystems(int next, int prev);  ///< sets the previous and next systems for this fleet.  Useful after moving a moving fleet to a different location, so that it moves along its new local starlanes
    //@}

    /* returns a name for a fleet based on the specified \a ship_ids */
    static std::string      GenerateFleetName(const std::vector<int>& ship_ids, int new_fleet_id = INVALID_OBJECT_ID);

    static const int            ETA_NEVER;                                  ///< returned by ETA when fleet can't reach destination due to lack of route or inability to move
    static const int            ETA_UNKNOWN;                                ///< returned when ETA can't be determined
    static const int            ETA_OUT_OF_RANGE;                           ///< returned by ETA when fleet can't reach destination due to insufficient fuel capacity and lack of fleet resupply on route

protected:
    virtual void            ResetTargetMaxUnpairedMeters();

private:
    void                    ShortenRouteToEndAtSystem(std::list<int>& travel_route, int last_system);   ///< removes any systems on the route after the specified system

    ShipIDSet               VisibleContainedObjects(int empire_id) const;   ///< returns the subset of m_ships that is visible to empire with id \a empire_id

    ShipIDSet                   m_ships;
    int                         m_moving_to;

    // these two uniquely describe the starlane graph edge the fleet is on, if it it's on one
    int                         m_prev_system;                              ///< the next system in the route, if any
    int                         m_next_system;                              ///< the previous system in the route, if any 

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
    int                         m_arrival_starlane;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(Fleet, 1)

#endif // _Fleet_h_
