// -*- C++ -*-
#ifndef _Fleet_h_
#define _Fleet_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#include <list>

class System;

/** */
class Fleet : public UniverseObject
{
private:
    typedef std::set<int> ShipIDSet;
   
public:
    typedef ShipIDSet::iterator         iterator;         ///< an iterator to the ships in the fleet
    typedef ShipIDSet::const_iterator   const_iterator;   ///< a const iterator to the ships in the fleet
   
    /** \name Structors */ //@{
    Fleet(); ///< default ctor
    Fleet(const std::string& name, double x, double y, int owner);
    Fleet(const XMLElement& elem); ///< ctor that constructs a Fleet object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Fleet object
    //@}

    /** \name Accessors */ //@{
    const_iterator begin() const  {return m_ships.begin();}  ///< returns the begin const_iterator for the ships in the fleet
    const_iterator end() const    {return m_ships.end();}    ///< returns the end const_iterator for the ships in the fleet

    virtual UniverseObject::Visibility GetVisibility(int empire_id) const;
    virtual const std::string& PublicName(int empire_id) const;
    virtual XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const;

    /** Returns the list of systems that this fleet will move through en route to its destination (may be empty). 
        If this fleet is currently at a system, that system will be the first one in the list. */
    const std::list<System*>& TravelRoute() const;

    /// Returns the number of turns which must elapse before the fleet arrives at its final destination and the turns to the next system, respectively.
    std::pair<int, int> ETA() const;
   
    /// Returns ID of system that this fleet is moving to.
    int FinalDestinationID() const {return m_moving_to;}

    /// Returns system that this fleet is moving to (may be null).
    System* FinalDestination() const;

    /// Returns ID of system that this fleet is moving away from as it moves to its destination.
    int PreviousSystemID() const {return m_prev_system;}

    /// Returns ID of system that this fleet is moving to next as it moves to its destination.
    int NextSystemID() const {return m_next_system;}

    /// Returns true iff this fleet can change its direction while in interstellar space.
    bool CanChangeDirectionEnRoute() const;

    /// Returns true if there is at least one armed ship in the fleet.
    bool HasArmedShips() const;

    /// Returns number of ships in fleet.
    int NumShips() const {return m_ships.size();}

    /// Returns true iff this Fleet contains a ship with ID \a id.
    bool ContainsShip(int id) const {return m_ships.find(id) != m_ships.end();}

    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;
    //@}
   
    /** \name Mutators */ //@{
    void              SetRoute(const std::list<System*>& route, double distance);  ///< orders the fleet to move through the systems in the list, in order
   
    void              AddShip(int ship_id);                        ///< adds the ship to the fleet
    bool              RemoveShip(int ship); ///< removes the ship from the fleet. Returns false if no ship with ID \a id was found.
    void              AddShips(const std::vector<int>& ships);     ///< adds the ships with the IDs stored in \a ships to the fleet
    std::vector<int>  RemoveShips(const std::vector<int>& ships);  ///< removes the ships with the IDs stored in \a ships from the fleet, and returns any IDs not found in the fleet
    std::vector<int>  DeleteShips(const std::vector<int>& ships);  ///< removes and deletes the ships with the IDs stored in \a ships from the fleet, and returns any IDs not found in the fleet
   
    iterator begin()  {return m_ships.begin();}  ///< returns the begin iterator for the ships in the fleet
    iterator end()    {return m_ships.end();}    ///< returns the end iterator for the ships in the fleet

    virtual void MovementPhase();
    virtual void PopGrowthProductionResearchPhase();
    //@}

private:
    void CalculateRoute() const; // sets m_travel_route and m_travel_distance to their proper values based on the other member data

    ShipIDSet           m_ships;
    int                 m_moving_to;

    // these two uniquely describe the starlane graph edge the fleet is on, if it it's on one
    int                 m_prev_system;  // the next system in the route, if any
    int                 m_next_system;  // the previous system in the route, if any 

    // these are filled temporarily and only as needed, and they can be deduced from the other info above; don't put them in the XML encoding
    mutable std::list<System*>  m_travel_route;
    mutable double              m_travel_distance;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Fleet::serialize(Archive& ar, const unsigned int version)
{
    bool visible;
    int moving_to;
    if (Archive::is_saving::value)
        visible = ALL_OBJECTS_VISIBLE || Universe::s_encoding_empire == Universe::ALL_EMPIRES || OwnedBy(Universe::s_encoding_empire);
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(visible);
    if (Archive::is_saving::value)
        moving_to = visible ? m_moving_to : m_next_system;
    ar  & BOOST_SERIALIZATION_NVP(m_ships)
        & BOOST_SERIALIZATION_NVP(moving_to)
        & BOOST_SERIALIZATION_NVP(m_prev_system)
        & BOOST_SERIALIZATION_NVP(m_next_system);
    if (Archive::is_loading::value)
        m_moving_to = moving_to;
}

inline std::string FleetRevision()
{return "$Id$";}

#endif // _Fleet_h_
