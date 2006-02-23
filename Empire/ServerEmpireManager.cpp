#include "ServerEmpireManager.h"

#include "../util/MultiplayerCommon.h"
#include "../universe/Planet.h"
#include "../server/ServerApp.h"
#include "../universe/Universe.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>


namespace {
    bool temp_header_bool = RecordHeaderFile(ServerEmpireManagerRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}


ServerEmpireManager::ServerEmpireManager() :
    EmpireManager()
{}


Empire* ServerEmpireManager::CreateEmpire(int id,
                                          const std::string& name, 
                                          const std::string& player_name, 
                                          const GG::Clr& color, 
                                          int planetID)
{
    Empire* emp = new Empire(name, player_name, id, color, planetID);

    ++id;

    // Add system homeplanet is in to the explored system list
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
    


XMLElement ServerEmpireManager::CreateClientEmpireUpdate(int empire_id)
{
    XMLElement this_turn(EmpireManager::EMPIRE_UPDATE_TAG);

    Empire* emp = Lookup(empire_id);
    if (!emp)
        throw std::runtime_error("Invalid empire_id passed to ServerEmpireManager::CreateClientEmpireUpdate()");

    this_turn.SetAttribute("empire_id", boost::lexical_cast<std::string>(empire_id));

    for (EmpireManager::iterator it = begin(); it != end(); ++it) {
        XMLElement current_empire("Empire" + boost::lexical_cast<std::string>(it->second->EmpireID()));
        current_empire.AppendChild(it->second->XMLEncode(emp->EmpireID()));
        this_turn.AppendChild(current_empire);
    }

    return this_turn;
}
