// -*- C++ -*-
#ifndef _Fleet_h_
#define _Fleet_h_

#include "System.h"

#include <list>

////////////////////////////////////////////////
// MovePathNode
////////////////////////////////////////////////
/** Contains info about a single notable point on the move path of a fleet or other UniverseObject. */
struct MovePathNode {
    MovePathNode(double x_, double y_, bool turn_end_, int eta_) :
        x(x_), y(y_), turn_end(turn_end_), eta(eta_)
    {}
    double x, y;    ///< location in Universe of node
    bool turn_end;  ///< true if the fleet will end a turn at this point
    int eta;        ///< estimated turns to reach this node
};

/** encapsulates data for a FreeOrion fleet.  Fleets are basically a group of ships that travel together. */
class Fleet : public UniverseObject
{
private:
    typedef std::set<int>               ShipIDSet;

public:
    typedef ShipIDSet::iterator         iterator;               ///< an iterator to the ships in the fleet
    typedef ShipIDSet::const_iterator   const_iterator;   ///< a const iterator to the ships in the fleet

    /** \name Structors */ //@{
    Fleet(); ///< default ctor
    Fleet(const std::string& name, double x, double y, int owner);
    //@}

    /** \name Accessors */ //@{
    const_iterator                      begin() const;                      ///< returns the begin const_iterator for the ships in the fleet
    const_iterator                      end() const;                        ///< returns the end const_iterator for the ships in the fleet

    const std::set<int>&                ShipIDs() const;                    ///< returns set of IDs of ships in fleet.

    virtual UniverseObject::Visibility  GetVisibility(int empire_id) const;
    virtual const std::string&          PublicName(int empire_id) const;

    /** Returns the list of systems that this fleet will move through en route to its destination (may be empty). 
        If this fleet is currently at a system, that system will be the first one in the list. */
    const std::list<System*>&           TravelRoute() const;

    /** Returns a list of locations at which notable events will occur along the fleet's path if it follows the 
        specified route.  It is assumed in the calculation that the fleet starts its move path at its actual current
        location, however the fleet's current location will not be on the list, even if it is currently in a system. */
    std::list<MovePathNode>             MovePath(const std::list<System*>& route) const;
    std::list<MovePathNode>             MovePath() const;                   ///< Returns MovePath for fleet's current TravelRoute
    std::pair<int, int>                 ETA() const;                        ///< Returns the number of turns which must elapse before the fleet arrives at its final destination and the turns to the next system, respectively.
    int                                 FinalDestinationID() const;         ///< Returns ID of system that this fleet is moving to.
    System*                             FinalDestination() const;           ///< Returns system that this fleet is moving to (may be null).
    int                                 PreviousSystemID() const;           ///< Returns ID of system that this fleet is moving away from as it moves to its destination.
    int                                 NextSystemID() const;               ///< Returns ID of system that this fleet is moving to next as it moves to its destination.
    double                              Speed() const;                      ///< Returns speed of fleet. (Should be equal to speed of slowest ship in fleet, unless in future the calculation of fleet speed changes.)
    bool                                CanChangeDirectionEnRoute() const;  ///< Returns true iff this fleet can change its direction while in interstellar space.
    bool                                HasArmedShips() const;              ///< Returns true if there is at least one armed ship in the fleet.
    int                                 NumShips() const;                   ///< Returns number of ships in fleet.
    virtual bool                        Contains(int object_id) const;      ///< Returns true iff this Fleet contains a ship with ID \a id.
    virtual std::vector<UniverseObject*>FindObjects() const;                ///< returns objects contained within this fleet
    virtual std::vector<int>            FindObjectIDs() const;              ///< returns ids of objects contained within this fleet

    /** Returns true iff this fleet is moving, but the route is unknown.  This is usually the case when a foreign
      * player A's fleet is represented on another player B's client, and player B has never seen one or more of the
      * systems in the fleet's route. */
    bool                                UnknownRoute() const;

    virtual UniverseObject*             Accept(const UniverseObjectVisitor& visitor) const;
    //@}

    /** \name Mutators */ //@{
    void                    SetRoute(const std::list<System*>& route, double distance);  ///< orders the fleet to move through the systems in the list, in order

    void                    AddShip(int ship_id);                           ///< adds the ship to the fleet
    bool                    RemoveShip(int ship);                           ///< removes the ship from the fleet. Returns false if no ship with ID \a id was found.
    void                    AddShips(const std::vector<int>& ships);        ///< adds the ships with the IDs stored in \a ships to the fleet
    std::vector<int>        RemoveShips(const std::vector<int>& ships);     ///< removes the ships with the IDs stored in \a ships from the fleet, and returns any IDs not found in the fleet
    std::vector<int>        DeleteShips(const std::vector<int>& ships);     ///< removes and deletes the ships with the IDs stored in \a ships from the fleet, and returns any IDs not found in the fleet

    iterator                begin();                                        ///< returns the begin iterator for the ships in the fleet
    iterator                end();                                          ///< returns the end iterator for the ships in the fleet

    virtual void            SetSystem(int sys);
    virtual void            MovementPhase();
    virtual void            PopGrowthProductionResearchPhase();
    //@}

    static const int            ETA_NEVER;
    static const int            ETA_UNKNOWN;

private:
    void                    CalculateRoute() const;                         ///< sets m_travel_route and m_travel_distance to their proper values based on the other member data
    void                    RecalculateFleetSpeed();                        ///< recalculates the speed of the fleet by finding the lowest speed of the ships in the fleet.
    void                    ShortenRouteToEndAtSystem(std::list<System*>& travel_route, int last_system);   ///< removes any systems on the route after the specified system

    ShipIDSet                   m_ships;
    int                         m_moving_to;

    double                      m_speed;                                    ///< updated whenever ship is added or removed from fleet.  equal to speed of slowest ship in fleet.

    // these two uniquely describe the starlane graph edge the fleet is on, if it it's on one
    int                         m_prev_system;                              ///< the next system in the route, if any
    int                         m_next_system;                              ///< the previous system in the route, if any 

    mutable std::list<System*>  m_travel_route;                             ///< Note that this may contain a single null ptr, indicating that the route is unknown, but needs not be recomputed.
    mutable double              m_travel_distance;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(Fleet, 1)


// template implementations
template <class Archive>
void Fleet::serialize(Archive& ar, const unsigned int version)
{
    bool vis;
    int moving_to;
    std::list<System*> travel_route;
    double travel_distance;
    if (Archive::is_saving::value)
        vis = GetVisibility(Universe::s_encoding_empire);
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(vis);
    if (Archive::is_saving::value) {
        moving_to = (Universe::ALL_OBJECTS_VISIBLE || vis == FULL_VISIBILITY) ? m_moving_to : m_next_system;
        if (1 <= version) {
            ShortenRouteToEndAtSystem(travel_route, moving_to);
            travel_distance = m_travel_distance;
            if (!travel_route.empty() && travel_route.front() != 0 && travel_route.size() != m_travel_route.size()) {
                if (moving_to == m_moving_to)
                    moving_to = travel_route.back()->ID();
                travel_distance -= GetUniverse().ShortestPath(travel_route.back()->ID(), m_travel_route.back()->ID()).second;
            }
        }
    }
    ar  & BOOST_SERIALIZATION_NVP(m_ships)
        & BOOST_SERIALIZATION_NVP(moving_to)
        & BOOST_SERIALIZATION_NVP(m_prev_system)
        & BOOST_SERIALIZATION_NVP(m_next_system);
    if (1 <= version) {
        ar  & BOOST_SERIALIZATION_NVP(m_speed)
            & BOOST_SERIALIZATION_NVP(travel_route)
            & BOOST_SERIALIZATION_NVP(travel_distance);
    }
    if (Archive::is_loading::value) {
        m_moving_to = moving_to;
        if (1 <= version) {
            std::swap(m_travel_route, travel_route);
            m_travel_distance = travel_distance;
        }
    }
    if (Universe::ALL_OBJECTS_VISIBLE ||
        vis == FULL_VISIBILITY)
        ar  & BOOST_SERIALIZATION_NVP(m_speed);
}

#endif // _Fleet_h_
