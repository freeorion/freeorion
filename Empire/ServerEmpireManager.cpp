#ifndef _ServerEmpireManager_h_ 
#include "ServerEmpireManager.h"
#endif

#ifndef _ServerApp_h_
#include "../server/ServerApp.h"
#endif

#ifndef _ServerUniverse_h_
#include "../universe/ServerUniverse.h"
#endif

#ifndef _Planet_h_
#include "../universe/Planet.h"
#endif

#include <stdexcept>

#ifndef __XDIFF__
#include "../network/XDiff.hpp"
#endif

ServerEmpireManager::ServerEmpireManager() :
    EmpireManager(),
    m_next_id(1)
{
    // nothing to do yet
}


Empire* ServerEmpireManager::CreateEmpire(const std::string& name, 
                                   const GG::Clr& color, 
                                   int planetID, 
                                   Empire::ControlStatus state)
{
    Empire* emp = new Empire(name, m_next_id, color, state);
    m_next_id++;
    emp->AddPlanet(planetID);

    // Add system homeplanet is in to the ExploredSystem list
    ServerApp* server_app = ServerApp::GetApp();
    ServerUniverse* universe = &(server_app->Universe());
    UniverseObject*   uni_obj = universe->Object(planetID);
    Planet*   planet = dynamic_cast<Planet*>(uni_obj);
    emp->AddExploredSystem(planet->SystemID());
    
    // add a dummy empire update to the map, so that when 
    // the first empire update is produced, we have something to diff with
    GG::XMLElement state_init (EmpireManager::EMPIRE_UPDATE_TAG, std::string(""));
    m_last_turn_empire_states.insert(std::pair<int, GG::XMLElement>(emp->EmpireID(), state_init));
    
    InsertEmpire(emp);
    
    return emp;
}



bool ServerEmpireManager::EliminateEmpire(int ID)
{
    Empire* emp = Lookup(ID);
    RemoveEmpire(emp);
    
    // dont need his last empire update anymore
    m_last_turn_empire_states.erase(ID);
    
    // prevent memory leak
    delete emp;
    
    return (emp != NULL);
}
    


GG::XMLElement ServerEmpireManager::CreateClientEmpireUpdate(int EmpireID)
{
    GG::XMLElement this_turn(EmpireManager::EMPIRE_UPDATE_TAG);
      
    // find whatever empire they're talking about
    Empire* emp = Lookup(EmpireID);
    
    // perform sanity check
    if(emp == NULL)
    {
        throw std::runtime_error("Invalid EmpireID passed to ServerEmpireManager::CreateClientSitrepUpdate()");
    }
    
    // convert Empire ID to string
    char sIDString[12];  // integer cannot possibly exceed 11 digits
    sprintf(sIDString, "%d", EmpireID);
        
    // set ID attribute of the update to indicate whose empire this is
    this_turn.SetAttribute("EmpireID", sIDString);
    
    // ****************************************************************
    // For version 0.1, we simply encode the states of all empires
    // since all information about all empires is always known to all others
    // ****************************************************************
    for(EmpireManager::iterator itr = begin(); itr != end(); itr++)
    {
        
        char empire_tag[20];
        sprintf(empire_tag, "Empire%d", (*itr).second->EmpireID());
        
        GG::XMLElement current_empire(empire_tag);
        current_empire.AppendChild( (*itr).second->XMLEncode(*emp) );
        this_turn.AppendChild(current_empire);
    }
    
    // make XMLDocs for last turn's state, and for this turn's state
    GG::XMLDoc last_turn;
    GG::XMLDoc this_turn_doc;
    last_turn.root_node = (*m_last_turn_empire_states.find(EmpireID)).second;
    this_turn_doc.root_node = this_turn;
    
    // diff them to produce the update patch, and return its root element
    GG::XMLDoc update_patch;
    XDiff(last_turn, this_turn_doc, update_patch);
    
    return update_patch.root_node;
}

/*
GG::XMLElement ServerEmpireManager::CreateClientSitrepUpdate(int EmpireID)
{
    GG::XMLElement update(SITREP_UPDATE_TAG);
    
    // find whatever empire they're talking about
    Empire* emp = Lookup(EmpireID);
    
    // perform sanity check
    if(emp == NULL)
    {
        throw std::runtime_error("Invalid EmpireID passed to ServerEmpireManager::CreateClientSitrepUpdate()");
    }
    
    // convert Empire ID to string
    char sIDString[12];  // integer cannot possibly exceed 11 characters
    sprintf(sIDString, "%d", EmpireID);
        
    // set ID attribute of the sitrep update to indicate whose empire this is
    update.SetAttribute("EmpireID", sIDString);
        
    // add the empire's entire sitrep to the update element
    for( Empire::SitRepItr itr = emp->SitRepBegin(); itr != emp->SitRepEnd(); itr++)
    {
        update.AppendChild((*itr)->XMLEncode());
    }
    
    
    return update;

}*/
