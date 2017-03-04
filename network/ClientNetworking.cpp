#define WIN32_LEAN_AND_MEAN

#include "ClientNetworking.h"

#include "Networking.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/bind.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>

#include <thread>

using boost::asio::ip::tcp;
using namespace Networking;

namespace {
    const bool TRACE_EXECUTION = false;

    /** A simple client that broadcasts UDP datagrams on the local network for
        FreeOrion servers, and reports any it finds. */
    class ServerDiscoverer {
    public:
        typedef ClientNetworking::ServerList ServerList;

        ServerDiscoverer(boost::asio::io_service& io_service) :
            m_io_service(&io_service),
            m_timer(io_service),
            m_socket(io_service),
            m_recv_buf(),
            m_receive_successful(false),
            m_server_name()
        {}

        const ServerList& Servers() const
        { return m_servers; }

        void DiscoverServers() {
            using namespace boost::asio::ip;
            udp::resolver resolver(*m_io_service);
            udp::resolver::query query(udp::v4(), "255.255.255.255",
                                       std::to_string(Networking::DiscoveryPort()),
                                       resolver_query_base::address_configured |
                                       resolver_query_base::numeric_service);
            udp::resolver::iterator end_it;
            for (udp::resolver::iterator it = resolver.resolve(query); it != end_it; ++it) {
                udp::endpoint receiver_endpoint = *it;

                m_socket.close();
                m_socket.open(udp::v4());
                m_socket.set_option(boost::asio::socket_base::broadcast(true));

                m_socket.send_to(boost::asio::buffer(DISCOVERY_QUESTION),
                                 receiver_endpoint);

                m_socket.async_receive_from(
                    boost::asio::buffer(m_recv_buf),
                    m_sender_endpoint,
                    boost::bind(&ServerDiscoverer::HandleReceive,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
                m_timer.expires_from_now(std::chrono::seconds(2));
                m_timer.async_wait(boost::bind(&ServerDiscoverer::CloseSocket, this));
                m_io_service->run();
                m_io_service->reset();
                if (m_receive_successful) {
                    boost::asio::ip::address address = m_server_name == "localhost" ?
                        boost::asio::ip::address::from_string("127.0.0.1") :
                        m_sender_endpoint.address();
                    m_servers.push_back({address, m_server_name});
                }
                m_receive_successful = false;
                m_server_name = "";
            }
        }

    private:
        void HandleReceive(const boost::system::error_code& error, std::size_t length) {
            if (error == boost::asio::error::message_size) {
                m_socket.async_receive_from(
                    boost::asio::buffer(m_recv_buf),
                    m_sender_endpoint,
                    boost::bind(&ServerDiscoverer::HandleReceive,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
            } else if (!error) {
                std::string buffer_string(m_recv_buf.begin(), m_recv_buf.begin() + length);
                if (boost::algorithm::starts_with(buffer_string, DISCOVERY_ANSWER)) {
                    m_receive_successful = true;
                    m_server_name =
                        boost::algorithm::erase_first_copy(buffer_string, DISCOVERY_ANSWER);
                    if (m_server_name == boost::asio::ip::host_name())
                        m_server_name = "localhost";
                    m_timer.cancel();
                    m_socket.close();
                }
            }
        }

        void CloseSocket()
        { m_socket.close(); }

        boost::asio::io_service*       m_io_service;
        boost::asio::basic_waitable_timer<std::chrono::high_resolution_clock> m_timer;
        boost::asio::ip::udp::socket   m_socket;

        std::array<char, 1024> m_recv_buf;

        boost::asio::ip::udp::endpoint m_sender_endpoint;
        bool                           m_receive_successful;
        std::string                    m_server_name;
        ServerList                     m_servers;
    };
}

////////////////////////////////////////////////
// ClientNetworking
////////////////////////////////////////////////
ClientNetworking::ClientNetworking() :
    m_player_id(Networking::INVALID_PLAYER_ID),
    m_host_player_id(Networking::INVALID_PLAYER_ID),
    m_io_service(),
    m_socket(m_io_service),
    m_incoming_messages(m_mutex),
    m_connected(false),
    m_cancel_retries(false)
{}

bool ClientNetworking::Connected() const {
    boost::mutex::scoped_lock lock(m_mutex);
    return m_connected;
}

bool ClientNetworking::MessageAvailable() const
{ return !m_incoming_messages.Empty(); }

int ClientNetworking::PlayerID() const
{ return m_player_id; }

int ClientNetworking::HostPlayerID() const
{ return m_host_player_id; }

bool ClientNetworking::PlayerIsHost(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return false;
    return player_id == m_host_player_id;
}

ClientNetworking::ServerList ClientNetworking::DiscoverLANServers() {
    if (!Connected())
        return ServerList();
    ServerDiscoverer discoverer(m_io_service);
    discoverer.DiscoverServers();
    return discoverer.Servers();
}

bool ClientNetworking::ConnectToServer(
    const std::string& ip_address,
    std::chrono::milliseconds timeout/* = std::chrono::seconds(10)*/,
    bool verbose)
{
    if (verbose)
        DebugLogger() << "ClientNetworking::ConnectToServer : attempting to connect to server at " << ip_address;

    using namespace boost::asio::ip;
    tcp::resolver resolver(m_io_service);
    tcp::resolver::query query(tcp::v4(), ip_address,
                               std::to_string(Networking::MessagePort()),
                               boost::asio::ip::resolver_query_base::numeric_service);

    tcp::resolver::iterator end_it;

    try {
        for (tcp::resolver::iterator it = resolver.resolve(query); it != end_it; ++it) {
            m_socket.close();
            boost::asio::basic_waitable_timer<std::chrono::high_resolution_clock> timer(m_io_service);

            m_socket.async_connect(*it, boost::bind(&ClientNetworking::HandleConnection, this,
                                                    &it,
                                                    &timer,
                                                    boost::asio::placeholders::error));
            timer.expires_from_now(timeout);
            timer.async_wait(boost::bind(&ClientNetworking::CancelRetries, this));
            m_cancel_retries = false;
            m_io_service.run();
            m_io_service.reset();

            if (Connected()) {
                DebugLogger() << "tcp::resolver::iterator host_name: " << it->host_name()
                              << "  address: " << it->endpoint().address()
                              << "  port: " << it->endpoint().port();

                DebugLogger() << "ClientNetworking::ConnectToServer : connected to server";
                if (GetOptionsDB().Get<bool>("binary-serialization"))
                    DebugLogger() << "ClientNetworking::ConnectToServer : this client using binary serialization.";
                else
                    DebugLogger() << "ClientNetworking::ConnectToServer : this client using xml serialization.";
                m_socket.set_option(boost::asio::socket_base::linger(true, SOCKET_LINGER_TIME));
                m_socket.set_option(boost::asio::socket_base::keep_alive(true));
                DebugLogger() << "ClientNetworking::ConnectToServer : starting networking thread";
                boost::thread(boost::bind(&ClientNetworking::NetworkingThread, this));
                break;
            } else if (verbose) {
                DebugLogger() << "tcp::resolver::iterator host_name: " << it->host_name()
                              << "  address: " << it->endpoint().address()
                              << "  port: " << it->endpoint().port();
                DebugLogger() << "ClientNetworking::ConnectToServer : no connection yet...";
            }
        }
        if (!Connected() && verbose)
            DebugLogger() << "ClientNetworking::ConnectToServer : failed to connect to server (no exceptions)";

    } catch (const std::exception& e) {
        ErrorLogger() << "ClientNetworking::ConnectToServer unable to connect to server at "
                      << ip_address << " due to exception: " << e.what();
    }
    return Connected();
}

bool ClientNetworking::ConnectToLocalHostServer(
    std::chrono::milliseconds timeout/* = std::chrono::seconds(10)*/,
    bool verbose)
{
    bool retval = false;
#if FREEORION_WIN32
    try {
#endif
        retval = ConnectToServer("127.0.0.1", timeout, verbose);
#if FREEORION_WIN32
    } catch (const boost::system::system_error& e) {
        if (e.code().value() != WSAEADDRNOTAVAIL)
            throw;
    }
#endif
    return retval;
}

void ClientNetworking::DisconnectFromServer() {
    if (Connected())
        m_io_service.post(boost::bind(&ClientNetworking::DisconnectFromServerImpl, this));
    // HACK! wait a bit for the disconnect to occur
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void ClientNetworking::SetPlayerID(int player_id) {
    DebugLogger() << "ClientNetworking::SetPlayerID: player id set to: " << player_id;
    m_player_id = player_id;
}

void ClientNetworking::SetHostPlayerID(int host_player_id)
{ m_host_player_id = host_player_id; }

void ClientNetworking::SendMessage(Message message) {
    if (!Connected()) {
        ErrorLogger() << "ClientNetworking::SendMessage can't send message when not connected";
        return;
    }
    if (TRACE_EXECUTION)
        DebugLogger() << "ClientNetworking::SendMessage() : sending message " << message;
    m_io_service.post(boost::bind(&ClientNetworking::SendMessageImpl, this, message));
}

void ClientNetworking::GetMessage(Message& message) {
    if (!MessageAvailable()) {
        ErrorLogger() << "ClientNetworking::GetMessage can't get message if none available";
        return;
    }
    m_incoming_messages.PopFront(message);
    if (TRACE_EXECUTION)
        DebugLogger() << "ClientNetworking::GetMessage() : received message "
                      << message;
}

void ClientNetworking::SendSynchronousMessage(Message message, Message& response_message) {
    if (TRACE_EXECUTION)
        DebugLogger() << "ClientNetworking::SendSynchronousMessage : sending message "
                               << message;
    SendMessage(message);
    // note that this is a blocking operation
    m_incoming_messages.EraseFirstSynchronousResponse(response_message);
    if (TRACE_EXECUTION)
        DebugLogger() << "ClientNetworking::SendSynchronousMessage : received "
                               << "response message " << response_message;
}

void ClientNetworking::HandleConnection(tcp::resolver::iterator* it,
                                        boost::asio::basic_waitable_timer<std::chrono::high_resolution_clock>* timer,
                                        const boost::system::error_code& error)
{
    if (error) {
        if (!m_cancel_retries) {
            if (TRACE_EXECUTION)
                DebugLogger() << "ClientNetworking::HandleConnection : connection error ... retrying";
            m_socket.async_connect(**it, boost::bind(&ClientNetworking::HandleConnection, this,
                                                     it,
                                                     timer,
                                                     boost::asio::placeholders::error));
        }
    } else {
        if (TRACE_EXECUTION)
            DebugLogger() << "ClientNetworking::HandleConnection : connected";
        timer->cancel();
        boost::mutex::scoped_lock lock(m_mutex);
        m_connected = true;
    }
}

void ClientNetworking::CancelRetries()
{ m_cancel_retries = true; }

void ClientNetworking::HandleException(const boost::system::system_error& error) {
    if (error.code() == boost::asio::error::eof ||
        error.code() == boost::asio::error::connection_reset ||
        error.code() == boost::asio::error::operation_aborted)
    {
        DebugLogger() << "ClientNetworking::NetworkingThread() : Networking thread will be terminated "
                      << "due to disconnect exception \"" << error.what() << "\"";
    } else {
        ErrorLogger() << "ClientNetworking::NetworkingThread() : Networking thread will be terminated "
                      << "due to unhandled exception \"" << error.what() << "\"";
    }
}

void ClientNetworking::NetworkingThread() {
    try {
        if (!m_outgoing_messages.empty())
            AsyncWriteMessage();
        AsyncReadMessage();
        while (1) {
            try {
                m_io_service.run();
                break;
            } catch (const boost::system::system_error& error) {
                HandleException(error);
                break;
            }
        }
    } catch (const boost::system::system_error& error) {
        HandleException(error);
    }
    m_incoming_messages.Clear();
    m_outgoing_messages.clear();
    m_io_service.reset();
    boost::mutex::scoped_lock lock(m_mutex);
    m_connected = false;
    if (TRACE_EXECUTION)
        DebugLogger() << "ClientNetworking::NetworkingThread() : Networking thread terminated.";
}

void ClientNetworking::HandleMessageBodyRead(boost::system::error_code error,
                                             std::size_t bytes_transferred)
{
    if (error) {
        throw boost::system::system_error(error);
    } else {
        assert(static_cast<int>(bytes_transferred) <= m_incoming_header[4]);
        if (static_cast<int>(bytes_transferred) == m_incoming_header[4]) {
            m_incoming_messages.PushBack(m_incoming_message);
            AsyncReadMessage();
        }
    }
}

void ClientNetworking::HandleMessageHeaderRead(boost::system::error_code error, std::size_t bytes_transferred) {
    if (error) {
        throw boost::system::system_error(error);
        return;
    }
    assert(bytes_transferred <= Message::HeaderBufferSize);
    if (bytes_transferred != Message::HeaderBufferSize)
        return;

    BufferToHeader(m_incoming_header, m_incoming_message);
    m_incoming_message.Resize(m_incoming_header[4]);
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_incoming_message.Data(), m_incoming_message.Size()),
        boost::bind(&ClientNetworking::HandleMessageBodyRead,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void ClientNetworking::AsyncReadMessage() {
    boost::asio::async_read(m_socket, boost::asio::buffer(m_incoming_header),
                            boost::bind(&ClientNetworking::HandleMessageHeaderRead, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void ClientNetworking::HandleMessageWrite(boost::system::error_code error, std::size_t bytes_transferred) {
    if (error) {
        throw boost::system::system_error(error);
        return;
    }

    assert(static_cast<int>(bytes_transferred) <= static_cast<int>(Message::HeaderBufferSize) + m_outgoing_header[4]);
    if (static_cast<int>(bytes_transferred) != static_cast<int>(Message::HeaderBufferSize) + m_outgoing_header[4])
        return;

    m_outgoing_messages.pop_front();
    if (!m_outgoing_messages.empty())
        AsyncWriteMessage();
}

void ClientNetworking::AsyncWriteMessage() {
    HeaderToBuffer(m_outgoing_messages.front(), m_outgoing_header);
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(m_outgoing_header));
    buffers.push_back(boost::asio::buffer(m_outgoing_messages.front().Data(),
                                          m_outgoing_messages.front().Size()));
    boost::asio::async_write(m_socket, buffers,
                             boost::bind(&ClientNetworking::HandleMessageWrite, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void ClientNetworking::SendMessageImpl(Message message) {
    bool start_write = m_outgoing_messages.empty();
    m_outgoing_messages.push_back(Message());
    swap(m_outgoing_messages.back(), message);
    if (start_write)
        AsyncWriteMessage();
}

void ClientNetworking::DisconnectFromServerImpl()
{ m_socket.close(); }
