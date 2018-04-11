#ifndef _ClientNetworking_h_
#define _ClientNetworking_h_

#include <boost/optional/optional_fwd.hpp>

#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include <memory>

class Message;

namespace Networking {
    class AuthRoles;

    enum RoleType : size_t;
}

/** Encapsulates the networking facilities of the client.  The client must
    execute its networking code in a separate thread from its main processing
    thread, for UI and networking responsiveness.

    Because of this, ClientNetworking operates in two threads: the main
    thread, in which the UI processing operates; the networking thread, which
    is created and terminates when the client is connected to and disconnected
    from the server, respectively.  The entire public interface is safe to
    call from the main thread at all times.  Note that the main thread must
    periodically request the next incoming message; the arrival of incoming
    messages is never explicitly signalled to the main thread.  The same
    applies to unintentional disconnects from the server.  The client must
    periodically check IsConnected().

    The ClientNetworking has two modes of operation.  First, it can discover
    FreeOrion servers on the local network; this is a blocking operation with
    a timeout.  Second, it can send an asynchronous message to the server
    (when connected); this is a non-blocking operation.*/
class ClientNetworking : public std::enable_shared_from_this<ClientNetworking> {
public:
    /** The type of list returned by a call to DiscoverLANServers(). */
    using ServerNames =  std::vector<std::string>;

    /** \name Structors */ //@{
    ClientNetworking();
    ~ClientNetworking();
    //@}

    /** \name Accessors */ //@{
    /** Returns true iff the client is full duplex connected to the server. */
    bool IsConnected() const;

    /** Returns true iff the client is connected to receive from the server. */
    bool IsRxConnected() const;

    /** Returns true iff the client is connected to send to the server. */
    bool IsTxConnected() const;

    /** Returns the ID of the player on this client. */
    int PlayerID() const;

    /** Returns the ID of the host player, or INVALID_PLAYER_ID if there is no host player. */
    int HostPlayerID() const;

    /** Returns whether the indicated player ID is the host. */
    bool PlayerIsHost(int player_id) const;

    /** Checks if the client has some authorization \a role. */
    bool HasAuthRole(Networking::RoleType role) const;

    /** Returns address of multiplayer server entered by player. */
    const std::string& Destination() const;
    //@}

    /** \name Mutators */ //@{
    /** Returns a list of the addresses and names of all servers on the Local
        Area Network. */
    ServerNames DiscoverLANServerNames();

    /** Connects to the server at \a ip_address.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToServer(const std::string& ip_address,
                         const std::chrono::milliseconds& timeout = std::chrono::seconds(10));

    /** Connects to the server on the client's host.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToLocalHostServer(
        const std::chrono::milliseconds& timeout = std::chrono::seconds(10));

    /** Return true if the server can be connected to within \p timeout seconds. */
    bool PingServer(
        const std::string& ip_address,
        const std::chrono::milliseconds& timeout = std::chrono::seconds(10));

    /** Return true if the local server can be connected to within \p timeout seconds. */
    bool PingLocalHostServer(
        const std::chrono::milliseconds& timeout = std::chrono::seconds(10));

    /** Sends \a message to the server.  This function actually just enqueues
        the message for sending and returns immediately. */
    void SendMessage(const Message& message);

    /** Return the next incoming message from the server if available or boost::none.
        Remove the message from the incoming message queue. */
    boost::optional<Message> GetMessage();

    /** Disconnects the client from the server. First tries to send any pending transmit messages. */
    void DisconnectFromServer();

    /** Sets player ID for this client. */
    void SetPlayerID(int player_id);

    /** Sets Host player ID. */
    void SetHostPlayerID(int host_player_id);

    /** Access to client's authorization roles */
    Networking::AuthRoles& AuthorizationRoles();
    //@}

private:
    class Impl;
    std::unique_ptr<Impl> const m_impl;
};

#endif
