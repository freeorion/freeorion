#ifndef _ClientNetworkCore_h_
#define _ClientNetworkCore_h_

#ifndef _NetworkCore_h_
#include "NetworkCore.h"
#endif

#include <set>

/** the network core needed by the FreeOrion clients.  ClientNetworkCore is rather simple, since it only needs to worry
   itself with a single connection, especially since that connection is initiated by ClientNetworkCore itself.  
   ClientNetworkCore extends NetworkCore by implementing 3 different connection techniques: one for connecting to a
   server on the same machine, on for connecting to a server on the LAN where the client is, and one for connecting
   to a server at an arbitrary IP address.*/
class ClientNetworkCore : public NetworkCore
{
public:
   /** \name Structors */ //@{
   ClientNetworkCore();
   virtual ~ClientNetworkCore();
   //@}

   /** \name Accessors */ //@{
   bool Connected() const {return m_server_socket != -1;} ///< returns true if there is a valid connection to the server
   virtual void SendMessage(const Message& msg) const;
   //@}

   /** \name Mutators */ //@{
   /** discovers if there is one or more FreeOrion server on the local subnet.  This function broadcasts a request
      once per second and listens for responders; it will do so for \a timeout seconds.  It returns a set 
      of servers found, if any.*/
   std::set<std::string> DiscoverLANServers(int timeout);
   
   /** attempts to connect to a FreeOrion server on localhost; returns true on success */
   bool ConnectToLocalhostServer();
   
   /** attempts to connect to a specific FreeOrion server; returns true on success. The \a server parameter 
      may be a hostname or an IP address string */
   bool ConnectToServer(const std::string& server);

   bool DisconnectFromServer(); ///< immediately but cleanly terminates connection to server; returns true if client was already connected and the connection succeeded

   virtual void HandleNetEvent(SDL_Event& event);
   //@}
   
private:
   using NetworkCore::SendMessage;
   
   const ClientNetworkCore& operator=(const ClientNetworkCore&); // disabled
   ClientNetworkCore(const ClientNetworkCore&); // disabled
   
   virtual void DispatchMessage(const Message& msg, int socket);
   
   int         m_server_socket;     ///< the number of the socket through which the client is connected to the server; -1 if not connected
   std::string m_receive_stream;    ///< all network traffic must go through the server, so we only need this one input streams
   bool        m_listening_on_LAN;  ///< set to true when the client is broadcasting a discovery request on the LAN
};

#endif // _ClientNetworkCore_h_

