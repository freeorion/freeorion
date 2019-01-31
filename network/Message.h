#ifndef _Message_h_
#define _Message_h_

#include "Networking.h"
#include "../util/Export.h"
#include <GG/Enum.h>

#include <boost/shared_array.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/uuid/uuid.hpp>

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <array>
#include <set>
#include <map>
#include <string>
#include <vector>


enum class LogLevel;
class EmpireManager;
class SupplyManager;
class SpeciesManager;
struct CombatLog;
class CombatLogManager;
class Message;
struct MultiplayerLobbyData;
struct ChatHistoryEntity;
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
class FO_COMMON_API Message {
public:
    enum Parts : size_t {TYPE = 0, SIZE, Parts_Count};

    typedef std::array<int, Parts::Parts_Count> HeaderBuffer;

    constexpr static size_t HeaderBufferSize =
        std::tuple_size<HeaderBuffer>::value* sizeof(HeaderBuffer::value_type);

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
        LOBBY_EXIT,             ///< sent to server by clients when a player leaves the multiplayer lobby, or by server to clients when a player leaves the multiplayer lobby
        START_MP_GAME,          ///< sent to server (by the "host" client only) when the settings in the MP lobby are satisfactory and it is time to start the game
        SAVE_GAME_INITIATE,     ///< sent to server (by the "host" client only) when a game is to be saved
        SAVE_GAME_COMPLETE,     ///< sent to clients (by the "host" client only) when a game has been saved and written to disk
        LOAD_GAME,              ///< sent to server (by the "host" client only) when a game is to be loaded
        GAME_START,             ///< sent to each client before the first turn of a new or newly loaded game, instead of a TURN_UPDATE
        TURN_UPDATE,            ///< sent to a client when the server updates the client Universes and Empires, and sends the SitReps each turn; indicates to the receiver that a new turn has begun
        TURN_PARTIAL_UPDATE,    ///< sent to a client when the server updates part of the client gamestate after partially processing a turn, such as after fleet movement but before the rest of the turn is processed.  Does NOT indicate a new turn has begun.
        TURN_ORDERS,            ///< sent to the server by a client that has orders to be processed at the end of a turn
        TURN_PROGRESS,          ///< sent to clients to display a turn progress message
        PLAYER_STATUS,          ///< sent to clients to inform them that a player has some status, such as having finished playing a turn and submitted orders, or is resolving combat, or is playing a turn normally
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
        DISPATCH_SAVE_PREVIEWS, ///< sent by host to client to provide the savegame previews
        REQUEST_COMBAT_LOGS,    ///< sent by client to request combat logs
        DISPATCH_COMBAT_LOGS,   ///< sent by host to client to provide combat logs
        LOGGER_CONFIG,          ///< sent by host to server and server to ais to configure logging
        CHECKSUM,               ///< sent by host to clients to specify what the parsed content checksum values should be
        AUTH_REQUEST,           ///< sent by server to client if choosed player_name require authentiation
        AUTH_RESPONSE,          ///< sent by client to server to provide password or other credentials
        CHAT_HISTORY,           ///< sent by server to client to show previous messages
        SET_AUTH_ROLES,         ///< sent by server to client to set authorization roles
        ELIMINATE_SELF,         ///< sent by client to server if the player wants to resign
        UNREADY,                ///< sent by client to server to revoke ready state of turn orders and sent by server to client to acknowledge it
        TURN_PARTIAL_ORDERS     ///< sent to the server by a client that has changes in orders to be stored
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

    /// \todo change to EmpireStatus on compatibility breakage
    GG_CLASS_ENUM(PlayerStatus,
        PLAYING_TURN,           ///< empire is playing a turn, on the galax map
        WAITING                 ///< empire is waiting for others to submit orders, to resolve combats, or for turn processing to complete
    )

    GG_CLASS_ENUM(EndGameReason,
        LOCAL_CLIENT_DISCONNECT,///< the local player's client networking detected a disconnection from the server
        PLAYER_DISCONNECT,      ///< an active player (not an observer) was disconnected
    )

    /** \name Structors */ //@{
    Message();

    Message(MessageType message_type,
            const std::string& text);
    //@}

    /** \name Accessors */ //@{
    MessageType Type() const;               ///< Returns the type of the message.
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
    int           m_message_size;

    boost::shared_array<char> m_message_text;

    friend FO_COMMON_API void BufferToHeader(const HeaderBuffer&, Message&);
};

/** Fills in the relevant portions of \a message with the values in the buffer \a buffer. */
FO_COMMON_API void BufferToHeader(const Message::HeaderBuffer& buffer, Message& message);

/** Fills \a header_buf from the relevant portions of \a message. */
FO_COMMON_API void HeaderToBuffer(const Message& message, Message::HeaderBuffer& buffer);

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
FO_COMMON_API Message ErrorMessage(const std::string& problem, bool fatal = true,
                                   int player_id = Networking::INVALID_PLAYER_ID);

/** creates a HOST_SP_GAME message*/
FO_COMMON_API Message HostSPGameMessage(const SinglePlayerSetupData& setup_data);

/** creates a minimal HOST_MP_GAME message used to initiate multiplayer "lobby" setup*/
FO_COMMON_API Message HostMPGameMessage(const std::string& host_player_name);

/** creates a JOIN_GAME message.  The sender's player name, client type, and cookie are sent in the message.*/
FO_COMMON_API Message JoinGameMessage(const std::string& player_name,
                                      Networking::ClientType client_type,
                                      boost::uuids::uuid cookie);

/** creates a HOST_ID message.  The player ID of the host is sent in the message. */
FO_COMMON_API Message HostIDMessage(int host_player_id);

/** creates a GAME_START message.  Contains the initial game state visible to the player.*/
FO_COMMON_API Message GameStartMessage(bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, CombatLogManager& combat_logs,
                                       const SupplyManager& supply, const std::map<int, PlayerInfo>& players,
                                       const GalaxySetupData& galaxy_setup_data, bool use_binary_serialization);

/** creates a GAME_START message.  Contains the initial game state visible to
  * the player.  Also includes data loaded from a saved game. */
FO_COMMON_API Message GameStartMessage(bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, CombatLogManager& combat_logs,
                                       const SupplyManager& supply,
                                       const std::map<int, PlayerInfo>& players, const OrderSet& orders,
                                       const SaveGameUIData* ui_data,
                                       const GalaxySetupData& galaxy_setup_data, bool use_binary_serialization);

/** creates a GAME_START message.  Contains the initial game state visible to
  * the player.  Also includes state string loaded from a saved game. */
FO_COMMON_API Message GameStartMessage(bool single_player_game, int empire_id, int current_turn,
                                       const EmpireManager& empires, const Universe& universe,
                                       const SpeciesManager& species, CombatLogManager& combat_logs,
                                       const SupplyManager& supply,
                                       const std::map<int, PlayerInfo>& players, const OrderSet& orders,
                                       const std::string* save_state_string,
                                       const GalaxySetupData& galaxy_setup_data, bool use_binary_serialization);

/** creates a HOST_SP_GAME acknowledgement message.  The \a player_id is the ID
  * of the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API Message HostSPAckMessage(int player_id);

/** creates a HOST_MP_GAME acknowledgement message.  The \a player_id is the ID
  * of the receiving player.  This message should only be sent by the server.*/
FO_COMMON_API Message HostMPAckMessage(int player_id);

/** creates a JOIN_GAME acknowledgement message.  The \a player_id is the ID of
  * the receiving player and \a cookie is a token to quickly authenticate player.
  * This message should only be sent by the server.*/
FO_COMMON_API Message JoinAckMessage(int player_id, boost::uuids::uuid cookie);

/** creates a TURN_ORDERS message, including UI data but without a state string. */
FO_COMMON_API Message TurnOrdersMessage(const OrderSet& orders, const SaveGameUIData& ui_data);

/** creates a TURN_ORDERS message, without UI data but with a state string. */
FO_COMMON_API Message TurnOrdersMessage(const OrderSet& orders, const std::string& save_state_string);

/** creates a TURN_PARTIAL_ORDERS message with orders changes. */
FO_COMMON_API Message TurnPartialOrdersMessage(const std::pair<OrderSet, std::set<int>>& orders_updates);

/** creates a TURN_PROGRESS message. */
FO_COMMON_API Message TurnProgressMessage(Message::TurnProgressPhase phase_id);

/** creates a PLAYER_STATUS message. */
FO_COMMON_API Message PlayerStatusMessage(int about_player_id,
                                          Message::PlayerStatus player_status,
                                          int about_empire_id);

/** creates a TURN_UPDATE message. */
FO_COMMON_API Message TurnUpdateMessage(int empire_id, int current_turn,
                                        const EmpireManager& empires, const Universe& universe,
                                        const SpeciesManager& species, CombatLogManager& combat_logs,
                                        const SupplyManager& supply,
                                        const std::map<int, PlayerInfo>& players, bool use_binary_serialization);

/** create a TURN_PARTIAL_UPDATE message. */
FO_COMMON_API Message TurnPartialUpdateMessage(int empire_id, const Universe& universe,
                                               bool use_binary_serialization);

/** creates a SAVE_GAME_INITIATE request message.  This message should only be sent by
  * the host player.*/
FO_COMMON_API Message HostSaveGameInitiateMessage(const std::string& filename);

/** creates a SAVE_GAME_COMPLETE complete message.  This message should only be
    sent by the server to inform clients that the last initiated save has been
    completed successfully. */
FO_COMMON_API Message ServerSaveGameCompleteMessage(const std::string& save_filename, int bytes_written);

/** creates a DIPLOMACY message, which is sent between players via the server to
  * declare, proposed, or accept / reject diplomatic arrangements or agreements. */
FO_COMMON_API Message DiplomacyMessage(const DiplomaticMessage& diplo_message);

/** creates a DIPLOMATIC_STATUS message, which is sent to players by the server to
  * update them on diplomatic status changes between players. */
FO_COMMON_API Message DiplomaticStatusMessage(const DiplomaticStatusUpdateInfo& diplo_update);

/** creates an END_GAME message used to terminate an active game. */
FO_COMMON_API Message EndGameMessage(Message::EndGameReason reason, const std::string& reason_player_name = "");

/** creates an AI_END_GAME_ACK message used to indicate that the AI has shutdown. */
FO_COMMON_API Message AIEndGameAcknowledgeMessage();

/** creates a MODERATOR_ACTION message used to implement moderator commands. */
FO_COMMON_API Message ModeratorActionMessage(const Moderator::ModeratorAction& mod_action);

/** tells server to shut down. */
FO_COMMON_API Message ShutdownServerMessage();

/** requests previews of savefiles from server */
FO_COMMON_API Message RequestSavePreviewsMessage(std::string relative_directory);

/** returns the savegame previews to the client */
FO_COMMON_API Message DispatchSavePreviewsMessage(const PreviewInformation& preview);

/** requests combat logs from server */
FO_COMMON_API Message RequestCombatLogsMessage(const std::vector<int>& ids);

/** returns combat logs to the client */
FO_COMMON_API Message DispatchCombatLogsMessage(const std::vector<std::pair<int, const CombatLog>>& logs);

/** Sends logger configuration details to server or ai process. */
FO_COMMON_API Message LoggerConfigMessage(int sender, const std::set<std::tuple<std::string, std::string, LogLevel>>& options);

////////////////////////////////////////////////
// Multiplayer Lobby Message named ctors
////////////////////////////////////////////////

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the
    server, then to other users.  Clients must send all such updates to the server directly; the server
    will send updates to the other clients as needed.*/
FO_COMMON_API Message LobbyUpdateMessage(const MultiplayerLobbyData& lobby_data);

/** creates an LOBBY_UPDATE message containing changes to the lobby settings that need to propogate to the users.
    This message should only be sent by the server.*/
FO_COMMON_API Message ServerLobbyUpdateMessage(const MultiplayerLobbyData& lobby_data);

/** creates an CHAT_HISTORY message containing latest chat messages.
    This message should only be sent by the server.*/
FO_COMMON_API Message ChatHistoryMessage(const std::vector<std::reference_wrapper<const ChatHistoryEntity>>& chat_history);

/** creates an PLAYER_CHAT message containing a chat string to be broadcast to player \a receiver, or all players if \a
    receiver is Networking::INVALID_PLAYER_ID. Note that the receiver of this message is always the server.*/
FO_COMMON_API Message PlayerChatMessage(const std::string& text, int receiver = Networking::INVALID_PLAYER_ID);

/** creates an PLAYER_CHAT message containing a chat string from \a sender at \a timestamp to be displayed in chat.
    This message should only be sent by the server.*/
FO_COMMON_API Message ServerPlayerChatMessage(int sender,
                                              const boost::posix_time::ptime& timestamp,
                                              const std::string& text);

/** creates a START_MP_GAME used to finalize the multiplayer lobby setup.*/
FO_COMMON_API Message StartMPGameMessage();

/** creates a CHECKSUM message containing checksums of parsed content. */
FO_COMMON_API Message ContentCheckSumMessage();

/** creates a AUTH_REQUEST message containing \a player_name to login and \a auth additional authentication data. */
FO_COMMON_API Message AuthRequestMessage(const std::string& player_name, const std::string& auth);

/** creates a AUTH_RESPONSE message containing \a player_name to login and \a auth credentials. */
FO_COMMON_API Message AuthResponseMessage(const std::string& player_name, const std::string& auth);

/** notifies client about changes in his authorization \a roles. */
FO_COMMON_API Message SetAuthorizationRolesMessage(const Networking::AuthRoles& roles);

/** creates a ELIMINATE_SELF message to resign from the game. */
FO_COMMON_API Message EliminateSelfMessage();

/** creates a UNREADY message to revoke ready state of turn orders or acknowledge it */
FO_COMMON_API Message UnreadyMessage();

////////////////////////////////////////////////
// Message data extractors
////////////////////////////////////////////////

FO_COMMON_API void ExtractErrorMessageData(const Message& msg, int& player_id, std::string& problem, bool& fatal);

FO_COMMON_API void ExtractHostMPGameMessageData(const Message& msg, std::string& host_player_name,
                                                std::string& client_version_string);

FO_COMMON_API void ExtractLobbyUpdateMessageData(const Message& msg, MultiplayerLobbyData& lobby_data);

FO_COMMON_API void ExtractChatHistoryMessage(const Message& msg, std::vector<ChatHistoryEntity>& chat_history);

FO_COMMON_API void ExtractPlayerChatMessageData(const Message& msg, int& receiver, std::string& data);

FO_COMMON_API void ExtractServerPlayerChatMessageData(const Message& msg,
                                                      int& sender,
                                                      boost::posix_time::ptime& timestamp,
                                                      std::string& data);

FO_COMMON_API void ExtractGameStartMessageData(const Message& msg, bool& single_player_game, int& empire_id,
                                               int& current_turn, EmpireManager& empires, Universe& universe,
                                               SpeciesManager& species, CombatLogManager& combat_logs,
                                               SupplyManager& supply,
                                               std::map<int, PlayerInfo>& players, OrderSet& orders,
                                               bool& loaded_game_data, bool& ui_data_available,
                                               SaveGameUIData& ui_data, bool& save_state_string_available,
                                               std::string& save_state_string, GalaxySetupData& galaxy_setup_data);

FO_COMMON_API void ExtractJoinGameMessageData(const Message& msg, std::string& player_name,
                                              Networking::ClientType& client_type,
                                              std::string& version_string,
                                              boost::uuids::uuid& cookie);

FO_COMMON_API void ExtractJoinAckMessageData(const Message& msg, int& player_id,
                                             boost::uuids::uuid& cookie);

FO_COMMON_API void ExtractTurnOrdersMessageData(const Message& msg,
                                                OrderSet& orders,
                                                bool& ui_data_available,
                                                SaveGameUIData& ui_data,
                                                bool& save_state_string_available,
                                                std::string& save_state_string);

FO_COMMON_API void ExtractTurnPartialOrdersMessageData(const Message& msg, OrderSet& added, std::set<int>& deleted);

FO_COMMON_API void ExtractTurnUpdateMessageData(const Message& msg, int empire_id, int& current_turn, EmpireManager& empires,
                                                Universe& universe, SpeciesManager& species, CombatLogManager& combat_logs,
                                                SupplyManager& supply, std::map<int, PlayerInfo>& players);

FO_COMMON_API void ExtractTurnPartialUpdateMessageData(const Message& msg, int empire_id, Universe& universe);

FO_COMMON_API void ExtractTurnProgressMessageData(const Message& msg, Message::TurnProgressPhase& phase_id);

FO_COMMON_API void ExtractPlayerStatusMessageData(const Message& msg,
                                                  int& about_player_id,
                                                  Message::PlayerStatus& status,
                                                  int& about_empire_id);

FO_COMMON_API void ExtractHostSPGameMessageData(const Message& msg, SinglePlayerSetupData& setup_data, std::string& client_version_string);

FO_COMMON_API void ExtractEndGameMessageData(const Message& msg, Message::EndGameReason& reason, std::string& reason_player_name);

FO_COMMON_API void ExtractModeratorActionMessageData(const Message& msg, Moderator::ModeratorAction*& action);

FO_COMMON_API void ExtractDiplomacyMessageData(const Message& msg, DiplomaticMessage& diplo_message);

FO_COMMON_API void ExtractDiplomaticStatusMessageData(const Message& msg, DiplomaticStatusUpdateInfo& diplo_update);

FO_COMMON_API void ExtractRequestSavePreviewsMessageData(const Message& msg, std::string& directory);

FO_COMMON_API void ExtractDispatchSavePreviewsMessageData(const Message& msg, PreviewInformation& previews);

FO_COMMON_API void ExtractServerSaveGameCompleteMessageData(const Message& msg, std::string& save_filename, int& bytes_written);

FO_COMMON_API void ExtractRequestCombatLogsMessageData(const Message& msg, std::vector<int>& ids);

FO_COMMON_API void ExtractDispatchCombatLogsMessageData(const Message& msg, std::vector<std::pair<int, CombatLog>>& logs);

FO_COMMON_API void ExtractLoggerConfigMessageData(const Message& msg, std::set<std::tuple<std::string, std::string, LogLevel>>& options);

FO_COMMON_API void ExtractContentCheckSumMessageData(const Message& msg, std::map<std::string, unsigned int>& checksums);

FO_COMMON_API void ExtractAuthRequestMessageData(const Message& msg, std::string& player_name, std::string& auth);

FO_COMMON_API void ExtractAuthResponseMessageData(const Message& msg, std::string& player_name, std::string& auth);

FO_COMMON_API void ExtractSetAuthorizationRolesMessage(const Message &msg, Networking::AuthRoles& roles);

#endif // _Message_h_
