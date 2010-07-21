#include "Planet.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "System.h"
#include "Predicates.h"
#include "Species.h"
#include "Condition.h"
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

    double SizeRotationFactor(PlanetSize size)
    {
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
    m_is_about_to_be_colonized(false)
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
    m_is_about_to_be_colonized(false)
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

Planet* Planet::Clone(int empire_id) const
{
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return 0;

    Planet* retval = new Planet();
    retval->Copy(this, empire_id);
    return retval;
}

void Planet::Copy(const UniverseObject* copied_object, int empire_id)
{
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

        if (vis >= VIS_FULL_VISIBILITY) {
            this->m_available_trade =           copied_planet->m_available_trade;
            this->m_is_about_to_be_colonized =  copied_planet->m_is_about_to_be_colonized;
        }
    }
}

const std::string& Planet::TypeName() const
{
    return UserString("PLANET");
}

void Planet::Dump() const
{
    UniverseObject::Dump();
    Logger().debugStream() << " ... (Planet " << this->ID() << ": " << this->Name() << "): species: " << PopCenter::SpeciesName();
}

void Planet::Init() {
    AddMeter(METER_SUPPLY);
    AddMeter(METER_SHIELD);
    AddMeter(METER_MAX_SHIELD);
    AddMeter(METER_DEFENSE);
    AddMeter(METER_MAX_DEFENSE);
    AddMeter(METER_DETECTION);
}

PlanetEnvironment Planet::EnvironmentForSpecies(const std::string& species_name/* = ""*/) const
{
    const Species* species = 0;
    if (species_name.empty()) {
        species = GetSpecies(this->SpeciesName());
        if (!species) {
            Logger().errorStream() << "Planet::EnvironmentForSpecies couldn't get own species with name \"" << this->SpeciesName() << "\"";
            return PE_UNINHABITABLE;
        }
    } else {
        species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "Planet::EnvironmentForSpecies couldn't get species with name \"" << species_name << "\"";
            return PE_UNINHABITABLE;
        }
    }
    return species->GetPlanetEnvironment(m_type);
}

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

double Planet::BuildingCosts() const
{
    double retval = 0.0;
    const ObjectMap& objects = GetMainObjectMap();
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        const Building* building = objects.Object<Building>(*it);
//        if (building->Operating()) {
            const BuildingType* bulding_type = GetBuildingType(building->BuildingTypeName());
            retval += bulding_type->MaintenanceCost();
//        }
    }
    return retval;
}

bool Planet::Contains(int object_id) const
{
    return m_buildings.find(object_id) != m_buildings.end();
}

std::vector<UniverseObject*> Planet::FindObjects() const
{
    ObjectMap& objects = GetMainObjectMap();
    std::vector<UniverseObject*> retval;
    // add buildings on this planet
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it)
        retval.push_back(objects.Object(*it));
    return retval;
}

std::vector<int> Planet::FindObjectIDs() const
{
    std::vector<int> retval;
    // add buildings on this planet
    std::copy(m_buildings.begin(), m_buildings.end(), std::back_inserter(retval));
    return retval;
}

UniverseObject* Planet::Accept(const UniverseObjectVisitor& visitor) const
{
    return visitor.Visit(const_cast<Planet* const>(this));
}

Meter* Planet::GetMeter(MeterType type)
{return UniverseObject::GetMeter(type);}

const Meter* Planet::GetMeter(MeterType type) const
{return UniverseObject::GetMeter(type);}

double Planet::CurrentMeterValue(MeterType type) const
{
    return UniverseObject::CurrentMeterValue(type);
}

double Planet::NextTurnCurrentMeterValue(MeterType type) const
{
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
    default:
        return UniverseObject::NextTurnCurrentMeterValue(type);
    }
}

std::vector<std::string> Planet::AvailableFoci() const
{
    std::vector<std::string> retval;
    if (const Species* species = GetSpecies(this->SpeciesName())) {
        const std::vector<FocusType>& foci = species->Foci();
        for (std::vector<FocusType>::const_iterator it = foci.begin(); it != foci.end(); ++it) {
            const FocusType& focus_type = *it;
            if (const Condition::ConditionBase* location = focus_type.Location()) {
                const UniverseObject* source = this;
                Condition::ObjectSet potential_targets; potential_targets.insert(this);
                Condition::ObjectSet matched_targets;
                location->Eval(source, matched_targets, potential_targets);
                if (!matched_targets.empty())
                    retval.push_back(focus_type.Name());
            }
        }
    }
    return retval;
}

const std::string& Planet::FocusIcon(const std::string& focus_name) const
{
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

void Planet::SetType(PlanetType type)
{
    if (type <= INVALID_PLANET_TYPE)
        type = PT_SWAMP;
    if (NUM_PLANET_TYPES <= type)
        type = PT_GASGIANT;
    m_type = type;
    StateChangedSignal();
}

void Planet::SetSize(PlanetSize size)
{
    if (size <= SZ_NOWORLD)
        size = SZ_TINY;
    if (NUM_PLANET_SIZES <= size)
        size = SZ_GASGIANT;
    m_size = size;
    StateChangedSignal();
}

void Planet::SetOrbitalPeriod(unsigned int orbit, bool tidal_lock)
{
    assert(orbit < 10);
    const double THIRD_ORBIT_PERIOD = 4;
    const double THIRD_ORBIT_RADIUS = OrbitalRadius(2);
    const double ORBIT_RADIUS = OrbitalRadius(orbit);
    // Kepler's third law.
    m_orbital_period =
        std::sqrt(std::pow(THIRD_ORBIT_PERIOD, 2.0) /
                  std::pow(THIRD_ORBIT_RADIUS, 3.0) *
                  std::pow(ORBIT_RADIUS, 3.0));

    if (tidal_lock)
        SetRotationalPeriod(Day(m_orbital_period));
}

void Planet::SetRotationalPeriod(Day days)
{ m_rotational_period = days; }

void Planet::SetHighAxialTilt()
{
    const double MAX_TILT = 90.0;
    m_axial_tilt = HIGH_TILT_THERSHOLD + RandZeroToOne() * (MAX_TILT - HIGH_TILT_THERSHOLD);
}

void Planet::AddBuilding(int building_id)
{
    if (this->Contains(building_id)) {
        Logger().debugStream() << "Planet::AddBuilding this planet " << this->Name() << " already contained building " << building_id;
        return;
    }
    //Logger().debugStream() << "Planet::AddBuilding " << this->Name() << " adding building: " << building_id;
    if (Building* building = GetObject<Building>(building_id)) {
        if (System* system = GetObject<System>(this->SystemID())) {
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

bool Planet::RemoveBuilding(int building_id)
{
    if (m_buildings.find(building_id) != m_buildings.end()) {
        m_buildings.erase(building_id);
        StateChangedSignal();
        return true;
    }
    return false;
}

void Planet::SetAvailableTrade(double trade)
{
    m_available_trade = trade;
}

void Planet::AddOwner(int id)
{
    if (System* system = GetObject<System>(this->SystemID()))
        system->UniverseObject::AddOwner(id);
    else
        Logger().errorStream() << "Planet::Addowner couldn't get system with id " << this->SystemID();

    UniverseObject::AddOwner(id);
}

void Planet::RemoveOwner(int id)
{
    System* system = GetObject<System>(this->SystemID());

    // check if Empire(id) is owner of at least one other planet in same system
    std::vector<int> planets = system->FindObjectIDs<Planet>();
    int empire_owned_planets_in_this_planet_system = 0;
    for (std::vector<int>::const_iterator plt_it = planets.begin(); plt_it != planets.end(); ++plt_it)
        if (Planet* planet = GetObject<Planet>(*plt_it))
            if (planet->OwnedBy(id))
                ++empire_owned_planets_in_this_planet_system;

    // if this is the only planet owned by this planet's current owner, removing
    // that owner means the empire owns no planets in this system, so loses
    // any ownership of the system as well
    if (empire_owned_planets_in_this_planet_system < 2)
        system->UniverseObject::RemoveOwner(id);

    UniverseObject::RemoveOwner(id);
}

void Planet::Reset()
{
    // remove owners
    ClearOwners();

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
        if (Building* building = GetMainObjectMap().Object<Building>(*it))
            building->Reset();

    // reset other state
    m_available_trade = 0.0;
    m_just_conquered = false;
    m_is_about_to_be_colonized = false;
}

void Planet::Conquer(int conquerer)
{
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
            const CaptureResult cap_result = type->GetCaptureResult(obj->Owners(), conquerer, this->ID(), false);

            if (cap_result == CR_CAPTURE) {
                // remove existing owners and replace with conquerer
                obj->ClearOwners();
                obj->AddOwner(conquerer);
            } else if (cap_result == CR_DESTROY) {
                // destroy object
                Logger().debugStream() << "Planet::Conquer destroying object: " << obj->Name();
                GetUniverse().Destroy(obj->ID());
                obj = 0;
            } else if (cap_result == CR_RETAIN) {
                // do nothing
            } else if (cap_result == CR_SHARE) {
                // add conquerer, but retain any previous owners
                obj->AddOwner(conquerer);
            }
        }

        // TODO: deal with any other UniverseObject subclasses...?
    }

    // remove existing owners of planet itself and replace with conquerer
    this->ClearOwners();
    this->AddOwner(conquerer);
}

void Planet::SetIsAboutToBeColonized(bool b)
{
    bool initial_status = m_is_about_to_be_colonized;
    if (b == initial_status) return;
    m_is_about_to_be_colonized = b;
    StateChangedSignal();
}

void Planet::ResetIsAboutToBeColonized()
{
    SetIsAboutToBeColonized(false);
}

void Planet::SetSystem(int sys)
{
    //Logger().debugStream() << "Planet::MoveTo(UniverseObject* object)";
    UniverseObject::SetSystem(sys);
    ObjectMap& objects = GetMainObjectMap();
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        UniverseObject* obj = objects.Object(*it);
        if (!obj) {
            Logger().errorStream() << "Planet::SetSystem couldn't get building object with id " << *it;
            continue;
        }
        obj->SetSystem(sys);
    }
}

void Planet::MoveTo(double x, double y)
{
    //Logger().debugStream() << "Planet::MoveTo(double x, double y)";
    // move planet itself
    UniverseObject::MoveTo(x, y);
    ObjectMap& objects = GetMainObjectMap();
    // move buildings
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        UniverseObject* obj = objects.Object(*it);
        if (!obj) {
            Logger().errorStream() << "Planet::MoveTo couldn't get building object with id " << *it;
            continue;
        }
        obj->UniverseObject::MoveTo(x, y);
    }
}

void Planet::PopGrowthProductionResearchPhase()
{
    UniverseObject::PopGrowthProductionResearchPhase();

    // do not do production if planet was just conquered
    if (m_just_conquered)
        m_just_conquered = false;
    else
        ResourceCenterPopGrowthProductionResearchPhase();

    PopCenterPopGrowthProductionResearchPhase();

    // check for starvation
    if (GetMeter(METER_POPULATION)->Current() < PopCenter::MINIMUM_POP_CENTER_POPULATION) {
        Logger().debugStream() << "Planet::PopGrowthProductionResearchPhase Planet " << this->Name() << " " << this->ID() << " is starving!";
        // starving.

        // generate starvation sitreps
        const std::set<int>& owners = this->Owners();
        for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it) {
            if (Empire* empire = Empires().Lookup(*it))
                empire->AddSitRepEntry(CreatePlanetStarvedToDeathSitRep(this->ID()));
            else
                Logger().errorStream() << "Planet::PopGrowthProductionResearchPhase couldn't get Empire with id " << *it << " to generate sitrep about starved planet";
        }

        // reset planet to empty and unowned
        Reset();

    } else {
        // not starving.  grow meters
        GetMeter(METER_SHIELD)->AddToCurrent(1.0);
        GetMeter(METER_DEFENSE)->AddToCurrent(1.0);
    }

    StateChangedSignal();
}

void Planet::ResetTargetMaxUnpairedMeters(MeterType meter_type)
{
    UniverseObject::ResetTargetMaxUnpairedMeters(meter_type);
    ResourceCenterResetTargetMaxUnpairedMeters(meter_type);
    PopCenterResetTargetMaxUnpairedMeters(meter_type);

    // give planets base stealth slightly above zero, so that they can't be seen from a distance without high detection ability
    if (meter_type == INVALID_METER_TYPE || meter_type == METER_STEALTH)
        if (Meter* stealth = GetMeter(METER_STEALTH)) {
            stealth->ResetCurrent();
            stealth->AddToCurrent(0.01);
        }

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_SUPPLY)
        GetMeter(METER_SUPPLY)->ResetCurrent();

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_MAX_SHIELD)
        GetMeter(METER_MAX_SHIELD)->ResetCurrent();

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_MAX_DEFENSE)
        GetMeter(METER_MAX_DEFENSE)->ResetCurrent();

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_DETECTION)
        GetMeter(METER_DETECTION)->ResetCurrent();
}

void Planet::ClampMeters()
{
    UniverseObject::ClampMeters();
    ResourceCenterClampMeters();
    PopCenterClampMeters();
}

std::set<int> Planet::VisibleContainedObjects(int empire_id) const
{
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

double PlanetRadius(PlanetSize size)
{
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
