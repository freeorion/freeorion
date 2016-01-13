// -*- C++ -*-
#ifndef _Ship_h_
#define _Ship_h_

#include "UniverseObject.h"
#include "Meter.h"

#include "../util/Export.h"

class ShipDesign;

/** a class representing a single FreeOrion ship */
class FO_COMMON_API Ship : public UniverseObject {
public:
    typedef std::map<std::pair<MeterType, std::string>, Meter>          PartMeterMap;

    /** \name Accessors */ //@{
    virtual std::set<std::string>
                                Tags() const;                                       ///< returns all tags this object has
    virtual bool                HasTag(const std::string& name) const;              ///< returns true iff this object has the tag with the indicated \a name

    virtual UniverseObjectType  ObjectType() const;
    virtual std::string         Dump() const;

    const ShipDesign*           Design() const;     ///< returns the design of the ship, containing engine type, weapons, etc.
    int                         DesignID() const            { return m_design_id; }             ///< returns the design id of the ship

    virtual int                 ContainerObjectID() const   { return m_fleet_id; }             ///< returns id of the object that directly contains this object, if any, or INVALID_OBJECT_ID if this object is not contained by any other
    virtual bool                ContainedBy(int object_id) const;                               ///< returns true if there is an object with id \a object_id that contains this UniverseObject
    int                         FleetID() const             { return m_fleet_id; }              ///< returns the ID of the fleet the ship is residing in

    int                         ProducedByEmpireID() const  { return m_produced_by_empire_id; } ///< returns the empire ID of the empire that produced this ship

    virtual const std::string&  PublicName(int empire_id) const;

    bool                        IsMonster() const;
    bool                        IsArmed() const;
    bool                        CanColonize() const;
    bool                        HasTroops() const;
    bool                        CanBombard() const;
    const std::string&          SpeciesName() const         { return m_species_name; }
    float                       Speed() const;
    float                       ColonyCapacity() const;
    float                       TroopCapacity() const;

    virtual TemporaryPtr<UniverseObject>
                                Accept(const UniverseObjectVisitor& visitor) const;

    virtual float               NextTurnCurrentMeterValue(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn

    bool                        OrderedScrapped() const         { return m_ordered_scrapped; }          ///< returns true iff this ship has been ordered scrapped, or false otherwise
    int                         OrderedColonizePlanet() const   { return m_ordered_colonize_planet_id; }///< returns the ID of the planet this ship has been ordered to colonize, or INVALID_OBJECT_ID if this ship hasn't been ordered to colonize a planet
    int                         OrderedInvadePlanet() const     { return m_ordered_invade_planet_id; }  ///< returns the ID of the planet this ship has been ordered to invade with ground troops, or INVALID_OBJECT_ID if this ship hasn't been ordered to invade a planet
    int                         OrderedBombardPlanet() const    { return m_ordered_bombard_planet_id; } ///< returns the ID of the planet this ship has been ordered to bombard, or INVALID_OBJECT_ID if this ship hasn't been ordered to bombard a planet
    int                         LastTurnActiveInCombat() const  { return m_last_turn_active_in_combat; }///< returns the last turn this ship has been actively involved in combat

    const PartMeterMap&         PartMeters() const { return m_part_meters; }                                ///< returns this Ship's part meters
    const Meter*                GetPartMeter(MeterType type, const std::string& part_name) const;           ///< returns the requested part Meter, or 0 if no such part Meter of that type is found in this ship for that part name
    float                       CurrentPartMeterValue(MeterType type, const std::string& part_name) const;  ///< returns current value of the specified part meter \a type for the specified part name
    float                       InitialPartMeterValue(MeterType type, const std::string& part_name) const;  ///< returns this turn's initial value for the speicified part meter \a type for the specified part name

    float                       TotalWeaponsDamage(float shield_DR = 0.0f, bool include_fighters = true) const; ///< versus an enemy with a given shields DR
    std::vector<float>          AllWeaponsDamage(float shield_DR = 0.0f, bool include_fighters = true) const;   ///< any nonzero weapons strengths after adjustment versus an enemy with a given shields DR
    std::vector<float>          AllWeaponsMaxDamage(float shield_DR = 0.0f, bool include_fighters = true) const;///< any nonzero weapons strengths, assuming the shpi has been refueled recently, after adjustment versus an enemy with a given shields DR
    //@}

    /** \name Mutators */ //@{
    virtual void    Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES);

    virtual void    BackPropegateMeters();                                      ///< back propegates part meters (which UniverseObject equivalent doesn't)

    void            SetFleetID(int fleet_id);                                   ///< sets the ID of the fleet the ship resides in

    void            Resupply();

    void            SetSpecies(const std::string& species_name);

    void            SetOrderedScrapped(bool b = true);                          ///< flags ship for scrapping
    void            SetColonizePlanet(int planet_id);                           ///< marks ship to colonize the indicated planet
    void            ClearColonizePlanet();                                      ///< marks ship to colonize no planets
    void            SetInvadePlanet(int planet_id);                             ///< marks ship to invade the indicated planet
    void            ClearInvadePlanet();                                        ///< marks ship to invade no planets
    void            SetBombardPlanet(int planet_id);                            ///< marks ship to bombard the indicated planet
    void            ClearBombardPlanet();                                       ///< marks ship to bombard no planets

    void            SetLastTurnActiveInCombat(int turn) { m_last_turn_active_in_combat = turn; } ///< sets the last turn this ship was actively involved in combat

    Meter*          GetPartMeter(MeterType type, const std::string& part_name); ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object

    virtual void    ResetTargetMaxUnpairedMeters();
    virtual void    ResetPairedActiveMeters();
    virtual void    SetShipMetersToMax();
    //@}

protected:
    friend class Universe;
    /** \name Structors */ //@{
    Ship();                                         ///< default ctor
    Ship(int empire_id, int design_id, const std::string& species_name,
         int produced_by_empire_id = ALL_EMPIRES);  ///< general ctor taking ship's empire and design id, species name and production empire id.

    template <class T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);
    template <class T> friend void boost::checked_delete(T* x);
    ~Ship() {}

    virtual Ship*   Clone(int empire_id = ALL_EMPIRES) const;   ///< returns new copy of this Ship
    //@}


private:
    virtual void    PopGrowthProductionResearchPhase();
    virtual void    ClampMeters();

    int             m_design_id;
    int             m_fleet_id;
    bool            m_ordered_scrapped;
    int             m_ordered_colonize_planet_id;
    int             m_ordered_invade_planet_id;
    int             m_ordered_bombard_planet_id;
    int             m_last_turn_active_in_combat;
    PartMeterMap    m_part_meters;
    std::string     m_species_name;
    int             m_produced_by_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

FO_COMMON_API std::string NewMonsterName();

#endif // _Ship_h_
