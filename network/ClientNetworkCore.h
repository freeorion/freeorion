// -*- C++ -*-
#ifndef _ClientNetworkCore_h_
#define _ClientNetworkCore_h_

#ifndef _NetworkCore_h_
#include "NetworkCore.h"
#endif

#ifndef _Message_h_
#include "Message.h"
#endif

#include <set>

// deal with dirty, dirty MS macros
#if defined(_MSC_VER)
# if defined(SendMessage)
#  undef SendMessage
# endif
# if defined(DispatchMessage)
#  undef DispatchMessage
# endif
#endif

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
    virtual void SendMessage(const Message& msg);
   
    bool SendSynchronousMessage( const Message& msg, Message& response_data ); ///< Blocking function. Will not exit until response message is sent by the server. Only works with messages that are expecting a response. The data value is an integer send with server response message. Returns false if timeout occured
    void SynchronousMessageEarlyTimout(); ///< cancels any currently-pending wait on a synchronous a message
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

    void HandleSynchronousResponse( const Message& msg );
   
    int          m_server_socket;     ///< the number of the socket through which the client is connected to the server; -1 if not connected
    std::string  m_receive_stream;    ///< all network traffic must go through the server, so we only need this one input streams
    bool         m_listening_on_LAN;  ///< set to true when the client is broadcasting a discovery request on the LAN
    unsigned int m_synch_message_start_time;

    Message::MessageType m_waiting_for_msg;   ///< for a synchronous message, this is set to the msg type it's waiting for to be returned by the server
    Message              m_response_msg;     ///< the synchronous response will contain data which the calling sender is expecting
};

inline std::pair<std::string, std::string> ClientNetworkCoreRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _ClientNetworkCore_h_

