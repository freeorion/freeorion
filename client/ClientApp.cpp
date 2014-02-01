#include "ClientApp.h"

#include "../combat/CombatOrder.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Serialize.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Networking.h"

#include <stdexcept>
#include <boost/bind.hpp>

ClientApp::ClientApp() :
    IApp(),
    m_universe(),
    m_empire_id(ALL_EMPIRES),
    m_current_turn(INVALID_GAME_TURN)
{
#ifdef FREEORION_BUILD_HUMAN
    EmpireEliminatedSignal.connect(boost::bind(&Universe::HandleEmpireElimination, &m_universe, _1));
#endif
}

ClientApp::~ClientApp()
{}

int ClientApp::PlayerID() const
{ return m_networking.PlayerID(); }

int ClientApp::EmpireID() const
{ return m_empire_id; }

int ClientApp::CurrentTurn() const
{ return m_current_turn; }

Universe& ClientApp::GetUniverse()
{ return m_universe; }

const Universe& ClientApp::GetUniverse() const
{ return m_universe; }

GalaxySetupData& ClientApp::GetGalaxySetupData()
{ return m_galaxy_setup_data; }

const GalaxySetupData& ClientApp::GetGalaxySetupData() const
{ return m_galaxy_setup_data; }

EmpireManager& ClientApp::Empires()
{ return m_empires; }

TemporaryPtr<UniverseObject> ClientApp::GetUniverseObject(int object_id)
{ return GetUniverse().Objects().Object(object_id); }

ObjectMap& ClientApp::EmpireKnownObjects(int empire_id) {
    // observers and moderators should have accurate info about what each empire knows
    if (m_empire_id == ALL_EMPIRES)
        return m_universe.EmpireKnownObjects(empire_id);    // returns player empire's known universe objects if empire_id == ALL_EMPIRES

    // players controlling empires with visibility limitations only know their
    // own version of the universe, and should use that
    return m_universe.Objects();
}

TemporaryPtr<UniverseObject> ClientApp::EmpireKnownObject(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object(object_id); }

const OrderSet& ClientApp::Orders() const
{ return m_orders; }

const CombatOrderSet& ClientApp::CombatOrders() const
{ return m_combat_orders; }

const ClientNetworking& ClientApp::Networking() const
{ return m_networking; }

int ClientApp::EmpirePlayerID(int empire_id) const {
    for (std::map<int, PlayerInfo>::const_iterator it = m_player_info.begin(); it != m_player_info.end(); ++it)
        if (it->second.empire_id == empire_id)
            return it->first;
    return Networking::INVALID_PLAYER_ID;
}

const Networking::ClientType ClientApp::GetEmpireClientType(int empire_id) const
{ return GetPlayerClientType(ClientApp::EmpirePlayerID(empire_id)); }

const Networking::ClientType ClientApp::GetPlayerClientType(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return Networking::INVALID_CLIENT_TYPE;
    std::map<int, PlayerInfo>::const_iterator it = m_player_info.find(player_id);
    if (it != m_player_info.end())
        return it->second.client_type;
    return Networking::INVALID_CLIENT_TYPE;
}

const Networking::ClientType ClientApp::GetClientType() const
{ return GetPlayerClientType(m_networking.PlayerID()); }

const std::map<int, PlayerInfo>& ClientApp::Players() const
{ return m_player_info; }

std::map<int, PlayerInfo>& ClientApp::Players()
{ return m_player_info; }

void ClientApp::StartTurn() {
    m_networking.SendMessage(TurnOrdersMessage(m_networking.PlayerID(), m_orders));
    m_orders.Reset();
}

void ClientApp::SendCombatSetup() {
    m_networking.SendMessage(CombatTurnOrdersMessage(m_networking.PlayerID(), m_combat_orders));
    m_combat_orders.clear();
}

void ClientApp::StartCombatTurn() {
    m_networking.SendMessage(CombatTurnOrdersMessage(m_networking.PlayerID(), m_combat_orders));
    m_combat_orders.clear();
}

OrderSet& ClientApp::Orders()
{ return m_orders; }

CombatOrderSet& ClientApp::CombatOrders()
{ return m_combat_orders; }

ClientNetworking& ClientApp::Networking()
{ return m_networking; }

std::string ClientApp::GetVisibleObjectName(TemporaryPtr<const UniverseObject> object) {
    if (!object) {
        Logger().errorStream() << "ServerApp::GetVisibleObjectName(): expected non null object pointer.";
        return std::string();
    }

    std::string name_text = object->PublicName(m_empire_id);
    if (TemporaryPtr<const System> system = boost::dynamic_pointer_cast<const System>(object))
        name_text = system->ApparentName(m_empire_id);

    return name_text;
}

int ClientApp::GetNewObjectID() {
    Message msg;
    m_networking.SendSynchronousMessage(RequestNewObjectIDMessage(m_networking.PlayerID()), msg);
    std::string text = msg.Text();
    if (text.empty())
        throw std::runtime_error("ClientApp::GetNewObjectID() didn't get a new object ID");
    return boost::lexical_cast<int>(text);
}

int ClientApp::GetNewDesignID() {
    Message msg;
    m_networking.SendSynchronousMessage(RequestNewDesignIDMessage(m_networking.PlayerID()), msg);
    std::string text = msg.Text();
    if (text.empty())
        throw std::runtime_error("ClientApp::GetNewDesignID() didn't get a new design ID");
    return boost::lexical_cast<int>(text);
}

ClientApp* ClientApp::GetApp()
{ return static_cast<ClientApp*>(s_app); }

void ClientApp::SetEmpireID(int id)
{ m_empire_id = id; }

void ClientApp::SetCurrentTurn(int turn)
{ m_current_turn = turn; }
