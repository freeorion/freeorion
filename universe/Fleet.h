// -*- C++ -*-
#ifndef _Fleet_h_
#define _Fleet_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

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
    Fleet(const GG::XMLElement& elem); ///< ctor that constructs a Fleet object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Fleet object
    //@}

    /** \name Accessors */ //@{
    const_iterator begin() const  {return m_ships.begin();}  ///< returns the begin const_iterator for the ships in the fleet
    const_iterator end() const    {return m_ships.end();}    ///< returns the end const_iterator for the ships in the fleet

    virtual UniverseObject::Visibility Visible(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
   
    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a Fleet object
    virtual GG::XMLElement XMLEncode(int empire_id) const; ///< constructs an XMLElement from a Fleet object with visibility limited relative to the input empire
   
    /// Returns the number of turns which must elapse before the fleet arrives at its destination.  
    int ETA() const;
   
    /// Returns ID of system that this fleet is moving to
    int DestinationID() const {return m_moving_to;}

    /// Returns system that this fleet is moving to (may be null)
    System* Destination() const;
   
    /// Returns true if there is at least one armed ship in the fleet
    bool HasArmedShips() const;

    /// Returns number of ships in fleet
    int NumShips() const {return m_ships.size();}

    /// Returns true iff this Fleet contains a ship with ID \a id
    bool ContainsShip(int id) const {return m_ships.find(id) != m_ships.end();}
    //@}
   
    /** \name Mutators */ //@{
    void              SetDestination(int id);  ///< orders the fleet to move to the system with ID \a id
   
    void              AddShips(const std::vector<int>& ships);     ///< adds the ships with the IDs stored in \a ships to the fleet
    void              AddShip(int ship_id);                        ///< adds the ship to the fleet
    std::vector<int>  RemoveShips(const std::vector<int>& ships);  ///< removes the ships with the IDs stored in \a ships from the fleet, and returns any IDs not found in the fleet
    std::vector<int>  DeleteShips(const std::vector<int>& ships);  ///< removes and deletes the ships with the IDs stored in \a ships from the fleet, and returns any IDs not found in the fleet
   
    iterator begin()  {return m_ships.begin();}  ///< returns the begin iterator for the ships in the fleet
    iterator end()    {return m_ships.end();}    ///< returns the end iterator for the ships in the fleet

    bool RemoveShip(int ship); ///< removes the ship from the fleet. Returns false if no ship with ID \a id was found.

    virtual void MovementPhase( );
    virtual void PopGrowthProductionResearchPhase( );
    //@}

private:
    ShipIDSet   m_ships;
    int         m_moving_to;
};

#endif // _Fleet_h_
