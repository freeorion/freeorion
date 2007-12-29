#include "Planet.h"

#include "../util/AppInterface.h"
#include "Building.h"
#include "../util/DataTable.h"
#include "Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "Predicates.h"
#include "Ship.h"
#include "System.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>


using boost::lexical_cast;

namespace {
    DataTableMap& PlanetDataTables() {
        static DataTableMap map;
        if (map.empty()) {
            std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
            if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
                settings_dir += '/';
            LoadDataTables(settings_dir + "planet_tables.txt", map);
        }
        return map;
    }

    double MaxPopMod(PlanetSize size, PlanetEnvironment environment) {
        return PlanetDataTables()["PlanetMaxPop"][size][environment];
    }

    double MaxHealthMod(PlanetEnvironment environment) {
        return PlanetDataTables()["PlanetEnvHealthMod"][0][environment];
    }

    void GrowPlanetMeters(Meter* meter, double updated_current_construction) {
        // TODO: something better here...
        assert(meter);
        double delta = updated_current_construction / (10.0 + meter->Current());
        double new_cur = std::min(meter->Max(), meter->Current() + delta);
        meter->SetCurrent(new_cur);
    }
}

Planet::Planet() :
    UniverseObject(),
    PopCenter(0),
    ResourceCenter(),
    m_type(PT_TERRAN),
    m_size(SZ_MEDIUM),
    m_available_trade(0.0),
    m_just_conquered(false),
    m_is_about_to_be_colonized(false),
    m_def_bases(0)
{
    GG::Connect(ResourceCenter::GetObjectSignal, &Planet::This, this);
    GG::Connect(PopCenter::GetObjectSignal, &Planet::This, this);
    // assumes PopCenter and ResourceCenter don't need to be initialized, due to having been re-created
    // in functional form by deserialization.  Also assumes planet-specific meters don't need to be re-added.
}

Planet::Planet(PlanetType type, PlanetSize size) :
    UniverseObject(),
    PopCenter(0),
    ResourceCenter(),
    m_type(PT_TERRAN),
    m_size(SZ_MEDIUM),
    m_available_trade(0.0),
    m_just_conquered(false),
    m_is_about_to_be_colonized(false),
    m_def_bases(0)
{
    GG::Connect(ResourceCenter::GetObjectSignal, &Planet::This, this);
    GG::Connect(PopCenter::GetObjectSignal, &Planet::This, this);
    PopCenter::Init(MaxPopMod(size, Environment(type)), MaxHealthMod(Environment(type)));
    ResourceCenter::Init();
    Planet::Init();
    SetType(type);
    SetSize(size);
}

void Planet::Init() {
    InsertMeter(METER_SUPPLY, Meter());
    InsertMeter(METER_SHIELD, Meter());
    InsertMeter(METER_DEFENSE, Meter());
    InsertMeter(METER_DETECTION, Meter());
    InsertMeter(METER_STEALTH, Meter());
}

PlanetEnvironment Planet::Environment() const
{
    return Environment(m_type);
}

double Planet::AvailableTrade() const
{
    return m_available_trade;
}

double Planet::BuildingCosts() const
{
    double retval = 0.0;
    Universe& universe = GetUniverse();
    for (std::set<int>::const_iterator it = m_buildings.begin(); it != m_buildings.end(); ++it) {
        Building* building = universe.Object<Building>(*it);
        if (building->Operating()) {
            const BuildingType* bulding_type = GetBuildingType(building->BuildingTypeName());
            retval += bulding_type->MaintenanceCost();
        }
    }
    return retval;
}

UniverseObject::Visibility Planet::GetVisibility(int empire_id) const
{
    // use the containing system's visibility
    return GetSystem()->GetVisibility(empire_id);
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
    case METER_STEALTH:
        original_meter = GetMeter(type);
        assert(original_meter);
        meter = Meter(*original_meter);
        GrowPlanetMeters(&meter, ProjectedCurrentMeter(METER_CONSTRUCTION));
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
    case METER_STEALTH:
        return GetMeter(type)->Current();
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
    case METER_STEALTH:
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

void Planet::AddBuilding(int building_id)
{
    if (Building* building = GetUniverse().Object<Building>(building_id)) {
        building->SetPlanetID(ID());
        if (System* system = GetSystem()) {
            system->Insert(building);
        } else {
            building->SetSystem(SystemID());
        }
        building->MoveTo(X(), Y());
        m_buildings.insert(building_id);
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

bool Planet::DeleteBuilding(int building_id)
{
    if (m_buildings.find(building_id) != m_buildings.end()) {
        GetUniverse().Delete(building_id);
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

void Planet::AddOwner (int id)
{
    GetSystem()->UniverseObject::AddOwner(id);
    UniverseObject::AddOwner(id);
}

void Planet::RemoveOwner(int id)
{
    System *system = GetSystem();
    std::vector<Planet*> planets = system->FindObjects<Planet>();

    // check if Empire(id) is owner of at least one other planet
    std::vector<Planet*>::const_iterator plt_it;
    int count_planets = 0;
    for(plt_it=planets.begin();plt_it != planets.end();++plt_it)
        if((*plt_it)->Owners().find(id) != (*plt_it)->Owners().end())
            count_planets++;

    if(count_planets==1)
        system->UniverseObject::RemoveOwner(id);

    UniverseObject::RemoveOwner(id);
}

void Planet::Reset()
{
    std::set<int> owners = Owners();
    for (std::set<int>::iterator it = owners.begin(); it != owners.end(); ++it) {
        RemoveOwner(*it);
    }
    PopCenter::Reset(MaxPopMod(Size(), Environment(Type())), MaxHealthMod(Environment(Type())));
    ResourceCenter::Reset();
    m_buildings.clear();
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

    empire->ConquerBuildsAtLocation(ID());

    // TODO: Something with buildings on planet (not on queue, as above)

    // RemoveOwner will change owners, invalidating iterators over owners, so iterate over temp set
    std::set<int> temp_owner(Owners());
    for(std::set<int>::const_iterator own_it = temp_owner.begin(); own_it != temp_owner.end(); ++own_it) {
        // remove previous owner who has lost ownership due to other empire conquering planet
        RemoveOwner(*own_it);
    }

    AddOwner(conquerer);
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

void Planet::MovementPhase()
{
}

void Planet::ApplyUniverseTableMaxMeterAdjustments()
{
    ResourceCenter::ApplyUniverseTableMaxMeterAdjustments();
    PopCenter::ApplyUniverseTableMaxMeterAdjustments();
}

void Planet::PopGrowthProductionResearchPhase( )
{
    // do not do production if planet was just conquered
    if (m_just_conquered)
        m_just_conquered = false;
    else
        ResourceCenter::PopGrowthProductionResearchPhase();

    PopCenter::PopGrowthProductionResearchPhase();

    double current_construction = GetMeter(METER_CONSTRUCTION)->Current();
    GrowPlanetMeters(GetMeter(METER_SUPPLY), current_construction);
    GrowPlanetMeters(GetMeter(METER_SHIELD), current_construction);
    GrowPlanetMeters(GetMeter(METER_DEFENSE), current_construction);
    GrowPlanetMeters(GetMeter(METER_DETECTION), current_construction);
    GrowPlanetMeters(GetMeter(METER_STEALTH), current_construction);

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
