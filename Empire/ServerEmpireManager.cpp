#include "ServerEmpireManager.h"

#include "../util/MultiplayerCommon.h"
#include "../universe/Planet.h"
#include "../server/ServerApp.h"
#include "../universe/Universe.h"
#include "../network/XDiff.hpp"

#include <boost/lexical_cast.hpp>

#include <stdexcept>


namespace {
    bool temp_header_bool = RecordHeaderFile(ServerEmpireManagerRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


ServerEmpireManager::ServerEmpireManager() :
    EmpireManager()
{
}


Empire* ServerEmpireManager::CreateEmpire(int id,
                                          const std::string& name, 
                                          const std::string& player_name, 
                                          const GG::Clr& color, 
                                          int planetID)
{
    Empire* emp = new Empire(name, player_name, id, color, planetID);
    if(emp == NULL)
    {
        throw std::runtime_error("Memory allocation failed.  ServerEmpireManager::CreateEmpire()");
    }

    id++;

    // Add system homeplanet is in to the ExploredSystem list
    ServerApp* server_app = ServerApp::GetApp();
    Universe& universe = server_app->GetUniverse();
    Planet* planet = universe.Object<Planet>(planetID);
    emp->AddExploredSystem(planet->SystemID());

    InsertEmpire(emp);

    return emp;
}



bool ServerEmpireManager::EliminateEmpire(int id)
{
    Empire* emp = Lookup(id);
    RemoveEmpire(emp);
    
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

    for (EmpireManager::iterator it = begin(); it != end(); it++)
    {
        GG::XMLElement current_empire("Empire" + boost::lexical_cast<std::string>(it->second->EmpireID()));
	// Only append visible information
        current_empire.AppendChild(it->second->XMLEncode(*emp));
        this_turn.AppendChild(current_empire);
    }

    return this_turn;
}
