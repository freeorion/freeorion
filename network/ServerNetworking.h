// -*- C++ -*-
#ifndef _ServerNetworking_h_
#define _ServerNetworking_h_

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/iterator/filter_iterator.hpp>


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
    typedef boost::filter_iterator<EstablishedPlayer, PlayerConnections::iterator> iterator;
    typedef boost::filter_iterator<EstablishedPlayer, PlayerConnections::const_iterator> const_iterator;

    /** \name Structors */ //@{
    /** Basic ctor. */
    ServerNetworking(boost::asio::io_service& io_service,
                     boost::function<void (const Message&, PlayerConnectionPtr)> nonplayer_message_callback,
                     boost::function<void (const Message&, PlayerConnectionPtr)> player_message_callback,
                     boost::function<void (PlayerConnectionPtr)> disconnected_callback);

    ~ServerNetworking(); //< Dtor.
    //@}

    /** \name Accessors */ //@{
    const_iterator GetPlayer(int id) const; ///< Returns an iterator to the established PlayerConnection object with ID \a id, or end() if none is found.
    const_iterator begin() const;           ///< Returns an iterator to the first established PlayerConnection object.
    const_iterator end() const;             ///< Returns an iterator to the one-past-the-last established PlayerConnection object.
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

    iterator GetPlayer(int id); ///< Returns an iterator to the established PlayerConnection object with ID \a id, or end() if none is found.
    iterator begin();           ///< Returns an iterator to the first established PlayerConnection object.
    iterator end();             ///< Returns an iterator to the one-past-the-last established PlayerConnection object.
    //@}

private:
    void Init();
    void AcceptNextConnection();
    void AcceptConnection(PlayerConnectionPtr player_connection, const boost::system::error_code& error);
    void DisconnectImpl(PlayerConnectionPtr player_connection);

    DiscoveryServer*               m_discovery_server;
    boost::asio::ip::tcp::acceptor m_player_connection_acceptor;
    PlayerConnections              m_player_connections;

    boost::function<void (const Message&, PlayerConnectionPtr)> m_nonplayer_message_callback;
    boost::function<void (const Message&, PlayerConnectionPtr)> m_player_message_callback;
    boost::function<void (PlayerConnectionPtr)>                 m_disconnected_callback;
};

#endif
