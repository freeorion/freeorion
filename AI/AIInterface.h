#ifndef AI_INTERFACE
#define AI_INTERFACE

#include <string>
#include <vector>

#include <boost/python/str.hpp>

struct CombatData;
class Empire;
class Tech;
class DiplomaticMessage;
class Universe;
struct DiplomaticStatusUpdateInfo;
struct GalaxySetupData;

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
    virtual const std::string& GetSaveStateString();

    /** @brief Set the aggressiveness of this AI
     *
     * The AI should change their behaviour when setting another aggression
     * level.
     *
     * @param aggr The new aggression level.  The value should be one of
     *      ::Aggression.
     */
    void SetAggression(int aggr);

protected:
    /** @brief The current aggressiveness of this AI
     *
     * The value should be one of ::Aggression.
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

    /** @brief Return the ::Universe known to this client
     *
     * @return A constant reference to the single ::Universe instance
     *      representing the known universe of this client.
     */
    const Universe& GetUniverse();

    /** @brief Return the ::Tech identified by @a name
     *
     * @param name The identifying name of the requested ::Tech.
     *
     * @return A pointer to the ::Tech matching @a name or NULL if no ::Tech
     *      with that name was found.
     */
    const Tech* GetTech(const std::string& name);

    /** @brief Return the current game turn
     *
     * @return The number representing the current game turn.
     */
    int CurrentTurn();

    /** @brief Return the OptionsDB option @a option
     *
     * @return Return the OptionsDB option @a option or None if not set.
     * @ throw boost::bad_any_cast if option exists but is not a string
     */
    boost::python::object GetOptionsDBOptionStr(std::string const &option);

    /** @brief Return the OptionsDB option @a option
     *
     * @return Return the OptionsDB option @a option or None if not set.
     * @ throw boost::bad_any_cast if option exists but is not an int
     */
    boost::python::object GetOptionsDBOptionInt(std::string const &option);

    /** @brief Return the OptionsDB option @a option
     *
     * @return Return the OptionsDB option @a option or None if not set.
     * @ throw boost::bad_any_cast if option exists but is not bool
     */
    boost::python::object GetOptionsDBOptionBool(std::string const &option);

    /** @brief Return the OptionsDB option @a option
     *
     * @return Return the OptionsDB option @a option or None if not set.
     * @ throw boost::bad_any_cast if option exists but is not a double
     */
    boost::python::object GetOptionsDBOptionDouble(std::string const &option);

    /** @brief Return the canonical AI directory path
     *
     * The value depends on the ::OptionsDB `resource-dir` and `ai-path` keys.
     *
     * @return The canonical path pointing to the directory containing all
     *      python AI scripts.
     */
    std::string GetAIDir();

    /** @brief Return the ::GalaxySetupData of this game
     *
     * @return A reference to the ::GalaxySetupData used in this game session.
     */
    const GalaxySetupData& GetGalaxySetupData();

    /** @} */

    /** @name Game state prediction */ /** @{ */

    /** @brief Initialize and update game state based last turn update
     *
     * Initialize and update game state by updating this client
     *
     * * Global ::Meter
     * * ::ResourcePool
     * * ::ProductionQueue
     * * ::ResearchQueue
     *
     * instances based on the latest turn update.
     */
    void InitTurn();

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

    int IssueRenameOrder(int object_id, const std::string& new_name);
    int IssueScrapOrder(const std::vector<int>& object_ids);
    int IssueScrapOrder(int object_id);
    int IssueFleetMoveOrder(int fleet_id, int destination_id);
    int IssueNewFleetOrder(const std::string& fleet_name, const std::vector<int>& ship_ids);
    int IssueNewFleetOrder(const std::string& fleet_name, int ship_id);
    int IssueFleetTransferOrder(int ship_id, int new_fleet_id);
    int IssueColonizeOrder(int ship_id, int planet_id);
    int IssueInvadeOrder(int ship_id, int planet_id);
    int IssueBombardOrder(int ship_id, int planet_id);
    int IssueDeleteFleetOrder();
    int IssueAggressionOrder(int object_id, bool aggressive);
    int IssueGiveObjectToEmpireOrder(int object_id, int recipient_id);

    int IssueChangeFocusOrder(int planet_id, const std::string& focus);

    int IssueEnqueueTechOrder(const std::string& tech_name, int position);
    int IssueDequeueTechOrder(const std::string& tech_name);

    int IssueEnqueueBuildingProductionOrder(const std::string& item_name, int location_id);
    int IssueEnqueueShipProductionOrder(int design_id, int location_id);
    int IssueChangeProductionQuantityOrder(int queue_index, int new_quantity, int new_blocksize);
    int IssueRequeueProductionOrder(int old_queue_index, int new_queue_index);
    int IssueDequeueProductionOrder(int queue_index);

    int IssueCreateShipDesignOrder(const std::string& name, const std::string& description,
                                   const std::string& hull, const std::vector<std::string> parts,
                                   const std::string& graphic, const std::string& model, bool nameDescInStringTable);

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
