// -*- C++ -*-
#ifndef _NetworkCore_h_
#define _NetworkCore_h_

#include <string>

#ifndef _SDL_H
#include "SDL.h"
#endif

#ifndef _NET2_H_
#include <GG/net/net2.h>
#endif

// deal with dirty, dirty MS macros
#if defined(_MSC_VER)
# if defined(SendMessage)
#  undef SendMessage
# endif
# if defined(DispatchMessage)
#  undef DispatchMessage
# endif
#endif

/** the states the server may be in at various points during its execution*/
enum ServerState {SERVER_IDLE,         ///< there is no game yet and no one has send a HOST_GAME Message yet; this is the initial state
                  SERVER_MP_LOBBY,     ///< the host and possibly other players are in the multiplayer lobby, preparing to start a game
                  SERVER_GAME_SETUP,   ///< a HOST_GAME Message has been received, and a game is being set up (the server is waiting for all players to join)
                  SERVER_WAITING,      ///< a game is in progress and currently the server is waiting for players to finish their turns
                  SERVER_PROCESSING,   ///< the server is processing a turn
                  SERVER_DISCONNECT,   ///< the server has encountered a disconnect error and is dealing with it
                  SERVER_DYING         ///< the server is ending its execution
                 };

class Message;

/** abstract base class for ServerNetworkCore and ClientNetworkCore.  The NetworkCore is responsible for sending 
   Messages to and from the server and the clients.  It also handles SDL_net2 events for the app that uses it, so
   the app need not know anything about sockets, TCP, UDP, etc.*/
class NetworkCore
{
public:
   /** \name Accessors */ //@{
   /** sends entire contents of \a msg to the appropriate socket (based on the receiver), followed by a special 
      end-of-message marker */
   virtual void SendMessage(const Message& msg)= 0; 
   //@}
   
   /** \name Mutators */ //@{
   virtual void HandleNetEvent(SDL_Event& event) = 0; ///< directly handles SDL network events
   //@}
   
   static const std::string   EOM_STR;                   ///< the string that marks the end of a Message in a TCP byte stream
   static const int           SERVER_FIND_LISTEN_PORT;   ///< the port used by servers to their IP address(es)
   static const int           SERVER_FIND_RESPONSE_PORT; ///< the port used to catch server responses when looking for the IP address(es) of server(s) on the LAN
   static const int           CONNECT_PORT;              ///< the port used to make TCP connections
   static const int           HOST_PLAYER_ID;            ///< the ID number assigned to the hosting player
   static const std::string   SERVER_FIND_QUERY_MSG;     ///< the UDP message used to ask if this server is hosting a game that the querying host may join
   static const std::string   SERVER_FIND_YES_MSG;       ///< the UDP message used to indicate this server is hosting a game that the querying host may join
   static const std::string   SERVER_FIND_NO_MSG;        ///< the UDP message used to indicate this server is not hosting a game that the querying host may join

    virtual ~NetworkCore();
protected:
   /** sends \a msg to the designated socket.  Creates a log entry on error.  \throw std::invalid_argument 
      Throws std::invalid_argument if \a socket is < 0.*/
   void SendMessage(const Message& msg, int socket, const std::string& app_name) const; 
   
   /** packages incoming data from \a socket into Messages and sends them out via DispatchMessage(); partial 
      Messages will remain in \a stream until the rest of the Message arrives.  Creates a log entry on error.  
      \throw std::invalid_argument Throws std::invalid_argument if \a socket is < 0.*/
   void ReceiveData(int socket, std::string& stream, const std::string& app_name); 

private:
   /** routes \a msg to the appropriate destination module. 
      \throw std::invalid_argument May throw std::invalid_argument if the destination does not exist on this host.*/
   virtual void DispatchMessage(const Message& msg, int socket) = 0;
};

/** returns the dotted address notation for \a addr.  Provided since the only stringification provided by SDL 
   net/net2 involves hostname resolution.*/
std::string ToString(const IPaddress& addr);

inline std::string NetworkCoreRevision()
{return "$Id$";}

#endif // _NetworkCore_h_

