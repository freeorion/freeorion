// -*- C++ -*-
#ifndef _ServerNetworkCore_h_
#define _ServerNetworkCore_h_

#ifndef _NetworkCore_h_
#include "NetworkCore.h"
#endif

#ifndef _NET2_H_
#include "../SDL_net2/net2.h"
#endif

#include <map>
#include <string>
#include <vector>

class Message;
class PlayerInfo;

/** the network core needed by the FreeOrion server.  This class extends NetworkCore by allowing mutliple connections, 
   allowing connections to be associated with players by the server app, and allowing connections to be terminated by 
   the server app.  The ServerNetworkCore allows \a any incoming TCP connection, but maintains a record of each one; 
   when the server app indicates that one of these connections is a valid player, the ServerNetworkCore moves it to 
   the list of player connections that it also maintains.  
   Future Message traffic to that player will go out on that TCP connection.  The ServerNetworkCore does no authentication
   of any kind; it is up to the server app to establish a connection as a player or to dump the connection.  The 
   ServerNetworkCore does however authenticate Messages to the best of its ability.  The ServerNetworkCore will pass
   Messages from established players to the server app via a call to HandleMessage(), whereas
   an unknown connection's Message will be passed in a call to HandleNonPlayerMessage().  Messages that are obviously 
   bogus or malformed will be logged and discarded.  Incoming UDP requests for the server's IP address are also handled
   by the ServerNetworkCore, with no involvement by the server app.*/
class ServerNetworkCore : public NetworkCore
{
public:
   /** \name Structors */ //@{
   ServerNetworkCore();
   virtual ~ServerNetworkCore();
   //@}

   /** \name Accessors */ //@{
   const std::map<int, PlayerInfo>& Players() const {return m_player_connections;}

   virtual void SendMessage(const Message& msg);
   //@}
   
   /** \name Mutators */ //@{
   void ListenToPorts(); ///< closes any currently-open listen-ports, then sets up ports for incoming connections
   bool EstablishPlayer(int socket, int player_id, const PlayerInfo& data); ///< establishes player with ID number \a id as being the connection on \a socket; returns true on success
   bool DumpPlayer(int player_id); ///< disconnects player \a player_id; returns true if a connection to \a player_id existed and was terminated
   bool DumpConnection(int socket); ///< disconnects the connection on socket number \a socket; returns true if a connection on \a socket existed and was terminated
   void DumpAllConnections(); ///< closes all connections

   virtual void HandleNetEvent(SDL_Event& event);
   //@}
   
private:
   using NetworkCore::SendMessage;
   
   const ServerNetworkCore& operator=(const ServerNetworkCore&); // disabled
   ServerNetworkCore(const ServerNetworkCore&); // disabled

   virtual void DispatchMessage(const Message& msg, int socket);
   void ClosePorts();
   
   std::map<int, std::string> m_receive_streams;      ///< a map of streams of incoming data, keyed on socket number
   std::vector<PlayerInfo>    m_new_connections;      ///< connection info objects for new connections
   std::map<int, PlayerInfo>  m_player_connections;   ///< connection info objects associated with established players; indexed by player id number
   int                        m_TCP_socket;           ///< the "socket" number returned by SDL_net2; close this socket to stop listening on the port
   int                        m_UDP_socket;           ///< the "socket" number returned by SDL_net2; close this socket to stop listening on the port
};

#endif // _ServerNetworkCore_h_
