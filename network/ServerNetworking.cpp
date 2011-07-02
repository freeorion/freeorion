#include "ServerNetworking.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

#include <GG/SignalsAndSlots.h>

#include <boost/iterator/filter_iterator.hpp>


using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using namespace Networking;

namespace {
    const bool TRACE_EXECUTION = true;
}

/** A simple server that listens for FreeOrion-server-discovery UDP datagrams
    on the local network and sends out responses to them. */
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
            { return player_connection->PlayerID() == m_id; }
    private:
        int m_id;
    };
}

////////////////////////////////////////////////////////////////////////////////
// PlayerConnection
////////////////////////////////////////////////////////////////////////////////
PlayerConnection::PlayerConnection(boost::asio::io_service& io_service,
                                   MessageAndConnectionFn nonplayer_message_callback,
                                   MessageAndConnectionFn player_message_callback,
                                   ConnectionFn disconnected_callback) :
    m_socket(io_service),
    m_ID(INVALID_PLAYER_ID),
    m_new_connection(true),
    m_client_type(Networking::INVALID_CLIENT_TYPE),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{}

PlayerConnection::~PlayerConnection()
{ m_socket.close(); }

bool PlayerConnection::EstablishedPlayer() const
{ return m_ID != INVALID_PLAYER_ID; }

int PlayerConnection::PlayerID() const
{ return m_ID; }

const std::string& PlayerConnection::PlayerName() const
{ return m_player_name; }

Networking::ClientType PlayerConnection::GetClientType() const
{ return m_client_type; }

void PlayerConnection::Start()
{ AsyncReadMessage(); }

void PlayerConnection::SendMessage(const Message& message)
{ WriteMessage(m_socket, message); }

void PlayerConnection::EstablishPlayer(int id, const std::string& player_name, Networking::ClientType client_type)
{
    if (TRACE_EXECUTION)
        Logger().debugStream() << "PlayerConnection(@ " << this << ")::EstablishPlayer("
                               << id << ", " << player_name << ", " << client_type << ")";
    // ensure that this connection isn't already established
    if (m_ID != INVALID_PLAYER_ID || !m_player_name.empty() || m_client_type != Networking::INVALID_CLIENT_TYPE) {
        Logger().errorStream() << "PlayerConnection::EstablishPlayer attempting to re-establish an already established connection.";
        return;
    }

    if (id < 0) {
        Logger().errorStream() << "PlayerConnection::EstablishPlayer attempting to establish a player with an invalid id: " << id;
        return;
    }
    // TODO (maybe): Verify that no other players have this ID in server networking

    if (player_name.empty()) {
        Logger().errorStream() << "PlayerConnection::EstablishPlayer attempting to establish a player with an empty name";
        return;
    }
    if (client_type == Networking::INVALID_CLIENT_TYPE || client_type >= NUM_CLIENT_TYPES) {
        Logger().errorStream() << "PlayerConnection::EstablishPlayer passed invalid client type: " << client_type;
        return;
    }
    m_ID = id;
    m_player_name = player_name;
    m_client_type = client_type;
}

void PlayerConnection::SetClientType(Networking::ClientType client_type)
{
    m_client_type = client_type;
    if (m_client_type == Networking::INVALID_CLIENT_TYPE)
        Logger().errorStream() << "PlayerConnection client type set to INVALID_CLIENT_TYPE...?";
}

PlayerConnectionPtr
PlayerConnection::NewConnection(boost::asio::io_service& io_service,
                                MessageAndConnectionFn nonplayer_message_callback,
                                MessageAndConnectionFn player_message_callback,
                                ConnectionFn disconnected_callback)
{
    return PlayerConnectionPtr(
        new PlayerConnection(io_service,
                             nonplayer_message_callback,
                             player_message_callback,
                             disconnected_callback));
}

void PlayerConnection::HandleMessageBodyRead(boost::system::error_code error,
                                             std::size_t bytes_transferred)
{
    if (error) {
        if (error == boost::asio::error::eof ||
            error == boost::asio::error::connection_reset)
            EventSignal(boost::bind(m_disconnected_callback, shared_from_this()));
        else
            Logger().errorStream() << "PlayerConnection::HandleMessageBodyRead(): error \""
                                   << error << "\"";
    } else {
        assert(static_cast<int>(bytes_transferred) <= m_incoming_header_buffer[4]);
        if (static_cast<int>(bytes_transferred) == m_incoming_header_buffer[4]) {
            //if (TRACE_EXECUTION)
                //Logger().debugStream() << "PlayerConnection::HandleMessageBodyRead(): "
                //                       << "received message " << m_incoming_message;
            if (EstablishedPlayer()) {
                EventSignal(boost::bind(m_player_message_callback,
                                        m_incoming_message,
                                        shared_from_this()));
            } else {
                EventSignal(boost::bind(m_nonplayer_message_callback,
                                        m_incoming_message,
                                        shared_from_this()));
            }
            m_incoming_message = Message();
            AsyncReadMessage();
        }
    }
}

void PlayerConnection::HandleMessageHeaderRead(boost::system::error_code error,
                                               std::size_t bytes_transferred)
{
    if (error) {
        // HACK! This entire m_new_connection case should not be needed as far
        // as I can see.  It is here because without it, there are
        // intermittent problems, like the server gets a disconnect event for
        // each connection, just after the connection.  I cannot figure out
        // whay this is so, but putting a pause in place seems to at least
        // mask the problem.  For now, this is sufficient, since rapid
        // connects and disconnects are not a priority.
        if (m_new_connection) {
            // wait half a second if the first data read is an error; we
            // probably just need more setup time
            Sleep(500);
        } else {
            if (error == boost::asio::error::eof ||
                error == boost::asio::error::connection_reset)
                EventSignal(boost::bind(m_disconnected_callback, shared_from_this()));
            else
                Logger().errorStream() << "PlayerConnection::HandleMessageHeaderRead(): "
                                       << "error \"" << error << "\"";
        }
    } else {
        m_new_connection = false;
        assert(static_cast<int>(bytes_transferred) <= HEADER_SIZE);
        if (static_cast<int>(bytes_transferred) == HEADER_SIZE) {
            BufferToHeader(m_incoming_header_buffer.c_array(), m_incoming_message);
            m_incoming_message.Resize(m_incoming_header_buffer[4]);
            boost::asio::async_read(
                m_socket,
                boost::asio::buffer(m_incoming_message.Data(), m_incoming_message.Size()),
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
    if (!error &&
        std::string(m_recv_buffer.begin(), m_recv_buffer.end()) == DISCOVERY_QUESTION) {
        m_socket.send_to(
            boost::asio::buffer(DISCOVERY_ANSWER + boost::asio::ip::host_name()),
            m_remote_endpoint);
    }
    Listen();
}

////////////////////////////////////////////////////////////////////////////////
// ServerNetworking
////////////////////////////////////////////////////////////////////////////////
bool ServerNetworking::EstablishedPlayer::operator()(
    const PlayerConnectionPtr& player_connection) const
{ return player_connection->EstablishedPlayer(); }

ServerNetworking::ServerNetworking(boost::asio::io_service& io_service,
                                   MessageAndConnectionFn nonplayer_message_callback,
                                   MessageAndConnectionFn player_message_callback,
                                   ConnectionFn disconnected_callback) :
    m_host_player_id(Networking::INVALID_PLAYER_ID),
    m_discovery_server(new DiscoveryServer(io_service)),
    m_player_connection_acceptor(io_service),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{ Init(); }

ServerNetworking::~ServerNetworking()
{ delete m_discovery_server; }

bool ServerNetworking::empty() const
{ return m_player_connections.empty(); }

std::size_t ServerNetworking::size() const
{ return m_player_connections.size(); }

ServerNetworking::const_iterator ServerNetworking::begin() const
{ return m_player_connections.begin(); }

ServerNetworking::const_iterator ServerNetworking::end() const 
{ return m_player_connections.end(); }

std::size_t ServerNetworking::NumEstablishedPlayers() const
{ return std::distance(established_begin(), established_end()); }

ServerNetworking::const_established_iterator ServerNetworking::GetPlayer(int id) const
{ return std::find_if(established_begin(), established_end(), PlayerID(id)); }

ServerNetworking::const_established_iterator ServerNetworking::established_begin() const
{
    return const_established_iterator(EstablishedPlayer(),
                                      m_player_connections.begin(),
                                      m_player_connections.end());
}

ServerNetworking::const_established_iterator ServerNetworking::established_end() const
{
    return const_established_iterator(EstablishedPlayer(),
                                      m_player_connections.end(),
                                      m_player_connections.end());
}

int ServerNetworking::NewPlayerID() const
{
    int biggest_current_player_id(0);
    for (PlayerConnections::const_iterator it = m_player_connections.begin(); it != m_player_connections.end(); ++it) {
        int player_id = (*it)->PlayerID();
        if (player_id != INVALID_PLAYER_ID && player_id > biggest_current_player_id)
            biggest_current_player_id = (*it)->PlayerID();
    }
    return biggest_current_player_id + 1;
}

int ServerNetworking::HostPlayerID() const
{
    return m_host_player_id;
}

bool ServerNetworking::PlayerIsHost(int player_id) const
{
    if (player_id == Networking::INVALID_PLAYER_ID)
        return false;
    return player_id == m_host_player_id;
}

void ServerNetworking::SendMessage(const Message& message,
                                   PlayerConnectionPtr player_connection)
{
    //if (TRACE_EXECUTION)
    //    Logger().debugStream() << "ServerNetworking::SendMessage : sending message "
    //                           << message;
    player_connection->SendMessage(message);
}

void ServerNetworking::SendMessage(const Message& message)
{
    established_iterator it = GetPlayer(message.ReceivingPlayer());
    if (it == established_end()) {
        Logger().errorStream() << "ServerNetworking::SendMessage couldn't find player with id " << message.ReceivingPlayer() << " to disconnect.  aborting";
        return;
    }
    PlayerConnectionPtr player = *it;
    if (player->PlayerID() != message.ReceivingPlayer()) {
        Logger().errorStream() << "ServerNetworking::SendMessage got PlayerConnectionPtr with inconsistent player id (" << message.ReceivingPlayer() << ") to what was requrested (" << message.ReceivingPlayer() << ")";
        return;
    }

    //if (TRACE_EXECUTION)
    //    Logger().debugStream() << "ServerNetworking::SendMessage : sending message "
    //                           << message;
    player->SendMessage(message);
}

void ServerNetworking::Disconnect(int id)
{
    established_iterator it = GetPlayer(id);
    if (it == established_end()) {
        Logger().errorStream() << "ServerNetworking::Disconnect couldn't find player with id " << id << " to disconnect.  aborting";
        return;
    }
    PlayerConnectionPtr player = *it;
    if (player->PlayerID() != id) {
        Logger().errorStream() << "ServerNetworking::Disconnect got PlayerConnectionPtr with inconsistent player id (" << player->PlayerID() << ") to what was requrested (" << id << ")";
        return;
    }
    Disconnect(player);
}

void ServerNetworking::Disconnect(PlayerConnectionPtr player_connection)
{ DisconnectImpl(player_connection); }

void ServerNetworking::DisconnectAll()
{
    for (const_iterator it = m_player_connections.begin();
         it != m_player_connections.end(); ) {
        PlayerConnectionPtr player_connection = *it++;
        DisconnectImpl(player_connection);
    }
}

ServerNetworking::iterator ServerNetworking::begin()
{ return m_player_connections.begin(); }

ServerNetworking::iterator ServerNetworking::end()
{ return m_player_connections.end(); }

ServerNetworking::established_iterator ServerNetworking::GetPlayer(int id)
{ return std::find_if(established_begin(), established_end(), PlayerID(id)); }

ServerNetworking::established_iterator ServerNetworking::established_begin()
{
    return established_iterator(EstablishedPlayer(),
                                m_player_connections.begin(),
                                m_player_connections.end());
}

ServerNetworking::established_iterator ServerNetworking::established_end()
{
    return established_iterator(EstablishedPlayer(),
                                m_player_connections.end(),
                                m_player_connections.end());
}

void ServerNetworking::HandleNextEvent()
{
    if (!m_event_queue.empty()) {
        boost::function<void ()> f = m_event_queue.front();
        m_event_queue.pop();
        f();
    }
}

void ServerNetworking::SetHostPlayerID(int host_player_id)
{
    m_host_player_id = host_player_id;
}

void ServerNetworking::Init()
{
    tcp::endpoint endpoint(tcp::v4(), MESSAGE_PORT);
    m_player_connection_acceptor.open(endpoint.protocol());
    m_player_connection_acceptor.set_option(
        boost::asio::socket_base::reuse_address(true));
    m_player_connection_acceptor.set_option(
        boost::asio::socket_base::linger(true, SOCKET_LINGER_TIME));
    m_player_connection_acceptor.bind(endpoint);
    m_player_connection_acceptor.listen();
    AcceptNextConnection();
}

void ServerNetworking::AcceptNextConnection()
{
    PlayerConnectionPtr next_connection =
        PlayerConnection::NewConnection(
            m_player_connection_acceptor.get_io_service(),
            m_nonplayer_message_callback,
            m_player_message_callback,
            boost::bind(&ServerNetworking::DisconnectImpl, this, _1));
    GG::Connect(next_connection->EventSignal, &ServerNetworking::EnqueueEvent, this);
    m_player_connection_acceptor.async_accept(
        next_connection->m_socket,
        boost::bind(&ServerNetworking::AcceptConnection,
                    this,
                    next_connection,
                    boost::asio::placeholders::error));
}

void ServerNetworking::AcceptConnection(PlayerConnectionPtr player_connection,
                                        const boost::system::error_code& error)
{
    if (!error) {
        if (TRACE_EXECUTION)
            Logger().debugStream() << "ServerNetworking::AcceptConnection : connected to "
                                   << "new player";
        m_player_connections.insert(player_connection);
        player_connection->Start();
        AcceptNextConnection();
    } else {
        throw error;
    }
}

void ServerNetworking::DisconnectImpl(PlayerConnectionPtr player_connection)
{
    if (TRACE_EXECUTION)
        Logger().debugStream() << "ServerNetworking::DisconnectImpl : disconnecting player "
                               << player_connection->PlayerID();
    m_player_connections.erase(player_connection);
    m_disconnected_callback(player_connection);
}

void ServerNetworking::EnqueueEvent(const NullaryFn& fn)
{ m_event_queue.push(fn); }
