#ifndef _Empire_h_
#include "Empire.h"
#endif

#ifndef _TechManager_h_
#include "TechManager.h"
#endif

#ifndef __XDIFF__
#include "../network/XDiff.hpp"
#endif


#include <algorithm>
using std::find;

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

/** Constructors */ 

Empire::Empire(const std::string& name, int ID, const GG::Clr& color, ControlStatus& control) :
 // initialize members
 m_id(ID),
 m_total_rp(0),
 m_name(name),  
 m_color(color), 
 m_control_state(control),
 m_next_design_id(1)

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
    m_color = GG::Clr( elem.Child("m_color").Child(0) );
    m_next_design_id = lexical_cast<int> ( elem.Child("m_next_design_id").Attribute("value") );
    
 
    GG::XMLObjectFactory<SitRepEntry> sitrep_factory;
    //SitRepEntry::InitObjectFactory(sitrep_factory);
    XMLElement sitrep = elem.Child("m_sitrep_entries");
    for(int i=0; i<sitrep.NumChildren(); i++)
    {
        AddSitRepEntry( sitrep_factory.GenerateObject(sitrep.Child(i)) );
    }
     
    XMLElement container_elem = elem.Child("m_fleets");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_fleets.insert(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_planets");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_planets.insert(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_explored_systems");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_explored_systems.insert(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_visible_fleets");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_visible_fleets.insert(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_techs");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_techs.insert(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
    
    container_elem = elem.Child("m_ship_designs");
    for (int i=0; i<container_elem.NumChildren(); i++)
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

/** 
* Get the empire's accumulated research points
* this number keeps accumulating even after research breakthroughs
* have been achieved
*/
int Empire::TotalRP() const
{
    return m_total_rp;
}


/// Searches for a ship design and copies over the input design and returns success/failure
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

// Each of the following
// Returns true if the given item is in the appropriate list,
// false if it is not.
bool Empire::HasTech(int ID) const
{
    Empire::ConstTechIDItr item = m_techs.find(ID);
    
    return (item != TechEnd());

}

bool Empire::HasPlanet(int ID) const
{
    Empire::ConstPlanetIDItr item = m_planets.find(ID);
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

Empire::ConstShipDesignItr Empire::ShipDesignBegin() const
{
    return m_ship_designs.begin();
}
Empire::ConstShipDesignItr Empire::ShipDesignEnd() const
{
    return m_ship_designs.end();
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

Empire::ShipDesignItr Empire::ShipDesignBegin() 
{
    return m_ship_designs.begin();
}
Empire::ShipDesignItr Empire::ShipDesignEnd() 
{
    return m_ship_designs.end();
}

 /* ************************************************
    Methods to add items to our various lists
**************************************************/

/// Inserts the given ID into the Empire's list of Technologies.
void Empire::AddTech(int ID)
{
    m_techs.insert(ID);
}

/// Inserts the given ID into the Empire's list of owned planets.
void Empire::AddPlanet(int ID)
{
    m_planets.insert(ID);
}

/// Inserts the given ID into the Empire's list of explored systems.
void Empire::AddExploredSystem(int ID)
{
    m_explored_systems.insert(ID);
}

/// Inserts the given ID into the Empire's list of owned fleets.
void Empire::AddFleet(int ID)
{
    m_fleets.insert(ID);
}

/// Inserts the given ID into the Empire's list of visible fleets.
void Empire::AddVisibleFleet(int ID)
{
    m_visible_fleets.insert(ID);
}

/// Inserts the given sitrep entry into the empire's sitrep list
void Empire::AddSitRepEntry( SitRepEntry* entry)
{
    m_sitrep_entries.push_back(entry);
}

/// Inserts the design into the empire's ship design list, assigns it an ID and returns the ID.
int Empire::AddShipDesign(const ShipDesign& design)
{
   ShipDesign new_design = design;
   new_design.id = m_next_design_id;
   m_ship_designs.insert(std::pair<int, ShipDesign>(m_next_design_id, new_design));
    
   return m_next_design_id++;
}

 /* ************************************************
    Methods to remove items from our various lists
**************************************************/

/// Removes the given ID from the empire's list
void Empire::RemoveTech(int ID)
{
    m_techs.erase(ID);
}

/// Removes the given ID from the empire's list
void Empire::RemoveOwnedPlanet(int ID)
{
    m_planets.erase(ID);
}
/// Removes the given ID from the empire's list
void Empire::RemoveExploredSystem(int ID)
{
    m_explored_systems.erase(ID);
}
/// Removes the given ID from the empire's list
void Empire::RemoveFleet(int ID)
{
    m_fleets.erase(ID);
}

void Empire::RemoveVisibleFleet(int ID)
{
    m_visible_fleets.erase(ID);
}

void Empire::ClearSitRep()
{
    SitRepItr itr = SitRepBegin();
    
    // deallocate all sitrep entries in the collection
    while(itr != SitRepEnd())
    {
        delete *itr;
        itr++;
    }
    
    m_sitrep_entries.clear();
}


void Empire::RemoveShipDesign(int id)
{
    Empire::ShipDesignItr itr = m_ship_designs.find(id);
    
    if (itr != ShipDesignEnd())
    {
      m_ship_designs.erase( itr );
    }
    
    return;
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

    XMLElement design_id("m_next_design_id");
    design_id.SetAttribute( "value", lexical_cast<std::string>(m_next_design_id) );
    element.AppendChild(design_id);
    
    // There is no need to serialize the sitrep entries since they are
    // handled by the empire manager sitrep update functionality
    
    XMLElement sitrep("m_sitrep_entries");
    for(ConstSitRepItr itr = SitRepBegin(); itr != SitRepEnd(); itr++)
    {
       sitrep.AppendChild( (*itr)->XMLEncode() );
    }
    element.AppendChild(sitrep);
    
    XMLElement ship_designs("m_ship_designs");
    for(ConstShipDesignItr itr = ShipDesignBegin(); itr != ShipDesignEnd(); itr++)
    {
       ship_designs.AppendChild( (*itr).second.XMLEncode() );
    }
    element.AppendChild(ship_designs);
    
    XMLElement fleets("m_fleets");
    EncodeIntList(fleets, m_fleets);
    element.AppendChild(fleets);
    
    XMLElement planets("m_planets");
    EncodeIntList(planets, m_planets);
    element.AppendChild(planets);
    
    XMLElement explored("m_explored_systems");
    EncodeIntList(explored, m_explored_systems);
    element.AppendChild(explored);
    
    XMLElement techs("m_techs");
    EncodeIntList(techs, m_techs);
    element.AppendChild(techs);
    
    XMLElement visible_fleets("m_visible_fleets");
    EncodeIntList(visible_fleets, m_visible_fleets);
    element.AppendChild(visible_fleets);
    
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
    
    // total_rp member needs to have a value so it can get initialized when we de-serialize.
    // set it to 0 because the other empire isnt' supposed to know this
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
    
    XMLElement fleets("m_fleets");
    element.AppendChild(fleets);
    
    XMLElement planets("m_planets");
    element.AppendChild(planets);
    
    XMLElement explored("m_explored_systems");
    element.AppendChild(explored);
    
    XMLElement techs("m_techs");
    element.AppendChild(techs);
    
    XMLElement visible_fleets("m_visible_fleets");
    element.AppendChild(visible_fleets);
    
    return element;

}


void Empire::XMLMerge(const GG::XMLElement& elem)
{
    GG::XMLDoc diff_doc;
    diff_doc.root_node = elem;
    
    GG::XMLElement obj_elem( XMLEncode() );
    GG::XMLDoc obj_doc;
    obj_doc.root_node = obj_elem;
    
    XPatch(obj_doc, diff_doc);
    
    *this = Empire( obj_doc.root_node );
}



 /* ************************************************
    Miscellaneous mutators
**************************************************/

/** 
* increases the empire's accumulated research points
* by the specified amount, Returns total accumulated research points
* after the addition. 
*/
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


// private helper method for encoding a list of integers
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
