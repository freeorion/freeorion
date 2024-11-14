#include "ServerNetworking.h"

#include "ServerApp.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"
#include "../universe/ValueRefs.h"
#include "../parse/Parse.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <thread>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using namespace Networking;

#if BOOST_VERSION < 107600
namespace boost::asio::ip { using port_type = uint_least16_t; }
#endif

namespace {
    DeclareThreadSafeLogger(network);
}

/** A simple server that listens for FreeOrion-server-discovery UDP datagrams
    on the local network and sends out responses to them. */
ServerNetworking::DiscoveryServer::DiscoveryServer(boost::asio::io_context& io_context) :
    m_socket(io_context)
{
    const auto disc_port = static_cast<boost::asio::ip::port_type>(Networking::DiscoveryPort());
    // use a dual stack (ipv6 + ipv4) socket
    udp::endpoint discovery_endpoint(udp::v6(), disc_port);

    if (GetOptionsDB().Get<bool>("singleplayer")) {
        // when hosting a single player game only accept connections from
        // the localhost via the loopback interface instead of the any
        // interface.
        // This should prevent unnecessary triggering of Desktop Firewalls as
        // reported by various users when running single player games.
        discovery_endpoint.address(boost::asio::ip::address_v4::loopback());
    }

    try {
        m_socket = udp::socket(io_context, discovery_endpoint);
    } catch (const std::exception &e) {
        ErrorLogger(network) << "DiscoveryServer cannot open IPv6 socket: " << e.what()
                                << ". Fallback to IPv4";
        discovery_endpoint = udp::endpoint(udp::v4(), disc_port);
        if (GetOptionsDB().Get<bool>("singleplayer"))
            discovery_endpoint.address(boost::asio::ip::address_v4::loopback());

        m_socket = udp::socket(io_context, discovery_endpoint);
    }

    Listen();
}

void ServerNetworking::DiscoveryServer::Listen() {
    m_recv_buffer.fill('\0');
    m_socket.async_receive_from(
        boost::asio::buffer(m_recv_buffer),
        m_remote_endpoint,
        boost::bind(&DiscoveryServer::HandleReceive, this, boost::asio::placeholders::error));
}

void ServerNetworking::DiscoveryServer::HandleReceive(boost::system::error_code error) {
    if (error) {
        ErrorLogger(network) << "DiscoveryServer received and ignored error: " << error
                                << "\nfrom: " << m_remote_endpoint;
        Listen();
        return;
    }

    auto message = std::string(m_recv_buffer.begin(), m_recv_buffer.end());
    message.erase(std::find(message.begin(), message.end(), '\0'), message.end());
    boost::trim(message);

    if (message == DISCOVERY_QUESTION) {
        m_socket.send_to(boost::asio::buffer(DISCOVERY_ANSWER), m_remote_endpoint);
        DebugLogger(network) << "DiscoveryServer received from: " << m_remote_endpoint // operator<< outputs "IP:port"
                             << "\nmessage: " << message
                             << "\nreplied: " << DISCOVERY_ANSWER;
        Listen();
        return;
    }

    DebugLogger(network) << "DiscoveryServer evaluating FOCS expression: " << message;
    std::string reply;
    try {
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        if (parse::int_free_variable(message)) {
            auto value_ref = std::make_unique<ValueRef::Variable<int>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, message);
            reply = std::to_string(value_ref->Eval(context));
            DebugLogger(network) << "DiscoveryServer evaluated expression as integer with result: " << reply;

        } else if (parse::double_free_variable(message)) {
            auto value_ref = std::make_unique<ValueRef::Variable<double>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, message);
            reply = std::to_string(value_ref->Eval(context));
            DebugLogger(network) << "DiscoveryServer evaluated expression as double with result: " << reply;

        } else if (parse::string_free_variable(message)) {
            auto value_ref = std::make_unique<ValueRef::Variable<std::string>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, message);
            reply = value_ref->Eval(context);
            DebugLogger(network) << "DiscoveryServer evaluated expression as string with result: " << reply;

        //} else {
        //    auto value_ref = std::make_unique<ValueRef::Variable<std::vector<std::string>>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, message);
        //    auto result = value_ref->Eval(context);
        //    for (auto entry : result)
        //        reply += entry + "\n";
        //    DebugLogger(network) << "DiscoveryServer evaluated expression as string vector with result: " << reply;

        } else {
            ErrorLogger(network) << "DiscoveryServer couldn't interpret message";
            reply = "FOCS ERROR";
        }
    } catch (...) {
        ErrorLogger(network) << "DiscoveryServer caught exception processing message";
        reply = "EXCEPTION ERROR";
    }

    m_socket.send_to(boost::asio::buffer(reply), m_remote_endpoint);

    Listen();
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
    m_cookie(boost::uuids::nil_uuid()),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{}

namespace {
    struct AsyncCloseClosure {
        AsyncCloseClosure(boost::optional<boost::asio::ip::tcp::socket> &&socket, int id) :
            m_socket(std::move(socket)),
            m_id(id)
        {}

        void operator()() {
            if (m_socket) {
                TraceLogger(network) << "Asynchronously closing socket for player id " << m_id;
                m_socket->close();
                TraceLogger(network) << "Socket closed for player id " << m_id;
            }
        }
    private:
        boost::optional<boost::asio::ip::tcp::socket> m_socket;
        const int m_id;
    };
}

PlayerConnection::~PlayerConnection() {
    boost::system::error_code error;
    m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
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

    std::thread(AsyncCloseClosure(std::move(m_socket), m_ID)).detach();
}

bool PlayerConnection::EstablishedPlayer() const noexcept
{ return m_ID != INVALID_PLAYER_ID; }

void PlayerConnection::Start() {
    try {
        m_is_local_connection = m_socket->remote_endpoint().address().is_loopback();
    } catch (const boost::system::system_error& err) {
        m_is_local_connection = false;
        ErrorLogger(network) << "PlayerConnection::Start remote endpont error: " << err.what();
    }
    AsyncReadMessage();
}

void PlayerConnection::SendMessage(const Message& message)
{ SendMessage(message, ALL_EMPIRES, INVALID_GAME_TURN); }

void PlayerConnection::SendMessage(const Message& message, int empire_id, int turn) {
    if (!m_valid) {
        ErrorLogger(network) << "PlayerConnection::SendMessage can't send message when not transmit connected";
        MessageSentSignal(false, empire_id, turn);
        return;
    }
    boost::asio::post(m_service, boost::bind(&PlayerConnection::SendMessageImpl, shared_from_this(),
                                             message, empire_id, turn));
}

bool PlayerConnection::IsEstablished() const noexcept {
    return (m_ID != INVALID_PLAYER_ID && !m_player_name.empty()
            && m_client_type != Networking::ClientType::INVALID_CLIENT_TYPE);
}

std::string PlayerConnection::GetIpAddress() const {
    if (m_socket) {
        try {
            return m_socket->remote_endpoint().address().to_string();
        } catch (const boost::system::system_error& err) {
            ErrorLogger(network) << "PlayerConnection::GetIpAddress remote endpont error: " << err.what();
        }
    }
    return "";
}

void PlayerConnection::AwaitPlayer(Networking::ClientType client_type, std::string client_version_string) {
    TraceLogger(network) << "PlayerConnection(@ " << this << ")::AwaitPlayer("
                         << client_type << ", " << client_version_string << ")";
    if (m_client_type != Networking::ClientType::INVALID_CLIENT_TYPE) {
        ErrorLogger(network) << "PlayerConnection::AwaitPlayer attempting to re-await an already awaiting connection.";
        return;
    }
    if (client_type == Networking::ClientType::INVALID_CLIENT_TYPE ||
        client_type >= Networking::ClientType::NUM_CLIENT_TYPES)
    {
        ErrorLogger(network) << "PlayerConnection::EstablishPlayer passed invalid client type: " << client_type;
        return;
    }
    m_client_type = client_type;
    m_client_version_string = std::move(client_version_string);
}

void PlayerConnection::EstablishPlayer(int id, std::string player_name, Networking::ClientType client_type,
                                       std::string client_version_string)
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
    if (client_type == Networking::ClientType::INVALID_CLIENT_TYPE ||
        client_type >= Networking::ClientType::NUM_CLIENT_TYPES)
    {
        ErrorLogger(network) << "PlayerConnection::EstablishPlayer passed invalid client type: " << client_type;
        return;
    }
    m_ID = id;
    m_player_name = std::move(player_name);
    m_client_type = client_type;
    m_client_version_string = std::move(client_version_string);
}

void PlayerConnection::SetClientType(Networking::ClientType client_type) {
    m_client_type = client_type;
    if (m_client_type == Networking::ClientType::INVALID_CLIENT_TYPE)
        ErrorLogger(network) << "PlayerConnection client type set to INVALID_CLIENT_TYPE...?";
}

void PlayerConnection::SetAuthenticated() {
    m_authenticated = true;
}

void PlayerConnection::SetAuthRoles(Networking::AuthRoles roles) {
    m_roles = roles;
    SendMessage(SetAuthorizationRolesMessage(m_roles));
}

void PlayerConnection::SetAuthRole(Networking::RoleType role, bool value) {
    m_roles.SetRole(role, value);
    SendMessage(SetAuthorizationRolesMessage(m_roles));
}

void PlayerConnection::SetCookie(boost::uuids::uuid cookie) noexcept
{ m_cookie = cookie; }

bool PlayerConnection::IsBinarySerializationUsed() const {
    return GetOptionsDB().Get<bool>("network.server.binary.enabled")
        && !m_client_version_string.empty()
        && m_client_version_string == FreeOrionVersionString();
}

namespace {
    std::string MessageTypeName(Message::MessageType type) {
        switch (type) {
        case Message::MessageType::UNDEFINED:                return "Undefined";
        case Message::MessageType::DEBUG:                    return "Debug";
        case Message::MessageType::ERROR_MSG:                return "Error";
        case Message::MessageType::HOST_SP_GAME:             return "Host SP Game";
        case Message::MessageType::HOST_MP_GAME:             return "Host MP Game";
        case Message::MessageType::JOIN_GAME:                return "Join Game";
        case Message::MessageType::HOST_ID:                  return "Host ID";
        case Message::MessageType::LOBBY_UPDATE:             return "Lobby Update";
        case Message::MessageType::LOBBY_EXIT:               return "Lobby Exit";
        case Message::MessageType::START_MP_GAME:            return "Start MP Game";
        case Message::MessageType::SAVE_GAME_INITIATE:       return "Save Game";
        case Message::MessageType::SAVE_GAME_COMPLETE:       return "Save Game";
        case Message::MessageType::LOAD_GAME:                return "Load Game";
        case Message::MessageType::GAME_START:               return "Game Start";
        case Message::MessageType::TURN_UPDATE:              return "Turn Update";
        case Message::MessageType::TURN_PARTIAL_UPDATE:      return "Turn Partial Update";
        case Message::MessageType::TURN_ORDERS:              return "Turn Orders";
        case Message::MessageType::TURN_PROGRESS:            return "Turn Progress";
        case Message::MessageType::PLAYER_STATUS:            return "Player Status";
        case Message::MessageType::PLAYER_CHAT:              return "Player Chat";
        case Message::MessageType::DIPLOMACY:                return "Diplomacy";
        case Message::MessageType::DIPLOMATIC_STATUS:        return "Diplomatic Status";
        case Message::MessageType::REQUEST_NEW_OBJECT_ID:    return "Request New Object ID";
        case Message::MessageType::DISPATCH_NEW_OBJECT_ID:   return "Dispatch New Object ID";
        case Message::MessageType::REQUEST_NEW_DESIGN_ID:    return "Request New Design ID";
        case Message::MessageType::DISPATCH_NEW_DESIGN_ID:   return "Dispatch New Design ID";
        case Message::MessageType::END_GAME:                 return "End Game";
        case Message::MessageType::AI_END_GAME_ACK:          return "Acknowledge Shut Down Server";
        case Message::MessageType::MODERATOR_ACTION:         return "Moderator Action";
        case Message::MessageType::SHUT_DOWN_SERVER:         return "Shut Down Server";
        case Message::MessageType::REQUEST_SAVE_PREVIEWS:    return "Request save previews";
        case Message::MessageType::DISPATCH_SAVE_PREVIEWS:   return "Dispatch save previews";
        case Message::MessageType::REQUEST_COMBAT_LOGS:      return "Request combat logs";
        case Message::MessageType::DISPATCH_COMBAT_LOGS:     return "Dispatch combat logs";
        case Message::MessageType::LOGGER_CONFIG:            return "Logger config";
        case Message::MessageType::CHECKSUM:                 return "Checksum";
        case Message::MessageType::AUTH_REQUEST:             return "Authentication request";
        case Message::MessageType::AUTH_RESPONSE:            return "Authentication response";
        case Message::MessageType::CHAT_HISTORY:             return "Chat history";
        case Message::MessageType::SET_AUTH_ROLES:           return "Set authorization roles";
        case Message::MessageType::ELIMINATE_SELF:           return "Eliminate self";
        case Message::MessageType::UNREADY:                  return "Unready";
        default:                                             return std::string("Unknown Type(") + std::to_string(static_cast<int>(type)) + ")";
        };
    }
}

void PlayerConnection::HandleMessageBodyRead(boost::system::error_code error,
                                             std::size_t bytes_transferred)
{
    if (error) {
        if (error == boost::asio::error::eof ||
            error == boost::asio::error::connection_reset)
        {
            ErrorLogger(network) << "PlayerConnection::HandleMessageBodyRead(): "
                                 << "error #" << error.value() << "  category: " << error.category().name() << "  message: " << error.message();
            EventSignal(boost::bind(m_disconnected_callback, shared_from_this()));
        } else {
            ErrorLogger(network) << "PlayerConnection::HandleMessageBodyRead(): "
                                 << "error #" << error.value() << "  category: " << error.category().name() << "  message: " << error.message();
        }
    } else {
        assert(static_cast<int>(bytes_transferred) <= m_incoming_header_buffer[Message::Parts::SIZE]);
        if (static_cast<int>(bytes_transferred) == m_incoming_header_buffer[Message::Parts::SIZE]) {
            if (m_incoming_message.Type() != Message::MessageType::REQUEST_NEW_DESIGN_ID) {   // new design id messages ignored due to log spam
                TraceLogger(network) << "Server received message from player id: " << m_ID
                                     << " of type " << MessageTypeName(m_incoming_message.Type())
                                     << " and size " << m_incoming_message.Size();
                //TraceLogger(network) << "     Full message: " << m_incoming_message;
            }
            if (EstablishedPlayer()) {
                EventSignal(boost::bind(m_player_message_callback, m_incoming_message, shared_from_this()));
            } else {
                EventSignal(boost::bind(m_nonplayer_message_callback, m_incoming_message, shared_from_this()));
            }
            m_incoming_message.Reset();
            AsyncReadMessage();
        }
    }
}

void PlayerConnection::HandleMessageHeaderRead(boost::system::error_code error,
                                               std::size_t bytes_transferred)
{
    if (error) {
        ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead():"
                             << " player ID: " << m_ID
                             << "  name: " << m_player_name
                             << "  client type: " << to_string(m_client_type)
                             << "  client version: " << m_client_version_string
                             << "  authenticated: " << m_authenticated
                             << "  cookie: " << boost::uuids::to_string(m_cookie)
                             << "  valid: " << m_valid
                             << "  roles: " << m_roles.Text()
                             << "  error #: " << error.value()
                             << "  category: " << error.category().name()
                             << "  message: " << error.message();

        // HACK! This entire m_new_connection case should not be needed as far
        // as I can see.  It is here because without it, there are
        // intermittent problems, like the server gets a disconnect event for
        // each connection, just after the connection.  I cannot figure out
        // why this is so, but putting a pause in place seems to at least
        // mask the problem.  For now, this is sufficient, since rapid
        // connects and disconnects are not a priority.
        if (m_new_connection && error != boost::asio::error::eof) {
            ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                 << "new connection ... waiting for 0.5s";
            // wait half a second if the first data read is an error; we
            // probably just need more setup time
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            m_new_connection = false;
            if (m_socket->is_open()) {
                ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                     << "Spurious network error on startup of client.  Retrying read...";
                AsyncReadMessage();
            } else {
                ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                     << "Network connection for client failed on startup.";
            }
        } else {
            if (error == boost::asio::error::eof ||
                error == boost::asio::error::connection_reset ||
                error == boost::asio::error::timed_out)
            {
                ErrorLogger() << "PlayerConnection::HandleMessageHeaderRead(): disconnect callback...";
                EventSignal(boost::bind(m_disconnected_callback, shared_from_this()));
            }
        }

    } else {
        m_new_connection = false;
        assert(bytes_transferred <= Message::HeaderBufferSize);
        if (bytes_transferred == Message::HeaderBufferSize) {
            BufferToHeader(m_incoming_header_buffer, m_incoming_message);
            auto msg_size = m_incoming_header_buffer[Message::Parts::SIZE];
            TraceLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                 << "Server Handling Message maybe allocating buffer of size: " << msg_size;
            if (GetOptionsDB().Get<int>("network.server.client-message-size.max") > 0 &&
                msg_size > GetOptionsDB().Get<int>("network.server.client-message-size.max"))
            {
                ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                     << "too big message " << msg_size << " bytes ";
                boost::system::error_code ignored_error;
                m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_error);
                m_socket->close();
                return;
            }
            try {
                m_incoming_message.Resize(msg_size);
            } catch (const std::exception& e) {
                ErrorLogger(network) << "PlayerConnection::HandleMessageHeaderRead(): "
                                     << "caught exception resizing message buffer to size "
                                     << msg_size << " : " << e.what();
                boost::system::error_code ignored_error;
                m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_error);
                m_socket->close();
                return;
            }
            boost::asio::async_read(
                *m_socket,
                boost::asio::buffer(m_incoming_message.Data(), m_incoming_message.Size()),
                boost::bind(&PlayerConnection::HandleMessageBodyRead, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
        }
    }
}

void PlayerConnection::AsyncReadMessage() {
    boost::asio::async_read(*m_socket, boost::asio::buffer(m_incoming_header_buffer),
                            boost::bind(&PlayerConnection::HandleMessageHeaderRead,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void PlayerConnection::SendMessageImpl(PlayerConnectionPtr self, Message message, int empire_id, int turn) {
    const bool start_write = self->m_outgoing_messages.empty();
    self->m_outgoing_messages.emplace(std::move(message), empire_id, turn);
    if (start_write)
        self->AsyncWriteMessage();
}

void PlayerConnection::AsyncWriteMessage() {
    if (!m_valid) {
        ErrorLogger(network) << "PlayerConnection::AsyncWriteMessage(): player id = " << m_ID
                             << ". Socket is closed. Dropping message.";
        return;
    }
    using boost::asio::async_write;
    using boost::asio::buffer;
    using boost::asio::const_buffer;
    using boost::asio::placeholders::error;
    using boost::asio::placeholders::bytes_transferred;

    HeaderToBuffer(m_outgoing_messages.front().m_message, m_outgoing_header);
    std::array<const_buffer, 2> buffers{
        buffer(m_outgoing_header),
        buffer(m_outgoing_messages.front().m_message.Data(), m_outgoing_messages.front().m_message.Size())
    };
    async_write(*m_socket, buffers,
                boost::bind(&PlayerConnection::HandleMessageWrite, shared_from_this(),
                            error, bytes_transferred,
                            m_outgoing_messages.front().m_empire_id,
                            m_outgoing_messages.front().m_turn));
}

void PlayerConnection::HandleMessageWrite(PlayerConnectionPtr self,
                                          boost::system::error_code error,
                                          std::size_t bytes_transferred,
                                          int empire_id, int turn)
{
    if (error) {
        self->m_valid = false;
        ErrorLogger(network) << "PlayerConnection::AsyncWriteMessage(): player id = " << self->m_ID
                             << " error #" << error.value() << " \"" << error.message() << "\"";
        boost::asio::high_resolution_timer t(self->m_service);
        t.async_wait(boost::bind(&PlayerConnection::AsyncErrorHandler, self, error, boost::asio::placeholders::error));
        self->MessageSentSignal(false, empire_id, turn);
        return;
    }

    if (static_cast<int>(bytes_transferred) !=
        static_cast<int>(Message::HeaderBufferSize) + self->m_outgoing_header[Message::Parts::SIZE])
    { return; }

    self->MessageSentSignal(true, empire_id, turn);
    self->m_outgoing_messages.pop();
    if (!self->m_outgoing_messages.empty())
        self->AsyncWriteMessage();
}

void PlayerConnection::AsyncErrorHandler(PlayerConnectionPtr self,
                                         boost::system::error_code handled_error,
                                         boost::system::error_code error)
{ self->EventSignal(boost::bind(self->m_disconnected_callback, self)); }


////////////////////////////////////////////////////////////////////////////////
// ServerNetworking
////////////////////////////////////////////////////////////////////////////////
ServerNetworking::ServerNetworking(boost::asio::io_context& io_context,
                                   MessageAndConnectionFn nonplayer_message_callback,
                                   MessageAndConnectionFn player_message_callback,
                                   ConnectionFn disconnected_callback) :
    m_discovery_server{io_context},
    m_player_connection_acceptor(io_context),
    m_nonplayer_message_callback(nonplayer_message_callback),
    m_player_message_callback(player_message_callback),
    m_disconnected_callback(disconnected_callback)
{ Init(); }

std::size_t ServerNetworking::NumEstablishedPlayers() const
{ return std::distance(established_begin(), established_end()); }

ServerNetworking::const_established_iterator ServerNetworking::GetPlayer(int id) const
{ return std::find_if(established_begin(), established_end(),
                      [id](const auto& player_con) noexcept { return player_con->PlayerID() == id; });
}

ServerNetworking::const_established_iterator ServerNetworking::established_begin() const {
    return const_established_iterator(is_established_player,
                                      m_player_connections.begin(),
                                      m_player_connections.end());
}

ServerNetworking::const_established_iterator ServerNetworking::established_end() const {
    return const_established_iterator(is_established_player,
                                      m_player_connections.end(),
                                      m_player_connections.end());
}

int ServerNetworking::NewPlayerID() const {
    int biggest_current_player_id(0);
    for (const PlayerConnectionPtr& player : m_player_connections) {
        const int player_id = player->PlayerID();
        if (player_id != INVALID_PLAYER_ID && player_id > biggest_current_player_id)
            biggest_current_player_id = player_id;
    }
    return biggest_current_player_id + 1;
}

bool ServerNetworking::PlayerIsHost(int player_id) const noexcept {
    if (player_id == Networking::INVALID_PLAYER_ID)
        return false;
    return player_id == m_host_player_id;
}

bool ServerNetworking::ModeratorsInGame() const noexcept {
    for (const PlayerConnectionPtr& player : m_player_connections) {
        if (player->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
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

    const auto it = m_cookies.find(cookie);
    if (it != m_cookies.end() && player_name == it->second.player_name) {
        const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        if (it->second.expired >= now) {
            roles = it->second.roles;
            authenticated = it->second.authenticated;
            return true;
        }
    }
    return false;
}

void ServerNetworking::SendMessageAll(const Message& message) {
    for (auto player_it = established_begin(); player_it != established_end(); ++player_it)
        (*player_it)->SendMessage(message);
}

void ServerNetworking::Disconnect(int id) {
    const established_iterator it = GetPlayer(id);
    if (it == established_end()) {
        ErrorLogger(network) << "ServerNetworking::Disconnect couldn't find player with id " << id << " to disconnect.  aborting";
        return;
    }
    const PlayerConnectionPtr player = *it;
    if (player->PlayerID() != id) {
        ErrorLogger(network) << "ServerNetworking::Disconnect got PlayerConnectionPtr with inconsistent player id (" << player->PlayerID() << ") to what was requested (" << id << ")";
        return;
    }
    Disconnect(player);
}

void ServerNetworking::Disconnect(PlayerConnectionPtr player_connection) {
    DebugLogger(network) << "ServerNetworking::Disconnect";
    DisconnectImpl(player_connection);
}

void ServerNetworking::DisconnectAll() {
    DebugLogger(network) << "ServerNetworking::DisconnectAll";
    const auto connections_copy{m_player_connections};
    for (auto& pcon : connections_copy)
        DisconnectImpl(pcon);
}

ServerNetworking::established_iterator ServerNetworking::GetPlayer(int id) {
    return std::find_if(established_begin(), established_end(),
                      [id](const auto& player_con) noexcept { return player_con->PlayerID() == id; });
}

ServerNetworking::established_iterator ServerNetworking::established_begin() {
    return established_iterator(is_established_player,
                                m_player_connections.begin(),
                                m_player_connections.end());
}

ServerNetworking::established_iterator ServerNetworking::established_end() {
    return established_iterator(is_established_player,
                                m_player_connections.end(),
                                m_player_connections.end());
}

void ServerNetworking::HandleNextEvent() {
    if (!m_event_queue.empty()) {
        auto f = std::move(m_event_queue.front());
        m_event_queue.pop();
        f();
    }
}

boost::uuids::uuid ServerNetworking::GenerateCookie(std::string player_name,
                                                    Networking::AuthRoles roles,
                                                    bool authenticated)
{
    boost::uuids::uuid cookie = boost::uuids::random_generator()();
    m_cookies.erase(cookie); // remove previous cookie if exists
    m_cookies.emplace(cookie, CookieData(std::move(player_name),
                                         boost::posix_time::second_clock::local_time() +
                                             boost::posix_time::minutes(GetOptionsDB().Get<int>("network.server.cookies.expire-minutes")),
                                         roles,
                                         authenticated));
    return cookie;
}

void ServerNetworking::UpdateCookie(boost::uuids::uuid cookie) {
    if (cookie.is_nil())
        return;

    const auto it = m_cookies.find(cookie);
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
    for (auto it = established_begin(); it != established_end(); ++it)
        to_delete.erase((*it)->Cookie());

    for (const auto& cookie : to_delete)
        m_cookies.erase(cookie);
}

void ServerNetworking::Init() {
#if defined(FREEORION_LINUX)
    if (GetOptionsDB().Get<int>("network.server.listen.fd") >= 0) {
        try {
            m_player_connection_acceptor.assign(tcp::v6(), GetOptionsDB().Get<int>("network.server.listen.fd"));
        } catch (const std::exception &e) {
            ErrorLogger(network) << "Server cannot assign to IPv6 socket: " << e.what()
                                 << ". Fallback to IPv4";
            m_player_connection_acceptor.assign(tcp::v4(), GetOptionsDB().Get<int>("network.server.listen.fd"));
        }
        AcceptNextMessagingConnection();
        return;
    }
#endif

    // use a dual stack (ipv6 + ipv4) socket
    tcp::endpoint message_endpoint{tcp::v6(), static_cast<uint16_t>(Networking::MessagePort())};

    if (GetOptionsDB().Get<bool>("singleplayer")) {
        // when hosting a single player game only accept connections from
        // the localhost via the loopback interface instead of the any
        // interface.
        // This should prevent unnecessary triggering of Desktop Firewalls as
        // reported by various users when running single player games.
        message_endpoint.address(boost::asio::ip::address_v4::loopback());
    }

    try {
        m_player_connection_acceptor.open(message_endpoint.protocol());
    } catch (const std::exception &e) {
        ErrorLogger(network) << "Server cannot open IPv6 socket: " << e.what()
                             << ". Fallback to IPv4";
        const auto msg_port = static_cast<boost::asio::ip::port_type>(Networking::MessagePort());
        message_endpoint = tcp::endpoint(tcp::v4(), msg_port);
        if (GetOptionsDB().Get<bool>("singleplayer"))
            message_endpoint.address(boost::asio::ip::address_v4::loopback());

        m_player_connection_acceptor.open(message_endpoint.protocol());
    }
    m_player_connection_acceptor.set_option(
        boost::asio::socket_base::reuse_address(true));
    if (message_endpoint.protocol() == boost::asio::ip::tcp::v6())
        m_player_connection_acceptor.set_option(
            boost::asio::ip::v6_only(false));  // may be true by default on some systems
    m_player_connection_acceptor.set_option(
        boost::asio::socket_base::linger(true, SOCKET_LINGER_TIME));
    m_player_connection_acceptor.bind(message_endpoint);
    m_player_connection_acceptor.listen();

    AcceptNextMessagingConnection();
}

void ServerNetworking::AcceptNextMessagingConnection() {
    using boost::placeholders::_1;

    auto next_connection = std::make_shared<PlayerConnection>(
        m_player_connection_acceptor.get_executor().context(),
        m_nonplayer_message_callback,
        m_player_message_callback,
        boost::bind(&ServerNetworking::DisconnectImpl, this, _1));
    next_connection->EventSignal.connect(
        boost::bind(&ServerNetworking::EnqueueEvent, this, _1));
    next_connection->MessageSentSignal.connect(MessageSentSignal);

    m_player_connection_acceptor.async_accept(
        *next_connection->m_socket,
        boost::bind(&ServerNetworking::AcceptPlayerMessagingConnection,
                    this,
                    next_connection,
                    boost::asio::placeholders::error));
}

void ServerNetworking::AcceptPlayerMessagingConnection(PlayerConnectionPtr player_connection,
                                                       boost::system::error_code error)
{
    if (!error) {
        DebugLogger(network) << "ServerNetworking::AcceptPlayerMessagingConnection : connected to new player";
        m_player_connections.insert(player_connection);
        player_connection->Start();
        AcceptNextMessagingConnection();
    } else {
        throw error;
    }
}

void ServerNetworking::DisconnectImpl(PlayerConnectionPtr player_connection) {
    DebugLogger(network) << "ServerNetworking::DisconnectImpl : disconnecting player "
                         << player_connection->PlayerID();
    m_player_connections.erase(player_connection);
    m_disconnected_callback(player_connection);
}

void ServerNetworking::EnqueueEvent(NullaryFn fn)
{ m_event_queue.push(std::move(fn)); }
