// -*- C++ -*-
#ifndef _Message_h_
#define _Message_h_

#include "../util/XMLDoc.h"

#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
#include <boost/serialization/shared_ptr.hpp>
#endif

#include <string>

/** compresses \a str using zlib, and puts the result into \a zipped_str */
void ZipString(const std::string& str, std::string& zipped_str);

/** decompresses \a str using zlib, and puts the result into \a unzipped_str. The uncompressed size of 
   the string must be known beforehand, and passed in \a size.  Results are undefined when \a str does 
   not conatain a valid zipped byte sequence.*/
void UnzipString(const std::string& str, std::string& unzipped_str, int size);



class ClientNetworkCore;
class ServerNetworkCore;

/** FreeOrion network message class.  Messages are designed to be created, sent, received, read, then destroyed.
   They are not meant to be altered; there are no mutators methods. Upon creation, the text of the Message is
   compressed using zlib.  Subsequent reads incur the expense of decompressing the text.  The compressed text
   is stored in a string at the end of a shared_ptr, so copying Message objects is extremely inexpensive.*/
class Message
{
public:
    /** Represents the type of the message */
    enum MessageType {
        UNDEFINED,
        DEBUG,                   ///< used to send special messages used for debugging purposes
        SERVER_STATUS,           ///< sent to the client when requested, and when the server first recieves a connection from a client
        HOST_GAME,               ///< sent when a client wishes to establish a game at the server
        JOIN_GAME,               ///< sent when a client wishes to join a game being established at the server
        LOBBY_UPDATE,            ///< used to synchronize multiplayer lobby dialogs among different players, when a user changes a setting, or the server updates the state
        LOBBY_CHAT,              ///< used to send chat messages in the multiplayer lobby
        LOBBY_HOST_ABORT,        ///< sent to server (by the "host" client only) when a multiplayer game is to be cancelled while it is still being set up in the multiplayer lobby
        LOBBY_EXIT,              ///< sent to server (by a non-"host" client only) when a player leaves the multiplayer lobby
        SAVE_GAME,               ///< sent to server (by the "host" client only) when a game is to be saved, or from the server to the clients when the game is being saved
        LOAD_GAME,               ///< sent to server (by the "host" client only) when a game is to be loaded, or from the server to the clients when the game is being loaded
        GAME_START,              ///< sent to each client before the first turn of a new or newly loaded game, instead of a TURN_UPDATE
        TURN_UPDATE,             ///< sent to a client when the server updates the client Universes and Empires, and sends the SitReps each turn; indicates to the receiver that a new turn has begun
        TURN_ORDERS,             ///< sent to the server by a client that has orders to be processed at the end of a turn
        TURN_PROGRESS,           ///< sent to clients to display a turn progress message. To make messages short, IDs are used
        CLIENT_SAVE_DATA,        ///< sent to the server in response to a server request for the data needed to create a save file
        COMBAT_START,            ///< sent to clients when a combat is about to start
        COMBAT_ROUND_UPDATE,     ///< sent to clients when a combat round has been resolved
        COMBAT_END,              ///< sent to clients when a combat is concluded
        HUMAN_PLAYER_MSG,        ///< sent when one player sends a text message to another in multiplayer
        PLAYER_ELIMINATED,       ///< sent to all clients when a player is eliminated from play
        PLAYER_EXIT,             ///< sent to the "host" client when another player leaves the game
        REQUEST_NEW_OBJECT_ID,   ///< sent by client to server requesting a new object ID.
        DISPATCH_NEW_OBJECT_ID,  ///< sent by server to client with the new object ID.
        END_GAME,                ///< sent to the server by the host client when the current game is to end
    };
               
    /** Represents the module which is the destination for the message */
    enum ModuleType {
        CORE,                           ///< this module is the ServerCore or ClientCore, as appropriate; all server-bound messages go here
        CLIENT_LOBBY_MODULE,            ///< the human-only multiplayer lobby dialog
        CLIENT_UNIVERSE_MODULE,         ///< the ClientUniverse module
        CLIENT_EMPIRE_MODULE,           ///< the ClientEmpire module
        CLIENT_COMBAT_MODULE,           ///< the client Combat module
        CLIENT_SYNCHRONOUS_RESPONSE     ///< client-only - the response to a synchronous message
    };

    enum TurnProgressPhase {
        FLEET_MOVEMENT,           ///< fleet movement turn progress message
        COMBAT,                   ///< combat turn progress message
        EMPIRE_PRODUCTION,        ///< empire production turn progress message
        WAITING_FOR_PLAYERS,      ///< waiting for other to end their turn
        PROCESSING_ORDERS,        ///< processing orders
        DOWNLOADING               ///< downloading new game state from server
    };


    /** \name Structors */ //@{
    /** default ctor. */
    Message();

    /** standard ctor.  Senders that are not part of a game and so have no player number should send -1 as the \a 
        sender parameter. \throw std::invalid_argument May throw std::invalid_argument if the parameters would form
        an invalid message */
    Message(MessageType msg_type, int sender, int receiver, ModuleType module, const std::string& text, MessageType response_msg = UNDEFINED);
 
    /** convienience ctor that converts \a doc into a std::string automatically.  Senders that are not part of a game 
        and so have no player number should send -1 as the \a sender parameter. \throw std::invalid_argument May throw 
        std::invalid_argument if the parameters would form an invalid message */
    Message(MessageType msg_type, int sender, int receiver, ModuleType module, const XMLDoc& doc, MessageType response_msg = UNDEFINED);
    //@}

    /** \name Accessors */ //@{
    MessageType Type() const;      ///< returns type of message
    MessageType Response() const;  ///< returns response this message expects, implies a sychronous message
    int         Sender() const;    ///< returns the ID of the player sending the message (-1 represents server or a client not yet in a game)
    int         Receiver() const;  ///< returns the ID of the player receiving the message (-1 represents server)
    ModuleType  Module() const;    ///< returns the module that is to get the message at the receiving end
    std::string GetText() const;   ///< returns the message text.  \note \note This function uncompresses the text in order to return it.
    //@}
 
private:
/** private ctor to be used by the NetworkCore classes. Constructs a message from a string of bytes received over a 
    network connection. \throw std::invalid_argument May throw std::invalid_argument if the parameter would form
    an invalid message */
    Message(const std::string& raw_msg);
   
    std::string HeaderString() const; ///< for use by the NetworkCore classes to create a string of bytes of the non-text portion of a Message
    void ValidateMessage(); ///< checks that the data in the message are consistent
    void CompressMessage(std::string& compressed_msg) const;     ///< fills \a compressed_msg with the compressed text of the message text
    void DecompressMessage(std::string& uncompressed_msg) const; ///< fills \a decompressed_msg with the decompressed text of the message text

    MessageType                    m_message_type;
    int                            m_sending_player;
    int                            m_receiving_player;
    ModuleType                     m_receiving_module;
    boost::shared_ptr<std::string> m_message_text;
    bool                           m_compressed;
    int                            m_uncompressed_size;
    MessageType                    m_response_msg;  ///< implies a synchronous message is set, this is the message it's expecting to be returned from the server ( only client messages can be synchronous )

    friend class NetworkCore;         ///< grant access for calls to HeaderString()
    friend class ClientNetworkCore;   ///< grant access for calls to private ctor and HeaderString()
    friend class ServerNetworkCore;   ///< grant access for calls to private ctor and HeaderString()
};


/** Returns a string representation of \a type. */
std::string MessageTypeStr(Message::MessageType type);

/** Returns a string representation of \a type. */
std::string ModuleTypeStr(Message::ModuleType type);

/** Returns a string representation of \a phase. */
std::string TurnProgressPhaseStr(Message::TurnProgressPhase phase);

/** Writes \a msg to \a os.  The format of the output is designed for debugging purposes. */
std::ostream& operator<<(std::ostream& os, const Message& msg);


/** creates a HOST_GAME message*/
Message HostGameMessage(int player_id, const XMLDoc& game_parameters);

/** creates a minimal HOST_GAME message used to enter and finalize the multiplayer "lobby" setup*/
Message HostGameMessage(int player_id, const std::string& host_player_name);

/** creates a JOIN_GAME message.  The sender's player name is sent in the message.*/
Message JoinGameMessage(const std::string& player_name);

/** creates a JOIN_GAME message.  Sends an xml document of the player's details.*/
Message JoinGameSetup(const XMLDoc& player_setup);

/** creates a SERVER_STATUS message that indicates that the JOIN_GAME message just received from the client will
    not be honored because the client and server are using different versions of code or essential configuration
    files.  This message should only be sent by the server.*/
Message VersionConflictMessage(int player_id, const XMLDoc& conflict_details);

/** creates a GAME_START message.  Contains the initial game state visible to player \a player_id.*/
Message GameStartMessage(int player_id, const std::string& data);

/** creates a HOST_GAME acknowledgement message.  The \a player_id is the ID of the receiving player.  This message
   should only be sent by the server.*/
Message HostAckMessage(int player_id);

/** creates a JOIN_GAME acknowledgement message.  The \a player_id is the ID of the receiving player.  This message
   should only be sent by the server.*/
Message JoinAckMessage(int player_id);

/** creates a SERVER_STATUS message that renames a player that has just joined a game with a name already in use.  
    The \a player_id is the ID of the receiving player.  This message should only be sent by the server.*/
Message RenameMessage(int player_id, const std::string& new_name);

/** creates an END_GAME message used to terminate an active game.  Only END_GAME messages sent from the host client 
    and the server are considered valid.*/
Message EndGameMessage(int sender, int receiver);

/** creates an END_GAME message indicating that the recipient has won the game.  This message should only be sent by the server.*/
Message VictoryMessage(int receiver);

/** creates an TURN_ORDERS message. */
Message TurnOrdersMessage(int sender, const std::string& data);

/** creates an TURN_PROGRESS message. */
Message TurnProgressMessage(int player_id, Message::TurnProgressPhase phase_id, int empire_id);

/** creates a TURN_UPDATE message. */
Message TurnUpdateMessage(int player_id, const std::string& data);

/** creates an CLIENT_SAVE_DATA message. */
Message ClientSaveDataMessage(int sender, const std::string& data);

/** creates an REQUEST_NEW_OBJECT_ID  message. This message is a synchronous message, when sent it will wait for a reply form the server */
Message RequestNewObjectIDMessage(int sender);

/** creates an DISPATCH_NEW_OBJECT_ID  message.  This message is sent to a client who is waiting for a new object ID */
Message DispatchObjectIDMessage(int player_id, int new_id);

/** creates a SAVE_GAME request message.  This message should only be sent by the host player.*/
Message HostSaveGameMessage(int sender, const std::string& filename);

/** creates a LOAD_GAME request message.  This message should only be sent by the the host player.*/
Message HostLoadGameMessage(int sender, const std::string& filename);

/** creates a SAVE_GAME data request message.  This message should only be sent by the server to get game data from a client.*/
Message ServerSaveGameMessage(int receiver, bool done = false);

/** creates a LOAD_GAME data message.  This message should only be sent by the server to provide saved game data to a client.*/
Message ServerLoadGameMessage(int receiver, const std::string& data);

/** creates a HUMAN_PLAYER_MSG, which is sent to the server, and then from the server to all human players, including the 
    originating player.  This is used for MP chat.*/
Message ChatMessage(int sender, const std::string& msg);

/** creates a HUMAN_PLAYER_MSG, which is sent to the specific indicated receiver.  This is used for MP chat.*/
Message ChatMessage(int sender, int receiver, const std::string& msg);

/** creates a PLAYER_EXIT message, which is sent to all remaining clients when a client looses its connection to the server.  
    This message should only be sent by the server.*/
Message PlayerDisconnectedMessage(int receiver, const std::string& player_name);

/** creates a PLAYER_ELIMINATED message, which is sent to all clients when a client is eliminated from play.  
    This message should only be sent by the server.*/
Message PlayerEliminatedMessage(int receiver, const std::string& empire_name);



////////////////////////////////////////////////
// Multiplayer Lobby Messages
////////////////////////////////////////////////

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the 
    server, then to other users.  Clients must send all such updates to the server directly; the server
    will send updates to the other clients as needed.*/
Message LobbyUpdateMessage(int sender, const std::string& data);

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the users.  
    This message should only be sent by the server.*/
Message ServerLobbyUpdateMessage(int receiver, const std::string& data);

/** creates an LOBBY_CHAT message containing a chat string to be broadcast to player \a receiver, or all players if \a
    receiver is -1. Note that the receiver of this message is always the server.*/
Message LobbyChatMessage(int sender, int receiver, const std::string& text);

/** creates an LOBBY_CHAT message containing a chat string from \sender to be displayed in \a receiver's lobby dialog.
    This message should only be sent by the server.*/
Message ServerLobbyChatMessage(int sender, int receiver, const std::string& text);

/** creates an LOBBY_HOST_ABORT message.  This message should only be sent by the host player.*/
Message LobbyHostAbortMessage(int sender);

/** creates an LOBBY_HOST_ABORT message.  This message should only be sent by the server.*/
Message ServerLobbyHostAbortMessage(int receiver);

/** creates an LOBBY_EXIT message.  This message should only be sent by a non-host player.*/
Message LobbyExitMessage(int sender);

/** creates an LOBBY_EXIT message.  This message should only be sent by the server.*/
Message ServerLobbyExitMessage(int sender, int receiver);

#endif // _Message_h_
