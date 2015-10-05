#include "Message.h"

#include "../combat/CombatLogManager.h"
#include "../Empire/EmpireManager.h"
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
#include <boost/lexical_cast.hpp>
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
std::ostream& operator<<(std::ostream& os, const Message& msg) {
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
// Message
////////////////////////////////////////////////
Message::Message() :
    m_type(UNDEFINED),
    m_sending_player(0),
    m_receiving_player(0),
    m_synchronous_response(false),
    m_message_size(0),
    m_message_text()
{}

Message::Message(MessageType type,
                 int sending_player,
                 int receiving_player,
                 const std::string& text,
                 bool synchronous_response/* = false*/) :
    m_type(type),
    m_sending_player(sending_player),
    m_receiving_player(receiving_player),
    m_synchronous_response(synchronous_response),
    m_message_size(text.size()),
    m_message_text(new char[text.size()])
{ std::copy(text.begin(), text.end(), m_message_text.get()); }

Message::MessageType Message::Type() const
{ return m_type; }

int Message::SendingPlayer() const
{ return m_sending_player; }

int Message::ReceivingPlayer() const
{ return m_receiving_player; }

bool Message::SynchronousResponse() const
{ return m_synchronous_response; }

std::size_t Message::Size() const
{ return m_message_size; }

const char* Message::Data() const
{ return m_message_text.get(); }

std::string Message::Text() const
{ return std::string(m_message_text.get(), m_message_size); }

void Message::Resize(std::size_t size) {
    m_message_size = size;
    m_message_text.reset(new char[m_message_size]);
}

char* Message::Data()
{ return m_message_text.get(); }

void Message::Swap(Message& rhs) {
    std::swap(m_type, rhs.m_type);
    std::swap(m_sending_player, rhs.m_sending_player);
    std::swap(m_receiving_player, rhs.m_receiving_player);
    std::swap(m_synchronous_response, rhs.m_synchronous_response);
    std::swap(m_message_size, rhs.m_message_size);
    std::swap(m_message_text, rhs.m_message_text);
}

bool operator==(const Message& lhs, const Message& rhs) {
    return
        lhs.Type() == rhs.Type() &&
        lhs.SendingPlayer() == rhs.SendingPlayer() &&
        lhs.ReceivingPlayer() == rhs.ReceivingPlayer() &&
        lhs.Text() == rhs.Text();
}

bool operator!=(const Message& lhs, const Message& rhs)
{ return !(lhs == rhs); }

void swap(Message& lhs, Message& rhs)
{ lhs.Swap(rhs); }

void BufferToHeader(const int* header_buf, Message& message) {
    message.m_type = static_cast<Message::MessageType>(header_buf[0]);
    message.m_sending_player = header_buf[1];
    message.m_receiving_player = header_buf[2];
    message.m_synchronous_response = (header_buf[3] != 0);
    message.m_message_size = header_buf[4];
}

void HeaderToBuffer(const Message& message, int* header_buf) {
    header_buf[0] = message.Type();
    header_buf[1] = message.SendingPlayer();
    header_buf[2] = message.ReceivingPlayer();
    header_buf[3] = message.SynchronousResponse();
    header_buf[4] = message.Size();
}

////////////////////////////////////////////////
// Message named ctors
////////////////////////////////////////////////
Message ErrorMessage(const std::string& problem, bool fatal/* = true*/) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(problem)
               << BOOST_SERIALIZATION_NVP(fatal);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(problem)
               << BOOST_SERIALIZATION_NVP(fatal);
        }
    }
    return Message(Message::ERROR_MSG, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

Message ErrorMessage(int player_id, const std::string& problem, bool fatal/* = true*/) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(problem)
               << BOOST_SERIALIZATION_NVP(fatal);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(problem)
               << BOOST_SERIALIZATION_NVP(fatal);
        }
    }
    return Message(Message::ERROR_MSG, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message HostSPGameMessage(const SinglePlayerSetupData& setup_data) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(setup_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(setup_data);
        }
    }
    return Message(Message::HOST_SP_GAME, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

Message HostMPGameMessage(const std::string& host_player_name)
{ return Message(Message::HOST_MP_GAME, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, host_player_name); }

Message JoinGameMessage(const std::string& player_name, Networking::ClientType client_type) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(player_name)
               << BOOST_SERIALIZATION_NVP(client_type);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(player_name)
               << BOOST_SERIALIZATION_NVP(client_type);
        }
    }
    return Message(Message::JOIN_GAME, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID, os.str());
}

Message HostIDMessage(int host_player_id) {
    return Message(Message::HOST_ID, Networking::INVALID_PLAYER_ID, Networking::INVALID_PLAYER_ID,
                   boost::lexical_cast<std::string>(host_player_id));
}

Message GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         const CombatLogManager& combat_logs,
                         const std::map<int, PlayerInfo>& players,
                         const GalaxySetupData& galaxy_setup_data)
{
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
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
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
            Serialize(oa, universe);
            bool loaded_game_data = false;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return Message(Message::GAME_START, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         const CombatLogManager& combat_logs,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const SaveGameUIData* ui_data,
                         const GalaxySetupData& galaxy_setup_data)
{
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = (ui_data != 0);
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
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = (ui_data != 0);
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            if (ui_data_available)
                oa << boost::serialization::make_nvp("ui_data", *ui_data);
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return Message(Message::GAME_START, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const SpeciesManager& species,
                         const CombatLogManager& combat_logs,
                         const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const std::string* save_state_string,
                         const GalaxySetupData& galaxy_setup_data)
{
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(single_player_game)
               << BOOST_SERIALIZATION_NVP(empire_id)
               << BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            bool save_state_string_available = (save_state_string != 0);
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
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
            Serialize(oa, universe);
            bool loaded_game_data = true;
            oa << BOOST_SERIALIZATION_NVP(players)
               << BOOST_SERIALIZATION_NVP(loaded_game_data);
            Serialize(oa, orders);
            bool ui_data_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available);
            bool save_state_string_available = (save_state_string != 0);
            oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
            if (save_state_string_available)
                oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
            oa << BOOST_SERIALIZATION_NVP(galaxy_setup_data);
        }
    }
    return Message(Message::GAME_START, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message HostSPAckMessage(int player_id)
{ return Message(Message::HOST_SP_GAME, Networking::INVALID_PLAYER_ID, player_id, ACKNOWLEDGEMENT); }

Message HostMPAckMessage(int player_id)
{ return Message(Message::HOST_MP_GAME, Networking::INVALID_PLAYER_ID, player_id, ACKNOWLEDGEMENT); }

Message JoinAckMessage(int player_id)
{ return Message(Message::JOIN_GAME, Networking::INVALID_PLAYER_ID, player_id, ACKNOWLEDGEMENT); }

Message TurnOrdersMessage(int sender, const OrderSet& orders) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            Serialize(oa, orders);
        } else {
            freeorion_xml_oarchive oa(os);
            Serialize(oa, orders);
        }
    }
    return Message(Message::TURN_ORDERS, sender, Networking::INVALID_PLAYER_ID, os.str());
}

Message TurnProgressMessage(Message::TurnProgressPhase phase_id, int player_id) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(phase_id);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(phase_id);
        }
    }
    return Message(Message::TURN_PROGRESS, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message PlayerStatusMessage(int player_id, int about_player_id, Message::PlayerStatus player_status) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(about_player_id)
               << BOOST_SERIALIZATION_NVP(player_status);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(about_player_id)
               << BOOST_SERIALIZATION_NVP(player_status);
        }
    }
    return Message(Message::PLAYER_STATUS, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message TurnUpdateMessage(int player_id, int empire_id, int current_turn,
                          const EmpireManager& empires, const Universe& universe,
                          const SpeciesManager& species, const CombatLogManager& combat_logs,
                          const std::map<int, PlayerInfo>& players)
{
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn)
               << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        } else {
            freeorion_xml_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            oa << BOOST_SERIALIZATION_NVP(current_turn)
               << BOOST_SERIALIZATION_NVP(empires)
               << BOOST_SERIALIZATION_NVP(species)
               << BOOST_SERIALIZATION_NVP(combat_logs);
            Serialize(oa, universe);
            oa << BOOST_SERIALIZATION_NVP(players);
        }
    }
    return Message(Message::TURN_UPDATE, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message TurnPartialUpdateMessage(int player_id, int empire_id, const Universe& universe) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            Serialize(oa, universe);
        } else {
            freeorion_xml_oarchive oa(os);
            GetUniverse().EncodingEmpire() = empire_id;
            Serialize(oa, universe);
        }
    }
    return Message(Message::TURN_PARTIAL_UPDATE, Networking::INVALID_PLAYER_ID, player_id, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders, const SaveGameUIData& ui_data) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            Serialize(oa, orders);
            bool ui_data_available = true;
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available)
               << BOOST_SERIALIZATION_NVP(ui_data)
               << BOOST_SERIALIZATION_NVP(save_state_string_available);
        } else {
            freeorion_xml_oarchive oa(os);
            Serialize(oa, orders);
            bool ui_data_available = true;
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available)
               << BOOST_SERIALIZATION_NVP(ui_data)
               << BOOST_SERIALIZATION_NVP(save_state_string_available);
        }
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, Networking::INVALID_PLAYER_ID, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders, const std::string& save_state_string) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            Serialize(oa, orders);
            bool ui_data_available = false;
            bool save_state_string_available = true;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available)
               << BOOST_SERIALIZATION_NVP(save_state_string_available)
               << BOOST_SERIALIZATION_NVP(save_state_string);
        } else {
            freeorion_xml_oarchive oa(os);
            Serialize(oa, orders);
            bool ui_data_available = false;
            bool save_state_string_available = true;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available)
               << BOOST_SERIALIZATION_NVP(save_state_string_available)
               << BOOST_SERIALIZATION_NVP(save_state_string);
        }
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, Networking::INVALID_PLAYER_ID, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            Serialize(oa, orders);
            bool ui_data_available = false;
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available)
               << BOOST_SERIALIZATION_NVP(save_state_string_available);
        } else {
            freeorion_xml_oarchive oa(os);
            Serialize(oa, orders);
            bool ui_data_available = false;
            bool save_state_string_available = false;
            oa << BOOST_SERIALIZATION_NVP(ui_data_available)
               << BOOST_SERIALIZATION_NVP(save_state_string_available);
        }
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, Networking::INVALID_PLAYER_ID, os.str());
}

Message RequestNewObjectIDMessage(int sender)
{ return Message(Message::REQUEST_NEW_OBJECT_ID, sender, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE); }

Message DispatchObjectIDMessage(int player_id, int new_id) {
    return Message(Message::DISPATCH_NEW_OBJECT_ID, Networking::INVALID_PLAYER_ID, player_id,
                   boost::lexical_cast<std::string>(new_id), true);
}

Message RequestNewDesignIDMessage(int sender)
{ return Message(Message::REQUEST_NEW_DESIGN_ID, sender, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE, true); }

Message DispatchDesignIDMessage(int player_id, int new_id) {
    return Message(Message::DISPATCH_NEW_DESIGN_ID, Networking::INVALID_PLAYER_ID, player_id,
                   boost::lexical_cast<std::string>(new_id), true);
}

Message HostSaveGameMessage(int sender, const std::string& filename)
{ return Message(Message::SAVE_GAME, sender, Networking::INVALID_PLAYER_ID, filename); }

Message ServerSaveGameMessage(int receiver, bool synchronous_response)
{ return Message(Message::SAVE_GAME, Networking::INVALID_PLAYER_ID, receiver, DUMMY_EMPTY_MESSAGE, synchronous_response); }

Message GlobalChatMessage(int sender, const std::string& msg)
{ return Message(Message::PLAYER_CHAT, sender, Networking::INVALID_PLAYER_ID, msg); }

Message SingleRecipientChatMessage(int sender, int receiver, const std::string& msg)
{ return Message(Message::PLAYER_CHAT, sender, receiver, msg); }

Message DiplomacyMessage(int sender, int receiver, const DiplomaticMessage& diplo_message) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(diplo_message);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(diplo_message);
        }
    }
    return Message(Message::DIPLOMACY, sender, receiver, os.str());
}

Message DiplomaticStatusMessage(int receiver, const DiplomaticStatusUpdateInfo& diplo_update) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
               << BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
               << BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
               << BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
               << BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);
        }
    }
    return Message(Message::DIPLOMATIC_STATUS, Networking::INVALID_PLAYER_ID, receiver, os.str());
}

Message EndGameMessage(int receiver, Message::EndGameReason reason,
                       const std::string& reason_player_name/* = ""*/)
{
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(reason)
               << BOOST_SERIALIZATION_NVP(reason_player_name);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(reason)
               << BOOST_SERIALIZATION_NVP(reason_player_name);
        }
    }
    return Message(Message::END_GAME, Networking::INVALID_PLAYER_ID, receiver, os.str());
}

Message ModeratorActionMessage(int sender, const Moderator::ModeratorAction& action) {
    std::ostringstream os;
    {
        const Moderator::ModeratorAction* mod_action = &action;
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(mod_action);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(mod_action);
        }
    }
    return Message(Message::MODERATOR_ACTION, sender, Networking::INVALID_PLAYER_ID, os.str());
}

Message ShutdownServerMessage(int sender)
{ return Message(Message::SHUT_DOWN_SERVER, sender, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE); }

/** requests previews of savefiles from server synchronously */
Message RequestSavePreviewsMessage(int sender, std::string directory)
{ return Message(Message::REQUEST_SAVE_PREVIEWS, sender, Networking::INVALID_PLAYER_ID, directory); }

/** returns the savegame previews to the client */
Message DispatchSavePreviewsMessage(int receiver, const PreviewInformation& previews) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(previews);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(previews);
        }
    }
    return Message(Message::DISPATCH_SAVE_PREVIEWS, Networking::INVALID_PLAYER_ID, receiver, os.str(), true);
}

////////////////////////////////////////////////
// Multiplayer Lobby Message named ctors
////////////////////////////////////////////////
Message LobbyUpdateMessage(int sender, const MultiplayerLobbyData& lobby_data) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(lobby_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(lobby_data);
        }
    }
    return Message(Message::LOBBY_UPDATE, sender, Networking::INVALID_PLAYER_ID, os.str());
}

Message ServerLobbyUpdateMessage(int receiver, const MultiplayerLobbyData& lobby_data) {
    std::ostringstream os;
    {
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(lobby_data);
        } else {
            freeorion_xml_oarchive oa(os);
            oa << BOOST_SERIALIZATION_NVP(lobby_data);
        }
    }
    return Message(Message::LOBBY_UPDATE, Networking::INVALID_PLAYER_ID, receiver, os.str());
}

Message LobbyChatMessage(int sender, int receiver, const std::string& data)
{ return Message(Message::LOBBY_CHAT, sender, receiver, data); }

Message ServerLobbyChatMessage(int sender, int receiver, const std::string& data)
{ return Message(Message::LOBBY_CHAT, sender, receiver, data); }

Message StartMPGameMessage(int player_id)
{ return Message(Message::START_MP_GAME, player_id, Networking::INVALID_PLAYER_ID, DUMMY_EMPTY_MESSAGE); }


////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////
void ExtractMessageData(const Message& msg, std::string& problem, bool& fatal) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(problem)
               >> BOOST_SERIALIZATION_NVP(fatal);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(problem)
               >> BOOST_SERIALIZATION_NVP(fatal);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, std::string& problem, bool& fatal) failed!  "
                      << "Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, MultiplayerLobbyData& lobby_data) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(lobby_data);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(lobby_data);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, MultiplayerLobbyData& "
                      << "lobby_data) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, bool& single_player_game, int& empire_id,
                        int& current_turn, EmpireManager& empires, Universe& universe,
                        SpeciesManager& species, CombatLogManager& combat_logs,
                        std::map<int, PlayerInfo>& players, OrderSet& orders,
                        bool& loaded_game_data, bool& ui_data_available,
                        SaveGameUIData& ui_data, bool& save_state_string_available,
                        std::string& save_state_string, GalaxySetupData& galaxy_setup_data)
{
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(single_player_game)
               >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;

            boost::timer deserialize_timer;
            ia >> BOOST_SERIALIZATION_NVP(empires);
            DebugLogger() << "ExtractMessage empire deserialization time " << (deserialize_timer.elapsed() * 1000.0);

            ia >> BOOST_SERIALIZATION_NVP(species)
            >> BOOST_SERIALIZATION_NVP(combat_logs);

            deserialize_timer.restart();
            Deserialize(ia, universe);
            DebugLogger() << "ExtractMessage universe deserialization time " << (deserialize_timer.elapsed() * 1000.0);


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
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(single_player_game)
               >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(current_turn);
            GetUniverse().EncodingEmpire() = empire_id;

            boost::timer deserialize_timer;
            ia >> BOOST_SERIALIZATION_NVP(empires);
            DebugLogger() << "ExtractMessage empire deserialization time " << (deserialize_timer.elapsed() * 1000.0);

            ia >> BOOST_SERIALIZATION_NVP(species)
               >> BOOST_SERIALIZATION_NVP(combat_logs);

            deserialize_timer.restart();
            Deserialize(ia, universe);
            DebugLogger() << "ExtractMessage universe deserialization time " << (deserialize_timer.elapsed() * 1000.0);


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
        ErrorLogger() << "ExtractMessageData(const Message& msg, bool& single_player_game, int& empire_id, "
                      << "int& current_turn, EmpireManager& empires, Universe& universe, "
                      << "std::map<int, PlayerInfo>& players, OrderSet& orders, "
                      << "bool& loaded_game_data, bool& ui_data_available, "
                      << "SaveGameUIData& ui_data, bool& save_state_string_available, "
                      << "std::string& save_state_string) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, std::string& player_name, Networking::ClientType& client_type) {
    DebugLogger() << "ExtractMessageData() from " << player_name << " client type " << client_type;
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(player_name)
               >> BOOST_SERIALIZATION_NVP(client_type);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(player_name)
               >> BOOST_SERIALIZATION_NVP(client_type);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, std::string& player_name, "
                      << "Networking::ClientType client_type) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, OrderSet& orders) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            Deserialize(ia, orders);
        } else {
            freeorion_xml_iarchive ia(is);
            Deserialize(ia, orders);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, OrderSet& orders) failed!  "
                      << "Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, int empire_id, int& current_turn,
                        EmpireManager& empires, Universe& universe,
                        SpeciesManager& species, CombatLogManager& combat_logs,
                        std::map<int, PlayerInfo>& players)
{
    try {
        ScopedTimer timer("Turn Update Unpacking", true);
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            ia >> BOOST_SERIALIZATION_NVP(current_turn)
               >> BOOST_SERIALIZATION_NVP(empires)
               >> BOOST_SERIALIZATION_NVP(species)
               >> BOOST_SERIALIZATION_NVP(combat_logs);
            Deserialize(ia, universe);
            ia >> BOOST_SERIALIZATION_NVP(players);
        } else {
            freeorion_xml_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            ia >> BOOST_SERIALIZATION_NVP(current_turn)
               >> BOOST_SERIALIZATION_NVP(empires)
               >> BOOST_SERIALIZATION_NVP(species)
               >> BOOST_SERIALIZATION_NVP(combat_logs);
            Deserialize(ia, universe);
            ia >> BOOST_SERIALIZATION_NVP(players);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, int empire_id, int& "
                      << "current_turn, EmpireManager& empires, Universe& universe, "
                      << "std::map<int, PlayerInfo>& players) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, int empire_id, Universe& universe) {
    try {
        ScopedTimer timer("Mid Turn Update Unpacking", true);
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            Deserialize(ia, universe);
        } else {
            freeorion_xml_iarchive ia(is);
            GetUniverse().EncodingEmpire() = empire_id;
            Deserialize(ia, universe);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, int empire_id, "
                      << "Universe& universe) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, OrderSet& orders, bool& ui_data_available,
                        SaveGameUIData& ui_data, bool& save_state_string_available,
                        std::string& save_state_string)
{
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
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
        } else {
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
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, OrderSet& orders, "
                      << "bool& ui_data_available, SaveGameUIData& ui_data, "
                      << "bool& save_state_string_available, std::string& save_state_string) "
                      << "failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, Message::TurnProgressPhase& phase_id) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(phase_id);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(phase_id);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, Message::TurnProgressPhase& "
                      << "phase_id) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, int& about_player_id, Message::PlayerStatus& status) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(about_player_id)
               >> BOOST_SERIALIZATION_NVP(status);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(about_player_id)
               >> BOOST_SERIALIZATION_NVP(status);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, int& about_player_id "
                      << "Message::PlayerStatus&) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, SinglePlayerSetupData& setup_data) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(setup_data);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(setup_data);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, SinglePlayerSetupData& "
                      << "setup_data) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, Message::EndGameReason& reason,
                        std::string& reason_player_name)
{
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(reason)
               >> BOOST_SERIALIZATION_NVP(reason_player_name);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(reason)
               >> BOOST_SERIALIZATION_NVP(reason_player_name);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, Message::EndGameReason& reason, "
                      << "std::string& reason_player_name) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, Moderator::ModeratorAction*& mod_action) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(mod_action);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(mod_action);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, Moderator::ModeratorAction& mod_act) "
                      << "failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
    }
}

void ExtractMessageData(const Message& msg, int& empire_id, std::string& empire_name) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(empire_name);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(empire_id)
               >> BOOST_SERIALIZATION_NVP(empire_name);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, int empire_id, std::string& "
                      << "empire_name) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, DiplomaticMessage& diplo_message) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(diplo_message);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(diplo_message);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, DiplomaticMessage& "
                      << "diplo_message) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, DiplomaticStatusUpdateInfo& diplo_update) {
    try {
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
               >> BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
               >> BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(diplo_update.empire1_id)
               >> BOOST_SERIALIZATION_NVP(diplo_update.empire2_id)
               >> BOOST_SERIALIZATION_NVP(diplo_update.diplo_status);
        }
    } catch (const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, DiplomaticStatusUpdate& "
                      << "diplo_update) failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}

void ExtractMessageData(const Message& msg, std::string& directory)
{ directory = msg.Text(); }

void ExtractMessageData(const Message& msg, PreviewInformation& previews) {
    try{
        std::istringstream is(msg.Text());
        if (GetOptionsDB().Get<bool>("binary-serialization")) {
            freeorion_bin_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(previews);
        } else {
            freeorion_xml_iarchive ia(is);
            ia >> BOOST_SERIALIZATION_NVP(previews);
        }
    } catch(const std::exception& err) {
        ErrorLogger() << "ExtractMessageData(const Message& msg, PreviewInformation& previews"
                      << ") failed!  Message:\n"
                      << msg.Text() << "\n"
                      << "Error: " << err.what();
        throw err;
    }
}
