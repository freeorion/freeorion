#include "ClientApp.h"

#include "../network/Message.h"

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
}

ClientApp::~ClientApp()
{
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

    /// send message
    m_network_core.SendMessage(TurnOrdersMessage( m_player_id, -1, orders_doc ) );
}

 
