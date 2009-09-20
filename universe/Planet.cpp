#include "Planet.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "System.h"
#include "Predicates.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/DataTable.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/Directories.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <GG/SignalsAndSlots.h>

#include <boost/lexical_cast.hpp>

#include <stdexcept>


using boost::lexical_cast;

namespace {
    // high tilt is arbitrarily taken to mean 35 degrees or more
    const double HIGH_TILT_THERSHOLD = 35.0;

    DataTableMap& PlanetDataTables() {
        static DataTableMap map;
        if (map.empty())
            LoadDataTables((GetResourceDir() / "planet_tables.txt").file_string(), map);
        return map;
    }

    double MaxPopMod(PlanetSize size, PlanetEnvironment environment) {
        return PlanetDataTables()["PlanetMaxPop"][size][environment];
    }

    double MaxHealthMod(PlanetEnvironment environment) {
        return PlanetDataTables()["PlanetEnvHealthMod"][0][environment];
    }

    void GrowMeter(Meter* meter, double updated_current_construction) {
        // TODO: something better here, likely depending on meter type
        assert(meter);
        double initial_current =    meter->InitialCurrent();

        double delta =              updated_current_construction / (10.0 + initial_current);

        meter->AdjustCurrent(delta);
    }

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
    PopCenter(0),
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
    GG::Connect(ResourceCenter::GetObjectSignal,    &Planet::This, this);
    GG::Connect(PopCenter::GetObjectSignal,         &Planet::This, this);
    // assumes PopCenter and ResourceCenter don't need to be initialized, due to having been re-created
    // in functional form by deserialization.  Also assumes planet-specific meters don't need to be re-added.
}

Planet::Planet(PlanetType type, PlanetSize size) :
    UniverseObject(),
    PopCenter(0),
    ResourceCenter(),
    m_type(PT_TERRAN),
    m_size(SZ_MEDIUM),
    m_orbital_period(1.0),
    m_initial_orbital_position(RandZeroToOne() * 2 * 3.14159),
    m_rotational_period(1.0),
    m_axial_tilt(RandZeroToOne() * HIGH_TILT_THERSHOLD),
    m_available_trade(0.0),
    m_just_conquered(false),
    m_is_about_to_be_colonized(false)
{
    GG::Connect(ResourceCenter::GetObjectSignal,    &Planet::This, this);
    GG::Connect(PopCenter::GetObjectSignal,         &Planet::This, this);
    UniverseObject::Init();
    PopCenter::Init(MaxPopMod(size, Environment(type)), MaxHealthMod(Environment(type)));
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

void Planet::Init() {
    InsertMeter(METER_SUPPLY, Meter());
    InsertMeter(METER_SHIELD, Meter());
    InsertMeter(METER_DEFENSE, Meter());
    InsertMeter(METER_DETECTION, Meter());
}

PlanetEnvironment Planet::Environment() const
{ return Environment(m_type); }

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
    Universe& universe = GetUniverse();
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        Building* building = universe.Object<Building>(*it);
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
    Universe& universe = GetUniverse();
    std::vector<UniverseObject*> retval;
    // add buildings on this planet
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it)
        retval.push_back(universe.Object(*it));
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

double Planet::ProjectedCurrentMeter(MeterType type) const
{
    const Meter* original_meter = 0;
    Meter meter;

    switch (type) {
    case METER_FARMING:
    case METER_MINING:
    case METER_INDUSTRY:
    case METER_RESEARCH:
    case METER_TRADE:
    case METER_CONSTRUCTION:
        return ResourceCenter::ProjectedCurrentMeter(type);
        break;
    case METER_POPULATION:
    case METER_HEALTH:
        return PopCenter::ProjectedCurrentMeter(type);
        break;
    case METER_SUPPLY:
    case METER_SHIELD:
    case METER_DEFENSE:
    case METER_DETECTION:
        original_meter = GetMeter(type);
        assert(original_meter);
        meter = Meter(*original_meter);
        GrowMeter(&meter, ProjectedCurrentMeter(METER_CONSTRUCTION));
        meter.Clamp();
        return meter.Current();
        break;
    default:
        return UniverseObject::ProjectedCurrentMeter(type);
        break;
    }
}

double Planet::MeterPoints(MeterType type) const
{
    switch (type) {
    case METER_FARMING:
    case METER_MINING:
    case METER_INDUSTRY:
    case METER_RESEARCH:
    case METER_TRADE:
    case METER_CONSTRUCTION:
        return ResourceCenter::MeterPoints(type);
        break;
    case METER_POPULATION:
    case METER_HEALTH:
        return PopCenter::MeterPoints(type);
        break;
    case METER_SUPPLY:
    case METER_SHIELD:
    case METER_DEFENSE:
    case METER_DETECTION:
        return GetMeter(type)->InitialCurrent();
        break;
    default:
        return UniverseObject::MeterPoints(type);
        break;
    }
}

double Planet::ProjectedMeterPoints(MeterType type) const
{
    switch (type) {
    case METER_FARMING:
    case METER_MINING:
    case METER_INDUSTRY:
    case METER_RESEARCH:
    case METER_TRADE:
    case METER_CONSTRUCTION:
        return ResourceCenter::ProjectedMeterPoints(type);
        break;
    case METER_POPULATION:
    case METER_HEALTH:
        return PopCenter::ProjectedMeterPoints(type);
        break;
    case METER_SUPPLY:
    case METER_SHIELD:
    case METER_DEFENSE:
    case METER_DETECTION:
        return ProjectedCurrentMeter(type);
    default:
        return UniverseObject::ProjectedMeterPoints(type);
        break;
    }
}

void Planet::SetType(PlanetType type)
{
    if (type <= INVALID_PLANET_TYPE)
        type = PT_SWAMP;
    if (NUM_PLANET_TYPES <= type)
        type = PT_GASGIANT;
    double old_farming_modifier = PlanetDataTables()["PlanetEnvFarmingMod"][0][Environment(m_type)];
    double new_farming_modifier = PlanetDataTables()["PlanetEnvFarmingMod"][0][Environment(type)];
    GetMeter(METER_FARMING)->AdjustMax(new_farming_modifier - old_farming_modifier);
    double old_health_modifier = PlanetDataTables()["PlanetEnvHealthMod"][0][Environment(m_type)];
    double new_health_modifier = PlanetDataTables()["PlanetEnvHealthMod"][0][Environment(type)];
    GetMeter(METER_HEALTH)->AdjustMax(new_health_modifier - old_health_modifier);
    double old_population_modifier = PlanetDataTables()["PlanetMaxPop"][m_size][Environment(m_type)];
    double new_population_modifier = PlanetDataTables()["PlanetMaxPop"][m_size][Environment(type)];
    GetMeter(METER_POPULATION)->AdjustMax(new_population_modifier - old_population_modifier);
    m_type = type;
    StateChangedSignal();
}

void Planet::SetSize(PlanetSize size)
{
    if (size <= SZ_NOWORLD)
        size = SZ_TINY;
    if (NUM_PLANET_SIZES <= size)
        size = SZ_GASGIANT;
    double old_industry_modifier = PlanetDataTables()["PlanetSizeIndustryMod"][0][m_size];
    double new_industry_modifier = PlanetDataTables()["PlanetSizeIndustryMod"][0][size];
    GetMeter(METER_INDUSTRY)->AdjustMax(new_industry_modifier - old_industry_modifier);
    double old_population_modifier = PlanetDataTables()["PlanetMaxPop"][m_size][Environment(m_type)];
    double new_population_modifier = PlanetDataTables()["PlanetMaxPop"][size][Environment(m_type)];
    GetMeter(METER_POPULATION)->AdjustMax(new_population_modifier - old_population_modifier);
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
    Logger().debugStream() << "Planet " << this->Name() << " adding building: " << building_id;
    if (Building* building = GetUniverse().Object<Building>(building_id)) {
        if (System* system = GetSystem()) {
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
    GetSystem()->UniverseObject::AddOwner(id);
    UniverseObject::AddOwner(id);
}

void Planet::RemoveOwner(int id)
{
    System* system = GetSystem();

    // check if Empire(id) is owner of at least one other planet in same system
    std::vector<Planet*> planets = system->FindObjects<Planet>();
    int empire_owned_planets_in_this_planet_system = 0;
    for (std::vector<Planet*>::const_iterator plt_it = planets.begin(); plt_it != planets.end(); ++plt_it)
        if ((*plt_it)->OwnedBy(id))
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
    PopCenter::Reset(MaxPopMod(Size(), Environment(Type())), MaxHealthMod(Environment(Type())));

    // reset resourcecenter meters
    ResourceCenter::Reset();

    // reset planet meters
    GetMeter(METER_SUPPLY)->ResetMax();
    GetMeter(METER_SHIELD)->ResetMax();
    GetMeter(METER_DEFENSE)->ResetMax();
    GetMeter(METER_DETECTION)->ResetMax();

    // reset buildings
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it)
        if (Building* building = GetUniverse().Object<Building>(*it))
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
    if (!empire)
        throw std::invalid_argument("Planet::Conquer: attempted to conquer a planet with an invalid conquerer.");

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
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        UniverseObject* obj = GetUniverse().Object(*it);
        assert(obj);
        obj->SetSystem(sys);
    }
}

void Planet::MoveTo(double x, double y)
{
    //Logger().debugStream() << "Planet::MoveTo(double x, double y)";
    // move planet itself
    UniverseObject::MoveTo(x, y);
    // move buildings
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        UniverseObject* obj = GetUniverse().Object(*it);
        assert(obj);
        obj->UniverseObject::MoveTo(x, y);
    }
}

void Planet::MovementPhase()
{
}

void Planet::ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type)
{
    ResourceCenter::ApplyUniverseTableMaxMeterAdjustments(meter_type);
    PopCenter::ApplyUniverseTableMaxMeterAdjustments(meter_type);

    // give planets base stealth slightly above zero, so that they can't be seen from a distance without high detection ability
    if (meter_type == INVALID_METER_TYPE || meter_type == METER_STEALTH)
        if (Meter* stealth = GetMeter(METER_STEALTH))
            stealth->AdjustMax(5.0);
}

void Planet::PopGrowthProductionResearchPhase()
{
    // do not do production if planet was just conquered
    if (m_just_conquered)
        m_just_conquered = false;
    else
        ResourceCenter::PopGrowthProductionResearchPhase();

    PopCenter::PopGrowthProductionResearchPhase();

    // check for starvation
    if (GetPopMeter()->Current() < PopCenter::MINIMUM_POP_CENTER_POPULATION) {
        Reset();
    } else {
        double current_construction = GetMeter(METER_CONSTRUCTION)->Current();  // want current construction, that has been updated from initial current construction
        GrowMeter(GetMeter(METER_SUPPLY),       current_construction);
        GrowMeter(GetMeter(METER_SHIELD),       current_construction);
        GrowMeter(GetMeter(METER_DEFENSE),      current_construction);
        GrowMeter(GetMeter(METER_DETECTION),    current_construction);
    }

    StateChangedSignal();
}

PlanetEnvironment Planet::Environment(PlanetType type)
{
    switch (type)
    {
    case PT_INFERNO:
    case PT_RADIATED:
    case PT_TOXIC:
    case PT_BARREN:     return PE_HOSTILE;      // 3 or more away from EP
    case PT_SWAMP:
    case PT_TUNDRA:     return PE_POOR;         // 2 away from EP
    case PT_DESERT:
    case PT_OCEAN:      return PE_ADEQUATE;     // 1 away form EP
    case PT_TERRAN:     return PE_GOOD;         // EP
    case PT_ASTEROIDS:
    case PT_GASGIANT:
    default:            return PE_UNINHABITABLE;// out of the loop
    }
}

std::set<int> Planet::VisibleContainedObjects(int empire_id) const
{
    std::set<int> retval;
    Universe& universe = GetUniverse();
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
