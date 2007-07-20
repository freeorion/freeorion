// -*- C++ -*-
#ifndef _ClientNetworking_h_
#define _ClientNetworking_h_

#include "Message.h"
#include "MessageQueue.h"

#if defined(_MSC_VER)
  // HACK! this keeps VC 7.x from barfing when it sees "typedef __int64 int64_t;"
  // in boost/cstdint.h when compiling under windows
#  if defined(int64_t)
#    undef int64_t
#  endif
#elif defined(WIN32)
  // HACK! this keeps gcc 3.x from barfing when it sees "typedef long long uint64_t;"
  // in boost/cstdint.h when compiling under windows
#  define BOOST_MSVC -1
#endif

#include <boost/array.hpp>
#include <boost/asio.hpp>


/** Encapsulates the networking facilities of the client.  The client must execute its networking code in a separate
    thread from its main processing thread, for UI and networking responsiveness, and to process synchronous
    communication with the server.  It is optional to do so in the server, which can be entirely network-driven.

    Because of this, ClientNetworking operates in two threads: the main thread, in which the UI processing operates; the
    networking thread, which is created and terminates when the client is connected to and disconnected from the server,
    respectively.  The entire public interface is safe to call from the main thread at all times.  Note that the main
    thread must periodically request the next incoming message; the arrival of incoming messages is never explicitly
    signalled to the main thread.  The same applies to unintentional disconnects from the server.  The client must
    periodically check Connected().

    The ClientNetworking has three modes of operation.  First, it can discover FreeOrion servers on the local network;
    this is a blocking operation with a timeout.  Second, it can send an asynchronous message to the server (when
    connected); this is a non-blocking operation.  Third, it can send a synchronous message to the server (when
    connected) and return the server's response; this is a blocking operation.

    Note that the SendSynchronousMessage() does not interrupt the sending or receiving of asynchronous messages.  When
    SendSynchronousMessage() is called and the main thread blocks to wait for the response message, regular messages are
    still being sent and received.  Some of these regular messages may arrive before the response message, but
    SendSynchronousMessage() will still return the response message first, even if it is not at the front of the queue
    at the time.  This implies that some response messages may be handled out of order with respect to regular messages,
    but these are in fact the desired semantics. */
class ClientNetworking
{
public:
    /** The type of list returned by a call to DiscoverLANServers(). */
    typedef std::vector<std::pair<boost::asio::ip::address, std::string> > ServerList;

    /** \name Structors */ //@{
    ClientNetworking(); ///< Basic ctor.
    //@}

    /** \name Accessors */ //@{
    bool Connected() const;        ///< Returns true iff the client is connected to the server.
    bool MessageAvailable() const; ///< Returns true iff there is at least one incoming message available.
    //@}

    /** \name Mutators */ //@{
    /** Returns a list of the addresses and names of all servers on the Local Area Network. */
    ServerList DiscoverLANServers();

    /** Connects to the server at \a ip_address.  On failure, repeated attempts will be made until \a timeout seconds
        has elapsed. */
    bool ConnectToServer(const std::string& ip_address, boost::posix_time::seconds timeout = boost::posix_time::seconds(5));

    /** Connects to the server on the client's host.  On failure, repeated attempts will be made until \a timeout
        seconds has elapsed. */
    bool ConnectToLocalHostServer(boost::posix_time::seconds timeout = boost::posix_time::seconds(5));

    /** Sends \a message to the server.  This function actually just enqueues the message for sending and returns immediately. */
    void SendMessage(const Message& message);

    /** Gets the next incoming message from the server, places it into \a message, and removes it from the incoming
        message queue.  The function assumes that there is at least one message in the incoming queue.  Users must call
        MessageAvailable() first to make sure this is the case. */
    void GetMessage(Message& message);

    /** Sends \a message to the server, then blocks until it sees the first synchronous response from the server. */
    void SendSynchronousMessage(const Message& message, Message& response_message);

    /** Disconnects the client from the server. */
    void DisconnectFromServer();
    //@}

private:
    typedef boost::array<int, 5> MessageHeaderBuffer;

    void HandleException(const boost::system::system_error& error);
    void HandleConnection(boost::asio::ip::tcp::resolver::iterator* it, boost::asio::deadline_timer* timer, const boost::system::error_code& error);
    void CancelRetries();
    void NetworkingThread();
    void HandleMessageBodyRead(boost::system::error_code error, std::size_t bytes_transferred);
    void HandleMessageHeaderRead(boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncReadMessage();
    void HandleMessageWrite(boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncWriteMessage();
    void SendMessageImpl(Message& message);
    void DisconnectFromServerImpl();

    boost::asio::io_service          m_io_service;
    boost::asio::ip::tcp::socket     m_socket;
    mutable boost::mutex             m_mutex;
    MessageQueue                     m_incoming_messages;     // accessed from multiple threads, but its interface is threadsafe
    std::list<Message>               m_outgoing_messages;
    bool                             m_connected;             // accessed from multiple threads
    bool                             m_cancel_retries;

    MessageHeaderBuffer              m_incoming_header;
    Message                          m_incoming_message;
    MessageHeaderBuffer              m_outgoing_header;

    enum { HEADER_SIZE = MessageHeaderBuffer::static_size * sizeof(MessageHeaderBuffer::value_type) };
};

#endif
