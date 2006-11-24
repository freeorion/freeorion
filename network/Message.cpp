#include "Message.h"

#include "../util/MultiplayerCommon.h"
#include "../universe/Universe.h"
#include "../util/OptionsDB.h"
#include "../util/Serialize.h"
#include "../util/XMLDoc.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>

#include <zlib.h>

#include <stdexcept>
#include <sstream>


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
void ZipString(const std::string& str, std::string& zipped_str)
{
    zipped_str.resize(static_cast<int>(str.size() * 1.01 + 13)); // space required by zlib
    uLongf zipped_size = zipped_str.size();
    int zip_result = compress(reinterpret_cast<Bytef*>(const_cast<char*>(zipped_str.c_str())), &zipped_size, 
                              reinterpret_cast<const Bytef*>(str.c_str()), str.size());
    if (zip_result != Z_OK)
        throw std::runtime_error("ZipString : call to zlib's compress() failed with code " + boost::lexical_cast<std::string>(zip_result));
   
    zipped_str.resize(zipped_size);
}

void UnzipString(const std::string& str, std::string& unzipped_str, int size)
{
    unzipped_str.resize(size);
    uLongf unzipped_size = unzipped_str.size();
    int zip_result = uncompress(reinterpret_cast<Bytef*>(const_cast<char*>(unzipped_str.c_str())), &unzipped_size, 
                                reinterpret_cast<const Bytef*>(str.c_str()), str.size());
    if (zip_result != Z_OK)
        throw std::runtime_error("ZipString : call to zlib's uncompress() failed with code " + boost::lexical_cast<std::string>(zip_result));
   
    unzipped_str.resize(unzipped_size);
}

namespace GG {
    GG_ENUM_MAP_BEGIN(Message::MessageType)
    GG_ENUM_MAP_INSERT(Message::UNDEFINED)
    GG_ENUM_MAP_INSERT(Message::DEBUG)
    GG_ENUM_MAP_INSERT(Message::SERVER_STATUS)
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
    GG_ENUM_MAP_INSERT(Message::COMBAT_ROUND_UPDATE)
    GG_ENUM_MAP_INSERT(Message::COMBAT_END)
    GG_ENUM_MAP_INSERT(Message::HUMAN_PLAYER_MSG)
    GG_ENUM_MAP_INSERT(Message::PLAYER_ELIMINATED)
    GG_ENUM_MAP_INSERT(Message::PLAYER_EXIT)
    GG_ENUM_MAP_INSERT(Message::REQUEST_NEW_OBJECT_ID)
    GG_ENUM_MAP_INSERT(Message::DISPATCH_NEW_OBJECT_ID)
    GG_ENUM_MAP_INSERT(Message::END_GAME)
    GG_ENUM_MAP_END
}

std::string MessageTypeStr(Message::MessageType type)
{
    return StripMessageScoping(GG::GetEnumMap<Message::MessageType>().FromEnum(type));
}

namespace GG {
    GG_ENUM_MAP_BEGIN(Message::ModuleType)
    GG_ENUM_MAP_INSERT(Message::CORE)
    GG_ENUM_MAP_INSERT(Message::CLIENT_LOBBY_MODULE)
    GG_ENUM_MAP_INSERT(Message::CLIENT_COMBAT_MODULE)
    GG_ENUM_MAP_INSERT(Message::CLIENT_SYNCHRONOUS_RESPONSE)
    GG_ENUM_MAP_END
}

std::string ModuleTypeStr(Message::ModuleType type)
{
    return StripMessageScoping(GG::GetEnumMap<Message::ModuleType>().FromEnum(type));
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
       << msg.Sender();

    if (msg.Sender() == -1)
        os << "(server/unknown) --> ";
    else if (msg.Sender() == 0)
        os << "(host) --> ";
    else
        os << " --> ";

    os << msg.Receiver();

    if (msg.Receiver() == -1)
        os << "(server/unknown).";
    else if (msg.Receiver() == 0)
        os << "(host).";
    else
        os << ".";

    os << ModuleTypeStr(msg.Module()) << " "
       << "\"" << msg.GetText() << "\"\n";

    return os;
}

////////////////////////////////////////////////
// Message
////////////////////////////////////////////////
Message::Message() : 
    m_message_type(UNDEFINED),
    m_sending_player(-2),
    m_receiving_player(-2),
    m_message_text(new std::string),
    m_response_msg(UNDEFINED)
{
    m_compressed = false;
}

Message::Message(MessageType msg_type, int sender, int receiver, ModuleType module, const std::string& text, MessageType response_msg) : 
    m_message_type(msg_type),
    m_sending_player(sender),
    m_receiving_player(receiver),
    m_receiving_module(module),
    m_message_text(new std::string),
    m_response_msg( response_msg )
{
    ZipString(text, *m_message_text);
    m_compressed = true;
    m_uncompressed_size = text.size();
}

Message::Message(MessageType msg_type, int sender, int receiver, ModuleType module, const XMLDoc& doc,  MessageType response_msg) : 
    m_message_type(msg_type),
    m_sending_player(sender),
    m_receiving_player(receiver),
    m_receiving_module(module),
    m_message_text(new std::string),
    m_response_msg( response_msg )
{
    std::stringstream stream;
    doc.WriteDoc(stream);
    ZipString(stream.str(), *m_message_text);
    m_compressed = true;
    m_uncompressed_size = stream.str().size();
}

// private ctor
Message::Message(const std::string& raw_msg) : 
    m_message_text(new std::string),
    m_response_msg( UNDEFINED )
{
    std::stringstream stream(raw_msg);
   
    int temp;
    stream >> temp;
    m_message_type = MessageType(temp);
    stream >> m_sending_player >> m_receiving_player >> temp;
    m_receiving_module = ModuleType(temp);
    stream >> m_compressed >> m_uncompressed_size;
   
    *m_message_text = raw_msg.substr(static_cast<int>(stream.tellg()) + 1);
}

Message::MessageType Message::Type() const
{
    return m_message_type;
}

Message::MessageType Message::Response() const
{
    return m_response_msg;
}

int Message::Sender() const
{
    return m_sending_player;
}

int Message::Receiver() const
{
    return m_receiving_player;
}

Message::ModuleType Message::Module() const
{
    return m_receiving_module;
}

std::string Message::GetText() const
{
    if (m_compressed) {
        std::string retval;
        DecompressMessage(retval);
        return retval;
    } else {
        return *m_message_text;
    }
}

std::string Message::HeaderString() const
{
    // compose header info (everything but the message text)
    std::stringstream stream;
    stream << m_message_type << " " << m_sending_player << " " << m_receiving_player << " " << 
        m_receiving_module << " " << m_compressed << " " << m_uncompressed_size << " ";
    return stream.str();
}

void Message::ValidateMessage()
{
}

void Message::CompressMessage(std::string& compressed_msg) const
{
    if (m_compressed)
        compressed_msg = *m_message_text;
    else
        ZipString(*m_message_text, compressed_msg);
}

void Message::DecompressMessage(std::string& uncompressed_msg) const
{
    if (m_compressed)
        UnzipString(*m_message_text, uncompressed_msg, m_uncompressed_size);
    else
        uncompressed_msg = *m_message_text;
}


////////////////////////////////////////////////
// Message named ctors
////////////////////////////////////////////////

Message HostSPGameMessage(int player_id, const SinglePlayerSetupData& setup_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(setup_data);
    }
    return Message(Message::HOST_SP_GAME, player_id, -1, Message::CORE, os.str());
}

Message HostMPGameMessage(int player_id, const std::string& host_player_name)
{
    return Message(Message::HOST_MP_GAME, player_id, -1, Message::CORE, host_player_name);
}

Message JoinGameMessage(const std::string& player_name)
{
    return Message(Message::JOIN_GAME, -1, -1, Message::CORE, player_name);
}

Message GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn, const EmpireManager& empires, const Universe& universe)
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
    }
    return Message(Message::GAME_START, -1, player_id, Message::CORE, os.str());
}

Message HostSPAckMessage(int player_id)
{
    return Message(Message::HOST_SP_GAME, -1, player_id, Message::CORE, "ACK");
}

Message HostMPAckMessage(int player_id)
{
    return Message(Message::HOST_MP_GAME, -1, player_id, Message::CORE, "ACK");
}

Message JoinAckMessage(int player_id)
{
    return Message(Message::JOIN_GAME, -1, player_id, Message::CORE, boost::lexical_cast<std::string>(player_id));
}

Message RenameMessage(int player_id, const std::string& new_name)
{
    return Message(Message::RENAME_PLAYER, -1, player_id, Message::CORE, new_name);
}

Message EndGameMessage(int sender, int receiver)
{
    return Message(Message::END_GAME, sender, receiver, Message::CORE, "");
}

Message VictoryMessage(int receiver)
{
    return Message(Message::END_GAME, -1, receiver, Message::CORE, "VICTORY");
}

Message TurnOrdersMessage(int sender, const OrderSet& orders)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
    }
    return Message(Message::TURN_ORDERS, sender, -1, Message::CORE, os.str());
}

Message TurnProgressMessage(int player_id, Message::TurnProgressPhase phase_id, int empire_id)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(phase_id)
           << BOOST_SERIALIZATION_NVP(empire_id);
    }
    return Message(Message::TURN_PROGRESS, -1, player_id, Message::CORE, os.str());
}

Message TurnUpdateMessage(int player_id, int empire_id, int current_turn, const EmpireManager& empires, const Universe& universe)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Universe::s_encoding_empire = empire_id;
        oa << BOOST_SERIALIZATION_NVP(current_turn);
        Serialize(oa, empires);
        Serialize(oa, universe);
    }
    return Message(Message::TURN_UPDATE, -1, player_id, Message::CORE, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders, const SaveGameUIData& ui_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
        bool ui_data_available = true;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available)
           << BOOST_SERIALIZATION_NVP(ui_data);
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, -1, Message::CORE, os.str());
}

Message ClientSaveDataMessage(int sender, const OrderSet& orders)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
        bool ui_data_available = false;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available);
    }
    return Message(Message::CLIENT_SAVE_DATA, sender, -1, Message::CORE, os.str());
}

Message RequestNewObjectIDMessage(int sender)
{
    return Message(Message::REQUEST_NEW_OBJECT_ID, sender, -1, Message::CORE, "", Message::DISPATCH_NEW_OBJECT_ID);
}

Message DispatchObjectIDMessage(int player_id, int new_id)
{
    return Message(Message::DISPATCH_NEW_OBJECT_ID, -1, player_id, Message::CLIENT_SYNCHRONOUS_RESPONSE, boost::lexical_cast<std::string>(new_id));
}

Message HostSaveGameMessage(int sender, const std::string& filename)
{
    return Message(Message::SAVE_GAME, sender, -1, Message::CORE, filename, Message::SAVE_GAME);
}

Message HostLoadGameMessage(int sender, const std::string& filename)
{
    return Message(Message::LOAD_GAME, sender, -1, Message::CORE, filename);
}

Message ServerSaveGameMessage(int receiver, bool done/* = false*/)
{
    return Message(Message::SAVE_GAME, -1, receiver, done ? Message::CLIENT_SYNCHRONOUS_RESPONSE : Message::CORE, "");
}

Message ServerLoadGameMessage(int receiver, const OrderSet& orders, const boost::shared_ptr<SaveGameUIData>& ui_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        Serialize(oa, orders);
        bool ui_data_available = ui_data;
        oa << BOOST_SERIALIZATION_NVP(ui_data_available);
        if (ui_data_available)
            oa << boost::serialization::make_nvp("ui_data", *ui_data);
    }
    return Message(Message::LOAD_GAME, -1, receiver, Message::CORE, os.str());
}

Message ChatMessage(int sender, const std::string& msg)
{
    return Message(Message::HUMAN_PLAYER_MSG, sender, -1, Message::CORE, msg);
}

Message ChatMessage(int sender, int receiver, const std::string& msg)
{
    return Message(Message::HUMAN_PLAYER_MSG, sender, receiver, Message::CORE, msg);
}

Message PlayerDisconnectedMessage(int receiver, const std::string& player_name)
{
    return Message(Message::PLAYER_EXIT, -1, receiver, Message::CORE, player_name);
}

Message PlayerEliminatedMessage(int receiver, const std::string& empire_name)
{
    return Message(Message::PLAYER_ELIMINATED, -1, receiver, Message::CORE, empire_name);
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
    return Message(Message::LOBBY_UPDATE, sender, -1, Message::CORE, os.str());
}

Message ServerLobbyUpdateMessage(int receiver, const MultiplayerLobbyData& lobby_data)
{
    std::ostringstream os;
    {
        FREEORION_OARCHIVE_TYPE oa(os);
        oa << BOOST_SERIALIZATION_NVP(lobby_data);
    }
    return Message(Message::LOBBY_UPDATE, -1, receiver, Message::CLIENT_LOBBY_MODULE, os.str());
}

Message LobbyChatMessage(int sender, int receiver, const std::string& data)
{
    return Message(Message::LOBBY_CHAT, sender, receiver, Message::CORE, data);
}

Message ServerLobbyChatMessage(int sender, int receiver, const std::string& data)
{
    return Message(Message::LOBBY_CHAT, sender, receiver, Message::CLIENT_LOBBY_MODULE, data);
}

Message LobbyHostAbortMessage(int sender)
{
    return Message(Message::LOBBY_HOST_ABORT, sender, -1, Message::CORE, "");
}

Message ServerLobbyHostAbortMessage(int receiver)
{
    return Message(Message::LOBBY_HOST_ABORT, -1, receiver, Message::CLIENT_LOBBY_MODULE, "");
}

Message LobbyExitMessage(int sender)
{
    return Message(Message::LOBBY_EXIT, sender, -1, Message::CORE, "");
}

Message ServerLobbyExitMessage(int sender, int receiver)
{
    return Message(Message::LOBBY_EXIT, sender, receiver, Message::CLIENT_LOBBY_MODULE, "");
}

Message StartMPGameMessage(int player_id)
{
    return Message(Message::START_MP_GAME, player_id, -1, Message::CORE, "");
}


////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////

void ExtractMessageData(const Message& msg, MultiplayerLobbyData& lobby_data)
{
    std::istringstream is(msg.GetText());
    FREEORION_IARCHIVE_TYPE ia(is);
    ia >> BOOST_SERIALIZATION_NVP(lobby_data);
}

void ExtractMessageData(const Message& msg, bool& single_player_game, int& empire_id, int& current_turn, EmpireManager& empires, Universe& universe)
{
    std::istringstream is(msg.GetText());
    FREEORION_IARCHIVE_TYPE ia(is);
    ia >> BOOST_SERIALIZATION_NVP(single_player_game)
       >> BOOST_SERIALIZATION_NVP(empire_id)
       >> BOOST_SERIALIZATION_NVP(current_turn);
    Universe::s_encoding_empire = empire_id;
    Deserialize(ia, empires);
    Deserialize(ia, universe);
}

void ExtractMessageData(const Message& msg, OrderSet& orders)
{
    std::istringstream is(msg.GetText());
    FREEORION_IARCHIVE_TYPE ia(is);
    Deserialize(ia, orders);
}

void ExtractMessageData(const Message& msg, int empire_id, int& current_turn, EmpireManager& empires, Universe& universe)
{
    std::istringstream is(msg.GetText());
    FREEORION_IARCHIVE_TYPE ia(is);
    Universe::s_encoding_empire = empire_id;
    ia >> BOOST_SERIALIZATION_NVP(current_turn);
    Deserialize(ia, empires);
    Deserialize(ia, universe);
}

bool ExtractMessageData(const Message& msg, OrderSet& orders, SaveGameUIData& ui_data)
{
    std::istringstream is(msg.GetText());
    FREEORION_IARCHIVE_TYPE ia(is);
    bool ui_data_available;
    Deserialize(ia, orders);
    ia >> BOOST_SERIALIZATION_NVP(ui_data_available);
    if (ui_data_available)
        ia >> BOOST_SERIALIZATION_NVP(ui_data);
    return ui_data_available;
}

void ExtractMessageData(const Message& msg, Message::TurnProgressPhase& phase_id, int& empire_id)
{
    std::istringstream is(msg.GetText());
    FREEORION_IARCHIVE_TYPE ia(is);
    ia >> BOOST_SERIALIZATION_NVP(phase_id)
       >> BOOST_SERIALIZATION_NVP(empire_id);
}

void ExtractMessageData(const Message& msg, SinglePlayerSetupData& setup_data)
{
    std::istringstream is(msg.GetText());
    FREEORION_IARCHIVE_TYPE ia(is);
    ia >> BOOST_SERIALIZATION_NVP(setup_data);
}
