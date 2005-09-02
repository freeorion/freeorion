#include "Building.h"

#include "Effect.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "Planet.h"
#include "Predicates.h"

#include <fstream>

namespace {
    // loads and stores BuildingTypes specified in [settings-dir]/buildings.xml
    class BuildingTypeManager
    {
    public:
        BuildingTypeManager()
        {
            std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
            if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
                settings_dir += '/';
            std::ifstream ifs((settings_dir + "buildings.xml").c_str());
            GG::XMLDoc doc;
            doc.ReadDoc(ifs);
            for (GG::XMLElement::const_child_iterator it = doc.root_node.child_begin(); it != doc.root_node.child_end(); ++it) {
                if (it->Tag() != "BuildingType")
                    throw std::runtime_error("ERROR: Encountered non-BuildingType in buildings.xml!");
                BuildingType* building_type = 0;
                try {
                    building_type = new BuildingType(*it);
                } catch (const std::runtime_error& e) {
                    std::stringstream stream;
                    it->WriteElement(stream);
                    throw std::runtime_error(std::string("ERROR: \"") + e.what() + "\" encountered when loading this Building XML code:\n" + stream.str());
                }
                if (m_building_types.find(building_type->Name()) != m_building_types.end())
                    throw std::runtime_error(("ERROR: More than one building type in buildings.xml has the name " + building_type->Name()).c_str());
                m_building_types[building_type->Name()] = building_type;
            }
            ifs.close();
        }

        const BuildingType* GetBuildingType(const std::string& name) const
        {
            std::map<std::string, BuildingType*>::const_iterator it = m_building_types.find(name);
            return it != m_building_types.end() ? it->second : 0;
        }

    private:
        std::map<std::string, BuildingType*> m_building_types;
    };

    bool temp_header_bool = RecordHeaderFile(BuildingRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

Building::Building() :
    m_building_type(""),
    m_operating(true),
    m_planet_id(INVALID_OBJECT_ID)
{
}

Building::Building(int empire_id, const std::string& building_type, int planet_id) :
    m_building_type(building_type),
    m_operating(true),
    m_planet_id(planet_id)
{
   AddOwner(empire_id);
}

Building::Building(const GG::XMLElement& elem) :
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
    if (Owners().size() != 1)
        return ::GetBuildingType(m_building_type);

    Empire* empire = Empires().Lookup(*Owners().begin());
    return empire->GetBuildingType(m_building_type);
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

GG::XMLElement Building::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
    using GG::XMLElement;
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
{
}


BuildingType::BuildingType() :
    m_name(""),
    m_description(""),
    m_build_cost(0.0),
    m_build_time(0),
    m_maintenance_cost(0.0),
    m_effects(0),
    m_graphic("")
{
}

BuildingType::BuildingType(const std::string& name, const std::string& description) :
    m_name(name),
    m_description(description),
    m_build_cost(0.0),
    m_build_time(0),
    m_maintenance_cost(0.0),
    m_effects(0),
    m_graphic("")
{
}

BuildingType::BuildingType(const GG::XMLElement& elem)
{
    if (elem.Tag() != "BuildingType")
        throw std::invalid_argument("Attempted to construct a BuildingType from an XMLElement that had a tag other than \"BuildingType\"");

    using boost::lexical_cast;

    m_name = elem.Child("name").Text();
    m_description = elem.Child("description").Text();
    m_build_cost = lexical_cast<double>(elem.Child("build_cost").Text());
    m_build_time = lexical_cast<int>(elem.Child("build_time").Text());
    m_maintenance_cost = lexical_cast<double>(elem.Child("maintenance_cost").Text());
    for (GG::XMLElement::const_child_iterator it = elem.Child("effects").child_begin(); it != elem.Child("effects").child_end(); ++it) {
        m_effects.push_back(boost::shared_ptr<const Effect::EffectsGroup>(new Effect::EffectsGroup(*it)));
    }
    m_graphic = elem.Child("graphic").Text();
}

const std::string& BuildingType::Name() const
{
    return m_name;
}

const std::string& BuildingType::Description() const
{
    return m_description;
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

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& BuildingType::Effects() const
{
    return m_effects;
}

const std::string& BuildingType::Graphic() const
{
    return m_graphic;
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
