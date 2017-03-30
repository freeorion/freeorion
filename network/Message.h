#ifndef _Message_h_
#define _Message_h_

#include "Networking.h"
#include "../util/Export.h"
#include <GG/Enum.h>

#include <boost/shared_array.hpp>

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <array>
#include <map>
#include <string>
#include <vector>


class EmpireManager;
class SupplyManager;
class SpeciesManager;
class CombatLog;
class CombatLogManager;
class MessagePacket;
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

/** Encapsulates a variable-length char buffer containing a message to be passed
  * among the server and one or more clients.  Note that std::string is often
  * thread unsafe on many platforms, so a dynamically allocated char array is
  * used instead.  (It was feared that using another STL container of char might
  * misbehave as well.) */
class FO_COMMON_API MessagePacket {
public:
    typedef std::array<int, 5> HeaderBuffer;

    constexpr static size_t HeaderBufferSize =
        std::tuple_size<HeaderBuffer>::value * sizeof(HeaderBuffer::value_type);

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
        SAVE_GAME_INITIATE,     ///< sent to server (by the "host" client only) when a game is to be saved
        SAVE_GAME_DATA_REQUEST, ///< sent to clients by the server when the game is being saved and the server needs save state info from clients
        SAVE_GAME_COMPLETE,     ///< sent to clients (by the "host" client only) when a game has been saved and written to disk
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
        AI_END_GAME_ACK,        ///< sent by the ai client when it has shutdown
        MODERATOR_ACTION,       ///< sent by client to server when a moderator edits the universe
        SHUT_DOWN_SERVER,       ///< sent by host client to server to kill the server process
        REQUEST_SAVE_PREVIEWS,  ///< sent by client to request previews of available savegames
        DISPATCH_SAVE_PREVIEWS,  ///< sent by host to client to provide the savegame previews
        REQUEST_COMBAT_LOGS,  ///< sent by client to request combat logs
        DISPATCH_COMBAT_LOGS  ///< sent by host to client to provide combat logs
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
    MessagePacket();

    MessagePacket(MessageType message_type,
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
    void        Swap(MessagePacket& rhs);         ///< Swaps the contents of \a *this with \a rhs.  Does not throw.
    //@}

private:
    MessageType   m_type;
    int           m_sending_player;
    int           m_receiving_player;
    bool          m_synchronous_response;
    int           m_message_size;

    boost::shared_array<char> m_message_text;

    friend FO_COMMON_API void BufferToHeader(const HeaderBuffer&, MessagePacket&);
};

/** Fills in the relevant portions of \a message with the values in the buffer \a buffer. */
FO_COMMON_API void BufferToHeader(const MessagePacket::HeaderBuffer& buffer, MessagePacket& message);

/** Fills \a header_buf from the relevant portions of \a message. */
FO_COMMON_API void HeaderToBuffer(const MessagePacket& message, MessagePacket::HeaderBuffer& buffer);

bool operator==(const MessagePacket& lhs, const MessagePacket& rhs);
bool operator!=(const MessagePacket& lhs, const MessagePacket& rhs);

FO_COMMON_API void swap(MessagePacket& lhs, MessagePacket& rhs); ///< Swaps the contents of \a lhs and \a rhs.  Does not throw.


////////////////////////////////////////////////
// MessagePacket stringification
////////////////////////////////////////////////

/** Writes \a msg to \a os.  The format of the output is designed for debugging purposes. */
FO_COMMON_API std::ostream& operator<<(std::ostream& os, const MessagePacket& msg);


////////////////////////////////////////////////
// MessagePacket named ctors
////////////////////////////////////////////////

/** creates an ERROR_MSG message*/
FO_COMMON_API MessagePacket ErrorMessage(const std::string& problem, bool fatal = true);
FO_COMMON_API MessagePacket ErrorMessage(int player_id, const std::string& problem, bool fatal = true);

/** creates a HOST_SP_GAME message*/
FO_COMMON_API MessagePacket HostSPGameMessage(const SinglePlayerSetupData& setup_data);

/** creates a minimal HOST_MP_GAME message used to initiate multiplayer "lobby" setup*/
FO_COMMON_API MessagePacket HostMPGameMessage(const std::string& host_player_name);

/** creates a JOIN_GAME message.  The sender's player name and client type are sent in the message.*/
FO_COMMON_API MessagePacket JoinGameMessage(const std::string& player_name, Networking::ClientType client_type);

/** creates a HOST_ID message.  The player ID of the host is sent in the message. */
FO_COMMON_API MessagePacket HostIDMessage(int host_player_id);

/** creates a GAME_START message.  Contains the initial game state visible to player \a player_id.*/
FO_COMMON_API MessagePacket GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, CombatLogManager& combat_logs,
                                       const SupplyManager& supply, const std::map<int, PlayerInfo>& players,
                                       const GalaxySetupData& galaxy_setup_data, bool use_binary_serialization);

/** creates a GAME_START message.  Contains the initial game state visible to
  * player \a player_id.  Also includes data loaded from a saved game. */
FO_COMMON_API MessagePacket GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, CombatLogManager& combat_logs,
                                       const SupplyManager& supply,
                                       const std::map<int, PlayerInfo>& players, const OrderSet& orders,
                                       const SaveGameUIData* ui_data,
                                       const GalaxySetupData& galaxy_setup_data, bool use_binary_serialization);

/** creates a GAME_START message.  Contains the initial game state visible to
  * player \a player_id.  Also includes state string loaded from a saved game. */
FO_COMMON_API MessagePacket GameStartMessage(int player_id, bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, CombatLogManager& combat_logs,
                                       const SupplyManager& supply,
                                       const std::map<int, PlayerInfo>& players, const OrderSet& orders,
                                       const std::string* save_state_string,
                                       const GalaxySetupData& galaxy_setup_data, bool use_binary_serialization);

/** creates a HOST_SP_GAME acknowledgement message.  The \a player_id is the ID
  * of the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API MessagePacket HostSPAckMessage(int player_id);

/** creates a HOST_MP_GAME acknowledgement message.  The \a player_id is the ID
  * of the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API MessagePacket HostMPAckMessage(int player_id);

/** creates a JOIN_GAME acknowledgement message.  The \a player_id is the ID of
  * the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API MessagePacket JoinAckMessage(int player_id);

/** creates a TURN_ORDERS message. */
FO_COMMON_API MessagePacket TurnOrdersMessage(int sender, const OrderSet& orders);

/** creates a TURN_PROGRESS message. */
FO_COMMON_API MessagePacket TurnProgressMessage(MessagePacket::TurnProgressPhase phase_id, int player_id = Networking::INVALID_PLAYER_ID);

/** creates a PLAYER_STATUS message. */
FO_COMMON_API MessagePacket PlayerStatusMessage(int player_id, int about_player_id, MessagePacket::PlayerStatus player_status);

/** creates a TURN_UPDATE message. */
FO_COMMON_API MessagePacket TurnUpdateMessage(int player_id, int empire_id, int current_turn,
                                        const EmpireManager& empires, const Universe& universe,
                                        const SpeciesManager& species, CombatLogManager& combat_logs,
                                        const SupplyManager& supply,
                                        const std::map<int, PlayerInfo>& players, bool use_binary_serialization);

/** create a TURN_PARTIAL_UPDATE message. */
FO_COMMON_API MessagePacket TurnPartialUpdateMessage(int player_id, int empire_id, const Universe& universe,
                                               bool use_binary_serialization);

/** creates a CLIENT_SAVE_DATA message, including UI data but without a state string. */
FO_COMMON_API MessagePacket ClientSaveDataMessage(int sender, const OrderSet& orders, const SaveGameUIData& ui_data);

/** creates a CLIENT_SAVE_DATA message, without UI data but with a state string. */
FO_COMMON_API MessagePacket ClientSaveDataMessage(int sender, const OrderSet& orders, const std::string& save_state_string);

/** creates a CLIENT_SAVE_DATA message, without UI data and without a state string. */
FO_COMMON_API MessagePacket ClientSaveDataMessage(int sender, const OrderSet& orders);

/** creates an REQUEST_NEW_OBJECT_ID message. This message is a synchronous
    message, when sent it will wait for a reply from the server */
FO_COMMON_API MessagePacket RequestNewObjectIDMessage(int sender);

/** creates an DISPATCH_NEW_OBJECT_ID  message.  This message is sent to a
  * client who is waiting for a new object ID */
FO_COMMON_API MessagePacket DispatchObjectIDMessage(int player_id, int new_id);

/** creates an REQUEST_NEW_DESIGN_ID message. This message is a synchronous
    message, when sent it will wait for a reply from the server */
FO_COMMON_API MessagePacket RequestNewDesignIDMessage(int sender);

/** creates an DISPATCH_NEW_DESIGN_ID  message.  This message is sent to a
  * client who is waiting for a new design ID */
FO_COMMON_API MessagePacket DispatchDesignIDMessage(int player_id, int new_id);

/** creates a SAVE_GAME_INITIATE request message.  This message should only be sent by
  * the host player.*/
FO_COMMON_API MessagePacket HostSaveGameInitiateMessage(int sender, const std::string& filename);

/** creates a SAVE_GAME_DATA_REQUEST data request message.  This message should
    only be sent by the server to get game data from a client, or to respond to
    the host player requesting a save be initiated. */
FO_COMMON_API MessagePacket ServerSaveGameDataRequestMessage(int receiver, bool synchronous_response);

/** creates a SAVE_GAME_COMPLETE complete message.  This message should only be
    sent by the server to inform clients that the last initiated save has been
    completed successfully. */
FO_COMMON_API MessagePacket ServerSaveGameCompleteMessage(const std::string& save_filename, int bytes_written);

/** creates a PLAYER_CHAT, which is sent to the server, and then from the server
  * to all players, including the originating player.*/
FO_COMMON_API MessagePacket GlobalChatMessage(int sender, const std::string& msg);

/** creates a PLAYER_CHAT message, which is sent to the server, and then from
  * the server to a single recipient player */
FO_COMMON_API MessagePacket SingleRecipientChatMessage(int sender, int receiver, const std::string& msg);

/** creates a DIPLOMACY message, which is sent between players via the server to
  * declare, proposed, or accept / reject diplomatic arrangements or agreements. */
FO_COMMON_API MessagePacket DiplomacyMessage(int sender, int receiver, const DiplomaticMessage& diplo_message);

/** creates a DIPLOMATIC_STATUS message, which is sent to players by the server to
  * update them on diplomatic status changes between players. */
FO_COMMON_API MessagePacket DiplomaticStatusMessage(int receiver, const DiplomaticStatusUpdateInfo& diplo_update);

/** creates an END_GAME message used to terminate an active game. */
FO_COMMON_API MessagePacket EndGameMessage(int receiver, MessagePacket::EndGameReason reason, const std::string& reason_player_name = "");

/** creates an AI_END_GAME_ACK message used to indicate that the AI has shutdown. */
FO_COMMON_API MessagePacket AIEndGameAcknowledgeMessage(int sender);

/** creates a MODERATOR_ACTION message used to implement moderator commands. */
FO_COMMON_API MessagePacket ModeratorActionMessage(int sender, const Moderator::ModeratorAction& mod_action);

/** tells server to shut down. */
FO_COMMON_API MessagePacket ShutdownServerMessage(int sender);

/** requests previews of savefiles from server */
FO_COMMON_API MessagePacket RequestSavePreviewsMessage(int sender, std::string directory);

/** returns the savegame previews to the client */
FO_COMMON_API MessagePacket DispatchSavePreviewsMessage(int receiver, const PreviewInformation& preview);

/** requests combat logs from server */
FO_COMMON_API MessagePacket RequestCombatLogsMessage(int sender, const std::vector<int>& ids);

/** returns combat logs to the client */
FO_COMMON_API MessagePacket DispatchCombatLogsMessage(int receiver, const std::vector<std::pair<int, const CombatLog>>& logs);

////////////////////////////////////////////////
// Multiplayer Lobby MessagePacket named ctors
////////////////////////////////////////////////

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the 
    server, then to other users.  Clients must send all such updates to the server directly; the server
    will send updates to the other clients as needed.*/
FO_COMMON_API MessagePacket LobbyUpdateMessage(int sender, const MultiplayerLobbyData& lobby_data);

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the users.  
    This message should only be sent by the server.*/
FO_COMMON_API MessagePacket ServerLobbyUpdateMessage(int receiver, const MultiplayerLobbyData& lobby_data);

/** creates an LOBBY_CHAT message containing a chat string to be broadcast to player \a receiver, or all players if \a
    receiver is Networking::INVALID_PLAYER_ID. Note that the receiver of this message is always the server.*/
FO_COMMON_API MessagePacket LobbyChatMessage(int sender, int receiver, const std::string& text);

/** creates an LOBBY_CHAT message containing a chat string from \a sender to be displayed in \a receiver's lobby dialog.
    This message should only be sent by the server.*/
FO_COMMON_API MessagePacket ServerLobbyChatMessage(int sender, int receiver, const std::string& text);

/** creates a START_MP_GAME used to finalize the multiplayer lobby setup.*/
FO_COMMON_API MessagePacket StartMPGameMessage(int player_id);


////////////////////////////////////////////////
// MessagePacket data extractors
////////////////////////////////////////////////

FO_COMMON_API void ExtractErrorMessageData(const MessagePacket& msg, std::string& problem, bool& fatal);

FO_COMMON_API void ExtractHostMPGameMessageData(const MessagePacket& msg, std::string& host_player_name,
                                                std::string& client_version_string);

FO_COMMON_API void ExtractLobbyUpdateMessageData(const MessagePacket& msg, MultiplayerLobbyData& lobby_data);

FO_COMMON_API void ExtractGameStartMessageData(const MessagePacket& msg, bool& single_player_game, int& empire_id,
                                               int& current_turn, EmpireManager& empires, Universe& universe,
                                               SpeciesManager& species, CombatLogManager& combat_logs,
                                               SupplyManager& supply,
                                               std::map<int, PlayerInfo>& players, OrderSet& orders,
                                               bool& loaded_game_data, bool& ui_data_available,
                                               SaveGameUIData& ui_data, bool& save_state_string_available,
                                               std::string& save_state_string, GalaxySetupData& galaxy_setup_data);

FO_COMMON_API void ExtractJoinGameMessageData(const MessagePacket& msg, std::string& player_name,
                                              Networking::ClientType& client_type,
                                              std::string& version_string);

FO_COMMON_API void ExtractTurnOrdersMessageData(const MessagePacket& msg, OrderSet& orders);

FO_COMMON_API void ExtractTurnUpdateMessageData(const MessagePacket& msg, int empire_id, int& current_turn, EmpireManager& empires,
                                                Universe& universe, SpeciesManager& species, CombatLogManager& combat_logs,
                                                SupplyManager& supply, std::map<int, PlayerInfo>& players);

FO_COMMON_API void ExtractTurnPartialUpdateMessageData(const MessagePacket& msg, int empire_id, Universe& universe);

FO_COMMON_API void ExtractClientSaveDataMessageData(const MessagePacket& msg, OrderSet& orders,
                                                    bool& ui_data_available, SaveGameUIData& ui_data,
                                                    bool& save_state_string_available,
                                                    std::string& save_state_string);

FO_COMMON_API void ExtractTurnProgressMessageData(const MessagePacket& msg, MessagePacket::TurnProgressPhase& phase_id);

FO_COMMON_API void ExtractPlayerStatusMessageData(const MessagePacket& msg, int& about_player_id, MessagePacket::PlayerStatus& status);

FO_COMMON_API void ExtractHostSPGameMessageData(const MessagePacket& msg, SinglePlayerSetupData& setup_data, std::string& client_version_string);

FO_COMMON_API void ExtractEndGameMessageData(const MessagePacket& msg, MessagePacket::EndGameReason& reason, std::string& reason_player_name);

FO_COMMON_API void ExtractModeratorActionMessageData(const MessagePacket& msg, Moderator::ModeratorAction*& action);

FO_COMMON_API void ExtractDiplomacyMessageData(const MessagePacket& msg, DiplomaticMessage& diplo_message);

FO_COMMON_API void ExtractDiplomaticStatusMessageData(const MessagePacket& msg, DiplomaticStatusUpdateInfo& diplo_update);

FO_COMMON_API void ExtractRequestSavePreviewsMessageData(const MessagePacket& msg, std::string& directory);

FO_COMMON_API void ExtractDispatchSavePreviewsMessageData(const MessagePacket& msg, PreviewInformation& previews);

FO_COMMON_API void ExtractServerSaveGameCompleteMessageData(const MessagePacket& msg, std::string& save_filename, int& bytes_written);

FO_COMMON_API void ExtractRequestCombatLogsMessageData(const MessagePacket& msg, std::vector<int>& ids);

FO_COMMON_API void ExtractDispatchCombatLogsMessageData(const MessagePacket& msg, std::vector<std::pair<int, CombatLog>>& logs);

#endif // _Message_h_
