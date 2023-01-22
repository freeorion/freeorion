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

    enum class RoleType : uint8_t;
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
    using ServerNames = std::vector<std::string>;

    ClientNetworking();
public:
    ~ClientNetworking();

    /** Returns true iff the client is full duplex connected to the server. */
    [[nodiscard]] bool IsConnected() const;

    /** Returns true iff the client is connected to receive from the server. */
    [[nodiscard]] bool IsRxConnected() const;

    /** Returns true iff the client is connected to send to the server. */
    [[nodiscard]] bool IsTxConnected() const;

    /** Returns the ID of the player on this client. */
    [[nodiscard]] int PlayerID() const noexcept;

    /** Returns the ID of the host player, or INVALID_PLAYER_ID if there is no host player. */
    [[nodiscard]] int HostPlayerID() const noexcept;

    /** Returns whether the indicated player ID is the host. */
    [[nodiscard]] bool PlayerIsHost(int player_id) const noexcept;

    /** Checks if the client has some authorization \a role. */
    [[nodiscard]] bool HasAuthRole(Networking::RoleType role) const;

    /** Returns address of multiplayer server entered by player. */
    [[nodiscard]] const std::string& Destination() const noexcept;

    /** Returns a list of the addresses and names of all servers on the Local
        Area Network. */
    [[nodiscard]] ServerNames DiscoverLANServerNames();

    /** Connects to the server at \a ip_address.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToServer(std::string ip_address,
                         std::chrono::milliseconds timeout = std::chrono::seconds(10));

    /** Connects to the server on the client's host.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToLocalHostServer(std::chrono::milliseconds timeout = std::chrono::seconds(10));

    /** Return true if the server can be connected to within \p timeout seconds. */
    bool PingServer(std::string ip_address, std::chrono::milliseconds timeout = std::chrono::seconds(10));

    /** Return true if the local server can be connected to within \p timeout seconds. */
    bool PingLocalHostServer(std::chrono::milliseconds timeout = std::chrono::seconds(10));

    /** Sends \a message to the server.  This function actually just enqueues
        the message for sending and returns immediately. */
    void SendMessage(Message message);

    /** Puts \a message on this client's message queue. */
    void SendSelfMessage(Message message);

    /** Return the next incoming message from the server if available or boost::none.
        Remove the message from the incoming message queue. */
    [[nodiscard]] boost::optional<Message> GetMessage();

    /** Disconnects the client from the server. First tries to send any pending transmit messages. */
    void DisconnectFromServer();

    /** Sets player ID for this client. */
    void SetPlayerID(int player_id);

    /** Sets Host player ID. */
    void SetHostPlayerID(int host_player_id) noexcept;

    /** Access to client's authorization roles */
    [[nodiscard]] Networking::AuthRoles& AuthorizationRoles();

private:
    friend class ClientApp;

    class Impl;
    std::unique_ptr<Impl> const m_impl;
};


#endif
