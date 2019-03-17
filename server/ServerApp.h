#ifndef _ServerApp_h_
#define _ServerApp_h_

#include "../util/Process.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"
#include "../python/server/ServerFramework.h"
#include "../network/ServerNetworking.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

#include <set>
#include <vector>

#include <boost/circular_buffer.hpp>

class OrderSet;
struct GalaxySetupData;
struct SaveGameUIData;
struct ServerFSM;

/** the application framework class for the FreeOrion server. */
class ServerApp : public IApp {
public:
    ServerApp();

    ServerApp(const ServerApp&) = delete;

    ServerApp(ServerApp&&) = delete;

    ~ServerApp() override;

    const ServerApp& operator=(const ServerApp&) = delete;

    ServerApp& operator=(IApp&&) = delete;

    /** \name Accessors */ //@{

    /** Returns a ClientApp pointer to the singleton instance of the app. */
    static ServerApp* GetApp();
    Universe& GetUniverse() override;
    EmpireManager& Empires() override;
    Empire* GetEmpire(int id) override;
    SupplyManager& GetSupplyManager() override;

    std::shared_ptr<UniverseObject> GetUniverseObject(int object_id) override;

    /** Returns the server's map for known objects of specified empire. */
    ObjectMap& EmpireKnownObjects(int empire_id) override;

    std::shared_ptr<UniverseObject> EmpireKnownObject(int object_id, int empire_id) override;

    std::string GetVisibleObjectName(std::shared_ptr<const UniverseObject> object) override;

    int CurrentTurn() const override
    { return m_current_turn; }

    const GalaxySetupData&  GetGalaxySetupData() const override
    { return m_galaxy_setup_data; }

    /** Checks if player with ID \a player_id is a human player
        who's client runs on the same machine as the server */
    bool IsLocalHumanPlayer(int player_id);

    /** Returns the networking client type for the given empire_id. */
    Networking::ClientType GetEmpireClientType(int empire_id) const override;

    /** Returns the networking client type for the given player_id. */
    Networking::ClientType GetPlayerClientType(int player_id) const override;

    int EffectsProcessingThreads() const override;

    /** Returns the empire ID for the player with ID \a player_id */
    int PlayerEmpireID(int player_id) const;

    /** Returns the player ID for the player controlling the empire with id \a
        empire_id */
    int EmpirePlayerID(int empire_id) const;

    /** Checks if \a player_name are not used by other players. */
    bool IsAvailableName(const std::string& player_name) const;

    /** Checks if server runs in a hostless mode. */
    bool IsHostless() const;

    /** Returns chat history buffer. */
    const boost::circular_buffer<ChatHistoryEntity>& GetChatHistory() const;

    /** Extracts player save game data. */
    std::vector<PlayerSaveGameData> GetPlayerSaveGameData() const;
    //@}


    /** \name Mutators */ //@{
    void    operator()();               ///< external interface to Run()

    void StartBackgroundParsing() override;

    /** Returns the galaxy setup data used for the current game */
    GalaxySetupData&    GetGalaxySetupData() { return m_galaxy_setup_data; }

    /** creates an AI client child process for each element of \a AIs*/
    void    CreateAIClients(const std::vector<PlayerSetupData>& player_setup_data, int max_aggression = 4);

    /** Adds save game data includes turn orders for the given empire for the current turn.
      * \a save_game_data will be freed when all processing is done for the turn */
    void    SetEmpireSaveGameData(int empire_id, std::unique_ptr<PlayerSaveGameData>&& save_game_data);

    /** Updated empire orders without changes in readiness status. Removes all \a deleted orders
      * and insert \a added orders. */
    void    UpdatePartialOrders(int empire_id, const OrderSet& added, const std::set<int>& deleted);

    /** Revokes turn order's ready state for the given empire. */
    void    RevokeEmpireTurnReadyness(int empire_id);

    /** Sets all empire turn orders to an empty set. */
    void    ClearEmpireTurnOrders();

    /** Determines if all empired have submitted their orders for this turn It
      * will loop the turn squence vector and check for a set order_set. A
      * order_set of 0 indicates that the empire has not yet submitted their
      * orders for the given turn */
    bool    AllOrdersReceived();

    /** Executes player orders, does colonization, does ordered scrapping, does
      * fleet movements, and updates visibility before combats are handled. */
    void    PreCombatProcessTurns();

    /** Determines which combats will occur, handles running the combats and
      * updating the universe after the results are available. */
    void    ProcessCombats();

    /** Used post combat, to selectively clear the m_arrival_starlane flag of monsters
     *  so that they can impose blockades */
    void    UpdateMonsterTravelRestrictions();

    /** Determines resource and supply distribution pathes and connections,
      * updates research, production, trade spending,
      * does population growth, updates current turn number, checks for
      * eliminated or victorious empires / players, sends new turn updates. */
    void    PostCombatProcessTurns();

    /** Determines if any empires are eliminated (for the first time this turn,
      * skipping any which were also eliminated previously) and if any empires
      * are thereby victorious. */
    void    CheckForEmpireElimination();

    /** Intializes single player game universe.*/
    void    NewSPGameInit(const SinglePlayerSetupData& single_player_setup_data);

    /** Return true if single player game AIs are compatible with created
      * universe and are ready to start a new game. */
    bool    VerifySPGameAIs(const SinglePlayerSetupData& single_player_setup_data);

    /** Intializes multi player game universe, sends out initial game state to
      * clients, and signals clients to start first turn */
    void    NewMPGameInit(const MultiplayerLobbyData& multiplayer_lobby_data);

    /** Restores saved single player gamestate and human and AI client state
      * information. */
    void    LoadSPGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                           std::shared_ptr<ServerSaveGameData> server_save_game_data);

    /** Restores saved multiplayer gamestate and human and AI client state
      * information. */
    void    LoadMPGameInit(const MultiplayerLobbyData& lobby_data,
                           const std::vector<PlayerSaveGameData>& player_save_game_data,
                           std::shared_ptr<ServerSaveGameData> server_save_game_data);

    /** Checks if \a player_name requires auth to login and fill \a roles if not. */
    bool IsAuthRequiredOrFillRoles(const std::string& player_name, Networking::AuthRoles& roles);

    /** Checks if \a auth match \a player_name and fill \a roles if successed. */
    bool IsAuthSuccessAndFillRoles(const std::string& player_name, const std::string& auth, Networking::AuthRoles& roles);

    /** Adds new observing player to running game.
      * Simply sends GAME_START message so established player knows he is in the game. */
    void AddObserverPlayerIntoGame(const PlayerConnectionPtr& player_connection);

    /** Eliminate player's empire by \a player_connection. Return true if player was eliminated. */
    bool EliminatePlayer(const PlayerConnectionPtr& player_connection);

    /** Drop link between player with \a player_id and his empire. */
    void DropPlayerEmpireLink(int planet_id);

    /** Adds new player to running game.
      * Search empire by player's name and return empire id if success and ALL_EMPIRES if no empire found.
      * Simply sends GAME_START message so established player knows he is in the game.
      * Notificates the player about statuses of other empires. */
    int AddPlayerIntoGame(const PlayerConnectionPtr& player_connection);
    //@}

    void UpdateSavePreviews(const Message& msg, PlayerConnectionPtr player_connection);

    /** Send the requested combat logs to the client.*/
    void UpdateCombatLogs(const Message& msg, PlayerConnectionPtr player_connection);

    /** Loads chat history via python script. */
    void LoadChatHistory();

    void PushChatMessage(const std::string& text,
                         const std::string& player_name,
                         GG::Clr text_color,
                         const boost::posix_time::ptime& timestamp);

    ServerNetworking&           Networking();     ///< returns the networking object for the server

private:
    void    Run();          ///< initializes app state, then executes main event handler/render loop (Poll())

    /** Initialize the python engine if not already running. Return true on success. */
    void InitializePython();

    /** Called when server process receive termination signal */
    void    SignalHandler(const boost::system::error_code& error, int signal_number);


    /** Clears any old game stored orders, victors or eliminated players, ads
      * empires to turn processing list, does start-of-turn empire supply and
      * resource pool determination. */
    void    NewGameInitConcurrentWithJoiners(const GalaxySetupData& galaxy_setup_data,
                                             const std::vector<PlayerSetupData>& player_setup_data);

    /** Return true if player data is consistent with starting a new game. */
    bool    NewGameInitVerifyJoiners(const std::vector<PlayerSetupData>& player_setup_data);

    /** Sends out initial new game state to clients, and signals clients to start first turn. */
    void SendNewGameStartMessages();

    /** Clears any old game stored orders, victors or eliminated players, ads
      * empires to turn processing list, does start-of-turn empire supply and
      * resource pool determination, regenerates pathfinding graphc, restores
      * any UI or AI state information players had saved, and compiles info
      * about all players to send out to all other players are part of game
      * start messages. */
    void    LoadGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                         const std::vector<std::pair<int, int>>& player_id_to_save_game_data_index,
                         std::shared_ptr<ServerSaveGameData> server_save_game_data);

    /** Calls Python universe generator script.
      * Supposed to be called to create a new universe so it can be used by content
      * scripters to customize universe generation. */
    void    GenerateUniverse(std::map<int, PlayerSetupData>& player_setup_data);

    /** Calls Python turn events script.
      * Supposed to be called every turn so it can be used by content scripters to
      * implement user customizable turn events. */
    void    ExecuteScriptedTurnEvents();

    void    CleanupAIs();   ///< cleans up AI processes: kills the process and empties the container of AI processes

    /** Sets the priority for all AI processes */
    void    SetAIsProcessPriorityToLow(bool set_to_low);

    /** Get players info map to send it in GameStart message */
    std::map<int, PlayerInfo> GetPlayerInfoMap() const;

    /** Handles an incoming message from the server with the appropriate action
      * or response */
    void    HandleMessage(const Message& msg, PlayerConnectionPtr player_connection);

    /** Checks validity of shut down message from player, then attempts to
      * cleanly shut down this server process. */
    void    HandleShutdownMessage(const Message& msg, PlayerConnectionPtr player_connection);

    /** Checks validity of logger config message and then update logger and loggers of all AIs. */
    void    HandleLoggerConfig(const Message& msg, PlayerConnectionPtr player_connection);

    /** When Messages arrive from connections that are not established players,
      * they arrive via a call to this function*/
    void    HandleNonPlayerMessage(const Message& msg, PlayerConnectionPtr player_connection);

    /** Called by ServerNetworking when a player's TCP connection is closed*/
    void    PlayerDisconnected(PlayerConnectionPtr player_connection);

    /** Handle shutdown timeout by killing all ais. */
    void ShutdownTimedoutHandler(boost::system::error_code error);

    /** Called when the host player has disconnected.  Select a new host player*/
    void    SelectNewHost();

    /** Called when this server's EmpireManager changes the diplomatic status
      * between two empires. Updates those empires of the change. */
    void    HandleDiplomaticStatusChange(int empire1_id, int empire2_id);

    /** Called when this sever's EmpireManager changes the diplmatic message
      * between two empires. Updates those empires of the change. */
    void    HandleDiplomaticMessageChange(int empire1_id, int empire2_id);

    /**  Adds an existing empire to turn processing. The position the empire is
      * in the vector is it's position in the turn processing.*/
    void    AddEmpireTurn(int empire_id, const PlayerSaveGameData& psgd);

    /** Removes an empire from turn processing. This is most likely called when
      * an empire is eliminated from the game */
    void    RemoveEmpireTurn(int empire_id);

    boost::asio::io_context m_io_context;
    boost::asio::signal_set m_signals;

    Universe                m_universe;
    EmpireManager           m_empires;
    SupplyManager           m_supply_manager;
    ServerNetworking        m_networking;
    ServerFSM*              m_fsm;
    PythonServer            m_python_server;
    std::map<int, int>      m_player_empire_ids;    ///< map from player id to empire id that the player controls.
    int                     m_current_turn;         ///< current turn number
    std::vector<Process>    m_ai_client_processes;  ///< AI client child processes
    bool                    m_single_player_game;   ///< true when the game being played is single-player
    GalaxySetupData         m_galaxy_setup_data;    ///< stored setup data for the game currently being played
    boost::circular_buffer<ChatHistoryEntity> m_chat_history; ///< Stored last chat messages.

    /** Turn sequence map is used for turn processing. Each empire is added at
      * the start of a game or reload and then the map maintains OrderSets for
      * that turn.
      * The map contains pointer to orders from empire with ready state which should be true
      * to advance turn.
      * */
    std::map<int, std::unique_ptr<PlayerSaveGameData>>  m_turn_sequence;

    // Give FSM and its states direct access.  We are using the FSM code as a
    // control-flow mechanism; it is all notionally part of this class.
    friend struct ServerFSM;
    friend struct Idle;
    friend struct MPLobby;
    friend struct WaitingForSPGameJoiners;
    friend struct WaitingForMPGameJoiners;
    friend struct PlayingGame;
    friend struct WaitingForTurnEnd;
    friend struct WaitingForTurnEndIdle;
    friend struct WaitingForSaveData;
    friend struct ProcessingTurn;
    friend struct ShuttingDownServer;
};

#endif // _ServerApp_h_
