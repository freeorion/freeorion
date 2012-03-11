#include "Planet.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "System.h"
#include "Predicates.h"
#include "Species.h"
#include "Condition.h"
#include "ValueRef.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/Directories.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/assign/list_of.hpp>

namespace {
    // high tilt is arbitrarily taken to mean 35 degrees or more
    const double HIGH_TILT_THERSHOLD = 35.0;

    double SizeRotationFactor(PlanetSize size) {
        switch (size) {
        case SZ_TINY:     return 1.5;
        case SZ_SMALL:    return 1.25;
        case SZ_MEDIUM:   return 1.0;
        case SZ_LARGE:    return 0.75;
        case SZ_HUGE:     return 0.5;
        case SZ_GASGIANT: return 0.25;
        default:          return 1.0;
        }
        return 1.0;
    }

    static const std::string EMPTY_STRING;
}


////////////////////////////////////////////////////////////
// Year
////////////////////////////////////////////////////////////
Year::Year(Day d) :
    TypesafeDouble(d / 360.0)
{}


////////////////////////////////////////////////////////////
// Planet
////////////////////////////////////////////////////////////
Planet::Planet() :
    UniverseObject(),
    PopCenter(),
    ResourceCenter(),
    m_type(PT_TERRAN),
    m_size(SZ_MEDIUM),
    m_orbital_period(1.0),
    m_initial_orbital_position(0.0),
    m_rotational_period(1.0),
    m_axial_tilt(23.0),
    m_available_trade(0.0),
    m_just_conquered(false),
    m_is_about_to_be_colonized(false),
    m_is_about_to_be_invaded(false),
    m_last_turn_attacked_by_ship(-1)
{
    //Logger().debugStream() << "Planet::Planet()";
    // assumes PopCenter and ResourceCenter don't need to be initialized, due to having been re-created
    // in functional form by deserialization.  Also assumes planet-specific meters don't need to be re-added.
}

Planet::Planet(PlanetType type, PlanetSize size) :
    UniverseObject(),
    PopCenter(),
    ResourceCenter(),
    m_type(type),
    m_size(size),
    m_orbital_period(1.0),
    m_initial_orbital_position(RandZeroToOne() * 2 * 3.14159),
    m_rotational_period(1.0),
    m_axial_tilt(RandZeroToOne() * HIGH_TILT_THERSHOLD),
    m_available_trade(0.0),
    m_just_conquered(false),
    m_is_about_to_be_colonized(false),
    m_is_about_to_be_invaded(false),
    m_last_turn_attacked_by_ship(-1)
{
    //Logger().debugStream() << "Planet::Planet(" << type << ", " << size <<")";
    UniverseObject::Init();
    PopCenter::Init();
    ResourceCenter::Init();
    Planet::Init();
    SetType(type);
    SetSize(size);

    const double SPIN_STD_DEV = 0.1;
    const double REVERSE_SPIN_CHANCE = 0.01;
    m_rotational_period = RandGaussian(1.0, SPIN_STD_DEV) / SizeRotationFactor(m_size);
    if (RandZeroToOne() < REVERSE_SPIN_CHANCE)
        m_rotational_period = -m_rotational_period;
}

Planet* Planet::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return 0;

    Planet* retval = new Planet();
    retval->Copy(this, empire_id);
    return retval;
}

void Planet::Copy(const UniverseObject* copied_object, int empire_id) {
    if (copied_object == this)
        return;
    const Planet* copied_planet = universe_object_cast<Planet*>(copied_object);
    if (!copied_planet) {
        Logger().errorStream() << "Planet::Copy passed an object that wasn't a Planet";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis);
    PopCenter::Copy(copied_planet, vis);
    ResourceCenter::Copy(copied_planet, vis);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_buildings =                 copied_planet->VisibleContainedObjects(empire_id);
        this->m_type =                      copied_planet->m_type;
        this->m_size =                      copied_planet->m_size;
        this->m_orbital_period =            copied_planet->m_orbital_period;
        this->m_initial_orbital_position =  copied_planet->m_initial_orbital_position;
        this->m_rotational_period =         copied_planet->m_rotational_period;
        this->m_axial_tilt =                copied_planet->m_axial_tilt;
        this->m_just_conquered =            copied_planet->m_just_conquered;

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            if (vis >= VIS_FULL_VISIBILITY) {
                this->m_available_trade =           copied_planet->m_available_trade;
                this->m_is_about_to_be_colonized =  copied_planet->m_is_about_to_be_colonized;
                this->m_is_about_to_be_invaded   =  copied_planet->m_is_about_to_be_invaded;
                this->m_last_turn_attacked_by_ship= copied_planet->m_last_turn_attacked_by_ship;
            } else {
                // copy system name if at partial visibility, as it won't be copied
                // by UniverseObject::Copy unless at full visibility, but players
                // should know planet names even if they don't own the planet
                Universe::InhibitUniverseObjectSignals(true);
                this->Rename(copied_planet->Name());
                Universe::InhibitUniverseObjectSignals(false);
            }
        }
    }
}

const std::string& Planet::TypeName() const
{ return UserString("PLANET"); }

std::string Planet::Dump() const {
    std::stringstream os;
    os << UniverseObject::Dump();
    os << PopCenter::Dump();
    os << ResourceCenter::Dump();
    os << " planet type: " << UserString(GG::GetEnumMap<PlanetType>().FromEnum(m_type))
       << " size: " << UserString(GG::GetEnumMap<PlanetSize>().FromEnum(m_size))
       << " buildings: ";
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end();) {
        int building_id = *it;
        ++it;
        os << building_id << (it == m_buildings.end() ? "" : ", ");
    }
    if (m_is_about_to_be_colonized)
        os << " (About to be Colonize)";
    if (m_is_about_to_be_invaded)
        os << " (About to be Invaded)";
    if (m_just_conquered)
        os << " (Just Conquered)";
    os << " last attacked on turn: " << m_last_turn_attacked_by_ship;

    return os.str();
}

void Planet::Init() {
    AddMeter(METER_SUPPLY);
    AddMeter(METER_SHIELD);
    AddMeter(METER_MAX_SHIELD);
    AddMeter(METER_DEFENSE);
    AddMeter(METER_MAX_DEFENSE);
    AddMeter(METER_TROOPS);
    AddMeter(METER_MAX_TROOPS);
    AddMeter(METER_DETECTION);
}

PlanetEnvironment Planet::EnvironmentForSpecies(const std::string& species_name/* = ""*/) const {
    const Species* species = 0;
    if (species_name.empty()) {
        const std::string& this_planet_species_name = this->SpeciesName();
        if (this_planet_species_name.empty())
            return PE_UNINHABITABLE;
        species = GetSpecies(this_planet_species_name);
    } else {
        species = GetSpecies(species_name);
    }
    if (!species) {
        Logger().errorStream() << "Planet::EnvironmentForSpecies couldn't get species with name \"" << species_name << "\"";
        return PE_UNINHABITABLE;
    }
    return species->GetPlanetEnvironment(m_type);
}

PlanetType Planet::NextBetterPlanetTypeForSpecies(const std::string& species_name/* = ""*/) const {
    const Species* species = 0;
    if (species_name.empty()) {
        const std::string& this_planet_species_name = this->SpeciesName();
        if (this_planet_species_name.empty())
            return m_type;
        species = GetSpecies(this_planet_species_name);
    } else {
        species = GetSpecies(species_name);
    }
    if (!species) {
        Logger().errorStream() << "Planet::NextBetterPlanetTypeForSpecies couldn't get species with name \"" << species_name << "\"";
        return m_type;
    }
    return species->NextBetterPlanetType(m_type);
}

namespace {
    PlanetType LoopPlanetTypeIncrement(PlanetType initial_type, int step) {
        // avoid too large steps that would mess up enum arithmatic
        if (std::abs(step) >= PT_ASTEROIDS) {
            Logger().debugStream() << "LoopPlanetTypeIncrement giving too large step: " << step;
            return initial_type;
        }
        // some types can't be terraformed
        if (initial_type == PT_GASGIANT)
            return PT_GASGIANT;
        if (initial_type == PT_ASTEROIDS)
            return PT_ASTEROIDS;
        if (initial_type == INVALID_PLANET_TYPE)
            return INVALID_PLANET_TYPE;
        if (initial_type == NUM_PLANET_TYPES)
            return NUM_PLANET_TYPES;
        // calculate next planet type, accounting for loop arounds
        PlanetType new_type(PlanetType(initial_type + step));
        if (new_type >= PT_ASTEROIDS)
            new_type = PlanetType(new_type - PT_ASTEROIDS);
        else if (new_type <= INVALID_PLANET_TYPE)
            new_type = PlanetType(new_type + PT_ASTEROIDS);
        return new_type;
    }
}

PlanetType Planet::ClockwiseNextPlanetType() const
{ return LoopPlanetTypeIncrement(m_type, 1); }

PlanetType Planet::CounterClockwiseNextPlanetType() const
{ return LoopPlanetTypeIncrement(m_type, -1); }

namespace {
    PlanetSize PlanetSizeIncrement(PlanetSize initial_size, int step) {
        // some sizes don't have meaningful increments
        if (initial_size == SZ_GASGIANT)
            return SZ_GASGIANT;
        if (initial_size == SZ_ASTEROIDS)
            return SZ_ASTEROIDS;
        if (initial_size == SZ_NOWORLD)
            return SZ_NOWORLD;
        if (initial_size == INVALID_PLANET_SIZE)
            return INVALID_PLANET_SIZE;
        if (initial_size == NUM_PLANET_SIZES)
            return NUM_PLANET_SIZES;
        // calculate next planet size
        PlanetSize new_type(PlanetSize(initial_size + step));
        if (new_type >= SZ_HUGE)
            return SZ_HUGE;
        if (new_type <= SZ_TINY)
            return SZ_TINY;
        return new_type;
    }
}

PlanetSize Planet::NextLargerPlanetSize() const
{ return PlanetSizeIncrement(m_size, 1); }

PlanetSize Planet::NextSmallerPlanetSize() const
{ return PlanetSizeIncrement(m_size, -1); }

Year Planet::OrbitalPeriod() const
{ return m_orbital_period; }

Radian Planet::InitialOrbitalPosition() const
{ return m_initial_orbital_position; }

Radian Planet::OrbitalPositionOnTurn(int turn) const
{ return m_initial_orbital_position + OrbitalPeriod() * 2.0 * 3.1415926 / 4 * turn; }

Day Planet::RotationalPeriod() const
{ return m_rotational_period; }

Degree Planet::AxialTilt() const
{ return m_axial_tilt; }

double Planet::AvailableTrade() const
{ return m_available_trade; }

double Planet::BuildingCosts() const {
    double retval = 0.0;
    //for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
    //    const Building* building = GetBuilding(*it);
    //    const BuildingType* bulding_type = GetBuildingType(building->BuildingTypeName());
    //    retval += bulding_type->MaintenanceCost();
    //}
    return retval;
}

bool Planet::Contains(int object_id) const
{ return m_buildings.find(object_id) != m_buildings.end(); }

std::vector<UniverseObject*> Planet::FindObjects() const {
    std::vector<UniverseObject*> retval;
    // add buildings on this planet
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it)
        retval.push_back(GetUniverseObject(*it));
    return retval;
}

std::vector<int> Planet::FindObjectIDs() const {
    std::vector<int> retval;
    // add buildings on this planet
    std::copy(m_buildings.begin(), m_buildings.end(), std::back_inserter(retval));
    return retval;
}

UniverseObject* Planet::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(const_cast<Planet* const>(this)); }

Meter* Planet::GetMeter(MeterType type)
{ return UniverseObject::GetMeter(type); }

const Meter* Planet::GetMeter(MeterType type) const
{ return UniverseObject::GetMeter(type); }

double Planet::InitialMeterValue(MeterType type) const
{ return UniverseObject::InitialMeterValue(type); }

double Planet::CurrentMeterValue(MeterType type) const
{ return UniverseObject::CurrentMeterValue(type); }

double Planet::NextTurnCurrentMeterValue(MeterType type) const {
    MeterType max_meter_type = INVALID_METER_TYPE;
    switch (type) {
    case METER_TARGET_POPULATION:
    case METER_TARGET_HEALTH:
    case METER_POPULATION:
    case METER_HEALTH:
    case METER_FOOD_CONSUMPTION:
        return PopCenterNextTurnMeterValue(type);
        break;
    case METER_TARGET_FARMING:
    case METER_TARGET_INDUSTRY:
    case METER_TARGET_RESEARCH:
    case METER_TARGET_TRADE:
    case METER_TARGET_MINING:
    case METER_TARGET_CONSTRUCTION:
    case METER_FARMING:
    case METER_INDUSTRY:
    case METER_RESEARCH:
    case METER_TRADE:
    case METER_MINING:
    case METER_CONSTRUCTION:
        return ResourceCenterNextTurnMeterValue(type);
        break;
    case METER_SHIELD:  max_meter_type = METER_MAX_SHIELD;  break;
    case METER_TROOPS:  max_meter_type = METER_MAX_TROOPS;  break;
    case METER_DEFENSE: max_meter_type = METER_MAX_DEFENSE; break;
        break;
    default:
        return UniverseObject::NextTurnCurrentMeterValue(type);
    }

    const Meter* meter = GetMeter(type);
    if (!meter) {
        throw std::invalid_argument("Planet::NextTurnCurrentMeterValue passed meter type that the Planet does not have, but should.");
    }
    double current_meter_value = meter->Current();

    const Meter* max_meter = GetMeter(max_meter_type);
    if (!max_meter) {
        throw std::runtime_error("Planet::NextTurnCurrentMeterValue dealing with invalid meter type");
    }
    double max_meter_value = max_meter->Current();

    // being attacked prevents meter growth
    if (LastTurnAttackedByShip() >= CurrentTurn()) {
        return std::min(current_meter_value, max_meter_value);
    }

    // currently meter growth is one per turn.
    return std::min(current_meter_value + 1.0, max_meter_value);
}

std::vector<std::string> Planet::AvailableFoci() const {
    std::vector<std::string> retval;
    ScriptingContext context(this);
    if (const Species* species = GetSpecies(this->SpeciesName())) {
        const std::vector<FocusType>& foci = species->Foci();
        for (std::vector<FocusType>::const_iterator it = foci.begin(); it != foci.end(); ++it) {
            const FocusType& focus_type = *it;
            if (const Condition::ConditionBase* location = focus_type.Location()) {
                if (location->Eval(context, this))
                    retval.push_back(focus_type.Name());
            }
        }
    }
    return retval;
}

const std::string& Planet::FocusIcon(const std::string& focus_name) const {
    if (const Species* species = GetSpecies(this->SpeciesName())) {
        const std::vector<FocusType>& foci = species->Foci();
        for (std::vector<FocusType>::const_iterator it = foci.begin(); it != foci.end(); ++it) {
            const FocusType& focus_type = *it;
            if (focus_type.Name() == focus_name)
                return focus_type.Graphic();
        }
    }
    return EMPTY_STRING;
}

void Planet::SetType(PlanetType type) {
    if (type <= INVALID_PLANET_TYPE)
        type = PT_SWAMP;
    if (NUM_PLANET_TYPES <= type)
        type = PT_GASGIANT;
    m_type = type;
    StateChangedSignal();
}

void Planet::SetSize(PlanetSize size) {
    if (size <= SZ_NOWORLD)
        size = SZ_TINY;
    if (NUM_PLANET_SIZES <= size)
        size = SZ_GASGIANT;
    m_size = size;
    StateChangedSignal();
}

void Planet::SetOrbitalPeriod(unsigned int orbit) {
    assert(orbit < 10);
    const double THIRD_ORBIT_PERIOD = 4;
    const double THIRD_ORBIT_RADIUS = OrbitalRadius(2);
    const double ORBIT_RADIUS = OrbitalRadius(orbit);
    // Kepler's third law.
    m_orbital_period =
        std::sqrt(std::pow(THIRD_ORBIT_PERIOD, 2.0) /
                  std::pow(THIRD_ORBIT_RADIUS, 3.0) *
                  std::pow(ORBIT_RADIUS, 3.0));
}

void Planet::SetRotationalPeriod(Day days)
{ m_rotational_period = days; }

void Planet::SetHighAxialTilt() {
    const double MAX_TILT = 90.0;
    m_axial_tilt = HIGH_TILT_THERSHOLD + RandZeroToOne() * (MAX_TILT - HIGH_TILT_THERSHOLD);
}

void Planet::AddBuilding(int building_id) {
    if (this->Contains(building_id)) {
        Logger().debugStream() << "Planet::AddBuilding this planet " << this->Name() << " already contained building " << building_id;
        return;
    }
    //Logger().debugStream() << "Planet::AddBuilding " << this->Name() << " adding building: " << building_id;
    if (Building* building = GetBuilding(building_id)) {
        if (System* system = GetSystem(this->SystemID())) {
            system->Insert(building);
        } else {
            Logger().errorStream() << "... planet is not located in a system?!?!";
            building->MoveTo(X(), Y());
            building->SetSystem(SystemID());
        }
        building->SetPlanetID(ID());
        m_buildings.insert(building_id);
    } else {
        Logger().errorStream() << "Planet::AddBuilding() : Attempted to add an id of a non-building object to a planet.";
    }
    StateChangedSignal();
}

bool Planet::RemoveBuilding(int building_id) {
    if (m_buildings.find(building_id) != m_buildings.end()) {
        m_buildings.erase(building_id);
        StateChangedSignal();
        return true;
    }
    return false;
}

void Planet::SetAvailableTrade(double trade)
{ m_available_trade = trade; }

void Planet::Reset() {
    // remove owner
    SetOwner(ALL_EMPIRES);

    // reset popcenter meters
    PopCenter::Reset();

    // reset resourcecenter meters
    ResourceCenter::Reset();

    // reset planet meters
    GetMeter(METER_SUPPLY)->Reset();
    GetMeter(METER_SHIELD)->Reset();
    GetMeter(METER_MAX_SHIELD)->Reset();
    GetMeter(METER_DEFENSE)->Reset();
    GetMeter(METER_MAX_DEFENSE)->Reset();
    GetMeter(METER_DETECTION)->Reset();

    // reset buildings
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it)
        if (Building* building = GetBuilding(*it))
            building->Reset();

    // reset other state
    m_available_trade = 0.0;
    m_just_conquered = false;
    m_is_about_to_be_colonized = false;
    m_is_about_to_be_invaded = false;
}

void Planet::Conquer(int conquerer) {
    if (conquerer == ALL_EMPIRES)
        return;

    m_just_conquered = true;
    Empire* empire = Empires().Lookup(conquerer);
    if (!empire) {
        Logger().errorStream() << "Planet::Conquer: attempted to conquer a planet with an invalid conquerer with id: " << conquerer;
        return;
    }

    // deal with things on production queue located at this planet
    empire->ConquerBuildsAtLocation(ID());

    // deal with UniverseObjects (eg. buildings) located on this planet
    std::vector<UniverseObject*> contained_objects = this->FindObjects();
    for (std::vector<UniverseObject*>::iterator it = contained_objects.begin(); it != contained_objects.end(); ++it) {
        UniverseObject* obj = *it;

        // Buildings:
        if (Building* building = universe_object_cast<Building*>(obj)) {
            const BuildingType* type = building->GetBuildingType();

            // determine what to do with building of this type...
            const CaptureResult cap_result = type->GetCaptureResult(obj->Owner(), conquerer, this->ID(), false);

            if (cap_result == CR_CAPTURE) {
                // replace ownership
                obj->SetOwner(conquerer);
            } else if (cap_result == CR_DESTROY) {
                // destroy object
                Logger().debugStream() << "Planet::Conquer destroying object: " << obj->Name();
                GetUniverse().Destroy(obj->ID());
                obj = 0;
            } else if (cap_result == CR_RETAIN) {
                // do nothing
            }
        }

        // TODO: deal with any other UniverseObject subclasses...?
    }

    // replace ownership
    SetOwner(conquerer);
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

void Planet::SetLastTurnAttackedByShip(int turn)
{ m_last_turn_attacked_by_ship = turn; }

void Planet::SetSurfaceTexture(const std::string& texture)
{ m_surface_texture = texture; }

void Planet::SetSystem(int sys) {
    //Logger().debugStream() << "Planet::MoveTo(UniverseObject* object)";
    UniverseObject::SetSystem(sys);
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        UniverseObject* obj = GetUniverseObject(*it);
        if (!obj) {
            Logger().errorStream() << "Planet::SetSystem couldn't get building object with id " << *it;
            continue;
        }
        obj->SetSystem(sys);
    }
}

void Planet::MoveTo(double x, double y) {
    //Logger().debugStream() << "Planet::MoveTo(double x, double y)";
    // move planet itself
    UniverseObject::MoveTo(x, y);
    // move buildings
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        UniverseObject* obj = GetUniverseObject(*it);
        if (!obj) {
            Logger().errorStream() << "Planet::MoveTo couldn't get building object with id " << *it;
            continue;
        }
        obj->UniverseObject::MoveTo(x, y);
    }
}

void Planet::PopGrowthProductionResearchPhase() {
    UniverseObject::PopGrowthProductionResearchPhase();

    // do not do production if planet was just conquered
    if (m_just_conquered)
        m_just_conquered = false;
    else
        ResourceCenterPopGrowthProductionResearchPhase();

    PopCenterPopGrowthProductionResearchPhase();

    // check for planets with zero population.  If they have any owners
    // then the planet has likely just starved.  Regardless, resetting the
    // planet keeps things consistent.
    if (GetMeter(METER_POPULATION)->Current() == 0.0 && !Unowned()) {
        // generate starvation sitrep for empire that owns this depopulated planet
        if (Empire* empire = Empires().Lookup(this->Owner()))
            empire->AddSitRepEntry(CreatePlanetStarvedToDeathSitRep(this->ID()));

        // reset planet to empty and unowned
        Reset();

    } else {
        GetMeter(METER_SHIELD)->SetCurrent(Planet::NextTurnCurrentMeterValue(METER_SHIELD));
        GetMeter(METER_DEFENSE)->SetCurrent(Planet::NextTurnCurrentMeterValue(METER_DEFENSE));
        GetMeter(METER_TROOPS)->SetCurrent(Planet::NextTurnCurrentMeterValue(METER_TROOPS));
    }

    StateChangedSignal();
}

void Planet::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();
    ResourceCenterResetTargetMaxUnpairedMeters();
    PopCenterResetTargetMaxUnpairedMeters();

    // give planets base stealth slightly above zero, so that they can't be
    // seen from a distance without high detection ability
    if (Meter* stealth = GetMeter(METER_STEALTH)) {
        stealth->ResetCurrent();
        stealth->AddToCurrent(0.01);
    }

    GetMeter(METER_SUPPLY)->ResetCurrent();
    GetMeter(METER_MAX_SHIELD)->ResetCurrent();
    GetMeter(METER_MAX_DEFENSE)->ResetCurrent();
    GetMeter(METER_MAX_TROOPS)->ResetCurrent();
    GetMeter(METER_DETECTION)->ResetCurrent();
}

void Planet::ClampMeters() {
    UniverseObject::ClampMeters();
    ResourceCenterClampMeters();
    PopCenterClampMeters();

    UniverseObject::GetMeter(METER_MAX_SHIELD)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_SHIELD)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(METER_MAX_SHIELD)->Current());
    UniverseObject::GetMeter(METER_MAX_DEFENSE)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_DEFENSE)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(METER_MAX_DEFENSE)->Current());
    UniverseObject::GetMeter(METER_MAX_TROOPS)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_TROOPS)->ClampCurrentToRange(Meter::DEFAULT_VALUE, UniverseObject::GetMeter(METER_MAX_TROOPS)->Current());

    UniverseObject::GetMeter(METER_DETECTION)->ClampCurrentToRange();
    UniverseObject::GetMeter(METER_SUPPLY)->ClampCurrentToRange();
}

std::set<int> Planet::VisibleContainedObjects(int empire_id) const {
    std::set<int> retval;
    const Universe& universe = GetUniverse();
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        int object_id = *it;
        if (universe.GetObjectVisibilityByEmpire(object_id, empire_id) >= VIS_BASIC_VISIBILITY)
            retval.insert(object_id);
    }
    return retval;
}

// free functions

double PlanetRadius(PlanetSize size) {
    double retval = 0.0;
    switch (size) {
    case INVALID_PLANET_SIZE: retval = 0.0; break;
    case SZ_NOWORLD:          retval = 0.0; break;
    case SZ_TINY:             retval = 2.0; break;
    case SZ_SMALL:            retval = 3.5; break;
    default:
    case SZ_MEDIUM:           retval = 5.0; break;
    case SZ_LARGE:            retval = 7.0; break;
    case SZ_HUGE:             retval = 9.0; break;
    case SZ_ASTEROIDS:        retval = 0.0; break;
    case SZ_GASGIANT:         retval = 11.0; break; // this one goes to eleven
    case NUM_PLANET_SIZES:    retval = 0.0; break;
    };
    return retval;
}

double AsteroidBeltRadius()
{ return 12.5; }
