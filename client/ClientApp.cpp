#include "ClientApp.h"

#include "../util/MultiplayerCommon.h"
#include "../util/Serialize.h"
#include "../universe/UniverseObject.h"

#include <stdexcept>


// static member(s)
ClientApp* ClientApp::s_app = 0;

ClientApp::ClientApp() : 
    m_multiplayer_lobby_wnd(0),
    m_current_combat(0), 
    m_player_id(-1),
    m_empire_id(-1),
    m_current_turn(INVALID_GAME_TURN)
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of ClientApp");
    s_app = this;
}

ClientApp::~ClientApp()
{}

const std::string& ClientApp::PlayerName() const
{ return m_player_name; }

int ClientApp::PlayerID() const
{ return m_player_id; }

int ClientApp::EmpireID() const
{ return m_empire_id; }

int ClientApp::CurrentTurn() const
{ return m_current_turn; }

const Universe& ClientApp::GetUniverse() const
{ return m_universe; }

const EmpireManager& ClientApp::Empires() const
{ return m_empires; }

const OrderSet& ClientApp::Orders() const
{ return m_orders; }

const ClientNetworking& ClientApp::Networking() const
{ return m_networking; }

void ClientApp::StartTurn()
{
    m_networking.SendMessage(TurnOrdersMessage(m_player_id, m_orders));
    m_orders.Reset();
}

Universe& ClientApp::GetUniverse()
{ return m_universe; }

EmpireManager& ClientApp::Empires()
{ return m_empires; }

OrderSet& ClientApp::Orders()
{ return m_orders; }

ClientNetworking& ClientApp::Networking()
{ return m_networking; }

MultiplayerLobbyWnd* ClientApp::MultiplayerLobby()
{ return m_multiplayer_lobby_wnd; }

CombatModule* ClientApp::CurrentCombat()
{ return m_current_combat; }

int ClientApp::GetNewObjectID()
{
    Message msg;
    m_networking.SendSynchronousMessage(RequestNewObjectIDMessage(m_player_id), msg);
    return boost::lexical_cast<int>(msg.Text());
}

ClientApp* ClientApp::GetApp()
{ return s_app; }

void ClientApp::SetPlayerName(const std::string& name)
{ m_player_name = name; }

void ClientApp::SetPlayerID(int id)
{ m_player_id = id; }

void ClientApp::SetEmpireID(int id)
{ m_empire_id = id; }

void ClientApp::SetCurrentTurn(int turn)
{ m_current_turn = turn; }

void ClientApp::SetMultiplayerLobby(MultiplayerLobbyWnd* wnd)
{ m_multiplayer_lobby_wnd = wnd; }

void ClientApp::SetCurrentCombat(CombatModule* combat)
{ m_current_combat = combat; }

int& ClientApp::EmpireIDRef()
{ return m_empire_id; }

int& ClientApp::CurrentTurnRef()
{ return m_current_turn; }
