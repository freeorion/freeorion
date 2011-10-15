#include "Building.h"

#include "Effect.h"
#include "Condition.h"
#include "Planet.h"
#include "Predicates.h"
#include "Universe.h"
#include "Enums.h"
#include "Parser.h"
#include "ParserUtil.h"
#include "ValueRef.h"
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
    struct store_building_type_impl {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, BuildingType*>& building_types, const T& building_type) const {
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
    m_planet_id(INVALID_OBJECT_ID),
    m_ordered_scrapped(false),
    m_produced_by_empire_id(ALL_EMPIRES)
{}

Building::Building(int empire_id, const std::string& building_type, int produced_by_empire_id/* = ALL_EMPIRES*/) :
    UniverseObject(),
    m_building_type(building_type),
    m_planet_id(INVALID_OBJECT_ID),
    m_ordered_scrapped(false),
    m_produced_by_empire_id(produced_by_empire_id)
{
    SetOwner(empire_id);
    const BuildingType* type = GetBuildingType();
    if (type)
        Rename(UserString(type->Name()));
    else
        Rename(UserString("ENC_BUILDING"));

    UniverseObject::Init();
}

Building* Building::Clone(int empire_id) const
{
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return 0;

    Building* retval = new Building();
    retval->Copy(this, empire_id);
    return retval;
}

void Building::Copy(const UniverseObject* copied_object, int empire_id)
{
    if (copied_object == this)
        return;
    const Building* copied_building = universe_object_cast<Building*>(copied_object);
    if (!copied_building) {
        Logger().errorStream() << "Building::Copy passed an object that wasn't a Building";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_building_type =             copied_building->m_building_type;
        this->m_planet_id =                 copied_building->m_planet_id;

        if (vis >= VIS_FULL_VISIBILITY) {
            this->m_ordered_scrapped =      copied_building->m_ordered_scrapped;
            this->m_produced_by_empire_id = copied_building->m_produced_by_empire_id;
        }
    }
}

const std::string& Building::TypeName() const
{ return UserString("BUILDING"); }

std::string Building::Dump() const
{
    std::stringstream os;
    os << UniverseObject::Dump();
    os << " building type: " << m_building_type
       << " produced by empire id: " << m_produced_by_empire_id;
    return os.str();
}

const BuildingType* Building::GetBuildingType() const
{ return ::GetBuildingType(m_building_type); }

const std::string& Building::BuildingTypeName() const
{ return m_building_type; }

int Building::PlanetID() const
{ return m_planet_id; }

int Building::ProducedByEmpireID() const
{ return m_produced_by_empire_id; }

UniverseObject* Building::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(const_cast<Building* const>(this)); }

void Building::SetPlanetID(int planet_id)
{
    if (Planet* planet = GetObject<Planet>(m_planet_id))
        planet->RemoveBuilding(this->ID());
    m_planet_id = planet_id;
}

void Building::MoveTo(double x, double y)
{
    //Logger().debugStream() << "Building::MoveTo(" << x << ", " << y << ")";
    UniverseObject::MoveTo(x, y);

    // if building is being moved away from its planet, remove from the planet.  otherwise, keep building on planet
    if (Planet* planet = GetObject<Planet>(m_planet_id))
        planet->RemoveBuilding(this->ID());
}

void Building::ResetTargetMaxUnpairedMeters(MeterType meter_type/* = INVALID_METER_TYPE*/)
{
    UniverseObject::ResetTargetMaxUnpairedMeters(meter_type);

    // give buildings base stealth slightly above 0, so that they can't be seen from a distance without high detection ability
    if (meter_type == INVALID_METER_TYPE || meter_type == METER_STEALTH)
        if (Meter* stealth = GetMeter(METER_STEALTH))
            stealth->AddToCurrent(0.01);
}

void Building::Reset()
{
    SetOwner(ALL_EMPIRES);
    m_ordered_scrapped = false;
}

void Building::SetOrderedScrapped(bool b)
{
    bool initial_status = m_ordered_scrapped;
    if (b == initial_status) return;
    m_ordered_scrapped = b;
    StateChangedSignal();
}

/////////////////////////////////////////////////
// BuildingType                                //
/////////////////////////////////////////////////
BuildingType::BuildingType() :
    m_name(""),
    m_description(""),
    m_production_cost(0.0),
    m_production_time(0),
    m_capture_result(CR_DESTROY),
    m_location(0),
    m_effects(0),
    m_graphic("")
{}

BuildingType::BuildingType(const std::string& name, const std::string& description,
                           double production_cost, int production_time, bool producible,
                           CaptureResult capture_result, const Condition::ConditionBase* location,
                           const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
                           const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_production_cost(production_cost),
    m_production_time(production_time),
    m_producible(producible),
    m_capture_result(capture_result),
    m_location(location),
    m_effects(effects),
    m_graphic(graphic)
{}

BuildingType::~BuildingType()
{ delete m_location; }

const std::string& BuildingType::Name() const
{ return m_name; }

const std::string& BuildingType::Description() const
{ return m_description; }

std::string BuildingType::Dump() const
{
    using boost::lexical_cast;

    std::string retval = DumpIndent() + "BuildingType\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    retval += DumpIndent() + "buildcost = " + lexical_cast<std::string>(m_production_cost) + "\n";
    retval += DumpIndent() + "buildtime = " + lexical_cast<std::string>(m_production_time) + "\n";
    retval += DumpIndent() + "captureresult = " + lexical_cast<std::string>(m_capture_result) + "\n";
    retval += DumpIndent() + "location = \n";
    ++g_indent;
    retval += m_location->Dump();
    --g_indent;
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

double BuildingType::ProductionCost() const
{
    if (!CHEAP_AND_FAST_BUILDING_PRODUCTION)
        return m_production_cost;
    else
        return 1.0;
}

double BuildingType::PerTurnCost() const
{ return ProductionCost() / std::max(1, ProductionTime()); }

int BuildingType::ProductionTime() const
{
    if (!CHEAP_AND_FAST_BUILDING_PRODUCTION)
        return m_production_time;
    else
        return 1;
}

bool BuildingType::Producible() const
{ return m_producible; }

double BuildingType::MaintenanceCost() const
{ return 0.0; }

const Condition::ConditionBase* BuildingType::Location() const
{ return m_location; }

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& BuildingType::Effects() const
{ return m_effects; }

const std::string& BuildingType::Graphic() const
{ return m_graphic; }

bool BuildingType::ProductionLocation(int empire_id, int location_id) const {
    const ObjectMap& objects = GetMainObjectMap();

    const UniverseObject* location = objects.Object(location_id);
    if (!location) return false;

    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().debugStream() << "BuildingType::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    // get a source object, which is owned by the empire with the passed-in
    // empire id.  this is used in conditions to reference which empire is
    // doing the building.  Ideally this will be the capital, but any object
    // owned by the empire will work.
    const UniverseObject* source = objects.Object(empire->CapitalID());
    if (!source && location->OwnedBy(empire_id))
        source = location;
    // still no valid source?!  scan through all objects to find one owned by this empire
    if (!source) {
        for (ObjectMap::const_iterator obj_it = objects.const_begin(); obj_it != objects.const_end(); ++obj_it) {
            if (obj_it->second->OwnedBy(empire_id)) {
                source = obj_it->second;
                break;
            }
        }
    }
    // if this empire doesn't own ANYTHING, then how is it building anyway?
    if (!source)
        return false;

    ScriptingContext sc(source);

    Condition::ObjectSet potential_targets;
    potential_targets.reserve(RESERVE_SET_SIZE);
    potential_targets.push_back(location);
    Condition::ObjectSet matched_targets;
    matched_targets.reserve(RESERVE_SET_SIZE);
    m_location->Eval(sc, matched_targets, potential_targets);

    return !matched_targets.empty();
}

CaptureResult BuildingType::GetCaptureResult(int from_empire_id, int to_empire_id,
                                             int location_id, bool as_production_item) const
{ return m_capture_result; }

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

    using namespace boost::spirit::classic;
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
{ return m_building_types.begin(); }

BuildingTypeManager::iterator BuildingTypeManager::end() const
{ return m_building_types.end(); }


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
BuildingTypeManager& GetBuildingTypeManager()
{ return BuildingTypeManager::GetBuildingTypeManager(); }

const BuildingType* GetBuildingType(const std::string& name)
{ return GetBuildingTypeManager().GetBuildingType(name); }
