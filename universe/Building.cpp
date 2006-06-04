#include "Building.h"

#include "Effect.h"
#include "Condition.h"
#include "../universe/Parser.h"
#include "../universe/ParserUtil.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "Planet.h"
#include "Predicates.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "Universe.h"
#include "../util/AppInterface.h"

#include <fstream>
#include <iostream>

std::string DumpIndent();

extern int g_indent;

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

    // loads and stores BuildingTypes specified in [settings-dir]/buildings.txt
    class BuildingTypeManager
    {
    public:
        BuildingTypeManager()
        {
            std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
            if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
                settings_dir += '/';
            std::string filename = settings_dir + "buildings.txt";
            std::ifstream ifs(filename.c_str());
            std::string input;
            std::getline(ifs, input, '\0');
            ifs.close();
            using namespace boost::spirit;
            using namespace phoenix;
            parse_info<const char*> result =
                parse(input.c_str(),
                      as_lower_d[*building_type_p[store_building_type_(var(m_building_types), arg1)]],
                      skip_p);
            if (!result.full)
                ReportError(std::cerr, input.c_str(), result);
        }

        const BuildingType* GetBuildingType(const std::string& name) const
        {
            std::map<std::string, BuildingType*>::const_iterator it = m_building_types.find(name);
            return it != m_building_types.end() ? it->second : 0;
        }

    private:
        std::map<std::string, BuildingType*> m_building_types;
    };

}

Building::Building() :
    m_building_type(""),
    m_operating(true),
    m_planet_id(INVALID_OBJECT_ID)
{}

Building::Building(int empire_id, const std::string& building_type, int planet_id) :
    m_building_type(building_type),
    m_operating(true),
    m_planet_id(planet_id)
{
   AddOwner(empire_id);
}

Building::Building(const XMLElement& elem) :
    UniverseObject(elem.Child("UniverseObject"))
{
    if (elem.Tag().find("Building") == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Building from an XMLElement that had a tag other than \"Building\"");

    using boost::lexical_cast;

    m_building_type = elem.Child("m_building_type").Text();
    m_operating = lexical_cast<bool>(elem.Child("m_operating").Text());
    m_planet_id = lexical_cast<int>(elem.Child("m_planet_id").Text());
}

const BuildingType* Building::GetBuildingType() const
{
    return ::GetBuildingType(m_building_type);
}

const std::string& Building::BuildingTypeName() const
{
    return m_building_type;
}

bool Building::Operating() const
{
    return m_operating;
}

int Building::PlanetID() const
{
    return m_planet_id;
}

Planet* Building::GetPlanet() const
{
    return m_planet_id == INVALID_OBJECT_ID ? 0 : GetUniverse().Object<Planet>(m_planet_id);
}

XMLElement Building::XMLEncode(int empire_id/* = ALL_EMPIRES*/) const
{
    using boost::lexical_cast;

    XMLElement retval("Building" + lexical_cast<std::string>(ID()));
    retval.AppendChild(UniverseObject::XMLEncode(empire_id));
    retval.AppendChild(XMLElement("m_building_type", m_building_type));
    retval.AppendChild(XMLElement("m_operating", lexical_cast<std::string>(m_operating)));
    retval.AppendChild(XMLElement("m_planet_id", lexical_cast<std::string>(m_planet_id)));

    return retval;
}

UniverseObject* Building::Accept(const UniverseObjectVisitor& visitor) const
{
    return visitor.Visit(const_cast<Building* const>(this));
}

void Building::Activate(bool activate)
{
    m_operating = activate;
}

void Building::SetPlanetID(int planet_id)
{
    if (Planet* planet = GetPlanet())
        planet->RemoveBuilding(ID());
    m_planet_id = planet_id;
}

void Building::MovementPhase()
{
}

void Building::PopGrowthProductionResearchPhase()
{}


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
    return m_build_cost;
}

int BuildingType::BuildTime() const
{
    return m_build_time;
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

    Empire * empire = Empires().Lookup(empire_id);
    if (!empire) {
        Logger().debugStream() << "BuildingType::ProductionLocation: Unable to get pointer to empire " << empire_id;
        return false;
    }

    UniverseObject * source = universe.Object(empire->CapitolID());
    if (!source) return false;

    locations.insert(loc);

    m_location->Eval(source, locations, non_locations, Condition::TARGETS);
        
    return !(locations.empty());
}

void BuildingType::AddEffects(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects)
{
    std::copy(effects.begin(), effects.end(), m_effects.end());
}

const BuildingType* GetBuildingType(const std::string& name)
{
    static BuildingTypeManager manager;
    return manager.GetBuildingType(name);
}
