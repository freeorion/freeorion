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
    Ship(int empire_id, int design_id, const std::string& species_name,
         int produced_by_empire_id = ALL_EMPIRES);  ///< general ctor taking ship's empire and design id, species name and production empire id.

    virtual Ship*               Clone(int empire_id = ALL_EMPIRES) const;   ///< returns new copy of this Ship
    //@}

    /** \name Accessors */ //@{
    virtual const std::string&  TypeName() const;           ///< returns user-readable string indicating the type of UniverseObject this is

    const ShipDesign*           Design() const;             ///< returns the design of the ship, containing engine type, weapons, etc.
    int                         DesignID() const;           ///< returns the design id of the ship
    int                         FleetID() const;            ///< returns the ID of the fleet the ship is residing in
    int                         ProducedByEmpireID() const; ///< returns the empire ID of the empire that produced this ship

    virtual const std::string&  PublicName(int empire_id) const;
    virtual std::string         Dump() const;

    bool                        IsMonster() const;
    bool                        IsArmed() const;
    bool                        CanColonize() const;
    const std::string&          SpeciesName() const;
    double                      Speed() const;

    const ConsumablesMap&       Fighters() const;
    const ConsumablesMap&       Missiles() const;

    virtual UniverseObject*     Accept(const UniverseObjectVisitor& visitor) const;

    virtual double              NextTurnCurrentMeterValue(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn

    bool                        OrderedScrapped() const;        ///< returns true iff this ship has been ordered scrapped, or false otherwise
    int                         OrderedColonizePlanet() const;  ///< returns the ID of the planet this ship has been ordered to colonize, or INVALID_OBJECT_ID if this ship hasn't been ordered to colonize a planet

    const Meter*                GetMeter(MeterType type, const std::string& part_name) const; ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object
    //@}

    /** \name Mutators */ //@{
    virtual void    Copy(const UniverseObject* copied_object, int empire_id = ALL_EMPIRES);

    void            SetFleetID(int fleet_id);                               ///< sets the ID of the fleet the ship resides in

    void            Resupply();

    void            AddFighters(const std::string& part_name, std::size_t n);
    void            RemoveFighters(const std::string& part_name, std::size_t n);
    void            RemoveMissiles(const std::string& part_name, std::size_t n);

    void            SetSpecies(const std::string& species_name);

    virtual void    MoveTo(double x, double y);

    void            SetOrderedScrapped(bool b = true);                      ///< flags ship for scrapping
    void            SetColonizePlanet(int planet_id);                       ///< marks ship to colonize the indicated planet
    void            ClearColonizePlanet();                                  ///< marks ship to colonize no planets

    Meter*          GetMeter(MeterType type, const std::string& part_name); ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object
    //@}

private:
    typedef std::map<std::pair<MeterType, std::string>, Meter> PartMeters;

    virtual void    ResetTargetMaxUnpairedMeters(MeterType meter_type = INVALID_METER_TYPE);
    virtual void    ClampMeters();

    int             m_design_id;
    int             m_fleet_id;
    bool            m_ordered_scrapped;
    int             m_ordered_colonize_planet_id;
    ConsumablesMap  m_fighters;
    ConsumablesMap  m_missiles;
    PartMeters      m_part_meters;
    std::string     m_species_name;
    int             m_produced_by_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _Ship_h_
