#include "ClientApp.h"

#include "../util/Logger.h"
#include "../util/Serialize.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Networking.h"
#include "../network/ClientNetworking.h"

#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

ClientApp::ClientApp() :
    IApp(),
    m_universe(),
    m_networking(std::make_shared<ClientNetworking>()),
    m_empire_id(ALL_EMPIRES),
    m_current_turn(INVALID_GAME_TURN)
{}

int ClientApp::PlayerID() const
{ return m_networking->PlayerID(); }

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

const EmpireManager& ClientApp::Empires() const
{ return m_empires; }

Empire* ClientApp::GetEmpire(int empire_id)
{ return m_empires.GetEmpire(empire_id); }

SupplyManager& ClientApp::GetSupplyManager()
{ return m_supply_manager; }

std::shared_ptr<UniverseObject> ClientApp::GetUniverseObject(int object_id)
{ return GetUniverse().Objects().Object(object_id); }

ObjectMap& ClientApp::EmpireKnownObjects(int empire_id) {
    // observers and moderators should have accurate info about what each empire knows
    if (m_empire_id == ALL_EMPIRES)
        return m_universe.EmpireKnownObjects(empire_id);    // returns player empire's known universe objects if empire_id == ALL_EMPIRES

    // players controlling empires with visibility limitations only know their
    // own version of the universe, and should use that
    return m_universe.Objects();
}

std::shared_ptr<UniverseObject> ClientApp::EmpireKnownObject(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object(object_id); }

const OrderSet& ClientApp::Orders() const
{ return m_orders; }

const ClientNetworking& ClientApp::Networking() const
{ return *m_networking; }

int ClientApp::EmpirePlayerID(int empire_id) const {
    for (const auto& entry : m_player_info)
        if (entry.second.empire_id == empire_id)
            return entry.first;
    return Networking::INVALID_PLAYER_ID;
}

Networking::ClientType ClientApp::GetEmpireClientType(int empire_id) const
{ return GetPlayerClientType(ClientApp::EmpirePlayerID(empire_id)); }

Networking::ClientType ClientApp::GetPlayerClientType(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return Networking::INVALID_CLIENT_TYPE;
    auto it = m_player_info.find(player_id);
    if (it != m_player_info.end())
        return it->second.client_type;
    return Networking::INVALID_CLIENT_TYPE;
}

Networking::ClientType ClientApp::GetClientType() const
{ return GetPlayerClientType(m_networking->PlayerID()); }

const std::map<int, PlayerInfo>& ClientApp::Players() const
{ return m_player_info; }

std::map<int, PlayerInfo>& ClientApp::Players()
{ return m_player_info; }

const std::map<int, Message::PlayerStatus>& ClientApp::EmpireStatus() const
{ return m_empire_status; }

std::map<int, Message::PlayerStatus>& ClientApp::EmpireStatus()
{ return m_empire_status; }

void ClientApp::SetEmpireStatus(int empire_id, Message::PlayerStatus status) {
    if (empire_id == ALL_EMPIRES)
        return;
    m_empire_status[empire_id] = status;
}

void ClientApp::StartTurn(const SaveGameUIData& ui_data)
{ m_networking->SendMessage(TurnOrdersMessage(m_orders, ui_data)); }

void ClientApp::StartTurn(const std::string& save_state_string)
{ m_networking->SendMessage(TurnOrdersMessage(m_orders, save_state_string)); }

void ClientApp::SendPartialOrders() {
    auto changes = m_orders.ExtractChanges();
    if (changes.first.empty() && changes.second.empty())
        return;
    m_networking->SendMessage(TurnPartialOrdersMessage(changes));
}

void ClientApp::HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id) {
    switch (phase_id) {
    case Message::WAITING_FOR_PLAYERS:
        // Orders have been received by server, so clear the orders.
        m_orders.Reset();
        break;
    case Message::FLEET_MOVEMENT:
    case Message::COMBAT:
    case Message::EMPIRE_PRODUCTION:
    case Message::PROCESSING_ORDERS:
    case Message::COLONIZE_AND_SCRAP:
    case Message::DOWNLOADING:
    case Message::LOADING_GAME:
    case Message::GENERATING_UNIVERSE:
    case Message::STARTING_AIS:
    default:
        break;
    }
}

OrderSet& ClientApp::Orders()
{ return m_orders; }

ClientNetworking& ClientApp::Networking()
{ return *m_networking; }

std::string ClientApp::GetVisibleObjectName(std::shared_ptr<const UniverseObject> object) {
    if (!object) {
        ErrorLogger() << "ClientApp::GetVisibleObjectName(): expected non null object pointer.";
        return std::string();
    }

    std::string name_text = object->PublicName(m_empire_id);
    if (auto system = std::dynamic_pointer_cast<const System>(object))
        name_text = system->ApparentName(m_empire_id);

    return name_text;
}

ClientApp* ClientApp::GetApp()
{ return static_cast<ClientApp*>(s_app); }

void ClientApp::SetEmpireID(int empire_id)
{ m_empire_id = empire_id; }

void ClientApp::SetCurrentTurn(int turn)
{ m_current_turn = turn; }

bool ClientApp::VerifyCheckSum(const Message& msg) {
    std::map<std::string, unsigned int> server_checksums;
    ExtractContentCheckSumMessageData(msg, server_checksums);

    auto client_checksums = CheckSumContent();

    if (server_checksums == client_checksums) {
        InfoLogger() << "Checksum received from server matches client checksum.";
        return true;
    } else {
        WarnLogger() << "Checksum received from server does not match client checksum.";
        for (const auto& name_and_checksum : server_checksums) {
            const auto& name = name_and_checksum.first;
            const auto client_checksum = client_checksums[name];
            if (client_checksum != name_and_checksum.second)
                WarnLogger() << "Checksum for " << name << " on server "
                             << name_and_checksum.second << " != client "
                             << client_checksum;
        }
        return false;
    }
}
