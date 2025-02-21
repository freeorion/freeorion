#ifndef _Planet_h_
#define _Planet_h_


#include "EnumsFwd.h"
#include "Meter.h"
#include "UniverseObject.h"
#include "../Empire/EmpireManager.h"
#include "../util/Export.h"

//! Types of Planet%s
FO_ENUM(
    (PlanetType),
    ((INVALID_PLANET_TYPE, -1))
    ((PT_SWAMP))
    ((PT_TOXIC))
    ((PT_INFERNO))
    ((PT_RADIATED))
    ((PT_BARREN))
    ((PT_TUNDRA))
    ((PT_DESERT))
    ((PT_TERRAN))
    ((PT_OCEAN))
    ((PT_ASTEROIDS))
    ((PT_GASGIANT))
    ((NUM_PLANET_TYPES))
)


//! Sizes of Planet%s
FO_ENUM(
    (PlanetSize),
    ((INVALID_PLANET_SIZE, -1))
    ((SZ_NOWORLD))  //!< Used to designate an empty planet slot
    ((SZ_TINY))
    ((SZ_SMALL))
    ((SZ_MEDIUM))
    ((SZ_LARGE))
    ((SZ_HUGE))
    ((SZ_ASTEROIDS))
    ((SZ_GASGIANT))
    ((NUM_PLANET_SIZES))
)

[[nodiscard]] constexpr std::string_view DumpEnum(PlanetSize value) noexcept {
    switch (value) {
    case PlanetSize::SZ_TINY:      return "Tiny";
    case PlanetSize::SZ_SMALL:     return "Small";
    case PlanetSize::SZ_MEDIUM:    return "Medium";
    case PlanetSize::SZ_LARGE:     return "Large";
    case PlanetSize::SZ_HUGE:      return "Huge";
    case PlanetSize::SZ_ASTEROIDS: return "Asteroids";
    case PlanetSize::SZ_GASGIANT:  return "GasGiant";
    default:                       return "?";
    }
}

[[nodiscard]] constexpr std::string_view DumpEnum(PlanetType value) noexcept {
    switch (value) {
    case PlanetType::PT_SWAMP:      return "Swamp";
    case PlanetType::PT_TOXIC:      return "Toxic";
    case PlanetType::PT_INFERNO:    return "Inferno";
    case PlanetType::PT_RADIATED:   return "Radiated";
    case PlanetType::PT_BARREN:     return "Barren";
    case PlanetType::PT_TUNDRA:     return "Tundra";
    case PlanetType::PT_DESERT:     return "Desert";
    case PlanetType::PT_TERRAN:     return "Terran";
    case PlanetType::PT_OCEAN:      return "Ocean";
    case PlanetType::PT_ASTEROIDS:  return "Asteroids";
    case PlanetType::PT_GASGIANT:   return "GasGiant";
    default:            return "?";
    }
}


/** a class representing a FreeOrion planet. */
class FO_COMMON_API Planet final : public UniverseObject {
public:
    [[nodiscard]] TagVecs      Tags(const ScriptingContext& context) const override;
    [[nodiscard]] bool         HasTag(std::string_view name, const ScriptingContext& context) const override;

    [[nodiscard]] std::string  Dump(uint8_t ntabs = 0) const override;

    using UniverseObject::IDSet;
    [[nodiscard]] int          ContainerObjectID() const noexcept override { return this->SystemID(); }
    [[nodiscard]] const IDSet& ContainedObjectIDs() const noexcept override { return m_buildings; }
    [[nodiscard]] bool         Contains(int object_id) const override;
    [[nodiscard]] bool         ContainedBy(int object_id) const noexcept override;

    [[nodiscard]] const auto&                   Focus() const noexcept { return m_focus; }
    [[nodiscard]] int                           TurnsSinceFocusChange(int current_turn) const noexcept;
    [[nodiscard]] std::vector<std::string_view> AvailableFoci(const ScriptingContext& context) const;
    [[nodiscard]] bool                          FocusAvailable(std::string_view focus, const ScriptingContext& context) const;
    [[nodiscard]] const std::string&            FocusIcon(std::string_view focus_name, const ScriptingContext& context) const;

    [[nodiscard]] bool                Populated() const noexcept;
    [[nodiscard]] auto&               SpeciesName() const noexcept { return m_species_name; }

    [[nodiscard]] PlanetType          Type() const noexcept            { return m_type; }
    [[nodiscard]] PlanetType          OriginalType() const noexcept    { return m_original_type; }
    [[nodiscard]] int                 DistanceFromOriginalType() const noexcept { return TypeDifference(m_type, m_original_type); }
    [[nodiscard]] PlanetSize          Size() const noexcept            { return m_size; }
    [[nodiscard]] int                 HabitableSize() const;

    [[nodiscard]] bool                HostileToEmpire(int empire_id, const EmpireManager& empires) const override;

    [[nodiscard]] PlanetEnvironment   EnvironmentForSpecies(const SpeciesManager& sm,
                                                            std::string_view species_name = "") const;
    [[nodiscard]] PlanetType          NextBestPlanetTypeForSpecies(const ScriptingContext& context,
                                                                   const std::string& species_name = "") const;
    [[nodiscard]] PlanetType          NextBetterPlanetTypeForSpecies(const ScriptingContext& context,
                                                                     const std::string& species_name = "") const;
    [[nodiscard]] PlanetType          NextCloserToOriginalPlanetType() const noexcept;
    [[nodiscard]] PlanetType          ClockwiseNextPlanetType() const noexcept;
    [[nodiscard]] PlanetType          CounterClockwiseNextPlanetType() const noexcept;
    [[nodiscard]] PlanetSize          NextLargerPlanetSize() const noexcept;
    [[nodiscard]] PlanetSize          NextSmallerPlanetSize() const noexcept;

    /** An orbital period is equal to a planets "year". A "year" is arbitrarily
      * defined to be 4 turns. */
    [[nodiscard]] float OrbitalPeriod() const noexcept              { return m_orbital_period; }
    /** @returns an angle in radians. */
    [[nodiscard]] float InitialOrbitalPosition() const noexcept     { return m_initial_orbital_position; }
    /** @returns an angle in radians. */
    [[nodiscard]] float OrbitalPositionOnTurn(int turn) const noexcept;
    /** The rotational period represents a planets "day".  A "day" is
      * arbitrarily defined to be 1/360 of a "year", and 1/90 of a turn. */
    [[nodiscard]] float RotationalPeriod() const noexcept           { return m_rotational_period; }
    /** @returns an angle in degree. */
    [[nodiscard]] float AxialTilt() const noexcept                  { return m_axial_tilt; }

    [[nodiscard]] const auto& BuildingIDs() const noexcept          { return m_buildings; }

    [[nodiscard]] bool IsAboutToBeAnnexed() const noexcept          { return m_ordered_annexed_by_empire_id != ALL_EMPIRES; }
    [[nodiscard]] bool IsAboutToBeColonized() const noexcept        { return m_is_about_to_be_colonized; }
    [[nodiscard]] bool IsAboutToBeInvaded() const noexcept          { return m_is_about_to_be_invaded; }
    [[nodiscard]] bool IsAboutToBeBombarded() const noexcept        { return m_is_about_to_be_bombarded; }
    [[nodiscard]] int OrderedAnnexedByEmpire() const noexcept       { return m_ordered_annexed_by_empire_id; }
    [[nodiscard]] int LastAnnexedByEmpire() const noexcept          { return m_last_annexed_by_empire_id; }
    [[nodiscard]] double AnnexationCost(int empire_id, const ScriptingContext& context) const;
    [[nodiscard]] int OrderedGivenToEmpire() const noexcept         { return m_ordered_given_to_empire_id; }
    [[nodiscard]] int LastTurnAttackedByShip() const noexcept       { return m_last_turn_attacked_by_ship; }
    [[nodiscard]] int LastTurnColonized() const noexcept            { return m_turn_last_colonized; }
    [[nodiscard]] int TurnsSinceColonization(int current_turn) const noexcept;
    [[nodiscard]] int LastColonizedByEmpire() const                 { return m_last_colonized_by_empire_id; }
    [[nodiscard]] int LastTurnConquered() const noexcept            { return m_turn_last_conquered; }
    [[nodiscard]] int TurnsSinceLastConquered(int current_turn) const noexcept;
    [[nodiscard]] int OwnerBeforeLastConquered() const noexcept     { return m_owner_before_last_conquered; }
    [[nodiscard]] int LastInvadedByEmpire() const noexcept          { return m_last_invaded_by_empire_id; }
    [[nodiscard]] int LastTurnAnnexed() const noexcept              { return m_turn_last_annexed; }
    [[nodiscard]] int TurnsSinceLastAnnexed(int current_turn) const noexcept;

    [[nodiscard]] const auto& SurfaceTexture() const noexcept       { return m_surface_texture; }
    [[nodiscard]] std::string CardinalSuffix(const ObjectMap& objects) const; ///< returns a roman number representing this planets orbit in relation to other planets

    [[nodiscard]] std::map<int, double> EmpireGroundCombatForces() const;

    [[nodiscard]] std::size_t           SizeInMemory() const override;

    void Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id = ALL_EMPIRES) override;
    void Copy(const Planet& copied_planet, const Universe& universe, int empire_id = ALL_EMPIRES);

    void Reset(ObjectMap& objects);

    void Depopulate(int current_turn);
    void SetSpecies(std::string species_name, int turn, const SpeciesManager& sm);

    void SetFocus(std::string focus, const ScriptingContext& context);
    void ClearFocus(int current_turn);
    void UpdateFocusHistory();

    void SetType(PlanetType type);          ///< sets the type of this Planet to \a type
    void SetOriginalType(PlanetType type);  ///< sets the original type of this Planet to \a type
    void SetSize(PlanetSize size);          ///< sets the size of this Planet to \a size

    void SetRotationalPeriod(float days);   ///< sets the rotational period of this planet
    void SetHighAxialTilt();                ///< randomly generates a new, high axial tilt

    void AddBuilding(int building_id);      ///< adds the building to the planet
    bool RemoveBuilding(int building_id);   ///< removes the building from the planet; returns false if no such building was found

    void Conquer(int conquerer, ScriptingContext& context); ///< Called during combat when a planet changes hands
    bool Colonize(int empire_id, std::string species_name,  ///< Called during colonization handling to do the actual colonizing
                  double population, ScriptingContext& context);
    void SetIsAboutToBeColonized(bool b);   ///< Called during colonization when a planet is about to be colonized
    void ResetIsAboutToBeColonized();       ///< Called after colonization, to reset the number of prospective colonizers to 0
    void SetLastColonizedByEmpire(int id);  ///< Records the empire (or no empire) that most recently colonized this planet.
    void SetTurnLastColonized(int turn);    ///< Sets the last turn this planet was colonized
    void SetIsAboutToBeInvaded(bool b);     ///< Marks planet as being invaded or not, depending on whether \a b is true or false
    void SetLastInvadedByEmpire(int id);    ///< Records the empire (or no empire) that most recently invaded a planet.
    void ResetIsAboutToBeInvaded();         ///< Marks planet as not being invaded
    void SetIsOrderAnnexedByEmpire(int empire_id);
    void ResetBeingAnnxed();
    void SetLastAnnexedByEmpire(int id);    ///< Records the empire (or no empire) that most recently annexed this planet.
    void SetIsAboutToBeBombarded(bool b);   ///< Marks planet as being bombarded or not, depending on whether \a b is true or false
    void ResetIsAboutToBeBombarded();       ///< Marks planet as not being bombarded
    void SetGiveToEmpire(int empire_id);    ///< Marks planet to be given to empire
    void ClearGiveToEmpire();               ///< Marks planet not to be given to any empire

    void SetLastTurnAttackedByShip(int turn) noexcept;///< Sets the last turn this planet was attacked by a ship
    void SetLastTurnAnnexed(int turn) noexcept;
    void SetSurfaceTexture(const std::string& texture);
    void ResetTargetMaxUnpairedMeters() override;

    [[nodiscard]] static int TypeDifference(PlanetType type1, PlanetType type2) noexcept;

    /** Given initial set of ground forces on planet, determine ground forces on
      * planet after a turn of ground combat. */
    static void ResolveGroundCombat(std::map<int, double>& empires_troops,
                                    const DiploStatusMap& diplo_statuses);

    /** Create planet from @p type and @p size. */
    Planet(PlanetType type, PlanetSize size, int creation_turn);
    Planet() : UniverseObject(UniverseObjectType::OBJ_PLANET) { AddMeters(planet_meter_types); }

    /** returns new copy of this Planet. */
    [[nodiscard]] std::shared_ptr<UniverseObject> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

    mutable boost::signals2::signal<void ()> ResourceCenterChangedSignal;

private:
    friend class ObjectMap;
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    static constexpr auto planet_meter_types = []() {
        using MT = MeterType;
        auto retval = std::array{
            MT::METER_POPULATION,   MT::METER_TARGET_POPULATION, MT::METER_HAPPINESS,       MT::METER_TARGET_HAPPINESS,
            MT::METER_INDUSTRY,     MT::METER_RESEARCH,          MT::METER_INFLUENCE,
            MT::METER_CONSTRUCTION, MT::METER_TARGET_INDUSTRY,   MT::METER_TARGET_RESEARCH, MT::METER_TARGET_INFLUENCE, MT::METER_TARGET_CONSTRUCTION,
            MT::METER_SUPPLY,       MT::METER_MAX_SUPPLY,        MT::METER_STOCKPILE,       MT::METER_MAX_STOCKPILE,    MT::METER_SHIELD,       MT::METER_MAX_SHIELD,
            MT::METER_DEFENSE,      MT::METER_MAX_DEFENSE,       MT::METER_TROOPS,          MT::METER_MAX_TROOPS,       MT::METER_DETECTION,
            MT::METER_REBEL_TROOPS
        };
#if defined(__cpp_lib_constexpr_algorithms)
        std::sort(retval.begin(), retval.end());
#endif
        return retval;
    }();

    void PopGrowthProductionResearchPhase(ScriptingContext& context) override;

    void ClampMeters() override;

    std::string m_species_name;

    std::string m_focus;
    int         m_last_turn_focus_changed = INVALID_GAME_TURN;
    std::string m_focus_turn_initial;
    int         m_last_turn_focus_changed_turn_initial = INVALID_GAME_TURN;

    PlanetType  m_type = PlanetType::PT_SWAMP;
    PlanetType  m_original_type = PlanetType::PT_SWAMP;
    PlanetSize  m_size = PlanetSize::SZ_TINY;
    float       m_orbital_period = 1.0f;
    float       m_initial_orbital_position = 0.0f;
    float       m_rotational_period = 1.0f;
    float       m_axial_tilt = 23.0f;

    IDSet       m_buildings;

    int         m_last_annexed_by_empire_id = ALL_EMPIRES;
    int         m_owner_before_last_conquered = ALL_EMPIRES;
    int         m_last_invaded_by_empire_id = ALL_EMPIRES;
    int         m_last_colonized_by_empire_id = ALL_EMPIRES;

    int         m_turn_last_annexed = INVALID_GAME_TURN;
    int         m_turn_last_colonized = INVALID_GAME_TURN;
    int         m_turn_last_conquered = INVALID_GAME_TURN;
    int         m_ordered_given_to_empire_id = ALL_EMPIRES;
    int         m_last_turn_attacked_by_ship = -1;

    std::string m_surface_texture;  // intentionally not serialized; set by local effects

    int         m_ordered_annexed_by_empire_id = ALL_EMPIRES;

    bool        m_is_about_to_be_colonized = false;
    bool        m_is_about_to_be_invaded = false;
    bool        m_is_about_to_be_bombarded = false;

    template <typename Archive>
    friend void serialize(Archive&, Planet&, unsigned int const);
};


#endif
