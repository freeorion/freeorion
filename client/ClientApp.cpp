#include "ClientApp.h"

#include "../Empire/TechManager.h"
#include "../network/XDiff.hpp"

#include <stdexcept>


// static member(s)
ClientApp* ClientApp::s_app = 0;

ClientApp::ClientApp() : 
    m_multiplayer_lobby_wnd(0),
    m_current_combat(0), 
    m_player_id(-1)
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of ClientApp");
   
    s_app = this;

    // initialize tech manager
    TechManager::instance().LoadTechTree( "" );

}

ClientApp::~ClientApp()
{
    // shutdown tech tree
    TechManager::instance().ClearAll();
}

Message ClientApp::TurnOrdersMessage(bool save_game_data/* = false*/) const
{
    GG::XMLDoc orders_doc;
    if (save_game_data)
        orders_doc.root_node.AppendChild("save_game_data");
    for (OrderSet::const_iterator order_it = m_orders.begin(); order_it != m_orders.end(); ++order_it) {
        orders_doc.root_node.AppendChild(order_it->second->XMLEncode());
    }
    return ::TurnOrdersMessage(m_player_id, -1, orders_doc);
}

void ClientApp::HandleMessage(const Message& msg)
{
    s_app->HandleMessageImpl(msg);
}


Universe& ClientApp::GetUniverse()
{
    return s_app->m_universe;
}

MultiplayerLobbyWnd* ClientApp::MultiplayerLobby()
{
    return s_app->m_multiplayer_lobby_wnd;
}

ClientEmpireManager& ClientApp::Empires()
{
    return s_app->m_empires;
}

CombatModule* ClientApp::CurrentCombat()
{
    return s_app->m_current_combat;
}

OrderSet& ClientApp::Orders()
{
    return s_app->m_orders;
}

ClientNetworkCore& ClientApp::NetworkCore()
{
    return s_app->m_network_core;
}


void ClientApp::StartTurn( )
{
    // send message
    m_network_core.SendMessage(TurnOrdersMessage());

    // clear order set
    m_orders.Reset( );
}


int ClientApp::GetNewObjectID( )
{
    int new_id = UniverseObject::INVALID_OBJECT_ID; 

    // ask the server for a new universe object ID. This is a blocking method and can timeout without a valid ID
    Message msg;
    s_app->m_network_core.SendSynchronousMessage( RequestNewObjectIDMessage( s_app->m_player_id ), msg );
    return boost::lexical_cast<int>(msg.GetText());

    return new_id;
}


void ClientApp::UpdateTurnData( GG::XMLDoc &doc )
{
    GG::XMLDoc new_doc;
    GG::XMLElement universe_data;
    GG::XMLElement empire_data;

    // we may not have a universe object if nothing has changed 
    if(doc.root_node.ContainsChild("Universe")) 
    {
        // get old universe data
        universe_data = m_universe.XMLEncode( );
        new_doc.root_node.AppendChild( universe_data );
    }
 
    if(doc.root_node.ContainsChild(EmpireManager::EMPIRE_UPDATE_TAG) )
    {
        // get old empire data
        empire_data = m_empires.EncodeEmpires( );
        new_doc.root_node.AppendChild( empire_data );
    }

    // XPatch
    XPatch( new_doc, doc );

    // write patch data for debug
    std::ofstream output("patch_merge_result.txt");
    new_doc.WriteDoc(output);
    output.close();		

    // apply universe
    if(new_doc.root_node.ContainsChild( "Universe" ) )
    {
        m_universe.SetUniverse( new_doc.root_node.Child( "Universe" ) );
    }

    // apply empire
    if(new_doc.root_node.ContainsChild( EmpireManager::EMPIRE_UPDATE_TAG ) ) 
    {
        m_empires.HandleEmpireElementUpdate( new_doc.root_node.Child( EmpireManager::EMPIRE_UPDATE_TAG ) );
    }
}
