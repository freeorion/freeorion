#include "Empire.h"

#include "ResourcePool.h"

#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include <algorithm>

#include <boost/lexical_cast.hpp>

using std::find;
using boost::lexical_cast;


/** Constructors */ 
Empire::Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, int homeworld_id) :
    m_id(ID),
    m_total_rp(0),
    m_name(name),
    m_player_name(player_name),
    m_color(color), 
    m_homeworld_id(homeworld_id), 
    m_mineral_resource_pool(),m_food_resource_pool(),m_research_resource_pool(),m_population_resource_pool(),m_trade_resource_pool()
{}

Empire::Empire(const GG::XMLElement& elem) : 
    m_mineral_resource_pool(elem.Child("m_mineral_resource_pool").Child("MineralResourcePool")),
    m_food_resource_pool(elem.Child("m_food_resource_pool").Child("FoodResourcePool")),
    m_research_resource_pool(elem.Child("m_research_resource_pool").Child("ResearchResourcePool")),
    m_population_resource_pool(elem.Child("m_population_resource_pool").Child("PopulationResourcePool")),
    m_trade_resource_pool(elem.Child("m_trade_resource_pool").Child("TradeResourcePool"))

{
    if (elem.Tag() != "Empire")
        throw std::invalid_argument("Attempted to construct a Empire from an XMLElement that had a tag other than \"Empire\"");

    using GG::XMLElement;

    m_id = lexical_cast<int>(elem.Child("m_id").Text());
    m_name = elem.Child("m_name").Text();
    m_player_name = elem.Child("m_player_name").Text();
    m_total_rp = lexical_cast<int>(elem.Child("m_total_rp").Text());
    m_color = GG::Clr(elem.Child("m_color").Child("GG::Clr"));
    m_homeworld_id = elem.Child("m_homeworld_id").Text().empty() ? 
        UniverseObject::INVALID_OBJECT_ID : lexical_cast<int>(elem.Child("m_homeworld_id").Text());

    const XMLElement& sitreps_elem = elem.Child("m_sitrep_entries");
    for (int i = 0; i < sitreps_elem.NumChildren(); ++i) {
        AddSitRepEntry(new SitRepEntry(sitreps_elem.Child(i)));
    }

    const XMLElement& designs_elem = elem.Child("m_ship_designs");
    for (int i = 0; i < designs_elem.NumChildren(); ++i) {
        ShipDesign ship_design(designs_elem.Child(i).Child(0));
        m_ship_designs[ship_design.name] = ship_design;
    }

    m_explored_systems = GG::ContainerFromString<std::set<int> >(elem.Child("m_explored_systems").Text());
    const XMLElement& techs_elem = elem.Child("m_techs");
    for (int i = 0; i < techs_elem.NumChildren(); ++i) {
        m_techs.insert(techs_elem.Child(i).Text());
    }
    const XMLElement& building_types_elem = elem.Child("m_building_types");
    for (int i = 0; i < building_types_elem.NumChildren(); ++i) {
        m_building_types.insert(building_types_elem.Child(i).Text());
    }
}

Empire::~Empire()
{}

/** Misc Accessors */
const std::string& Empire::Name() const
{
    return m_name;
}

const std::string& Empire::PlayerName() const
{
    return m_player_name;
}

int Empire::EmpireID() const
{
    return m_id;
}

const GG::Clr& Empire::Color() const
{
    return m_color;
}

int Empire::HomeworldID() const
{
    return m_homeworld_id;
}

int Empire::TotalRP() const
{
    return m_total_rp;
}

const ShipDesign* Empire::GetShipDesign(const std::string& name) const
{
    Empire::ShipDesignItr it = m_ship_designs.find(name);
    return (it == m_ship_designs.end()) ? 0 : &it->second;
}

bool Empire::TechAvailable(const std::string& name) const
{
    Empire::TechItr item = m_techs.find(name);
    return item != m_techs.end();
}

bool Empire::BuildingTypeAvailable(const std::string& name) const
{
    Empire::BuildingTypeItr item = m_building_types.find(name);
    return item != m_building_types.end();
}

bool Empire::HasExploredSystem(int ID) const
{
    Empire::SystemIDItr item = find(ExploredBegin(), ExploredEnd(), ID);
    return (item != ExploredEnd());
}

int Empire::NumSitRepEntries() const
{
    return m_sitrep_entries.size();
}


/* *************************************
    (const) Iterators over our various lists
***************************************/
Empire::TechItr Empire::TechBegin() const
{
    return m_techs.begin();
}
Empire::TechItr Empire::TechEnd() const
{
    return m_techs.end();
}

Empire::TechItr Empire::BuildingTypeBegin() const
{
    return m_building_types.begin();
}
Empire::TechItr Empire::BuildingTypeEnd() const
{
    return m_building_types.end();
}

Empire::SystemIDItr Empire::ExploredBegin()  const
{
    return m_explored_systems.begin();
}
Empire::SystemIDItr Empire::ExploredEnd() const
{
    return m_explored_systems.end();
}

Empire::ShipDesignItr Empire::ShipDesignBegin() const
{
    return m_ship_designs.begin();
}
Empire::ShipDesignItr Empire::ShipDesignEnd() const
{
    return m_ship_designs.end();
}

Empire::SitRepItr Empire::SitRepBegin() const
{
    return m_sitrep_entries.begin();
}
Empire::SitRepItr Empire::SitRepEnd() const
{
    return m_sitrep_entries.end();
}


/*************************************************
    Methods to add items to our various lists
**************************************************/
void Empire::AddTech(const std::string& name)
{
    m_techs.insert(name);
}

void Empire::AddBuildingType(const std::string& name)
{
    m_building_types.insert(name);
}

void Empire::AddExploredSystem(int ID)
{
    m_explored_systems.insert(ID);
}

void Empire::AddShipDesign(const ShipDesign& design)
{
   m_ship_designs[design.name] = design;
}

void Empire::AddSitRepEntry(SitRepEntry* entry)
{
    m_sitrep_entries.push_back(entry);
}


/*************************************************
    Methods to remove items from our various lists
**************************************************/
void Empire::RemoveTech(const std::string& name)
{
    m_techs.erase(name);
}

void Empire::RemoveBuildingType(const std::string& name)
{
    m_building_types.erase(name);
}

void Empire::ClearSitRep()
{
    for (SitRepItr it = m_sitrep_entries.begin(); it != m_sitrep_entries.end(); ++it)
        delete *it;
    m_sitrep_entries.clear();
}


/*************************************************
    Methods to support XML Serialization
**************************************************/
GG::XMLElement Empire::XMLEncode() const
{
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_total_rp", lexical_cast<std::string>(m_total_rp)));
    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_homeworld_id", lexical_cast<std::string>(m_homeworld_id)));

    retval.AppendChild(XMLElement("m_sitrep_entries"));
    for (SitRepItr it = SitRepBegin(); it != SitRepEnd(); ++it) {
       retval.LastChild().AppendChild((*it)->XMLEncode());
    }

    retval.AppendChild(XMLElement("m_ship_designs"));
    int i = 0;
    for (ShipDesignItr it = ShipDesignBegin(); it != ShipDesignEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("design" + lexical_cast<std::string>(i++), it->second.XMLEncode()));
    }

    retval.AppendChild(XMLElement("m_explored_systems", GG::StringFromContainer<std::set<int> >(m_explored_systems)));
    retval.AppendChild(XMLElement("m_techs"));
    i = 0;
    for (TechItr it = TechBegin(); it != TechEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("tech" + lexical_cast<std::string>(i++), *it));
    }

    retval.AppendChild(XMLElement("m_building_types"));
    i = 0;
    for (BuildingTypeItr it = BuildingTypeBegin(); it != BuildingTypeEnd(); ++it) {
        retval.LastChild().AppendChild(XMLElement("building_type" + lexical_cast<std::string>(i++), *it));
    }

    retval.AppendChild(XMLElement("m_mineral_resource_pool", m_mineral_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_food_resource_pool", m_food_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_research_resource_pool", m_research_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_population_resource_pool", m_population_resource_pool.XMLEncode()));
    retval.AppendChild(XMLElement("m_trade_resource_pool", m_trade_resource_pool.XMLEncode()));

    return retval;
}

GG::XMLElement Empire::XMLEncode(const Empire& viewer) const
{
    // same empire --->  call other version
    if (viewer.EmpireID() == this->EmpireID())
    {
        return XMLEncode();
    }
    
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_total_rp", lexical_cast<std::string>(m_total_rp)));
    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));

    // leave these in, but unpopulated
    retval.AppendChild(XMLElement("m_homeworld_id"));
    retval.AppendChild(XMLElement("m_sitrep_entries"));
    retval.AppendChild(XMLElement("m_ship_designs"));
    retval.AppendChild(XMLElement("m_explored_systems"));
    retval.AppendChild(XMLElement("m_techs"));
    retval.AppendChild(XMLElement("m_building_types"));
    retval.AppendChild(XMLElement("m_mineral_resource_pool", MineralResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_food_resource_pool", FoodResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_research_resource_pool", ResearchResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_population_resource_pool", PopulationResourcePool().XMLEncode()));
    retval.AppendChild(XMLElement("m_trade_resource_pool", TradeResourcePool().XMLEncode()));

    return retval;
}


/*************************************************
    Miscellaneous mutators
**************************************************/
int Empire::AddRP(int moreRPs)
{
    m_total_rp += moreRPs;

    return m_total_rp;
}


void Empire::CheckResearchProgress()
{
}

void Empire::SetColor(const GG::Clr& color)
{
    m_color = color;
}

void Empire::SetName(const std::string& name)
{
    m_name = name;
}

void Empire::SetPlayerName(const std::string& player_name)
{
    m_player_name = player_name;
}

void Empire::UpdateResourcePool()
{
  m_mineral_resource_pool.SetPlanets(GetUniverse().FindObjects(IsOwnedObjectFunctor<Planet>(m_id)));
  m_food_resource_pool.SetPlanets(GetUniverse().FindObjects(IsOwnedObjectFunctor<Planet>(m_id)));
  m_research_resource_pool.SetPlanets(GetUniverse().FindObjects(IsOwnedObjectFunctor<Planet>(m_id)));
  m_population_resource_pool.SetPlanets(GetUniverse().FindObjects(IsOwnedObjectFunctor<Planet>(m_id)));
  m_industry_resource_pool.SetPlanets(GetUniverse().FindObjects(IsOwnedObjectFunctor<Planet>(m_id)));
  m_trade_resource_pool.SetPlanets(GetUniverse().FindObjects(IsOwnedObjectFunctor<Planet>(m_id)));
}
