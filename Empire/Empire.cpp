#include "Empire.h"

#include "TechManager.h"

#include <algorithm>

#include <boost/lexical_cast.hpp>

using std::find;
using boost::lexical_cast;


/** Constructors */ 
Empire::Empire(const std::string& name, int ID, const GG::Clr& color, ControlStatus& control) :
    m_id(ID),
    m_total_rp(0),
    m_name(name),  
    m_color(color), 
    m_control_state(control),
    m_next_design_id(1)
{
}

Empire::Empire(const GG::XMLElement& elem)
{
    using GG::XMLElement;

    m_id = lexical_cast<int> ( elem.Child("m_id").Attribute("value") );
    m_name = elem.Child("m_name").Text();
    m_total_rp = lexical_cast<int> ( elem.Child("m_total_rp").Attribute("value") );
    m_control_state = (ControlStatus) lexical_cast <int> ( elem.Child("m_control_state").Attribute("value") );
    m_color = GG::Clr( elem.Child("m_color").Child(0) );
    m_next_design_id = lexical_cast<int> ( elem.Child("m_next_design_id").Attribute("value") );

    XMLElement sitrep = elem.Child("m_sitrep_entries");
    for(int i=0; i<sitrep.NumChildren(); i++)
    {
        AddSitRepEntry( new SitRepEntry( sitrep.Child(i) ) );
    }

    XMLElement container_elem = elem.Child("m_explored_systems");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_explored_systems.insert(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }

    container_elem = elem.Child("m_techs");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_techs.insert(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_ship_designs");
    for (int i = 0; i < container_elem.NumChildren(); ++i)
    {
       XMLElement design_elem = container_elem.Child(i);

       ShipDesign* ship_design = new ShipDesign(design_elem);
       int design_id = ship_design->id;

       m_ship_designs.insert(std::pair<int, ShipDesign>(design_id, *ship_design));
       delete ship_design;
    }
}

/** Misc Accessors */
Empire::ControlStatus Empire::ControlState() const
{
    return m_control_state;
}

const std::string& Empire::Name() const
{
    return m_name;
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
    
    XMLElement element("Empire");
    
    XMLElement ID("m_id");
    ID.SetAttribute( "value", lexical_cast<std::string>(m_id) );
    element.AppendChild(ID);
    
    XMLElement name("m_name");
    name.SetText(m_name);
    element.AppendChild(name);
    
    XMLElement total_rp("m_total_rp");
    total_rp.SetAttribute( "value", lexical_cast<std::string>(m_total_rp) );
    element.AppendChild(total_rp);
    
    XMLElement control("m_control_state");
    control.SetAttribute( "value",  lexical_cast<std::string>( (int) m_control_state) );
    element.AppendChild(control);
    
    XMLElement color("m_color");
    GG::XMLElement colorelem = m_color.XMLEncode();
    color.AppendChild(colorelem);
    element.AppendChild(color);

    XMLElement design_id("m_next_design_id");
    design_id.SetAttribute( "value", lexical_cast<std::string>(m_next_design_id) );
    element.AppendChild(design_id);
    
    XMLElement sitrep("m_sitrep_entries");
    for(SitRepItr itr = SitRepBegin(); itr != SitRepEnd(); itr++)
    {
       sitrep.AppendChild( (*itr)->XMLEncode() );
    }
    element.AppendChild(sitrep);
    
    XMLElement ship_designs("m_ship_designs");
    for(ShipDesignItr itr = ShipDesignBegin(); itr != ShipDesignEnd(); itr++)
    {
       ship_designs.AppendChild( (*itr).second.XMLEncode() );
    }
    element.AppendChild(ship_designs);
    
    XMLElement explored("m_explored_systems");
    EncodeIntList(explored, m_explored_systems);
    element.AppendChild(explored);
    
    XMLElement techs("m_techs");
    EncodeIntList(techs, m_techs);
    element.AppendChild(techs);
    
    return element;
}

GG::XMLElement Empire::XMLEncode(const Empire& viewer) const
{
    // same empire --->  call other version
    if(viewer.EmpireID() == this->EmpireID())
    {
        return this->XMLEncode();
    }
    
    using GG::XMLElement;
    using boost::lexical_cast;
    
    XMLElement element("Empire");
    
    XMLElement ID("m_id");
    ID.SetAttribute( "value", lexical_cast<std::string>(m_id) );
    element.AppendChild(ID);
    
    XMLElement name("m_name");
    name.SetText(m_name);
    element.AppendChild(name);
    
    // total_rp member needs to have a value so just use 0
    XMLElement total_rp("m_total_rp");
    total_rp.SetAttribute( "value", lexical_cast<std::string>(0) );
    element.AppendChild(total_rp);
    
    XMLElement control("m_control_state");
    control.SetAttribute( "value",  lexical_cast<std::string>( (int) m_control_state) );
    element.AppendChild(control);
    
    XMLElement color("m_color");
    GG::XMLElement colorelem = m_color.XMLEncode();
    color.AppendChild(colorelem);
    element.AppendChild(color);
    
    XMLElement design_id("m_next_design_id");
    design_id.SetAttribute( "value", lexical_cast<std::string>(m_next_design_id) );
    element.AppendChild(design_id);
    
    
    // for the lists, put the child elements in but do not populate them
    
    XMLElement sitrep("m_sitrep_entries");
    element.AppendChild(sitrep);
    
    XMLElement ship_designs("m_ship_designs");
    element.AppendChild(ship_designs);
    
    XMLElement explored("m_explored_systems");
    element.AppendChild(explored);
    
    XMLElement techs("m_techs");
    element.AppendChild(techs);
    
    return element;
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

void Empire::Color(const GG::Clr& color)
{
    m_color = color;
}

void Empire::ControlState(ControlStatus state)
{
    m_control_state = state;
}

void Empire::Name(const std::string& name)
{
    m_name = name;
}

void Empire::EncodeIntList(GG::XMLElement& container, const std::set<int>& lst)
{
    
    int i=0;
    for(std::set<int>::const_iterator itr = lst.begin(); itr != lst.end(); itr++)
    {
        GG::XMLElement item("index" + lexical_cast<std::string>(i) );
        i++;
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        container.AppendChild(item);
    }
}
