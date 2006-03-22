#include "Message.h"

#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <zlib.h>

#include <stdexcept>
#include <sstream>

////////////////////////////////////////////////
// Free Functions
////////////////////////////////////////////////
void ZipString(std::string str, std::string& zipped_str)
{
    zipped_str.resize(static_cast<int>(str.size() * 1.01 + 13)); // space required by zlib
    uLongf zipped_size = zipped_str.size();
    int zip_result = compress(reinterpret_cast<Bytef*>(const_cast<char*>(zipped_str.c_str())), &zipped_size, 
                              reinterpret_cast<const Bytef*>(str.c_str()), str.size());
    if (zip_result != Z_OK)
        throw std::runtime_error("ZipString : call to zlib's compress() failed with code " + boost::lexical_cast<std::string>(zip_result));
   
    zipped_str.resize(zipped_size);
}

void UnzipString(std::string str, std::string& unzipped_str, int size)
{
    unzipped_str.resize(size);
    uLongf unzipped_size = unzipped_str.size();
    int zip_result = uncompress(reinterpret_cast<Bytef*>(const_cast<char*>(unzipped_str.c_str())), &unzipped_size, 
                                reinterpret_cast<const Bytef*>(str.c_str()), str.size());
    if (zip_result != Z_OK)
        throw std::runtime_error("ZipString : call to zlib's uncompress() failed with code " + boost::lexical_cast<std::string>(zip_result));
   
    unzipped_str.resize(unzipped_size);
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
// Message-Creation Free Functions
////////////////////////////////////////////////
Message HostGameMessage(int player_id, const XMLDoc& game_parameters)
{
    XMLDoc doc(game_parameters);
    return Message(Message::HOST_GAME, player_id, -1, Message::CORE, doc);
}

Message HostGameMessage(int player_id, const std::string& host_player_name)
{
    XMLDoc doc;
    doc.root_node.AppendChild(XMLElement("host_player_name", host_player_name));
    return Message(Message::HOST_GAME, player_id, -1, Message::CORE, doc);
}

Message JoinGameMessage(const std::string& player_name)
{
    XMLDoc doc;
    doc.root_node.AppendChild(XMLElement("player_name", player_name));
    return Message(Message::JOIN_GAME, -1, -1, Message::CORE, doc);
}

Message JoinGameSetup(const XMLDoc& player_setup)
{
    XMLDoc doc(player_setup);
    return Message(Message::JOIN_GAME, -1, -1, Message::CORE, doc);
}

Message VersionConflictMessage(int player_id, const XMLDoc& conflict_details)
{
    return Message(Message::SERVER_STATUS, -1, player_id, Message::CORE, conflict_details);
}

Message GameStartMessage(int player_id, const XMLDoc& start_data)
{
    return Message(Message::GAME_START, -1, player_id, Message::CORE, start_data);
}

Message HostAckMessage(int player_id)
{
    return Message(Message::HOST_GAME, -1, player_id, Message::CORE, "ACK");
}

Message JoinAckMessage(int player_id)
{
    return Message(Message::JOIN_GAME, -1, player_id, Message::CORE, boost::lexical_cast<std::string>(player_id));
}

Message RenameMessage(int player_id, const std::string& new_name)
{
    XMLDoc doc;
    doc.root_node.AppendChild(XMLElement("new_name", new_name));
    return Message(Message::SERVER_STATUS, -1, player_id, Message::CORE, doc);
}

Message EndGameMessage(int sender, int receiver)
{
    return Message(Message::END_GAME, sender, receiver, Message::CORE, "");
}

Message VictoryMessage(int receiver)
{
    return Message(Message::END_GAME, -1, receiver, Message::CORE, "VICTORY");
}

Message TurnOrdersMessage(int sender, int receiver, const XMLDoc& orders_data)
{
    return Message(Message::TURN_ORDERS, sender, receiver, Message::CORE, orders_data);
}

Message TurnProgressMessage(int player_id, Message::TurnProgressPhase phase_id, int empire_id)
{
    /// Turn progres message sends down message ID instead of text for faster transfer
    /// The data is a number indicating the phase being started and the empire ID
    XMLDoc doc;

    XMLElement phase_id_elt("phase_id");
    phase_id_elt.SetAttribute("value", boost::lexical_cast<std::string>(phase_id));
    doc.root_node.AppendChild( phase_id_elt );

    XMLElement empire_id_elt("empire_id");
    empire_id_elt.SetAttribute("value", boost::lexical_cast<std::string>(empire_id));
    doc.root_node.AppendChild( empire_id_elt );
    return Message(Message::TURN_PROGRESS, -1, player_id, Message::CORE, doc);
}

Message TurnUpdateMessage(int player_id, const XMLDoc& start_data)
{
    return Message(Message::TURN_UPDATE, -1, player_id, Message::CORE, start_data);
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

Message ServerLoadGameMessage(int receiver, const XMLDoc& data)
{
    return Message(Message::LOAD_GAME, -1, receiver, Message::CORE, data);
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
// Multiplayer Lobby Messages
////////////////////////////////////////////////

Message LobbyUpdateMessage(int sender, const XMLDoc& doc)
{
    return Message(Message::LOBBY_UPDATE, sender, -1, Message::CORE, doc);
}

Message ServerLobbyUpdateMessage(int receiver, const XMLDoc& doc)
{
    return Message(Message::LOBBY_UPDATE, -1, receiver, Message::CLIENT_LOBBY_MODULE, doc);
}

Message LobbyChatMessage(int sender, int receiver, const std::string& text)
{
    XMLDoc doc;
    doc.root_node.AppendChild(XMLElement("receiver", boost::lexical_cast<std::string>(receiver)));
    doc.root_node.AppendChild(XMLElement("text", text));
    return Message(Message::LOBBY_UPDATE, sender, -1, Message::CORE, doc);
}

Message ServerLobbyChatMessage(int sender, int receiver, const std::string& text)
{
    XMLDoc doc;
    doc.root_node.AppendChild(XMLElement("sender", boost::lexical_cast<std::string>(sender)));
    doc.root_node.AppendChild(XMLElement("text", text));
    return Message(Message::LOBBY_UPDATE, -1, receiver, Message::CLIENT_LOBBY_MODULE, doc);
}
