#include "Planet.h"

#include "../util/AppInterface.h"
#include "Building.h"
#include "../util/DataTable.h"
#include "Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "Predicates.h"
#include "../server/ServerApp.h"
#include "Ship.h"
#include "System.h"

#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "../util/AppInterface.h"
#include <stdexcept>

namespace {
    DataTableMap& PlanetDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/planet_tables.txt", map);
        }
        return map;
    }

    double MaxPopMod(PlanetSize size, PlanetEnvironment environment)
    {
        return PlanetDataTables()["PlanetMaxPop"][size][environment];
    }

    double MaxHealthMod(PlanetEnvironment environment)
    {
        return PlanetDataTables()["PlanetEnvHealthMod"][0][environment];
    }


    bool temp_header_bool = RecordHeaderFile(PlanetRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

Planet::Planet() :
    UniverseObject(),
    PopCenter(this, MaxPopMod(SZ_MEDIUM, Environment(PT_TERRAN)), MaxHealthMod(Environment(PT_TERRAN))),
    ProdCenter(PopCenter::PopulationMeter(), this),
    m_type(PT_TERRAN),
    m_size(SZ_MEDIUM),
    m_available_trade(0.0),
    m_just_conquered(false),
    m_is_about_to_be_colonized(0)
{
}

Planet::Planet(PlanetType type, PlanetSize size) :
    UniverseObject(),
    PopCenter(this, MaxPopMod(size, Environment(type)), MaxHealthMod(Environment(type))),
    ProdCenter(PopCenter::PopulationMeter(), this),
    m_type(PT_TERRAN),
    m_size(SZ_MEDIUM),
    m_available_trade(0.0),
    m_just_conquered(false),
    m_is_about_to_be_colonized(0)
{
    SetType(type);
    SetSize(size);
    m_def_bases = 0;
}

Planet::Planet(const GG::XMLElement& elem) :
    UniverseObject(elem.Child("UniverseObject")),
    PopCenter(elem.Child("PopCenter"), this),
    ProdCenter(elem.Child("ProdCenter"), PopCenter::PopulationMeter(), this),
    m_is_about_to_be_colonized(0),
    m_def_bases(0)
{
    using GG::XMLElement;

    if (elem.Tag().find( "Planet" ) == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Planet from an XMLElement that had a tag other than \"Planet\"");

    try {
        m_type = lexical_cast<PlanetType>(elem.Child("m_type").Text());
        m_size = lexical_cast<PlanetSize>(elem.Child("m_size").Text());
        m_just_conquered = lexical_cast<bool>(elem.Child("m_just_conquered").Text());

        Visibility vis = Visibility(lexical_cast<int>(elem.Child("UniverseObject").Child("vis").Text()));
        if (vis == FULL_VISIBILITY) {
            m_def_bases = lexical_cast<int>(elem.Child("m_def_bases").Text());
            m_is_about_to_be_colonized = lexical_cast<int>(elem.Child("m_is_about_to_be_colonized").Text());
            m_buildings = GG::ContainerFromString<std::set<int> >(elem.Child("m_buildings").Text());
            m_available_trade  = lexical_cast<double>(elem.Child("m_available_trade").Text());
        }
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in Planet::Planet(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
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
            BuildingType* bulding_type = GetBuildingType(building->BuildingTypeName());
            retval += bulding_type->MaintenanceCost();
        }
    }
    return retval;
}

const Meter* Planet::GetMeter(MeterType type) const
{
    switch (type) {
    case METER_POPULATION:
        return &PopulationMeter();
        break;
    case METER_FARMING:
        return &FarmingMeter();
        break;
    case METER_INDUSTRY:
        return &IndustryMeter();
        break;
    case METER_RESEARCH:
        return &ResearchMeter();
        break;
    case METER_TRADE:
        return &TradeMeter();
        break;
    case METER_MINING:
        return &MiningMeter();
        break;
    case METER_CONSTRUCTION:
        return &ConstructionMeter();
        break;
    case METER_HEALTH:
        return &HealthMeter();
        break;
    default:
        break;
    }
    return 0;
}

UniverseObject::Visibility Planet::GetVisibility(int empire_id) const
{
    // use the containing system's visibility
    return GetSystem()->GetVisibility(empire_id);
}

GG::XMLElement Planet::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
    // Partial encoding of Planet for limited visibility
    using GG::XMLElement;
    using boost::lexical_cast;
    using std::string;

    Visibility vis= GetVisibility(empire_id);
    if (empire_id == Universe::ALL_EMPIRES)
        vis = FULL_VISIBILITY;

    XMLElement retval("Planet" + boost::lexical_cast<std::string>(ID()));
    retval.AppendChild(UniverseObject::XMLEncode(empire_id));
    retval.AppendChild(PopCenter::XMLEncode(vis));
    retval.AppendChild(ProdCenter::XMLEncode(vis));
    retval.AppendChild(XMLElement("m_type", lexical_cast<std::string>(m_type)));
    retval.AppendChild(XMLElement("m_size", lexical_cast<std::string>(m_size)));
    retval.AppendChild(XMLElement("m_just_conquered", lexical_cast<std::string>(m_just_conquered)));
    if (vis == FULL_VISIBILITY) {
        retval.AppendChild(XMLElement("m_def_bases", lexical_cast<std::string>(m_def_bases)));
        retval.AppendChild(XMLElement("m_is_about_to_be_colonized", lexical_cast<std::string>(m_is_about_to_be_colonized)));
        retval.AppendChild(XMLElement("m_buildings", GG::StringFromContainer<std::set<int> >(m_buildings)));
        retval.AppendChild(XMLElement("m_available_trade", lexical_cast<std::string>(m_available_trade)));
    }
    return retval;
}

UniverseObject* Planet::Accept(const UniverseObjectVisitor& visitor) const
{
    return visitor.Visit(const_cast<Planet* const>(this));
}

void Planet::SetType(PlanetType type)
{
    if (type <= INVALID_PLANET_TYPE)
        type = PT_SWAMP;
    if (NUM_PLANET_TYPES <= type)
        type = PT_GASGIANT;
    double old_modifier = PlanetDataTables()["PlanetEnvFarmingMod"][0][Environment(m_type)];
    double new_modifier = PlanetDataTables()["PlanetEnvFarmingMod"][0][Environment(type)];
    GetMeter(METER_FARMING)->AdjustMax(new_modifier - old_modifier);
    m_type = type;
    StateChangedSignal()();
}

void Planet::SetSize(PlanetSize size)
{
    if (size <= SZ_NOWORLD)
        size = SZ_TINY;
    if (NUM_PLANET_SIZES <= size)
        size = SZ_GASGIANT;
    double old_modifier = PlanetDataTables()["PlanetSizeIndustryMod"][0][m_size];
    double new_modifier = PlanetDataTables()["PlanetSizeIndustryMod"][0][size];
    GetMeter(METER_INDUSTRY)->AdjustMax(new_modifier - old_modifier);
    m_size = size;
    StateChangedSignal()();
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
    StateChangedSignal()();
}

bool Planet::RemoveBuilding(int building_id)
{
    if (m_buildings.find(building_id) != m_buildings.end()) {
        if (Building* building = GetUniverse().Object<Building>(building_id)) {
            building->SetPlanetID(INVALID_OBJECT_ID);
            building->SetSystem(INVALID_OBJECT_ID);
        }
        m_buildings.erase(building_id);
        StateChangedSignal()();
        return true;
    }
    return false;
}

bool Planet::DeleteBuilding(int building_id)
{
    if (m_buildings.find(building_id) != m_buildings.end()) {
        GetUniverse().Delete(building_id);
        m_buildings.erase(building_id);
        StateChangedSignal()();
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
    System *system=GetSystem();
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

void Planet::Conquer(int conquerer)
{
    m_just_conquered = true;

    // RemoveOwner will change owners - without temp_owner => side effect
    std::set<int> temp_owner(Owners());
    for(std::set<int>::const_iterator own_it = temp_owner.begin();own_it != temp_owner.end();++own_it)
        RemoveOwner(*own_it);

    AddOwner(conquerer);
}

void Planet::SetIsAboutToBeColonized(bool b)
{
    bool overall_status = m_is_about_to_be_colonized;
    b ? ++m_is_about_to_be_colonized : --m_is_about_to_be_colonized;
    if (m_is_about_to_be_colonized < 0)
        m_is_about_to_be_colonized = 0;
    if (overall_status != static_cast<bool>(m_is_about_to_be_colonized))
        StateChangedSignal()();
}

void Planet::ResetIsAboutToBeColonized()
{
    m_is_about_to_be_colonized = 0;
}

Meter* Planet::GetMeter(MeterType type)
{
    switch (type) {
    case METER_POPULATION:
        return &PopulationMeter();
        break;
    case METER_FARMING:
        return &FarmingMeter();
        break;
    case METER_INDUSTRY:
        return &IndustryMeter();
        break;
    case METER_RESEARCH:
        return &ResearchMeter();
        break;
    case METER_TRADE:
        return &TradeMeter();
        break;
    case METER_MINING:
        return &MiningMeter();
        break;
    case METER_CONSTRUCTION:
        return &ConstructionMeter();
        break;
    case METER_HEALTH:
        return &HealthMeter();
        break;
    default:
        break;
    }
    return 0;
}

void Planet::MovementPhase()
{
}

void Planet::AdjustMaxMeters()
{
    ProdCenter::AdjustMaxMeters();
    PopCenter::AdjustMaxMeters();
}

void Planet::PopGrowthProductionResearchPhase( )
{
    // do not do production if planet was just conquered
    if (m_just_conquered)
        m_just_conquered = false;
    else
        ProdCenter::PopGrowthProductionResearchPhase();

    PopCenter::PopGrowthProductionResearchPhase();

    StateChangedSignal()();
}

PlanetEnvironment Planet::Environment(PlanetType type)
{
    switch (type)
    {
    case PT_ASTEROIDS:
    case PT_GASGIANT:   return PE_UNINHABITABLE;
    case PT_SWAMP:
    case PT_TOXIC:
    case PT_INFERNO:
    case PT_RADIATED:
    case PT_BARREN:
    case PT_TUNDRA:     return PE_TERRIBLE;
    case PT_DESERT:
    case PT_OCEAN:      return PE_ADEQUATE;
    case PT_TERRAN:     return PE_SUPERB;
    case PT_GAIA:       return PE_OPTIMAL;
    default:            return PE_UNINHABITABLE;
    }
}
