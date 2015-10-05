// -*- C++ -*-
#ifndef _ServerApp_h_
#define _ServerApp_h_

#include "../util/Process.h"
#include "../Empire/EmpireManager.h"
#include "../network/ServerNetworking.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

#include <set>
#include <vector>

class OrderSet;
struct GalaxySetupData;
struct SaveGameUIData;
struct ServerFSM;

/** contains the data that must be saved for a single player.  Note that the m_empire member is not deallocated by
    PlayerSaveGameData.  Users of PlayerSaveGameData are resposible for managing its lifetime. */
struct PlayerSaveGameData {
    PlayerSaveGameData(); ///< default ctor
    PlayerSaveGameData(const std::string& name, int empire_id, const boost::shared_ptr<OrderSet>& orders,
                       const boost::shared_ptr<SaveGameUIData>& ui_data, const std::string& save_state_string,
                       Networking::ClientType client_type); ///< ctor

    std::string                         m_name;
    int                                 m_empire_id;
    boost::shared_ptr<OrderSet>         m_orders;
    boost::shared_ptr<SaveGameUIData>   m_ui_data;
    std::string                         m_save_state_string;
    Networking::ClientType              m_client_type;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** contains data that must be retained by the server when saving and loading a game that isn't player data or
    the universe */
struct ServerSaveGameData {
    ServerSaveGameData();                               ///< default ctor
    ServerSaveGameData(int current_turn);

    int                                     m_current_turn;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** the application framework class for the FreeOrion server. */
class ServerApp : public IApp {
public:
    /** \name Structors */ //@{
    ServerApp();
    ~ServerApp();
    //@}

    /** \name Accessors */ //@{
    int     CurrentTurn() const {return m_current_turn;}   ///< returns current turn of the server

    /** Returns the galaxy setup data used for the current game */
    const GalaxySetupData&  GetGalaxySetupData() const {return m_galaxy_setup_data; }

    /** Returns the empire ID for the player with ID \a player_id */
    int     PlayerEmpireID(int player_id) const;

    /** Returns the player ID for the player controlling the empire with id \a empire_id */
    int     EmpirePlayerID(int empire_id) const;

    /** Checks if player with ID \a player_id is a human player
        who's client runs on the same machine as the server */
    bool    IsLocalHumanPlayer(int player_id);
    //@}


    /** \name Mutators */ //@{
    void    operator()();               ///< external interface to Run()
    void    Exit(int code);             ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()

    /** Returns the galaxy setup data used for the current game */
    GalaxySetupData&    GetGalaxySetupData() { return m_galaxy_setup_data; }

    /** creates an AI client child process for each element of \a AIs*/
    void    CreateAIClients(const std::vector<PlayerSetupData>& player_setup_data, int max_aggression = 4);

    /**  Adds an existing empire to turn processing. The position the empire is
      * in the vector is it's position in the turn processing.*/
    void    AddEmpireTurn(int empire_id);

    /** Removes an empire from turn processing. This is most likely called when
      * an empire is eliminated from the game */
    void    RemoveEmpireTurn(int empire_id);

    /** Adds turn orders for the given empire for the current turn. order_set
      * will be freed when all processing is done for the turn */
    void    SetEmpireTurnOrders(int empire_id, OrderSet* order_set);

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

    /** Intializes single player game universe, sends out initial game state to
      * clients, and signals clients to start first turn */
    void    NewSPGameInit(const SinglePlayerSetupData& single_player_setup_data);

    /** Intializes multi player game universe, sends out initial game state to
      * clients, and signals clients to start first turn */
    void    NewMPGameInit(const MultiplayerLobbyData& multiplayer_lobby_data);

    /** Restores saved single player gamestate and human and AI client state
      * information. */
    void    LoadSPGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                                       boost::shared_ptr<ServerSaveGameData> server_save_game_data);

    /** Restores saved multiplayer gamestate and human and AI client state
      * information. */
    void    LoadMPGameInit(const MultiplayerLobbyData& lobby_data,
                           const std::vector<PlayerSaveGameData>& player_save_game_data,
                           boost::shared_ptr<ServerSaveGameData> server_save_game_data);
    //@}

    void UpdateSavePreviews(const Message& msg, PlayerConnectionPtr player_connection);

    static ServerApp*           GetApp();         ///< returns a ClientApp pointer to the singleton instance of the app
    Universe&                   GetUniverse();    ///< returns server's copy of Universe
    EmpireManager&              Empires();        ///< returns the server's copy of the Empires
    Empire*                     GetEmpire(int id);
    TemporaryPtr<UniverseObject>GetUniverseObject(int object_id);
    ObjectMap&                  EmpireKnownObjects(int empire_id); ///< returns the server's map for known objects of specified empire. */
    TemporaryPtr<UniverseObject>EmpireKnownObject(int object_id, int empire_id);

    ServerNetworking&           Networking();     ///< returns the networking object for the server

    std::string                 GetVisibleObjectName(TemporaryPtr<const UniverseObject> object);

    /** returns a universe object ID which can be used for new objects.
        Can return INVALID_OBJECT_ID if an ID cannot be created. */
    int                         GetNewObjectID();

    /** returns a design ID which can be used for a new design to uniquely identify it.
        Can return INVALID_OBJECT_ID if an ID cannot be created. */
    int                         GetNewDesignID();

private:
    const ServerApp& operator=(const ServerApp&); // disabled
    ServerApp(const ServerApp&); // disabled

    void    Run();          ///< initializes app state, then executes main event handler/render loop (Poll())

    /** Called when server process receive termination signal */
    void    SignalHandler(const boost::system::error_code& error, int signal_number);


    /** Clears any old game stored orders, victors or eliminated players, ads
      * empires to turn processing list, does start-of-turn empire supply and
      * resource pool determination, and compiles and sends out basic
      * information about players in game for all other players as part of the
      * game start messages sent to players. */
    void    NewGameInit(const GalaxySetupData& galaxy_setup_data,
                        const std::map<int, PlayerSetupData>& player_id_setup_data);

    /** Clears any old game stored orders, victors or eliminated players, ads
      * empires to turn processing list, does start-of-turn empire supply and
      * resource pool determination, regenerates pathfinding graphc, restores
      * any UI or AI state information players had saved, and compiles info
      * about all players to send out to all other players are part of game
      * start messages. */
    void    LoadGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                         const std::vector<std::pair<int, int> >& player_id_to_save_game_data_index,
                         boost::shared_ptr<ServerSaveGameData> server_save_game_data);

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

    /** When Messages arrive from connections that are not established players,
      * they arrive via a call to this function*/
    void    HandleNonPlayerMessage(const Message& msg, PlayerConnectionPtr player_connection);

    /** Called by ServerNetworking when a player's TCP connection is closed*/
    void    PlayerDisconnected(PlayerConnectionPtr player_connection);

    /** Called when the host player has disconnected.  Select a new host player*/
    void    SelectNewHost();

    /** Called when this server's EmpireManager changes the diplomatic status
      * between two empires. Updates those empires of the change. */
    void    HandleDiplomaticStatusChange(int empire1_id, int empire2_id);

    /** Called when this sever's EmpireManager changes the diplmatic message
      * between two empires. Updates those empires of the change. */
    void    HandleDiplomaticMessageChange(int empire1_id, int empire2_id);

    boost::asio::io_service m_io_service;
    boost::asio::signal_set m_signals;

    Universe                m_universe;
    EmpireManager           m_empires;
    ServerNetworking        m_networking;
    ServerFSM*              m_fsm;
    std::map<int, int>      m_player_empire_ids;    ///< map from player id to empire id that the player controls.
    int                     m_current_turn;         ///< current turn number
    std::vector<Process>    m_ai_client_processes;  ///< AI client child processes
    bool                    m_single_player_game;   ///< true when the game being played is single-player
    GalaxySetupData         m_galaxy_setup_data;    ///< stored setup data for the game currently being played

    /** Turn sequence map is used for turn processing. Each empire is added at
      * the start of a game or reload and then the map maintains OrderSets for
      * that turn. */
    std::map<int, OrderSet*>                m_turn_sequence;

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
};

// template implementations
template <class Archive>
void PlayerSaveGameData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_orders)
        & BOOST_SERIALIZATION_NVP(m_ui_data)
        & BOOST_SERIALIZATION_NVP(m_save_state_string)
        & BOOST_SERIALIZATION_NVP(m_client_type);
}

template <class Archive>
void ServerSaveGameData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_current_turn);
}

#endif // _ServerApp_h_
