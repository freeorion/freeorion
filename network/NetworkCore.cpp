#include "NetworkCore.h"

#include "Message.h"
#include "../SDL_net2/net2.h"

#include <log4cpp/Category.hh>

const std::string NetworkCore::EOM_STR = "_MSG_END_";
const int         NetworkCore::FIND_PORT = 12345;
const int         NetworkCore::CONNECT_PORT = 12346;
const int         NetworkCore::FIND_SERVER_PACKET_SIZE = 256;

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
   int curr_posn = 0;
   int next_EOM = std::string::npos;
   while ((next_EOM = stream.find(NetworkCore::EOM_STR, curr_posn)) != std::string::npos) {
      DispatchMessage(Message(stream.substr(curr_posn, next_EOM - curr_posn)), socket);
      curr_posn = next_EOM + NetworkCore::EOM_STR.size();
   }
   if (curr_posn)
      stream = stream.substr(curr_posn);
}

