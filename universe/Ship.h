#ifndef _Ship_h_
#define _Ship_h_

#include "UniverseObject.h"
#include "Meter.h"

#include "../util/Export.h"

FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int INVALID_DESIGN_ID;
FO_COMMON_API extern const int INVALID_GAME_TURN;
FO_COMMON_API extern const int BEFORE_FIRST_TURN;
FO_COMMON_API extern const int INVALID_OBJECT_ID;
class ShipDesign;

/** a class representing a single FreeOrion ship */
class FO_COMMON_API Ship : public UniverseObject {
public:
    typedef std::map<std::pair<MeterType, std::string>, Meter>          PartMeterMap;

    /** \name Accessors */ //@{
    bool HostileToEmpire(int empire_id) const override;
    std::set<std::string> Tags() const override;
    bool HasTag(const std::string& name) const override;
    UniverseObjectType ObjectType() const override;
    std::string Dump(unsigned short ntabs = 0) const override;

    int ContainerObjectID() const override
    { return m_fleet_id; }

    bool ContainedBy(int object_id) const override;
    const std::string& PublicName(int empire_id) const override;
    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;

    /** Back propagates part meters (which UniverseObject equivalent doesn't). */
    void BackPropagateMeters() override;

    void ResetTargetMaxUnpairedMeters() override;
    void ResetPairedActiveMeters() override;
    void ClampMeters() override;

    /** Returns new copy of this Ship. */
    Ship* Clone(int empire_id = ALL_EMPIRES) const override;

    void Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES) override;

    const ShipDesign*           Design() const;     ///< returns the design of the ship, containing engine type, weapons, etc.
    int                         DesignID() const            { return m_design_id; }             ///< returns the design id of the ship

    int                         FleetID() const             { return m_fleet_id; }              ///< returns the ID of the fleet the ship is residing in

    int                         ProducedByEmpireID() const  { return m_produced_by_empire_id; } ///< returns the empire ID of the empire that produced this ship
    int                         ArrivedOnTurn() const       { return m_arrived_on_turn; }       ///< returns the turn on which this ship arrived in its current system
    int                         LastResuppliedOnTurn() const{ return m_last_resupplied_on_turn;}///< returns the turn on which this ship was last resupplied / upgraded

    bool                        IsMonster() const;
    bool                        IsArmed() const;
    bool                        HasFighters() const;
    bool                        CanColonize() const;
    bool                        HasTroops() const;
    bool                        CanHaveTroops() const;
    bool                        CanBombard() const;
    const std::string&          SpeciesName() const         { return m_species_name; }
    float                       Speed() const;
    float                       ColonyCapacity() const;
    float                       TroopCapacity() const;

    bool                        OrderedScrapped() const         { return m_ordered_scrapped; }          ///< returns true iff this ship has been ordered scrapped, or false otherwise
    int                         OrderedColonizePlanet() const   { return m_ordered_colonize_planet_id; }///< returns the ID of the planet this ship has been ordered to colonize, or INVALID_OBJECT_ID if this ship hasn't been ordered to colonize a planet
    int                         OrderedInvadePlanet() const     { return m_ordered_invade_planet_id; }  ///< returns the ID of the planet this ship has been ordered to invade with ground troops, or INVALID_OBJECT_ID if this ship hasn't been ordered to invade a planet
    int                         OrderedBombardPlanet() const    { return m_ordered_bombard_planet_id; } ///< returns the ID of the planet this ship has been ordered to bombard, or INVALID_OBJECT_ID if this ship hasn't been ordered to bombard a planet
    int                         LastTurnActiveInCombat() const  { return m_last_turn_active_in_combat; }///< returns the last turn this ship has been actively involved in combat

    const PartMeterMap&         PartMeters() const { return m_part_meters; }                                ///< returns this Ship's part meters
    const Meter*                GetPartMeter(MeterType type, const std::string& part_name) const;           ///< returns the requested part Meter, or 0 if no such part Meter of that type is found in this ship for that part name
    float                       CurrentPartMeterValue(MeterType type, const std::string& part_name) const;  ///< returns current value of the specified part meter \a type for the specified part name
    float                       InitialPartMeterValue(MeterType type, const std::string& part_name) const;  ///< returns this turn's initial value for the speicified part meter \a type for the specified part name

    /** Returns sum of current value for part meter @p type of all parts with ShipPartClass @p part_class */
    float                       SumCurrentPartMeterValuesForPartClass(MeterType type, ShipPartClass part_class) const;

    float                       TotalWeaponsDamage(float shield_DR = 0.0f, bool include_fighters = true) const; ///< versus an enemy with a given shields DR
    float                       FighterCount() const;
    float                       FighterMax() const;
    std::vector<float>          AllWeaponsDamage(float shield_DR = 0.0f, bool include_fighters = true) const;   ///< any nonzero weapons strengths after adjustment versus an enemy with a given shields DR
    std::vector<float>          AllWeaponsMaxDamage(float shield_DR = 0.0f, bool include_fighters = true) const;///< any nonzero weapons strengths, assuming the shpi has been refueled recently, after adjustment versus an enemy with a given shields DR
    //@}

    /** \name Mutators */ //@{
    void            SetFleetID(int fleet_id);                                   ///< sets the ID of the fleet the ship resides in
    void            SetArrivedOnTurn(int turn);

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

    virtual void    SetShipMetersToMax();
    //@}

protected:
    friend class Universe;

    /** \name Structors */ //@{
    Ship();

public:
    /** Create a ship from an @p empire_id, @p design_id, @p species_name and
        @p production_by_empire_id. */
    Ship(int empire_id, int design_id, const std::string& species_name,
         int produced_by_empire_id = ALL_EMPIRES);

protected:
    template <class T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

public:
    ~Ship() {}
    //@}

private:
    int             m_design_id = INVALID_DESIGN_ID;
    int             m_fleet_id = INVALID_OBJECT_ID;
    bool            m_ordered_scrapped = false;
    int             m_ordered_colonize_planet_id = INVALID_OBJECT_ID;
    int             m_ordered_invade_planet_id = INVALID_OBJECT_ID;
    int             m_ordered_bombard_planet_id = INVALID_OBJECT_ID;
    int             m_last_turn_active_in_combat = INVALID_GAME_TURN;
    PartMeterMap    m_part_meters;
    std::string     m_species_name;
    int             m_produced_by_empire_id = ALL_EMPIRES;
    int             m_arrived_on_turn = INVALID_GAME_TURN;
    int             m_last_resupplied_on_turn = BEFORE_FIRST_TURN;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

FO_COMMON_API std::string NewMonsterName();

#endif // _Ship_h_
