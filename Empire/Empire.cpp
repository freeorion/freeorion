#include "Empire.h"

#include "TechManager.h"

#include "ResourcePool.h"

#include "../universe/Predicates.h"
#include "../universe/Planet.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include <algorithm>

#include <boost/lexical_cast.hpp>

using std::find;
using boost::lexical_cast;


/** Constructors */ 
Empire::Empire(const std::string& name, const std::string& player_name, int ID, const GG::Clr& color, ControlStatus& control) :
    m_id(ID),
    m_total_rp(0),
    m_name(name),
    m_player_name(player_name),
    m_color(color), 
    m_control_state(control),
    m_next_design_id(1),
    m_mineral_resource_pool(),m_food_resource_pool(),m_research_resource_pool(),m_population_resource_pool()
{}

Empire::Empire(const GG::XMLElement& elem) : 
    m_mineral_resource_pool(),m_food_resource_pool()
{
    using GG::XMLElement;

    m_id = lexical_cast<int>(elem.Child("m_id").Text());
    m_name = elem.Child("m_name").Text();
    m_player_name = elem.Child("m_player_name").Text();
    m_total_rp = lexical_cast<int>(elem.Child("m_total_rp").Text());
    m_control_state = ControlStatus(lexical_cast<int>(elem.Child("m_control_state").Text()));
    m_color = GG::Clr(elem.Child("m_color").Child("GG::Clr"));
    m_next_design_id = lexical_cast<int>(elem.Child("m_next_design_id").Text());

    const XMLElement& sitreps_elem = elem.Child("m_sitrep_entries");
    for (int i = 0; i < sitreps_elem.NumChildren(); ++i) {
        AddSitRepEntry(new SitRepEntry(sitreps_elem.Child(i)));
    }

    m_explored_systems = GG::ContainerFromString<std::set<int> >(elem.Child("m_explored_systems").Text());
    m_techs = GG::ContainerFromString<std::set<int> >(elem.Child("m_techs").Text());

    const XMLElement& designs_elem = elem.Child("m_ship_designs");
    for (int i = 0; i < designs_elem.NumChildren(); ++i)
    {
       ShipDesign ship_design(designs_elem.Child(i));
       m_ship_designs.insert(std::make_pair(ship_design.id, ship_design));
    }
}

Empire::~Empire()
{}

/** Misc Accessors */
Empire::ControlStatus Empire::ControlState() const
{
    return m_control_state;
}

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

int Empire::TotalRP() const
{
    return m_total_rp;
}

bool Empire::CopyShipDesign(int design_id, ShipDesign& design_target)
{
   Empire::ShipDesignItr itr = m_ship_designs.find(design_id);
   
   if (itr != ShipDesignEnd())
   {
      design_target = (*itr).second;
      return true;
   }

   return false;
}

bool Empire::HasTech(int ID) const
{
    Empire::TechIDItr item = m_techs.find(ID);
    
    return (item != TechEnd());

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
Empire::TechIDItr Empire::TechBegin() const
{
    return m_techs.begin();
}
Empire::TechIDItr Empire::TechEnd() const
{
    return m_techs.end();
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
void Empire::AddTech(int ID)
{
    m_techs.insert(ID);
}

void Empire::AddExploredSystem(int ID)
{
    m_explored_systems.insert(ID);
}

int Empire::AddShipDesign(const ShipDesign& design)
{
   ShipDesign new_design = design;
   new_design.id = m_next_design_id;
   m_ship_designs.insert(std::pair<int, ShipDesign>(m_next_design_id, new_design));
    
   return m_next_design_id++;
}

void Empire::AddSitRepEntry(SitRepEntry* entry)
{
    m_sitrep_entries.push_back(entry);
}


/*************************************************
    Methods to remove items from our various lists
**************************************************/
void Empire::RemoveTech(int ID)
{
    m_techs.erase(ID);
}

void Empire::RemoveShipDesign(int id)
{
    Empire::ShipDesignItr it = m_ship_designs.find(id);
    if (it != m_ship_designs.end()) {
        m_ship_designs.erase(id);
    }
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
    retval.AppendChild(XMLElement("m_control_state", lexical_cast<std::string>(m_control_state)));
    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_next_design_id", lexical_cast<std::string>(m_next_design_id)));

    retval.AppendChild(XMLElement("m_sitrep_entries"));
    for (SitRepItr it = SitRepBegin(); it != SitRepEnd(); ++it)
    {
       retval.LastChild().AppendChild((*it)->XMLEncode());
    }

    retval.AppendChild(XMLElement("m_ship_designs"));
    for (ShipDesignItr it = ShipDesignBegin(); it != ShipDesignEnd(); ++it)
    {
        retval.LastChild().AppendChild(it->second.XMLEncode());
    }

    retval.AppendChild(XMLElement("m_explored_systems", GG::StringFromContainer<std::set<int> >(m_explored_systems)));
    retval.AppendChild(XMLElement("m_techs", GG::StringFromContainer<std::set<int> >(m_techs)));
    return retval;
}

GG::XMLElement Empire::XMLEncode(const Empire& viewer) const
{
    // same empire --->  call other version
    if (viewer.EmpireID() == this->EmpireID())
    {
        return this->XMLEncode();
    }
    
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement retval("Empire");
    retval.AppendChild(XMLElement("m_id", lexical_cast<std::string>(m_id)));
    retval.AppendChild(XMLElement("m_name", m_name));
    retval.AppendChild(XMLElement("m_player_name", m_player_name));
    retval.AppendChild(XMLElement("m_total_rp", lexical_cast<std::string>(m_total_rp)));
    retval.AppendChild(XMLElement("m_control_state", lexical_cast<std::string>(m_control_state)));
    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_next_design_id", lexical_cast<std::string>(m_next_design_id)));

    // leave these in, but unpopulated
    retval.AppendChild(XMLElement("m_sitrep_entries"));
    retval.AppendChild(XMLElement("m_ship_designs"));
    retval.AppendChild(XMLElement("m_explored_systems"));
    retval.AppendChild(XMLElement("m_techs"));
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


void Empire::CheckResearchProgress( )
{
    // check the TechManager for new techs
    
    TechManager::iterator itr = TechManager::instance().begin();
    while(itr != TechManager::instance().end())
    {
        if ( (*itr).second->GetMinPts() <= m_total_rp )
        {
            if( !HasTech( (*itr).second->GetID() ) )
            {
                AddTech( (*itr).first );

        		// add sit rep
                SitRepEntry *p_entry = CreateTechResearchedSitRep( (*itr).first );
                AddSitRepEntry( p_entry );
            }
        }
        
        itr++;
    }
}

void Empire::SetColor(const GG::Clr& color)
{
    m_color = color;
}

void Empire::SetControlState(ControlStatus state)
{
    m_control_state = state;
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
}
