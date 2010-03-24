// -*- C++ -*-
#ifndef _Ship_h_
#define _Ship_h_

#include "UniverseObject.h"
#include "Meter.h"

class Fighter;
class Fleet;
class ShipDesign;

/** a class representing a single FreeOrion ship*/
class Ship : public UniverseObject
{
public:
    // map from part type name to (number of parts in the design of that type,
    // number of fighters (or missiles) available of that type) pairs
    typedef std::map<std::string, std::pair<std::size_t, std::size_t> > ConsumablesMap;

    /** \name Structors */ //@{
    Ship();                                         ///< default ctor
    Ship(int empire_id, int design_id);             ///< general ctor taking ship's empire and design id; from this the design can be looked up and used to create the ship

    virtual Ship*               Clone(int empire_id = ALL_EMPIRES) const;  ///< returns new copy of this Ship
    //@}

    /** \name Accessors */ //@{
    virtual const std::string&  TypeName() const;   ///< returns user-readable string indicating the type of UniverseObject this is

    const ShipDesign*           Design() const;     ///< returns the design of the ship, containing engine type, weapons, etc.
    int                         DesignID() const;   ///< returns the design id of the ship
    int                         FleetID() const;    ///< returns the ID of the fleet the ship is residing in

    virtual const std::string&  PublicName(int empire_id) const;

    bool                        IsArmed() const;
    bool                        CanColonize() const;
    double                      Speed() const;

    const ConsumablesMap&       Fighters() const;
    const ConsumablesMap&       Missiles() const;

    virtual UniverseObject*     Accept(const UniverseObjectVisitor& visitor) const;

    virtual double              ProjectedCurrentMeter(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn

    bool                        OrderedScrapped() const;

    const Meter*                GetMeter(MeterType type, const std::string& part_name) const; ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object

    using UniverseObject::GetMeter;
    //@}

    /** \name Mutators */ //@{
    virtual void                Copy(const UniverseObject* copied_object, int empire_id = ALL_EMPIRES);

    void                        SetFleetID(int fleet_id);                       ///< sets the ID of the fleet the ship resides in

    void                        Resupply();

    void                        AddFighters(const std::string& part_name, std::size_t n);
    void                        RemoveFighters(const std::string& part_name, std::size_t n);
    void                        RemoveMissiles(const std::string& part_name, std::size_t n);

    virtual void                MoveTo(double x, double y);

    void                        SetOrderedScrapped(bool b = true);              ///< flags ship for scrapping

    Meter*                      GetMeter(MeterType type, const std::string& part_name); ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object
    //@}

private:
    typedef std::map<std::pair<MeterType, std::string>, Meter> PartMeters;

    virtual void CustomResetMaxMeters(MeterType meter_type = INVALID_METER_TYPE);
    virtual void CustomClampMeters();

    int             m_design_id;
    int             m_fleet_id;
    bool            m_ordered_scrapped;
    ConsumablesMap  m_fighters;
    ConsumablesMap  m_missiles;
    PartMeters      m_part_meters;

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
        & BOOST_SERIALIZATION_NVP(m_fleet_id)
        & BOOST_SERIALIZATION_NVP(m_ordered_scrapped)
        & BOOST_SERIALIZATION_NVP(m_fighters)
        & BOOST_SERIALIZATION_NVP(m_missiles)
        & BOOST_SERIALIZATION_NVP(m_part_meters);
}

#endif // _Ship_h_
