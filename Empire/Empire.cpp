#ifndef _FREEORION_Empire_h_
#include "Empire.h"
#endif

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


Empire::Empire(const GG::XMLElement& elemenet)
{
    // write me!
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
    return true; // FINISH ME!
}

bool Empire::HasPlanet(int ID) const
{
    return true; // FINISH ME!
}

bool Empire::HasExploredSystem(int ID) const
{
    return true; // FINISH ME!
}

bool Empire::HasFleet(int ID) const
{
    return true; // FINISH ME!
}

bool Empire::HasVisibleFleet(int ID) const
{
    return true; // FINISH ME!
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
    GG::XMLElement element;
    
    // WRITE ME!
    
    return element;
}

void Empire::XMLMerge(const GG::XMLElement& elem)
{
    // WRITE ME!
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


