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
{
    return m_player_name;
}

int ClientApp::PlayerID() const
{
    return m_player_id;
}

int ClientApp::EmpireID() const
{
    return m_empire_id;
}

int ClientApp::CurrentTurn() const
{
    return m_current_turn;
}

Message ClientApp::TurnOrdersMessage(bool save_game_data/* = false*/) const
{
    if (save_game_data) {
        XMLDoc orders_doc;
        orders_doc.root_node.AppendChild(XMLElement("Orders"));
        for (OrderSet::const_iterator order_it = m_orders.begin(); order_it != m_orders.end(); ++order_it) {
            orders_doc.root_node.LastChild().AppendChild(order_it->second->XMLEncode());
        }
        return ClientSaveDataMessage(m_player_id, orders_doc);
    } else {
        std::ostringstream os;
        {
            boost::archive::xml_oarchive oa(os);
            Serialize(&oa, m_orders);
        }
        return ::TurnOrdersMessage(m_player_id, os.str());
    }
}

void ClientApp::StartTurn()
{
    m_network_core.SendMessage(TurnOrdersMessage());
    m_orders.Reset();
}

Universe& ClientApp::GetUniverse()
{
    return m_universe;
}

ClientEmpireManager& ClientApp::Empires()
{
    return m_empires;
}

OrderSet& ClientApp::Orders()
{
    return m_orders;
}

ClientNetworkCore& ClientApp::NetworkCore()
{
    return m_network_core;
}

MultiplayerLobbyWnd* ClientApp::MultiplayerLobby()
{
    return m_multiplayer_lobby_wnd;
}

CombatModule* ClientApp::CurrentCombat()
{
    return m_current_combat;
}

void ClientApp::HandleMessage(const Message& msg)
{
    s_app->HandleMessageImpl(msg);
}

void ClientApp::HandleServerDisconnect()
{
    s_app->HandleServerDisconnectImpl();
}

int ClientApp::GetNewObjectID()
{
    int new_id = UniverseObject::INVALID_OBJECT_ID; 

    // ask the server for a new universe object ID. This is a blocking method and can timeout without a valid ID
    Message msg;
    s_app->m_network_core.SendSynchronousMessage( RequestNewObjectIDMessage( s_app->m_player_id ), msg );

    if (msg.GetText().length()>0)
      new_id = boost::lexical_cast<int>(msg.GetText());

    return new_id;
}

ClientApp* ClientApp::GetApp()
{
    return s_app;
}

void ClientApp::UpdateTurnData(const XMLDoc &new_doc)
{
    if (new_doc.root_node.ContainsChild(EmpireManager::EMPIRE_UPDATE_TAG))
        m_empires.HandleEmpireElementUpdate(new_doc.root_node.Child(EmpireManager::EMPIRE_UPDATE_TAG));
    if (new_doc.root_node.ContainsChild("Universe"))
        m_universe.SetUniverse(new_doc.root_node.Child("Universe"));
}
