#include "Planet.h"

#include "BuildingType.h"
#include "Building.h"
#include "Condition.h"
#include "Fleet.h"
#include "Ship.h"
#include "Species.h"
#include "System.h"
#include "UniverseObjectVisitor.h"
#include "Universe.h"
#include "ValueRef.h"
#include "../empire/EmpireManager.h"
#include "../empire/Empire.h"
#include "../util/GameRules.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/SitRepEntry.h"
#include "../util/i18n.h"


namespace {
    // high tilt is arbitrarily taken to mean 45 degrees or more
    const float HIGH_TILT_THERESHOLD = 45.0f;

    float SizeRotationFactor(PlanetSize size) {
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

    static const std::string EMPTY_STRING;

    /** @content_tag{CTRL_STAT_SKIP_DEPOP} Do not count PopCenter%s with this tag for SpeciesPlanetsDepoped stat */
    const std::string TAG_STAT_SKIP_DEPOP = "CTRL_STAT_SKIP_DEPOP";
}


////////////////////////////////////////////////////////////
// Planet
////////////////////////////////////////////////////////////
Planet::Planet(PlanetType type, PlanetSize size) :
    m_type(type),
    m_original_type(type),
    m_size(size),
    m_initial_orbital_position(RandZeroToOne() * 2 * 3.14159f),
    m_axial_tilt(RandZeroToOne() * HIGH_TILT_THERESHOLD)
{
    //DebugLogger() << "Planet::Planet(" << type << ", " << size <<")";
    UniverseObject::Init();
    PopCenter::Init();
    ResourceCenter::Init();
    Planet::Init();

    const double SPIN_STD_DEV = 0.1;
    const double REVERSE_SPIN_CHANCE = 0.06;
    m_rotational_period = RandGaussian(1.0, SPIN_STD_DEV) / SizeRotationFactor(m_size);
    if (RandZeroToOne() < REVERSE_SPIN_CHANCE)
        m_rotational_period = -m_rotational_period;
}

Planet* Planet::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    Planet* retval = new Planet(m_type, m_size);
    retval->Copy(UniverseObject::shared_from_this(), empire_id);
    return retval;
}

void Planet::Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object.get() == this)
        return;
    auto copied_planet = std::dynamic_pointer_cast<const Planet>(copied_object);
    if (!copied_planet) {
        ErrorLogger() << "Planet::Copy passed an object that wasn't a Planet";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);
    PopCenter::Copy(copied_planet, vis);
    ResourceCenter::Copy(copied_planet, vis);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_name =                      copied_planet->m_name;

        this->m_buildings =                 copied_planet->VisibleContainedObjectIDs(empire_id);
        this->m_type =                      copied_planet->m_type;
        this->m_original_type =             copied_planet->m_original_type;
        this->m_size =                      copied_planet->m_size;
        this->m_orbital_period =            copied_planet->m_orbital_period;
        this->m_initial_orbital_position =  copied_planet->m_initial_orbital_position;
        this->m_rotational_period =         copied_planet->m_rotational_period;
        this->m_axial_tilt =                copied_planet->m_axial_tilt;
        this->m_turn_last_conquered =       copied_planet->m_turn_last_conquered;
        this->m_turn_last_colonized =       copied_planet->m_turn_last_colonized;


        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            if (vis >= Visibility::VIS_FULL_VISIBILITY) {
                this->m_is_about_to_be_colonized =  copied_planet->m_is_about_to_be_colonized;
                this->m_is_about_to_be_invaded   =  copied_planet->m_is_about_to_be_invaded;
                this->m_is_about_to_be_bombarded =  copied_planet->m_is_about_to_be_bombarded;
                this->m_ordered_given_to_empire_id =copied_planet->m_ordered_given_to_empire_id;
                this->m_last_turn_attacked_by_ship= copied_planet->m_last_turn_attacked_by_ship;
            } else {
                // copy system name if at partial visibility, as it won't be copied
                // by UniverseObject::Copy unless at full visibility, but players
                // should know planet names even if they don't own the planet
                GetUniverse().InhibitUniverseObjectSignals(true);
                this->Rename(copied_planet->Name());
                GetUniverse().InhibitUniverseObjectSignals(false);
            }
        }
    }
}

bool Planet::HostileToEmpire(int empire_id) const
{
    if (OwnedBy(empire_id))
        return false;

    // Empire owned planets are hostile to ALL_EMPIRES
    if (empire_id == ALL_EMPIRES)
        return !Unowned();

    // Unowned planets are only considered hostile if populated
    auto pop_meter = GetMeter(MeterType::METER_TARGET_POPULATION);
    if (Unowned())
        return pop_meter && (pop_meter->Current() != 0.0f);

    // both empires are normal empires
    return Empires().GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

std::set<std::string> Planet::Tags() const {
    const Species* species = GetSpecies(SpeciesName());
    if (!species)
        return {};
    return species->Tags();
}

bool Planet::HasTag(const std::string& name) const {
    const Species* species = GetSpecies(SpeciesName());

    return species && species->Tags().count(name);
}

UniverseObjectType Planet::ObjectType() const
{ return UniverseObjectType::OBJ_PLANET; }

std::string Planet::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << PopCenter::Dump(ntabs);
    os << ResourceCenter::Dump(ntabs);
    os << " type: " << m_type
       << " original type: " << m_original_type
       << " size: " << m_size
       << " rot period: " << m_rotational_period
       << " axis tilt: " << m_axial_tilt
       << " buildings: ";
    for (auto it = m_buildings.begin(); it != m_buildings.end();) {
        int building_id = *it;
        ++it;
        os << building_id << (it == m_buildings.end() ? "" : ", ");
    }
    if (m_is_about_to_be_colonized)
        os << " (About to be Colonized)";
    if (m_is_about_to_be_invaded)
        os << " (About to be Invaded)";

    os << " colonized on turn: " << m_turn_last_colonized;
    os << " conquered on turn: " << m_turn_last_conquered;
    if (m_is_about_to_be_bombarded)
        os << " (About to be Bombarded)";
    if (m_ordered_given_to_empire_id != ALL_EMPIRES)
        os << " (Ordered to be given to empire with id: " << m_ordered_given_to_empire_id << ")";
    os << " last attacked on turn: " << m_last_turn_attacked_by_ship;

    return os.str();
}

int Planet::HabitableSize() const {
    const auto& gr = GetGameRules();
    switch (m_size) {
    case PlanetSize::SZ_GASGIANT:  return gr.Get<int>("RULE_HABITABLE_SIZE_GASGIANT");   break;
    case PlanetSize::SZ_HUGE:      return gr.Get<int>("RULE_HABITABLE_SIZE_HUGE");   break;
    case PlanetSize::SZ_LARGE:     return gr.Get<int>("RULE_HABITABLE_SIZE_LARGE");   break;
    case PlanetSize::SZ_MEDIUM:    return gr.Get<int>("RULE_HABITABLE_SIZE_MEDIUM");   break;
    case PlanetSize::SZ_ASTEROIDS: return gr.Get<int>("RULE_HABITABLE_SIZE_ASTEROIDS");   break;
    case PlanetSize::SZ_SMALL:     return gr.Get<int>("RULE_HABITABLE_SIZE_SMALL");   break;
    case PlanetSize::SZ_TINY:      return gr.Get<int>("RULE_HABITABLE_SIZE_TINY");   break;
    default:                       return 0;   break;
    }
}

void Planet::Init() {
    AddMeter(MeterType::METER_SUPPLY);
    AddMeter(MeterType::METER_MAX_SUPPLY);
    AddMeter(MeterType::METER_STOCKPILE);
    AddMeter(MeterType::METER_MAX_STOCKPILE);
    AddMeter(MeterType::METER_SHIELD);
    AddMeter(MeterType::METER_MAX_SHIELD);
    AddMeter(MeterType::METER_DEFENSE);
    AddMeter(MeterType::METER_MAX_DEFENSE);
    AddMeter(MeterType::METER_TROOPS);
    AddMeter(MeterType::METER_MAX_TROOPS);
    AddMeter(MeterType::METER_DETECTION);
    AddMeter(MeterType::METER_REBEL_TROOPS);
}

PlanetEnvironment Planet::EnvironmentForSpecies(const std::string& species_name/* = ""*/) const {
    const Species* species = nullptr;
    if (species_name.empty()) {
        const std::string& this_planet_species_name = this->SpeciesName();
        if (this_planet_species_name.empty())
            return PlanetEnvironment::PE_UNINHABITABLE;
        species = GetSpecies(this_planet_species_name);
    } else {
        species = GetSpecies(species_name);
    }
    if (!species) {
        ErrorLogger() << "Planet::EnvironmentForSpecies couldn't get species with name \"" << species_name << "\"";
        return PlanetEnvironment::PE_UNINHABITABLE;
    }
    return species->GetPlanetEnvironment(m_type);
}

PlanetType Planet::NextBetterPlanetTypeForSpecies(const std::string& species_name/* = ""*/) const {
    const Species* species = nullptr;
    if (species_name.empty()) {
        const std::string& this_planet_species_name = this->SpeciesName();
        if (this_planet_species_name.empty())
            return m_type;
        species = GetSpecies(this_planet_species_name);
    } else {
        species = GetSpecies(species_name);
    }
    if (!species) {
        ErrorLogger() << "Planet::NextBetterPlanetTypeForSpecies couldn't get species with name \"" << species_name << "\"";
        return m_type;
    }
    return species->NextBetterPlanetType(m_type);
}

namespace {
    PlanetType RingNextPlanetType(PlanetType current_type) {
        PlanetType next(PlanetType(int(current_type)+1));
        if (next >= PlanetType::PT_ASTEROIDS)
            next = PlanetType::PT_SWAMP;
        return next;
    }
    PlanetType RingPreviousPlanetType(PlanetType current_type) {
        PlanetType next(PlanetType(int(current_type)-1));
        if (next <= PlanetType::INVALID_PLANET_TYPE)
            next = PlanetType::PT_OCEAN;
        return next;
    }
}

PlanetType Planet::NextCloserToOriginalPlanetType() const {
    if (m_type == PlanetType::INVALID_PLANET_TYPE ||
        m_type == PlanetType::PT_GASGIANT ||
        m_type == PlanetType::PT_ASTEROIDS ||
        m_original_type == PlanetType::INVALID_PLANET_TYPE ||
        m_original_type == PlanetType::PT_GASGIANT ||
        m_original_type == PlanetType::PT_ASTEROIDS)
    { return m_type; }

    if (m_type == m_original_type)
        return m_type;

    PlanetType cur_type = m_type;
    int cw_steps = 0;
    while (cur_type != m_original_type) {
        cw_steps++;
        cur_type = RingNextPlanetType(cur_type);
    }

    cur_type = m_type;
    int ccw_steps = 0;
    while (cur_type != m_original_type) {
        ccw_steps++;
        cur_type = RingPreviousPlanetType(cur_type);
    }

    if (cw_steps <= ccw_steps)
        return RingNextPlanetType(m_type);
    return RingPreviousPlanetType(m_type);
}

namespace {
    PlanetType LoopPlanetTypeIncrement(PlanetType initial_type, int step) {
        // avoid too large steps that would mess up enum arithmatic
        if (std::abs(step) >= int(PlanetType::PT_ASTEROIDS)) {
            DebugLogger() << "LoopPlanetTypeIncrement giving too large step: " << step;
            return initial_type;
        }
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
}

PlanetType Planet::ClockwiseNextPlanetType() const
{ return LoopPlanetTypeIncrement(m_type, 1); }

PlanetType Planet::CounterClockwiseNextPlanetType() const
{ return LoopPlanetTypeIncrement(m_type, -1); }

int Planet::TypeDifference(PlanetType type1, PlanetType type2) {
    // no distance defined for invalid types
    if (type1 == PlanetType::INVALID_PLANET_TYPE || type2 == PlanetType::INVALID_PLANET_TYPE)
        return 0;
    // if the same, distance is zero
    if (type1 == type2)
        return 0;
    // no distance defined for asteroids or gas giants with anything else
    if (type1 == PlanetType::PT_ASTEROIDS || type1 == PlanetType::PT_GASGIANT || type2 == PlanetType::PT_ASTEROIDS || type2 == PlanetType::PT_GASGIANT)
        return 0;
    // find distance around loop:
    //
    //  0  1  2
    //  8     3
    //  7     4
    //    6 5
    int diff = std::abs(int(type1) - int(type2));
    // raw_dist -> actual dist
    //  0 to 4       0 to 4
    //  5 to 8       4 to 1
    if (diff > 4)
        diff = 9 - diff;
    //std::cout << "typedifference type1: " << int(type1) << "  type2: " << int(type2) << "  diff: " << diff << "\n";
    return diff;
}

namespace {
    PlanetSize PlanetSizeIncrement(PlanetSize initial_size, int step) {
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

PlanetSize Planet::NextLargerPlanetSize() const
{ return PlanetSizeIncrement(m_size, 1); }

PlanetSize Planet::NextSmallerPlanetSize() const
{ return PlanetSizeIncrement(m_size, -1); }

float Planet::OrbitalPeriod() const
{ return m_orbital_period; }

float Planet::InitialOrbitalPosition() const
{ return m_initial_orbital_position; }

float Planet::OrbitalPositionOnTurn(int turn) const
{ return m_initial_orbital_position + OrbitalPeriod() * 2.0 * 3.1415926 / 4 * turn; }

float Planet::RotationalPeriod() const
{ return m_rotational_period; }

float Planet::AxialTilt() const
{ return m_axial_tilt; }

std::shared_ptr<UniverseObject> Planet::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Planet>(std::static_pointer_cast<const Planet>(UniverseObject::shared_from_this()))); }

Meter* Planet::GetMeter(MeterType type)
{ return UniverseObject::GetMeter(type); }

const Meter* Planet::GetMeter(MeterType type) const
{ return UniverseObject::GetMeter(type); }

std::string Planet::CardinalSuffix() const {
    std::string retval = "";
    // Early return for invalid ID
    if (ID() == INVALID_OBJECT_ID) {
        WarnLogger() << "Planet " << Name() << " has invalid ID";
        return retval;
    }

    auto cur_system = Objects().get<System>(SystemID());
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

        PlanetType other_planet_type = Objects().get<Planet>(sys_orbit)->Type();
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

int Planet::ContainerObjectID() const
{ return this->SystemID(); }

const std::set<int>& Planet::ContainedObjectIDs() const
{ return m_buildings; }

bool Planet::Contains(int object_id) const
{ return object_id != INVALID_OBJECT_ID && m_buildings.count(object_id); }

bool Planet::ContainedBy(int object_id) const
{ return object_id != INVALID_OBJECT_ID && this->SystemID() == object_id; }

std::vector<std::string> Planet::AvailableFoci() const {
    std::vector<std::string> retval;
    auto this_planet = std::dynamic_pointer_cast<const Planet>(UniverseObject::shared_from_this());
    if (!this_planet)
        return retval;
    ScriptingContext context(this_planet);
    if (const auto* species = GetSpecies(this_planet->SpeciesName())) {
        retval.reserve(species->Foci().size());
        for (const auto& focus_type : species->Foci()) {
            if (const auto* location = focus_type.Location()) {
                if (location->Eval(context, this_planet))
                    retval.emplace_back(focus_type.Name());
            }
        }
    }

    return retval;
}

const std::string& Planet::FocusIcon(const std::string& focus_name) const {
    if (const Species* species = GetSpecies(this->SpeciesName())) {
        for (const FocusType& focus_type : species->Foci()) {
            if (focus_type.Name() == focus_name)
                return focus_type.Graphic();
        }
    }
    return EMPTY_STRING;
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
    const double MAX_TILT = 90.0;
    m_axial_tilt = HIGH_TILT_THERESHOLD + RandZeroToOne() * (MAX_TILT - HIGH_TILT_THERESHOLD);
}

void Planet::AddBuilding(int building_id) {
    size_t buildings_size = m_buildings.size();
    m_buildings.insert(building_id);
    if (buildings_size != m_buildings.size())
        StateChangedSignal();
    // expect calling code to set building's planet
}

bool Planet::RemoveBuilding(int building_id) {
    if (m_buildings.count(building_id)) {
        m_buildings.erase(building_id);
        StateChangedSignal();
        return true;
    }
    return false;
}

void Planet::Reset() {
    PopCenter::Reset();
    ResourceCenter::Reset();

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

    if (m_is_about_to_be_colonized && !OwnedBy(ALL_EMPIRES)) {
        for (const auto& building : Objects().find<Building>(m_buildings)) {
            if (!building)
                continue;
            building->Reset();
        }
    }

    //m_turn_last_colonized left unchanged
    //m_turn_last_conquered left unchanged
    m_is_about_to_be_colonized = false;
    m_is_about_to_be_invaded = false;
    m_is_about_to_be_bombarded = false;
    SetOwner(ALL_EMPIRES);
}

void Planet::Depopulate() {
    PopCenter::Depopulate();

    GetMeter(MeterType::METER_INDUSTRY)->Reset();
    GetMeter(MeterType::METER_RESEARCH)->Reset();
    GetMeter(MeterType::METER_INFLUENCE)->Reset();
    GetMeter(MeterType::METER_CONSTRUCTION)->Reset();

    ClearFocus();
}

void Planet::Conquer(int conquerer) {
    m_turn_last_conquered = CurrentTurn();

    // deal with things on production queue located at this planet
    Empire::ConquerProductionQueueItemsAtLocation(ID(), conquerer);

    // deal with UniverseObjects (eg. buildings) located on this planet
    for (auto& building : Objects().find<Building>(m_buildings)) {
        const BuildingType* type = GetBuildingType(building->BuildingTypeName());

        // determine what to do with building of this type...
        const CaptureResult cap_result = type->GetCaptureResult(building->Owner(), conquerer, this->ID(), false);

        if (cap_result == CaptureResult::CR_CAPTURE) {
            // replace ownership
            building->SetOwner(conquerer);
        } else if (cap_result == CaptureResult::CR_DESTROY) {
            // destroy object
            //DebugLogger() << "Planet::Conquer destroying object: " << building->Name();
            this->RemoveBuilding(building->ID());
            if (auto system = Objects().get<System>(this->SystemID()))
                system->Remove(building->ID());
            GetUniverse().Destroy(building->ID());
        } else if (cap_result == CaptureResult::CR_RETAIN) {
            // do nothing
        }
    }

    // replace ownership
    SetOwner(conquerer);

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

void Planet::SetSpecies(std::string species_name) {
    if (SpeciesName().empty() && !species_name.empty())
        m_turn_last_colonized = CurrentTurn();  // if setting species with an effect, not via Colonize, consider it a colonization when there was no previous species set
    PopCenter::SetSpecies(std::move(species_name));
}

bool Planet::Colonize(int empire_id, std::string species_name, double population) {
    const Species* species = nullptr;

    // if desired pop > 0, we want a colony, not an outpost, so we need to do some checks
    if (population > 0.0) {
        // check if specified species exists and get reference
        species = GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "Planet::Colonize couldn't get species: " << species_name;
            return false;
        }
        // check if specified species can colonize this planet
        if (EnvironmentForSpecies(species_name) < PlanetEnvironment::PE_HOSTILE) {
            ErrorLogger() << "Planet::Colonize: can't colonize planet with species " << species_name << " because planet is "
                          << m_type << " which for that species is environment: " << EnvironmentForSpecies(species_name);
            return false;
        }
    }

    // reset the planet to unowned/unpopulated
    if (!OwnedBy(empire_id)) {
        Reset();
    } else {
        PopCenter::Reset();
        for (const auto& building : Objects().find<Building>(m_buildings)) {
            if (!building)
                continue;
            building->Reset();
        }
        m_is_about_to_be_colonized = false;
        m_is_about_to_be_invaded = false;
        m_is_about_to_be_bombarded = false;
        SetOwner(ALL_EMPIRES);
    }

    // if desired pop > 0, we want a colony, not an outpost, so we have to set the colony species
    if (population > 0.0)
        SetSpecies(std::move(species_name));
    m_turn_last_colonized = CurrentTurn();  // may be redundant with same in SetSpecies, but here occurrs always, whereas in SetSpecies is only done if species is initially empty

    // find a default focus. use first defined available focus.
    // AvailableFoci function should return a vector of all names of
    // available foci.
    auto available_foci = AvailableFoci();
    if (species && !available_foci.empty()) {
        bool found_preference = false;
        for (const auto& focus : available_foci) {
            if (!focus.empty() && focus == species->DefaultFocus()) {
                SetFocus(focus);
                found_preference = true;
                break;
            }
        }

        if (!found_preference)
            SetFocus(*available_foci.begin());
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

    // if there are buildings on the planet, set the specified empire as their owner too
    for (auto& building : Objects().find<Building>(BuildingIDs()))
        building->SetOwner(empire_id);

    return true;
}

void Planet::SetIsAboutToBeColonized(bool b) {
    bool initial_status = m_is_about_to_be_colonized;
    if (b == initial_status) return;
    m_is_about_to_be_colonized = b;
    StateChangedSignal();
}

void Planet::ResetIsAboutToBeColonized()
{ SetIsAboutToBeColonized(false); }

void Planet::SetIsAboutToBeInvaded(bool b) {
    bool initial_status = m_is_about_to_be_invaded;
    if (b == initial_status) return;
    m_is_about_to_be_invaded = b;
    StateChangedSignal();
}

void Planet::ResetIsAboutToBeInvaded()
{ SetIsAboutToBeInvaded(false); }

void Planet::SetIsAboutToBeBombarded(bool b) {
    bool initial_status = m_is_about_to_be_bombarded;
    if (b == initial_status) return;
    m_is_about_to_be_bombarded = b;
    StateChangedSignal();
}

void Planet::ResetIsAboutToBeBombarded()
{ SetIsAboutToBeBombarded(false); }

void Planet::SetGiveToEmpire(int empire_id) {
    if (empire_id == m_ordered_given_to_empire_id) return;
    m_ordered_given_to_empire_id = empire_id;
    StateChangedSignal();
}

void Planet::ClearGiveToEmpire()
{ SetGiveToEmpire(ALL_EMPIRES); }

void Planet::SetLastTurnAttackedByShip(int turn)
{ m_last_turn_attacked_by_ship = turn; }

void Planet::SetSurfaceTexture(const std::string& texture) {
    m_surface_texture = texture;
    StateChangedSignal();
}

void Planet::PopGrowthProductionResearchPhase() {
    UniverseObject::PopGrowthProductionResearchPhase();
    PopCenterPopGrowthProductionResearchPhase();

    // should be run after a meter update, but before a backpropagation, so check current, not initial, meter values

    // check for colonies without positive population, and change to outposts
    if (!SpeciesName().empty() && GetMeter(MeterType::METER_POPULATION)->Current() <= 0.0f) {
        if (Empire* empire = GetEmpire(this->Owner())) {
            empire->AddSitRepEntry(CreatePlanetDepopulatedSitRep(this->ID()));

            if (!HasTag(TAG_STAT_SKIP_DEPOP))
                empire->RecordPlanetDepopulated(*this);
        }
        // remove species
        PopCenter::Reset();
    }

    StateChangedSignal();
}

void Planet::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();
    ResourceCenterResetTargetMaxUnpairedMeters();
    PopCenterResetTargetMaxUnpairedMeters();

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
    ResourceCenterClampMeters();
    PopCenterClampMeters();

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
