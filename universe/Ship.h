#ifndef _Ship_h_
#define _Ship_h_


#include "Meter.h"
#include "ConstantsFwd.h"
#include "ScriptingContext.h"
#include "UniverseObject.h"
#include "../util/Export.h"

class ShipDesign;
class ShipPart;

/** a class representing a single FreeOrion ship */
class FO_COMMON_API Ship final : public UniverseObject {
public:
    struct string_metertype_pair_less {
        template <typename S1, typename S2>
        bool operator()(const std::pair<S1, MeterType>& lhs, const std::pair<S2, MeterType>& rhs) const noexcept
        { return (lhs.first < rhs.first) || ((lhs.first == rhs.first) && (lhs.second < rhs.second)); }

        using is_transparent = int;
    };
    using PartMeterMap = boost::container::flat_map<std::pair<std::string, MeterType>, Meter, string_metertype_pair_less>;

    [[nodiscard]] bool HostileToEmpire(int empire_id, const EmpireManager& empires) const override;

    [[nodiscard]] TagVecs     Tags(const ScriptingContext& context) const override;
    [[nodiscard]] bool        HasTag(std::string_view name, const ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    [[nodiscard]] int ContainerObjectID() const noexcept override { return m_fleet_id; }
    [[nodiscard]] bool ContainedBy(int object_id) const noexcept override;

    [[nodiscard]] const std::string& PublicName(int empire_id, const Universe& universe) const override;
    [[nodiscard]] const std::string& PublicName(int empire_id) const;

    /** Back propagates part meters (which UniverseObject equivalent doesn't). */
    void BackPropagateMeters() noexcept override;

    void ResetTargetMaxUnpairedMeters() override;
    void ResetPairedActiveMeters() override;
    void ClampMeters() override;

    /** Returns new copy of this Ship. */
    [[nodiscard]] std::shared_ptr<UniverseObject> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

    void Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id = ALL_EMPIRES) override;
    void Copy(const Ship& copied_ship, const Universe& universe, int empire_id = ALL_EMPIRES);

    [[nodiscard]] int   DesignID() const noexcept             { return m_design_id; }             ///< returns the design id of the ship
    [[nodiscard]] int   FleetID() const noexcept              { return m_fleet_id; }              ///< returns the ID of the fleet the ship is residing in
    [[nodiscard]] int   ProducedByEmpireID() const noexcept   { return m_produced_by_empire_id; } ///< returns the empire ID of the empire that produced this ship
    [[nodiscard]] int   ArrivedOnTurn() const noexcept        { return m_arrived_on_turn; }       ///< returns the turn on which this ship arrived in its current system
    [[nodiscard]] int   LastResuppliedOnTurn() const noexcept { return m_last_resupplied_on_turn;}///< returns the turn on which this ship was last resupplied / upgraded
    [[nodiscard]] bool  IsMonster(const Universe& universe) const;
    [[nodiscard]] bool  CanDamageShips(const ScriptingContext& context, float target_shields = 0.0f) const;
    [[nodiscard]] bool  CanDestroyFighters(const ScriptingContext& context) const;
    [[nodiscard]] bool  IsArmed(const ScriptingContext& context) const;
    [[nodiscard]] bool  HasFighters(const Universe& universe) const;
    [[nodiscard]] bool  CanColonize(const Universe& universe, const SpeciesManager& sm) const;
    [[nodiscard]] bool  HasTroops(const Universe& universe) const;
    [[nodiscard]] bool  CanHaveTroops(const Universe& universe) const;
    [[nodiscard]] bool  CanBombard(const Universe& universe) const;
    [[nodiscard]] auto& SpeciesName() const noexcept { return m_species_name; }
    [[nodiscard]] float Speed() const;
    [[nodiscard]] float ColonyCapacity(const Universe& universe) const;
    [[nodiscard]] float TroopCapacity(const Universe& universe) const;

    [[nodiscard]] bool  OrderedScrapped() const noexcept        { return m_ordered_scrapped; }          ///< returns true iff this ship has been ordered scrapped, or false otherwise
    [[nodiscard]] int   OrderedColonizePlanet() const noexcept  { return m_ordered_colonize_planet_id; }///< returns the ID of the planet this ship has been ordered to colonize, or INVALID_OBJECT_ID if this ship hasn't been ordered to colonize a planet
    [[nodiscard]] int   OrderedInvadePlanet() const noexcept    { return m_ordered_invade_planet_id; }  ///< returns the ID of the planet this ship has been ordered to invade with ground troops, or INVALID_OBJECT_ID if this ship hasn't been ordered to invade a planet
    [[nodiscard]] int   OrderedBombardPlanet() const noexcept   { return m_ordered_bombard_planet_id; } ///< returns the ID of the planet this ship has been ordered to bombard, or INVALID_OBJECT_ID if this ship hasn't been ordered to bombard a planet
    [[nodiscard]] int   LastTurnActiveInCombat() const noexcept { return m_last_turn_active_in_combat; }///< returns the last turn this ship has been actively involved in combat

    [[nodiscard]] const auto&  PartMeters() const noexcept      { return m_part_meters; }                  ///< returns this Ship's part meters
    [[nodiscard]] const Meter* GetPartMeter(MeterType type, const std::string& part_name) const;           ///< returns the requested part Meter, or 0 if no such part Meter of that type is found in this ship for that part name
    [[nodiscard]] float        CurrentPartMeterValue(MeterType type, const std::string& part_name) const;  ///< returns current value of the specified part meter \a type for the specified part name
    [[nodiscard]] float        InitialPartMeterValue(MeterType type, const std::string& part_name) const;  ///< returns this turn's initial value for the specified part meter \a type for the specified part name

    /** Returns sum of current value for part meter @p type of all parts with ShipPartClass @p part_class */
    [[nodiscard]] float SumCurrentPartMeterValuesForPartClass(MeterType type, ShipPartClass part_class, const Universe& universe) const;

    [[nodiscard]] float WeaponPartFighterDamage(const ShipPart* part, const ScriptingContext& context) const; ///< versus fighter enemies
    [[nodiscard]] float WeaponPartShipDamage(const ShipPart* part, const ScriptingContext& context) const; ///< versus an enemy context.effect_target ship with a given shields meter
    [[nodiscard]] float TotalWeaponsFighterDamage(const ScriptingContext& context, bool include_fighters = true) const; ///< versus an fighter enemy
    [[nodiscard]] float TotalWeaponsShipDamage(const ScriptingContext& context, float shield_DR = 0.0f, bool include_fighters = true) const; ///< versus an enemy ship with a given shields DR
    [[nodiscard]] float FighterCount() const;
    [[nodiscard]] float FighterMax() const;

    [[nodiscard]] std::vector<float> AllWeaponsFighterDamage(const ScriptingContext& context, bool include_fighters = true) const;   ///< any shots against enemy fighters
    /** returns any nonzero weapons strengths after adjustment versus an enemy with a given @p shield_DR shield rating,
      * uses the normal meters so it might be lower than AllWeaponsMaxShipDamage
      * if e.g. the ship has less than a full complement of fighters */
    [[nodiscard]] std::vector<float> AllWeaponsShipDamage(const ScriptingContext& context, float shield_DR = 0.0f, bool include_fighters = true) const;
    /** returns any nonzero weapons strengths after adjustment versus an enemy with a given @p shield_DR shield rating,
      * assuming the ship has been resupplied recently (i.e. this uses Max*Meters) */
    [[nodiscard]] std::vector<float> AllWeaponsMaxShipDamage(const ScriptingContext& context, float shield_DR = 0.0f, bool include_fighters = true) const;

    [[nodiscard]] std::size_t        SizeInMemory() const override;

    void SetFleetID(int fleet_id); ///< sets the ID of the fleet the ship resides in
    void SetArrivedOnTurn(int turn);
    void Resupply(int turn);
    void SetSpecies(std::string species_name, const SpeciesManager& sm);
    void SetOrderedScrapped(bool b = true); ///< flags ship for scrapping
    void SetColonizePlanet(int planet_id);  ///< marks ship to colonize the indicated planet
    void ClearColonizePlanet();             ///< marks ship to colonize no planets
    void SetInvadePlanet(int planet_id);    ///< marks ship to invade the indicated planet
    void ClearInvadePlanet();               ///< marks ship to invade no planets
    void SetBombardPlanet(int planet_id);   ///< marks ship to bombard the indicated planet
    void ClearBombardPlanet();              ///< marks ship to bombard no planets
    void SetLastTurnActiveInCombat(int turn) noexcept { m_last_turn_active_in_combat = turn; } ///< sets the last turn this ship was actively involved in combat

    [[nodiscard]] Meter* GetPartMeter(MeterType type, const std::string& part_name); ///< returns the requested Meter, or 0 if no such Meter of that type is found in this object

    virtual void SetShipMetersToMax();

    /** Create a ship from an @p empire_id, @p design_id, @p species_name and
        @p production_by_empire_id. */
    Ship(int empire_id, int design_id, std::string species_name, const Universe& universe,
         const SpeciesManager& species, int produced_by_empire_id, int current_turn);
    Ship() : UniverseObject(UniverseObjectType::OBJ_SHIP) { AddMeters(ship_meter_types); }
    Ship(Ship&&) = default;

private:
    friend class Universe;
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    static constexpr auto ship_meter_types = []() {
        using MT = MeterType;
        auto retval = std::array{
            MT::METER_FUEL,            MT::METER_MAX_FUEL,        MT::METER_SHIELD,     MT::METER_MAX_SHIELD,
            MT::METER_DETECTION,       MT::METER_STRUCTURE,       MT::METER_MAX_STRUCTURE,
            MT::METER_SPEED,           MT::METER_TARGET_INDUSTRY, MT::METER_INDUSTRY,
            MT::METER_TARGET_RESEARCH, MT::METER_RESEARCH,        MT::METER_TARGET_INFLUENCE,
            MT::METER_INFLUENCE
        };
#if defined(__cpp_lib_constexpr_algorithms)
        std::sort(retval.begin(), retval.end());
#endif
        return retval;
    }();

    PartMeterMap    m_part_meters;
    std::string     m_species_name;
    int             m_design_id = INVALID_DESIGN_ID;
    int             m_fleet_id = INVALID_OBJECT_ID;
    int             m_ordered_colonize_planet_id = INVALID_OBJECT_ID;
    int             m_ordered_invade_planet_id = INVALID_OBJECT_ID;
    int             m_ordered_bombard_planet_id = INVALID_OBJECT_ID;
    int             m_last_turn_active_in_combat = INVALID_GAME_TURN;
    int             m_produced_by_empire_id = ALL_EMPIRES;
    int             m_arrived_on_turn = INVALID_GAME_TURN;
    int             m_last_resupplied_on_turn = BEFORE_FIRST_TURN;
    bool            m_ordered_scrapped = false;

    template <typename Archive>
    friend void serialize(Archive&, Ship&, unsigned int const);
};

FO_COMMON_API std::string NewMonsterName();


#endif
