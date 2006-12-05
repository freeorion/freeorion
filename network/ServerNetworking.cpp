#include "ServerNetworking.h"

#include "Networking.h"

#include <boost/bind.hpp>
#include <boost/iterator/filter_iterator.hpp>


using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using namespace Networking;

/** A simple server that listens for FreeOrion-server-discovery UDP datagrams on the local network and sends out
    responses to them. */
class DiscoveryServer
{
public:
    DiscoveryServer(boost::asio::io_service& io_service);

private:
    void Listen();
    void HandleReceive(const boost::system::error_code& error);

    boost::asio::ip::udp::socket   m_socket;
    boost::asio::ip::udp::endpoint m_remote_endpoint;
    boost::array<char, 32>         m_recv_buffer;
};

namespace {
    void WriteMessage(boost::asio::ip::tcp::socket& socket, const Message& message)
    {
        int header_buf[5];
        HeaderToBuffer(message, header_buf);
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(header_buf));
        buffers.push_back(boost::asio::buffer(message.Data(), message.Size()));
        boost::asio::write(socket, buffers);
    }

    struct PlayerID
    {
        PlayerID(int id) : m_id(id) {}
        bool operator()(const PlayerConnectionPtr& player_connection)
            { return player_connection->ID(); }
    private:
        int m_id;
    };
}

////////////////////////////////////////////////////////////////////////////////
// PlayerConnection
////////////////////////////////////////////////////////////////////////////////
// static(s)
const int PlayerConnection::INVALID_PLAYER_ID = -1;

PlayerConnection::PlayerConnection(boost::asio::io_service& io_service,
                                   boost::function<void (const Message&, PlayerConnectionPtr)> nonplayer_message_callback,
                                   boost::function<void (const Message&, PlayerConnectionPtr)> player_message_callback,
                                   boost::function<void (PlayerConnectionPtr)> disconnected_callback) :
    m_socket(io_service),
    m_ID(INVALID_PLAYER_ID),
    m_host(false),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{}

PlayerConnection::~PlayerConnection()
{ m_socket.close(); }

bool PlayerConnection::EstablishedPlayer() const
{ return m_ID != INVALID_PLAYER_ID; }

int PlayerConnection::ID() const
{ return m_ID; }

const std::string& PlayerConnection::PlayerName() const
{ return m_player_name; }

bool PlayerConnection::Host() const
{ return m_host; }

void PlayerConnection::Start()
{ AsyncReadMessage(); }

void PlayerConnection::SendMessage(const Message& message)
{ WriteMessage(m_socket, message); }

void PlayerConnection::EstablishPlayer(int id, const std::string& player_name, bool host)
{
    assert(m_ID == INVALID_PLAYER_ID && m_player_name == "");
    assert(0 <= id);
    assert(player_name != "");
    m_ID = id;
    m_player_name = player_name;
    m_host = host;
}

PlayerConnectionPtr PlayerConnection::NewConnection(boost::asio::io_service& io_service,
                                                          boost::function<void (const Message&, PlayerConnectionPtr)> nonplayer_message_callback,
                                                          boost::function<void (const Message&, PlayerConnectionPtr)> player_message_callback,
                                                          boost::function<void (PlayerConnectionPtr)> disconnected_callback)
{ return PlayerConnectionPtr(new PlayerConnection(io_service, nonplayer_message_callback, player_message_callback, disconnected_callback)); }

void PlayerConnection::HandleMessageBodyRead(boost::system::error_code error, std::size_t bytes_transferred)
{
    if (error) {
        if (error == boost::asio::error::eof ||
            error == boost::asio::error::connection_reset)
            m_disconnected_callback(shared_from_this());
        else
            std::cout << "PlayerConnection::HandleMessageBodyRead(): error \"" << error << "\"" << std::endl;
    } else {
        assert(static_cast<int>(bytes_transferred) <= m_incoming_header_buffer[4]);
        if (static_cast<int>(bytes_transferred) == m_incoming_header_buffer[4]) {
            if (EstablishedPlayer())
                m_player_message_callback(m_incoming_message, shared_from_this());
            else
                m_nonplayer_message_callback(m_incoming_message, shared_from_this());
            AsyncReadMessage();
        }
    }
}

void PlayerConnection::HandleMessageHeaderRead(boost::system::error_code error, std::size_t bytes_transferred)
{
    if (error) {
        if (error == boost::asio::error::eof ||
            error == boost::asio::error::connection_reset)
            m_disconnected_callback(shared_from_this());
        else
            std::cout << "PlayerConnection::HandleMessageHeaderRead(): error \"" << error << "\"" << std::endl;
    } else {
        assert(static_cast<int>(bytes_transferred) <= HEADER_SIZE);
        if (static_cast<int>(bytes_transferred) == HEADER_SIZE) {
            BufferToHeader(m_incoming_header_buffer.c_array(), m_incoming_message);
            m_incoming_message.Resize(m_incoming_header_buffer[4]);
            boost::asio::async_read(m_socket, boost::asio::buffer(m_incoming_message.Data(), m_incoming_message.Size()),
                                    boost::bind(&PlayerConnection::HandleMessageBodyRead, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    }
}

void PlayerConnection::AsyncReadMessage()
{
    boost::asio::async_read(m_socket, boost::asio::buffer(m_incoming_header_buffer),
                            boost::bind(&PlayerConnection::HandleMessageHeaderRead, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

////////////////////////////////////////////////////////////////////////////////
// DiscoveryServer
////////////////////////////////////////////////////////////////////////////////
DiscoveryServer::DiscoveryServer(boost::asio::io_service& io_service) :
    m_socket(io_service, udp::endpoint(udp::v4(), DISCOVERY_PORT))
{ Listen(); }

void DiscoveryServer::Listen()
{
    m_socket.async_receive_from(
        boost::asio::buffer(m_recv_buffer), m_remote_endpoint,
        boost::bind(&DiscoveryServer::HandleReceive, this,
                    boost::asio::placeholders::error));
}

void DiscoveryServer::HandleReceive(const boost::system::error_code& error)
{
    if (!error && std::string(m_recv_buffer.begin(), m_recv_buffer.end()) == DISCOVERY_QUESTION)
        m_socket.send_to(boost::asio::buffer(DISCOVERY_ANSWER + boost::asio::ip::host_name()), m_remote_endpoint);
    Listen();
}

////////////////////////////////////////////////////////////////////////////////
// ServerNetworking
////////////////////////////////////////////////////////////////////////////////
bool ServerNetworking::EstablishedPlayer::operator()(const PlayerConnectionPtr& player_connection) const
{ return player_connection->EstablishedPlayer(); }

ServerNetworking::ServerNetworking(boost::asio::io_service& io_service,
                                   boost::function<void (const Message&, PlayerConnectionPtr)> nonplayer_message_callback,
                                   boost::function<void (const Message&, PlayerConnectionPtr)> player_message_callback,
                                   boost::function<void (PlayerConnectionPtr)> disconnected_callback) :
    m_discovery_server(new DiscoveryServer(io_service)),
    m_player_connection_acceptor(io_service),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{ Init(); }

ServerNetworking::~ServerNetworking()
{ delete m_discovery_server; }

ServerNetworking::const_iterator ServerNetworking::GetPlayer(int id) const
{ return std::find_if(begin(), end(), PlayerID(id)); }

bool ServerNetworking::empty() const
{ return m_player_connections.empty(); }

std::size_t ServerNetworking::size() const
{ return m_player_connections.size(); }

ServerNetworking::const_iterator ServerNetworking::begin() const
{ return const_iterator(EstablishedPlayer(), m_player_connections.begin(), m_player_connections.end()); }

ServerNetworking::const_iterator ServerNetworking::end() const
{ return const_iterator(EstablishedPlayer(), m_player_connections.end(), m_player_connections.end()); }

ServerNetworking::const_reverse_iterator ServerNetworking::rbegin() const
{ return const_reverse_iterator(EstablishedPlayer(), m_player_connections.rbegin(), m_player_connections.rend()); }

ServerNetworking::const_reverse_iterator ServerNetworking::rend() const
{ return const_reverse_iterator(EstablishedPlayer(), m_player_connections.rend(), m_player_connections.rend()); }

void ServerNetworking::SendMessage(const Message& message, PlayerConnectionPtr player_connection)
{ player_connection->SendMessage(message); }

void ServerNetworking::SendMessage(const Message& message)
{
    iterator it = GetPlayer(message.ReceivingPlayer());
    assert(it != end());
    (*it)->SendMessage(message);
}

void ServerNetworking::Disconnect(int id)
{
    iterator it = GetPlayer(id);
    assert(it != end());
    Disconnect(*it);
}

void ServerNetworking::Disconnect(PlayerConnectionPtr player_connection)
{ DisconnectImpl(player_connection); }

void ServerNetworking::DisconnectAll()
{ std::for_each(m_player_connections.begin(), m_player_connections.end(), boost::bind(&ServerNetworking::DisconnectImpl, this, _1)); }

ServerNetworking::iterator ServerNetworking::GetPlayer(int id)
{ return std::find_if(begin(), end(), PlayerID(id)); }

ServerNetworking::iterator ServerNetworking::begin()
{ return iterator(EstablishedPlayer(), m_player_connections.begin(), m_player_connections.end()); }

ServerNetworking::iterator ServerNetworking::end()
{ return iterator(EstablishedPlayer(), m_player_connections.end(), m_player_connections.end()); }

ServerNetworking::reverse_iterator ServerNetworking::rbegin()
{ return reverse_iterator(EstablishedPlayer(), m_player_connections.rbegin(), m_player_connections.rend()); }

ServerNetworking::reverse_iterator ServerNetworking::rend()
{ return reverse_iterator(EstablishedPlayer(), m_player_connections.rend(), m_player_connections.rend()); }

void ServerNetworking::Init()
{
    tcp::endpoint endpoint(tcp::v4(), MESSAGE_PORT);
    m_player_connection_acceptor.open(endpoint.protocol());
    m_player_connection_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    m_player_connection_acceptor.set_option(boost::asio::socket_base::linger(true, SOCKET_LINGER_TIME));
    m_player_connection_acceptor.bind(endpoint);
    m_player_connection_acceptor.listen();
    AcceptNextConnection();
}

void ServerNetworking::AcceptNextConnection()
{
    PlayerConnectionPtr next_connection =
        PlayerConnection::NewConnection(m_player_connection_acceptor.io_service(),
                                        m_nonplayer_message_callback, m_player_message_callback, boost::bind(&ServerNetworking::DisconnectImpl, this, _1));
    m_player_connection_acceptor.async_accept(next_connection->m_socket,
                                              boost::bind(&ServerNetworking::AcceptConnection, this,
                                                          next_connection,
                                                          boost::asio::placeholders::error));
}

void ServerNetworking::AcceptConnection(PlayerConnectionPtr player_connection, const boost::system::error_code& error)
{
    if (!error) {
        m_player_connections.insert(player_connection);
        player_connection->Start();
        AcceptNextConnection();
    } else {
        throw error;
    }
}

void ServerNetworking::DisconnectImpl(PlayerConnectionPtr player_connection)
{
    m_player_connections.erase(player_connection);
    m_disconnected_callback(player_connection);
}
