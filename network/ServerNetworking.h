// -*- C++ -*-
#ifndef _ServerNetworking_h_
#define _ServerNetworking_h_

#include "Message.h"

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

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <set>


class DiscoveryServer;
class Message;
class PlayerConnection;
typedef boost::shared_ptr<PlayerConnection> PlayerConnectionPtr;

/** Encapsulates the networking facilities of the server.  This class listens for incoming UDP LAN server-discovery
    requests and TCP player connections.  The server also sends and receives messages over the TCP player connections.
    Note that all the networking code runs in the main thread.  Note that iterating from begin() to end() will iterate
    over only the established PlayerConnections.  Unestablished ones will be skipped; see PlayerConnection for further
    information on established vs. unestablished player connections. */
class ServerNetworking
{
private:
    typedef std::set<PlayerConnectionPtr> PlayerConnections;
    struct EstablishedPlayer
    { bool operator()(const PlayerConnectionPtr& player_connection) const; };

public:
    typedef std::set<PlayerConnectionPtr>::iterator iterator;
    typedef std::set<PlayerConnectionPtr>::const_iterator const_iterator;
    typedef boost::filter_iterator<EstablishedPlayer, PlayerConnections::iterator> established_iterator;
    typedef boost::filter_iterator<EstablishedPlayer, PlayerConnections::const_iterator> const_established_iterator;

    /** \name Structors */ //@{
    /** Basic ctor. */
    ServerNetworking(boost::asio::io_service& io_service,
                     boost::function<void (Message&, PlayerConnectionPtr)> nonplayer_message_callback,
                     boost::function<void (Message&, PlayerConnectionPtr)> player_message_callback,
                     boost::function<void (PlayerConnectionPtr)> disconnected_callback);

    ~ServerNetworking(); //< Dtor.
    //@}

    /** \name Accessors */ //@{
    bool empty() const;             ///< Returns true if size() == 0.
    std::size_t size() const;       ///< Returns the \a total number of PlayerConnections (not just established ones).
    const_iterator begin() const;   ///< Returns an iterator to the first PlayerConnection object.
    const_iterator end() const;     ///< Returns an iterator to the one-past-the-last PlayerConnection object.
    std::size_t NumPlayers() const; ///< Returns the number of established-player PlayerConnections.
    const_established_iterator GetPlayer(int id) const;   ///< Returns an iterator to the established PlayerConnection object with ID \a id, or end() if none is found.
    const_established_iterator established_begin() const; ///< Returns an iterator to the first \a established PlayerConnection object.
    const_established_iterator established_end() const;   ///< Returns an iterator to the one-past-the-last \a established PlayerConnection object.
    int GreatestPlayerID() const;   ///< Returns the highest player ID of all the established players.
    //@}

    /** \name Mutators */ //@{
    /** Sends message \a message to \a player_connection. */
    void SendMessage(const Message& message, PlayerConnectionPtr player_connection);

    /** Sends message \a message to the player indicated in the message. */
    void SendMessage(const Message& message);

    /** Disconnects the server from player \a id. */
    void Disconnect(int id);

    /** Disconnects the server from the client represented by \a player_connection. */
    void Disconnect(PlayerConnectionPtr player_connection);

    /** Disconnects the server from all clients. */
    void DisconnectAll();

    iterator begin();                         ///< Returns an iterator to the first PlayerConnection object.
    iterator end();                           ///< Returns an iterator to the one-past-the-last PlayerConnection object.
    established_iterator GetPlayer(int id);   ///< Returns an iterator to the established PlayerConnection object with ID \a id, or end() if none is found.
    established_iterator established_begin(); ///< Returns an iterator to the first established PlayerConnection object.
    established_iterator established_end();   ///< Returns an iterator to the one-past-the-last established PlayerConnection object.
    //@}

private:
    void Init();
    void AcceptNextConnection();
    void AcceptConnection(PlayerConnectionPtr player_connection, const boost::system::error_code& error);
    void DisconnectImpl(PlayerConnectionPtr player_connection);

    DiscoveryServer*               m_discovery_server;
    boost::asio::ip::tcp::acceptor m_player_connection_acceptor;
    PlayerConnections              m_player_connections;

    boost::function<void (Message&, PlayerConnectionPtr)> m_nonplayer_message_callback;
    boost::function<void (Message&, PlayerConnectionPtr)> m_player_message_callback;
    boost::function<void (PlayerConnectionPtr)>           m_disconnected_callback;
};

/** Encapsulates the connection to a single player.  This object should have nearly the same lifetime as the socket it
    represents, except that the object is constructed just before the socket connection is made.  A newly-constructed
    PlayerConnection has no associated player ID, player name, nor host-player status.  Once a PlayerConnection is
    accepted by the server as an actual player in a game, EstablishPlayer() should be called.  This establishes the
    aforementioned properties. */
class PlayerConnection :
    public boost::enable_shared_from_this<PlayerConnection>
{
public:
    /** \name Structors */ //@{
    ~PlayerConnection(); ///< Dtor.
    //@}

    /** \name Accessors */ //@{
    bool               EstablishedPlayer() const;           ///< Returns true iff EstablishPlayer() has been called on this connection.
    int                ID() const;                          ///< Returns the ID of the player associated with this connection, if any.
    const std::string& PlayerName() const;                  ///< Returns the name of the player associated with this connection, if any.
    bool               Host() const;                        ///< Returns true iff the player associated with this connection, if any, is the host of the current game.
    //@}

    /** \name Mutators */ //@{
    void               Start();                             ///< Starts the connection reading incoming messages on it socket.
    void               SendMessage(const Message& message); ///< Sends \a message to out on the connection.

    /** Establishes a connection as a player with a specific name and id.  This function must only be called once. */
    void               EstablishPlayer(int id, const std::string& player_name, bool host);
    //@}

    /** Creates a new PlayerConnection and returns it as a shared_ptr. */
    static PlayerConnectionPtr NewConnection(boost::asio::io_service& io_service,
                                             boost::function<void (Message&, PlayerConnectionPtr)> nonplayer_message_callback,
                                             boost::function<void (Message&, PlayerConnectionPtr)> player_message_callback,
                                             boost::function<void (PlayerConnectionPtr)> disconnected_callback);

private:
    typedef boost::array<int, 5> MessageHeaderBuffer;

    PlayerConnection(boost::asio::io_service& io_service,
                     boost::function<void (Message&, PlayerConnectionPtr)> nonplayer_message_callback,
                     boost::function<void (Message&, PlayerConnectionPtr)> player_message_callback,
                     boost::function<void (PlayerConnectionPtr)> disconnected_callback);
    void HandleMessageBodyRead(boost::system::error_code error, std::size_t bytes_transferred);
    void HandleMessageHeaderRead(boost::system::error_code error, std::size_t bytes_transferred);
    void AsyncReadMessage();

    boost::asio::ip::tcp::socket m_socket;
    MessageHeaderBuffer          m_incoming_header_buffer;
    Message                      m_incoming_message;
    int                          m_ID;
    std::string                  m_player_name;
    bool                         m_host;
    bool                         m_new_connection;

    boost::function<void (Message&, PlayerConnectionPtr)> m_nonplayer_message_callback;
    boost::function<void (Message&, PlayerConnectionPtr)> m_player_message_callback;
    boost::function<void (PlayerConnectionPtr)>           m_disconnected_callback;

    enum { HEADER_SIZE = MessageHeaderBuffer::static_size * sizeof(MessageHeaderBuffer::value_type) };

    friend class ServerNetworking;
};

#endif
