#ifndef _System_h_
#define _System_h_


#include <map>
#include "UniverseObject.h"
#include "../util/Enum.h"
#include "../util/Export.h"


class Fleet;
class ObjectMap;

//! Types of stars
FO_ENUM(
    (StarType),
    ((INVALID_STAR_TYPE, -1))
    ((STAR_BLUE))
    ((STAR_WHITE))
    ((STAR_YELLOW))
    ((STAR_ORANGE))
    ((STAR_RED))
    ((STAR_NEUTRON))
    ((STAR_BLACK))
    ((STAR_NONE))
    ((NUM_STAR_TYPES))
)

[[nodiscard]] constexpr std::string_view DumpEnum(StarType value) noexcept {
    switch (value) {
    case StarType::STAR_BLUE:    return "Blue";
    case StarType::STAR_WHITE:   return "White";
    case StarType::STAR_YELLOW:  return "Yellow";
    case StarType::STAR_ORANGE:  return "Orange";
    case StarType::STAR_RED:     return "Red";
    case StarType::STAR_NEUTRON: return "Neutron";
    case StarType::STAR_BLACK:   return "BlackHole";
    case StarType::STAR_NONE:    return "NoStar";
    default:                     return "Unknown";
    }
}


/** contains UniverseObjects and connections to other systems (starlanes and
   wormholes).  All systems are UniversObjects contained within the universe,
   and (nearly) all systems also contain UniverseObjects.  Systems are
   searchable using arbirary predicates, much in the same way that Universes
   are.  Each System consists of a star and 0 or more orbits.  UniversObjects
   can be placed in the system "at large" or in a particular orbit.  No
   checking is done to determine whether an object is orbit-bound (like a
   Planet) or not (like a Fleet) when placing them with the Insert()
   functions.  Iteration is available over all starlanes and wormholes
   (together), all system objects, all free system objects (those not in an
   orbit), and all objects in a paricular orbit.*/
class FO_COMMON_API System final : public UniverseObject {
public:
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    using UniverseObject::IDSet;
    [[nodiscard]] const IDSet& ContainedObjectIDs() const noexcept override { return m_objects; }

    [[nodiscard]] bool Contains(int object_id) const override;

    [[nodiscard]] bool ContainedBy(int object_id) const noexcept override { return false; }

    /** returns the name to display for players for this system.  While all
      * systems may have a proper name assigned, if they contain no planets or
      * star, then "Deep Space" (or its translation)  Or, a system that is not
      * yet explored might be "Unexplored Region", rather than an empty string
      * for the name.  This is distinct from PublicName functions, which filter
      * the name based on ownership. */
    [[nodiscard]] std::string   ApparentName(int empire_id, const Universe& u, bool blank_unexplored_and_none = false) const;

    [[nodiscard]] StarType      GetStarType() const noexcept { return m_star; }  ///< type of star for this system
    [[nodiscard]] StarType      NextOlderStarType() const noexcept;
    [[nodiscard]] StarType      NextYoungerStarType() const noexcept;

    [[nodiscard]] auto          Orbits() const noexcept       { return m_orbits.size(); }     ///< number of orbits in this system

    [[nodiscard]] int           NumStarlanes() const noexcept { return static_cast<int>(m_starlanes.size()); }; ///< number of starlanes from this system to other systems
    [[nodiscard]] bool          HasStarlaneTo(int id) const; ///< true if there is a starlane from this system to the system with ID number \a id

    [[nodiscard]] auto&         ObjectIDs() const noexcept        { return m_objects; }
    [[nodiscard]] auto&         PlanetIDs() const noexcept        { return m_planets; }
    [[nodiscard]] auto&         BuildingIDs() const noexcept      { return m_buildings; }
    [[nodiscard]] auto&         FleetIDs() const noexcept         { return m_fleets; }
    [[nodiscard]] auto&         ShipIDs() const noexcept          { return m_ships; }
    [[nodiscard]] auto&         FieldIDs() const noexcept         { return m_fields; }
    [[nodiscard]] auto&         PlanetIDsByOrbit() const noexcept { return m_orbits; }

    [[nodiscard]] int           PlanetInOrbit(int orbit) const;             ///< returns the ID of the planet in the specified \a orbit, or INVALID_OBJECT_ID if there is no planet in that orbit or it is an invalid orbit
    [[nodiscard]] int           OrbitOfPlanet(int object_id) const;         ///< returns the orbit ID in which the planet with \a object_id is located, or -1 the specified ID is not a planet in an orbit of this system
    [[nodiscard]] bool          OrbitOccupied(int orbit) const;             ///< returns true if there is an object in \a orbit
    [[nodiscard]] std::set<int> FreeOrbits() const;                         ///< returns the set of orbit numbers that are unoccupied

    [[nodiscard]] auto&         Starlanes() const noexcept { return m_starlanes; }
    [[nodiscard]] IDSet         VisibleStarlanes(int empire_id, const Universe& universe) const;

    [[nodiscard]] int           LastTurnBattleHere() const noexcept { return m_last_turn_battle_here; }

    [[nodiscard]] const auto&   OverlayTexture() const noexcept     { return m_overlay_texture; }
    [[nodiscard]] double        OverlaySize() const noexcept        { return m_overlay_size; }  ///< size in universe units

    [[nodiscard]] std::size_t   SizeInMemory() const override;


    /** fleets are inserted into system */
    mutable boost::signals2::signal<void (std::vector<int>, const ObjectMap&)> FleetsInsertedSignal;
    /** fleets are removed from system */
    mutable boost::signals2::signal<void (std::vector<int>)> FleetsRemovedSignal;

    void Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id = ALL_EMPIRES) override;
    void Copy(const System& copied_system, const Universe& universe, int empire_id = ALL_EMPIRES);

    void SetID(int id) override {
        this->m_system_id = id;
        UniverseObject::SetID(id);
    }

    void SetOwner(int id) override {} // no-op for systems

    void ResetTargetMaxUnpairedMeters() override;

    /** adds an object to this system. */
    static constexpr int NO_ORBIT = -1;
    void Insert(std::shared_ptr<UniverseObject> obj, int orbit, int current_turn, const ObjectMap& objects);
    void Insert(UniverseObject* obj, int orbit, int current_turn, const ObjectMap& objects);

    /** removes the object with ID number \a id from this system. */
    void Remove(int id);

    void SetStarType(StarType type);     ///< sets the type of the star in this Systems to \a StarType
    void AddStarlane(int id);            ///< adds a starlane between this system and the system with ID number \a id.  \note Adding a starlane to a system to which there is already a wormhole erases the wormhole; you may want to check for a wormhole before calling this function.
    bool RemoveStarlane(int id);         ///< removes a starlane between this system and the system with ID number \a id.  Returns false if there was no starlane from this system to system \a id.

    void SetLastTurnBattleHere(int turn) noexcept { m_last_turn_battle_here = turn; }

    void SetOverlayTexture(const std::string& texture, double size);

    System(StarType star, std::string name, double x, double y, int current_turn);
    System() : UniverseObject(UniverseObjectType::OBJ_SYSTEM) {}

private:
    template <typename T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);

    /** Returns new copy of this System. */
    [[nodiscard]] std::shared_ptr<UniverseObject> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

    static constexpr int SYSTEM_ORBITS = 7;

    StarType            m_star = StarType::INVALID_STAR_TYPE;
    std::vector<int>    m_orbits = std::vector<int>(SYSTEM_ORBITS, INVALID_OBJECT_ID);  ///< indexed by orbit number, indicates the id of the planet in that orbit
    IDSet               m_objects;
    IDSet               m_planets;
    IDSet               m_buildings;
    IDSet               m_fleets;
    IDSet               m_ships;
    IDSet               m_fields;
    IDSet               m_starlanes; ///< IDs of other connected systems
    int                 m_last_turn_battle_here = INVALID_GAME_TURN;  ///< the turn on which there was last a battle in this system

    std::string         m_overlay_texture;          // intentionally not serialized; set by local effects
    double              m_overlay_size = 1.0;

    friend ObjectMap;

    template <typename Archive>
    friend void serialize(Archive&, System&, unsigned int const);
};


#endif
