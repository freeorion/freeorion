#include "ClientApp.h"

#include "../network/Message.h"

#include "../Empire/TechManager.h"

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


void ClientApp::StartTurn(  )
{
    GG::XMLDoc orders_doc;
    OrderSet::const_iterator  order_it;
    GG::XMLElement order_elt;

    /// execute order set
    for ( order_it = m_orders.begin(); order_it != m_orders.end(); ++order_it)
    {
      order_elt = order_it->second->XMLEncode( );               
      
      orders_doc.root_node.AppendChild( order_elt );

    }

    /// clear order set
    m_orders.Reset( );

    /// send message
    m_network_core.SendMessage(TurnOrdersMessage( m_player_id, -1, orders_doc ) );
}


int ClientApp::GetNewObjectID( )
{
    int new_id = UniverseObject::INVALID_OBJECT_ID; 

    // ask the server for a new universe object ID. This is a blocking method and can timeout without a valid ID
    s_app->m_network_core.SendSynchronousMessage( RequestNewObjectIDMessage( s_app->m_player_id, -1 ), new_id );

    return new_id;
}
 
