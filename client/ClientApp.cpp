#include "ClientApp.h"

#include "../util/Logger.h"
#include "../universe/UniverseObject.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Networking.h"
#include "ClientNetworking.h"

#include <stdexcept>

ClientApp::ClientApp() :
    IApp(),
    m_networking(std::make_shared<ClientNetworking>()),
    m_empire_id(ALL_EMPIRES),
    m_current_turn(INVALID_GAME_TURN)
{}

int ClientApp::PlayerID() const
{ return m_networking->PlayerID(); }

Empire* ClientApp::GetEmpire(int empire_id)
{ return m_empires.GetEmpire(empire_id).get(); }

ObjectMap& ClientApp::EmpireKnownObjects(int empire_id) {
    // observers and moderators should have accurate info about what each empire knows
    if (m_empire_id == ALL_EMPIRES)
        return m_universe.EmpireKnownObjects(empire_id);    // returns player empire's known universe objects if empire_id == ALL_EMPIRES

    // players controlling empires with visibility limitations only know their
    // own version of the universe, and should use that
    return m_universe.Objects();
}

int ClientApp::EmpirePlayerID(int empire_id) const {
    for (const auto& [id, info] : m_player_info)
        if (info.empire_id == empire_id)
            return id;
    return Networking::INVALID_PLAYER_ID;
}

Networking::ClientType ClientApp::GetEmpireClientType(int empire_id) const
{ return GetPlayerClientType(ClientApp::EmpirePlayerID(empire_id)); }

Networking::ClientType ClientApp::GetPlayerClientType(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return Networking::ClientType::INVALID_CLIENT_TYPE;
    auto it = m_player_info.find(player_id);
    if (it != m_player_info.end())
        return it->second.client_type;
    return Networking::ClientType::INVALID_CLIENT_TYPE;
}

Networking::ClientType ClientApp::GetClientType() const
{ return GetPlayerClientType(m_networking->PlayerID()); }

void ClientApp::SetEmpireStatus(int empire_id, Message::PlayerStatus status) {
    if (auto empire = m_empires.GetEmpire(empire_id))
        empire->SetReady(status == Message::PlayerStatus::WAITING);
}

void ClientApp::StartTurn(const SaveGameUIData& ui_data)
{ m_networking->SendMessage(TurnOrdersMessage(m_orders, ui_data)); }

void ClientApp::StartTurn(const std::string& save_state_string)
{ m_networking->SendMessage(TurnOrdersMessage(m_orders, save_state_string)); }

void ClientApp::SendPartialOrders() {
    if (!m_networking || !m_networking->IsTxConnected())
        return;
    auto changes = m_orders.ExtractChanges();
    if (changes.first.empty() && changes.second.empty())
        return;
    m_networking->SendMessage(TurnPartialOrdersMessage(changes));
}

std::string ClientApp::GetVisibleObjectName(const UniverseObject& object) {
    if (object.ObjectType() == UniverseObjectType::OBJ_SYSTEM) {
        auto& system = static_cast<const System&>(object);
        return system.ApparentName(m_empire_id, m_universe);
    } else {
        return object.PublicName(m_empire_id, m_universe);
    }
}

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
