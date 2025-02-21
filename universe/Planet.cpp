#include "Planet.h"

#include "BuildingType.h"
#include "Building.h"
#include "Condition.h"
#include "Fleet.h"
#include "Ship.h"
#include "Species.h"
#include "System.h"
#include "Universe.h"
#include "ValueRef.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/SitRepEntry.h"
#include "../util/i18n.h"


namespace {
    // high tilt is arbitrarily taken to mean 45 degrees or more
    constexpr float HIGH_TILT_THERESHOLD = 45.0f;
    constexpr double MINIMUM_POP_CENTER_POPULATION = 0.01001;  // rounds up to 0.1 when showing 2 digits, down to 0.05 or 50.0 when showing 3

    constexpr float SizeRotationFactor(PlanetSize size) noexcept {
        switch (size) {
        case PlanetSize::SZ_TINY:     return 1.5f;
        case PlanetSize::SZ_SMALL:    return 1.25f;
        case PlanetSize::SZ_MEDIUM:   return 1.0f;
        case PlanetSize::SZ_LARGE:    return 0.75f;
        case PlanetSize::SZ_HUGE:     return 0.5f;
        case PlanetSize::SZ_GASGIANT: return 0.25f;
        default:                      return 1.0f;
        }
    }

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif

    /** @content_tag{CTRL_STAT_SKIP_DEPOP} Do not count Planets with this tag for SpeciesPlanetsDepoped stat */
    constexpr std::string_view TAG_STAT_SKIP_DEPOP = "CTRL_STAT_SKIP_DEPOP";
}

////////////////////////////////////////////////////////////
// Planet
////////////////////////////////////////////////////////////
Planet::Planet(PlanetType type, PlanetSize size, int creation_turn) :
    UniverseObject{UniverseObjectType::OBJ_PLANET, "", ALL_EMPIRES, creation_turn},
    m_type(type),
    m_original_type(type),
    m_size(size),
    m_initial_orbital_position(RandZeroToOne() * 2 * 3.14159f),
    m_axial_tilt(RandZeroToOne() * HIGH_TILT_THERESHOLD)
{
    //DebugLogger() << "Planet::Planet(" << type << ", " << size <<")";
    AddMeters(planet_meter_types);

    static constexpr double SPIN_STD_DEV = 0.1;
    static constexpr double REVERSE_SPIN_CHANCE = 0.06;
    m_rotational_period = RandGaussian(1.0, SPIN_STD_DEV) / SizeRotationFactor(m_size);
    if (RandZeroToOne() < REVERSE_SPIN_CHANCE)
        m_rotational_period = -m_rotational_period;
}

std::shared_ptr<UniverseObject> Planet::Clone(const Universe& universe, int empire_id) const {
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_shared<Planet>();
    retval->Copy(*this, universe, empire_id);
    return retval;
}

void Planet::Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id) {
    if (&copied_object == this)
        return;

    if (copied_object.ObjectType() != UniverseObjectType::OBJ_PLANET) {
        ErrorLogger() << "Planet::Copy passed an object that wasn't a Planet";
        return;
    }

    Copy(static_cast<const Planet&>(copied_object), universe, empire_id);
}

void Planet::Copy(const Planet& copied_planet, const Universe& universe, int empire_id) {
    if (&copied_planet == this)
        return;

    const int copied_object_id = copied_planet.ID();
    const Visibility vis = empire_id == ALL_EMPIRES ?
        Visibility::VIS_FULL_VISIBILITY : universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    const auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_planet, vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_name =                      copied_planet.m_name;

        this->m_buildings =                 copied_planet.VisibleContainedObjectIDs(empire_id, universe.GetEmpireObjectVisibility());
        this->m_type =                      copied_planet.m_type;
        this->m_original_type =             copied_planet.m_original_type;
        this->m_size =                      copied_planet.m_size;
        this->m_orbital_period =            copied_planet.m_orbital_period;
        this->m_initial_orbital_position =  copied_planet.m_initial_orbital_position;
        this->m_rotational_period =         copied_planet.m_rotational_period;
        this->m_axial_tilt =                copied_planet.m_axial_tilt;
        this->m_turn_last_conquered =       copied_planet.m_turn_last_conquered;
        this->m_turn_last_colonized =       copied_planet.m_turn_last_colonized;
        this->m_turn_last_annexed =         copied_planet.m_turn_last_annexed;

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            this->m_species_name =                          copied_planet.m_species_name;
            this->m_focus =                                 copied_planet.m_focus;
            this->m_last_turn_focus_changed =               copied_planet.m_last_turn_focus_changed;
            this->m_focus_turn_initial =                    copied_planet.m_focus_turn_initial;
            this->m_last_turn_focus_changed_turn_initial =  copied_planet.m_last_turn_focus_changed_turn_initial;
            this->m_last_turn_attacked_by_ship =            copied_planet.m_last_turn_attacked_by_ship;
            this->m_ordered_annexed_by_empire_id =          copied_planet.m_ordered_annexed_by_empire_id;
            this->m_is_about_to_be_colonized =              copied_planet.m_is_about_to_be_colonized;
            this->m_is_about_to_be_invaded =                copied_planet.m_is_about_to_be_invaded;
            this->m_is_about_to_be_bombarded =              copied_planet.m_is_about_to_be_bombarded;
            this->m_owner_before_last_conquered =           copied_planet.m_owner_before_last_conquered;
            this->m_last_invaded_by_empire_id =             copied_planet.m_last_invaded_by_empire_id;
            this->m_last_colonized_by_empire_id =           copied_planet.m_last_colonized_by_empire_id;
            this->m_last_annexed_by_empire_id =             copied_planet.m_last_annexed_by_empire_id;

            if (vis >= Visibility::VIS_FULL_VISIBILITY) {
                this->m_ordered_given_to_empire_id = copied_planet.m_ordered_given_to_empire_id;

            } else {
                // copy system name if at partial visibility, as it won't be copied
                // by UniverseObject::Copy unless at full visibility, but players
                // should know planet names even if they don't own the planet
                m_name = copied_planet.Name();
            }
        }
    }
}

bool Planet::HostileToEmpire(int empire_id, const EmpireManager& empires) const {
    if (OwnedBy(empire_id))
        return false;

    // Empire owned planets are hostile to ALL_EMPIRES
    if (empire_id == ALL_EMPIRES)
        return !Unowned();

    // Unowned planets are only considered hostile if populated
    auto pop_meter = UniverseObject::GetMeter(MeterType::METER_TARGET_POPULATION);
    if (Unowned())
        return pop_meter && (pop_meter->Current() != 0.0f);

    // both empires are normal empires
    return empires.GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

UniverseObject::TagVecs Planet::Tags(const ScriptingContext& context) const {
    const Species* species = context.species.GetSpecies(SpeciesName());
    return species ? TagVecs{species->Tags()} : TagVecs{};
}

bool Planet::HasTag(std::string_view name, const ScriptingContext& context) const {
    const Species* species = context.species.GetSpecies(SpeciesName());
    return species && species->HasTag(name);
}

std::string Planet::Dump(uint8_t ntabs) const {
    std::string retval = UniverseObject::Dump(ntabs);
    retval.reserve(2048);
    retval.append(" species: ").append(m_species_name).append("  ");
    retval.append(" focus: ").append(m_focus).append(" last changed on turn: ")
          .append(std::to_string(m_last_turn_focus_changed));
    retval.append(" type: ").append(to_string(m_type))
          .append(" original type: ").append(to_string(m_original_type))
          .append(" size: ").append(to_string(m_size))
          .append(" rot period: ").append(std::to_string(m_rotational_period))
          .append(" axis tilt: ").append(std::to_string(m_axial_tilt))
          .append(" buildings: ");
    for (auto it = m_buildings.begin(); it != m_buildings.end();) {
        int building_id = *it;
        ++it;
        retval.append(std::to_string(building_id)).append(it == m_buildings.end() ? "" : ", ");
    }
    if (m_ordered_annexed_by_empire_id != ALL_EMPIRES)
        retval.append(" (About to be Annexed by ").append(std::to_string(m_ordered_annexed_by_empire_id)).append(")");
    if (m_is_about_to_be_colonized)
        retval.append(" (About to be Colonized)");
    if (m_is_about_to_be_invaded)
        retval.append(" (About to be Invaded)");

    retval.append(" annexed on turn: ").append(std::to_string(m_turn_last_annexed))
          .append(" colonized on turn: ").append(std::to_string(m_turn_last_colonized))
          .append(" conquered on turn: ").append(std::to_string(m_turn_last_conquered))
          .append(" owner before being conquered: ").append(std::to_string(m_owner_before_last_conquered))
          .append(" last invaded by: ").append(std::to_string(m_last_invaded_by_empire_id))
          .append(" last colonized by: ").append(std::to_string(m_last_colonized_by_empire_id))
          .append(" last annexed by: ").append(std::to_string(m_last_annexed_by_empire_id));

    if (m_is_about_to_be_bombarded)
        retval.append(" (About to be Bombarded)");

    if (m_ordered_given_to_empire_id != ALL_EMPIRES)
        retval.append(" (Ordered to be given to empire with id: ")
              .append(std::to_string(m_ordered_given_to_empire_id)).append(")");

    retval.append(" last attacked on turn: ").append(std::to_string(m_last_turn_attacked_by_ship));

    return retval;
}

bool Planet::Populated() const noexcept
{ return UniverseObject::GetMeter(MeterType::METER_POPULATION)->Current() >= MINIMUM_POP_CENTER_POPULATION; }

int Planet::HabitableSize() const {
    auto& gr = GetGameRules();
    switch (m_size) {
    case PlanetSize::SZ_GASGIANT:  return gr.Get<int>("RULE_HABITABLE_SIZE_GASGIANT");  break;
    case PlanetSize::SZ_HUGE:      return gr.Get<int>("RULE_HABITABLE_SIZE_HUGE");      break;
    case PlanetSize::SZ_LARGE:     return gr.Get<int>("RULE_HABITABLE_SIZE_LARGE");     break;
    case PlanetSize::SZ_MEDIUM:    return gr.Get<int>("RULE_HABITABLE_SIZE_MEDIUM");    break;
    case PlanetSize::SZ_ASTEROIDS: return gr.Get<int>("RULE_HABITABLE_SIZE_ASTEROIDS"); break;
    case PlanetSize::SZ_SMALL:     return gr.Get<int>("RULE_HABITABLE_SIZE_SMALL");     break;
    case PlanetSize::SZ_TINY:      return gr.Get<int>("RULE_HABITABLE_SIZE_TINY");      break;
    default:                       return 0;                                            break;
    }
}

int Planet::TurnsSinceFocusChange(int current_turn) const noexcept {
    if (m_last_turn_focus_changed == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    if (current_turn == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    return current_turn - m_last_turn_focus_changed;
}

PlanetEnvironment Planet::EnvironmentForSpecies(const SpeciesManager& sm, std::string_view species_name) const {
    const Species* species = nullptr;
    if (species_name.empty()) {
        auto& this_planet_species_name = this->SpeciesName();
        if (this_planet_species_name.empty())
            return PlanetEnvironment::PE_UNINHABITABLE;
        species = sm.GetSpecies(this_planet_species_name);
    } else {
        species = sm.GetSpecies(species_name);
    }
    if (!species) {
        ErrorLogger() << "Planet::EnvironmentForSpecies couldn't get species with name \"" << species_name << "\"";
        return PlanetEnvironment::PE_UNINHABITABLE;
    }
    return species->GetPlanetEnvironment(m_type);
}

PlanetType Planet::NextBestPlanetTypeForSpecies(const ScriptingContext& context,
                                                const std::string& species_name) const
{
    const Species* species = nullptr;
    if (species_name.empty()) {
        const std::string& this_planet_species_name = this->SpeciesName();
        if (this_planet_species_name.empty())
            return m_type;
        species = context.species.GetSpecies(this_planet_species_name);
    } else {
        species = context.species.GetSpecies(species_name);
    }
    if (!species) {
        ErrorLogger() << "Planet::NextBestPlanetTypeForSpecies couldn't get species with name \"" << species_name << "\"";
        return m_type;
    }
    return species->NextBestPlanetType(m_type);
}

PlanetType Planet::NextBetterPlanetTypeForSpecies(const ScriptingContext& context,
                                                  const std::string& species_name) const
{
    const Species* species = nullptr;
    if (species_name.empty()) {
        const std::string& this_planet_species_name = this->SpeciesName();
        if (this_planet_species_name.empty())
            return m_type;
        species = context.species.GetSpecies(this_planet_species_name);
    } else {
        species = context.species.GetSpecies(species_name);
    }
    if (!species) {
        ErrorLogger() << "Planet::NextBetterPlanetTypeForSpecies couldn't get species with name \"" << species_name << "\"";
        return m_type;
    }
    return species->NextBetterPlanetType(m_type);
}

namespace {
    constexpr PlanetType RingNextPlanetType(PlanetType current_type) noexcept {
        switch (current_type) {
        case PlanetType::INVALID_PLANET_TYPE:
        case PlanetType::PT_ASTEROIDS:
        case PlanetType::PT_GASGIANT:
        case PlanetType::NUM_PLANET_TYPES:
            return current_type; break;
        case PlanetType::PT_OCEAN: return PlanetType::PT_SWAMP; break;
        default: return PlanetType(int(current_type)+1);
        }
    }
    constexpr PlanetType RingPreviousPlanetType(PlanetType current_type) noexcept {
        switch (current_type) {
        case PlanetType::INVALID_PLANET_TYPE:
        case PlanetType::PT_ASTEROIDS:
        case PlanetType::PT_GASGIANT:
        case PlanetType::NUM_PLANET_TYPES:
            return current_type; break;
        case PlanetType::PT_SWAMP: return PlanetType::PT_OCEAN; break;
        default: return PlanetType(int(current_type)-1);
        }
    }
    static_assert(RingNextPlanetType(PlanetType::INVALID_PLANET_TYPE) == PlanetType::INVALID_PLANET_TYPE);
    static_assert(RingNextPlanetType(PlanetType::PT_ASTEROIDS) == PlanetType::PT_ASTEROIDS);
    static_assert(RingNextPlanetType(PlanetType::PT_SWAMP) == PlanetType::PT_TOXIC);
    static_assert(RingNextPlanetType(PlanetType::PT_OCEAN) == PlanetType::PT_SWAMP);
    static_assert(RingPreviousPlanetType(PlanetType::INVALID_PLANET_TYPE) == PlanetType::INVALID_PLANET_TYPE);
    static_assert(RingPreviousPlanetType(PlanetType::PT_GASGIANT) == PlanetType::PT_GASGIANT);
    static_assert(RingPreviousPlanetType(PlanetType::PT_SWAMP) == PlanetType::PT_OCEAN);
    static_assert(RingPreviousPlanetType(PlanetType::PT_OCEAN) == PlanetType::PT_TERRAN);

    constexpr PlanetType NextCloserTo(PlanetType from, PlanetType towards) noexcept {
        switch (from) {
        case PlanetType::INVALID_PLANET_TYPE:
        case PlanetType::PT_ASTEROIDS:
        case PlanetType::PT_GASGIANT:
        case PlanetType::NUM_PLANET_TYPES:
            return from;
            break;
        default:
            break;
        }
        switch (towards) {
        case PlanetType::INVALID_PLANET_TYPE:
        case PlanetType::PT_ASTEROIDS:
        case PlanetType::PT_GASGIANT:
        case PlanetType::NUM_PLANET_TYPES:
            return from;
            break;
        default:
            break;
        }

        if (from == towards)
            return from;

        const int cw_steps = [towards, cur_type{from}]() mutable noexcept {
            int cw_steps = 0;
            while (cur_type != towards) {
                cw_steps++;
                cur_type = RingNextPlanetType(cur_type);
            }
            return cw_steps;
        }();

        const int ccw_steps = [towards, cur_type{from}]() mutable noexcept {
            int ccw_steps = 0;
            while (cur_type != towards) {
                ccw_steps++;
                cur_type = RingPreviousPlanetType(cur_type);
            }
            return ccw_steps;
        }();

        return (cw_steps <= ccw_steps) ? RingNextPlanetType(from) : RingPreviousPlanetType(from);
    }
    static_assert(NextCloserTo(PlanetType::INVALID_PLANET_TYPE, PlanetType::PT_OCEAN) == PlanetType::INVALID_PLANET_TYPE);
    static_assert(NextCloserTo(PlanetType::PT_OCEAN, PlanetType::PT_GASGIANT) == PlanetType::PT_OCEAN);
    static_assert(NextCloserTo(PlanetType::PT_OCEAN, PlanetType::PT_OCEAN) == PlanetType::PT_OCEAN);
    static_assert(NextCloserTo(PlanetType::PT_OCEAN, PlanetType::PT_TOXIC) == PlanetType::PT_SWAMP);
    static_assert(NextCloserTo(PlanetType::PT_SWAMP, PlanetType::PT_TOXIC) == PlanetType::PT_TOXIC);
    static_assert(NextCloserTo(PlanetType::PT_INFERNO, PlanetType::PT_OCEAN) == PlanetType::PT_TOXIC);
    static_assert(NextCloserTo(PlanetType::PT_INFERNO, PlanetType::PT_DESERT) == PlanetType::PT_RADIATED);
}

PlanetType Planet::NextCloserToOriginalPlanetType() const noexcept
{ return NextCloserTo(m_type, m_original_type); }

namespace {
    constexpr PlanetType LoopPlanetTypeIncrement(PlanetType initial_type, int step) noexcept {
        // avoid too large steps that would mess up enum arithmatic
        const int absstep = step >= 0 ? step : -step;
        if (absstep >= int(PlanetType::PT_ASTEROIDS))
            return initial_type;
        // some types can't be terraformed
        if (initial_type == PlanetType::PT_GASGIANT)
            return PlanetType::PT_GASGIANT;
        if (initial_type == PlanetType::PT_ASTEROIDS)
            return PlanetType::PT_ASTEROIDS;
        if (initial_type == PlanetType::INVALID_PLANET_TYPE)
            return PlanetType::INVALID_PLANET_TYPE;
        if (initial_type == PlanetType::NUM_PLANET_TYPES)
            return PlanetType::NUM_PLANET_TYPES;
        // calculate next planet type, accounting for loop arounds
        PlanetType new_type(PlanetType(int(initial_type) + int(step)));
        if (new_type >= PlanetType::PT_ASTEROIDS)
            new_type = PlanetType(int(new_type) - int(PlanetType::PT_ASTEROIDS));
        else if (new_type <= PlanetType::INVALID_PLANET_TYPE)
            new_type = PlanetType(int(new_type) + int(PlanetType::PT_ASTEROIDS));
        return new_type;
    }

    constexpr int PlanetTypeDifference(PlanetType type1, PlanetType type2) noexcept {
        // no distance defined for invalid types
        if (type1 == PlanetType::INVALID_PLANET_TYPE || type2 == PlanetType::INVALID_PLANET_TYPE)
            return 0;
        // if the same, distance is zero
        if (type1 == type2)
            return 0;
        // no distance defined for asteroids or gas giants with anything else
        if (type1 == PlanetType::PT_ASTEROIDS || type1 == PlanetType::PT_GASGIANT ||
            type2 == PlanetType::PT_ASTEROIDS || type2 == PlanetType::PT_GASGIANT)
        { return 0; }
        // find distance around loop:
        //
        //  0  1  2
        //  8     3
        //  7     4
        //    6 5
        int sdiff = int(type1) - int(type2);
        int diff = sdiff >= 0 ? sdiff : -sdiff;
        // raw_dist -> actual dist
        //  0 to 4       0 to 4
        //  5 to 8       4 to 1
        if (diff > 4)
            diff = 9 - diff;
        //std::cout << "typedifference type1: " << int(type1) << "  type2: " << int(type2) << "  diff: " << diff << "\n";
        return diff;
    }

    constexpr PlanetSize PlanetSizeIncrement(PlanetSize initial_size, int step) noexcept {
        // some sizes don't have meaningful increments
        if (initial_size == PlanetSize::SZ_GASGIANT)
            return PlanetSize::SZ_GASGIANT;
        if (initial_size == PlanetSize::SZ_ASTEROIDS)
            return PlanetSize::SZ_ASTEROIDS;
        if (initial_size == PlanetSize::SZ_NOWORLD)
            return PlanetSize::SZ_NOWORLD;
        if (initial_size == PlanetSize::INVALID_PLANET_SIZE)
            return PlanetSize::INVALID_PLANET_SIZE;
        if (initial_size == PlanetSize::NUM_PLANET_SIZES)
            return PlanetSize::NUM_PLANET_SIZES;
        // calculate next planet size
        PlanetSize new_type(PlanetSize(int(initial_size) + int(step)));
        if (new_type >= PlanetSize::SZ_HUGE)
            return PlanetSize::SZ_HUGE;
        if (new_type <= PlanetSize::SZ_TINY)
            return PlanetSize::SZ_TINY;
        return new_type;
    }
}

PlanetType Planet::ClockwiseNextPlanetType() const noexcept
{ return LoopPlanetTypeIncrement(m_type, 1); }

PlanetType Planet::CounterClockwiseNextPlanetType() const noexcept
{ return LoopPlanetTypeIncrement(m_type, -1); }

int Planet::TypeDifference(PlanetType type1, PlanetType type2) noexcept
{ return PlanetTypeDifference(type1, type2); }

PlanetSize Planet::NextLargerPlanetSize() const noexcept
{ return PlanetSizeIncrement(m_size, 1); }

PlanetSize Planet::NextSmallerPlanetSize() const noexcept
{ return PlanetSizeIncrement(m_size, -1); }

float Planet::OrbitalPositionOnTurn(int turn) const noexcept
{ return m_initial_orbital_position + OrbitalPeriod() * 2.0 * 3.1415926 / 4 * turn; }

std::string Planet::CardinalSuffix(const ObjectMap& objects) const {
    std::string retval;
    // Early return for invalid ID
    if (ID() == INVALID_OBJECT_ID) {
        WarnLogger() << "Planet " << Name() << " has invalid ID";
        return retval;
    }

    auto cur_system = objects.get<System>(SystemID());
    // Early return for no system
    if (!cur_system) {
        ErrorLogger() << "Planet " << Name() << "(" << ID()
                      << ") not assigned to a system";
        return retval;
    }

    // Early return for unknown orbit
    if (cur_system->OrbitOfPlanet(ID()) < 0) {
        WarnLogger() << "Planet " << Name() << "(" << ID() << ") "
                     << "has no current orbit";
        retval.append(RomanNumber(1));
        return retval;
    }

    int num_planets_lteq = 0;  // number of planets at this orbit or smaller
    int num_planets_total = 0;
    bool prior_current_planet = true;

    for (int sys_orbit : cur_system->PlanetIDsByOrbit()) {
        if (sys_orbit == INVALID_OBJECT_ID)
            continue;

        // all other planets are in further orbits
        if (sys_orbit == ID()) {
            prior_current_planet = false;
            ++num_planets_total;
            ++num_planets_lteq;
            continue;
        }

        PlanetType other_planet_type = objects.get<Planet>(sys_orbit)->Type();
        if (other_planet_type == PlanetType::INVALID_PLANET_TYPE)
            continue;

        // only increment suffix for non-asteroid planets
        if (Type() != PlanetType::PT_ASTEROIDS) {
            if (other_planet_type != PlanetType::PT_ASTEROIDS) {
                ++num_planets_total;
                if (prior_current_planet)
                    ++num_planets_lteq;
            }
        } else {
            // unless the planet being named is an asteroid
            // then only increment suffix for asteroid planets
            if (other_planet_type == PlanetType::PT_ASTEROIDS) {
                ++num_planets_total;
                if (prior_current_planet)
                    ++num_planets_lteq;
            }
        }
    }

    // Planets are grouped into asteroids, and non-asteroids
    if (Type() != PlanetType::PT_ASTEROIDS) {
        retval.append(RomanNumber(num_planets_lteq));
    } else {
        // Asteroids receive a localized prefix
        retval.append(UserString("NEW_ASTEROIDS_SUFFIX"));
        // If no other asteroids in this system, do not append an ordinal
        if (num_planets_total > 1)
            retval.append(" " + RomanNumber(num_planets_lteq));
    }
    return retval;
}

bool Planet::Contains(int object_id) const
{ return object_id != INVALID_OBJECT_ID && m_buildings.contains(object_id); }

bool Planet::ContainedBy(int object_id) const noexcept
{ return object_id != INVALID_OBJECT_ID && this->SystemID() == object_id; }

bool Planet::FocusAvailable(std::string_view focus, const ScriptingContext& context) const {
    const auto* species = context.species.GetSpecies(this->SpeciesName());
    if (!species)
        return false;
    const auto& foci = species->Foci();
    const auto it = std::find_if(foci.begin(), foci.end(),
                                 [focus](const FocusType& focus_type) noexcept { return focus_type.Name() == focus; });
    if (it == foci.end())
        return false;
    const auto* location = it->Location();
    if (!location)
        return false;

    const ScriptingContext planet_context(context, ScriptingContext::Source{}, this);
    return location->EvalOne(planet_context, this);
}

std::vector<std::string_view> Planet::AvailableFoci(const ScriptingContext& context) const {
    std::vector<std::string_view> retval;
    const auto* species = context.species.GetSpecies(this->SpeciesName());
    if (!species)
        return retval;

    const ScriptingContext planet_context(context, ScriptingContext::Source{}, this);

    const auto& foci = species->Foci();
    retval.reserve(species->Foci().size());
    for (const auto& focus_type : foci) {
        if (const auto* location = focus_type.Location()) {
            if (location->EvalOne(planet_context, this))
                retval.emplace_back(focus_type.Name());
        }
    }

    return retval;
}

const std::string& Planet::FocusIcon(std::string_view focus_name, const ScriptingContext& context) const {
    if (const Species* species = context.species.GetSpecies(this->SpeciesName())) {
        for (const FocusType& focus_type : species->Foci()) {
            if (focus_type.Name() == focus_name)
                return focus_type.Graphic();
        }
    }
    return EMPTY_STRING;
}

std::map<int, double> Planet::EmpireGroundCombatForces() const {
    std::map<int, double> empire_troops;
    if (UniverseObject::GetMeter(MeterType::METER_TROOPS)->Initial() > 0.0f) {
        // empires may have garrisons on planets
        empire_troops[Owner()] += UniverseObject::GetMeter(MeterType::METER_TROOPS)->Initial() + 0.0001; // small bonus to ensure ties are won by initial owner
    }
    if (!Unowned() && UniverseObject::GetMeter(MeterType::METER_REBEL_TROOPS)->Initial() > 0.0f) {
        // rebels may be present on empire-owned planets
        empire_troops[ALL_EMPIRES] += UniverseObject::GetMeter(MeterType::METER_REBEL_TROOPS)->Initial();
    }
    return empire_troops;
}

std::size_t Planet::SizeInMemory() const {
    std::size_t retval = UniverseObject::SizeInMemory();
    retval += sizeof(Planet) - sizeof(UniverseObject);

    retval += sizeof(decltype(m_species_name)::value_type)*m_species_name.capacity();
    retval += sizeof(decltype(m_focus)::value_type)*m_focus.capacity();
    retval += sizeof(decltype(m_focus_turn_initial)::value_type)*m_focus_turn_initial.capacity();
    retval += sizeof(decltype(m_buildings)::value_type)*m_buildings.capacity();
    retval += sizeof(decltype(m_surface_texture)::value_type)*m_surface_texture.capacity();

    return retval;
}

double Planet::AnnexationCost(int empire_id, const ScriptingContext& context) const {
    if (m_species_name.empty())
        return 0.0;
    const auto* species = context.species.GetSpecies(m_species_name);
    if (!species)
        return 0.0;
    const auto* ac = species->AnnexationCost();
    if (!ac)
        return 0.0;
    if (ac->ConstantExpr())
        return ac->Eval();

    const auto* source_for_empire = context.Empires().GetSource(empire_id, context.ContextObjects()).get();
    ScriptingContext source_planet_context{context, ScriptingContext::Source{}, source_for_empire};
    source_planet_context.condition_local_candidate = this;
    if (!source_planet_context.condition_root_candidate)
        source_planet_context.condition_root_candidate = this;
    return ac->Eval(source_planet_context);
}

int Planet::TurnsSinceColonization(int current_turn) const noexcept {
    if (m_turn_last_colonized == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    if (current_turn == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    return current_turn - m_turn_last_colonized;
}

int Planet::TurnsSinceLastConquered(int current_turn) const noexcept {
    if (m_turn_last_conquered == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    if (current_turn == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    return current_turn - m_turn_last_conquered;
}

int Planet::TurnsSinceLastAnnexed(int current_turn) const noexcept {
    if (m_turn_last_annexed == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    if (current_turn == INVALID_GAME_TURN)
        return current_turn - BEFORE_FIRST_TURN;
    return current_turn - m_turn_last_annexed;
}

void Planet::SetType(PlanetType type) {
    if (type <= PlanetType::INVALID_PLANET_TYPE)
        type = PlanetType::PT_SWAMP;
    if (PlanetType::NUM_PLANET_TYPES <= type)
        type = PlanetType::PT_GASGIANT;
    m_type = type;
    StateChangedSignal();
}

void Planet::SetOriginalType(PlanetType type) {
    if (type <= PlanetType::INVALID_PLANET_TYPE)
        type = PlanetType::PT_SWAMP;
    if (PlanetType::NUM_PLANET_TYPES <= type)
        type = PlanetType::PT_GASGIANT;
    m_original_type = type;
    StateChangedSignal();
}

void Planet::SetSize(PlanetSize size) {
    if (size <= PlanetSize::SZ_NOWORLD)
        size = PlanetSize::SZ_TINY;
    if (PlanetSize::NUM_PLANET_SIZES <= size)
        size = PlanetSize::SZ_GASGIANT;
    m_size = size;
    StateChangedSignal();
}

void Planet::SetRotationalPeriod(float days)
{ m_rotational_period = days; }

void Planet::SetHighAxialTilt() {
    static constexpr double MAX_TILT = 90.0;
    m_axial_tilt = HIGH_TILT_THERESHOLD + RandZeroToOne() * (MAX_TILT - HIGH_TILT_THERESHOLD);
}

void Planet::AddBuilding(int building_id) {
    auto buildings_size = m_buildings.size();
    m_buildings.insert(building_id);
    if (buildings_size != m_buildings.size())
        StateChangedSignal();
    // expect calling code to set building's planet
}

bool Planet::RemoveBuilding(int building_id) {
    if (m_buildings.contains(building_id)) {
        m_buildings.erase(building_id);
        StateChangedSignal();
        return true;
    }
    return false;
}

void Planet::Reset(ObjectMap& objects) {
    GetMeter(MeterType::METER_POPULATION)->Reset();
    GetMeter(MeterType::METER_TARGET_POPULATION)->Reset();
    GetMeter(MeterType::METER_HAPPINESS)->Reset();
    GetMeter(MeterType::METER_TARGET_HAPPINESS)->Reset();
    m_species_name.clear();

    m_focus.clear();
    m_last_turn_focus_changed = INVALID_GAME_TURN;

    GetMeter(MeterType::METER_INDUSTRY)->Reset();
    GetMeter(MeterType::METER_RESEARCH)->Reset();
    GetMeter(MeterType::METER_INFLUENCE)->Reset();
    GetMeter(MeterType::METER_CONSTRUCTION)->Reset();

    GetMeter(MeterType::METER_TARGET_INDUSTRY)->Reset();
    GetMeter(MeterType::METER_TARGET_RESEARCH)->Reset();
    GetMeter(MeterType::METER_TARGET_INFLUENCE)->Reset();
    GetMeter(MeterType::METER_TARGET_CONSTRUCTION)->Reset();

    GetMeter(MeterType::METER_SUPPLY)->Reset();
    GetMeter(MeterType::METER_MAX_SUPPLY)->Reset();
    GetMeter(MeterType::METER_STOCKPILE)->Reset();
    GetMeter(MeterType::METER_MAX_STOCKPILE)->Reset();
    GetMeter(MeterType::METER_SHIELD)->Reset();
    GetMeter(MeterType::METER_MAX_SHIELD)->Reset();
    GetMeter(MeterType::METER_DEFENSE)->Reset();
    GetMeter(MeterType::METER_MAX_DEFENSE)->Reset();
    GetMeter(MeterType::METER_DETECTION)->Reset();
    GetMeter(MeterType::METER_REBEL_TROOPS)->Reset();

    if (m_is_about_to_be_colonized) {
        for (auto* building : objects.findRaw<Building>(m_buildings)) {
            if (building)
                building->Reset();
        }
    }

    //m_last_turn_annexed left unchanged
    //m_turn_last_colonized left unchanged
    //m_turn_last_conquered left unchanged
    m_ordered_annexed_by_empire_id = ALL_EMPIRES;
    m_is_about_to_be_colonized = false;
    m_is_about_to_be_invaded = false;
    m_is_about_to_be_bombarded = false;
    m_ordered_given_to_empire_id = ALL_EMPIRES;
    m_last_annexed_by_empire_id = ALL_EMPIRES;
    m_last_invaded_by_empire_id = ALL_EMPIRES;
    m_last_colonized_by_empire_id = ALL_EMPIRES;
    SetOwner(ALL_EMPIRES);
}

void Planet::Depopulate(int current_turn) {
    GetMeter(MeterType::METER_POPULATION)->Reset();
    GetMeter(MeterType::METER_HAPPINESS)->Reset();
    GetMeter(MeterType::METER_INDUSTRY)->Reset();
    GetMeter(MeterType::METER_RESEARCH)->Reset();
    GetMeter(MeterType::METER_INFLUENCE)->Reset();
    GetMeter(MeterType::METER_CONSTRUCTION)->Reset();

    ClearFocus(current_turn);
}

void Planet::Conquer(int conquerer, ScriptingContext& context) {
    m_turn_last_conquered = context.current_turn;
    m_owner_before_last_conquered = this->Owner();

    // deal with things on production queue located at this planet
    Empire::ConquerProductionQueueItemsAtLocation(ID(), conquerer, context.Empires());

    ObjectMap& objects{context.ContextObjects()};
    const auto& ids_as_flatset{context.EmpireIDs()};
    const std::vector<int> empire_ids{ids_as_flatset.begin(), ids_as_flatset.end()};

    // deal with UniverseObjects (eg. buildings) located on this planet
    for (auto* building : objects.findRaw<Building>(m_buildings)) {
        const BuildingType* type = GetBuildingType(building->BuildingTypeName());

        // determine what to do with building of this type...
        const auto cap_result = type->GetCaptureResult(building->Owner(), conquerer, this->ID(), false);

        if (cap_result == CaptureResult::CR_CAPTURE) {
            // replace ownership
            building->SetOwner(conquerer);
            building->SetOrderedScrapped(false);
        } else if (cap_result == CaptureResult::CR_DESTROY) {
            // destroy object
            //DebugLogger() << "Planet::Conquer destroying object: " << building->Name();
            this->RemoveBuilding(building->ID());
            if (auto system = objects.getRaw<System>(this->SystemID()))
                system->Remove(building->ID());
            context.ContextUniverse().Destroy(building->ID(), empire_ids);
        } else if (cap_result == CaptureResult::CR_RETAIN) {
            // do nothing, including leaving scrapping state
        }
    }

    // replace ownership
    SetOwner(conquerer);
    m_last_invaded_by_empire_id = conquerer;
    ClearGiveToEmpire();

    if (conquerer == ALL_EMPIRES) {
        if (const auto species = context.species.GetSpecies(SpeciesName()))
            SetFocus(species->DefaultFocus(), context);
        else
            ClearFocus(m_turn_last_conquered);
    }

    GetMeter(MeterType::METER_SUPPLY)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_SUPPLY)->BackPropagate();
    GetMeter(MeterType::METER_STOCKPILE)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_STOCKPILE)->BackPropagate();
    GetMeter(MeterType::METER_INDUSTRY)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_INDUSTRY)->BackPropagate();
    GetMeter(MeterType::METER_RESEARCH)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_RESEARCH)->BackPropagate();
    GetMeter(MeterType::METER_INFLUENCE)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_INFLUENCE)->BackPropagate();
    GetMeter(MeterType::METER_CONSTRUCTION)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_CONSTRUCTION)->BackPropagate();
    GetMeter(MeterType::METER_DEFENSE)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_DEFENSE)->BackPropagate();
    GetMeter(MeterType::METER_SHIELD)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_SHIELD)->BackPropagate();
    GetMeter(MeterType::METER_HAPPINESS)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_HAPPINESS)->BackPropagate();
    GetMeter(MeterType::METER_DETECTION)->SetCurrent(0.0f);
    GetMeter(MeterType::METER_DETECTION)->BackPropagate();
}

void Planet::SetSpecies(std::string species_name, int turn, const SpeciesManager& sm) {
    if (m_species_name.empty() && !species_name.empty())
        m_turn_last_colonized = turn;  // if setting species with an effect, not via Colonize, consider it a colonization when there was no previous species set

    if (!species_name.empty() && !sm.GetSpecies(species_name))
        ErrorLogger() << "Planet::SetSpecies couldn't get species with name " << species_name;

    m_species_name = std::move(species_name);
}

void Planet::SetFocus(std::string focus, const ScriptingContext& context) {
    if (focus == m_focus)
        return;
    if (focus.empty()) {
        ClearFocus(context.current_turn);
        return;
    }
    if (!FocusAvailable(focus, context)) {
        ErrorLogger() << "Planet::SetFocus Exploiter!-- unavailable focus " << focus
                      << " attempted to be set for object w/ dump string: " << Dump();
        return;
    }

    m_focus = std::move(focus);
    if (m_focus == m_focus_turn_initial)
        m_last_turn_focus_changed = m_last_turn_focus_changed_turn_initial;
    else
        m_last_turn_focus_changed = context.current_turn;
    ResourceCenterChangedSignal();
}

void Planet::ClearFocus(int current_turn) {
    m_focus.clear();
    m_last_turn_focus_changed = current_turn;
    ResourceCenterChangedSignal();
}

void Planet::UpdateFocusHistory() {
    TraceLogger() << "Planet::UpdateFocusHistory: focus: " << m_focus
        << "  initial focus: " << m_focus_turn_initial
        << "  turns since change initial: " << m_last_turn_focus_changed_turn_initial;
    if (m_focus != m_focus_turn_initial) {
        m_focus_turn_initial = m_focus;
        m_last_turn_focus_changed_turn_initial = m_last_turn_focus_changed;
    }
}

bool Planet::Colonize(int empire_id, std::string species_name, double population,
                      ScriptingContext& context)
{
    const Species* species = nullptr;
    auto& objects = context.ContextObjects();

    // if desired pop > 0, we want a colony, not an outpost, so we need to do some checks
    if (population > 0.0) {
        // check if specified species exists and get reference
        species = context.species.GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "Planet::Colonize couldn't get species: " << species_name;
            return false;
        }
        // check if specified species can colonize this planet
        if (EnvironmentForSpecies(context.species, species_name) < PlanetEnvironment::PE_HOSTILE) {
            ErrorLogger() << "Planet::Colonize: can't colonize planet with species " << species_name
                          << " because planet is " << m_type 
                          << " which for that species is environment: "
                          << EnvironmentForSpecies(context.species, species_name);
            return false;
        }
    }

    // reset the planet to unowned/unpopulated
    if (!OwnedBy(empire_id)) {
        Reset(objects);

    } else {
        GetMeter(MeterType::METER_POPULATION)->Reset();
        GetMeter(MeterType::METER_TARGET_POPULATION)->Reset();
        GetMeter(MeterType::METER_HAPPINESS)->Reset();
        GetMeter(MeterType::METER_TARGET_HAPPINESS)->Reset();
        m_species_name.clear();

        for (auto* building : objects.findRaw<Building>(m_buildings)) {
            if (!building)
                continue;
            building->Reset();
        }
        m_ordered_annexed_by_empire_id = ALL_EMPIRES;
        m_is_about_to_be_colonized = false;
        m_is_about_to_be_invaded = false;
        m_is_about_to_be_bombarded = false;
        m_ordered_given_to_empire_id = ALL_EMPIRES;
        m_last_invaded_by_empire_id = ALL_EMPIRES;
        m_last_annexed_by_empire_id = ALL_EMPIRES;
        SetOwner(ALL_EMPIRES);
    }

    // if desired pop > 0, we want a colony, not an outpost, so we have to set the colony species
    if (population > MINIMUM_POP_CENTER_POPULATION)
        SetSpecies(std::move(species_name), context.current_turn, context.species);
    m_turn_last_colonized = context.current_turn; // may be redundant with same in SetSpecies, but here occurrs always, whereas in SetSpecies is only done if species is initially empty

    // find a default focus. use first defined available focus.
    // AvailableFoci function should return a vector of all names of
    // available foci.
    const auto available_foci = AvailableFoci(context);
    if (species && !available_foci.empty()) {
        bool found_preference = false;
        for (const auto focus : available_foci) {
            if (!focus.empty() && focus == species->DefaultFocus()) {
                SetFocus(std::string{focus}, context);
                found_preference = true;
                break;
            }
        }

        if (!found_preference)
            SetFocus(std::string{available_foci.front()}, context);
    } else {
        DebugLogger() << "Planet::Colonize unable to find a focus to set for species "
                      << this->SpeciesName();
    }

    // set colony population
    GetMeter(MeterType::METER_POPULATION)->SetCurrent(population);
    GetMeter(MeterType::METER_TARGET_POPULATION)->SetCurrent(population);
    BackPropagateMeters();


    // set specified empire as owner
    SetOwner(empire_id);
    m_last_colonized_by_empire_id = empire_id;

    // if there are buildings on the planet, set the specified empire as their owner too
    for (auto* building : objects.findRaw<Building>(BuildingIDs()))
        building->SetOwner(empire_id);

    return true;
}

void Planet::SetIsOrderAnnexedByEmpire(int empire_id) {
    const auto initial_empire = m_ordered_annexed_by_empire_id;
    if (empire_id == initial_empire)
        return;

    m_ordered_annexed_by_empire_id = empire_id;
    ResourceCenterChangedSignal();
}

void Planet::ResetBeingAnnxed()
{ SetIsOrderAnnexedByEmpire(ALL_EMPIRES); }

void Planet::SetLastAnnexedByEmpire(int id) {
    const auto initial_empire_id = m_last_annexed_by_empire_id;
    if (initial_empire_id == id) return;
    m_last_annexed_by_empire_id = id;
    StateChangedSignal();
}

void Planet::SetIsAboutToBeColonized(bool b) {
    bool initial_status = m_is_about_to_be_colonized;
    if (b == initial_status) return;
    m_is_about_to_be_colonized = b;
    StateChangedSignal();
}

void Planet::ResetIsAboutToBeColonized()
{ SetIsAboutToBeColonized(false); }

void Planet::SetLastColonizedByEmpire(int id) {
    const auto initial_empire_id = m_last_colonized_by_empire_id;
    if (initial_empire_id == id) return;
    m_last_colonized_by_empire_id = id;
    StateChangedSignal();
}

void Planet::SetTurnLastColonized(int turn) {
    const auto initial_turn = m_turn_last_colonized;
    if (initial_turn == turn) return;
    m_turn_last_colonized = turn;
    StateChangedSignal();
}

void Planet::SetIsAboutToBeInvaded(bool b) {
    bool initial_status = m_is_about_to_be_invaded;
    if (b == initial_status) return;
    m_is_about_to_be_invaded = b;
    StateChangedSignal();
}

void Planet::ResetIsAboutToBeInvaded()
{ SetIsAboutToBeInvaded(false); }

void Planet::SetLastInvadedByEmpire(int id) {
    const auto initial_empire_id = m_last_invaded_by_empire_id;
    if (initial_empire_id == id) return;
    m_last_invaded_by_empire_id = id;
    StateChangedSignal();
}

void Planet::SetIsAboutToBeBombarded(bool b) {
    bool initial_status = m_is_about_to_be_bombarded;
    if (b == initial_status) return;
    m_is_about_to_be_bombarded = b;
    StateChangedSignal();
}

void Planet::ResetIsAboutToBeBombarded()
{ SetIsAboutToBeBombarded(false); }

void Planet::SetGiveToEmpire(int empire_id) {
    if (empire_id != m_ordered_given_to_empire_id) {
        m_ordered_given_to_empire_id = empire_id;
        StateChangedSignal();
    }
}

void Planet::ClearGiveToEmpire()
{ SetGiveToEmpire(ALL_EMPIRES); }

void Planet::SetLastTurnAttackedByShip(int turn) noexcept
{ m_last_turn_attacked_by_ship = turn; }

void Planet::SetLastTurnAnnexed(int turn) noexcept
{ m_turn_last_annexed = turn; }

void Planet::SetSurfaceTexture(const std::string& texture) {
    m_surface_texture = texture;
    StateChangedSignal();
}

void Planet::PopGrowthProductionResearchPhase(ScriptingContext& context) {
    UniverseObject::PopGrowthProductionResearchPhase(context);

    if (!m_species_name.empty() && !Populated()) {
        // Should be run after meter update but before a backpropagation,
        // so check current, not initial, meter values. If population falls
        // below threshold, kill off the remainder
        Depopulate(context.current_turn);
    }

    // check for colonies without positive population, and change to outposts
    if (!SpeciesName().empty() &&
        GetMeter(MeterType::METER_POPULATION)->Current() <= MINIMUM_POP_CENTER_POPULATION)
    {
        if (auto empire = context.GetEmpire(this->Owner())) {
            empire->AddSitRepEntry(CreatePlanetDepopulatedSitRep(this->ID(), context.current_turn));

            if (!HasTag(TAG_STAT_SKIP_DEPOP, context))
                empire->RecordPlanetDepopulated(*this);
        }
        // remove species
        GetMeter(MeterType::METER_POPULATION)->Reset();
        GetMeter(MeterType::METER_TARGET_POPULATION)->Reset();
        GetMeter(MeterType::METER_HAPPINESS)->Reset();
        GetMeter(MeterType::METER_TARGET_HAPPINESS)->Reset();
        m_species_name.clear();
    }

    StateChangedSignal();
}

void Planet::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    GetMeter(MeterType::METER_TARGET_INDUSTRY)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_RESEARCH)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_INFLUENCE)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_CONSTRUCTION)->ResetCurrent();

    GetMeter(MeterType::METER_TARGET_POPULATION)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_HAPPINESS)->ResetCurrent();

    GetMeter(MeterType::METER_MAX_SUPPLY)->ResetCurrent();
    GetMeter(MeterType::METER_MAX_STOCKPILE)->ResetCurrent();
    GetMeter(MeterType::METER_MAX_SHIELD)->ResetCurrent();
    GetMeter(MeterType::METER_MAX_DEFENSE)->ResetCurrent();
    GetMeter(MeterType::METER_MAX_TROOPS)->ResetCurrent();
    GetMeter(MeterType::METER_REBEL_TROOPS)->ResetCurrent();
    GetMeter(MeterType::METER_DETECTION)->ResetCurrent();
}

void Planet::ClampMeters() {
    UniverseObject::ClampMeters();

    GetMeter(MeterType::METER_TARGET_INDUSTRY)->ClampCurrentToRange();
    GetMeter(MeterType::METER_TARGET_RESEARCH)->ClampCurrentToRange();
    //GetMeter(MeterType::METER_TARGET_INFLUENCE)->ClampCurrentToRange(-Meter::LARGE_VALUE, Meter::LARGE_VALUE);
    GetMeter(MeterType::METER_TARGET_CONSTRUCTION)->ClampCurrentToRange();

    GetMeter(MeterType::METER_INDUSTRY)->ClampCurrentToRange();
    GetMeter(MeterType::METER_RESEARCH)->ClampCurrentToRange();
    //GetMeter(MeterType::METER_INFLUENCE)->ClampCurrentToRange(-Meter::LARGE_VALUE, Meter::LARGE_VALUE);
    GetMeter(MeterType::METER_CONSTRUCTION)->ClampCurrentToRange();

    UniverseObject::GetMeter(MeterType::METER_POPULATION)->ClampCurrentToRange();

    UniverseObject::GetMeter(MeterType::METER_MAX_SHIELD)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_SHIELD)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_SHIELD)->Current());
    UniverseObject::GetMeter(MeterType::METER_MAX_DEFENSE)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_DEFENSE)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_DEFENSE)->Current());
    UniverseObject::GetMeter(MeterType::METER_MAX_TROOPS)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_TROOPS)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_TROOPS)->Current());
    UniverseObject::GetMeter(MeterType::METER_MAX_SUPPLY)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_SUPPLY)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_SUPPLY)->Current());
    UniverseObject::GetMeter(MeterType::METER_MAX_STOCKPILE)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_STOCKPILE)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(MeterType::METER_MAX_STOCKPILE)->Current());

    UniverseObject::GetMeter(MeterType::METER_REBEL_TROOPS)->ClampCurrentToRange();
    UniverseObject::GetMeter(MeterType::METER_DETECTION)->ClampCurrentToRange();
}

namespace {
    // sorted pair, so order of empire IDs specified doesn't matter
    constexpr std::pair<int, int> DiploKey(int id1, int ind2)
        noexcept(noexcept(std::max(1, -3)) && noexcept(std::min(-124, 0)))
    { return {std::max(id1, ind2), std::min(id1, ind2)}; }
}

void Planet::ResolveGroundCombat(std::map<int, double>& empires_troops,
                                 const DiploStatusMap& diplo_statuses)
{
    if (empires_troops.empty() || empires_troops.size() == 1)
        return;

    // give bonuses for allied ground combat, so allies can effectively fight together
    auto effective_empires_troops{empires_troops};
    for (const auto empire1_id : empires_troops | range_keys) {
        const auto not_empire1 = [empire1_id](const auto& id2_troops) noexcept { return empire1_id != id2_troops.first; };

        for (const auto& [empire2_id, troop2_count] : empires_troops | range_filter(not_empire1)) {
            const auto it = diplo_statuses.find(DiploKey(empire1_id, empire2_id));
            if (it != diplo_statuses.end() && it->second == DiplomaticStatus::DIPLO_ALLIED)
                effective_empires_troops[empire1_id] += troop2_count;
        }
    }

    // find effective troops and ID of victor...
    std::multimap<double, int> inverted_empires_troops;
    for (const auto& [eff_emp_id, eff_troops] : effective_empires_troops)
        inverted_empires_troops.emplace(eff_troops, eff_emp_id);

    const auto [victor_self_troops, victor_id] = *inverted_empires_troops.rbegin();
    static_assert(std::is_integral_v<decltype(victor_id)>);


    // victor has effective troops reduced by the effective troop count of
    // the strongest enemy combatant (allied and at-peace co-combatants are
    // ignored for this reduction)
    double highest_loser_enemy_effective_troops = 0.0f;
    for (auto highest_loser_it = inverted_empires_troops.rbegin();
         highest_loser_it != inverted_empires_troops.rend(); ++highest_loser_it)
    {
        const auto [loser_effective_troops, loser_id] = *highest_loser_it;
        if (loser_id == victor_id)
            continue;
        const auto it = diplo_statuses.find(DiploKey(loser_id, victor_id));
        if (it != diplo_statuses.end() && it->second == DiplomaticStatus::DIPLO_PEACE)
            continue;

        // found a suitable loser combatant
        highest_loser_enemy_effective_troops = loser_effective_troops;
        break;
    }

    const auto victor_effective_troops = victor_self_troops - highest_loser_enemy_effective_troops;
    const auto victor_starting_troops = empires_troops[victor_id];

    // every other combatant loses all troops
    empires_troops.clear();

    // final victor troops can't be more than they started with
    empires_troops[victor_id] = std::min(victor_effective_troops, victor_starting_troops);
}
