#ifndef _ServerNetworking_h_
#define _ServerNetworking_h_

#include "Message.h"

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/functional/hash.hpp>

#include <memory>
#include <queue>
#include <set>
#include <unordered_map>

class DiscoveryServer;
class PlayerConnection;

typedef std::shared_ptr<PlayerConnection> PlayerConnectionPtr;
typedef boost::function<void (Message, PlayerConnectionPtr)> MessageAndConnectionFn;
typedef boost::function<void (PlayerConnectionPtr)> ConnectionFn;
typedef boost::function<void ()> NullaryFn;

/** Data associated with cookie */
struct CookieData {
    std::string                 player_name;
    boost::posix_time::ptime    expired;
    Networking::AuthRoles       roles;
    bool                        authenticated;

    CookieData(const std::string& player_name_,
               const boost::posix_time::ptime& expired_,
               const Networking::AuthRoles& roles_,
               bool authenticated_) :
        player_name(player_name_),
        expired(expired_),
        roles(roles_),
        authenticated(authenticated_)
    {}
};


/** In Boost 1.66, io_service was replaced with a typedef of io_context.
  * That typedef was removed in Boost 1.70 along with other interface changes.
  * This code uses io_context for future compatibility and adds the typedef
  * here for old versions of Boost. */
#if BOOST_VERSION < 106600
namespace boost { namespace asio {
    typedef io_service io_context;
}}
#endif


/** Encapsulates the networking facilities of the server.  This class listens
    for incoming UDP LAN server-discovery requests and TCP player connections.
    The server also sends and receives messages over the TCP player
    connections. */
class ServerNetworking {
private:
    typedef std::set<PlayerConnectionPtr> PlayerConnections;
    struct EstablishedPlayer
    { bool operator()(const PlayerConnectionPtr& player_connection) const; };

public:
    typedef std::set<PlayerConnectionPtr>::iterator                                         iterator;
    typedef std::set<PlayerConnectionPtr>::const_iterator                                   const_iterator;
    typedef boost::filter_iterator<EstablishedPlayer, PlayerConnections::iterator>          established_iterator;
    typedef boost::filter_iterator<EstablishedPlayer, PlayerConnections::const_iterator>    const_established_iterator;

    /** \name Structors */ //@{
    ServerNetworking(boost::asio::io_context& io_context,
                     MessageAndConnectionFn nonplayer_message_callback,
                     MessageAndConnectionFn player_message_callback,
                     ConnectionFn disconnected_callback);

    ~ServerNetworking();
    //@}

    /** \name Accessors */ //@{
    /** Returns true if size() == 0. */
    bool empty() const;

    /** Returns the \a total number of PlayerConnections (not just established
        ones). */
    std::size_t size() const;

    /** Returns an iterator to the first PlayerConnection object. */
    const_iterator begin() const;

    /** Returns an iterator to the one-past-the-last PlayerConnection object. */
    const_iterator end() const;

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
    int HostPlayerID() const;

    /** Returns whether the indicated player ID is the host. */
    bool PlayerIsHost(int player_id) const;

    /** Returns whether there are any moderators in the game. */
    bool ModeratorsInGame() const;

    /** Returns whether there no non-expired cookie with this player name. */
    bool IsAvailableNameInCookies(const std::string& player_name) const;

    /** Returns whether player have non-expired cookie with this player name.
      * Fills roles and authentication status on success. */
    bool CheckCookie(boost::uuids::uuid cookie,
                     const std::string& player_name,
                     Networking::AuthRoles& roles,
                     bool& authenticated) const;

    /** Returns count of stored cookies so we don't collide with reserved player names. */
    int GetCookiesSize() const;
    //@}

    /** \name Mutators */ //@{
    /** Sends a synchronous message \a message to the player indicated in the message and returns true on success. */
    bool SendMessageAll(const Message& message);

    /** Disconnects the server from player \a id. */
    void Disconnect(int id);

    /** Disconnects the server from the client represented by \a player_connection. */
    void Disconnect(PlayerConnectionPtr player_connection);

    /** Disconnects the server from all clients. */
    void DisconnectAll();

    /** Returns an iterator to the first PlayerConnection object. */
    iterator begin();

    /** Returns an iterator to the one-past-the-last PlayerConnection object. */
    iterator end();

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
    void SetHostPlayerID(int host_player_id);

    /** Generate cookies for player's name, roles, and authentication status. */
    boost::uuids::uuid GenerateCookie(const std::string& player_name,
                                      const Networking::AuthRoles& roles,
                                      bool authenticated);

    /** Bump cookie's expired date. */
    void UpdateCookie(boost::uuids::uuid cookie);

    /** Clean up expired cookies. */
    void CleanupCookies();
    //@}

private:
    void Init();
    void AcceptNextConnection();
    void AcceptConnection(PlayerConnectionPtr player_connection,
                          const boost::system::error_code& error);
    void DisconnectImpl(PlayerConnectionPtr player_connection);
    void EnqueueEvent(const NullaryFn& fn);

    int                             m_host_player_id;

    DiscoveryServer*                m_discovery_server;
#if BOOST_VERSION >= 107000
    boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, boost::asio::io_context::executor_type>
                                    m_player_connection_acceptor;
#else
    boost::asio::ip::tcp::acceptor  m_player_connection_acceptor;
#endif
    PlayerConnections               m_player_connections;
    std::queue<NullaryFn>           m_event_queue;
    std::unordered_map<boost::uuids::uuid, CookieData, boost::hash<boost::uuids::uuid>> m_cookies;

    MessageAndConnectionFn          m_nonplayer_message_callback;
    MessageAndConnectionFn          m_player_message_callback;
    ConnectionFn                    m_disconnected_callback;
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
    /** \name Structors */ //@{
    ~PlayerConnection(); ///< Dtor.
    //@}

    /** \name Accessors */ //@{
    /** Returns true if EstablishPlayer() successfully has been called on this
        connection. */
    bool EstablishedPlayer() const;

    /** Returns the ID of the player associated with this connection, if
        any. */
    int PlayerID() const;

    /** Returns the name of the player associated with this connection, if
        any. */
    const std::string& PlayerName() const;

    /** Returns the type of client associated with this connection (AI client,
      * human client, ...) */
    Networking::ClientType GetClientType() const;

    /** Returns the version string the client provided when joining. */
    const std::string& ClientVersionString() const;

    /** Checks if the server will enable binary serialization for this client's connection. */
    bool IsBinarySerializationUsed() const;

    /** Checks if client associated with this connection runs on the same
        physical machine as the server */
    bool IsLocalConnection() const;

    /** Checks if the player is established, has a valid name, id and client type. */
    bool IsEstablished() const;

    /** Checks if the player was authenticated. */
    bool IsAuthenticated() const;

    /** Checks if the player has a some role */
    bool HasAuthRole(Networking::RoleType role) const;

    /** Get cookie associated with this connection. */
    boost::uuids::uuid Cookie() const;
    //@}

    /** \name Mutators */ //@{
    /** Starts the connection reading incoming messages on its socket. */
    void Start();

    /** Sends \a synchronous message to out on the connection and return true on success. */
    bool SendMessage(const Message& message);

    /** Set player properties to use them after authentication successed. */
    void AwaitPlayer(Networking::ClientType client_type,
                     const std::string& client_version_string);

    /** Establishes a connection as a player with a specific name and id.
        This function must only be called once. */
    void EstablishPlayer(int id, const std::string& player_name, Networking::ClientType client_type,
                         const std::string& client_version_string);

    /** Sets this connection's client type. Useful for already-connected players
      * changing type such as in the multiplayer lobby. */
    void SetClientType(Networking::ClientType client_type);

    /** Sets authenticated status for connection. */
    void SetAuthenticated();

    /** Sets authorization roles and send message to client. */
    void SetAuthRoles(const std::initializer_list<Networking::RoleType>& roles);

    void SetAuthRoles(const Networking::AuthRoles& roles);

    /** Sets or unset authorizaion role and send message to client. */
    void SetAuthRole(Networking::RoleType role, bool value = true);

    /** Sets cookie value to this connection to update expire date. */
    void SetCookie(boost::uuids::uuid cookie);
    //@}

    mutable boost::signals2::signal<void (const NullaryFn&)> EventSignal;

    /** Creates a new PlayerConnection and returns it as a shared_ptr. */
    static PlayerConnectionPtr
    NewConnection(boost::asio::io_context& io_context, MessageAndConnectionFn nonplayer_message_callback,
                  MessageAndConnectionFn player_message_callback, ConnectionFn disconnected_callback);

private:

    PlayerConnection(boost::asio::io_context& io_context, MessageAndConnectionFn nonplayer_message_callback,
                     MessageAndConnectionFn player_message_callback, ConnectionFn disconnected_callback);
    void HandleMessageBodyRead(boost::system::error_code error, std::size_t bytes_transferred);
    void HandleMessageHeaderRead(boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncReadMessage();
    bool SyncWriteMessage(const Message& message);
    static void AsyncErrorHandler(PlayerConnectionPtr self, boost::system::error_code handled_error, boost::system::error_code error);

    boost::asio::io_context&        m_service;
    boost::asio::ip::tcp::socket    m_socket;
    Message::HeaderBuffer           m_incoming_header_buffer;
    Message                         m_incoming_message;
    int                             m_ID;
    std::string                     m_player_name;
    bool                            m_new_connection;
    Networking::ClientType          m_client_type;
    std::string                     m_client_version_string;
    bool                            m_authenticated;
    Networking::AuthRoles           m_roles;
    boost::uuids::uuid              m_cookie;

    MessageAndConnectionFn          m_nonplayer_message_callback;
    MessageAndConnectionFn          m_player_message_callback;
    ConnectionFn                    m_disconnected_callback;

    friend class ServerNetworking;
};

#endif
