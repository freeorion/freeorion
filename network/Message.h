// -*- C++ -*-
#ifndef _Message_h_
#define _Message_h_

#include "Networking.h"
#include "../util/Export.h"
#include <GG/Enum.h>

#include <boost/shared_array.hpp>

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <string>
#include <map>
#include <vector>

class EmpireManager;
class SpeciesManager;
class CombatLogManager;
class Message;
struct MultiplayerLobbyData;
class OrderSet;
struct PlayerInfo;
struct SaveGameUIData;
struct PreviewInformation;
struct GalaxySetupData;
struct SinglePlayerSetupData;
class ShipDesign;
class System;
class Universe;
class UniverseObject;
class DiplomaticMessage;
struct DiplomaticStatusUpdateInfo;
namespace Moderator {
    class ModeratorAction;
}

typedef std::map<int, ShipDesign*> ShipDesignMap;

/** Fills in the relevant portions of \a message with the values in the buffer \a header_buf. */
FO_COMMON_API void BufferToHeader(const int* header_buf, Message& message);

/** Fills \a header_buf from the relevant portions of \a message. */
FO_COMMON_API void HeaderToBuffer(const Message& message, int* header_buf);

/** Encapsulates a variable-length char buffer containing a message to be passed
  * among the server and one or more clients.  Note that std::string is often
  * thread unsafe on many platforms, so a dynamically allocated char array is
  * used instead.  (It was feared that using another STL container of char might
  * misbehave as well.) */
class FO_COMMON_API Message {
public:
    /** Represents the type of the message */
    GG_CLASS_ENUM(MessageType,
        UNDEFINED = 0,
        DEBUG,                  ///< used to send special messages used for debugging purposes
        ERROR_MSG,              ///< used to communicate errors between server and clients
        HOST_SP_GAME,           ///< sent when a client wishes to establish a single player game at the server
        HOST_MP_GAME,           ///< sent when a client wishes to establish a multiplayer game at the server
        JOIN_GAME,              ///< sent when a client wishes to join a game being established at the server
        HOST_ID,                ///< sent to clients when the server changes the ID of the host player
        LOBBY_UPDATE,           ///< used to synchronize multiplayer lobby dialogs among different players, when a user changes a setting, or the server updates the state
        LOBBY_CHAT,             ///< used to send chat messages in the multiplayer lobby
        LOBBY_EXIT,             ///< sent to server by clients when a player leaves the multiplayer lobby, or by server to clients when a player leaves the multiplayer lobby
        START_MP_GAME,          ///< sent to server (by the "host" client only) when the settings in the MP lobby are satisfactory and it is time to start the game
        SAVE_GAME,              ///< sent to server (by the "host" client only) when a game is to be saved, or from the server to the clients when the game is being saved
        LOAD_GAME,              ///< sent to server (by the "host" client only) when a game is to be loaded
        GAME_START,             ///< sent to each client before the first turn of a new or newly loaded game, instead of a TURN_UPDATE
        TURN_UPDATE,            ///< sent to a client when the server updates the client Universes and Empires, and sends the SitReps each turn; indicates to the receiver that a new turn has begun
        TURN_PARTIAL_UPDATE,    ///< sent to a client when the server updates part of the client gamestate after partially processing a turn, such as after fleet movement but before the rest of the turn is processed.  Does NOT indicate a new turn has begun.
        TURN_ORDERS,            ///< sent to the server by a client that has orders to be processed at the end of a turn
        TURN_PROGRESS,          ///< sent to clients to display a turn progress message
        PLAYER_STATUS,          ///< sent to clients to inform them that a player has some status, such as having finished playing a turn and submitted orders, or is resolving combat, or is playing a turn normally
        CLIENT_SAVE_DATA,       ///< sent to the server in response to a server request for the data needed to create a save file
        PLAYER_CHAT,            ///< sent when one player sends a chat message to another in multiplayer
        DIPLOMACY,              ///< sent by players to server or server to players to make or convey diplomatic proposals or declarations, or to accept / reject proposals from other players
        DIPLOMATIC_STATUS,      ///< sent by server to players to inform of mid-turn diplomatic status changes
        REQUEST_NEW_OBJECT_ID,  ///< sent by client to server requesting a new object ID.
        DISPATCH_NEW_OBJECT_ID, ///< sent by server to client with the new object ID.
        REQUEST_NEW_DESIGN_ID,  ///< sent by client to server requesting a new design ID.
        DISPATCH_NEW_DESIGN_ID, ///< sent by server to client with the new design ID.
        END_GAME,               ///< sent by the server when the current game is to ending (see EndGameReason for the possible reasons this message is sent out)
        MODERATOR_ACTION,       ///< sent by client to server when a moderator edits the universe
        SHUT_DOWN_SERVER,       ///< sent by host client to server to kill the server process
        REQUEST_SAVE_PREVIEWS,  ///< sent by client to request previews of available savegames
        DISPATCH_SAVE_PREVIEWS  ///< sent by host to client to provide the savegame previews
    )

    GG_CLASS_ENUM(TurnProgressPhase,
        FLEET_MOVEMENT,         ///< fleet movement turn progress message
        COMBAT,                 ///< combat turn progress message
        EMPIRE_PRODUCTION,      ///< empire production turn progress message
        WAITING_FOR_PLAYERS,    ///< waiting for other to end their turn
        PROCESSING_ORDERS,      ///< processing orders
        COLONIZE_AND_SCRAP,     ///< enacting colonization and scrapping orders
        DOWNLOADING,            ///< downloading new game state from server
        LOADING_GAME,           ///< loading gamestate from save
        GENERATING_UNIVERSE,    ///< creating new universe
        STARTING_AIS            ///< creating AI clients
    )

    GG_CLASS_ENUM(PlayerStatus,
        PLAYING_TURN,           ///< player is playing a turn, on the galax map
        RESOLVING_COMBAT,       ///< player is resolving a combat interactively
        WAITING                 ///< player is waiting for others to submit orders, to resolve combats, or for turn processing to complete
    )

    GG_CLASS_ENUM(EndGameReason,
        LOCAL_CLIENT_DISCONNECT,///< the local player's client networking detected a disconnection from the server
        PLAYER_DISCONNECT,      ///< an active player (not an observer) was disconnected
    )

    /** \name Structors */ //@{
    Message(); ///< Default ctor.

    /** Basic ctor. */
    Message(MessageType message_type,
            int sending_player,
            int receiving_player,
            const std::string& text,
            bool synchronous_response = false);
    //@}

    /** \name Accessors */ //@{
    MessageType Type() const;               ///< Returns the type of the message.
    int         SendingPlayer() const;      ///< Returns the ID of the sending player.
    int         ReceivingPlayer() const;    ///< Returns the ID of the receiving player.
    bool        SynchronousResponse() const;///< Returns true if this message is in reponse to a synchronous message
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
    bool          m_synchronous_response;
    int           m_message_size;

    boost::shared_array<char> m_message_text;

    friend void BufferToHeader(const int* header_buf, Message& message);
};

bool operator==(const Message& lhs, const Message& rhs);
bool operator!=(const Message& lhs, const Message& rhs);

FO_COMMON_API void swap(Message& lhs, Message& rhs); ///< Swaps the contents of \a lhs and \a rhs.  Does not throw.


////////////////////////////////////////////////
// Message stringification
////////////////////////////////////////////////

/** Writes \a msg to \a os.  The format of the output is designed for debugging purposes. */
FO_COMMON_API std::ostream& operator<<(std::ostream& os, const Message& msg);


////////////////////////////////////////////////
// Message named ctors
////////////////////////////////////////////////

/** creates an ERROR_MSG message*/
FO_COMMON_API Message ErrorMessage(const std::string& problem, bool fatal = true);
FO_COMMON_API Message ErrorMessage(int player_id, const std::string& problem, bool fatal = true);

/** creates a HOST_SP_GAME message*/
FO_COMMON_API Message HostSPGameMessage(const SinglePlayerSetupData& setup_data);

/** creates a minimal HOST_MP_GAME message used to initiate multiplayer "lobby" setup*/
FO_COMMON_API Message HostMPGameMessage(const std::string& host_player_name);

/** creates a JOIN_GAME message.  The sender's player name and client type are sent in the message.*/
FO_COMMON_API Message JoinGameMessage(const std::string& player_name, Networking::ClientType client_type);

/** creates a HOST_ID message.  The player ID of the host is sent in the message. */
FO_COMMON_API Message HostIDMessage(int host_player_id);

/** creates a GAME_START message.  Contains the initial game state visible to player \a player_id.*/
FO_COMMON_API Message GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, const CombatLogManager& combat_logs,
                                       const std::map<int, PlayerInfo>& players,
                                       const GalaxySetupData& galaxy_setup_data);

/** creates a GAME_START message.  Contains the initial game state visible to
  * player \a player_id.  Also includes data loaded from a saved game. */
FO_COMMON_API Message GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, const CombatLogManager& combat_logs,
                                       const std::map<int, PlayerInfo>& players, const OrderSet& orders,
                                       const SaveGameUIData* ui_data,
                                       const GalaxySetupData& galaxy_setup_data);

/** creates a GAME_START message.  Contains the initial game state visible to
  * player \a player_id.  Also includes state string loaded from a saved game. */
FO_COMMON_API Message GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, const CombatLogManager& combat_logs,
                                       const std::map<int, PlayerInfo>& players, const OrderSet& orders,
                                       const std::string* save_state_string,
                                       const GalaxySetupData& galaxy_setup_data);

/** creates a HOST_SP_GAME acknowledgement message.  The \a player_id is the ID
  * of the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API Message HostSPAckMessage(int player_id);

/** creates a HOST_MP_GAME acknowledgement message.  The \a player_id is the ID
  * of the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API Message HostMPAckMessage(int player_id);

/** creates a JOIN_GAME acknowledgement message.  The \a player_id is the ID of
  * the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API Message JoinAckMessage(int player_id);

/** creates a TURN_ORDERS message. */
FO_COMMON_API Message TurnOrdersMessage(int sender, const OrderSet& orders);

/** creates a TURN_PROGRESS message. */
FO_COMMON_API Message TurnProgressMessage(Message::TurnProgressPhase phase_id, int player_id = Networking::INVALID_PLAYER_ID);

/** creates a PLAYER_STATUS message. */
FO_COMMON_API Message PlayerStatusMessage(int player_id, int about_player_id, Message::PlayerStatus player_status);

/** creates a TURN_UPDATE message. */
FO_COMMON_API Message TurnUpdateMessage(int player_id, int empire_id, int current_turn,
                                        const EmpireManager& empires, const Universe& universe,
                                        const SpeciesManager& species,
                                        const CombatLogManager& combat_logs,
                                        const std::map<int, PlayerInfo>& players);

/** create a TURN_PARTIAL_UPDATE message. */
FO_COMMON_API Message TurnPartialUpdateMessage(int player_id, int empire_id, const Universe& universe);

/** creates a CLIENT_SAVE_DATA message, including UI data but without a state string. */
FO_COMMON_API Message ClientSaveDataMessage(int sender, const OrderSet& orders, const SaveGameUIData& ui_data);

/** creates a CLIENT_SAVE_DATA message, without UI data but with a state string. */
FO_COMMON_API Message ClientSaveDataMessage(int sender, const OrderSet& orders, const std::string& save_state_string);

/** creates a CLIENT_SAVE_DATA message, without UI data and without a state string. */
Message ClientSaveDataMessage(int sender, const OrderSet& orders);

/** creates an REQUEST_NEW_OBJECT_ID message. This message is a synchronous
    message, when sent it will wait for a reply form the server */
FO_COMMON_API Message RequestNewObjectIDMessage(int sender);

/** creates an DISPATCH_NEW_OBJECT_ID  message.  This message is sent to a
  * client who is waiting for a new object ID */
FO_COMMON_API Message DispatchObjectIDMessage(int player_id, int new_id);

/** creates an REQUEST_NEW_DESIGN_ID message. This message is a synchronous
    message, when sent it will wait for a reply form the server */
FO_COMMON_API Message RequestNewDesignIDMessage(int sender);

/** creates an DISPATCH_NEW_DESIGN_ID  message.  This message is sent to a
  * client who is waiting for a new design ID */
FO_COMMON_API Message DispatchDesignIDMessage(int player_id, int new_id);

/** creates a SAVE_GAME request message.  This message should only be sent by
  * the host player.*/
FO_COMMON_API Message HostSaveGameMessage(int sender, const std::string& filename);

/** creates a SAVE_GAME data request message.  This message should only be
    sent by the server to get game data from a client.*/
FO_COMMON_API Message ServerSaveGameMessage(int receiver, bool synchronous_response);

/** creates a PLAYER_CHAT, which is sent to the server, and then from the server
  * to all players, including the originating player.*/
FO_COMMON_API Message GlobalChatMessage(int sender, const std::string& msg);

/** creates a PLAYER_CHAT message, which is sent to the server, and then from
  * the server to a single recipient player */
FO_COMMON_API Message SingleRecipientChatMessage(int sender, int receiver, const std::string& msg);

/** creates a DIPLOMACY message, which is sent between players via the server to
  * declare, proposed, or accept / reject diplomatic arrangements or agreements. */
FO_COMMON_API Message DiplomacyMessage(int sender, int receiver, const DiplomaticMessage& diplo_message);

/** creates a DIPLOMATIC_STATUS message, which is sent to players by the server to
  * update them on diplomatic status changes between players. */
FO_COMMON_API Message DiplomaticStatusMessage(int receiver, const DiplomaticStatusUpdateInfo& diplo_update);

/** creates an END_GAME message used to terminate an active game. */
FO_COMMON_API Message EndGameMessage(int receiver, Message::EndGameReason reason, const std::string& reason_player_name = "");

/** creates a MODERATOR_ACTION message used to implement moderator commands. */
FO_COMMON_API Message ModeratorActionMessage(int sender, const Moderator::ModeratorAction& mod_action);

/** tells server to shut down. */
FO_COMMON_API Message ShutdownServerMessage(int sender);

/** requests previews of savefiles from server */
FO_COMMON_API Message RequestSavePreviewsMessage(int sender, std::string directory);

/** returns the savegame previews to the client */
FO_COMMON_API Message DispatchSavePreviewsMessage(int receiver, const PreviewInformation& preview);

////////////////////////////////////////////////
// Multiplayer Lobby Message named ctors
////////////////////////////////////////////////

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the 
    server, then to other users.  Clients must send all such updates to the server directly; the server
    will send updates to the other clients as needed.*/
FO_COMMON_API Message LobbyUpdateMessage(int sender, const MultiplayerLobbyData& lobby_data);

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the users.  
    This message should only be sent by the server.*/
FO_COMMON_API Message ServerLobbyUpdateMessage(int receiver, const MultiplayerLobbyData& lobby_data);

/** creates an LOBBY_CHAT message containing a chat string to be broadcast to player \a receiver, or all players if \a
    receiver is Networking::INVALID_PLAYER_ID. Note that the receiver of this message is always the server.*/
FO_COMMON_API Message LobbyChatMessage(int sender, int receiver, const std::string& text);

/** creates an LOBBY_CHAT message containing a chat string from \sender to be displayed in \a receiver's lobby dialog.
    This message should only be sent by the server.*/
FO_COMMON_API Message ServerLobbyChatMessage(int sender, int receiver, const std::string& text);

/** creates a START_MP_GAME used to finalize the multiplayer lobby setup.*/
FO_COMMON_API Message StartMPGameMessage(int player_id);


////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////

FO_COMMON_API void ExtractMessageData(const Message& msg, std::string& problem, bool& fatal);

FO_COMMON_API void ExtractMessageData(const Message& msg, MultiplayerLobbyData& lobby_data);

FO_COMMON_API void ExtractMessageData(const Message& msg, bool& single_player_game, int& empire_id,
                                      int& current_turn, EmpireManager& empires, Universe& universe,
                                      SpeciesManager& species, CombatLogManager& combat_logs,
                                      std::map<int, PlayerInfo>& players, OrderSet& orders,
                                      bool& loaded_game_data, bool& ui_data_available,
                                      SaveGameUIData& ui_data, bool& save_state_string_available,
                                      std::string& save_state_string, GalaxySetupData& galaxy_setup_data);

FO_COMMON_API void ExtractMessageData(const Message& msg, std::string& player_name, Networking::ClientType& client_type);

FO_COMMON_API void ExtractMessageData(const Message& msg, OrderSet& orders);

FO_COMMON_API void ExtractMessageData(const Message& msg, int empire_id, int& current_turn, EmpireManager& empires,
                        Universe& universe, SpeciesManager& species, CombatLogManager& combat_logs,
                        std::map<int, PlayerInfo>& players);

FO_COMMON_API void ExtractMessageData(const Message& msg, int empire_id, Universe& universe);

FO_COMMON_API void ExtractMessageData(const Message& msg, OrderSet& orders, bool& ui_data_available,
                        SaveGameUIData& ui_data, bool& save_state_string_available,
                        std::string& save_state_string);

FO_COMMON_API void ExtractMessageData(const Message& msg, Message::TurnProgressPhase& phase_id);

FO_COMMON_API void ExtractMessageData(const Message& msg, int& about_player_id, Message::PlayerStatus& status);

FO_COMMON_API void ExtractMessageData(const Message& msg, SinglePlayerSetupData& setup_data);

FO_COMMON_API void ExtractMessageData(const Message& msg, Message::EndGameReason& reason, std::string& reason_player_name);

FO_COMMON_API void ExtractMessageData(const Message& msg, Moderator::ModeratorAction*& action);

FO_COMMON_API void ExtractMessageData(const Message& msg, int& empire_id, std::string& empire_name);

FO_COMMON_API void ExtractMessageData(const Message& msg, DiplomaticMessage& diplo_message);

FO_COMMON_API void ExtractMessageData(const Message& msg, DiplomaticStatusUpdateInfo& diplo_update);

FO_COMMON_API void ExtractMessageData(const Message& msg, std::string& directory);

FO_COMMON_API void ExtractMessageData(const Message& msg, PreviewInformation& previews);

#endif // _Message_h_
