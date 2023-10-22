#ifndef _ServerNetworking_h_
#define _ServerNetworking_h_

#include "../network/Message.h"

#include <boost/asio.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/nil_generator.hpp>

#include <functional>
#include <memory>
#include <queue>
#include <set>
#include <unordered_map>

class DiscoveryServer;
class PlayerConnection;
using PlayerConnectionPtr = std::shared_ptr<PlayerConnection>;
using MessageAndConnectionFn = std::function<void (Message, PlayerConnectionPtr)>;
using ConnectionFn = std::function<void (PlayerConnectionPtr)>;
using NullaryFn = std::function<void ()>;

/** Data associated with cookie */
struct CookieData {
    std::string                 player_name;
    boost::posix_time::ptime    expired;
    const Networking::AuthRoles roles;
    const bool                  authenticated;

    template <typename SS>
    CookieData(SS&& player_name_, boost::posix_time::ptime expired_,
               Networking::AuthRoles roles_, bool authenticated_) :
        player_name(std::forward<SS>(player_name_)),
        expired(expired_),
        roles(roles_),
        authenticated(authenticated_)
    {}
};

struct OutgoingMessage {
    OutgoingMessage(Message&& message, int empire_id, int turn) :
       m_message(std::move(message)),
       m_empire_id(empire_id),
       m_turn(turn)
    {}

    const Message m_message;
    const int m_empire_id;
    const int m_turn;
};

/** Encapsulates the connection to a single player.  This object should have
    nearly the same lifetime as the socket it represents, except that the
    object is constructed just before the socket connection is made.  A
    newly-constructed PlayerConnection has no associated player ID, player
    name, nor host-player status.  Once a PlayerConnection is accepted by the
    server as an actual player in a game, EstablishPlayer() should be called.
    This establishes the aforementioned properties. */
class PlayerConnection :
    public std::enable_shared_from_this<PlayerConnection>
{
public:
    ~PlayerConnection();

    /** Returns true if EstablishPlayer() successfully has been called on this
        connection. */
    [[nodiscard]] bool EstablishedPlayer() const noexcept;

    /** Returns the ID of the player associated with this connection, if
        any. */
    [[nodiscard]] int PlayerID() const noexcept { return m_ID; }

    /** Returns the name of the player associated with this connection, if
        any. */
    [[nodiscard]] const std::string& PlayerName() const noexcept { return m_player_name; }

    /** Returns the type of client associated with this connection (AI client,
      * human client, ...) */
    [[nodiscard]] Networking::ClientType GetClientType() const noexcept { return m_client_type; }

    /** Returns the version string the client provided when joining. */
    [[nodiscard]] const std::string& ClientVersionString() const noexcept { return m_client_version_string; }

    /** Checks if the server will enable binary serialization for this client's connection. */
    [[nodiscard]] bool IsBinarySerializationUsed() const;

    /** Checks if client associated with this connection runs on the same
        physical machine as the server */
    [[nodiscard]] bool IsLocalConnection() const noexcept { return m_is_local_connection; }

    /** Checks if the player is established, has a valid name, id and client type. */
    [[nodiscard]] bool IsEstablished() const noexcept;

    /** Checks if the player was authenticated. */
    [[nodiscard]] bool IsAuthenticated() const noexcept { return m_authenticated; }

    /** Checks if the player has a some role */
    [[nodiscard]] bool HasAuthRole(Networking::RoleType role) const { return m_roles.HasRole(role); }

    /** Get cookie associated with this connection. */
    [[nodiscard]] boost::uuids::uuid Cookie() const noexcept { return m_cookie; }

    /** Get string representation of remote ip address. */
    [[nodiscard]] std::string GetIpAddress() const;

    /** Starts the connection reading incoming messages on its socket. */
    void Start();

    /** Sends \a synchronous message to out on the connection. */
    void SendMessage(const Message& message);
    void SendMessage(const Message& message, int empire_id, int turn);

    /** Set player properties to use them after authentication successed. */
    void AwaitPlayer(Networking::ClientType client_type, std::string client_version_string);

    /** Establishes a connection as a player with a specific name and id.
        This function must only be called once. */
    void EstablishPlayer(int id, std::string player_name, Networking::ClientType client_type,
                         std::string client_version_string);

    /** Sets this connection's client type. Useful for already-connected players
      * changing type such as in the multiplayer lobby. */
    void SetClientType(Networking::ClientType client_type);

    /** Sets authenticated status for connection. */
    void SetAuthenticated();

    void SetAuthRoles(Networking::AuthRoles roles);

    /** Sets or unset authorizaion role and send message to client. */
    void SetAuthRole(Networking::RoleType role, bool value = true);

    /** Sets cookie value to this connection to update expire date. */
    void SetCookie(boost::uuids::uuid cookie) noexcept;

    mutable boost::signals2::signal<void(bool, int, int)> MessageSentSignal;
    mutable boost::signals2::signal<void (const NullaryFn&)> EventSignal;

    PlayerConnection(boost::asio::io_context& io_context,
                     MessageAndConnectionFn nonplayer_message_callback,
                     MessageAndConnectionFn player_message_callback, ConnectionFn disconnected_callback);

private:
    void HandleMessageBodyRead(boost::system::error_code error, std::size_t bytes_transferred);
    void HandleMessageHeaderRead(boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncReadMessage();
    void AsyncWriteMessage();
    static void HandleMessageWrite(PlayerConnectionPtr self,
                                   boost::system::error_code error,
                                   std::size_t bytes_transferred,
                                   int empire_id, int turn);

    /** Places message to the end of sending queue and start asynchronous write if \a message was
        first in the queue. */
    static void SendMessageImpl(PlayerConnectionPtr self, Message message, int empire_id, int turn);
    static void AsyncErrorHandler(PlayerConnectionPtr self, boost::system::error_code handled_error,
                                  boost::system::error_code error);

    boost::asio::io_context&        m_service;
    boost::optional<boost::asio::ip::tcp::socket> m_socket;
    Message::HeaderBuffer           m_incoming_header_buffer = {};
    Message                         m_incoming_message;
    Message::HeaderBuffer           m_outgoing_header = {};
    std::queue<OutgoingMessage>     m_outgoing_messages;
    int                             m_ID = Networking::INVALID_PLAYER_ID;
    std::string                     m_player_name;
    Networking::ClientType          m_client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    std::string                     m_client_version_string;
    Networking::AuthRoles           m_roles;
    boost::uuids::uuid              m_cookie = boost::uuids::nil_uuid();
    bool                            m_new_connection = true;
    bool                            m_authenticated = false;
    bool                            m_valid = true;
    bool                            m_is_local_connection = false;

    MessageAndConnectionFn          m_nonplayer_message_callback;
    MessageAndConnectionFn          m_player_message_callback;
    ConnectionFn                    m_disconnected_callback;

    friend class ServerNetworking;
};


/** Encapsulates the networking facilities of the server.  This class listens
    for incoming UDP LAN server-discovery requests and TCP player connections.
    The server also sends and receives messages over the TCP player
    connections. */
class ServerNetworking {
private:
    static constexpr struct IsEstablishedPlayer {
        [[nodiscard]] bool operator()(const PlayerConnectionPtr& player_connection) const
            noexcept(noexcept(std::declval<PlayerConnectionPtr>()->EstablishedPlayer()))
        { return player_connection && player_connection->EstablishedPlayer(); }
    } is_established_player{};

public:
    using PlayerConnections = std::set<PlayerConnectionPtr>;
    using iterator = PlayerConnections::iterator;
    using const_iterator = PlayerConnections::const_iterator;
    using established_iterator = boost::filter_iterator<IsEstablishedPlayer, iterator>;
    using const_established_iterator= boost::filter_iterator<IsEstablishedPlayer, const_iterator>;

    ServerNetworking(boost::asio::io_context& io_context,
                     MessageAndConnectionFn nonplayer_message_callback,
                     MessageAndConnectionFn player_message_callback,
                     ConnectionFn disconnected_callback);

    bool empty() const noexcept { return m_player_connections.empty(); }

    /** Returns the \a total number of PlayerConnections (not just established ones). */
    std::size_t size() const noexcept { return m_player_connections.size(); }

    /** Returns an iterator to the first PlayerConnection object. */
    const_iterator begin() const noexcept { return m_player_connections.begin(); }

    /** Returns an iterator to the one-past-the-last PlayerConnection object. */
    const_iterator end() const noexcept { return m_player_connections.end(); }

    /** Returns the number of established-player PlayerConnections. */
    std::size_t NumEstablishedPlayers() const;

    /** Returns an iterator to the established PlayerConnection object with ID
        \a id, or established_end() if none is found. */
    const_established_iterator GetPlayer(int id) const;

    /** Returns an iterator to the first \a established PlayerConnection object. */
    const_established_iterator established_begin() const;

    /** Returns an iterator to the one-past-the-last \a established
        PlayerConnection object. */
    const_established_iterator established_end() const;

    /** Returns the ID number for new player, which will be larger than the ID of all the established players. */
    int NewPlayerID() const;

    /** Returns the ID of the host player, or INVALID_PLAYER_ID if there is no host player. */
    int HostPlayerID() const noexcept { return m_host_player_id; }

    /** Returns whether the indicated player ID is the host. */
    bool PlayerIsHost(int player_id) const noexcept;

    /** Returns whether there are any moderators in the game. */
    bool ModeratorsInGame() const noexcept;

    /** Returns whether there no non-expired cookie with this player name. */
    bool IsAvailableNameInCookies(const std::string& player_name) const;

    /** Returns whether player have non-expired cookie with this player name.
      * Fills roles and authentication status on success. */
    bool CheckCookie(boost::uuids::uuid cookie,
                     const std::string& player_name,
                     Networking::AuthRoles& roles,
                     bool& authenticated) const;

    /** Returns count of stored cookies so we don't collide with reserved player names. */
    int GetCookiesSize() const noexcept { return m_cookies.size(); }

    /** Sends a synchronous message \a message to the all established players. */
    void SendMessageAll(const Message& message);

    /** Disconnects the server from player \a id. */
    void Disconnect(int id);

    /** Disconnects the server from the client represented by \a player_connection. */
    void Disconnect(PlayerConnectionPtr player_connection);

    /** Disconnects the server from all clients. */
    void DisconnectAll();

    /** Returns an iterator to the first PlayerConnection object. */
    iterator begin() noexcept { return m_player_connections.begin(); }

    /** Returns an iterator to the one-past-the-last PlayerConnection object. */
    iterator end() noexcept { return m_player_connections.end(); }

    /** Returns an iterator to the established PlayerConnection object with ID
        \a id, or end() if none is found. */
    established_iterator GetPlayer(int id);

    /** Returns an iterator to the first established PlayerConnection
        object. */
    established_iterator established_begin();

    /** Returns an iterator to the one-past-the-last established
        PlayerConnection object. */
    established_iterator established_end();

    /** Dequeues and executes the next event in the queue.  Results in a noop
        if the queue is empty. */
    void HandleNextEvent();

    /** Sets Host player ID. */
    void SetHostPlayerID(int host_player_id) noexcept { m_host_player_id = host_player_id; }

    /** Generate cookies for player's name, roles, and authentication status. */
    boost::uuids::uuid GenerateCookie(std::string player_name,
                                      Networking::AuthRoles roles,
                                      bool authenticated);

    /** Bump cookie's expired date. */
    void UpdateCookie(boost::uuids::uuid cookie);

    /** Clean up expired cookies. */
    void CleanupCookies();

    /** Signal to notify if message sent successfully or failed for empire about turn. */
    mutable boost::signals2::signal<void (bool, int, int)> MessageSentSignal;

private:
    void Init();
    void AcceptNextMessagingConnection();
    void AcceptPlayerMessagingConnection(PlayerConnectionPtr player_connection,
                                         boost::system::error_code error);
    void DisconnectImpl(PlayerConnectionPtr player_connection);
    void EnqueueEvent(NullaryFn fn);

    int m_host_player_id = Networking::INVALID_PLAYER_ID;

    /** A simple server that listens for FreeOrion-server-discovery UDP datagrams
    on the local network and sends out responses to them. */
    class DiscoveryServer {
    public:
        explicit DiscoveryServer(boost::asio::io_context& io_context);

    private:
        void Listen();
        void HandleReceive(boost::system::error_code error);

        boost::asio::ip::udp::socket   m_socket;
        boost::asio::ip::udp::endpoint m_remote_endpoint;

        std::array<char, 1024> m_recv_buffer = {};
    };

    DiscoveryServer m_discovery_server;

#if BOOST_VERSION >= 107000
    boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, boost::asio::io_context::executor_type>
                           m_player_connection_acceptor;
#else
    boost::asio::ip::tcp::acceptor m_player_connection_acceptor;
#endif
    PlayerConnections      m_player_connections;
    std::queue<NullaryFn>  m_event_queue;
    std::unordered_map<boost::uuids::uuid, CookieData, boost::hash<boost::uuids::uuid>> m_cookies;

    MessageAndConnectionFn m_nonplayer_message_callback;
    MessageAndConnectionFn m_player_message_callback;
    ConnectionFn           m_disconnected_callback;
};

#endif
