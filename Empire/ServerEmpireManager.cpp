#include "ServerEmpireManager.h"

#include "../universe/Planet.h"
#include "../server/ServerApp.h"
#include "../universe/Universe.h"
#include "../network/XDiff.hpp"

#include <boost/lexical_cast.hpp>

#include <stdexcept>


ServerEmpireManager::ServerEmpireManager() :
    EmpireManager()
{
    // nothing to do yet
}


Empire* ServerEmpireManager::CreateEmpire(int id,
                                          const std::string& name, 
                                          const GG::Clr& color, 
                                          int planetID, 
                                          Empire::ControlStatus state)
{
    Empire* emp = new Empire(name, id, color, state);
    if(emp == NULL)
    {
        throw std::runtime_error("Memory allocation failed.  ServerEmpireManager::CreateEmpire()");
    }

    id++;
    emp->AddPlanet(planetID);

    // Add system homeplanet is in to the ExploredSystem list
    ServerApp* server_app = ServerApp::GetApp();
    Universe* universe = &(server_app->GetUniverse());
    UniverseObject* uni_obj = universe->Object(planetID);
    Planet* planet = dynamic_cast<Planet*>(uni_obj);
    emp->AddExploredSystem(planet->SystemID());

    InsertEmpire(emp);

    return emp;
}



bool ServerEmpireManager::EliminateEmpire(int id)
{
    Empire* emp = Lookup(id);
    RemoveEmpire(emp);
    
    // prevent memory leak
    delete emp;
    
    return (emp != NULL);
}
    


GG::XMLElement ServerEmpireManager::CreateClientEmpireUpdate(int empire_id)
{
    GG::XMLElement this_turn(EmpireManager::EMPIRE_UPDATE_TAG);

    // find whatever empire they're talking about
    Empire* emp = Lookup(empire_id);

    // perform sanity check
    if (!emp)
    {
        throw std::runtime_error("Invalid empire_id passed to ServerEmpireManager::CreateClientEmpireUpdate()");
    }

    // set ID attribute of the update to indicate whose empire this is
    this_turn.SetAttribute("empire_id", boost::lexical_cast<std::string>(empire_id));

    // ****************************************************************
    // For version 0.1, we simply encode the states of all empires
    // since all information about all empires is always known to all others
    // ****************************************************************
    for (EmpireManager::iterator it = begin(); it != end(); it++)
    {
        GG::XMLElement current_empire("Empire" + boost::lexical_cast<std::string>(it->second->EmpireID()));
        current_empire.AppendChild(it->second->XMLEncode(*emp));
        this_turn.AppendChild(current_empire);
    }

    return this_turn;
}

/*
GG::XMLElement ServerEmpireManager::CreateClientSitrepUpdate(int empire_id)
{
    GG::XMLElement update(SITREP_UPDATE_TAG);
    
    // find whatever empire they're talking about
    Empire* emp = Lookup(empire_id);
    
    // perform sanity check
    if(emp == NULL)
    {
        throw std::runtime_error("Invalid empire_id passed to ServerEmpireManager::CreateClientSitrepUpdate()");
    }
    
    // convert Empire ID to string
    char sIDString[12];  // integer cannot possibly exceed 11 characters
    sprintf(sIDString, "%d", empire_id);
        
    // set ID attribute of the sitrep update to indicate whose empire this is
    update.SetAttribute("empire_id", sIDString);
        
    // add the empire's entire sitrep to the update element
    for( Empire::SitRepItr itr = emp->SitRepBegin(); itr != emp->SitRepEnd(); itr++)
    {
        update.AppendChild((*itr)->XMLEncode());
    }
    
    
    return update;

}*/
