#include "ClientNetworkCore.h"

#include "Message.h"
#include "../client/ClientApp.h"
#include "net2.h"
#include "fastevents.h"
#ifdef FREEORION_BUILD_HUMAN
#include "../UI/MultiplayerLobbyWnd.h"
#endif

#include <log4cpp/Category.hh>

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

namespace {
log4cpp::Category& logger = log4cpp::Category::getRoot();
const unsigned int SYCHRONOUS_TIMEOUT = 5000;    // 5 seconds
}

ClientNetworkCore::ClientNetworkCore() : 
    m_server_socket(-1),
    m_waiting_for_msg( Message::UNDEFINED )
{
}

ClientNetworkCore::~ClientNetworkCore()
{
    if (m_server_socket != -1) {
        NET2_TCPClose(m_server_socket);
        logger.debug("ClientNetworkCore::HandleNetEvent : Connection to server terminated.");
    }
}

void ClientNetworkCore::SendMessage(const Message& msg) 
{
    if (m_server_socket != -1) {
        SendMessage(msg, m_server_socket, "ClientNetworkCore");
    } else {
        logger.error("ClientNetworkCore::SendMessage : Attempted to send a message to the server when not yet connected.");
    }
}   


bool ClientNetworkCore::SendSynchronousMessage( const Message& msg, Message& response_msg )
{
    bool success = false;   

    SendMessage( msg );

    // check for a synchronous message
    // this could be more sophisticated but there will probably not be many messages of this type
    if ( msg.Response() != Message::UNDEFINED ) {
        // wait here for a response
        // this works because thankfully the net2 code is threaded
        SDL_Event ev;            

        // setup information informing network code that we're waiting for a particular mesage type
        m_waiting_for_msg = msg.Response();
        m_response_msg = Message();

        // this requires a timeout since if there is a chance that we'll never get a response
        unsigned int start_time = SDL_GetTicks();

        while (( SDL_GetTicks() - start_time ) < SYCHRONOUS_TIMEOUT) {
            if (FE_PollEvent(&ev)) {
                // ignore all but network  messages
                if ( ev.type == SDL_USEREVENT ) {
                    int net2_type = NET2_GetEventType(const_cast<SDL_Event*>(&ev));
                    if (net2_type == NET2_ERROREVENT || 
                        net2_type == NET2_TCPACCEPTEVENT || 
                        net2_type == NET2_TCPRECEIVEEVENT || 
                        net2_type == NET2_TCPCLOSEEVENT || 
                        net2_type == NET2_UDPRECEIVEEVENT)
                        HandleNetEvent(const_cast<SDL_Event&>(ev));
                }

                if ( m_waiting_for_msg == Message::UNDEFINED ) {
                    // message has been received, get data and break loop
                    success = true;
                    response_msg = m_response_msg;
                    break;
                }

            } else {
              SDL_Delay(100); // reduce cpu load while waiting
	    }
	}
    }

    return success;
}

std::set<std::string> ClientNetworkCore::DiscoverLANServers(int timeout)
{
    std::set<std::string> retval;
   
    timeout *= 1000; // use milliseconds for convenience

    // set listen flag and initialize UDP socket
    m_listening_on_LAN = true;
    int socket = NET2_UDPAcceptOn(NetworkCore::SERVER_FIND_RESPONSE_PORT, NetworkCore::SERVER_FIND_QUERY_MSG.size());
    if (socket == -1) {
        const char* err_msg = NET2_GetError();
        logger.errorStream() << "ClientNetworkCore::DiscoverLANServers : Call to NET2_UDPAcceptOn() "
            "failed; SDL_net2 error: \"" << (err_msg ? err_msg : "[unknown]") << "\"";
    } else {
        // broadcast and poll for timeout milliseconds
        IPaddress broadcast_address;
        if (NET2_ResolveHost(&broadcast_address, "255.255.255.255", NetworkCore::SERVER_FIND_LISTEN_PORT) == -1 ||
            NET2_UDPSend(&broadcast_address, "255.255.255.255", 15) == -1) {
            const char* err_msg = NET2_GetError();
            logger.errorStream() << "ClientNetworkCore::DiscoverLANServers : Call to NET2_ResolveHost() "
                "or call to NET2_UDPSend() failed; SDL_net2 error: \"" << (err_msg ? err_msg : "[unknown]") << "\"";
        }

        int last_time = SDL_GetTicks();
        int last_second = timeout / 1000; // the last second in which a broadcast was made
        SDL_Event event;

        while (0 < timeout) {
            int time = SDL_GetTicks();

            // if it has been about a second since the last broadcast, do it again
            if (last_second < time / 1000) {
                if (NET2_UDPSend(&broadcast_address, const_cast<char*>(NetworkCore::SERVER_FIND_QUERY_MSG.c_str()), 
                                 NetworkCore::SERVER_FIND_QUERY_MSG.size()) == -1) {
                    const char* err_msg = NET2_GetError();
                    logger.errorStream() << "ClientNetworkCore::DiscoverLANServers : Call to NET2_UDPSend() "
                        "failed; SDL_net2 error: \"" << (err_msg ? err_msg : "[unknown]") << "\"";
                }
                last_second = time / 1000;
            }

            // handle events, picking out the ones that are incoming UDP packets
            if (FE_PollEvent(&event)) {
                if (event.type == SDL_USEREVENT && NET2_GetEventType(&event) == NET2_UDPRECEIVEEVENT &&
                    NET2_GetSocket(&event) == socket) {
                    UDPpacket* packet = 0;
                    while (packet = NET2_UDPRead(NET2_GetSocket(&event))) {
                        retval.insert(ToString(packet->address));
                        NET2_UDPFreePacket(packet);
                    }
                }
            }
            timeout -= time - last_time;
            last_time = time;
        }
      
        // reset listen flag and take down UDP socket
        m_listening_on_LAN = false;
        NET2_UDPClose(socket);
    }
    return retval;
}

bool ClientNetworkCore::ConnectToLocalhostServer()
{
    return ConnectToServer("localhost");
}

bool ClientNetworkCore::ConnectToServer(const std::string& server)
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

    case Message::CLIENT_LOBBY_MODULE:
        if (!ClientApp::MultiplayerLobby()) {
            logger.errorStream()<< "ClientNetworkCore::DispatchMessage : Attempted to pass message to client multiplayer lobby module when "
                "there is no current lobby.";
        } else {
#ifdef FREEORION_BUILD_HUMAN
            ClientApp::MultiplayerLobby()->HandleMessage(msg);
#endif
        }
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

    case Message::CLIENT_SYNCHRONOUS_RESPONSE:
        HandleSynchronousResponse( msg );
        break;

    default:
        logger.errorStream()<< "ClientNetworkCore::DispatchMessage : Unknown module value \"" << 
            msg.Module() << "\" encountered.";
        break;
    }
}

void ClientNetworkCore::HandleSynchronousResponse( const Message& msg )
{
    // check if sychronous response has been received
    if ( m_waiting_for_msg == msg.Type() ) {
        // handle each message
        switch( msg.Type() ) {
        case Message::DISPATCH_NEW_OBJECT_ID: 
            m_response_msg = msg;
            break;

        case Message::SAVE_GAME:
            m_response_msg = msg;
            break;

        default:
            logger.errorStream()<< "ClientNetworkCore::HandleSynchronousResponse : Unknown response \"" << 
                msg.Module() << "\" encountered.";
            break;
        }

        // signal we're done waiting for msg
        m_waiting_for_msg  = Message::UNDEFINED;
    }
}

