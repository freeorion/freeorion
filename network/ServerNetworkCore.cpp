#include "ServerNetworkCore.h"

#include "Message.h"
#include "../server/ServerApp.h"

#include <log4cpp/Category.hh>
#include <boost/lexical_cast.hpp>

#include <sstream>

////////////////////////////////////////////////
// ServerNetworkCore::ConnectionInfo
////////////////////////////////////////////////
ServerNetworkCore::ConnectionInfo::ConnectionInfo() : 
   socket(-1)
{
}

ServerNetworkCore::ConnectionInfo::ConnectionInfo(int sock, const IPaddress& addr) : 
   socket(sock),
   address(addr)
{
}


////////////////////////////////////////////////
// ServerNetworkCore
////////////////////////////////////////////////
ServerNetworkCore::ServerNetworkCore() :
   m_TCP_socket(-1), 
   m_UDP_socket(-1)
{
}

ServerNetworkCore::~ServerNetworkCore()
{
   ClosePorts();
}

void ServerNetworkCore::SendMessage(const Message& msg) const
{
   int receiver_id = msg.Receiver();
   int receiver_socket = -1;
   std::map<int, ConnectionInfo>::const_iterator it = m_player_connections.find(receiver_id);
   if (it != m_player_connections.end())
      receiver_socket = it->second.socket;

   if (receiver_socket != -1) {
      SendMessage(msg, receiver_socket, "ServerNetworkCore");
   } else {
      ServerApp::GetApp()->Logger().error("ServerNetworkCore::SendMessage : Attempted to send a message to unknown or unconnected receiver.");
   }
}   
   
void ServerNetworkCore::ListenToPorts()
{
   ClosePorts();

   if ((m_TCP_socket = NET2_TCPAcceptOn(NetworkCore::CONNECT_PORT)) == -1) {
      char* err_msg = NET2_GetError();
      ServerApp::GetApp()->Logger().fatalStream() << "ServerNetworkCore::ServerNetworkCore : failed to open port " << 
         NetworkCore::CONNECT_PORT << " for TCP connections.  SDL_net2 error: " << (err_msg ? err_msg : "[unknown error]");
      ServerApp::GetApp()->Exit(1);
   } else if ((m_UDP_socket = NET2_UDPAcceptOn(NetworkCore::FIND_PORT, NetworkCore::FIND_SERVER_PACKET_SIZE)) == -1) {
      char* err_msg = NET2_GetError();
      ServerApp::GetApp()->Logger().fatalStream() << "ServerNetworkCore::ServerNetworkCore : failed to open port " << 
         NetworkCore::FIND_PORT << " for UDP packets.  SDL_net2 error: " << (err_msg ? err_msg : "[unknown error]");
      NET2_TCPClose(m_TCP_socket);
      ServerApp::GetApp()->Exit(1);
   } else {
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore : Listening for TCP connections on port " << 
         NetworkCore::CONNECT_PORT;
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore : Listening for UDP packets on port " << 
         NetworkCore::FIND_PORT;
   }
}

bool ServerNetworkCore::DumpConnection(int socket)
{
   bool retval = false;
   int player_id = -1;
   for (std::map<int, ConnectionInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
      if (it->second.socket == socket) {
         player_id = it->first;
         m_player_connections.erase(it);
         retval = true;
         break;
      }
   }
   for (unsigned int i = 0; i < m_new_connections.size(); ++i) {
      if (m_new_connections[i].socket == socket) {
         m_new_connections.erase(m_new_connections.begin() + i);
         retval = true;
         break;
      }
   }

   if (retval) {
      IPaddress* addr = NET2_TCPGetPeerAddress(socket);
      char* socket_hostname = SDLNet_ResolveIP(addr);
      NET2_TCPClose(socket);
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::DumpConnection : Connection to " << 
         (player_id == -1 ? "" : "player " + boost::lexical_cast<std::string>(player_id)) << " " << 
         (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << socket << " terminated.";
   } else {
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::DumpConnection : Attempted to disconnect from "
         "socket " << socket << ". No such socket found.";
   }
   
   return retval;
}

bool ServerNetworkCore::EstablishPlayer(int player_id, int socket)
{
   bool retval = false;
   bool found = false;
   for (std::map<int, ConnectionInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
      if (it->second.socket == socket) {
         ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::EstablishPlayer : Attempted to establish "
            "player " << player_id << " on socket " << socket << ". But this player is already established on socket "
            << it->second.socket << ".";
         found = true;
         break;
      }
   }
   for (unsigned int i = 0; i < m_new_connections.size() && !found; ++i) {
      if (m_new_connections[i].socket == socket) {
         m_player_connections[player_id] = m_new_connections[i];
         m_new_connections.erase(m_new_connections.begin() + i);
         retval = true;
         ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::EstablishPlayer : Established "
            "player " << player_id << " on socket " << socket << ".";
         break;
      }
   }
   if (!retval) {
      ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::EstablishPlayer : Unable to establish "
         "player " << player_id << " on unknown socket " << socket << ".";
   }
   return retval;
}

void ServerNetworkCore::HandleNetEvent(SDL_Event& event)
{
// TODO: put this function in its own thread and create a message queue into which this function can dump messages
    if (event.type == SDL_USEREVENT) {
      switch(NET2_GetEventType(&event)) {
      case NET2_TCPACCEPTEVENT: {
         int port = NET2_GetEventData(&event);
         int socket = NET2_GetSocket(&event);
         if (port == NetworkCore::CONNECT_PORT) { // regular incoming connections
            IPaddress* addr = NET2_TCPGetPeerAddress(socket);
            char* socket_hostname = SDLNet_ResolveIP(addr);
            m_new_connections.push_back(ConnectionInfo(socket, *addr));
            ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::HandleNetEvent : Now connected to client at " << 
               (socket_hostname ? socket_hostname : "[unknown host]") << ", on socket " << socket << ".";
            SendMessage(Message(Message::SERVER_STATUS, -1, -1, Message::CORE, ServerApp::GetApp()->ServerStatus()), 
               socket, "ServerNetworkCore");
         } else { // oops. unknown port
            ServerApp::GetApp()->Logger().error("ServerNetworkCore::HandleNetEvent : Somehow we accepted a TCP connection "
               "on an unknown port!  Closing the new connection now.");
            NET2_TCPClose(socket);
         }
         break;
      }

      case NET2_TCPRECEIVEEVENT: {
         int socket = NET2_GetSocket(&event);
         ReceiveData(socket, m_receive_streams[socket], "ServerNetworkCore");
         break;
      }

      case NET2_TCPCLOSEEVENT: {
         int closing_socket = NET2_GetSocket(&event);
         int player_id = -1;
         for (std::map<int, ConnectionInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
            if (it->second.socket == closing_socket) {
               player_id = it->first;
               break;
            }
         }
         DumpConnection(closing_socket);
         if (player_id != -1)
            ServerApp::GetApp()->PlayerDisconnected(player_id);
         break;
      }
         
      case NET2_UDPRECEIVEEVENT: {
         int port = NET2_GetEventData(&event);
         if (port == NetworkCore::FIND_PORT) { // hosts looking for our IP address
// TODO: send back IP address via UDP
            
            ServerApp::GetApp()->Logger().debug("ServerNetworkCore::HandleNetEvent : Sent IP address to requsting host.");
         } else { // oops. unknown port
            ServerApp::GetApp()->Logger().error("ServerNetworkCore::HandleNetEvent : Somehow we accepted a UDP packet on "
               "an unknown port!  Ignoring packet.");
         }
         break;
      }

      case NET2_ERROREVENT: {
         if (const char* err_msg = NET2_GetEventError(&event))
            ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::HandleNetEvent : Network error \"" << err_msg << "\"";
         else 
            ServerApp::GetApp()->Logger().error("ServerNetworkCore::HandleNetEvent : Unknown network error.");
         break;
      }

      default: {
         ServerApp::GetApp()->Logger().error("ServerNetworkCore::HandleNetEvent : Recieved an SDL_USEREVENT that was not an SDL_net2 network event.");
         break;
      }
      }
   } else {
      ServerApp::GetApp()->Logger().error("ServerNetworkCore::HandleNetEvent : Recieved an SDL event that was not an SDL_USEREVENT.");
   }
}
   
void ServerNetworkCore::DispatchMessage(const Message& msg, int socket)
{
   bool sender_unknown = true;
   ConnectionInfo conn_info;
   for (std::map<int, ConnectionInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
      if (it->second.socket == socket) {
         sender_unknown = false;
         conn_info = it->second;
         break;
      }
   }
   if (sender_unknown) {
      for (std::vector<ConnectionInfo>::iterator it = m_new_connections.begin(); it != m_new_connections.end(); ++it) {
         if (it->socket == socket) {
            conn_info = *it;
            break;
         }
      }
   }
// TODO: add better Message screening
   if (msg.Receiver() == -1) { // a message destined for server
      switch (msg.Module()) {
      case Message::CORE:
         if (sender_unknown) {
            ServerApp::GetApp()->HandleNonPlayerMessage(msg, conn_info);
         } else {
            ServerApp::GetApp()->HandleMessage(msg);
         }
         break;
      default:
         ServerApp::GetApp()->Logger().errorStream()<< "ServerNetworkCore::DispatchMessage : Unknown module value \"" << 
            msg.Module() << "\" encountered.";
         break;
      }
   } else if (!sender_unknown) { // a valid player sending a message to another player, via the server
      SendMessage(msg);
   } else {
      ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::DispatchMessage : Unknown sender attempted to "
         "pass this message to player " << msg.Receiver() << ": " << msg.Type() << " " << msg.Sender() << " " << 
         msg.Receiver() << " " << msg.Module() << " " << msg.GetText();
   }
}

void ServerNetworkCore::ClosePorts()
{
   if (m_TCP_socket != -1) {
      NET2_TCPClose(m_TCP_socket);
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore : No longer listening for TCP connections on port " << 
         NetworkCore::CONNECT_PORT;
      m_TCP_socket = -1;
   }
   if (m_UDP_socket != -1) {
      NET2_UDPClose(m_UDP_socket);
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore : No longer listening for UDP packets on port " << 
         NetworkCore::FIND_PORT;
      m_UDP_socket = -1;
   }
}

