// -*- C++ -*-
#ifndef _Ship_h_
#define _Ship_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

class Fleet;
class ShipDesign;

/** a class representing a single FreeOrion ship*/
class Ship : public UniverseObject
{
public:
    /** \name Structors */ //@{
    Ship(); ///< default ctor
    Ship(int empire_id, const std::string& design_name); ///< general ctor taking just the ship's empire and design name; from this the design can be looked up and used to create the ship
    Ship(const GG::XMLElement& elem); ///< ctor that constructs a Ship object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Ship object
    //@}

    /** \name Accessors */ //@{
    const  ShipDesign* Design() const; ///< returns the design of the ship, containing engine type, weapons, etc.
    int    FleetID() const;            ///< returns the ID of the fleet the ship is residing in
    Fleet* GetFleet() const;           ///< returns the ID of the fleet the ship is residing in

    virtual UniverseObject::Visibility GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
    virtual GG::XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a Ship object with visibility limited relative to the input empire

    bool IsArmed() const;
    //@}

    /** \name Mutators */ //@{   
    void         SetFleetID(int fleet_id) {m_fleet_id = fleet_id; StateChangedSignal()();} ///< sets the ID of the fleet the ship resides in
    virtual void MovementPhase();
    virtual void PopGrowthProductionResearchPhase();
    //@}

private:
    std::string m_design_name;
    int         m_fleet_id;
};

#endif // _Ship_h_


