#ifndef _NetworkCore_h_
#define _NetworkCore_h_

#include <string>

#ifndef _SDL_H
#include <SDL.h>
#endif

#ifndef _SDLnet_h
#include "../SDL_net2/net2.h"
#endif

/** the states the server may be in at various points during its execution*/
enum ServerState {SERVER_IDLE,         ///< there is no game yet and no one has send a HOST_GAME Message yet; this is the initial state
                  SERVER_GAME_SETUP,   ///< a HOST_GAME Message has been received, and a game is being set up
                  SERVER_WAITING,      ///< a game is in progress and currently the server is waiting for players to finish their turns
                  SERVER_PROCESSING,   ///< the server is processing a turn
                  SERVER_DISCONNECT    ///< the server has encountered a disconnect error and is dealing with it
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
   virtual void SendMessage(const Message& msg) const = 0; 
   //@}
   
   /** \name Mutators */ //@{
   virtual void HandleNetEvent(SDL_Event& event) = 0; ///< directly handles SDL network events
   //@}
   
   static const std::string   EOM_STR;                ///< the string that marks the end of a Message in a TCP byte stream
   static const int           FIND_PORT;              ///< the port used to find the IP address(es) of server(s) on the LAN
   static const int           CONNECT_PORT;           ///< the port used to make TCP connections
   static const std::string   FIND_SERVER_PACKET_MSG; ///< the UDP message used to find IP address(es) of server(s) on the LAN
   
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

#endif // _NetworkCore_h_

