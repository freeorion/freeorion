#include "ClientNetworkCore.h"

#include "Message.h"
#include "../client/ClientApp.h"
#include "../SDL_net2/net2.h"

#include <log4cpp/Category.hh>

#include <sstream>

namespace {
log4cpp::Category& logger = log4cpp::Category::getRoot();
}

ClientNetworkCore::ClientNetworkCore() : 
   m_server_socket(-1)
{
}

ClientNetworkCore::~ClientNetworkCore()
{
   if (m_server_socket != -1) {
      NET2_TCPClose(m_server_socket);
      logger.debug("ClientNetworkCore::HandleNetEvent : Connection to server terminated.");
   }
}

void ClientNetworkCore::SendMessage(const Message& msg) const
{
   if (m_server_socket != -1) {
      SendMessage(msg, m_server_socket, "ClientNetworkCore");
   } else {
      logger.error("ClientNetworkCore::SendMessage : Attempted to send a message to the server when not yet connected.");
   }
}   
   
bool ClientNetworkCore::ConnectToLocalhostServer()
{
   return ConnectToInternetServer("localhost");
}

bool ClientNetworkCore::ConnectToLANServer()
{
// TODO find a server on this LAN's subnet
   
   // connect to server
/*
   if (Connected()) {
      logger.debugStream() << "ClientNetworkCore::ConnectToInternetServer : Connected "
         "to server \"" << server << "\"";
   } else {
      logger.debugStream() << "ClientNetworkCore::ConnectToInternetServer : Call to NET2_TCPConnectTo() "
         "failed with server= \"" << server << "\"";
   }
*/
}

bool ClientNetworkCore::ConnectToInternetServer(const std::string& server)
{
   m_server_socket = NET2_TCPConnectTo(const_cast<char*>(server.c_str()), NetworkCore::CONNECT_PORT);
   if (Connected()) {
      logger.debugStream() << "ClientNetworkCore::ConnectToInternetServer : Connected "
         "to server \"" << server << "\"";
   } else {
	   const char* err_msg = NET2_GetError();
      logger.errorStream() << "ClientNetworkCore::ConnectToInternetServer : Call to NET2_TCPConnectTo() "
         "failed with server= \"" << server << "\"; SDL_net2 error: \"" << (err_msg ? err_msg : "[unknown]") << "\"";
   }
   return Connected();
}

bool ClientNetworkCore::DisconnectFromServer()
{
   bool retval = Connected();
   if (retval)
      NET2_TCPClose(m_server_socket);
   return retval;
}

void ClientNetworkCore::HandleNetEvent(SDL_Event& event)
{
   if (event.type == SDL_USEREVENT) {
      switch(NET2_GetEventType(&event)) {
      case NET2_ERROREVENT: {
         if (const char* err_msg = NET2_GetEventError(&event))
            logger.errorStream() << "ClientNetworkCore::HandleNetEvent : Network error \"" << err_msg << "\"";
         else 
            logger.error("ClientNetworkCore::HandleNetEvent : Unknown network error.");
         break;
      }

      case NET2_TCPACCEPTEVENT: {
         logger.error("ClientNetworkCore::HandleNetEvent : Somehow, the client just accepted a connection!");
         break;
		}

      case NET2_TCPRECEIVEEVENT: {
         ReceiveData(NET2_GetSocket(&event), m_receive_stream, "ClientNetworkCore");
         break;
      }

      case NET2_TCPCLOSEEVENT: {
         if (!Connected())
            logger.error("ClientNetworkCore::HandleNetEvent : Somehow, a connection is closing and yet Connected() == false.");
         int closing_socket = NET2_GetSocket(&event);
         if (closing_socket == m_server_socket) { // connection to server
            logger.debug("ClientNetworkCore::HandleNetEvent : Connection to server terminated.");
            m_server_socket = -1;
         } else { // unknown connection
            IPaddress* addr = NET2_TCPGetPeerAddress(closing_socket);
            const char* socket_hostname = SDLNet_ResolveIP(addr);
            logger.errorStream() << "ClientNetworkCore::HandleNetEvent : Connection to " <<
               (socket_hostname ? "host " + std::string(socket_hostname) : "[unknown host]") << " on socket " << 
               closing_socket << " terminated";
         }
         NET2_TCPClose(closing_socket);
         break;
      }
         
      default: {
         logger.error("ClientNetworkCore::HandleNetEvent : Recieved an SDL_USEREVENT that was not an SDL_net2 network event.");
         break;
		}
      }
   } else {
      logger.error("ClientNetworkCore::HandleNetEvent : Recieved an SDL event that was not an SDL_USEREVENT.");
   }
}
   
void ClientNetworkCore::DispatchMessage(const Message& msg, int socket)
{
  switch (msg.Module()) {
   case Message::CORE:
      ClientApp::HandleMessage(msg);
      break;
   case Message::CLIENT_UNIVERSE_MODULE:
//ClientApp::ClientUniverse().HandleMessage(msg);
      break;
   case Message::CLIENT_EMPIRE_MODULE:
//ClientApp::ClientEmpire().HandleMessage(msg);
      break;
   case Message::CLIENT_COMBAT_MODULE:
      if (ClientApp::CurrentCombat()) {
//ClientApp::CombatModule().HandleMessage(msg);
      } else {
         logger.errorStream()<< "ClientNetworkCore::DispatchMessage : Attempted to pass message to Combat module when "
            "there is no current combat.";
      }
      break;
   default:
      logger.errorStream()<< "ClientNetworkCore::DispatchMessage : Unknown module value \"" << 
         msg.Module() << "\" encountered.";
      break;
   }
}

