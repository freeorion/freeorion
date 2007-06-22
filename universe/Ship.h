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
    Ship(int empire_id, int design_id);             ///< general ctor taking ship's empire and design id; from this the design can be looked up and used to create the ship
    //@}

    /** \name Accessors */ //@{
    const  ShipDesign* Design() const; ///< returns the design of the ship, containing engine type, weapons, etc.
    int    ShipDesignID() const;       ///< returns the design id of the ship
    int    FleetID() const;            ///< returns the ID of the fleet the ship is residing in
    Fleet* GetFleet() const;           ///< returns the ID of the fleet the ship is residing in

    virtual UniverseObject::Visibility GetVisibility(int empire_id) const;
    virtual const std::string& PublicName(int empire_id) const;

    bool IsArmed() const;
    double Speed() const;

    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;
    //@}

    /** \name Mutators */ //@{   
    void         SetFleetID(int fleet_id) {m_fleet_id = fleet_id; StateChangedSignal();} ///< sets the ID of the fleet the ship resides in
    virtual void MovementPhase();
    virtual void PopGrowthProductionResearchPhase();
    //@}

private:
    int         m_design_id;
    int         m_fleet_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Ship::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_fleet_id);
}

#endif // _Ship_h_


