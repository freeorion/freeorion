#include "ServerNetworkCore.h"

#include "Message.h"
#include "../server/ServerApp.h"

#include <log4cpp/Category.hh>
#include <boost/lexical_cast.hpp>

#include <sstream>

// deal with dirty, dirty MS macros
#if defined(_MSC_VER)
# if defined(SendMessage)
#  undef SendMessage
# endif
# if defined(DispatchMessage)
#  undef DispatchMessage
# endif
#endif

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

void ServerNetworkCore::SendMessage(const Message& msg)
{
   int receiver_id = msg.Receiver();
   int receiver_socket = -1;

   std::map<int, PlayerInfo>::const_iterator it = m_player_connections.find(receiver_id);
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
   } else if ((m_UDP_socket = NET2_UDPAcceptOn(NetworkCore::SERVER_FIND_LISTEN_PORT, NetworkCore::SERVER_FIND_QUERY_MSG.size())) == -1) {
      char* err_msg = NET2_GetError();
      ServerApp::GetApp()->Logger().fatalStream() << "ServerNetworkCore::ServerNetworkCore : failed to open port " << 
         NetworkCore::SERVER_FIND_LISTEN_PORT << " for UDP packets.  SDL_net2 error: " << (err_msg ? err_msg : "[unknown error]");
      NET2_TCPClose(m_TCP_socket);
      ServerApp::GetApp()->Exit(1);
   } else {
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore : Listening for TCP connections on port " << 
         NetworkCore::CONNECT_PORT;
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore : Listening for UDP packets on port " << 
         NetworkCore::SERVER_FIND_LISTEN_PORT;
   }
}

bool ServerNetworkCore::EstablishPlayer(int socket, int player_id, const PlayerInfo& data)//int player_id, const std::string& name, int socket)
{
   bool retval = false;
   bool found = false;
   for (std::map<int, PlayerInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
      if (it->second.socket == socket) {
         ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::EstablishPlayer : Attempted to establish \"" << data.name << 
            "\" (player #" << player_id << ") on socket " << socket << ". But this player is already established on socket "
            << it->second.socket << ".";
         found = true;
         break;
      }
   }
   for (unsigned int i = 0; i < m_new_connections.size() && !found; ++i) {
      if (m_new_connections[i].socket == socket) {
         m_player_connections[player_id] = data;
         m_player_connections[player_id].socket = m_new_connections[i].socket;
         m_player_connections[player_id].address = m_new_connections[i].address;
         m_new_connections.erase(m_new_connections.begin() + i);
         retval = true;
         for (unsigned int i = 0; i < m_player_connections.size(); ++i) {
             if (static_cast<int>(i) == player_id) 
                 continue;
             if (data.name == m_player_connections[i].name) {
                 m_player_connections[player_id].name += "0";
                 SendMessage(RenameMessage(player_id, m_player_connections[player_id].name));
                 break;
             }
         }
         ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::EstablishPlayer : Established \"" << m_player_connections[player_id].name << 
            "\" (player #" << player_id << ") on socket " << socket << ".";
         break;
      }
   }
   if (!retval) {
      ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::EstablishPlayer : Unable to establish \"" << data.name << 
         "\" (player #" << player_id << ") on unknown socket " << socket << ".";
   }
   return retval;
}

bool ServerNetworkCore::DumpPlayer(int player_id)
{
    bool retval = false;
    std::map<int, PlayerInfo>::iterator it = m_player_connections.find(player_id);
    if (it != m_player_connections.end())
        retval = DumpConnection(it->second.socket);
    return retval;
}

bool ServerNetworkCore::DumpConnection(int socket)
{
   bool retval = false;
   int player_id = -1;
   for (std::map<int, PlayerInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
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
      const char* socket_hostname = SDLNet_ResolveIP(addr);
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

void ServerNetworkCore::DumpAllConnections()
{
   for (std::map<int, PlayerInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
      int player_id = -1;
      int socket = it->second.socket;
      player_id = it->first;
      IPaddress* addr = NET2_TCPGetPeerAddress(socket);
      const char* socket_hostname = SDLNet_ResolveIP(addr);
      NET2_TCPClose(socket);
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::DumpAllConnections : Connection to " << 
         (player_id == -1 ? "" : "player " + boost::lexical_cast<std::string>(player_id)) << " " << 
         (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << socket << " terminated.";
      m_player_connections.erase(it);
      break;
   }
   for (unsigned int i = 0; i < m_new_connections.size(); ++i) {
      int socket = m_new_connections[i].socket;
      IPaddress* addr = NET2_TCPGetPeerAddress(socket);
      const char* socket_hostname = SDLNet_ResolveIP(addr);
      NET2_TCPClose(socket);
      ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::DumpAllConnections : Connection to non-player " <<
          (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << socket << " terminated.";
      m_new_connections.erase(m_new_connections.begin() + i);
      break;
   }
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
            const char* socket_hostname = SDLNet_ResolveIP(addr);
            m_new_connections.push_back(PlayerInfo(socket, *addr));
            ServerApp::GetApp()->Logger().debugStream() << "ServerNetworkCore::HandleNetEvent : Now connected to client at " <<
               (socket_hostname ? socket_hostname : "[unknown host]") << ", on socket " << socket << ".";
            SendMessage(Message(Message::SERVER_STATUS, -1, -1, Message::CORE, ServerApp::GetApp()->ServerStatusDoc()),
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
         for (std::map<int, PlayerInfo>::iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
            if (it->second.socket == closing_socket) {
               player_id = it->first;
               break;
            }
         }
         if (player_id != -1)
            ServerApp::GetApp()->PlayerDisconnected(player_id);
         DumpConnection(closing_socket);
         break;
      }
         
      case NET2_UDPRECEIVEEVENT: {
         int socket = NET2_GetSocket(&event);
         UDPpacket* packet = 0;
         while ((packet = NET2_UDPRead(socket))) {
            IPaddress return_address;
            std::string incoming_msg;
            for (int i = 0; i < packet->len; ++i)
               incoming_msg += packet->data[i];
            if (incoming_msg == SERVER_FIND_QUERY_MSG) {
               NET2_ResolveHost(&return_address, const_cast<char*>(ToString(packet->address).c_str()), NetworkCore::SERVER_FIND_RESPONSE_PORT);
               if (ServerApp::GetApp()->State() == SERVER_MP_LOBBY || ServerApp::GetApp()->State() == SERVER_GAME_SETUP) {
                   if (NET2_UDPSend(&return_address, const_cast<char*>(NetworkCore::SERVER_FIND_YES_MSG.c_str()), 
                                    NetworkCore::SERVER_FIND_YES_MSG.size()) == -1) {
                       const char* err_msg = NET2_GetError();
                       ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::HandleNetEvent : Call to "
                           "NET2_UDPSend() failed; SDL_net2 error: \"" << (err_msg ? err_msg : "[unknown]") << "\"";
                   } else {
                       ServerApp::GetApp()->Logger().debug("ServerNetworkCore::HandleNetEvent : Sent IP address to requsting host.");
                   }
               }
            } else {
                ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::HandleNetEvent : Received unknown UDP packet type "
                      "containing: \"" << incoming_msg << "\"";
            }
            NET2_UDPFreePacket(packet);
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
   bool spoofed_sender = false;
   PlayerInfo conn_info;
   std::map<int, PlayerInfo>::iterator sender_conn_it = m_player_connections.find(msg.Sender());
   if (sender_conn_it != m_player_connections.end()) {
      sender_unknown = false;
      conn_info = sender_conn_it->second;
      if (sender_conn_it->second.socket != socket)
         spoofed_sender = true;
   }
   if (sender_unknown) {
      for (std::vector<PlayerInfo>::iterator it = m_new_connections.begin(); it != m_new_connections.end(); ++it) {
         if (it->socket == socket) {
            conn_info = *it;
            break;
         }
      }
   }
// TODO: add more Message screening
   if (spoofed_sender) {
      ServerApp::GetApp()->Logger().errorStream() << "ServerNetworkCore::DispatchMessage : Player #" << sender_conn_it->first << 
         " sent this message pretending to be player #" << msg.Sender() << " (spoofing player will be dumped): " << 
         msg.Type() << " " << msg.Sender() << " " << msg.Receiver() << " " << msg.Module() << " " << msg.GetText();
         DumpConnection(conn_info.socket);
   } else if (msg.Receiver() == -1) { // a message addressed to the server
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
         NetworkCore::SERVER_FIND_LISTEN_PORT;
      m_UDP_socket = -1;
   }
}


