#include "Building.h"

#include "Effect.h"
#include "Condition.h"
#include "Planet.h"
#include "Predicates.h"
#include "Universe.h"
#include "Enums.h"
#include "Parser.h"
#include "ParserUtil.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"

#include <boost/filesystem/fstream.hpp>

std::string DumpIndent();

extern int g_indent;

namespace {
    const bool CHEAP_AND_FAST_BUILDING_PRODUCTION = false;    // makes all buildings cost 1 PP and take 1 turn to build
}

namespace {
    struct store_building_type_impl
    {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, BuildingType*>& building_types, const T& building_type) const
        {
            if (building_types.find(building_type->Name()) != building_types.end()) {
                std::string error_str = "ERROR: More than one building type in buildings.txt has the name " + building_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
            building_types[building_type->Name()] = building_type;
        }
    };

    const phoenix::function<store_building_type_impl> store_building_type_;
}

/////////////////////////////////////////////////
// Building                                    //
/////////////////////////////////////////////////
Building::Building() :
    UniverseObject(),
    m_building_type(""),
    m_planet_id(INVALID_OBJECT_ID)
{}

Building::Building(int empire_id, const std::string& building_type, int planet_id) :
    UniverseObject(),
    m_building_type(building_type),
    m_planet_id(planet_id)
{
    AddOwner(empire_id);
    const BuildingType* type = GetBuildingType();
    if (type)
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_BUILDING"));

    UniverseObject::Init();
}

const BuildingType* Building::GetBuildingType() const
{
    return ::GetBuildingType(m_building_type);
}

const std::string& Building::BuildingTypeName() const
{
    return m_building_type;
}

int Building::PlanetID() const
{
    return m_planet_id;
}

Planet* Building::GetPlanet() const
{
    return m_planet_id == INVALID_OBJECT_ID ? 0 : GetUniverse().Object<Planet>(m_planet_id);
}

Visibility Building::GetVisibility(int empire_id) const {
    const Planet* planet = GetPlanet();
    if (planet)
        return planet->GetVisibility(empire_id);
    else
        return VIS_NO_VISIBILITY;
}

UniverseObject* Building::Accept(const UniverseObjectVisitor& visitor) const
{
    return visitor.Visit(const_cast<Building* const>(this));
}

void Building::SetPlanetID(int planet_id)
{
    if (Planet* planet = GetPlanet())
        planet->RemoveBuilding(ID());
    m_planet_id = planet_id;
}

void Building::MoveTo(double x, double y)
{
    UniverseObject::MoveTo(x, y);

    // if building is being moved away from its planet, remove from the planet.  otherwise, keep building on planet
    if (Planet* planet = GetPlanet())
        planet->RemoveBuilding(this->ID());
}

void Building::MovementPhase()
{}

void Building::ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type/* = INVALID_METER_TYPE*/)
{
    // give buildings base stealth slightly above 0, so that they can't be seen from a distance without high detection ability
    if (meter_type == INVALID_METER_TYPE || meter_type == METER_STEALTH)
        if (Meter* stealth = GetMeter(METER_STEALTH))
            stealth->AdjustMax(0.001);
}

void Building::PopGrowthProductionResearchPhase()
{}

void Building::Reset()
{
    ClearOwners();
}

/////////////////////////////////////////////////
// BuildingType                                //
/////////////////////////////////////////////////
BuildingType::BuildingType() :
    m_name(""),
    m_description(""),
    m_build_cost(0.0),
    m_build_time(0),
    m_maintenance_cost(0.0),
    m_location(0),
    m_effects(0),
    m_graphic("")
{}

BuildingType::BuildingType(const std::string& name, const std::string& description,
                           double build_cost, int build_time, double maintenance_cost,
                           const Condition::ConditionBase* location,
                           const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
                           const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_build_cost(build_cost),
    m_build_time(build_time),
    m_maintenance_cost(maintenance_cost),
    m_location(location),
    m_effects(effects),
    m_graphic(graphic)
{}

BuildingType::~BuildingType()
{ delete m_location; }

const std::string& BuildingType::Name() const
{
    return m_name;
}

const std::string& BuildingType::Description() const
{
    return m_description;
}

std::string BuildingType::Dump() const
{
    using boost::lexical_cast;

    std::string retval = DumpIndent() + "BuildingType\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    retval += DumpIndent() + "buildcost = " + lexical_cast<std::string>(m_build_cost) + "\n";
    retval += DumpIndent() + "buildtime = " + lexical_cast<std::string>(m_build_time) + "\n";
    retval += DumpIndent() + "maintenancecost = " + lexical_cast<std::string>(m_maintenance_cost) + "\n";
    if (m_effects.size() == 1) {
        retval += DumpIndent() + "effectsgroups =\n";
        ++g_indent;
        retval += m_effects[0]->Dump();
        --g_indent;
    } else {
        retval += DumpIndent() + "effectsgroups = [\n";
        ++g_indent;
        for (unsigned int i = 0; i < m_effects.size(); ++i) {
            retval += m_effects[i]->Dump();
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

double BuildingType::BuildCost() const
{
    if (!CHEAP_AND_FAST_BUILDING_PRODUCTION)
        return m_build_cost;
    else
        return 1.0;
}

int BuildingType::BuildTime() const
{
    if (!CHEAP_AND_FAST_BUILDING_PRODUCTION)
        return m_build_time;
    else
        return 1.0;
}

double BuildingType::MaintenanceCost() const
{
    return m_maintenance_cost;
}

const Condition::ConditionBase* BuildingType::Location() const {
    return m_location;
}
const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& BuildingType::Effects() const
{
    return m_effects;
}

const std::string& BuildingType::Graphic() const
{
    return m_graphic;
}

bool BuildingType::ProductionLocation(int empire_id, int location_id) const {
    Condition::ObjectSet locations;
    Condition::ObjectSet non_locations;

    Universe& universe = GetUniverse();

    UniverseObject* loc = universe.Object(location_id);
    if (!loc) return false;

    Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().debugStream() << "BuildingType::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    UniverseObject* source = universe.Object(empire->CapitolID());
    if (!source) return false;

    locations.insert(loc);

    m_location->Eval(source, locations, non_locations, Condition::TARGETS);

    return !(locations.empty());
}

CaptureResult BuildingType::GetCaptureResult(const std::set<int>& from_empire_ids, int to_empire_id,
                                             int location_id, bool as_production_item) const
{
    Empire*         to_empire =     Empires().Lookup(to_empire_id);
    UniverseObject* location =      GetUniverse().Object(location_id);

    if (as_production_item && location && to_empire)
        return CR_CAPTURE;

    return CR_CAPTURE;
}

/////////////////////////////////////////////////
// BuildingTypeManager                         //
/////////////////////////////////////////////////
// static(s)
BuildingTypeManager* BuildingTypeManager::s_instance = 0;

BuildingTypeManager::BuildingTypeManager()
{
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one BuildingTypeManager.");

    s_instance = this;

    std::string file_name = "buildings.txt";
    std::string input;

    boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
    if (ifs) {
        std::getline(ifs, input, '\0');
        ifs.close();
    } else {
        Logger().errorStream() << "Unable to open data file " << file_name;
        return;
    }

    using namespace boost::spirit;
    using namespace phoenix;
    parse_info<const char*> result =
        parse(input.c_str(),
              as_lower_d[*building_type_p[store_building_type_(var(m_building_types), arg1)]]
              >> end_p,
              skip_p);
    if (!result.full)
        ReportError(input.c_str(), result);
}

BuildingTypeManager::~BuildingTypeManager()
{
    for (std::map<std::string, BuildingType*>::iterator it = m_building_types.begin(); it != m_building_types.end(); ++it) {
        delete it->second;
    }
}

const BuildingType* BuildingTypeManager::GetBuildingType(const std::string& name) const
{
    std::map<std::string, BuildingType*>::const_iterator it = m_building_types.find(name);
    return it != m_building_types.end() ? it->second : 0;
}

BuildingTypeManager& BuildingTypeManager::GetBuildingTypeManager()
{
    static BuildingTypeManager manager;
    return manager;
}

BuildingTypeManager::iterator BuildingTypeManager::begin() const
{
    return m_building_types.begin();
}

BuildingTypeManager::iterator BuildingTypeManager::end() const
{
    return m_building_types.end();
}


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
BuildingTypeManager& GetBuildingTypeManager()
{
    return BuildingTypeManager::GetBuildingTypeManager();
}

const BuildingType* GetBuildingType(const std::string& name)
{
    return GetBuildingTypeManager().GetBuildingType(name);
}
