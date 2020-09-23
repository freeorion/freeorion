#ifndef _ClientApp_h_
#define _ClientApp_h_

#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../network/Message.h"
#include "../universe/Species.h"
#include "../universe/Universe.h"
#include "../util/OrderSet.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

class ClientNetworking;

/** \brief Abstract base class for the application framework classes
 *
 * The static functions are designed to give both types of client (which are
 * very different) a unified interface.  This allows code in either type of
 * client app to handle Messages and gain access to the data structures
 * common to both apps, without worrying about which type of app the code is
 * being run in.
 */
class ClientApp : public IApp {
protected:
    ClientApp();

public:
    ClientApp(const ClientApp&) = delete;
    ClientApp(ClientApp&&) = delete;
    ~ClientApp() override = default;

    const ClientApp& operator=(const ClientApp&) = delete;
    ClientApp& operator=(ClientApp&&) = delete;

    /** @brief Return the player identifier of this client
     *
     * @return The player identifier of this client as assigned by the server.
     */
    int PlayerID() const;

    /** @brief Return the empire identifier of the empire this client controls
     *
     * @return An empire identifier.
     */
    int EmpireID() const;

    /** @brief Return the current game turn
     *
     * @return The number representing the current game turn.
     */
    int CurrentTurn() const override;

    /** @brief Return the player identfier of the player controlling the empire
     *      @a empire_id
     *
     * @param empire_id An empire identifier representing an empire.
     *
     * @return The player identifier of the client controlling the empire.
     */
    int EmpirePlayerID(int empire_id) const;

    /** @brief Return the players in game as ::PlayerInfo map
     *
     * @return Return a map containing ::PlayerInfo instances as value and
     *      their player identifier as key.
     *
     * @{ */
    std::map<int, PlayerInfo>& Players();
    const std::map<int, PlayerInfo>& Players() const;
    /** @} */

    /** @brief Return the ::Universe known to this client
     *
     * @return A reference to the single ::Universe instance representing
     *      the known universe of this client.
     *
     * @{ */
    Universe& GetUniverse() override;
    const Universe& GetUniverse() const;
    /** @} */

    /** @brief Return the ::GalaxySetupData of this game
     *
     * @return A reference to the ::GalaxySetupData used in this game session.
     *
     * @{ */
    GalaxySetupData& GetGalaxySetupData();
    const GalaxySetupData& GetGalaxySetupData() const override;
    /** @} */

    /** @brief Return the OrderSet of this client
     *
     * @return A reference to the OrderSet of this client.
     *
     * @{ */
    OrderSet& Orders();
    const OrderSet& Orders() const;
    /** @} */

    /** @brief Return the networking object of this clients player
     *
     * @return A reference to the ClientNetworking object of this client.
     *
     * @{ */
    ClientNetworking& Networking();
    const ClientNetworking& Networking() const;
    /** @} */

    /** @brief Return The Networking::ClientType of this client
     *
     * @return the networking client type of this players client.
     */
    Networking::ClientType GetClientType() const;

    /** @brief Return the Networking::ClientType of the empire @a empire_id
     *
     * @param empire_id An empire identifier.
     *
     * @return the networking client type of the empire represented by @a
     *      empire_id parameter.
     */
    Networking::ClientType GetEmpireClientType(int empire_id) const override;

    /** @brief Return the Networking::ClientType of the player @a player_id
     *
     * @param player_id An client identifier.
     *
     * @return the networking client type of the player represented by @a
     *      player_id parameter.
     */
    Networking::ClientType GetPlayerClientType(int player_id) const override;

    /** @brief Return the for this client visible name of @a object
     *
     * @param object The object to obtain the name from.
     *
     * @return The name of the @a object.  Depdending on Visibility it may not
     *      match with the actual object name.
     */
    std::string GetVisibleObjectName(std::shared_ptr<const UniverseObject> object) override;

    /** @brief Send the OrderSet and UI data to the server and start a new turn */
    virtual void StartTurn(const SaveGameUIData& ui_data);

    /** @brief Send the OrderSet and AI state to the server and start a new turn */
    virtual void StartTurn(const std::string& save_state_string);

    /** @brief Send turn orders updates to server without starting new turn */
    void SendPartialOrders();

    /** \brief Handle server acknowledgement of receipt of orders and clear
        the orders. */
    virtual void HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id);

    /** @brief Return the set of known Empire s for this client
     *
     * @return The EmpireManager instance in charge of maintaining the Empire
     *      object instances.
     * @{ */
    EmpireManager& Empires() override;
    const EmpireManager& Empires() const;
    /** @} */

    /** @brief Return the Empire identified by @a empire_id
     *
     * @param empire_id An empire identifier.
     *
     * @return A pointer to the Empire instance represented by @a empire_id.
     *      If there is no Empire with this @a empire_id or if the Empire is
     *      not yet known to this client a nullptr is returned.
     */
    Empire* GetEmpire(int empire_id) override;

    SpeciesManager& GetSpeciesManager() override;
    const SpeciesManager& GetSpeciesManager() const;
    Species* GetSpecies(const std::string& name) override;

    SupplyManager& GetSupplyManager() override;

    /** @brief Return all Objects known to @a empire_id
     *
     * @param empire_id An empire identifier.
     *
     * @return A map containing all Objects known to the ::Empire identified by
     *      @a empire_id.  If there is no ::Empire an empty map is returned.
     */
    ObjectMap& EmpireKnownObjects(int empire_id) override;

    /** @brief Set the identifier of the ::Empire controlled by this client to
     *      @a empire_id
     *
     * @param empire_id The new ::Empire identifier.
     */
    void SetEmpireID(int empire_id);

    /** @brief Set the current game turn of this client to @a turn
     *
     * @param turn The new turn number of this client.
     */
    void SetCurrentTurn(int turn);

    /** @brief Set the Message::PlayerStatus @a status for @a empire_id
     *
     * @param player_id A empire identifier.
     * @param status The new Message::PlayerStatus of the player identified by
     *      @a empire_id.
     */
    void SetEmpireStatus(int empire_id, Message::PlayerStatus status);

    /** @brief Return the singleton instance of this Application
     *
     * @return A pointer to the single ClientApp instance of this client.
     */
    static ClientApp* GetApp();

    /** @brief Compare content checksum from server with client content checksum.
     *
     * @return true if verify successed.
     */
    bool VerifyCheckSum(const Message& msg);

protected:
    // Gamestate...
    Universe                    m_universe;
    GalaxySetupData             m_galaxy_setup_data;
    EmpireManager               m_empires;
    SpeciesManager              m_species_manager;
    SupplyManager               m_supply_manager;
    // End Gamestate

    // client local order storage
    OrderSet                    m_orders;

    // other client local info
    std::shared_ptr<ClientNetworking>   m_networking;
    int                                 m_empire_id = ALL_EMPIRES;
    int                                 m_current_turn = INVALID_GAME_TURN;
    /** Indexed by player id, contains info about all players in the game */

    std::map<int, PlayerInfo>   m_player_info;
};


#endif
