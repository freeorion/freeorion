#include "Message.h"

#include "../combat/CombatOrder.h"
#include "../combat/OpenSteer/CombatObject.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Meter.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../util/OptionsDB.h"
#include "../util/Serialize.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>

#include <zlib.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>


namespace {
    const std::string MESSAGE_SCOPE_PREFIX = "Message::";
    std::string StripMessageScoping(const std::string& str)
    {
        return boost::algorithm::starts_with(str, MESSAGE_SCOPE_PREFIX) ?
            boost::algorithm::erase_first_copy(str, MESSAGE_SCOPE_PREFIX) :
            str;
    }
}

////////////////////////////////////////////////
// Free Functions
////////////////////////////////////////////////
namespace GG {
    GG_ENUM_MAP_BEGIN(Message::MessageType)
    GG_ENUM_MAP_INSERT(Message::UNDEFINED)
    GG_ENUM_MAP_INSERT(Message::DEBUG)
    GG_ENUM_MAP_INSERT(Message::HOST_SP_GAME)
    GG_ENUM_MAP_INSERT(Message::HOST_MP_GAME)
    GG_ENUM_MAP_INSERT(Message::JOIN_GAME)
    GG_ENUM_MAP_INSERT(Message::LOBBY_UPDATE)
    GG_ENUM_MAP_INSERT(Message::LOBBY_CHAT)
    GG_ENUM_MAP_INSERT(Message::LOBBY_HOST_ABORT)
    GG_ENUM_MAP_INSERT(Message::LOBBY_EXIT)
    GG_ENUM_MAP_INSERT(Message::START_MP_GAME)
    GG_ENUM_MAP_INSERT(Message::SAVE_GAME)
    GG_ENUM_MAP_INSERT(Message::LOAD_GAME)
    GG_ENUM_MAP_INSERT(Message::GAME_START)
    GG_ENUM_MAP_INSERT(Message::TURN_UPDATE)
    GG_ENUM_MAP_INSERT(Message::TURN_ORDERS)
    GG_ENUM_MAP_INSERT(Message::TURN_PROGRESS)
    GG_ENUM_MAP_INSERT(Message::CLIENT_SAVE_DATA)
    GG_ENUM_MAP_INSERT(Message::COMBAT_START)
    GG_ENUM_MAP_INSERT(Message::COMBAT_TURN_UPDATE)
    GG_ENUM_MAP_INSERT(Message::COMBAT_TURN_ORDERS)
    GG_ENUM_MAP_INSERT(Message::COMBAT_END)
    GG_ENUM_MAP_INSERT(Message::HUMAN_PLAYER_CHAT)
    GG_ENUM_MAP_INSERT(Message::PLAYER_ELIMINATED)
    GG_ENUM_MAP_INSERT(Message::REQUEST_NEW_OBJECT_ID)
    GG_ENUM_MAP_INSERT(Message::DISPATCH_NEW_OBJECT_ID)
    GG_ENUM_MAP_INSERT(Message::REQUEST_NEW_DESIGN_ID)
    GG_ENUM_MAP_INSERT(Message::DISPATCH_NEW_DESIGN_ID)
    GG_ENUM_MAP_INSERT(Message::END_GAME)
    GG_ENUM_MAP_END
}

std::string MessageTypeStr(Message::MessageType type)
{
    return StripMessageScoping(GG::GetEnumMap<Message::MessageType>().FromEnum(type));
}

namespace GG {
    GG_ENUM_MAP_BEGIN(Message::TurnProgressPhase)
    GG_ENUM_MAP_INSERT(Message::FLEET_MOVEMENT)
    GG_ENUM_MAP_INSERT(Message::COMBAT)
    GG_ENUM_MAP_INSERT(Message::EMPIRE_PRODUCTION)
    GG_ENUM_MAP_INSERT(Message::WAITING_FOR_PLAYERS)
    GG_ENUM_MAP_INSERT(Message::PROCESSING_ORDERS)
    GG_ENUM_MAP_INSERT(Message::DOWNLOADING)
    GG_ENUM_MAP_END
}

std::string TurnProgressPhaseStr(Message::TurnProgressPhase phase)
{
    return StripMessageScoping(GG::GetEnumMap<Message::TurnProgressPhase>().FromEnum(phase));
}

std::ostream& operator<<(std::ostream& os, const Message& msg)
{
    os << "Message: "
       << MessageTypeStr(msg.Type()) << " "
       << msg.SendingPlayer();

    if (msg.SendingPlayer() == -1)
        os << "(server/unknown) --> ";
    else if (msg.SendingPlayer() == 0)
        os << "(host) --> ";
    else
        os << " --> ";

    os << msg.ReceivingPlayer();

    if (msg.ReceivingPlayer() == -1)
        os << "(server/unknown)";
    else if (msg.ReceivingPlayer() == 0)
        os << "(host)";

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

void Message::Resize(std::size_t size)
{
    m_message_size = size;
    m_message_text.reset(new char[m_message_size]);
}

char* Message::Data()
{ return m_message_text.get(); }

void Message::Swap(Message& rhs)
{
    std::swap(m_type, rhs.m_type);
    std::swap(m_sending_player, rhs.m_sending_player);
    std::swap(m_receiving_player, rhs.m_receiving_player);
    std::swap(m_synchronous_response, rhs.m_synchronous_response);
    std::swap(m_message_size, rhs.m_message_size);
    std::swap(m_message_text, rhs.m_message_text);
}

bool operator==(const Message& lhs, const Message& rhs)
{
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

void BufferToHeader(const int* header_buf, Message& message)
{
    message.m_type = static_cast<Message::MessageType>(header_buf[0]);
    message.m_sending_player = header_buf[1];
    message.m_receiving_player = header_buf[2];
    message.m_synchronous_response = header_buf[3];
    message.m_message_size = header_buf[4];
}

void HeaderToBuffer(const Message& message, int* header_buf)
{
    header_buf[0] = message.Type();
    header_buf[1] = message.SendingPlayer();
    header_buf[2] = message.ReceivingPlayer();
    header_buf[3] = message.SynchronousResponse();
    header_buf[4] = message.Size();
}


////////////////////////////////////////////////
// Message named ctors
////////////////////////////////////////////////

Message HostSPGameMessage(const SinglePlayerSetupData& setup_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(setup_data);
    }
    return Message(Message::HOST_SP_GAME, -1, -1, os.str());
}

Message HostMPGameMessage(const std::string& host_player_name)
{
    return Message(Message::HOST_MP_GAME, -1, -1, host_player_name);
}

Message JoinGameMessage(const std::string& player_name)
{
    return Message(Message::JOIN_GAME, -1, -1, player_name);
}

Message GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const std::map<int, PlayerInfo>& players)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(single_player_game)
           << BOOST_SERIALIZATION_NVP(empire_id)
           << BOOST_SERIALIZATION_NVP(current_turn);
        Universe::s_encoding_empire = empire_id;
        Serialize(oa, empires);
        Serialize(oa, universe);
        bool loaded_game_data = false;
        oa << BOOST_SERIALIZATION_NVP(players)
           << BOOST_SERIALIZATION_NVP(loaded_game_data);
    }
    return Message(Message::GAME_START, -1, player_id, os.str());
}

Message GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const SaveGameUIData* ui_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(single_player_game)
           << BOOST_SERIALIZATION_NVP(empire_id)
           << BOOST_SERIALIZATION_NVP(current_turn);
        Universe::s_encoding_empire = empire_id;
        Serialize(oa, empires);
        Serialize(oa, universe);
        bool loaded_game_data = true;
        oa << BOOST_SERIALIZATION_NVP(players)
           << BOOST_SERIALIZATION_NVP(loaded_game_data);
        Serialize(oa, orders);
        bool ui_data_available = ui_data;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available);
        if (ui_data_available)
            oa << boost::serialization::make_nvp("ui_data", *ui_data);
        bool save_state_string_available = false;
        oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
    }
    return Message(Message::GAME_START, -1, player_id, os.str());
}

Message GameStartMessage(int player_id, bool single_player_game, int empire_id,
                         int current_turn, const EmpireManager& empires,
                         const Universe& universe, const std::map<int, PlayerInfo>& players,
                         const OrderSet& orders, const std::string* save_state_string)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(single_player_game)
           << BOOST_SERIALIZATION_NVP(empire_id)
           << BOOST_SERIALIZATION_NVP(current_turn);
        Universe::s_encoding_empire = empire_id;
        Serialize(oa, empires);
        Serialize(oa, universe);
        bool loaded_game_data = true;
        oa << BOOST_SERIALIZATION_NVP(players)
           << BOOST_SERIALIZATION_NVP(loaded_game_data);
        Serialize(oa, orders);
        bool ui_data_available = false;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available);
        bool save_state_string_available = save_state_string;
        oa << BOOST_SERIALIZATION_NVP(save_state_string_available);
        if (save_state_string_available)
            oa << boost::serialization::make_nvp("save_state_string", *save_state_string);
    }
    return Message(Message::GAME_START, -1, player_id, os.str());
}

Message HostSPAckMessage(int player_id)
{
    return Message(Message::HOST_SP_GAME, -1, player_id, "ACK");
}

Message HostMPAckMessage(int player_id)
{
    return Message(Message::HOST_MP_GAME, -1, player_id, "ACK");
}

Message JoinAckMessage(int player_id)
{
    return Message(Message::JOIN_GAME, -1, player_id, "ACK");
}

Message TurnOrdersMessage(int sender, const OrderSet& orders)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
    }
    return Message(Message::TURN_ORDERS, sender, -1, os.str());
}

Message TurnProgressMessage(int player_id, Message::TurnProgressPhase phase_id, int empire_id)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(phase_id)
           << BOOST_SERIALIZATION_NVP(empire_id);
    }
    return Message(Message::TURN_PROGRESS, -1, player_id, os.str());
}

Message TurnUpdateMessage(int player_id, int empire_id, int current_turn,
                          const EmpireManager& empires, const Universe& universe,
                          const std::map<int, PlayerInfo>& players)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Universe::s_encoding_empire = empire_id;
        oa << BOOST_SERIALIZATION_NVP(current_turn);
        Serialize(oa, empires);
        Serialize(oa, universe);
        oa << BOOST_SERIALIZATION_NVP(players);
    }
    return Message(Message::TURN_UPDATE, -1, player_id, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders,
                              const SaveGameUIData& ui_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
        bool ui_data_available = true;
        bool save_state_string_available = false;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(ui_data)
           << BOOST_SERIALIZATION_NVP(save_state_string_available);
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, -1, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders,
                              const std::string& save_state_string)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
        bool ui_data_available = false;
        bool save_state_string_available = true;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(save_state_string_available)
           << BOOST_SERIALIZATION_NVP(save_state_string);
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, -1, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
        bool ui_data_available = false;
        bool save_state_string_available = false;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(save_state_string_available);
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, -1, os.str());
}

Message RequestNewObjectIDMessage(int sender)
{
    return Message(Message::REQUEST_NEW_OBJECT_ID, sender, -1, "");
}

Message DispatchObjectIDMessage(int player_id, int new_id)
{
    return Message(Message::DISPATCH_NEW_OBJECT_ID, -1, player_id,
                   boost::lexical_cast<std::string>(new_id), true);
}

Message RequestNewDesignIDMessage(int sender)
{
    return Message(Message::REQUEST_NEW_DESIGN_ID, sender, -1, "", true);
}

Message DispatchDesignIDMessage(int player_id, int new_id)
{
    return Message(Message::DISPATCH_NEW_DESIGN_ID, -1, player_id,
                   boost::lexical_cast<std::string>(new_id), true);
}

Message HostSaveGameMessage(int sender, const std::string& filename)
{
    return Message(Message::SAVE_GAME, sender, -1, filename);
}

Message ServerSaveGameMessage(int receiver, bool synchronous_response)
{
    return Message(Message::SAVE_GAME, -1, receiver, "", synchronous_response);
}

Message GlobalChatMessage(int sender, const std::string& msg)
{
    return Message(Message::HUMAN_PLAYER_CHAT, sender, -1, msg);
}

Message SingleRecipientChatMessage(int sender, int receiver, const std::string& msg)
{
    return Message(Message::HUMAN_PLAYER_CHAT, sender, receiver, msg);
}

Message VictoryDefeatMessage(int receiver, Message::VictoryOrDefeat victory_or_defeat,
                             const std::string& reason_string, int empire_id)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(victory_or_defeat)
           << BOOST_SERIALIZATION_NVP(reason_string)
           << BOOST_SERIALIZATION_NVP(empire_id);
    }
    return Message(Message::VICTORY_DEFEAT, -1, receiver, os.str());
}

Message PlayerEliminatedMessage(int receiver, int empire_id, const std::string& empire_name)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(empire_id)
           << BOOST_SERIALIZATION_NVP(empire_name);
    }
    return Message(Message::PLAYER_ELIMINATED, -1, receiver, os.str());
}

Message EndGameMessage(int receiver, Message::EndGameReason reason,
                       const std::string& reason_player_name/* = ""*/)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(reason)
           << BOOST_SERIALIZATION_NVP(reason_player_name);
    }
    return Message(Message::END_GAME, -1, receiver, os.str());
}



////////////////////////////////////////////////
// Multiplayer Lobby Message named ctors
////////////////////////////////////////////////

Message LobbyUpdateMessage(int sender, const MultiplayerLobbyData& lobby_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return Message(Message::LOBBY_UPDATE, sender, -1, os.str());
}

Message ServerLobbyUpdateMessage(int receiver, const MultiplayerLobbyData& lobby_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return Message(Message::LOBBY_UPDATE, -1, receiver, os.str());
}

Message LobbyChatMessage(int sender, int receiver, const std::string& data)
{
    return Message(Message::LOBBY_CHAT, sender, receiver, data);
}

Message ServerLobbyChatMessage(int sender, int receiver, const std::string& data)
{
    return Message(Message::LOBBY_CHAT, sender, receiver, data);
}

Message LobbyHostAbortMessage(int sender)
{
    return Message(Message::LOBBY_HOST_ABORT, sender, -1, "");
}

Message ServerLobbyHostAbortMessage(int receiver)
{
    return Message(Message::LOBBY_HOST_ABORT, -1, receiver, "");
}

Message LobbyExitMessage(int sender)
{
    return Message(Message::LOBBY_EXIT, sender, -1, "");
}

Message ServerLobbyExitMessage(int sender, int receiver)
{
    return Message(Message::LOBBY_EXIT, sender, receiver, "");
}

Message StartMPGameMessage(int player_id)
{
    return Message(Message::START_MP_GAME, player_id, -1, "");
}

Message ServerCombatStartMessage(int receiver, int empire_id, const System* system,
                                 const std::map<int, UniverseObject*>& combat_universe)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Universe::s_encoding_empire = empire_id;
        oa << BOOST_SERIALIZATION_NVP(system);
        Serialize(oa, combat_universe);
    }
    return Message(Message::COMBAT_START, -1, receiver, os.str());
}

Message ServerCombatUpdateMessage(int receiver, int empire_id, const CombatData& combat_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Universe::s_encoding_empire = empire_id;
        oa << BOOST_SERIALIZATION_NVP(combat_data);
    }
    return Message(Message::COMBAT_TURN_UPDATE, -1, receiver, os.str());
}

Message ServerCombatEndMessage(int receiver)
{ return Message(Message::COMBAT_END, -1, receiver, ""); }

Message CombatTurnOrdersMessage(int sender, const CombatOrderSet& combat_orders)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(combat_orders);
    }
    return Message(Message::COMBAT_TURN_ORDERS, sender, -1, os.str());
}

////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////

void ExtractMessageData(const Message& msg, MultiplayerLobbyData& lobby_data)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(lobby_data);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, MultiplayerLobbyData& "
                  << "lobby_data) failed!  Message:\n" << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, bool& single_player_game, int& empire_id,
                        int& current_turn, EmpireManager& empires, Universe& universe,
                        std::map<int, PlayerInfo>& players, OrderSet& orders,
                        bool& loaded_game_data, bool& ui_data_available,
                        SaveGameUIData& ui_data, bool& save_state_string_available,
                        std::string& save_state_string)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(single_player_game)
           >> BOOST_SERIALIZATION_NVP(empire_id)
           >> BOOST_SERIALIZATION_NVP(current_turn);
        Universe::s_encoding_empire = empire_id;
        Deserialize(ia, empires);
        Deserialize(ia, universe);
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
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, bool& single_player_game, "
                  << "int& empire_id, int& current_turn, EmpireManager& empires, "
                  << "Universe& universe, std::map<int, PlayerInfo>& players, OrderSet& "
                  << "orders, SaveGameUIData& ui_data, bool& loaded_game_data, "
                  << "bool& ui_data_available) failed!  Message:\n"
                  << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, OrderSet& orders)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        Deserialize(ia, orders);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, OrderSet& orders) failed!  "
                  << "Message:\n" << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, int empire_id, int& current_turn,
                        EmpireManager& empires, Universe& universe,
                        std::map<int, PlayerInfo>& players)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        Universe::s_encoding_empire = empire_id;
        ia >> BOOST_SERIALIZATION_NVP(current_turn);
        Deserialize(ia, empires);
        Deserialize(ia, universe);
        ia >> BOOST_SERIALIZATION_NVP(players);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, int empire_id, int& "
                  << "current_turn, EmpireManager& empires, Universe& universe, "
                  << "std::map<int, PlayerInfo>& players) failed!  Message:\n"
                  << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, OrderSet& orders, bool& ui_data_available,
                        SaveGameUIData& ui_data, bool& save_state_string_available,
                        std::string& save_state_string)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        Deserialize(ia, orders);
        ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
        if (ui_data_available)
            ia >> BOOST_SERIALIZATION_NVP(ui_data);
        ia >> BOOST_SERIALIZATION_NVP(save_state_string_available);
        if (save_state_string_available)
            ia >> BOOST_SERIALIZATION_NVP(save_state_string);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, OrderSet& orders, "
                  << "SaveGameUIData& ui_data) failed!  Message:\n"
                  << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, Message::TurnProgressPhase& phase_id,
                        int& empire_id)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(phase_id)
           >> BOOST_SERIALIZATION_NVP(empire_id);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, Message::TurnProgressPhase& "
                  << "phase_id, int& empire_id) failed!  Message:\n"
                  << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, SinglePlayerSetupData& setup_data)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(setup_data);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, SinglePlayerSetupData& "
                  << "setup_data) failed!  Message:\n" << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, Message::EndGameReason& reason,
                        std::string& reason_player_name)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(reason)
           >> BOOST_SERIALIZATION_NVP(reason_player_name);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, Message::EndGameReason& reason, "
                  << "std::string& reason_player_name) failed!  Message:\n"
                  << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, int& empire_id, std::string& empire_name)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(empire_id)
           >> BOOST_SERIALIZATION_NVP(empire_name);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, int empire_id, std::string& "
                  << "empire_name) failed!  Message:\n" << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, Message::VictoryOrDefeat& victory_or_defeat,
                        std::string& reason_string, int& empire_id)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(victory_or_defeat)
           >> BOOST_SERIALIZATION_NVP(reason_string)
           >> BOOST_SERIALIZATION_NVP(empire_id);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, Message::VictoryOrDefeat "
                  << "victory_or_defeat, std::string& reason_string, int& empire_id) failed!  "
                  << "Message:\n" << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, CombatData& combat_data)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(combat_data);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, CombatData& "
                  << "combat_data) failed!  Message:\n" << msg.Text() << std::endl;
        throw;
    }
}

void ExtractMessageData(const Message& msg, System*& system,
                        std::map<int, UniverseObject*>& combat_universe)
{
    try {
        std::istringstream is(msg.Text());
        FREEORION_IARCHIVE_TYPE ia(is);
        ia >> BOOST_SERIALIZATION_NVP(system);
        Deserialize(ia, combat_universe);
    } catch (const boost::archive::archive_exception &e) {
        std::cerr << "ExtractMessageData(const Message& msg, System*& "
                  << "system) failed!  Message:\n" << msg.Text() << std::endl;
        throw;
    }
}
