#ifndef _FREEORION_Empire_h_
#include "Empire.h"
#endif

#include <algorithm>
using std::find;

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

/** Constructors */ 

Empire::Empire(const std::string& name, int ID, const GG::Clr& color, ControlStatus& control) :
 // initialize members
 m_name(name), 
 m_id(ID), 
 m_color(color), 
 m_control_state(control)
{
    // nothing else to do
}


Empire::Empire(const GG::XMLElement& elem)
{
    using GG::XMLElement;
    
    m_id = lexical_cast<int> ( elem.Child("m_id").Attribute("value") );
    m_name = elem.Child("m_name").Text();
    m_total_rp = lexical_cast<int> ( elem.Child("m_total_rp").Attribute("value") );
    m_control_state = (ControlStatus) lexical_cast <int> ( elem.Child("m_control_state").Attribute("value") );
    m_color = GG::Clr( elem.Child("m_color") );
    
    // TODO:  decode the sitrep
    
    
    XMLElement container_elem = elem.Child("m_fleets");
    for(unsigned int i=0; i<container_elem.NumChildren(); i++)
    {
        m_fleets.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_planets");
    for(unsigned int i=0; i<container_elem.NumChildren(); i++)
    {
        m_planets.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_explored_systems");
    for(unsigned int i=0; i<container_elem.NumChildren(); i++)
    {
        m_explored_systems.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_visible_fleets");
    for(unsigned int i=0; i<container_elem.NumChildren(); i++)
    {
        m_visible_fleets.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_techs");
    for(unsigned int i=0; i<container_elem.NumChildren(); i++)
    {
        m_techs.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
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

/** 
* Get the empire's accumulated research points
* this number keeps accumulating even after research breakthroughs
* have been achieved
*/
int Empire::TotalRP() const
{
    return m_total_rp;
}

// Each of the following
// Returns true if the given item is in the appropriate list,
// false if it is not.
bool Empire::HasTech(int ID) const
{
    Empire::ConstTechIDItr item = find(TechBegin(), TechEnd(), ID);
    
    return (item != TechEnd());

}

bool Empire::HasPlanet(int ID) const
{
    Empire::ConstPlanetIDItr item = find(PlanetBegin(), PlanetEnd(), ID);
    return (item != PlanetEnd());
}

bool Empire::HasExploredSystem(int ID) const
{
   Empire::ConstSystemIDItr item = find(ExploredBegin(), ExploredEnd(), ID);
   return (item != ExploredEnd());
}

bool Empire::HasFleet(int ID) const
{
    Empire::ConstFleetIDItr item = find(FleetBegin(), FleetEnd(), ID);
    return (item != FleetEnd());
}

bool Empire::HasVisibleFleet(int ID) const
{
    Empire::ConstFleetIDItr item = find(VisibleFleetBegin(), VisibleFleetEnd(), ID);
    return (item != VisibleFleetEnd());
}



/* *************************************
    (const) Iterators over our various lists
***************************************/

Empire::ConstTechIDItr Empire::TechBegin() const
{
    return m_techs.begin();
}
Empire::ConstTechIDItr Empire::TechEnd() const
{
    return m_techs.end();
}


Empire::ConstPlanetIDItr Empire::PlanetBegin() const
{
    return m_planets.begin();
}
Empire::ConstPlanetIDItr Empire::PlanetEnd() const
{
    return m_planets.end();
}


Empire::ConstFleetIDItr Empire::FleetBegin() const
{
    return m_fleets.begin();
}
Empire::ConstFleetIDItr Empire::FleetEnd() const
{
    return m_fleets.end();
}


Empire::ConstSystemIDItr Empire::ExploredBegin() const
{
    return m_explored_systems.begin();
}
Empire::ConstSystemIDItr Empire::ExploredEnd() const
{
    return m_explored_systems.end();
}


Empire::ConstFleetIDItr Empire::VisibleFleetBegin() const
{
    return m_visible_fleets.begin();
}
Empire::ConstFleetIDItr Empire::VisibleFleetEnd() const
{
    return m_visible_fleets.end();
}

Empire::ConstSitRepItr Empire::SitRepBegin() const
{
    return m_sitrep_entries.begin();
}
Empire::ConstSitRepItr Empire::SitRepEnd() const
{
    return m_sitrep_entries.end();
}

/* *************************************
    (non-const) Iterators over our various lists
***************************************/
 
Empire::TechIDItr Empire::TechBegin() 
{
    return m_techs.begin();
}
Empire::TechIDItr Empire::TechEnd() 
{
    return m_techs.end();
}


Empire::PlanetIDItr Empire::PlanetBegin() 
{
    return m_planets.begin();
}
Empire::PlanetIDItr Empire::PlanetEnd() 
{
    return m_planets.end();
}


Empire::FleetIDItr Empire::FleetBegin() 
{
    return m_fleets.begin();
}
Empire::FleetIDItr Empire::FleetEnd() 
{
    return m_fleets.end();
}


Empire::SystemIDItr Empire::ExploredBegin() 
{
    return m_explored_systems.begin();
}
Empire::SystemIDItr Empire::ExploredEnd() 
{
    return m_explored_systems.end();
}


Empire::FleetIDItr Empire::VisibleFleetBegin() 
{
    return m_visible_fleets.begin();
}
Empire::FleetIDItr Empire::VisibleFleetEnd() 
{
    return m_visible_fleets.end();
}

Empire::SitRepItr Empire::SitRepBegin() 
{
    return m_sitrep_entries.begin();
}
Empire::SitRepItr Empire::SitRepEnd()
{
    return m_sitrep_entries.end();
}

 /* ************************************************
    Methods to add items to our various lists
**************************************************/

/// Inserts the given ID into the Empire's list of Technologies.
void Empire::AddTech(int ID)
{
    m_techs.push_back(ID);
}

/// Inserts the given ID into the Empire's list of owned planets.
void Empire::AddPlanet(int ID)
{
    m_planets.push_back(ID);
}

/// Inserts the given ID into the Empire's list of explored systems.
void Empire::AddExploredSystem(int ID)
{
    m_explored_systems.push_back(ID);
}

/// Inserts the given ID into the Empire's list of owned fleets.
void Empire::AddFleet(int ID)
{
    m_fleets.push_back(ID);
}

/// Inserts the given ID into the Empire's list of visible fleets.
void Empire::AddVisibleFleet(int ID)
{
    m_visible_fleets.push_back(ID);
}

/// Inserts the given sitrep entry into the empire's sitrep list
void Empire::AddSitRepEntry( SitRepEntry* entry)
{
    m_sitrep_entries.push_back(entry);
}

 /* ************************************************
    Methods to remove items from our various lists
**************************************************/

/// Removes the given ID from the empire's list
void Empire::RemoveTech(int ID)
{
    m_techs.remove(ID);
}

/// Removes the given ID from the empire's list
void Empire::RemoveOwnedPlanet(int ID)
{
    m_planets.remove(ID);
}
/// Removes the given ID from the empire's list
void Empire::RemoveExploredSystem(int ID)
{
    m_explored_systems.remove(ID);
}
/// Removes the given ID from the empire's list
void Empire::RemoveFleet(int ID)
{
    m_fleets.remove(ID);
}

void Empire::RemoveVisibleFleet(int ID)
{
    m_visible_fleets.remove(ID);
}

void Empire::ClearSitRep()
{
    m_sitrep_entries.clear();
}



 /* ************************************************
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
    
    XMLElement sitrep("m_sitrep_entries");
    for(ConstSitRepItr itr = SitRepBegin(); itr != SitRepEnd(); itr++)
    {
       // sitrep.AppendChild( (*itr)->XMLEncode() );
    }
    element.AppendChild(sitrep);
    
    XMLElement fleets("m_fleets");
    for(ConstFleetIDItr itr = FleetBegin(); itr != FleetEnd(); itr++)
    {
        XMLElement item("item");
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        fleets.AppendChild(item);
    }
    element.AppendChild(fleets);
    
    XMLElement planets("m_planets");
    for(ConstPlanetIDItr itr = PlanetBegin(); itr != PlanetEnd(); itr++)
    {
        XMLElement item("item");
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        planets.AppendChild(item);
    }
    element.AppendChild(planets);
    
    XMLElement explored("m_explored_systems");
    for(ConstSystemIDItr itr = ExploredBegin(); itr != ExploredEnd(); itr++)
    {
        XMLElement item("item");
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        explored.AppendChild(item);
    }
    element.AppendChild(explored);
    
    XMLElement techs("m_techs");
    for(ConstTechIDItr itr = TechBegin(); itr != TechEnd(); itr++)
    {
        XMLElement item("item");
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        techs.AppendChild(item);
    }
    element.AppendChild(techs);
    
    XMLElement visible_fleets("m_visible_fleets");
    for(ConstFleetIDItr itr = VisibleFleetBegin(); itr != VisibleFleetEnd(); itr++)
    {
        XMLElement item("item");
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        visible_fleets.AppendChild(item);
    }
    element.AppendChild(visible_fleets);
    
    return element;
}



void Empire::XMLMerge(const GG::XMLElement& elem)
{

    
    
    
}



 /* ************************************************
    Miscellaneous mutators
**************************************************/

/** 
* increases the empire's accumulated research points
* by the specified amount, and adds any new technologies achieved to the
* Empire's list of technologies.  Returns total accumulated research points
* after the addition. 
*/
int Empire::AddRP(int moreRPs)
{
    m_total_rp += moreRPs;

    // FINISH ME!!
    // check the TechManager for new techs
    
    return m_total_rp;
}

   	
/// Mutator for empire color
void Empire::Color(const GG::Clr& color)
{
    m_color = color;
}

/// Mutator for control state
void Empire::ControlState(ControlStatus state)
{
    m_control_state = state;
}

/// Mutator for empire name
void Empire::Name(const std::string& name)
{
    m_name = name;
}


