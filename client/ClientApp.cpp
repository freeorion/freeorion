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


ClientEmpireManager& ClientApp::Empire()
{
   return s_app->m_empire;
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


