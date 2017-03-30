#include "Message.h"

#include "../combat/CombatLogManager.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../Empire/Diplomacy.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/ModeratorAction.h"
#include "../util/SaveGamePreviewUtils.h"
#include "../universe/Meter.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../universe/Species.h"
#include "../util/OptionsDB.h"
#include "../util/Serialize.h"
#include "../util/ScopedTimer.h"
#include "../util/i18n.h"
#include "../util/Version.h"
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/timer.hpp>

#include <zlib.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>


namespace {
    const std::string DUMMY_EMPTY_MESSAGE = "Lathanda";
    const std::string ACKNOWLEDGEMENT = "ACK";
}

////////////////////////////////////////////////
// Free Functions
////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const MessagePacket& msg) {
    os << "Message: "
       << msg.Type() << " "
       << msg.SendingPlayer();

    if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID)
        os << "(server/unknown) --> ";
    else
        os << " --> ";

    os << msg.ReceivingPlayer();

    if (msg.ReceivingPlayer() == Networking::INVALID_PLAYER_ID)
        os << "(server/unknown)";

    os << " \"" << msg.Text() << "\"\n";

    return os;
}


////////////////////////////////////////////////
// MessagePacket
////////////////////////////////////////////////
MessagePacket::MessagePacket() :
    m_type(UNDEFINED),
    m_sending_player(0),
    m_receiving_player(0),
    m_synchronous_response(false),
    m_message_size(0),
    m_message_text()
{}

MessagePacket::MessagePacket(MessageType type, int sending_player, int receiving_player,
                 const std::string& text, bool synchronous_response/* = false*/) :
    m_type(type),
    m_sending_player(sending_player),
    m_receiving_player(receiving_player),
    m_synchronous_response(synchronous_response),
    m_message_size(text.size()),
    m_message_text(new char[text.size()])
{ std::copy(text.begin(), text.end(), m_message_text.get()); }

MessagePacket::MessageType MessagePacket::Type() const
{ return m_type; }

int MessagePacket::SendingPlayer() const
{ return m_sending_player; }

int MessagePacket::ReceivingPlayer() const
{ return m_receiving_player; }

bool MessagePacket::SynchronousResponse() const
{ return m_synchronous_response; }

std::size_t MessagePacket::Size() const
{ return m_message_size; }

const char* MessagePacket::Data() const
{ return m_message_text.get(); }

std::string MessagePacket::Text() const
{ return std::string(m_message_text.get(), m_message_size); }

void MessagePacket::Resize(std::size_t size) {
    m_message_size = size;
    m_message_text.reset(new char[m_message_size]);
}

char* MessagePacket::Data()
{ return m_message_text.get(); }

void MessagePacket::Swap(MessagePacket& rhs) {
    std::swap(m_type, rhs.m_type);
    std::swap(m_sending_player, rhs.m_sending_player);
    std::swap(m_receiving_player, rhs.m_receiving_player);
    std::swap(m_synchronous_response, rhs.m_synchronous_response);
    std::swap(m_message_size, rhs.m_message_size);
    std::swap(m_message_text, rhs.m_message_text);
}

bool operator==(const MessagePacket& lhs, const MessagePacket& rhs) {
    return
        lhs.Type() == rhs.Type() &&
        lhs.SendingPlayer() == rhs.SendingPlayer() &&
        lhs.ReceivingPlayer() == rhs.ReceivingPlayer() &&
        lhs.Text() == rhs.Text();
}

bool operator!=(const MessagePacket& lhs, const MessagePacket& rhs)
{ return !(lhs == rhs); }

void swap(MessagePacket& lhs, MessagePacket& rhs)
{ lhs.Swap(rhs); }

void BufferToHeader(const MessagePacket::HeaderBuffer& buffer, MessagePacket& message) {
    message.m_type = static_cast<MessagePacket::MessageType>(buffer[0]);
    message.m_sending_player = buffer[1];
    message.m_receiving_player = buffer[2];
    message.m_synchronous_response = (buffer[3] != 0);
    message.m_message_size = buffer[4];
}

void HeaderToBuffer(const MessagePacket& message, MessagePacket::HeaderBuffer& buffer) {
    buffer[0] = message.Type();
    buffer[1] = message.SendingPlayer();
    buffer[2] = message.ReceivingPlayer();
    buffer[3] = message.SynchronousResponse();
    buffer[4] = message.Size();
}

////////////////////////////////////////////////
// MessagePacket named ctors
////////////////////////////////////////////////
MessagePacket ErrorMessage(const std::string& problem, bool fatal/* = true*/) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(problem)
           << BOOST_SERIALIZATION_NVP(fatal);
    }
    return MessagePacket(MessagePacket::ERROR_MSG, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket ErrorMessage(int player_id, const std::string& problem, bool fatal/* = true*/) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(problem)
           << BOOST_SERIALIZATION_NVP(fatal);
    }
    return MessagePacket(MessagePacket::ERROR_MSG, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket HostSPGameMessage(const SinglePlayerSetupData& setup_data) {
    std::ostringstream os;
    {
        std::string client_version_string = FreeOrionVersionString();
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(setup_data)
           << BOOST_SERIALIZATION_NVP(client_version_string);
    }
    return MessagePacket(MessagePacket::HOST_SP_GAME, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket HostMPGameMessage(const std::string& host_player_name)
{
    std::ostringstream os;
    {
        std::string client_version_string = FreeOrionVersionString();
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(host_player_name)
           << BOOST_SERIALIZATION_NVP(client_version_string);
    }
    return MessagePacket(MessagePacket::HOST_MP_GAME, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket JoinGameMessage(const std::string& player_name, Networking::ClientType client_type) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        std::string client_version_string = FreeOrionVersionString();
        oa << BOOST_SERIALIZATION_NVP(player_name)
           << BOOST_SERIALIZATION_NVP(client_type)
           << BOOST_SERIALIZATION_NVP(client_version_string);
    }
    return MessagePacket(MessagePacket::JOIN_GAME, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket HostIDMessage(int host_player_id) {
    return MessagePacket(MessagePacket::HOST_ID, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID,
                   std::to_string(host_player_id));
}

MessagePacket GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const GalaxySetupData& galaxy_setup_data,
                         bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = false;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = false;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return MessagePacket(MessagePacket::GAME_START, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const SaveGameUIData* ui_data,
                         const GalaxySetupData& galaxy_setup_data,
                         bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = (ui_data != nullptr);
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            if (ui_data_available)
                oa << boost::serialization::make_nvp("ui_data", *ui_data);
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = (ui_data != nullptr);
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            if (ui_data_available)
                oa << boost::serialization::make_nvp("ui_data", *ui_data);
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return MessagePacket(MessagePacket::GAME_START, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         CombatLogManager& combat_logs, const SupplyManager& supply,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const std::string* save_state_string,
                         const GalaxySetupData& galaxy_setup_data,
                         bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            bool save_state_string_available = (save_state_string != nullptr);
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            if (save_state_string_available)
                oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            bool save_state_string_available = (save_state_string != nullptr);
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            if (save_state_string_available)
                oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return MessagePacket(MessagePacket::GAME_START, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket HostSPAckMessage(int player_id)
{ return MessagePacket(MessagePacket::HOST_SP_GAME, Networking::INVALID_PLAYER_ID, player_id, ACKNOWLEDGEMENT); }

MessagePacket HostMPAckMessage(int player_id)
{ return MessagePacket(MessagePacket::HOST_MP_GAME, Networking::INVALID_PLAYER_ID, player_id, ACKNOWLEDGEMENT); }

MessagePacket JoinAckMessage(int player_id)
{ return MessagePacket(MessagePacket::JOIN_GAME, Networking::INVALID_PLAYER_ID, player_id, ACKNOWLEDGEMENT); }

MessagePacket TurnOrdersMessage(int sender, const OrderSet& orders) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders);
    }
    return MessagePacket(MessagePacket::TURN_ORDERS, sender, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket TurnProgressMessage(MessagePacket::TurnProgressPhase phase_id, int player_id) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(phase_id);
    }
    return MessagePacket(MessagePacket::TURN_PROGRESS, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket PlayerStatusMessage(int player_id, int about_player_id, MessagePacket::PlayerStatus player_status) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(about_player_id)
           << BOOST_SERIALIZATION_NVP(player_status);
    }
    return MessagePacket(MessagePacket::PLAYER_STATUS, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket TurnUpdateMessage(int player_id, int empire_id, int current_turn,
                          const EmpireManager& empires, const Universe& universe,
                          const SpeciesManager& species, CombatLogManager& combat_logs,
                          const SupplyManager& supply,
                          const std::map<int, PlayerInfo>& players,
                          bool use_binary_serialization)
{
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn);
            oa << BOOST_SERIALIZATION_NVP(empires);
            oa << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        } else {
            freeorion_xml_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn)
               << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(oa, 1);
            oa << BOOST_SERIALIZATION_NVP(supply);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        }
    }
    return MessagePacket(MessagePacket::TURN_UPDATE, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket TurnPartialUpdateMessage(int player_id, int empire_id, const Universe& universe,
                                 bool use_binary_serialization) {
    std::ostringstream os;
    {
        if (use_binary_serialization) {
            freeorion_bin_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            Serialize(oa, universe);
        } else {
            freeorion_xml_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            Serialize(oa, universe);
        }
    }
    return MessagePacket(MessagePacket::TURN_PARTIAL_UPDATE, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

MessagePacket ClientSaveDataMessage(int sender, const OrderSet& orders, const SaveGameUIData& ui_data) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders);
        bool ui_data_available = true;
        bool save_state_string_available = false;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(ui_data)
           << BOOST_SERIALIZATION_NVP(save_state_string_available);
    }
    return MessagePacket(MessagePacket::CLIENT_SAVE_DATA, sender, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket ClientSaveDataMessage(int sender, const OrderSet& orders, const std::string& save_state_string) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders);
        bool ui_data_available = false;
        bool save_state_string_available = true;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(save_state_string_available)
           << BOOST_SERIALIZATION_NVP(save_state_string);
}
    return MessagePacket(MessagePacket::CLIENT_SAVE_DATA, sender, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket ClientSaveDataMessage(int sender, const OrderSet& orders) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        Serialize(oa, orders);
        bool ui_data_available = false;
        bool save_state_string_available = false;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(save_state_string_available);
    }
    return MessagePacket(MessagePacket::CLIENT_SAVE_DATA, sender, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket RequestNewObjectIDMessage(int sender)
{ return MessagePacket(MessagePacket::REQUEST_NEW_OBJECT_ID, sender, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE); }

MessagePacket DispatchObjectIDMessage(int player_id, int new_id) {
    return MessagePacket(MessagePacket::DISPATCH_NEW_OBJECT_ID, Networking::INVALID_PLAYER_ID, player_id,
                   std::to_string(new_id), true);
}

MessagePacket RequestNewDesignIDMessage(int sender)
{ return MessagePacket(MessagePacket::REQUEST_NEW_DESIGN_ID, sender, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE, true); }

MessagePacket DispatchDesignIDMessage(int player_id, int new_id) {
    return MessagePacket(MessagePacket::DISPATCH_NEW_DESIGN_ID, Networking::INVALID_PLAYER_ID, player_id,
                   std::to_string(new_id), true);
}

MessagePacket HostSaveGameInitiateMessage(int sender, const std::string& filename)
{ return MessagePacket(MessagePacket::SAVE_GAME_INITIATE, sender, Networking::INVALID_PLAYER_ID, filename); }

MessagePacket ServerSaveGameDataRequestMessage(int receiver, bool synchronous_response) {
    return MessagePacket(MessagePacket::SAVE_GAME_DATA_REQUEST, Networking::INVALID_PLAYER_ID,
                   receiver, DUMMY_EMPTY_MESSAGE, synchronous_response);
}

MessagePacket ServerSaveGameCompleteMessage(const std::string& save_filename, int bytes_written) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(save_filename)
           << BOOST_SERIALIZATION_NVP(bytes_written);
    }
    return MessagePacket(MessagePacket::SAVE_GAME_COMPLETE, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket GlobalChatMessage(int sender, const std::string& msg)
{ return MessagePacket(MessagePacket::PLAYER_CHAT, sender, Networking::INVALID_PLAYER_ID, msg); }

MessagePacket SingleRecipientChatMessage(int sender, int receiver, const std::string& msg)
{ return MessagePacket(MessagePacket::PLAYER_CHAT, sender, receiver, msg); }

MessagePacket DiplomacyMessage(int sender, int receiver, const DiplomaticMessage& diplo_message) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(diplo_message);
    }
    return MessagePacket(MessagePacket::DIPLOMACY, sender, receiver, os.str());
}

MessagePacket DiplomaticStatusMessage(int receiver, const DiplomaticStatusUpdateInfo& diplo_update) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
           << BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
           << BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);
    }
    return MessagePacket(MessagePacket::DIPLOMATIC_STATUS, Networking::INVALID_PLAYER_ID, receiver, os.str());
}

MessagePacket EndGameMessage(int receiver, MessagePacket::EndGameReason reason,
                       const std::string& reason_player_name/* = ""*/)
{
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(reason)
           << BOOST_SERIALIZATION_NVP(reason_player_name);
    }
    return MessagePacket(MessagePacket::END_GAME, Networking::INVALID_PLAYER_ID, receiver, os.str());
}

MessagePacket AIEndGameAcknowledgeMessage(int sender)
{ return MessagePacket(MessagePacket::AI_END_GAME_ACK, sender, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE); }

MessagePacket ModeratorActionMessage(int sender, const Moderator::ModeratorAction& action) {
    std::ostringstream os;
    {
        const Moderator::ModeratorAction* mod_action = &action;
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(mod_action);
    }
    return MessagePacket(MessagePacket::MODERATOR_ACTION, sender, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket ShutdownServerMessage(int sender)
{ return MessagePacket(MessagePacket::SHUT_DOWN_SERVER, sender, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE); }

/** requests previews of savefiles from server synchronously */
MessagePacket RequestSavePreviewsMessage(int sender, std::string directory)
{ return MessagePacket(MessagePacket::REQUEST_SAVE_PREVIEWS, sender, Networking::INVALID_PLAYER_ID, directory); }

/** returns the savegame previews to the client */
MessagePacket DispatchSavePreviewsMessage(int receiver, const PreviewInformation& previews) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(previews);
    }
    return MessagePacket(MessagePacket::DISPATCH_SAVE_PREVIEWS, Networking::INVALID_PLAYER_ID, receiver, os.str(), true);
}

MessagePacket RequestCombatLogsMessage(int sender, const std::vector<int>& ids) {
    std::ostringstream os;
    freeorion_xml_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(ids);
    return MessagePacket(MessagePacket::REQUEST_COMBAT_LOGS, sender, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket DispatchCombatLogsMessage(int receiver, const std::vector<std::pair<int, const CombatLog>>& logs) {
    std::ostringstream os;
    freeorion_xml_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(logs);
    return MessagePacket(MessagePacket::DISPATCH_COMBAT_LOGS, Networking::INVALID_PLAYER_ID, receiver, os.str(), true);
}

////////////////////////////////////////////////
// Multiplayer Lobby MessagePacket named ctors
////////////////////////////////////////////////
MessagePacket LobbyUpdateMessage(int sender, const MultiplayerLobbyData& lobby_data) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return MessagePacket(MessagePacket::LOBBY_UPDATE, sender, Networking::INVALID_PLAYER_ID, os.str());
}

MessagePacket ServerLobbyUpdateMessage(int receiver, const MultiplayerLobbyData& lobby_data) {
    std::ostringstream os;
    {
        freeorion_xml_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return MessagePacket(MessagePacket::LOBBY_UPDATE, Networking::INVALID_PLAYER_ID, receiver, os.str());
}

MessagePacket LobbyChatMessage(int sender, int receiver, const std::string& data)
{ return MessagePacket(MessagePacket::LOBBY_CHAT, sender, receiver, data); }

MessagePacket ServerLobbyChatMessage(int sender, int receiver, const std::string& data)
{ return MessagePacket(MessagePacket::LOBBY_CHAT, sender, receiver, data); }

MessagePacket StartMPGameMessage(int player_id)
{ return MessagePacket(MessagePacket::START_MP_GAME, player_id, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE); }


////////////////////////////////////////////////
// MessagePacket data extractors
////////////////////////////////////////////////
void ExtractErrorMessageData(const MessagePacket& msg, std::string& problem, bool& fatal) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(problem)
           >> BOOST_SERIALIZATION_NVP(fatal);
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractErrorMessageData(const MessagePacket& msg, std::string& problem, bool& fatal) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        problem = UserStringNop("SERVER_MESSAGE_NOT_UNDERSTOOD");
        fatal = false;
    }
}

void ExtractHostMPGameMessageData(const MessagePacket& msg, std::string& host_player_name, std::string& client_version_string) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(host_player_name)
           >> BOOST_SERIALIZATION_NVP(client_version_string);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractHostMPGameMessageData(const MessagePacket& msg, std::string& host_player_name, std::string& client_version_string) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractLobbyUpdateMessageData(const MessagePacket& msg, MultiplayerLobbyData& lobby_data) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(lobby_data);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractLobbyUpdateMessageData(const MessagePacket& msg, MultiplayerLobbyData& lobby_data) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractGameStartMessageData(const MessagePacket& msg, bool& single_player_game, int& empire_id, int& current_turn,
                        EmpireManager& empires, Universe& universe, SpeciesManager& species, CombatLogManager& combat_logs,
                        SupplyManager& supply,
                        std::map<int, PlayerInfo>& players, OrderSet& orders, bool& loaded_game_data, bool& ui_data_available,
                        SaveGameUIData& ui_data, bool& save_state_string_available, std::string& save_state_string,
                        GalaxySetupData& galaxy_setup_data)
{
    try {
        try {
            // first attempt binary deserialziation
            std::istringstream is(msg.Text());

            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(single_player_game)
               >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;

            boost::timer deserialize_timer;
            ia >> BOOST_SERIALIZATION_NVP(empires);
            DebugLogger() << "ExtractGameStartMessage empire deserialization time " << (deserialize_timer.elapsed() * 1000.0);

            ia >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(ia, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);

            deserialize_timer.restart();
            Deserialize(ia, universe);
            DebugLogger() << "ExtractGameStartMessage universe deserialization time " << (deserialize_timer.elapsed() * 1000.0);


            ia >> BOOST_SERIALIZATION_NVP(players)
               >> BOOST_SERIALIZATION_NVP(loaded_game_data);
            if (loaded_game_data) {
                Deserialize(ia, orders);
                ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
                if (ui_data_available)
                    ia >> BOOST_SERIALIZATION_NVP(ui_data);
                ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
                if (save_state_string_available)
                    ia >> BOOST_SERIALIZATION_NVP(save_state_string);
            } else {
                ui_data_available = false;
                save_state_string_available = false;
            }
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);

        } catch (...) {
            // if binary deserialization failed, try more-portable XML deserialization
            std::istringstream is(msg.Text());

            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(single_player_game)
               >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;

            boost::timer deserialize_timer;
            ia >> BOOST_SERIALIZATION_NVP(empires);
            DebugLogger() << "ExtractGameStartMessage empire deserialization time " << (deserialize_timer.elapsed() * 1000.0);

            ia >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(ia, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);

            deserialize_timer.restart();
            Deserialize(ia, universe);
            DebugLogger() << "ExtractGameStartMessage universe deserialization time " << (deserialize_timer.elapsed() * 1000.0);


            ia >> BOOST_SERIALIZATION_NVP(players)
               >> BOOST_SERIALIZATION_NVP(loaded_game_data);
            if (loaded_game_data) {
                Deserialize(ia, orders);
                ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
                if (ui_data_available)
                    ia >> BOOST_SERIALIZATION_NVP(ui_data);
                ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
                if (save_state_string_available)
                    ia >> BOOST_SERIALIZATION_NVP(save_state_string);
            } else {
                ui_data_available = false;
                save_state_string_available = false;
            }
            ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractGameStartMessageData(...) failed!  MessagePacket probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractJoinGameMessageData(const MessagePacket& msg, std::string& player_name, Networking::ClientType& client_type,
                        std::string& version_string)
{
    DebugLogger() << "ExtractJoinGameMessageData() from " << player_name << " client type " << client_type;
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(player_name)
           >> BOOST_SERIALIZATION_NVP(client_type)
           >> BOOST_SERIALIZATION_NVP(version_string);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractJoinGameMessageData(const MessagePacket& msg, std::string& player_name, "
                      << "Networking::ClientType client_type, std::string& version_string) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnOrdersMessageData(const MessagePacket& msg, OrderSet& orders) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        Deserialize(ia, orders);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractTurnOrdersMessageData(const MessagePacket& msg, OrderSet& orders) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnUpdateMessageData(const MessagePacket& msg, int empire_id, int& current_turn, EmpireManager& empires,
                                  Universe& universe, SpeciesManager& species, CombatLogManager& combat_logs,
                                  SupplyManager& supply, std::map<int, PlayerInfo>& players)
{
    try {
        ScopedTimer timer("Turn Update Unpacking", true);

        try {
            // first attempt binary deserialization
            std::istringstream is(msg.Text());
            freeorion_bin_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            ia >> BOOST_SERIALIZATION_NVP(current_turn)
               >> BOOST_SERIALIZATION_NVP(empires)
               >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(ia, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);
            Deserialize(ia, universe);
            ia >> BOOST_SERIALIZATION_NVP(players);

        } catch (...) {
            // try again with more-portable XML deserialization
            std::istringstream is(msg.Text());
            freeorion_xml_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            ia >> BOOST_SERIALIZATION_NVP(current_turn)
               >> BOOST_SERIALIZATION_NVP(empires)
               >> BOOST_SERIALIZATION_NVP(species);
            combat_logs.SerializeIncompleteLogs(ia, 1);
            ia >> BOOST_SERIALIZATION_NVP(supply);
            Deserialize(ia, universe);
            ia >> BOOST_SERIALIZATION_NVP(players);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractTurnUpdateMessageData(...) failed!  MessagePacket probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnPartialUpdateMessageData(const MessagePacket& msg, int empire_id, Universe& universe) {
    try {
        ScopedTimer timer("Mid Turn Update Unpacking", true);

        try {
            // first attempt binary deserialization
            std::istringstream is(msg.Text());
            freeorion_bin_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            Deserialize(ia, universe);

        } catch (...) {
            // try again with more-portable XML deserialization
            std::istringstream is(msg.Text());
            freeorion_xml_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            Deserialize(ia, universe);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtracturnPartialUpdateMessageData(...) failed!  MessagePacket probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractClientSaveDataMessageData(const MessagePacket& msg, OrderSet& orders, bool& ui_data_available,
                                      SaveGameUIData& ui_data, bool& save_state_string_available,
                                      std::string& save_state_string)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        DebugLogger() << "deserializing orders";
        Deserialize(ia, orders);
        DebugLogger() << "checking for ui data";
        ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
        if (ui_data_available) {
            DebugLogger() << "deserializing UI data";
            ia >> BOOST_SERIALIZATION_NVP(ui_data);
        }
        DebugLogger() << "checking for save state string";
        ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
        if (save_state_string_available) {
            DebugLogger() << "deserializing save state string";
            ia >> BOOST_SERIALIZATION_NVP(save_state_string);
        }

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractClientSaveDataMessageData(...) failed!  MessagePacket probably long, so not outputting to log.\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractTurnProgressMessageData(const MessagePacket& msg, MessagePacket::TurnProgressPhase& phase_id) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(phase_id);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractTurnProgressMessageData(const MessagePacket& msg, MessagePacket::TurnProgressPhase& phase_id) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractPlayerStatusMessageData(const MessagePacket& msg, int& about_player_id, MessagePacket::PlayerStatus& status) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(about_player_id)
           >> BOOST_SERIALIZATION_NVP(status);
 
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractPlayerStatusMessageData(const MessagePacket& msg, int& about_player_id, MessagePacket::PlayerStatus&) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractHostSPGameMessageData(const MessagePacket& msg, SinglePlayerSetupData& setup_data, std::string& client_version_string) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(setup_data)
           >> BOOST_SERIALIZATION_NVP(client_version_string);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractHostSPGameMessageData(const MessagePacket& msg, SinglePlayerSetupData& setup_data, std::string& client_version_string) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractEndGameMessageData(const MessagePacket& msg, MessagePacket::EndGameReason& reason, std::string& reason_player_name) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(reason)
           >> BOOST_SERIALIZATION_NVP(reason_player_name);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractEndGameMessageData(const MessagePacket& msg, MessagePacket::EndGameReason& reason, "
                      << "std::string& reason_player_name) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractModeratorActionMessageData(const MessagePacket& msg, Moderator::ModeratorAction*& mod_action) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(mod_action);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractModeratorActionMessageData(const MessagePacket& msg, Moderator::ModeratorAction& mod_act) "
                      << "failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
    }
}

void ExtractDiplomacyMessageData(const MessagePacket& msg, DiplomaticMessage& diplo_message) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(diplo_message);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractDiplomacyMessageData(const MessagePacket& msg, DiplomaticMessage& "
                      << "diplo_message) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractDiplomaticStatusMessageData(const MessagePacket& msg, DiplomaticStatusUpdateInfo& diplo_update) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
           >> BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
           >> BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);

    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractDiplomaticStatusMessageData(const MessagePacket& msg, DiplomaticStatusUpdate& diplo_update) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractRequestSavePreviewsMessageData(const MessagePacket& msg, std::string& directory)
{ directory = msg.Text(); }

void ExtractDispatchSavePreviewsMessageData(const MessagePacket& msg, PreviewInformation& previews) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(previews);

    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractDispatchSavePreviewsMessageData(const MessagePacket& msg, PreviewInformation& previews) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

FO_COMMON_API void ExtractServerSaveGameCompleteMessageData(const MessagePacket& msg, std::string& save_filename, int& bytes_written) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(save_filename)
           >> BOOST_SERIALIZATION_NVP(bytes_written);

    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractServerSaveGameCompleteServerSaveGameCompleteMessageData(const MessagePacket& msg, std::string& save_filename, int& bytes_written) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

FO_COMMON_API void ExtractRequestCombatLogsMessageData(const MessagePacket& msg, std::vector<int>& ids) {
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(ids);
    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractRequestCombatLogMessageData(const MessagePacket& msg, std::vector<int>& ids) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

FO_COMMON_API void ExtractDispatchCombatLogsMessageData(
    const MessagePacket& msg, std::vector<std::pair<int, CombatLog>>& logs)
{
    try {
        std::istringstream is(msg.Text());
        freeorion_xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(logs);
    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractDispatchCombatLogMessageData(const MessagePacket& msg, std::vector<std::pair<int, const CombatLog&>>& logs) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }

}
