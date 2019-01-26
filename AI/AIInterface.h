#ifndef AI_INTERFACE
#define AI_INTERFACE

#include <string>
#include <vector>

#ifdef FREEORION_MACOSX
// Bugfix for https://github.com/freeorion/freeorion/issues/1228

// The problem on OSX is that the boost/python/str.hpp redefines toupper() and
// similar functions if they are not already defined.

// This includes iostream before the boost/python/str.hpp to fix this issue.
// If the subsequent #include <boost/python/str.hpp> is removed then so can this workaround.
#include <iostream>
#endif

#include <boost/python/str.hpp>

struct CombatData;
class Empire;
class Tech;
class DiplomaticMessage;
class Universe;
struct DiplomaticStatusUpdateInfo;
struct GalaxySetupData;
class OrderSet;

/** @brief Base class allowing AI to recieve basic game events.
 *
 * A subclass can overwrite any of the defined methods to implement a reaction
 * behaviour whenever an game event occurs.
 */
class AIBase {
public:
    virtual ~AIBase();

    /** @brief Call when the server has sent a new turn update.
     *
     * The AI subclass should review the new gamestate and send orders for
     * this turn.
     */
    virtual void GenerateOrders();

    /** @brief Called when another player sends a chat message to this AI.
     *
     * The AI subclass should respond or react to the message in a meaningful
     * way.
     *
     * @param sender_id The player identifier representing the player, who sent
     *      the message.
     * @param msg The text body of the sent message.
     */
    virtual void HandleChatMessage(int sender_id, const std::string& msg);

    /** @brief Called when another player sends a diplomatic message that
     *      affects this player
     *
     * The AI subclass should respond or react to the message in a meaningful
     * way.
     *
     * @param msg The diplomatic message sent.
     */
    virtual void HandleDiplomaticMessage(const DiplomaticMessage& msg);

    /** @brief Called when two empires diplomatic status changes
     *
     * The AI subclass should respond or react to the change in a meaningful
     * way.
     *
     * @param u The diplomatic status changed.
     */
    virtual void HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u);

    /** @brief Called when a new game (not loaded) is started
     *
     * The AI subclass should clear its state and prepare to start for a new
     * game.
     */
    virtual void StartNewGame();

    /** @brief Called when a game is loaded from a save
     *
     * The AI subclass should extract any state information stored in
     * @a save_state so it is able to continue generating orders when
     * asked to do so.
     *
     * @param save_state The serialized state information from a previous game
     *      run.
     */
    virtual void ResumeLoadedGame(const std::string& save_state);

    /** @brief Called when the server is saving the game
     *
     * The AI should store any state information it will need to resume at any
     * later time, and return this information.
     *
     * @return The serialized state information of the game running.
     */
    virtual const std::string& GetSaveStateString() const;

    /** @brief Set the aggressiveness of this AI
     *
     * The AI should change their behaviour when setting another aggression
     * level.
     *
     * @param aggr The new aggression level.  The value should be one of
     *      Aggression.
     */
    void SetAggression(int aggr);

protected:
    /** @brief The current aggressiveness of this AI
     *
     * The value should be one of Aggression.
     */
    int m_aggression;
};

/** @brief List of functions for AIs to query the game state, interact with
 *      other players and issue game orders.
 */
namespace AIInterface {
    /** @name Game state accessors */ /** @{ */

    /** @brief Return the player name of this client
     *
     * @return An UTF-8 encoded and NUL terminated string containing the player
     *      name of this client.
     */
    const std::string& PlayerName();

    /** @brief Return the player name of the client identified by @a player_id
     *
     * @param player_id An client identifier.
     *
     * @return An UTF-8 encoded and NUL terminated string containing the player
     *      name of this client or an empty string the player is not known or
     *      does not exist.
     */
    const std::string& PlayerName(int player_id);

    /** @brief Return the player identifier of this client
     *
     * @return The player identifier of this client as assigned by the server.
     */
    int PlayerID();

    /** @brief Return the player identfier of the player controlling the empire
     *      @a empire_id
     *
     * @param empire_id An empire identifier representing an empire.
     *
     * @return The player identifier of the client controlling the empire.
     */
    int EmpirePlayerID(int empire_id);

    /** @brief Return all player identifiers that are in game
     *
     * @return A vector containing the identifiers of all players.
     */
    std::vector<int> AllPlayerIDs();

    /** @brief Return if the player identified by @a player_id is an AI
     *
     * @param player_id An client identifier.
     *
     * @return True if the player is an AI, false if not.
     */
    bool PlayerIsAI(int player_id);

    /** @brief Return if the player identified by @a player_id is the game
     *      host
     *
     * @param player_id An client identifier.
     *
     * @return True if the player is the game host, false if not.
     */
    bool PlayerIsHost(int player_id);

    /** @brief Return the empire identifier of the empire this client controls
     *
     * @return An empire identifier.
     */
    int EmpireID();

    /** @brief Return the empire identifier of the empire @a player_id controls
     *
     * @param player_id An client identifier.
     *
     * @return An empire identifier.
     */
    int PlayerEmpireID(int player_id);

    /** @brief Return all empire identifiers that are in game
     *
     * @return A vector containing the identifiers of all empires.
     */
    std::vector<int> AllEmpireIDs();

    /** @brief Return the ::Empire this client controls
     *
     * @return A pointer to the Empire instance this client has the control
     *      over.
     */
    const Empire* GetEmpire();

    /** @brief Return the ::Empire identified by @a empire_id
     *
     * @param empire_id An empire identifier.
     *
     * @return A pointer to the Empire instance identified by @a empire_id.
     */
    const Empire* GetEmpire(int empire_id);

    /** @brief Return the ::GalaxySetupData of this game
     *
     * @return A reference to the ::GalaxySetupData used in this game session.
     */
    const GalaxySetupData& GetGalaxySetupData();

    /** @} */

    /** @name Game state prediction */ /** @{ */

    /** @brief Initialize and update the ::Universe ::Meter s
     *
     * @see ::Universe::InitMeterEstimatesAndDiscrepancies
     */
    void InitMeterEstimatesAndDiscrepancies();

    /** @brief Set ::Universe ::Meter instances to their estimated values as
     *      if the next turn processing phase were done
     *
     * @param pretend_to_own_unowned_planets When set to true pretend during
     *      calculation that this clients Empire owns all known uncolonized
     *      planets.  The unowned planets MAX ::Meter values will contain the
     *      estimated value for those planets.
     */
    void UpdateMeterEstimates(bool pretend_to_own_unowned_planets = false);

    /** @brief Calculate resource generation, update ::ResourcePool s and
     *      change queue spending.
     */
    void UpdateResourcePools();

    void UpdateResearchQueue();
    void UpdateProductionQueue();

    /** @} */

    /** \name Issuing orders */ /** @{ */
    OrderSet& IssuedOrders();

    int IssueEnqueueTechOrder(const std::string& tech_name, int position);
    int IssueDequeueTechOrder(const std::string& tech_name);

    int IssueEnqueueBuildingProductionOrder(const std::string& item_name, int location_id);
    int IssueEnqueueShipProductionOrder(int design_id, int location_id);
    int IssueChangeProductionQuantityOrder(int queue_index, int new_quantity, int new_blocksize);
    int IssueRequeueProductionOrder(int old_queue_index, int new_queue_index);
    int IssueDequeueProductionOrder(int queue_index);
    int IssuePauseProductionOrder(int queue_index, bool new_paused);
    int IssueAllowStockpileProductionOrder(int queue_index, bool new_use_stockpile);

    int IssueCreateShipDesignOrder(const std::string& name, const std::string& description,
                                   const std::string& hull, const std::vector<std::string> parts,
                                   const std::string& graphic, const std::string& model, bool name_desc_in_stringtable);

    void SendPlayerChatMessage(int recipient_player_id, const std::string& message_text);
    void SendDiplomaticMessage(const DiplomaticMessage& diplo_message);

    /** @brief Notify server that all orders for this client are given */
    void DoneTurn();

    /** @} */

    /** Logging */ /** @{ */

    /** @brief Log @a log_text with DEBUG level
     *
     * Writes the given text and appends a new line
     *
     * @param log_text The text that should be written to the log system.
     */
    void LogOutput(const std::string& log_text);

    /** @brief Log @a log_text with ERROR level
     *
     * Writes the given text and appends a new line
     *
     * @param log_text The text that should be written to the log system.
     */
    void ErrorOutput(const std::string& log_text);

    /** @} */
};

#endif
