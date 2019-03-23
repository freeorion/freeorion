#include "ServerNetworking.h"

#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>

#include <thread>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using namespace Networking;

namespace {
    DeclareThreadSafeLogger(network);
}

/** A simple server that listens for FreeOrion-server-discovery UDP datagrams
    on the local network and sends out responses to them. */
class DiscoveryServer {
public:
    DiscoveryServer(boost::asio::io_context& io_context);

private:
    void Listen();
    void HandleReceive(const boost::system::error_code& error);

    boost::asio::ip::udp::socket   m_socket;
    boost::asio::ip::udp::endpoint m_remote_endpoint;

    std::array<char, 32> m_recv_buffer;
};

namespace {
    struct PlayerID {
        PlayerID(int id) :
            m_id(id)
        {}

        bool operator()(const PlayerConnectionPtr& player_connection)
        { return player_connection->PlayerID() == m_id; }

    private:
        int m_id;
    };
}

////////////////////////////////////////////////////////////////////////////////
// PlayerConnection
////////////////////////////////////////////////////////////////////////////////
PlayerConnection::PlayerConnection(boost::asio::io_context& io_context,
                                   MessageAndConnectionFn nonplayer_message_callback,
                                   MessageAndConnectionFn player_message_callback,
                                   ConnectionFn disconnected_callback) :
    m_service(io_context),
    m_socket(io_context),
    m_ID(INVALID_PLAYER_ID),
    m_new_connection(true),
    m_client_type(Networking::INVALID_CLIENT_TYPE),
    m_authenticated(false),
    m_cookie(boost::uuids::nil_uuid()),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{}

PlayerConnection::~PlayerConnection() {
    boost::system::error_code error;
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    if (error && (m_ID != INVALID_PLAYER_ID)) {
        if (error == boost::asio::error::eof)
            TraceLogger(network) << "Player connection disconnected by EOF from client.";
        else if (error == boost::asio::error::connection_reset)
            TraceLogger(network) << "Player connection disconnected, reset by client.";
        else if (error == boost::asio::error::operation_aborted)
            TraceLogger(network) << "Player operation aborted by server.";
        else if (error == boost::asio::error::shut_down)
            TraceLogger(network) << "Player connection shutdown.";
        else if (error == boost::asio::error::connection_aborted)
            TraceLogger(network) << "Player connection closed by server.";
        else if (error == boost::asio::error::not_connected)
            TraceLogger(network) << "Player connection already down.";
        else {

            ErrorLogger(network) << "PlayerConnection::~PlayerConnection: shutdown error #"
                                 << error.value() << " \"" << error.message() << "\""
                                 << " for player id " << m_ID;
        }
    }
    m_socket.close();
}

bool PlayerConnection::EstablishedPlayer() const
{ return m_ID != INVALID_PLAYER_ID; }

int PlayerConnection::PlayerID() const
{ return m_ID; }

const std::string& PlayerConnection::PlayerName() const
{ return m_player_name; }

Networking::ClientType PlayerConnection::GetClientType() const
{ return m_client_type; }

bool PlayerConnection::IsLocalConnection() const
{ return (m_socket.remote_endpoint().address().is_loopback()); }

void PlayerConnection::Start()
{ AsyncReadMessage(); }

bool PlayerConnection::SendMessage(const Message& message) {
    return SyncWriteMessage(message);
}

bool PlayerConnection::IsEstablished() const {
    return (m_ID != INVALID_PLAYER_ID && !m_player_name.empty() && m_client_type != Networking::INVALID_CLIENT_TYPE);
}

bool PlayerConnection::IsAuthenticated() const {
    return m_authenticated;
}

bool PlayerConnection::HasAuthRole(Networking::RoleType role) const {
    return m_roles.HasRole(role);
}

boost::uuids::uuid PlayerConnection::Cookie() const
{ return m_cookie; }

void PlayerConnection::AwaitPlayer(Networking::ClientType client_type,
                                   const std::string& client_version_string)
{
    TraceLogger(network) << "PlayerConnection(@ " << this << ")::AwaitPlayer("
                         << client_type << ", " << client_version_string << ")";
    if (m_client_type != Networking::INVALID_CLIENT_TYPE) {
        ErrorLogger(network) << "PlayerConnection::AwaitPlayer attempting to re-await an already awaiting connection.";
        return;
    }
    if (client_type == Networking::INVALID_CLIENT_TYPE || client_type >= NUM_CLIENT_TYPES) {
        ErrorLogger(network) << "PlayerConnection::EstablishPlayer passed invalid client type: " << client_type;
        return;
    }
    m_client_type = client_type;
    m_client_version_string = client_version_string;
}

void PlayerConnection::EstablishPlayer(int id, const std::string& player_name, Networking::ClientType client_type,
                                       const std::string& client_version_string)
{
    TraceLogger(network) << "PlayerConnection(@ " << this << ")::EstablishPlayer("
                         << id << ", " << player_name << ", "
                         << client_type << ", " << client_version_string << ")";

    // ensure that this connection isn't already established
    if (IsEstablished()) {
        ErrorLogger(network) << "PlayerConnection::EstablishPlayer attempting to re-establish an already established connection.";
        return;
    }

    if (id < 0) {
        ErrorLogger(network) << "PlayerConnection::EstablishPlayer attempting to establish a player with an invalid id: " << id;
        return;
    }
    // TODO (maybe): Verify that no other players have this ID in server networking

    if (player_name.empty()) {
        ErrorLogger(network) << "PlayerConnection::EstablishPlayer attempting to establish a player with an empty name";
        return;
    }
    if (client_type == Networking::INVALID_CLIENT_TYPE || client_type >= NUM_CLIENT_TYPES) {
        ErrorLogger(network) << "PlayerConnection::EstablishPlayer passed invalid client type: " << client_type;
        return;
    }
    m_ID = id;
    m_player_name = player_name;
    m_client_type = client_type;
    m_client_version_string = client_version_string;
}

void PlayerConnection::SetClientType(Networking::ClientType client_type) {
    m_client_type = client_type;
    if (m_client_type == Networking::INVALID_CLIENT_TYPE)
        ErrorLogger(network) << "PlayerConnection client type set to INVALID_CLIENT_TYPE...?";
}

void PlayerConnection::SetAuthenticated() {
    m_authenticated = true;
}

void PlayerConnection::SetAuthRoles(const std::initializer_list<Networking::RoleType>& roles) {
    m_roles = Networking::AuthRoles(roles);
    SendMessage(SetAuthorizationRolesMessage(m_roles));
}

void PlayerConnection::SetAuthRoles(const Networking::AuthRoles& roles) {
    m_roles = roles;
    SendMessage(SetAuthorizationRolesMessage(m_roles));
}

void PlayerConnection::SetAuthRole(Networking::RoleType role, bool value) {
    m_roles.SetRole(role, value);
    SendMessage(SetAuthorizationRolesMessage(m_roles));
}

void PlayerConnection::SetCookie(boost::uuids::uuid cookie)
{ m_cookie = cookie; }

const std::string& PlayerConnection::ClientVersionString() const
{ return m_client_version_string; }

bool PlayerConnection::IsBinarySerializationUsed() const {
    return GetOptionsDB().Get<bool>("network.server.binary.enabled")
        && !m_client_version_string.empty()
        && m_client_version_string == FreeOrionVersionString();
}

PlayerConnectionPtr PlayerConnection::NewConnection(boost::asio::io_context& io_context,
                                                    MessageAndConnectionFn nonplayer_message_callback,
                                                    MessageAndConnectionFn player_message_callback,
                                                    ConnectionFn disconnected_callback)
{
    return PlayerConnectionPtr(
        new PlayerConnection(io_context, nonplayer_message_callback, player_message_callback,
                             disconnected_callback));
}

namespace {
    std::string MessageTypeName(Message::MessageType type) {
        switch (type) {
        case Message::UNDEFINED:                return "Undefined";
        case Message::DEBUG:                    return "Debug";
        case Message::ERROR_MSG:                return "Error";
        case Message::HOST_SP_GAME:             return "Host SP Game";
        case Message::HOST_MP_GAME:             return "Host MP Game";
        case Message::JOIN_GAME:                return "Join Game";
        case Message::HOST_ID:                  return "Host ID";
        case Message::LOBBY_UPDATE:             return "Lobby Update";
        case Message::LOBBY_EXIT:               return "Lobby Exit";
        case Message::START_MP_GAME:            return "Start MP Game";
        case Message::SAVE_GAME_INITIATE:       return "Save Game";
        case Message::SAVE_GAME_COMPLETE:       return "Save Game";
        case Message::LOAD_GAME:                return "Load Game";
        case Message::GAME_START:               return "Game Start";
        case Message::TURN_UPDATE:              return "Turn Update";
        case Message::TURN_PARTIAL_UPDATE:      return "Turn Partial Update";
        case Message::TURN_ORDERS:              return "Turn Orders";
        case Message::TURN_PROGRESS:            return "Turn Progress";
        case Message::PLAYER_STATUS:            return "Player Status";
        case Message::PLAYER_CHAT:              return "Player Chat";
        case Message::DIPLOMACY:                return "Diplomacy";
        case Message::DIPLOMATIC_STATUS:        return "Diplomatic Status";
        case Message::REQUEST_NEW_OBJECT_ID:    return "Request New Object ID";
        case Message::DISPATCH_NEW_OBJECT_ID:   return "Dispatch New Object ID";
        case Message::REQUEST_NEW_DESIGN_ID:    return "Request New Design ID";
        case Message::DISPATCH_NEW_DESIGN_ID:   return "Dispatch New Design ID";
        case Message::END_GAME:                 return "End Game";
        case Message::AI_END_GAME_ACK:          return "Acknowledge Shut Down Server";
        case Message::MODERATOR_ACTION:         return "Moderator Action";
        case Message::SHUT_DOWN_SERVER:         return "Shut Down Server";
        case Message::REQUEST_SAVE_PREVIEWS:    return "Request save previews";
        case Message::DISPATCH_SAVE_PREVIEWS:   return "Dispatch save previews";
        case Message::REQUEST_COMBAT_LOGS:      return "Request combat logs";
        case Message::DISPATCH_COMBAT_LOGS:     return "Dispatch combat logs";
        case Message::LOGGER_CONFIG:            return "Logger config";
        case Message::CHECKSUM:                 return "Checksum";
        case Message::AUTH_REQUEST:             return "Authentication request";
        case Message::AUTH_RESPONSE:            return "Authentication response";
        case Message::CHAT_HISTORY:             return "Chat history";
        case Message::SET_AUTH_ROLES:           return "Set authorization roles";
        case Message::ELIMINATE_SELF:           return "Eliminate self";
        case Message::UNREADY:                  return "Unready";
        default:                                return std::string("Unknown Type(") + std::to_string(static_cast<int>(type)) + ")";
        };
    }
}

void PlayerConnection::HandleMessageBodyRead(boost::system::error_code error,
                                             std::size_t bytes_transferred)
{
    if (error) {
        if (error == boost::asio::error::eof ||
            error == boost::asio::error::connection_reset) {
            ErrorLogger(network) << "PlayerConnection::HandleMessageBodyRead(): "
                                 << "error #" << error.value() << " \"" << error.message() << "\"";
            EventSignal(boost::bind(m_disconnected_callback, shared_from_this()));
        } else {
            ErrorLogger(network) << "PlayerConnection::HandleMessageBodyRead(): "
                                 << "error #" << error.value() << " \"" << error.message() << "\"";
        }
    } else {
        assert(static_cast<int>(bytes_transferred) <= m_incoming_header_buffer[Message::Parts::SIZE]);
        if (static_cast<int>(bytes_transferred) == m_incoming_header_buffer[Message::Parts::SIZE]) {
            if (m_incoming_message.Type() != Message::REQUEST_NEW_DESIGN_ID) {   // new design id messages ignored due to log spam
                TraceLogger(network) << "Server received message from player id: " << m_ID
                                     << " of type " << MessageTypeName(m_incoming_message.Type())
                                     << " and size " << m_incoming_message.Size();
                //TraceLogger(network) << "     Full message: " << m_incoming_message;
            }
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
        // why this is so, but putting a pause in place seems to at least
        // mask the problem.  For now, this is sufficient, since rapid
        // connects and disconnects are not a priority.
        if (m_new_connection && error != boost::asio::error::eof) {
            ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                 << "new connection error #" << error.value() << " \""
                                 << error.message() << "\"" << " waiting for 0.5s";
            // wait half a second if the first data read is an error; we
            // probably just need more setup time
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            m_new_connection = false;
            if (m_socket.is_open()) {
                ErrorLogger(network) << "Spurious network error on startup of client. player id = "
                                     << m_ID << ".  Retrying read...";
                AsyncReadMessage();
            } else {
                ErrorLogger(network) << "Network connection for client failed on startup. "
                                     << "player id = " << m_ID << "";
            }
        } else {
            if (error == boost::asio::error::eof ||
                error == boost::asio::error::connection_reset ||
                error == boost::asio::error::timed_out)
            {
                EventSignal(boost::bind(m_disconnected_callback, shared_from_this()));
            } else {
                ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                     << "error #" << error.value() << " \"" << error.message() << "\"";
            }
        }
    } else {
        m_new_connection = false;
        assert(bytes_transferred <= Message::HeaderBufferSize);
        if (bytes_transferred == Message::HeaderBufferSize) {
            BufferToHeader(m_incoming_header_buffer, m_incoming_message);
            m_incoming_message.Resize(m_incoming_header_buffer[Message::Parts::SIZE]);
            boost::asio::async_read(
                m_socket,
                boost::asio::buffer(m_incoming_message.Data(), m_incoming_message.Size()),
                boost::bind(&PlayerConnection::HandleMessageBodyRead, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
        }
    }
}

void PlayerConnection::AsyncReadMessage() {
    boost::asio::async_read(m_socket, boost::asio::buffer(m_incoming_header_buffer),
                            boost::bind(&PlayerConnection::HandleMessageHeaderRead,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

bool PlayerConnection::SyncWriteMessage(const Message& message) {
    // Synchronously write and asynchronously signal the errors.  This prevents PlayerConnections
    // being removed from the list while iterating to transmit to multiple receivers.
    Message::HeaderBuffer header_buf;
    HeaderToBuffer(message, header_buf);
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(header_buf));
    buffers.push_back(boost::asio::buffer(message.Data(), message.Size()));

    boost::system::error_code error;
    boost::asio::write(m_socket, buffers, error);

    if (error) {
        ErrorLogger(network) << "PlayerConnection::WriteMessage(): player id = " << m_ID
                             << " error #" << error.value() << " \"" << error.message();
        boost::asio::high_resolution_timer t(m_service);
        t.async_wait(boost::bind(&PlayerConnection::AsyncErrorHandler, shared_from_this(), error, boost::asio::placeholders::error));
    }

    return (!error);
}

void PlayerConnection::AsyncErrorHandler(PlayerConnectionPtr self, boost::system::error_code handled_error, boost::system::error_code error) {
    self->EventSignal(boost::bind(self->m_disconnected_callback, self));
}

////////////////////////////////////////////////////////////////////////////////
// DiscoveryServer
////////////////////////////////////////////////////////////////////////////////
DiscoveryServer::DiscoveryServer(boost::asio::io_context& io_context) :
    m_socket(io_context, udp::endpoint(udp::v4(), Networking::DiscoveryPort()))
{ Listen(); }

void DiscoveryServer::Listen() {
    m_socket.async_receive_from(
        boost::asio::buffer(m_recv_buffer), m_remote_endpoint,
        boost::bind(&DiscoveryServer::HandleReceive, this,
                    boost::asio::placeholders::error));
}

void DiscoveryServer::HandleReceive(const boost::system::error_code& error) {
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

ServerNetworking::ServerNetworking(boost::asio::io_context& io_context,
                                   MessageAndConnectionFn nonplayer_message_callback,
                                   MessageAndConnectionFn player_message_callback,
                                   ConnectionFn disconnected_callback) :
    m_host_player_id(Networking::INVALID_PLAYER_ID),
    m_discovery_server(nullptr),
    m_player_connection_acceptor(io_context),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{
    if (!GetOptionsDB().Get<bool>("singleplayer")) {
        // only start discovery service for multiplayer servers.
        m_discovery_server = new DiscoveryServer(io_context);
    }

    Init();
}

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

ServerNetworking::const_established_iterator ServerNetworking::established_begin() const {
    return const_established_iterator(EstablishedPlayer(),
                                      m_player_connections.begin(),
                                      m_player_connections.end());
}

ServerNetworking::const_established_iterator ServerNetworking::established_end() const {
    return const_established_iterator(EstablishedPlayer(),
                                      m_player_connections.end(),
                                      m_player_connections.end());
}

int ServerNetworking::NewPlayerID() const {
    int biggest_current_player_id(0);
    for (const PlayerConnectionPtr player : m_player_connections) {
        int player_id = player->PlayerID();
        if (player_id != INVALID_PLAYER_ID && player_id > biggest_current_player_id)
            biggest_current_player_id = player_id;
    }
    return biggest_current_player_id + 1;
}

int ServerNetworking::HostPlayerID() const
{ return m_host_player_id; }

bool ServerNetworking::PlayerIsHost(int player_id) const {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return false;
    return player_id == m_host_player_id;
}

bool ServerNetworking::ModeratorsInGame() const {
    for (const PlayerConnectionPtr player : m_player_connections) {
        if (player->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
            return true;
    }
    return false;
}

bool ServerNetworking::IsAvailableNameInCookies(const std::string& player_name) const {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    for (const auto& cookie : m_cookies) {
        if (cookie.second.expired >= now && cookie.second.player_name == player_name)
            return false;
    }
    return true;
}

bool ServerNetworking::CheckCookie(boost::uuids::uuid cookie,
                                   const std::string& player_name,
                                   Networking::AuthRoles& roles,
                                   bool& authenticated) const
{
    if (cookie.is_nil())
        return false;

    auto it = m_cookies.find(cookie);
    if (it != m_cookies.end() && player_name == it->second.player_name) {
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        if (it->second.expired >= now) {
            roles = it->second.roles;
            authenticated = it->second.authenticated;
            return true;
        }
    }
    return false;
}

int ServerNetworking::GetCookiesSize() const
{ return m_cookies.size(); }

bool ServerNetworking::SendMessageAll(const Message& message) {
    bool success = true;
    for (auto player_it = established_begin();
        player_it != established_end(); ++player_it)
    {
        success = success && (*player_it)->SendMessage(message);
    }
    return success;
}

void ServerNetworking::Disconnect(int id) {
    established_iterator it = GetPlayer(id);
    if (it == established_end()) {
        ErrorLogger(network) << "ServerNetworking::Disconnect couldn't find player with id " << id << " to disconnect.  aborting";
        return;
    }
    PlayerConnectionPtr player = *it;
    if (player->PlayerID() != id) {
        ErrorLogger(network) << "ServerNetworking::Disconnect got PlayerConnectionPtr with inconsistent player id (" << player->PlayerID() << ") to what was requrested (" << id << ")";
        return;
    }
    Disconnect(player);
}

void ServerNetworking::Disconnect(PlayerConnectionPtr player_connection)
{
    TraceLogger(network) << "ServerNetworking::Disconnect";
    DisconnectImpl(player_connection);
}

void ServerNetworking::DisconnectAll() {
    TraceLogger(network) << "ServerNetworking::DisconnectAll";
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

ServerNetworking::established_iterator ServerNetworking::established_begin() {
    return established_iterator(EstablishedPlayer(),
                                m_player_connections.begin(),
                                m_player_connections.end());
}

ServerNetworking::established_iterator ServerNetworking::established_end() {
    return established_iterator(EstablishedPlayer(),
                                m_player_connections.end(),
                                m_player_connections.end());
}

void ServerNetworking::HandleNextEvent() {
    if (!m_event_queue.empty()) {
        boost::function<void ()> f = m_event_queue.front();
        m_event_queue.pop();
        f();
    }
}

void ServerNetworking::SetHostPlayerID(int host_player_id)
{ m_host_player_id = host_player_id; }

boost::uuids::uuid ServerNetworking::GenerateCookie(const std::string& player_name,
                                                    const Networking::AuthRoles& roles,
                                                    bool authenticated)
{
    boost::uuids::uuid cookie = boost::uuids::random_generator()();
    m_cookies.erase(cookie); // remove previous cookie if exists
    m_cookies.emplace(cookie, CookieData(player_name,
                                         boost::posix_time::second_clock::local_time() +
                                             boost::posix_time::minutes(GetOptionsDB().Get<int>("network.server.cookies.expire-minutes")),
                                         roles,
                                         authenticated));
    return cookie;
}

void ServerNetworking::UpdateCookie(boost::uuids::uuid cookie) {
    if (cookie.is_nil())
        return;

    auto it = m_cookies.find(cookie);
    if (it != m_cookies.end()) {
        it->second.expired = boost::posix_time::second_clock::local_time() +
            boost::posix_time::minutes(GetOptionsDB().Get<int>("network.server.cookies.expire-minutes"));
    }
}

void ServerNetworking::CleanupCookies() {
    std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uuid>> to_delete;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    // clean up expired cookies
    for (const auto& cookie : m_cookies) {
        if (cookie.second.expired < now)
            to_delete.insert(cookie.first);
    }
    // don't clean up cookies from active connections
    for (auto it = established_begin();
        it != established_end(); ++it)
    {
        to_delete.erase((*it)->Cookie());
    }
    for (auto cookie : to_delete)
        m_cookies.erase(cookie);
}

void ServerNetworking::Init() {
    // use a dual stack (ipv6 + ipv4) socket
    tcp::endpoint endpoint(tcp::v6(), Networking::MessagePort());

    if (GetOptionsDB().Get<bool>("singleplayer")) {
        // when hosting a single player game only accept connections from
        // the localhost via the loopback interface instead of the any
        // interface.
        // This should prevent unnecessary triggering of Desktop Firewalls as
        // reported by various users when running single player games.
        endpoint.address(boost::asio::ip::address_v4::loopback());
    }

    m_player_connection_acceptor.open(endpoint.protocol());
    m_player_connection_acceptor.set_option(
        boost::asio::socket_base::reuse_address(true));
    if (endpoint.protocol() == boost::asio::ip::tcp::v6())
        m_player_connection_acceptor.set_option(
            boost::asio::ip::v6_only(false));  // may be true by default on some systems
    m_player_connection_acceptor.set_option(
        boost::asio::socket_base::linger(true, SOCKET_LINGER_TIME));
    m_player_connection_acceptor.bind(endpoint);
    m_player_connection_acceptor.listen();
    AcceptNextConnection();
}

void ServerNetworking::AcceptNextConnection() {
    auto next_connection = PlayerConnection::NewConnection(
#if BOOST_VERSION >= 106600
        m_player_connection_acceptor.get_executor().context(),
#else
        m_player_connection_acceptor.get_io_service(),
#endif
        m_nonplayer_message_callback,
        m_player_message_callback,
        boost::bind(&ServerNetworking::DisconnectImpl, this, _1));
    next_connection->EventSignal.connect(
        boost::bind(&ServerNetworking::EnqueueEvent, this, _1));
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
        TraceLogger(network) << "ServerNetworking::AcceptConnection : connected to new player";
        m_player_connections.insert(player_connection);
        player_connection->Start();
        AcceptNextConnection();
    } else {
        throw error;
    }
}

void ServerNetworking::DisconnectImpl(PlayerConnectionPtr player_connection) {
    TraceLogger(network) << "ServerNetworking::DisconnectImpl : disconnecting player "
                         << player_connection->PlayerID();
    m_player_connections.erase(player_connection);
    m_disconnected_callback(player_connection);
}

void ServerNetworking::EnqueueEvent(const NullaryFn& fn)
{ m_event_queue.push(fn); }
