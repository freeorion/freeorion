// -*- C++ -*-
#ifndef _Message_h_
#define _Message_h_

#include <boost/shared_ptr.hpp>

#include <string>


class EmpireManager;
class Message;
class MultiplayerLobbyData;
class OrderSet;
class SaveGameUIData;
class SinglePlayerSetupData;
class Universe;
class XMLDoc;

/** Fills in the relevant portions of \a message with the values in the buffer \a header_buf. */
void BufferToHeader(const int* header_buf, Message& message);

/** Fills \a header_buf from the relevant portions of \a message. */
void HeaderToBuffer(const Message& message, int* header_buf);

/** Encapsulates a variable-length char buffer containing a message to be passed among the server and one or more
    clients.  Note that std::string is often thread unsafe on many platforms, so a dynamically allocated char array is
    used instead.  (It was feared that using another STL container of char might misbehave as well.) */
class Message
{
public:
    /** Represents the type of the message */
    enum MessageType {
        UNDEFINED,
        DEBUG,                   ///< used to send special messages used for debugging purposes
        SERVER_DYING,            ///< sent to the client when the server is about to terminate
        HOST_SP_GAME,            ///< sent when a client wishes to establish a single player game at the server
        HOST_MP_GAME,            ///< sent when a client wishes to establish a multiplayer game at the server
        JOIN_GAME,               ///< sent when a client wishes to join a game being established at the server
        RENAME_PLAYER,           ///< sent when the server must assign a new name to a player, because another player already has her desired name
        LOBBY_UPDATE,            ///< used to synchronize multiplayer lobby dialogs among different players, when a user changes a setting, or the server updates the state
        LOBBY_CHAT,              ///< used to send chat messages in the multiplayer lobby
        LOBBY_HOST_ABORT,        ///< sent to server (by the "host" client only) when a multiplayer game is to be cancelled while it is still being set up in the multiplayer lobby
        LOBBY_EXIT,              ///< sent to server (by a non-"host" client only) when a player leaves the multiplayer lobby
        START_MP_GAME,           ///< sent to server (by the "host" client only) when the settings in the MP lobby are satisfactory and it is time to start the game
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
        HUMAN_PLAYER_CHAT,       ///< sent when one player sends a chat message to another in multiplayer
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
    Message(); ///< Default ctor.

    /** Basic ctor. */
    Message(MessageType message_type,
            int sending_player,
            int receiving_player,
            ModuleType receiving_module,
            const std::string& text);

    Message(const Message& rhs);            ///< Copy ctor.
    ~Message();                             ///< Dtor.
    Message& operator=(const Message& rhs); ///< Assignment.
    //@}

    /** \name Accessors */ //@{
    MessageType Type() const;               ///< Returns the type of the message.
    int         SendingPlayer() const;      ///< Returns the ID of the sending player.
    int         ReceivingPlayer() const;    ///< Returns the ID of the receiving player.
    ModuleType  ReceivingModule() const;    ///< Returns the module to which this message should be sent when it reaches its destination.
    std::size_t Size() const;               ///< Returns the size of the underlying buffer.
    const char* Data() const;               ///< Returns the underlying buffer.
    std::string Text() const;               ///< Returns the underlying buffer as a std::string.
    //@}

    /** \name Accessors */ //@{
    void        Resize(std::size_t size);   ///< Resizes the underlying char buffer to \a size uninitialized bytes.
    char*       Data();                     ///< Returns the underlying buffer.
    void        Swap(Message& rhs);         ///< Swaps the contents of \a *this with \a rhs.  Does not throw.
    //@}

private:
    MessageType   m_type;
    int           m_sending_player;
    int           m_receiving_player;
    ModuleType    m_receiving_module;
    int           m_message_size;
    char*         m_message_text;

    friend void BufferToHeader(const int* header_buf, Message& message);
};

bool operator==(const Message& lhs, const Message& rhs);
bool operator!=(const Message& lhs, const Message& rhs);

void swap(Message& lhs, Message& rhs); ///< Swaps the contents of \a lhs and \a rhs.  Does not throw.


////////////////////////////////////////////////
// Message stringification
////////////////////////////////////////////////

/** Returns a string representation of \a type. */
std::string MessageTypeStr(Message::MessageType type);

/** Returns a string representation of \a type. */
std::string ModuleTypeStr(Message::ModuleType type);

/** Returns a string representation of \a phase. */
std::string TurnProgressPhaseStr(Message::TurnProgressPhase phase);

/** Writes \a msg to \a os.  The format of the output is designed for debugging purposes. */
std::ostream& operator<<(std::ostream& os, const Message& msg);


////////////////////////////////////////////////
// Message named ctors
////////////////////////////////////////////////

/** creates a HOST_SP_GAME message*/
Message HostSPGameMessage(int player_id, const SinglePlayerSetupData& setup_data);

/** creates a minimal HOST_MP_GAME message used to initiate multiplayer "lobby" setup*/
Message HostMPGameMessage(int player_id, const std::string& host_player_name);

/** creates a JOIN_GAME message.  The sender's player name is sent in the message.*/
Message JoinGameMessage(const std::string& player_name);

/** creates a GAME_START message.  Contains the initial game state visible to player \a player_id.*/
Message GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn, const EmpireManager& empires, const Universe& universe);

/** creates a HOST_SP_GAME acknowledgement message.  The \a player_id is the ID of the receiving player.  This message
   should only be sent by the server.*/
Message HostSPAckMessage(int player_id);

/** creates a HOST_MP_GAME acknowledgement message.  The \a player_id is the ID of the receiving player.  This message
   should only be sent by the server.*/
Message HostMPAckMessage(int player_id);

/** creates a JOIN_GAME acknowledgement message.  The \a player_id is the ID of the receiving player.  This message
   should only be sent by the server.*/
Message JoinAckMessage(int player_id);

/** creates a RENAME_PLAYER message that renames a player that has just joined a game with a name already in use.  The
    \a player_id is the ID of the receiving player.  This message should only be sent by the server.*/
Message RenameMessage(int player_id, const std::string& new_name);

/** creates an END_GAME message used to terminate an active game.  Only END_GAME messages sent from the host client 
    and the server are considered valid.*/
Message EndGameMessage(int sender, int receiver);

/** creates an END_GAME message indicating that the recipient has won the game.  This message should only be sent by the server.*/
Message VictoryMessage(int receiver);

/** creates an TURN_ORDERS message. */
Message TurnOrdersMessage(int sender, const OrderSet& orders);

/** creates an TURN_PROGRESS message. */
Message TurnProgressMessage(int player_id, Message::TurnProgressPhase phase_id, int empire_id);

/** creates a TURN_UPDATE message. */
Message TurnUpdateMessage(int player_id, int empire_id, int current_turn, const EmpireManager& empires, const Universe& universe);

/** creates a CLIENT_SAVE_DATA message, including UI data. */
Message ClientSaveDataMessage(int sender, const OrderSet& orders, const SaveGameUIData& ui_data);

/** creates a CLIENT_SAVE_DATA message, without UI data. */
Message ClientSaveDataMessage(int sender, const OrderSet& orders);

/** creates an REQUEST_NEW_OBJECT_ID  message. This message is a synchronous message, when sent it will wait for a reply form the server */
Message RequestNewObjectIDMessage(int sender);

/** creates an DISPATCH_NEW_OBJECT_ID  message.  This message is sent to a client who is waiting for a new object ID */
Message DispatchObjectIDMessage(int player_id, int new_id);

/** creates a SAVE_GAME request message.  This message should only be sent by the host player.*/
Message HostSaveGameMessage(int sender, const std::string& filename);

/** creates a SAVE_GAME data request message.  This message should only be sent by the server to get game data from a client.*/
Message ServerSaveGameMessage(int receiver);

/** creates a LOAD_GAME data message.  This message should only be sent by the server to provide saved game data to a client.*/
Message ServerLoadGameMessage(int receiver, const OrderSet& orders, const SaveGameUIData* ui_data);

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
// Multiplayer Lobby Message named ctors
////////////////////////////////////////////////

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the 
    server, then to other users.  Clients must send all such updates to the server directly; the server
    will send updates to the other clients as needed.*/
Message LobbyUpdateMessage(int sender, const MultiplayerLobbyData& lobby_data);

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the users.  
    This message should only be sent by the server.*/
Message ServerLobbyUpdateMessage(int receiver, const MultiplayerLobbyData& lobby_data);

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

/** creates a START_MP_GAME used to finalize the multiplayer lobby setup.*/
Message StartMPGameMessage(int player_id);


////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////

void ExtractMessageData(const Message& msg, MultiplayerLobbyData& lobby_data);

void ExtractMessageData(const Message& msg, bool& single_player_game, int& empire_id, int& current_turn, EmpireManager& empires, Universe& universe);

void ExtractMessageData(const Message& msg, OrderSet& orders);

void ExtractMessageData(const Message& msg, int empire_id, int& current_turn, EmpireManager& empires, Universe& universe);

bool ExtractMessageData(const Message& msg, OrderSet& orders, SaveGameUIData& ui_data);

void ExtractMessageData(const Message& msg, Message::TurnProgressPhase& phase_id, int& empire_id);

void ExtractMessageData(const Message& msg, SinglePlayerSetupData& setup_data);

#endif // _Message_h_
