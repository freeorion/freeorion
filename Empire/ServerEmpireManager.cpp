#ifndef _FREEORION_ServerEmpireManager_h_ 
#include "ServerEmpireManager.h"
#endif

#include <stdexcept>

ServerEmpireManager::ServerEmpireManager() :
    m_next_id(1)
{
    // nothing to do yet
}

/** 
   Creates an empire with the specified properties and
   returns a pointer to it, after setting it up.
   homeID is the ID of the planet which is the empire's homeworld
   the empire will be created, and the given planetID added to its
   list of owned planets.
   this will only set up the data in Empire.  It is the caller's 
   responsibility to make sure that universe updates planet ownership.
   I do this because GameCore may want to call this under a variety of
   circumstances, and I do not want it to be too specific.
*/
Empire* ServerEmpireManager::CreateEmpire(const std::string& name, 
                                   const GG::Clr& color, 
                                   int planetID, 
                                   Empire::ControlStatus state)
{
    Empire* emp = new Empire(name, m_next_id, color, state);
    m_next_id++;
    emp->AddPlanet(planetID);
    
    InsertEmpire(emp);
    
    return emp;
}


/**
  removes all traces of the empire with the given ID.
  and deallocates that empire.  Pointers, references, and iterators
  to that empire will be invalidated.
  
  Nothing happens if an empire with the specified ID does not exist.
  
  Again, this method does not do anything to the universe,
  that is GameCore's responsibility.
  
  This method returns true if the empire was removed, false if it
  doesn't exist.
*/
bool ServerEmpireManager::EliminateEmpire(int ID)
{
    Empire* emp = Lookup(ID);
    RemoveEmpire(emp);
    return (emp != NULL);
}
    


/**
* Creates an XMLElement representing an XML diff between the
* present state of the empires and the state at the beginning of 
* the current turn.
*
* The returned element can be passed to the 
* ClientEmpireManager::HandleEmpireUpdate() method to bring the ClientEmpireManager
* in sync with the server manager.  
*
* When this method is called, the stored XMLElements for the empires at
* the beginning of the turn are replaced by fresh elements representing
* their current states.
*/
GG::XMLElement ServerEmpireManager::CreateClientEmpireUpdate(int EmpireID)
{
    GG::XMLElement update("EmpireUpdate");
    
    
    
    return update;
}

/**
* Creates an XMLElement representing the list of sitrep events
* for the empire with the given ID.  The returned element can be 
* sent to the client and decoded to put the proper sitrep events in
* the client's queue
*/
GG::XMLElement ServerEmpireManager::CreateClientSitrepUpdate(int EmpireID)
{
    GG::XMLElement update("SitrepUpdate");
    
    // find whatever empire they're talking about
    Empire* emp = Lookup(EmpireID);
    
    // perform sanity check
    if(emp == NULL)
    {
        throw std::runtime_error("Invalid EmpireID passed to ServerEmpireManager::CreateClientSitrepUpdate()");
    }
    else
    {
        // convert Empire ID to string
        char sIDString[12];  // integer cannot possibly exceed 10 digits
        sprintf(sIDString, "%d", EmpireID);
        
        // set ID attribute of the sitrep update to indicate whose empire this is
        update.SetAttribute("EmpireID", sIDString);
        
        // add the empire's entire sitrep to the update element
       /* for( Empire::SitRepItr itr = emp->SitRepBegin(); itr != emp->SitRepEnd(); itr++)
        {
            update.AppendChild((*itr).XMLEncode());
        }*/
    }
    
    return update;

}
