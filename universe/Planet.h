#ifndef _Planet_h_
#define _Planet_h_


#include "EnumsFwd.h"
#include "Meter.h"
#include "PopCenter.h"
#include "ResourceCenter.h"
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


/** a class representing a FreeOrion planet. */
class FO_COMMON_API Planet :
    public UniverseObject,
    public PopCenter,
    public ResourceCenter
{
public:
    [[nodiscard]] std::set<std::string>   Tags(const ScriptingContext& context) const override;
    [[nodiscard]] bool                    HasTag(const std::string& name, const ScriptingContext& context) const override;
    [[nodiscard]] UniverseObjectType      ObjectType() const override;

    [[nodiscard]] std::string             Dump(unsigned short ntabs = 0) const override;

    [[nodiscard]] int                     ContainerObjectID() const override;
    [[nodiscard]] const std::set<int>&    ContainedObjectIDs() const override;
    [[nodiscard]] bool                    Contains(int object_id) const override;
    [[nodiscard]] bool                    ContainedBy(int object_id) const override;

    [[nodiscard]] const Meter*            GetMeter(MeterType type) const override;

    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;

    [[nodiscard]] std::vector<std::string>AvailableFoci() const override;
    [[nodiscard]] const std::string&      FocusIcon(const std::string& focus_name) const override;

    [[nodiscard]] PlanetType          Type() const                        { return m_type; }
    [[nodiscard]] PlanetType          OriginalType() const                { return m_original_type; }
    [[nodiscard]] int                 DistanceFromOriginalType() const    { return TypeDifference(m_type, m_original_type); }
    [[nodiscard]] PlanetSize          Size() const                        { return m_size; }
    [[nodiscard]] int                 HabitableSize() const;

    [[nodiscard]] bool                HostileToEmpire(int empire_id, const EmpireManager& empires) const override;

    [[nodiscard]] PlanetEnvironment   EnvironmentForSpecies(const std::string& species_name = "") const;
    [[nodiscard]] PlanetType          NextBetterPlanetTypeForSpecies(const std::string& species_name = "") const;
    [[nodiscard]] PlanetType          NextCloserToOriginalPlanetType() const;
    [[nodiscard]] PlanetType          ClockwiseNextPlanetType() const;
    [[nodiscard]] PlanetType          CounterClockwiseNextPlanetType() const;
    [[nodiscard]] PlanetSize          NextLargerPlanetSize() const;
    [[nodiscard]] PlanetSize          NextSmallerPlanetSize() const;

    /** An orbital period is equal to a planets "year". A "year" is arbitrarily
      * defined to be 4 turns. */
    [[nodiscard]] float OrbitalPeriod() const;
    /** @returns an angle in radians. */
    [[nodiscard]] float InitialOrbitalPosition() const;
    /** @returns an angle in radians. */
    [[nodiscard]] float OrbitalPositionOnTurn(int turn) const;
    /** The rotational period represents a planets "day".  A "day" is
      * arbitrarily defined to be 1/360 of a "year", and 1/90 of a turn. */
    [[nodiscard]] float RotationalPeriod() const;
    /** @returns an angle in degree. */
    [[nodiscard]] float AxialTilt() const;

    [[nodiscard]] const std::set<int>& BuildingIDs() const    { return m_buildings; }

    [[nodiscard]] bool IsAboutToBeColonized() const           { return m_is_about_to_be_colonized; }
    [[nodiscard]] bool IsAboutToBeInvaded() const             { return m_is_about_to_be_invaded; }
    [[nodiscard]] bool IsAboutToBeBombarded() const           { return m_is_about_to_be_bombarded; }
    [[nodiscard]] int OrderedGivenToEmpire() const            { return m_ordered_given_to_empire_id; }
    [[nodiscard]] int LastTurnAttackedByShip() const          { return m_last_turn_attacked_by_ship; }
    [[nodiscard]] int LastTurnColonized() const               { return m_turn_last_colonized; }
    [[nodiscard]] int TurnsSinceColonization() const;
    [[nodiscard]] int LastTurnConquered() const               { return m_turn_last_conquered; }
    [[nodiscard]] int TurnsSinceLastConquered() const;

    [[nodiscard]] const std::string&  SurfaceTexture() const  { return m_surface_texture; }
    [[nodiscard]] std::string         CardinalSuffix() const; ///< returns a roman number representing this planets orbit in relation to other planets

    [[nodiscard]] std::map<int, double> EmpireGroundCombatForces() const;


    void Copy(std::shared_ptr<const UniverseObject> copied_object,
              const Universe& universe, int empire_id = ALL_EMPIRES) override;

    [[nodiscard]] Meter* GetMeter(MeterType type) override;

    void Reset() override;
    void Depopulate() override;
    void SetSpecies(std::string species_name) override;

    void SetType(PlanetType type);          ///< sets the type of this Planet to \a type
    void SetOriginalType(PlanetType type);  ///< sets the original type of this Planet to \a type
    void SetSize(PlanetSize size);          ///< sets the size of this Planet to \a size

    void SetRotationalPeriod(float days);   ///< sets the rotational period of this planet
    void SetHighAxialTilt();                ///< randomly generates a new, high axial tilt

    void AddBuilding(int building_id);      ///< adds the building to the planet
    bool RemoveBuilding(int building_id);   ///< removes the building from the planet; returns false if no such building was found

    void Conquer(int conquerer, EmpireManager& empires, Universe& universe);    ///< Called during combat when a planet changes hands
    bool Colonize(int empire_id, std::string species_name, double population);  ///< Called during colonization handling to do the actual colonizing
    void SetIsAboutToBeColonized(bool b);   ///< Called during colonization when a planet is about to be colonized
    void ResetIsAboutToBeColonized();       ///< Called after colonization, to reset the number of prospective colonizers to 0
    void SetIsAboutToBeInvaded(bool b);     ///< Marks planet as being invaded or not, depending on whether \a b is true or false
    void ResetIsAboutToBeInvaded();         ///< Marks planet as not being invaded
    void SetIsAboutToBeBombarded(bool b);   ///< Marks planet as being bombarded or not, depending on whether \a b is true or false
    void ResetIsAboutToBeBombarded();       ///< Marks planet as not being bombarded
    void SetGiveToEmpire(int empire_id);    ///< Marks planet to be given to empire
    void ClearGiveToEmpire();               ///< Marks planet not to be given to any empire

    void SetLastTurnAttackedByShip(int turn);///< Sets the last turn this planet was attacked by a ship
    void SetSurfaceTexture(const std::string& texture);
    void ResetTargetMaxUnpairedMeters() override;

    [[nodiscard]] static int TypeDifference(PlanetType type1, PlanetType type2);

    /** Given initial set of ground forces on planet, determine ground forces on
      * planet after a turn of ground combat. */
    static void ResolveGroundCombat(std::map<int, double>& empires_troops, const EmpireManager::DiploStatusMap& diplo_statuses);

    /** Create planet from @p type and @p size. */
    Planet(PlanetType type, PlanetSize size);
    Planet() = default;

    /** returns new copy of this Planet. */
    [[nodiscard]] Planet* Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

private:
    friend class ObjectMap;
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    void Init();

    void AddMeter(MeterType meter_type) override
    { UniverseObject::AddMeter(meter_type); }

    void PopGrowthProductionResearchPhase(ScriptingContext& context) override;

    void ClampMeters() override;

    PlanetType      m_type = PlanetType::PT_SWAMP;
    PlanetType      m_original_type = PlanetType::PT_SWAMP;
    PlanetSize      m_size = PlanetSize::SZ_TINY;
    float           m_orbital_period = 1.0f;
    float           m_initial_orbital_position = 0.0f;
    float           m_rotational_period = 1.0f;
    float           m_axial_tilt = 23.0f;

    std::set<int>   m_buildings;

    int             m_turn_last_colonized = INVALID_GAME_TURN;
    int             m_turn_last_conquered = INVALID_GAME_TURN;
    bool            m_is_about_to_be_colonized = false;
    bool            m_is_about_to_be_invaded = false;
    bool            m_is_about_to_be_bombarded = false;
    int             m_ordered_given_to_empire_id = ALL_EMPIRES;
    int             m_last_turn_attacked_by_ship = -1;

    std::string     m_surface_texture;  // intentionally not serialized; set by local effects

    template <typename Archive>
    friend void serialize(Archive&, Planet&, unsigned int const);
};


#endif
