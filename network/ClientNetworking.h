#ifndef _ClientNetworking_h_
#define _ClientNetworking_h_

#include "Message.h"
#include "MessageQueue.h"

// boost::asio pulls in windows.h which in turn defines the macros Message,
// MessageBox, min and max. Disabling the generation of the min and max macros
// and undefining those should avoid name collisions with std c++ library and
// FreeOrion function names.
#define NOMINMAX
#include <boost/asio.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#ifdef FREEORION_WIN32
#   undef Message
#   undef MessageBox
#endif

#include <memory>


/** Encapsulates the networking facilities of the client.  The client must
    execute its networking code in a separate thread from its main processing
    thread, for UI and networking responsiveness, and to process synchronous
    communication with the server.

    Because of this, ClientNetworking operates in two threads: the main
    thread, in which the UI processing operates; the networking thread, which
    is created and terminates when the client is connected to and disconnected
    from the server, respectively.  The entire public interface is safe to
    call from the main thread at all times.  Note that the main thread must
    periodically request the next incoming message; the arrival of incoming
    messages is never explicitly signalled to the main thread.  The same
    applies to unintentional disconnects from the server.  The client must
    periodically check IsConnected().

    The ClientNetworking has three modes of operation.  First, it can discover
    FreeOrion servers on the local network; this is a blocking operation with
    a timeout.  Second, it can send an asynchronous message to the server
    (when connected); this is a non-blocking operation.  Third, it can send a
    synchronous message to the server (when connected) and return the server's
    response; this is a blocking operation.

    Note that the SendSynchronousMessage() does not interrupt the sending or
    receiving of asynchronous messages.  When SendSynchronousMessage() is
    called and the main thread blocks to wait for the response message,
    regular messages are still being sent and received.  Some of these regular
    messages may arrive before the response message, but
    SendSynchronousMessage() will still return the response message first,
    even if it is not at the front of the queue at the time.  This implies
    that some response messages may be handled out of order with respect to
    regular messages, but these are in fact the desired semantics. */
class ClientNetworking : public std::enable_shared_from_this<ClientNetworking> {
public:
    /** The type of list returned by a call to DiscoverLANServers(). */
    typedef std::vector<std::pair<boost::asio::ip::address, std::string>> ServerList;

    /** \name Structors */ //@{
    ClientNetworking();
    //@}

    /** \name Accessors */ //@{
    /** Returns true iff the client is full duplex connected to the server. */
    bool IsConnected() const;

    /** Returns true iff the client is connected to receive from the server. */
    bool IsRxConnected() const;

    /** Returns true iff the client is connected to send to the server. */
    bool IsTxConnected() const;

    /** Returns true iff there is at least one incoming message available. */
    bool MessageAvailable() const;

    /** Returns the ID of the player on this client. */
    int PlayerID() const;

    /** Returns the ID of the host player, or INVALID_PLAYER_ID if there is no host player. */
    int HostPlayerID() const;

    /** Returns whether the indicated player ID is the host. */
    bool PlayerIsHost(int player_id) const;
    //@}

    /** \name Mutators */ //@{
    /** Returns a list of the addresses and names of all servers on the Local
        Area Network. */
    ServerList DiscoverLANServers();

    /** Connects to the server at \a ip_address.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToServer(const std::string& ip_address,
                         std::chrono::milliseconds timeout = std::chrono::seconds(10));

    /** Connects to the server on the client's host.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToLocalHostServer(std::chrono::milliseconds timeout =
                                  std::chrono::seconds(10));

    /** Sends \a message to the server.  This function actually just enqueues
        the message for sending and returns immediately. */
    void SendMessage(Message message);

    /** Gets the next incoming message from the server, places it into \a
        message, and removes it from the incoming message queue.  The function
        assumes that there is at least one message in the incoming queue.
        Users must call MessageAvailable() first to make sure this is the
        case. */
    void GetMessage(Message& message);

    /** Sends \a message to the server, then blocks until it sees the first
        synchronous response from the server. */
    void SendSynchronousMessage(Message message, Message& response_message);

    /** Disconnects the client from the server. First tries to send any pending transmit messages. */
    void DisconnectFromServer();

    /** Sets player ID for this client. */
    void SetPlayerID(int player_id);

    /** Sets Host player ID. */
    void SetHostPlayerID(int host_player_id);
    //@}

private:
    void HandleException(const boost::system::system_error& error);
    void HandleConnection(boost::asio::ip::tcp::resolver::iterator* it,
                          const boost::system::error_code& error);
    void CancelRetries();
    void NetworkingThread(std::shared_ptr<ClientNetworking>& self);
    void HandleMessageBodyRead(const std::shared_ptr<ClientNetworking>& keep_alive,
                               boost::system::error_code error, std::size_t bytes_transferred);
    void HandleMessageHeaderRead(const std::shared_ptr<ClientNetworking>& keep_alive,
                                 boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncReadMessage(const std::shared_ptr<ClientNetworking>& keep_alive);
    void HandleMessageWrite(        boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncWriteMessage();
    void SendMessageImpl(Message message);
    void DisconnectFromServerImpl();

    int                             m_player_id;
    int                             m_host_player_id;

    boost::asio::io_service         m_io_service;
    boost::asio::ip::tcp::socket    m_socket;

    // m_mutex guards m_incoming_message, m_rx_connected and m_tx_connected which are written by
    // the networking thread and read by the main thread to check incoming messages and connection
    // status. As those read and write operations are not atomic, shared access has to be
    // protected to prevent unpredictable results.
    mutable boost::mutex            m_mutex;

    MessageQueue                    m_incoming_messages; // accessed from multiple threads, but its interface is threadsafe
    std::list<Message>              m_outgoing_messages;
    bool                            m_rx_connected;      // accessed from multiple threads
    bool                            m_tx_connected;      // accessed from multiple threads

    Message::HeaderBuffer           m_incoming_header;
    Message                         m_incoming_message;
    Message::HeaderBuffer           m_outgoing_header;
};

#endif
