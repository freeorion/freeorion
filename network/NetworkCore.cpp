#include "NetworkCore.h"

#include "Message.h"

#include <boost/lexical_cast.hpp>
#include <log4cpp/Category.hh>

// static(s)
const std::string NetworkCore::EOM_STR = "_MSG_END_";
const int         NetworkCore::SERVER_FIND_LISTEN_PORT = 12345;
const int         NetworkCore::SERVER_FIND_RESPONSE_PORT = 12346;
const int         NetworkCore::CONNECT_PORT = 12347;
const int         NetworkCore::HOST_PLAYER_ID = 0;
const std::string NetworkCore::SERVER_FIND_QUERY_MSG = "HELLO";
const std::string NetworkCore::SERVER_FIND_YES_MSG = "NOPE.";
const std::string NetworkCore::SERVER_FIND_NO_MSG = "YEAH.";

namespace {
log4cpp::Category& logger = log4cpp::Category::getRoot();
}

void NetworkCore::SendMessage(const Message& msg, int socket, const std::string& app_name) const
{
   if (socket < 0) {
      logger.fatalStream() << app_name << "::SendMessage : Attempted to send a Message object to invalid socket " << socket << ".";
      throw std::invalid_argument("Attempted to send a Message object to an invalid socket.");
   }

   std::string header_str;
   header_str = msg.HeaderString();

   // do the send in three pieces, to avoid copying the bytes of the message (as we would if we combined the pieces)

   // send header info
   if (NET2_TCPSend(socket, const_cast<char*>(header_str.c_str()), header_str.size()) == -1) {
      const char* err_msg = NET2_GetError();
      logger.errorStream() << app_name << "::SendMessage : NET2_TCPSend call failed with "
         "socket= " << socket << " buf= \"" << header_str.c_str() << "\" len= " << 
         header_str.size() << " " << (err_msg ? err_msg : "");
   }
   
   // send message text
   if (NET2_TCPSend(socket, const_cast<char*>(msg.m_message_text->c_str()), msg.m_message_text->size()) == -1) {
      const char* err_msg = NET2_GetError();
      logger.errorStream() << app_name << "::SendMessage : NET2_TCPSend call failed with "
         "socket= " << socket << " buf= \"" << msg.m_message_text->c_str() << "\" len= " << 
         msg.m_message_text->size() << " " << (err_msg ? err_msg : "");
   }

   // send end-of-message marker
   if (NET2_TCPSend(socket, const_cast<char*>(NetworkCore::EOM_STR.c_str()), NetworkCore::EOM_STR.size()) == -1) {
      const char* err_msg = NET2_GetError();
      logger.errorStream() << app_name << "::SendMessage : NET2_TCPSend call failed with "
         "socket= " << socket << " buf= \"" << NetworkCore::EOM_STR.c_str() << "\" len= " << 
         NetworkCore::EOM_STR.size() << " " << (err_msg ? err_msg : "");
   }
}

void NetworkCore::ReceiveData(int socket, std::string& stream, const std::string& app_name)
{
   static char buf[2048];
   
   if (socket < 0) {
      logger.fatalStream() << app_name << "::ReceiveData : Attempted to read data from invalid socket " << socket << ".";
      throw std::invalid_argument("Attempted to read data from an invalid socket.");
   }

   // grab *all* pending data, as required by SDL_net2
   int recv_len = 0;
   while (0 != (recv_len = NET2_TCPRead(socket, buf, sizeof(buf)))) {
      for (int i = 0; i < recv_len; ++i)
         stream += buf[i];
   }

   // construct Message(s) from data, and dispatch them as appropriate
   unsigned int next_EOM = std::string::npos;
   while ((next_EOM = stream.find(NetworkCore::EOM_STR)) != std::string::npos) {
       Message msg(stream.substr(0, next_EOM));
       stream = stream.substr(next_EOM + NetworkCore::EOM_STR.size());
       DispatchMessage(msg, socket);
   }
}

std::string ToString(const IPaddress& addr)
{
   std::string retval;
   retval += boost::lexical_cast<std::string>(addr.host << 24 >> 24) + '.';
   retval += boost::lexical_cast<std::string>(addr.host << 16 >> 24) + '.';
   retval += boost::lexical_cast<std::string>(addr.host << 8  >> 24) + '.';
   retval += boost::lexical_cast<std::string>(addr.host << 0  >> 24);
   return retval;
}

