#include "Message.h"

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
   if (zip_result != Z_OK) {
      std::stringstream stream;
      stream << "ZipString : call to zlib's compress() failed with code " << zip_result;
      throw std::runtime_error(stream.str());
   }
   
   zipped_str.resize(zipped_size);
}

void UnzipString(std::string str, std::string& unzipped_str, int size)
{
   unzipped_str.resize(size);
   uLongf unzipped_size = unzipped_str.size();
   int zip_result = uncompress(reinterpret_cast<Bytef*>(const_cast<char*>(unzipped_str.c_str())), &unzipped_size, 
                               reinterpret_cast<const Bytef*>(str.c_str()), str.size());
   if (zip_result != Z_OK) {
      std::stringstream stream;
      stream << "UnzipString : call to zlib's uncompress() failed with code " << zip_result;
      throw std::runtime_error(stream.str());
   }
   
   unzipped_str.resize(unzipped_size);
}


////////////////////////////////////////////////
// Message
////////////////////////////////////////////////
Message::Message(MessageType msg_type, int sender, int receiver, ModuleType module, const std::string& text) : 
   m_message_type(msg_type),
   m_sending_player(sender),
   m_receiving_player(receiver),
   m_receiving_module(module),
   m_message_text(new std::string)
{
   ZipString(text, *m_message_text);
   m_compressed = true;
   m_uncompressed_size = text.size();
}

Message::Message(MessageType msg_type, int sender, int receiver, ModuleType module, const GG::XMLDoc& doc) : 
   m_message_type(msg_type),
   m_sending_player(sender),
   m_receiving_player(receiver),
   m_receiving_module(module),
   m_message_text(new std::string)
{
   std::stringstream stream;
   doc.WriteDoc(stream);
   ZipString(stream.str(), *m_message_text);
   m_compressed = true;
   m_uncompressed_size = stream.str().size();
}

// private ctor
Message::Message(const std::string& raw_msg) : 
   m_message_text(new std::string)
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
Message HostGameMessage(const GG::XMLDoc& game_parameters)
{
   return Message(Message::HOST_GAME, -1, -1, Message::CORE, game_parameters);
}

Message JoinGameMessage(const std::string& player_name)
{
   return Message(Message::JOIN_GAME, -1, -1, Message::CORE, player_name);
}

Message EmpireSetupMessage(const GG::XMLDoc& empire_setup)
{
   return Message(Message::EMPIRE_SETUP, -1, -1, Message::CORE, empire_setup);
}

Message HostAckMessage(int player_id)
{
   return Message(Message::HOST_GAME, -1, player_id, Message::CORE, "ACK");
}

Message JoinAckMessage(int player_id)
{
   return Message(Message::JOIN_GAME, -1, player_id, Message::CORE, boost::lexical_cast<std::string>(player_id));
}

Message EmpireSetupMessage(int sender, const GG::XMLDoc& empire_setup)
{
   return Message(Message::EMPIRE_SETUP, sender, -1, Message::CORE, empire_setup);
}

Message EndGameMessage(int sender, int receiver)
{
   return Message(Message::END_GAME, sender, receiver, Message::CORE, "");
}

