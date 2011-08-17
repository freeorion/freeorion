#include "ServerApp.h"

#include "SaveLoad.h"
#include "ServerFSM.h"
#include "../combat/CombatSystem.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Special.h"
#include "../universe/System.h"
#include "../universe/Species.h"
#include "../Empire/Empire.h"
#include "../util/Directories.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/OrderSet.h"
#include "../util/SitRepEntry.h"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <ctime>


namespace fs = boost::filesystem;


////////////////////////////////////////////////
// PlayerSaveGameData
////////////////////////////////////////////////
PlayerSaveGameData::PlayerSaveGameData() :
    m_name(),
    m_empire_id(ALL_EMPIRES),
    m_orders(),
    m_ui_data(),
    m_save_state_string(),
    m_client_type(Networking::INVALID_CLIENT_TYPE)
{}

PlayerSaveGameData::PlayerSaveGameData(const std::string& name, int empire_id, const boost::shared_ptr<OrderSet>& orders,
                                       const boost::shared_ptr<SaveGameUIData>& ui_data, const std::string& save_state_string,
                                       Networking::ClientType client_type) :
    m_name(name),
    m_empire_id(empire_id),
    m_orders(orders),
    m_ui_data(ui_data),
    m_save_state_string(save_state_string),
    m_client_type(client_type)
{}


////////////////////////////////////////////////
// ServerSaveGameData
////////////////////////////////////////////////
ServerSaveGameData::ServerSaveGameData() :
    m_current_turn(INVALID_GAME_TURN)
{}

ServerSaveGameData::ServerSaveGameData(int current_turn, const std::map<int, std::set<std::string> >& victors) :
    m_current_turn(current_turn),
    m_victors(victors)
{}


////////////////////////////////////////////////
// ServerApp
////////////////////////////////////////////////
// static member(s)
ServerApp*  ServerApp::s_app = 0;

ServerApp::ServerApp() :
    m_current_combat(0),
    m_networking(m_io_service,
                 boost::bind(&ServerApp::HandleNonPlayerMessage, this, _1, _2),
                 boost::bind(&ServerApp::HandleMessage, this, _1, _2),
                 boost::bind(&ServerApp::PlayerDisconnected, this, _1)),
    m_fsm(new ServerFSM(*this)),
    m_current_turn(INVALID_GAME_TURN),
    m_single_player_game(false)
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of singleton class ServerApp");

    s_app = this;

    const std::string SERVER_LOG_FILENAME((GetUserDir() / "freeoriond.log").string());

    // a platform-independent way to erase the old log
    std::ofstream temp(SERVER_LOG_FILENAME.c_str());
    temp.close();

    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", SERVER_LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p Server : %m%n");
    appender->setLayout(layout);
    Logger().setAdditivity(false);  // make appender the only appender used...
    Logger().setAppender(appender);
    Logger().setAdditivity(true);   // ...but allow the addition of others later
    Logger().setPriority(log4cpp::Priority::DEBUG);

    m_fsm->initiate();
}

ServerApp::~ServerApp()
{
    CleanupAIs();
    delete m_fsm;
}

void ServerApp::operator()()
{ Run(); }

void ServerApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    exit(code);
}

namespace {
    std::string AIClientExe()
    {
#ifdef FREEORION_WIN32
        return (GetBinDir() / "freeorionca.exe").string();
#else
        return (GetBinDir() / "freeorionca").string();
#endif
    }
}

void ServerApp::CreateAIClients(const std::vector<PlayerSetupData>& player_setup_data)
{
    // disconnect any old AI clients
    CleanupAIs();

    // binary / executable to run for AI clients
    const std::string AI_CLIENT_EXE = AIClientExe();

    // for each AI client player, create a new AI client process
    for (int i = 0; i < static_cast<int>(player_setup_data.size()); ++i) {
        const PlayerSetupData& psd = player_setup_data.at(i);

        if (psd.m_client_type != Networking::CLIENT_TYPE_AI_PLAYER)
            continue;

        // check that AIs have a name, as they will be sorted lated based on it
        std::string player_name = psd.m_player_name;
        if (player_name.empty()) {
            Logger().errorStream() << "ServerApp::CreateAIClients can't create a player with no name.";
            return;
        }

        // TODO: add other command line args to AI client invocation as needed
        std::vector<std::string> args;
        args.push_back("\"" + AI_CLIENT_EXE + "\"");
        args.push_back(player_name);
        args.push_back("--resource-dir");
        args.push_back("\"" + GetOptionsDB().Get<std::string>("resource-dir") + "\"");
        args.push_back("--log-level");
        args.push_back(GetOptionsDB().Get<std::string>("log-level"));

        Logger().debugStream() << "starting " << AI_CLIENT_EXE;

        m_ai_client_processes.push_back(Process(AI_CLIENT_EXE, args));

        Logger().debugStream() << "done starting " << AI_CLIENT_EXE;
    }
}

ServerApp* ServerApp::GetApp()
{ return s_app; }

Universe& ServerApp::GetUniverse()
{ return s_app->m_universe; }

EmpireManager& ServerApp::Empires()
{ return s_app->m_empires; }

CombatData* ServerApp::CurrentCombat()
{ return s_app->m_current_combat; }

ServerNetworking& ServerApp::Networking()
{ return s_app->m_networking; }

void ServerApp::Run()
{
    Logger().debugStream() << "FreeOrion server waiting for network events";
    std::cout << "FreeOrion server waiting for network events" << std::endl;
    while (1) {
        if (m_io_service.run_one())
            m_networking.HandleNextEvent();
        else
            break;
    }
}

void ServerApp::CleanupAIs()
{
    Logger().debugStream() << "ServerApp::CleanupAIs() killing " << m_ai_client_processes.size() << " AI clients.";
    for (std::vector<Process>::iterator it = m_ai_client_processes.begin(); it != m_ai_client_processes.end(); ++it)
        it->Kill();
    m_ai_client_processes.clear();
}

void ServerApp::HandleMessage(Message msg, PlayerConnectionPtr player_connection)
{
    if (msg.SendingPlayer() != player_connection->PlayerID()) {
        Logger().errorStream() << "ServerApp::HandleMessage : Received an message with a sender ID that differs from the sending player's ID.  Terminating connection.";
        m_networking.Disconnect(player_connection);
        return;
    }

    Logger().debugStream() << "ServerApp::HandleMessage type " << boost::lexical_cast<std::string>(msg.Type());

    switch (msg.Type()) {
    case Message::HOST_SP_GAME:             m_fsm->process_event(HostSPGame(msg, player_connection));       break;
    case Message::START_MP_GAME:            m_fsm->process_event(StartMPGame(msg, player_connection));      break;
    case Message::LOBBY_UPDATE:             m_fsm->process_event(LobbyUpdate(msg, player_connection));      break;
    case Message::LOBBY_CHAT:               m_fsm->process_event(LobbyChat(msg, player_connection));        break;
    case Message::SAVE_GAME:                m_fsm->process_event(SaveGameRequest(msg, player_connection));  break;
    case Message::TURN_ORDERS:              m_fsm->process_event(TurnOrders(msg, player_connection));       break;
    case Message::COMBAT_TURN_ORDERS:       m_fsm->process_event(CombatTurnOrders(msg, player_connection)); break;
    case Message::CLIENT_SAVE_DATA:         m_fsm->process_event(ClientSaveData(msg, player_connection));   break;
    case Message::PLAYER_CHAT:              m_fsm->process_event(PlayerChat(msg, player_connection));       break;
    case Message::REQUEST_NEW_OBJECT_ID:    m_fsm->process_event(RequestObjectID(msg, player_connection));  break;
    case Message::REQUEST_NEW_DESIGN_ID:    m_fsm->process_event(RequestDesignID(msg, player_connection));  break;

    // TODO: For prototyping only.
    case Message::COMBAT_END:               m_fsm->process_event(CombatComplete()); break;

    case Message::ERROR:
    case Message::DEBUG:                    break;

    default:
        Logger().errorStream() << "ServerApp::HandleMessage : Received an unknown message type \"" << msg.Type() << "\".  Terminating connection.";
        m_networking.Disconnect(player_connection);
        break;
    }
}

void ServerApp::HandleNonPlayerMessage(Message msg, PlayerConnectionPtr player_connection)
{
    switch (msg.Type()) {
    case Message::HOST_SP_GAME: m_fsm->process_event(HostSPGame(msg, player_connection));   break;
    case Message::HOST_MP_GAME: m_fsm->process_event(HostMPGame(msg, player_connection));   break;
    case Message::JOIN_GAME:    m_fsm->process_event(JoinGame(msg, player_connection));     break;
#ifndef FREEORION_RELEASE
    case Message::DEBUG:        break;
#endif
    default:
        Logger().errorStream() << "ServerApp::HandleNonPlayerMessage : Received an invalid message type \""
                                     << msg.Type() << "\" for a non-player Message.  Terminating connection.";
        m_networking.Disconnect(player_connection);
        break;
    }
}

void ServerApp::PlayerDisconnected(PlayerConnectionPtr player_connection)
{ m_fsm->process_event(Disconnection(player_connection)); }

void ServerApp::SelectNewHost()
{
    int new_host_id = Networking::INVALID_PLAYER_ID;
    int old_host_id = m_networking.HostPlayerID();

    Logger().debugStream() << "ServerApp::SelectNewHost old host id: " << old_host_id;

    // scan through players for a human to host
    for (ServerNetworking::established_iterator players_it = m_networking.established_begin();
            players_it != m_networking.established_end(); ++players_it)
    {
        PlayerConnectionPtr player_connection = *players_it;
        if (player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
        {
            new_host_id = player_connection->PlayerID();
        }
    }

    if (new_host_id == Networking::INVALID_PLAYER_ID) {
        // couldn't find a host... abort
        Logger().debugStream() << "ServerApp::SelectNewHost : Host disconnected and couldn't find a replacement.";
        m_networking.SendMessage(ErrorMessage("SERVER_UNABLE_TO_SELECT_HOST", false));
    }

    // set new host ID
    m_networking.SetHostPlayerID(new_host_id);

    // inform players.
    for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
        if ((*it)->PlayerID() != old_host_id)
            (*it)->SendMessage(HostIDMessage(new_host_id));
    }
}

void ServerApp::NewSPGameInit(const SinglePlayerSetupData& single_player_setup_data)
{
    // associate player IDs with player setup data.  the player connection with
    // id == m_networking.HostPlayerID() should be the human player in
    // PlayerSetupData.  AI player connections are assigned one of the remaining
    // PlayerSetupData entries that is for an AI player.
    std::map<int, PlayerSetupData> player_id_setup_data;
    const std::vector<PlayerSetupData>& player_setup_data = single_player_setup_data.m_players;

    for (std::vector<PlayerSetupData>::const_iterator setup_data_it = player_setup_data.begin();
         setup_data_it != player_setup_data.end(); ++setup_data_it)
    {
        const PlayerSetupData& psd = *setup_data_it;
        if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            // In a single player game, the host player is always the human player, so
            // this is just a matter of finding which player setup data is for
            // a human player, and assigning that setup data to the host player id
            player_id_setup_data[m_networking.HostPlayerID()] = psd;

        } else if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            // All AI player setup data, as determined from their client type, is
            // assigned to player IDs of established AI players with the appropriate names

            // find player connection with same name as this player setup data
            bool found_matched_name_connection = false;
            const std::string& player_name = psd.m_player_name;
            for (ServerNetworking::const_established_iterator established_player_it = m_networking.established_begin();
                 established_player_it != m_networking.established_end(); ++established_player_it)
            {
                const PlayerConnectionPtr player_connection = *established_player_it;
                if (player_connection->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER &&
                    player_connection->PlayerName() == player_name)
                {
                    // assign name-matched AI client's player setup data to appropriate AI connection
                    int player_id = player_connection->PlayerID();
                    player_id_setup_data[player_id] = psd;
                    found_matched_name_connection = true;
                    break;
                }
            }
            if (!found_matched_name_connection)
                Logger().errorStream() << "ServerApp::NewSPGameInit couldn't find player setup data for player with name: " << player_name;
        } else {
            // do nothing for any other player type, until another player type is implemented
            Logger().errorStream() << "ServerApp::NewSPGameInit skipping unsupported client type in player setup data";
        }
    }

    NewGameInit(single_player_setup_data, player_id_setup_data);
}

void ServerApp::NewMPGameInit(const MultiplayerLobbyData& multiplayer_lobby_data)
{
    // associate player IDs with player setup data by matching player IDs when
    // available (human) and names (for AI clients which didn't have an ID
    // before now because the lobby data was set up without connected/established
    // clients for the AIs.
    std::map<int, PlayerSetupData> player_id_setup_data;
    const std::list<std::pair<int, PlayerSetupData> >& player_setup_data = multiplayer_lobby_data.m_players;

    for (std::list<std::pair<int, PlayerSetupData> >::const_iterator setup_data_it = player_setup_data.begin();
         setup_data_it != player_setup_data.end(); ++setup_data_it)
    {
        const PlayerSetupData& psd = setup_data_it->second;
        if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
        {
            // Human players have consistent IDs, so these can be easily
            // matched between established player connections and setup data.

            // find player connection with same ID as this player setup data
            bool found_matched_id_connection = false;
            int player_id = setup_data_it->first;
            for (ServerNetworking::const_established_iterator established_player_it = m_networking.established_begin();
                 established_player_it != m_networking.established_end(); ++established_player_it)
            {
                const PlayerConnectionPtr player_connection = *established_player_it;
                if (player_connection->PlayerID() == player_id)
                {
                    player_id_setup_data[player_id] = psd;
                    found_matched_id_connection = true;
                    break;
                }
            }
            if (!found_matched_id_connection)
                Logger().errorStream() << "ServerApp::NewMPGameInit couldn't find player setup data for human player with id: " << player_id;

        } else if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            // All AI player setup data, as determined from their client type, is
            // assigned to player IDs of established AI players with the appropriate names

            // find player connection with same name as this player setup data
            bool found_matched_name_connection = false;
            const std::string& player_name = psd.m_player_name;
            for (ServerNetworking::const_established_iterator established_player_it = m_networking.established_begin();
                 established_player_it != m_networking.established_end(); ++established_player_it)
            {
                const PlayerConnectionPtr player_connection = *established_player_it;
                if (player_connection->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER &&
                    player_connection->PlayerName() == player_name)
                {
                    // assign name-matched AI client's player setup data to appropriate AI connection
                    int player_id = player_connection->PlayerID();
                    player_id_setup_data[player_id] = psd;
                    found_matched_name_connection = true;
                    break;
                }
            }
            if (!found_matched_name_connection)
                Logger().errorStream() << "ServerApp::NewMPGameInit couldn't find player setup data for AI player with name: " << player_name;

        } else {
            // do nothing for any other player type, until another player type
            // is implemented.  human observers don't need to be put into the
            // map of id to player setup data, as they don't need empires to be
            // created for them.
            Logger().errorStream() << "ServerApp::NewMPGameInit skipping unsupported client type in player setup data";
        }
    }

    NewGameInit(multiplayer_lobby_data, player_id_setup_data);
}

void ServerApp::NewGameInit(const GalaxySetupData& galaxy_setup_data, const std::map<int, PlayerSetupData>& player_id_setup_data)
{
    Logger().debugStream() << "ServerApp::NewGameInit";

    // ensure some reasonable inputs
    if (player_id_setup_data.empty()) {
        Logger().errorStream() << "ServerApp::NewGameInit passed empty player_id_setup_data.  Aborting";
        return;
    }
    // ensure number of players connected and for which data are provided are consistent
    if (m_networking.NumEstablishedPlayers() != player_id_setup_data.size())
        Logger().debugStream() << "ServerApp::NewGameInit has " << m_networking.NumEstablishedPlayers() << " established players but " << player_id_setup_data.size() << " entries in player setup data.";

    // validate some connection info / determine which players need empires created
    std::map<int, PlayerSetupData> active_players_id_setup_data;
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        Networking::ClientType client_type = player_connection->GetClientType();
        int player_id = player_connection->PlayerID();

        std::map<int, PlayerSetupData>::const_iterator player_id_setup_data_it = player_id_setup_data.find(player_id);
        if (player_id_setup_data_it == player_id_setup_data.end()) {
            Logger().errorStream() << "ServerApp::NewGameInit couldn't find player setup data for player with ID " << player_id;
            return;
        }
        const PlayerSetupData& psd = player_id_setup_data_it->second;
        if (psd.m_client_type != client_type) {
            Logger().errorStream() << "ServerApp::NewGameInit found inconsistent client type between player connection (" << client_type << ") and player setup data (" << psd.m_client_type << ")";
            return;
        }
        if (psd.m_player_name != player_connection->PlayerName()) {
            Logger().errorStream() << "ServerApp::NewGameInit found inconsistent player names: " << psd.m_player_name << " and " << player_connection->PlayerName();
            return;
        }
        if (player_connection->PlayerName().empty()) {
            Logger().errorStream() << "ServerApp::NewGameInit found player connection with empty name!";
            return;
        }

        if (client_type == Networking::CLIENT_TYPE_AI_PLAYER || client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            active_players_id_setup_data[player_id] = player_id_setup_data_it->second;
        } else if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER) {
            // do nothing
        } else {
            Logger().errorStream() << "ServerApp::NewGameInit found player connection with unsupported client type.";
        }
    }

    if (active_players_id_setup_data.empty()) {
        Logger().errorStream() << "ServerApp::NewGameInit found no active players!";
        return;
    }

    // clear previous game player state info
    m_turn_sequence.clear();
    m_eliminated_players.clear();
    m_player_empire_ids.clear();


    // set server state info for new game
    m_current_turn =    BEFORE_FIRST_TURN;
    m_victors.clear();


    // create universe and empires for players
    Logger().debugStream() << "ServerApp::NewGameInit: Creating Universe";
    // m_current_turn set above so that every UniverseObject created before game starts will have m_created_on_turn BEFORE_FIRST_TURN
    m_universe.CreateUniverse(galaxy_setup_data.m_size,             galaxy_setup_data.m_shape,
                              galaxy_setup_data.m_age,              galaxy_setup_data.m_starlane_freq,
                              galaxy_setup_data.m_planet_density,   galaxy_setup_data.m_specials_freq,
                              galaxy_setup_data.m_life_freq,
                              active_players_id_setup_data);
    // after all game initialization stuff has been created, can set current turn to 1 for start of game
    m_current_turn = 1;


    // record empires for each active player: ID of empire and player should be the same when creating a new game.
    for (std::map<int, PlayerSetupData>::const_iterator player_setup_it = active_players_id_setup_data.begin();
         player_setup_it != active_players_id_setup_data.end(); ++player_setup_it)
    {
        m_player_empire_ids[player_setup_it->first] = player_setup_it->first;
    }


    // add empires to turn processing
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        int empire_id = PlayerEmpireID(player_id);
        if (Empires().Lookup(empire_id))
            AddEmpireTurn(empire_id);
    }


    // compile information about players to send out to other players at start of game.
    Logger().debugStream() << "ServerApp::NewGameInit: Compiling PlayerInfo for each player";
    std::map<int, PlayerInfo> player_info_map;
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        int empire_id = PlayerEmpireID(player_id);
        player_info_map[player_id] = PlayerInfo(player_connection->PlayerName(),
                                                empire_id,
                                                player_connection->GetClientType(),
                                                m_networking.PlayerIsHost(player_connection->PlayerID()));
    }


    // update visibility information to ensure data sent out is up-to-date
    Logger().debugStream() << "ServerApp::NewGameInit: Updating first-turn Empire stuff";
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();


    // Determine initial supply distribution and exchanging and resource pools for empires
    EmpireManager& empires = Empires();
    for (EmpireManager::iterator it = empires.begin(); it != empires.end(); ++it) {
        if (empires.Eliminated(it->first))
            continue;   // skip eliminated empires.  presumably this shouldn't be an issue when initializing a new game, but apparently I thought this was worth checking for...
        Empire* empire = it->second;

        empire->UpdateSupplyUnobstructedSystems();  // determines which systems can propegate fleet and resource (same for both)
        empire->UpdateSystemSupplyRanges();         // sets range systems can propegate fleet and resourse supply (separately)
        empire->UpdateFleetSupply();                // determines which systems can access fleet supply, and starlane traversals used to do this
        empire->UpdateResourceSupply();             // determines the separate groups of systems within which (but not between which) resources can be shared
        empire->InitResourcePools();                // determines population centers and resource centers of empire, tells resource pools the centers and groups of systems that can share resources (note that being able to share resources doesn't mean a system produces resources)
        empire->UpdateResourcePools();              // determines how much of each resources is available in each resource sharing group
    }

    // self-allocate resources on unowned planets, so natives don't starve
    std::vector<Planet*> planets = m_universe.Objects().FindObjects<Planet>();
    for (std::vector<Planet*>::const_iterator it = planets.begin(); it != planets.end(); ++it)
        if ((*it)->Unowned() && (*it)->CurrentMeterValue(METER_POPULATION) > 0.0)
            (*it)->SetAllocatedFood(std::min((*it)->CurrentMeterValue(METER_FARMING), (*it)->CurrentMeterValue(METER_POPULATION)));


    // send new game start messages
    Logger().debugStream() << "ServerApp::NewGameInit: Sending GameStartMessages to players";
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        int empire_id = PlayerEmpireID(player_id);
        player_connection->SendMessage(GameStartMessage(player_id,
                                                        m_single_player_game,
                                                        empire_id,
                                                        m_current_turn,
                                                        m_empires,
                                                        m_universe,
                                                        GetSpeciesManager(),
                                                        player_info_map));
    }
}

void ServerApp::LoadSPGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                               boost::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    // Need to determine which data in player_save_game_data should be assigned to which established player
    std::map<int, int> player_id_to_save_game_data_index;

    ServerNetworking::const_established_iterator established_player_it = m_networking.established_begin();

    // assign all saved game data to a player ID
    for (int i = 0; i < static_cast<int>(player_save_game_data.size()); ++i) {
        const PlayerSaveGameData& psgd = player_save_game_data[i];
        if (psgd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            // In a single player game, the host player is always the human player, so
            // this is just a matter of finding which entry in player_save_game_data was
            // a human player, and assigning that saved player data to the host player ID
            player_id_to_save_game_data_index[m_networking.HostPlayerID()] = i;

        } else if (psgd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            // All saved AI player data, as determined from their client type, is
            // assigned to player IDs of established AI players

            // cycle to find next established AI player
            while (established_player_it != m_networking.established_end()) {
                const PlayerConnectionPtr player_connection = *established_player_it;
                ++established_player_it;
                // if player is an AI, assign it to this 
                if (player_connection->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER) {
                    int player_id = player_connection->PlayerID();
                    player_id_to_save_game_data_index[player_id] = i;
                    break;
                }
            }
        } else {
            // do nothing for any other player type, until another player type is implemented
            Logger().errorStream() << "ServerApp::LoadSPGameInit skipping unsupported client type in player save game data";
        }
    }

    LoadGameInit(player_save_game_data, player_id_to_save_game_data_index, server_save_game_data);
}

void ServerApp::LoadMPGameInit(const MultiplayerLobbyData& lobby_data,
                               const std::vector<PlayerSaveGameData>& player_save_game_data,
                               boost::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    // Need to determine which data in player_save_game_data should be assigned to which established player
    std::map<int, int> player_id_to_save_game_data_index;


    ServerNetworking::const_established_iterator established_player_it = m_networking.established_begin();
    const std::list<std::pair<int, PlayerSetupData> >& player_setup_data = lobby_data.m_players;

    // * Multiplayer lobby data has a map from player ID to PlayerSetupData.
    // * PlayerSetupData contains an empire ID that the player will be controlling.
    // * PlayerSaveGameData in a vector contain empire ID members.
    // * LoadGameInit (called below) need an index in the PlayerSaveGameData vector
    //   for each player ID
    // => Need to find which index into the PlayerSaveGameData vector has the right
    //    empire id for each player id.

    // assign all saved game data to a player ID
    for (std::list<std::pair<int, PlayerSetupData> >::const_iterator setup_data_it = player_setup_data.begin();
         setup_data_it != player_setup_data.end(); ++setup_data_it)
    {
        int player_id =                 setup_data_it->first;

        const PlayerSetupData& psd =    setup_data_it->second;
        int player_setup_empire_id =    psd.m_save_game_empire_id;

        // find index in PlayerSaveGameData that has same empire ID as PlayerSetupData for this Player ID
        for (int i = 0; i < static_cast<int>(player_save_game_data.size()); ++i) {
            const PlayerSaveGameData& psgd = player_save_game_data.at(i);
            if (psgd.m_empire_id == player_setup_empire_id) {
                player_id_to_save_game_data_index[player_id] = i;
                break;
            }
        }
    }

    LoadGameInit(player_save_game_data, player_id_to_save_game_data_index, server_save_game_data);
}

void ServerApp::LoadGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                             const std::map<int, int>& player_id_to_save_game_data_index,
                             boost::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    Logger().debugStream() << "ServerApp::LoadGameInit";


    // ensure some reasonable inputs
    if (player_save_game_data.empty()) {
        Logger().errorStream() << "ServerApp::LoadGameInit passed empty player save game data.  Aborting";
        return;
    }
    // ensure number of players connected and for which data are provided are consistent
    if (player_id_to_save_game_data_index.size() != player_save_game_data.size()) {
        Logger().errorStream() << "ServerApp::LoadGameInit passed index mapping and player save game data are of different sizes...";
    }
    if (m_networking.NumEstablishedPlayers() != player_save_game_data.size()) {
        Logger().errorStream() << "ServerApp::LoadGameInit has " << m_networking.NumEstablishedPlayers() << " established players but " << player_save_game_data.size() << " entries in player save game data.  Could be ok... so not aborting, but might crash";
    }
    // validate some connection info
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        Networking::ClientType client_type = player_connection->GetClientType();
        if (client_type != Networking::CLIENT_TYPE_AI_PLAYER && client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            Logger().errorStream() << "ServerApp::LoadGameInit found player connection with unsupported client type.";
        }
        if (player_connection->PlayerName().empty()) {
            Logger().errorStream() << "ServerApp::LoadGameInit found player connection with empty name!";
        }
    }


    // clear previous game player state info
    m_turn_sequence.clear();
    m_eliminated_players.clear();
    m_player_empire_ids.clear();


    // restore server state info from save
    m_current_turn =    server_save_game_data->m_current_turn;
    m_victors =         server_save_game_data->m_victors;
    // todo: save and restore m_eliminated_players ?


    // add empires to turn processing and record empires for each player
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();

        // get index indo save game data for this player
        std::map<int, int>::const_iterator index_it = player_id_to_save_game_data_index.find(player_id);
        if (index_it == player_id_to_save_game_data_index.end()) {
            Logger().errorStream() << "ServerApp::LoadGameInit couldn't find save game data index for player with id " << player_id;
            continue;
        }
        int player_save_game_data_index = index_it->second;
        // and get the player's saved game data
        const PlayerSaveGameData& psgd = player_save_game_data.at(player_save_game_data_index);
        int empire_id = psgd.m_empire_id;   // can't use GetPlayerEmpireID here because m_player_empire_ids hasn't been set up yet.


        // record player id to empire id mapping in loaded game.  Player IDs
        // and empire IDs are not necessarily the same when loading a game as
        // the player controlling a particular empire might have a different
        // player ID than when the game was first created
        m_player_empire_ids[player_id] = empire_id;


        // add empires to turn processing, and restore saved orders and UI data or save state data
        if (Empires().Lookup(empire_id)) {
            AddEmpireTurn(empire_id);
        } else {
            Logger().errorStream() << "ServerApp::LoadGameInit couldn't find empire with id " << empire_id << " to add to turn processing";
        }
    }


    // the Universe's system graphs for each empire aren't stored when saving
    // so need to be reinitialized when loading based on the gamestate
    m_universe.RebuildEmpireViewSystemGraphs();


    // Determine supply distribution and exchanging and resource pools for empires
    EmpireManager& empires = Empires();
    for (EmpireManager::iterator it = empires.begin(); it != empires.end(); ++it) {
        if (empires.Eliminated(it->first))
            continue;   // skip eliminated empires.  presumably this shouldn't be an issue when initializing a new game, but apparently I thought this was worth checking for...
        Empire* empire = it->second;

        empire->UpdateSupplyUnobstructedSystems();  // determines which systems can propegate fleet and resource (same for both)
        empire->UpdateSystemSupplyRanges();         // sets range systems can propegate fleet and resourse supply (separately)
        empire->UpdateFleetSupply();                // determines which systems can access fleet supply, and starlane traversals used to do this
        empire->UpdateResourceSupply();             // determines the separate groups of systems within which (but not between which) resources can be shared
        empire->InitResourcePools();                // determines population centers and resource centers of empire, tells resource pools the centers and groups of systems that can share resources (note that being able to share resources doesn't mean a system produces resources)
        empire->UpdateResourcePools();              // determines how much of each resources is available in each resource sharing group
    }

    // self-allocate resources on unowned planets, so natives don't starve
    std::vector<Planet*> planets = m_universe.Objects().FindObjects<Planet>();
    for (std::vector<Planet*>::const_iterator it = planets.begin(); it != planets.end(); ++it)
        if ((*it)->Unowned() && (*it)->CurrentMeterValue(METER_POPULATION) > 0.0)
            (*it)->SetAllocatedFood(std::min((*it)->CurrentMeterValue(METER_FARMING), (*it)->CurrentMeterValue(METER_POPULATION)));


    // compile information about players to send out to other players at start of game.
    Logger().debugStream() << "ServerApp::CommonGameInit: Compiling PlayerInfo for each player";
    std::map<int, PlayerInfo> player_info_map;
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();

        int empire_id = PlayerEmpireID(player_id);
        if (empire_id == ALL_EMPIRES)
            Logger().errorStream() << "ServerApp::CommonGameInit: couldn't find an empire for player with id " << player_id;


        // validate some connection info
        Networking::ClientType client_type = player_connection->GetClientType();
        if (client_type != Networking::CLIENT_TYPE_AI_PLAYER && client_type != Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            Logger().errorStream() << "ServerApp::CommonGameInit found player connection with unsupported client type.";
        }
        if (player_connection->PlayerName().empty()) {
            Logger().errorStream() << "ServerApp::CommonGameInit found player connection with empty name!";
        }


        // assemble player info for all players
        player_info_map[player_id] = PlayerInfo(player_connection->PlayerName(),
                                                empire_id,
                                                player_connection->GetClientType(),
                                                m_networking.PlayerIsHost(player_connection->PlayerID()));
    }


    // send load game start messages
    Logger().debugStream() << "ServerApp::CommonGameInit: Sending GameStartMessages to players";
    for (ServerNetworking::const_established_iterator player_connection_it = m_networking.established_begin();
         player_connection_it != m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();

        // get index indo save game data for this player
        std::map<int, int>::const_iterator index_it = player_id_to_save_game_data_index.find(player_id);
        if (index_it == player_id_to_save_game_data_index.end()) {
            Logger().errorStream() << "ServerApp::LoadGameInit couldn't find save game data index for player with id " << player_id;
            continue;
        }
        int player_save_game_data_index = index_it->second;
        // and get the player's saved game data
        const PlayerSaveGameData& psgd = player_save_game_data.at(player_save_game_data_index);


        int empire_id = PlayerEmpireID(player_id);


        // get saved orders.  these will be re-executed on client and re-sent
        // to the server (after possibly modification) by clients when they
        // end their turn
        boost::shared_ptr<OrderSet> orders =        psgd.m_orders;


        // send game start messages to players.  AIs get a message with a save
        // state string, and human clients get UI data that either has saved.
        Networking::ClientType client_type =        player_connection->GetClientType();
        if (client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            const std::string* sss = 0;
            if (!psgd.m_save_state_string.empty())
                sss = &psgd.m_save_state_string;
            player_connection->SendMessage(GameStartMessage(player_id, m_single_player_game, empire_id,
                                                            m_current_turn, m_empires, m_universe,
                                                            GetSpeciesManager(),
                                                            player_info_map, *orders, sss));

        } else if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            player_connection->SendMessage(GameStartMessage(player_id, m_single_player_game, empire_id,
                                                            m_current_turn, m_empires, m_universe,
                                                            GetSpeciesManager(),
                                                            player_info_map, *orders, psgd.m_ui_data.get()));

        } else {
            Logger().errorStream() << "ServerApp::CommonGameInit unsupported client type: skipping game start message.";
        }
    }
}

Empire* ServerApp::GetPlayerEmpire(int player_id) const
{
    return Empires().Lookup(PlayerEmpireID(player_id));
}

int ServerApp::PlayerEmpireID(int player_id) const
{
    std::map<int, int>::const_iterator it = m_player_empire_ids.find(player_id);
    if (it != m_player_empire_ids.end())
        return it->second;
    else
        return ALL_EMPIRES;
}

int ServerApp::EmpirePlayerID(int empire_id) const
{
    for (std::map<int, int>::const_iterator it = m_player_empire_ids.begin(); it != m_player_empire_ids.end(); ++it)
        if (it->second == empire_id)
            return it->first;
    return Networking::INVALID_PLAYER_ID;
}

void ServerApp::AddEmpireTurn(int empire_id)
{
    m_turn_sequence[empire_id] = 0; // std::map<int, OrderSet*>
}

void ServerApp::RemoveEmpireTurn(int empire_id)
{
    m_turn_sequence.erase(empire_id);
}

void ServerApp::SetEmpireTurnOrders(int empire_id, OrderSet* order_set)
{
    m_turn_sequence[empire_id] = order_set;
}

void ServerApp::ClearEmpireTurnOrders()
{
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        if (it->second) {
            delete it->second;
            it->second = 0;
        }
    }
}

bool ServerApp::AllOrdersReceived()
{
    Logger().debugStream() << "ServerApp::AllOrdersReceived()";

    // Loop through to find empire ID and check for valid orders pointer
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        if (!it->second)
            return false;
    }
    return true;
}

namespace {
    /** Returns true if \a empire has been eliminated by the applicable
      * definition of elimination.  As of this writing, elimination means
      * having no ships and no fleets. */
    bool EmpireEliminated(const Empire* empire, const Universe& universe) {
        if (!empire)
            return false;
        int empire_id = empire->EmpireID();
        return (universe.Objects().FindObjects(OwnedVisitor<Planet>(empire_id)).empty() &&    // no planets
                universe.Objects().FindObjects(OwnedVisitor<Fleet>(empire_id)).empty());      // no fleets
    }

    /** Compiles and return set of ids of empires that are controlled by a
      * human player.*/
    std::set<int> HumanControlledEmpires(const ServerApp* server_app, const ServerNetworking& server_networking) {
        std::set<int> retval;
        if (!server_app)
            return retval;

        for (ServerNetworking::const_established_iterator it = server_networking.established_begin();
             it != server_networking.established_end(); ++it)
        {
            boost::shared_ptr<PlayerConnection> player_connection = *it;
            Networking::ClientType client_type =        player_connection->GetClientType();
            if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
                int player_id =         player_connection->PlayerID();
                int empire_id =         server_app->PlayerEmpireID(player_id);
                if (empire_id == ALL_EMPIRES || player_id == Networking::INVALID_PLAYER_ID)
                    Logger().errorStream() << "HumanControlledEmpires couldn't get a human player id or empire id";
                else
                    retval.insert(empire_id);
            }
        }
        return retval;
    }

    void GetEmpireIDsWithFleetsAndCombatFleetsAtSystem(std::set<int>& ids_of_empires_with_fleets_here,
                                                       std::set<int>& ids_of_empires_with_combat_fleets_here,
                                                       int system_id)
    {
        ids_of_empires_with_fleets_here.clear();
        ids_of_empires_with_combat_fleets_here.clear();

        const System* system = GetObject<System>(system_id);
        if (!system)
            return;

        std::vector<int> fleet_ids = system->FindObjectIDs<Fleet>();
        for (std::vector<int>::const_iterator fleet_it = fleet_ids.begin(); fleet_it != fleet_ids.end(); ++fleet_it) {
            const Fleet* fleet = GetObject<Fleet>(*fleet_it);
            if (!fleet) {
                Logger().errorStream() << "GetEmpireIDsWithFleetsAndCombatFleetsAtSystem couldn't get Fleet with id " << *fleet_it;
                continue;
            }

            ids_of_empires_with_fleets_here.insert(fleet->Owner()); // may be ALL_EMPIRES

            if (fleet->HasArmedShips())
                ids_of_empires_with_combat_fleets_here.insert(fleet->Owner());
        }
    }

    void GetEmpireIDsWithPlanetsAtSystem(std::set<int>& ids_of_empires_with_planets_here, int system_id) {
        ids_of_empires_with_planets_here.clear();

        const System* system = GetObject<System>(system_id);
        if (!system)
            return;

        std::vector<int> planet_ids = system->FindObjectIDs<Planet>();
        for (std::vector<int>::const_iterator planet_it = planet_ids.begin(); planet_it != planet_ids.end(); ++planet_it) {
            const Planet* planet = GetObject<Planet>(*planet_it);
            if (!planet) {
                Logger().errorStream() << "GetEmpireIDsWithPlanetsAtSystem couldn't get Planet with id " << *planet_it;
                continue;
            }
            if (planet->CurrentMeterValue(METER_POPULATION) > 0.0)
                ids_of_empires_with_planets_here.insert(planet->Owner());
        }
    }


    /** Returns true iff there is an appropriate combination of objects in the
      * system with id \a system_id for a combat to occur. */
    bool CombatConditionsInSystem(int system_id) {

        std::set<int> ids_of_empires_with_combat_fleets_here;
        std::set<int> ids_of_empires_with_fleets_here;
        GetEmpireIDsWithFleetsAndCombatFleetsAtSystem(ids_of_empires_with_fleets_here, ids_of_empires_with_combat_fleets_here, system_id);

        // combat can occur if more than one empire has a combat fleet in system
        if (ids_of_empires_with_combat_fleets_here.size() > 1)
            return true;

        // combat can occur if one empire has a combat fleet and another empire
        // has any fleet
        if (!ids_of_empires_with_combat_fleets_here.empty() && ids_of_empires_with_fleets_here.size() > 1)
            return true;


        std::set<int> ids_of_empires_with_planets_here;
        GetEmpireIDsWithPlanetsAtSystem(ids_of_empires_with_planets_here, system_id);


        // combat can also occur if there is at least one fleet and one
        // other empire's planetary defenses that can harm the fleets
        if (!ids_of_empires_with_combat_fleets_here.empty() && !ids_of_empires_with_planets_here.empty()) {

            for (std::set<int>::const_iterator planet_owner_it = ids_of_empires_with_planets_here.begin();
                 planet_owner_it != ids_of_empires_with_planets_here.end();
                 ++planet_owner_it)
            {
                int planet_owner_id = *planet_owner_it;

                // find a combat fleet owned by a different empire
                for (std::set<int>::const_iterator combat_fleet_owner_it = ids_of_empires_with_combat_fleets_here.begin();
                     combat_fleet_owner_it != ids_of_empires_with_combat_fleets_here.end();
                     ++combat_fleet_owner_it)
                {
                    int combat_fleet_owner_id = *combat_fleet_owner_it;
                    if (combat_fleet_owner_id != planet_owner_id)
                        return true;
                }
            }
        }

        return false;   // no possible conditions for combat were found
    }

    /** Cleans up CombatInfo within \a system_combat_info. */
    void CleanupSystemCombatInfo(std::map<int, CombatInfo>& system_combat_info) {
        for (std::map<int, CombatInfo>::iterator it = system_combat_info.begin(); it != system_combat_info.end(); ++it)
            it->second.Clear();
        system_combat_info.clear();
    }

    /** Clears and refills \a system_combat_info with CombatInfo structs for
      * every system where a combat should occur this turn. */
    void AssembleSystemCombatInfo(std::map<int, CombatInfo>& system_combat_info) {
        CleanupSystemCombatInfo(system_combat_info);

        // for each system, find if a combat will occur in it, and if so, assemble
        // necessary information about that combat in system_combat_info
        std::vector<int> sys_ids = GetUniverse().Objects().FindObjectIDs<System>();

        for (std::vector<int>::const_iterator it = sys_ids.begin(); it != sys_ids.end(); ++it) {
            int sys_id = *it;
            if (CombatConditionsInSystem(sys_id))
                system_combat_info.insert(std::make_pair(sys_id, CombatInfo(sys_id)));
        }
    }

    /** Back project meter values of objects in combat info, so that changes to
      * meter values from combat aren't lost when resetting meters during meter
      * updating after combat. */
    void BackProjectSystemCombatInfoObjectMeters(std::map<int, CombatInfo>& system_combat_info) {
        for (std::map<int, CombatInfo>::iterator it = system_combat_info.begin(); it != system_combat_info.end(); ++it) {
            ObjectMap& objects = it->second.objects;
            for (ObjectMap::iterator obj_it = objects.begin(); obj_it != objects.end(); ++obj_it)
                obj_it->second->BackPropegateMeters();
            }
    }

    /** Takes contents of CombatInfo struct and puts it into the universe.
      * Used to store results of combat in main universe. */
    void DisseminateSystemCombatInfo(const std::map<int, CombatInfo>& system_combat_info) {
        Universe& universe = GetUniverse();

        // loop through resolved combat infos, updating actual main universe
        // with changes from combat
        for (std::map<int, CombatInfo>::const_iterator system_it = system_combat_info.begin();
             system_it != system_combat_info.end();
             ++system_it)
        {
            const CombatInfo& combat_info = system_it->second;

            //// DEBUG
            //const System* combat_system = combat_info.GetSystem();
            //Logger().debugStream() << "DisseminateSystemCombatInfo for combat at " << (combat_system ? combat_system->Name() : "(No System)");
            //Logger().debugStream() << combat_info.objects.Dump();
            //for (std::map<int, ObjectMap>::const_iterator eko_it = combat_info.empire_known_objects.begin(); eko_it != combat_info.empire_known_objects.end(); ++eko_it) {
            //    Logger().debugStream() << "known objects for empire " << eko_it->first;
            //    Logger().debugStream() << eko_it->second.Dump();
            //}
            //// END DEBUG


            // copy actual state of objects in combat after it was resolved
            universe.Objects().Copy(combat_info.objects);


            // copy empires' latest known state of objects in combat after it was resolved
            for (std::map<int, ObjectMap>::const_iterator eko_it = combat_info.empire_known_objects.begin();
                 eko_it != combat_info.empire_known_objects.end();
                 ++eko_it)
            {
                const ObjectMap& combat_known_objects = eko_it->second;
                int empire_id = eko_it->first;
                ObjectMap& actual_known_objects = universe.EmpireKnownObjects(empire_id);

                //Logger().debugStream() << "copying known objects from combat to main gamestate for empire " << empire_id;

                actual_known_objects.Copy(combat_known_objects);
            };


            // destroy in main universe objects that were destroyed in combat,
            // and any associated objects that should now logically also be
            // destroyed
            for (std::set<int>::const_iterator do_it = combat_info.destroyed_object_ids.begin();
                 do_it != combat_info.destroyed_object_ids.end();
                 ++do_it)
            {
                int destroyed_object_id = *do_it;
                universe.RecursiveDestroy(destroyed_object_id);
            }


            // update which empires know objects are destroyed.  this may
            // duplicate the destroyed object knowledge that is set when the
            // object is destroyed with Universe::Destroy, but there may also
            // be empires in this battle that otherwise couldn't see the object
            // as determined for galaxy map purposes, but which do know it has
            // been destroyed from having observed it during the battle.
            for (std::map<int, std::set<int> >::const_iterator dok_it = combat_info.destroyed_object_knowers.begin();
                 dok_it != combat_info.destroyed_object_knowers.end();
                 ++dok_it)
            {
                int empire_id = dok_it->first;
                const std::set<int>& object_ids = dok_it->second;

                for (std::set<int>::const_iterator object_it = object_ids.begin(); object_it != object_ids.end(); ++object_it) {
                    int object_id = *object_it;
                    universe.SetEmpireKnowledgeOfDestroyedObject(object_id, empire_id);
                }
            }


            // update system ownership after combat.  may be necessary if the
            // combat caused planets to change ownership.
            if (System* system = GetObject<System>(combat_info.system_id)) {
                system->UpdateOwnership();

                // ensure all participants get updates on system.  this ensures
                // that an empire who lose all objects in the system still
                // knows about a change in system ownership
                for (std::set<int>::const_iterator empire_it = combat_info.empire_ids.begin();
                     empire_it != combat_info.empire_ids.end(); ++empire_it)
                {
                    universe.EmpireKnownObjects(*empire_it).Copy(system, ALL_EMPIRES);
                }
            }
        }
    }

    /** Creates sitreps for all empires involved in a combat. */
    void CreateCombatSitReps(const std::map<int, CombatInfo>& system_combat_info) {
        for (std::map<int, CombatInfo>::const_iterator it = system_combat_info.begin();
             it != system_combat_info.end();
             ++it)
        {
            // basic "combat occured" sitreps
            const CombatInfo& combat_info = it->second;
            const std::set<int>& empire_ids = combat_info.empire_ids;
            for (std::set<int>::const_iterator empire_it = empire_ids.begin(); empire_it != empire_ids.end(); ++empire_it) {

                Empire* empire = Empires().Lookup(*empire_it);
                if (!empire)
                    continue;
                empire->AddSitRepEntry(CreateCombatSitRep(combat_info.system_id));
            }

            // sitreps about destroyed objects
            for (std::map<int, std::set<int> >::const_iterator empire_known_destroyed_objects_it = combat_info.destroyed_object_knowers.begin();
                 empire_known_destroyed_objects_it != combat_info.destroyed_object_knowers.end(); ++empire_known_destroyed_objects_it)
            {
                int empire_id = empire_known_destroyed_objects_it->first;
                Empire* empire = Empires().Lookup(empire_id);
                if (!empire)
                    continue;

                const std::set<int>& destroyed_object_ids = empire_known_destroyed_objects_it->second;
                for (std::set<int>::const_iterator dest_obj_it = destroyed_object_ids.begin();
                     dest_obj_it != destroyed_object_ids.end(); ++dest_obj_it)
                {
                    int destroyed_object_id = *dest_obj_it;
                    empire->AddSitRepEntry(CreateCombatDestroyedObjectSitRep(*dest_obj_it, combat_info.system_id, empire_id));
                }
            }
        }
    }
}

namespace {
    /** Does colonization, with safety checks */
    bool ColonizePlanet(ObjectMap& objects, int ship_id, int planet_id) {
        Ship* ship = objects.Object<Ship>(ship_id);
        if (!ship) {
            Logger().errorStream() << "ColonizePlanet couldn't get ship with id " << ship_id;
            return false;
        }
        Planet* planet = objects.Object<Planet>(planet_id);
        if (!planet) {
            Logger().errorStream() << "ColonizePlanet couldn't get planet with id " << planet_id;
            return false;
        }

        // get species to colonize with: species of ship
        const std::string& species_name = ship->SpeciesName();
        if (species_name.empty()) {
            Logger().errorStream() << "ColonizePlanet ship has no species";
            return false;
        }
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "ColonizePlanet couldn't get species with name: " << species_name;
            return false;
        }

        const ShipDesign* design = ship->Design();
        if (!design) {
            Logger().errorStream() << "ColonizePlanet couldn't find ship's design!";
            return false;
        }
        double colonist_capacity = design->ColonyCapacity();

        if (colonist_capacity <= 0.0) {
            Logger().debugStream() << "ColonizePlanet colonize order executed by ship with zero colonist capacity!";
            return false;
        }

        // get empire to give ownership of planet to
        if (ship->Unowned()) {
            Logger().errorStream() << "ColonizePlanet couldn't get an empire to colonize with";
            return false;
        }
        int empire_id = ship->Owner();

        // all checks passed.  proceed with colonization.


        planet->Reset();
        planet->SetSpecies(species_name);   // do this BEFORE destroying the ship, since species_name is a const reference to Ship::m_species_name

        GetUniverse().Destroy(ship->ID());


        // find a focus to give planets by default.  use first defined available focus.
        // the planet's AvailableFoci function should return a vector of all names of
        // available foci.
        std::vector<std::string> available_foci = planet->AvailableFoci();
        if (!available_foci.empty())
            planet->SetFocus(*available_foci.begin());

        planet->GetMeter(METER_POPULATION)->SetCurrent(colonist_capacity);
        planet->GetMeter(METER_TARGET_POPULATION)->SetCurrent(colonist_capacity);
        planet->GetMeter(METER_FARMING)->SetCurrent(colonist_capacity);
        planet->GetMeter(METER_TARGET_FARMING)->SetCurrent(colonist_capacity);
        planet->GetMeter(METER_HEALTH)->SetCurrent(20.0);
        planet->GetMeter(METER_TARGET_HEALTH)->SetCurrent(20.0);
        planet->BackPropegateMeters();

        planet->SetOwner(empire_id);

        const std::set<int>& planet_buildings = planet->Buildings();
        for (std::set<int>::const_iterator it = planet_buildings.begin(); it != planet_buildings.end(); ++it) {
            if (Building* building = objects.Object<Building>(*it)) {
                building->SetOwner(empire_id); // TODO: only add owner if empire has visibility of building.  Need to add a check for this every turn... maybe doesn't need to be done during colonization at all?
            } else {
                Logger().errorStream() << "ColonizePlanet couldn't find building with id: " << *it;
            }
        }

        return true;
    }

    /** Determines which ships ordered to colonize planet succeed, does
      * appropriate colonization, and cleans up after colonization orders */
    void HandleColonization(ObjectMap& objects, EmpireManager& empires) {
        // collect, for each planet, what ships have been ordered to colonize it
        std::map<int, std::map<int, std::set<int> > > planet_empire_colonization_ship_ids; // map from planet ID to map from empire ID to set of ship IDs
        std::vector<Ship*> ships = objects.FindObjects<Ship>();

        for (std::vector<Ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
            const Ship* ship = *it;
            if (!ship) {
                Logger().errorStream() << "HandleColonization couldn't get ship";
                continue;
            }
            if (ship->Unowned())
                continue;
            int owner_empire_id = ship->Owner();
            int ship_id = ship->ID();
            if (ship_id == UniverseObject::INVALID_OBJECT_ID)
                continue;

            int colonize_planet_id = ship->OrderedColonizePlanet();
            if (colonize_planet_id == UniverseObject::INVALID_OBJECT_ID)
                continue;
            Planet* planet = objects.Object<Planet>(colonize_planet_id);
            if (!planet)
                continue;
            planet->ResetIsAboutToBeColonized();
            if (!planet->Unowned())
                continue;

            if (ship->SystemID() != planet->SystemID() || ship->SystemID() == UniverseObject::INVALID_OBJECT_ID)
                continue;

            planet_empire_colonization_ship_ids[colonize_planet_id][owner_empire_id].insert(ship_id);
        }


        std::vector<int> newly_colonize_planet_ids;


        // execute colonization when:
        // 1) a single empire is colonizing a planet in a system that contains no enemy armed ships
        // 2) multiple empires try to colonize the same planet, but only one empire has armed ships in the system
        // otherwise, no colonization happens in the system
        for (std::map<int, std::map<int, std::set<int> > >::iterator planet_it = planet_empire_colonization_ship_ids.begin();
             planet_it != planet_empire_colonization_ship_ids.end();
             ++planet_it)
        {
            int planet_id = planet_it->first;
            Planet* planet = objects.Object<Planet>(planet_id);
            if (!planet) {
                Logger().errorStream() << "HandleColonization couldn't get planet with id " << planet_id;
                continue;
            }
            if (!planet->Unowned()) {
                Logger().errorStream() << "HandleColonization trying to colonize an already-occupied planet!";
                continue;
            }

            int system_id = planet->SystemID();
            const System* system = objects.Object<System>(system_id);
            if (!system) {
                Logger().errorStream() << "HandleColonization couldn't get system with id " << system_id;
                continue;
            }

            // find which empires have armed ships in system
            std::set<int> empires_with_armed_ships_in_system;
            std::vector<int> system_fleet_ids = system->FindObjectIDs<Fleet>();
            for (std::vector<int>::const_iterator fleet_it = system_fleet_ids.begin(); fleet_it != system_fleet_ids.end(); ++fleet_it) {
                const Fleet* fleet = objects.Object<Fleet>(*fleet_it);
                if (fleet->HasArmedShips())
                    empires_with_armed_ships_in_system.insert(fleet->Owner());  // may include ALL_EMPIRES, which is fine; this makes monsters prevent colonization
            }

            // no empires can colonize if there are more than one empire's armed fleets in system
            if (empires_with_armed_ships_in_system.size() > 1)
                continue;

            bool planet_colonized = false;

            // check if any of the ships that want to colonize belong to the empire with armed ships
            std::map<int, std::set<int> >& empire_colonization_ship_map = planet_it->second;
            for (std::map<int, std::set<int> >::iterator empire_it = empire_colonization_ship_map.begin();
                 empire_it != empire_colonization_ship_map.end();
                 ++empire_it)
            {
                int empire_id = empire_it->first;
                if (!empires_with_armed_ships_in_system.empty() &&
                    empires_with_armed_ships_in_system.find(empire_id) == empires_with_armed_ships_in_system.end())
                {
                    // this empire can't colonize here this turn
                    continue;
                }

                std::set<int>& colonizing_ships = empire_it->second;
                if (colonizing_ships.empty()) {
                    Logger().errorStream() << "HandleColonization got empty set of ships attempting to colonize planet for an empire?!";
                    continue;
                }

                // pick the first ship this empire has that is trying to colonize this planet.
                int ship_id = *colonizing_ships.begin();


                // do colonization
                if (!ColonizePlanet(objects, ship_id, planet_id))
                    continue;   // skip sitrep if colonization failed

                // record successful colonization
                planet_colonized = true;
                newly_colonize_planet_ids.push_back(planet_id);

                // remove ship from ships that wanted to colonize
                colonizing_ships.erase(ship_id);

                // sitrep about colonization
                Empire* empire = empires.Lookup(empire_id);
                if (!empire) {
                    Logger().errorStream() << "HandleColonization couldn't get empire with id " << empire_id;
                } else {
                    empire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet_id));
                }

                break;  // only one colonization per planet.
            }

            // if planet was colonized, remove colonize status from any other
            // ships that wanted to colonize this planet
            if (planet_colonized) {
                for (std::map<int, std::set<int> >::iterator empire_it = empire_colonization_ship_map.begin();
                     empire_it != empire_colonization_ship_map.end();
                     ++empire_it)
                {
                    std::set<int>& colonizing_ships = empire_it->second;
                    for (std::set<int>::const_iterator ship_it = colonizing_ships.begin(); ship_it != colonizing_ships.end(); ++ship_it)
                        if (Ship* ship = objects.Object<Ship>(*ship_it))
                            ship->ClearColonizePlanet();
                }
            }
        }

        // update food allocation meters for newly colonized planets, so they don't starve themselves later this turn

        // TODO elsewhere: clear colonization planet of ships when that planet
        //      has been destroyed or become invisible, or if the ship leaves
        //      the system?
    }

    /** Given initial set of ground forces on planet, determine ground forces on
      * planet after a turn of ground combat. */
    void ResolveGroundCombat(std::map<int, double>& empires_troops) {
        if (empires_troops.empty())
            return;
        int victor_id = empires_troops.begin()->first;
        double victor_troops = empires_troops.begin()->second;
        for (std::map<int, double>::iterator it = empires_troops.begin(); it != empires_troops.end(); ++it) {
            if (it->second > victor_troops) {
                victor_troops = it->second;
                victor_id = it->first;
            }
        }
        empires_troops.clear();
        empires_troops[victor_id] = victor_troops;  // wipe everyone else out, with no casualties!
    }

    /** Determines which ships ordered to invade planets, does invasion and
      * ground combat resolution */
    void HandleInvasion(ObjectMap& objects, EmpireManager& empires) {
        // collect, for each planet, what ships have been ordered to invade it
        std::map<int, std::map<int, double> > planet_empire_troops;  // map from planet ID to map from empire ID to pair consisting of set of ship IDs and amount of troops empires have at planet
        std::vector<Ship*> ships = objects.FindObjects<Ship>();

        for (std::vector<Ship*>::iterator it = ships.begin(); it != ships.end(); ++it) {
            const Ship* ship = *it;
            if (!ship) {
                Logger().errorStream() << "HandleInvasion couldn't get ship";
                continue;
            }
            if (!ship->HasTroops())     // can't invade without troops
                continue;
            if (ship->SystemID() == UniverseObject::INVALID_OBJECT_ID)
                continue;
            int ship_id = ship->ID();
            if (ship_id == UniverseObject::INVALID_OBJECT_ID)
                continue;
            const ShipDesign* design = ship->Design();
            if (!design)
                continue;

            int invade_planet_id = ship->OrderedInvadePlanet();
            if (invade_planet_id == UniverseObject::INVALID_OBJECT_ID)
                continue;
            Planet* planet = objects.Object<Planet>(invade_planet_id);
            if (!planet)
                continue;
            planet->ResetIsAboutToBeInvaded();
            if (planet->CurrentMeterValue(METER_SHIELD) > 0.0)
                continue;               // can't invade shielded planets
            if (planet->CurrentMeterValue(METER_POPULATION) <= 0.0)
                continue;               // can't invade unpopulated planets

            if (ship->SystemID() != planet->SystemID())
                continue;

            // how many troops are invading?
            planet_empire_troops[invade_planet_id][ship->Owner()] += design->TroopCapacity();
            // destroy invading ships
            GetUniverse().Destroy(ship_id);
            // current owner may also have troops from meter
            planet_empire_troops[invade_planet_id][planet->Owner()] += planet->CurrentMeterValue(METER_TROOPS) + 0.0001;    // small bonus to ensure ties are won by initial owner
        }

        // process each planet's invasions
        for (std::map<int, std::map<int, double> >::iterator planet_it = planet_empire_troops.begin();
             planet_it != planet_empire_troops.end(); ++planet_it)
        {
            int planet_id = planet_it->first;
            Planet* planet = objects.Object<Planet>(planet_id);
            // mark planet as no longer being invaded
            planet->ResetIsAboutToBeInvaded();

            std::map<int, double>& empires_troops = planet_it->second;
            ResolveGroundCombat(empires_troops);

            // who won?
            if (empires_troops.size() == 1) {
                int victor_id = empires_troops.begin()->first;
                // single empire was victorious.  conquer planet if appropriate...
                // if planet is unowned and victor is an empire, or if planet is
                // owned by an empire that is not the victor, conquer it
                if ((planet->Unowned() && victor_id != ALL_EMPIRES) ||
                    (!planet->Unowned() && !planet->OwnedBy(victor_id)))
                {
                    planet->Conquer(victor_id);
                }

                // regardless of whether battle resulted in conquering, it did
                // likely affect the troop numbers on the planet
                if (Meter* meter = planet->GetMeter(METER_TROOPS))
                    meter->SetCurrent(empires_troops.begin()->second);  // new troops on planet is remainder after battle

            } else {
                // no troops left?
                if (Meter* meter = planet->GetMeter(METER_TROOPS))
                    meter->SetCurrent(0.0);
            }
        }
    }
}

void ServerApp::PreCombatProcessTurns()
{
    EmpireManager& empires = Empires();
    ObjectMap& objects = m_universe.Objects();


    m_universe.RebuildEmpireViewSystemGraphs();


    Logger().debugStream() << "ServerApp::ProcessTurns executing orders";

    // inform players of order execution
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it)
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->PlayerID(), Message::PROCESSING_ORDERS));

    // execute orders
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        Empire* empire = empires.Lookup(it->first);
        empire->ClearSitRep();
        OrderSet* order_set = it->second;

        for (OrderSet::const_iterator order_it = order_set->begin(); order_it != order_set->end(); ++order_it)
            order_it->second->Execute();
    }



    Logger().debugStream() << "ServerApp::ProcessTurns colonization and scrapping";

    // player notifications
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->PlayerID(), Message::COLONIZE_AND_SCRAP));
    }


    // clean up orders, which are no longer needed
    ClearEmpireTurnOrders();


    HandleColonization(objects, empires);
    HandleInvasion(objects, empires);


    // scrap orders
    std::vector<int> objects_to_scrap;
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it) {
        int object_id = it->first;
        if (Ship* ship = universe_object_cast<Ship*>(it->second)) {
            if (ship->OrderedScrapped())
                objects_to_scrap.push_back(object_id);
        } else if (Building* building = universe_object_cast<Building*>(it->second)) {
            if (building->OrderedScrapped())
                objects_to_scrap.push_back(object_id);
        }
    }
    for (std::vector<int>::const_iterator it = objects_to_scrap.begin(); it != objects_to_scrap.end(); ++it)
        m_universe.Destroy(*it);

    // check for empty fleets after scrapping
    std::vector<Fleet*> fleets = objects.FindObjects<Fleet>();
    for (std::vector<Fleet*>::iterator it = fleets.begin(); it != fleets.end(); ++it) {
        if (Fleet* fleet = *it)
            if (fleet->Empty())
                m_universe.Destroy(fleet->ID());
    }


    Logger().debugStream() << "ServerApp::ProcessTurns movement";
    // process movement phase

    // player notifications
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->PlayerID(), Message::FLEET_MOVEMENT));
    }

    // fleet movement
    fleets = objects.FindObjects<Fleet>();
    for (std::vector<Fleet*>::iterator it = fleets.begin(); it != fleets.end(); ++it) {
        // save for possible SitRep generation after moving...
        Fleet* fleet = *it;
        if (!fleet)
            continue;

        fleet->MovementPhase();

        // TODO: Do movement incrementally, and if the moving fleet encounters
        // stationary combat fleets or planetary defenses that can hurt it, it
        // must be resolved as a combat.

        // SitRep for fleets having arrived at destinations, to all owners of those fleets
        if (fleet->ArrivedThisTurn() && !fleet->Unowned()) {
            Empire* empire = empires.Lookup(fleet->Owner());
            if (!empire)
                continue;
            empire->AddSitRepEntry(CreateFleetArrivedAtDestinationSitRep(fleet->SystemID(), fleet->ID()));
        }
    }


    // post-movement visibility update
    m_universe.UpdateEmpireObjectVisibilities();
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();


    // update fleet routes after movement
    for (std::vector<Fleet*>::iterator it = fleets.begin(); it != fleets.end(); ++it) {
        Fleet* fleet = *it;
        if (fleet)
            fleet->CalculateRoute();
    }

    // indicate that the clients are waiting for their new Universes
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin();
         player_it != m_networking.established_end();
         ++player_it)
    {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->PlayerID(), Message::DOWNLOADING));
    }

    // send partial turn updates to all players after orders and movement
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin();
         player_it != m_networking.established_end();
         ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        int player_id = player->PlayerID();
        player->SendMessage(TurnPartialUpdateMessage(player_id,
                                                     PlayerEmpireID(player_id),
                                                     m_universe));
    }
}

void ServerApp::ProcessCombats()
{
    Logger().debugStream() << "ServerApp::ProcessCombats";
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->PlayerID(), Message::COMBAT));
    }

    std::set<int> human_controlled_empire_ids = HumanControlledEmpires(this, m_networking);
    std::map<int, CombatInfo> system_combat_info;   // map from system ID to CombatInfo for that system


    // collect data about locations where combat is to occur
    AssembleSystemCombatInfo(system_combat_info);

    // TODO: inform players of locations of controllable combats, and get
    // players to specify which should be controlled and which should be
    // auto-resolved

    // loop through assembled combat infos, handling each combat to update the
    // various systems' CombatInfo structs
    for (std::map<int, CombatInfo>::iterator it = system_combat_info.begin(); it != system_combat_info.end(); ++it) {
        CombatInfo& combat_info = it->second;

        //// DEBUG
        //const System* combat_system = combat_info.GetSystem();
        //Logger().debugStream() << "Processing combat at " << (combat_system ? combat_system->Name() : "(No System)");
        //Logger().debugStream() << combat_info.objects.Dump();
        //for (std::map<int, ObjectMap>::const_iterator eko_it = combat_info.empire_known_objects.begin(); eko_it != combat_info.empire_known_objects.end(); ++eko_it) {
        //    Logger().debugStream() << "known objects for empire " << eko_it->first;
        //    Logger().debugStream() << eko_it->second.Dump();
        //}
        //// END DEBUG

        // TODO: Remove this up-front check when the 3D combat system is in
        // place
        if (!GetOptionsDB().Get<bool>("test-3d-combat")) {
            AutoResolveCombat(combat_info);
            continue;
        }

        std::set<int>& empire_ids = combat_info.empire_ids;

        // find which human players are involved in this battle
        std::set<int> human_empires_involved;
        for (std::set<int>::const_iterator empires_with_fleets_it = empire_ids.begin(); empires_with_fleets_it != empire_ids.end(); ++empires_with_fleets_it) {
            int empire_id = *empires_with_fleets_it;
            if (human_controlled_empire_ids.find(empire_id) != human_controlled_empire_ids.end())
                human_empires_involved.insert(empire_id);
        }

        // if no human players are involved, resolve battle automatically
        if (human_empires_involved.empty()) {
            AutoResolveCombat(combat_info);
            continue;
        }

        // TODO: Until there is a fully-implemented interactive combat system
        // to use, we autoresolve anyway, unless we're testing the
        // in-development 3D system.
        if (GetOptionsDB().Get<bool>("test-3d-combat")) {
            m_fsm->process_event(
                ResolveCombat(GetObject<System>(combat_info.system_id), combat_info.empire_ids));
            while (m_current_combat) {
                m_io_service.run_one();
                m_networking.HandleNextEvent();
            }
        } else {
            AutoResolveCombat(combat_info);
        }
    }

    BackProjectSystemCombatInfoObjectMeters(system_combat_info);

    DisseminateSystemCombatInfo(system_combat_info);

    CreateCombatSitReps(system_combat_info);

    CleanupSystemCombatInfo(system_combat_info);
}

void ServerApp::PostCombatProcessTurns()
{
    EmpireManager& empires = Empires();
    ObjectMap& objects = m_universe.Objects();

    // post-combat visibility update
    m_universe.UpdateEmpireObjectVisibilities();
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();


    // check for loss of empire capitals
    for (EmpireManager::iterator empire_it = empires.begin(); empire_it != empires.end(); ++empire_it) {
        int capital_id = empire_it->second->CapitalID();
        if (const UniverseObject* capital = GetObject(capital_id)) {
            if (!capital->OwnedBy(empire_it->first))
                empire_it->second->SetCapitalID(UniverseObject::INVALID_OBJECT_ID);
        } else {
            empire_it->second->SetCapitalID(UniverseObject::INVALID_OBJECT_ID);
        }
    }


    // process production and growth phase

    // notify players that production and growth is being processed
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->PlayerID(), Message::EMPIRE_PRODUCTION));
    }


    Logger().debugStream() << "ServerApp::ProcessTurns effects and meter updates";


    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!!!!!BEFORE TURN PROCESSING EFFECTS APPLICATION";
    Logger().debugStream() << objects.Dump();

    // execute all effects and update meters prior to production, research, etc.
    m_universe.ApplyAllEffectsAndUpdateMeters();


    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!!!!!AFTER TURN PROCESSING EFFECTS APPLICATION";
    Logger().debugStream() << objects.Dump();


    Logger().debugStream() << "ServerApp::ProcessTurns empire resources updates";


    // Determine how much of each resource is available, and determine how to distribute it to planets or on queues
    for (EmpireManager::iterator it = empires.begin(); it != empires.end(); ++it) {
        if (empires.Eliminated(it->first))
            continue;   // skip eliminated empires
        Empire* empire = it->second;

        empire->UpdateSupplyUnobstructedSystems();  // determines which systems can propegate fleet and resource (same for both)
        empire->UpdateSystemSupplyRanges();         // sets range systems can propegate fleet and resourse supply (separately)
        empire->UpdateFleetSupply();                // determines which systems can access fleet supply, and starlane traversals used to do this
        empire->UpdateResourceSupply();             // determines the separate groups of systems within which (but not between which) resources can be shared
        empire->InitResourcePools();                // determines population centers and resource centers of empire, tells resource pools the centers and groups of systems that can share resources (note that being able to share resources doesn't mean a system produces resources)
        empire->UpdateResourcePools();              // determines how much of each resources is available in each resource sharing group
    }

    // self-allocate resources on unowned planets, so natives don't starve
    std::vector<Planet*> planets = m_universe.Objects().FindObjects<Planet>();
    for (std::vector<Planet*>::const_iterator it = planets.begin(); it != planets.end(); ++it)
        if ((*it)->Unowned() && (*it)->CurrentMeterValue(METER_POPULATION) > 0.0)
            (*it)->SetAllocatedFood(std::min((*it)->CurrentMeterValue(METER_FARMING), (*it)->CurrentMeterValue(METER_POPULATION)));


    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!!!!!AFTER UPDATING RESOURCE POOLS AND SUPPLY STUFF";
    Logger().debugStream() << objects.Dump();


    Logger().debugStream() << "ServerApp::ProcessTurns queue progress checking";

    // Consume distributed resources to planets and on queues, create new
    // objects for completed production and give techs to empires that have
    // researched them
    for (EmpireManager::iterator it = empires.begin(); it != empires.end(); ++it) {
        if (empires.Eliminated(it->first))
            continue;   // skip eliminated empires
        Empire* empire = it->second;
        empire->CheckResearchProgress();
        empire->CheckProductionProgress();
        empire->CheckTradeSocialProgress();
        empire->CheckGrowthFoodProgress();
    }


    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!!!!!AFTER CHECKING QUEUE AND RESOURCE PROGRESS";
    Logger().debugStream() << objects.Dump();


    // determine the IDs of new objects created before or during queue processing
    std::vector<int> new_object_ids;
    int current_turn = CurrentTurn();   // skip objects created on invalid turns or before this turn
    if (current_turn >= 0 && current_turn != IMPOSSIBLY_LARGE_TURN) {
        for (ObjectMap::const_iterator it = objects.const_begin(); it != objects.const_end(); ++it)
            if (it->second->CreationTurn() == current_turn)
                new_object_ids.push_back(it->first);
    }

    // Execute meter-related effects on objects created this turn, so that new
    // UniverseObjects will have effects applied to them this turn, allowing
    // (for example) ships to have max fuel meters greater than 0 on the turn
    // they are created.
    m_universe.ApplyMeterEffectsAndUpdateMeters();


    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!!!!!AFTER UPDATING METERS OF ALL OBJECTS";
    Logger().debugStream() << objects.Dump();


    // Population growth or loss, health meter growth, resource current meter
    // growth, etc.
    for (ObjectMap::iterator it = objects.begin(); it != objects.end(); ++it) {
        it->second->PopGrowthProductionResearchPhase();
        it->second->ClampMeters();  // ensures growth doesn't leave meters over MAX.  should otherwise be redundant with ClampMeters() in Universe::ApplyMeterEffectsAndUpdateMeters()
    }

    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!!!!!AFTER GROWTH AND CLAMPING";
    Logger().debugStream() << objects.Dump();


    // check for loss of empire capitals
    for (EmpireManager::iterator empire_it = empires.begin(); empire_it != empires.end(); ++empire_it) {
        int capital_id = empire_it->second->CapitalID();
        if (const UniverseObject* capital = GetObject(capital_id)) {
            if (!capital->OwnedBy(empire_it->first))
                empire_it->second->SetCapitalID(UniverseObject::INVALID_OBJECT_ID);
        } else {
            empire_it->second->SetCapitalID(UniverseObject::INVALID_OBJECT_ID);
        }
    }


    // store initial values of meters for this turn.
    m_universe.BackPropegateObjectMeters();


    // store any changes in objects from various progress functions, such as
    // starvation of planets, before updating visibility again, so that if the
    // visibility update removes an empires ability to detect an object, the
    // empire will still know the latest state (eg. 0 population planet) on the
    // turn when the empire did have detection ability for the object
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();



    // post-production and meter-effects visibility update
    m_universe.UpdateEmpireObjectVisibilities();


    // regenerate empire system graphs based on latest visibility information.
    // this is used by CalculateRoute and other system connectivity tests
    m_universe.RebuildEmpireViewSystemGraphs();



    // update fleet routes after combat, production, growth, effects, etc.
    std::vector<Fleet*> fleets = objects.FindObjects<Fleet>();
    for (std::vector<Fleet*>::iterator it = fleets.begin(); it != fleets.end(); ++it) {
        Fleet* fleet = *it;
        if (fleet)
            fleet->CalculateRoute();
    }

    Logger().debugStream() << "!!!!!!!!!!!!!!!!!!!!!!AFTER TURN PROCESSING POP GROWTH PRODCUTION RESEARCH";
    Logger().debugStream() << objects.Dump();



    // update current turn number so that following visibility updates and info
    // sent to players will have updated turn associated with them
    ++m_current_turn;


    // new turn visibility update
    m_universe.UpdateEmpireObjectVisibilities();
    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();



    CheckForEmpireEliminationOrVictory();



    // indicate that the clients are waiting for their new gamestate
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin();
         player_it != m_networking.established_end();
         ++player_it)
    {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->PlayerID(), Message::DOWNLOADING));
    }


    // compile map of PlayerInfo, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin();
         player_it != m_networking.established_end();
         ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        int player_id = player->PlayerID();
        players[player_id] = PlayerInfo(player->PlayerName(),
                                        PlayerEmpireID(player_id),
                                        player->GetClientType(),
                                        m_networking.PlayerIsHost(player_id));
    }

    // send new-turn updates to all players
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin();
         player_it != m_networking.established_end();
         ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        int player_id = player->PlayerID();
        player->SendMessage(TurnUpdateMessage(player_id,
                                              PlayerEmpireID(player_id),
                                              m_current_turn,
                                              m_empires,
                                              m_universe,
                                              GetSpeciesManager(),
                                              players));
    }
}

void ServerApp::CheckForEmpireEliminationOrVictory()
{
    //EmpireManager& empires = Empires();
    //ObjectMap& objects = m_universe.Objects();

    //// check for eliminated empires and players
    //std::map<int, int> eliminations; // map from player id to empire id of eliminated players, for empires eliminated this turn
    //for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it) {
    //    int empire_id = it->first;
    //    if (empires.Eliminated(empire_id))
    //        continue;   // don't double-eliminate an empire
    //    Logger().debugStream() << "empire " << empire_id << " not yet eliminated";

    //    const Empire* empire = it->second;
    //    if (!EmpireEliminated(empire, m_universe))
    //        continue;
    //    Logger().debugStream() << " ... but IS eliminated this turn";

    //    int elim_player_id = EmpirePlayerID(empire_id);
    //    eliminations[elim_player_id] = empire_id;
    //}


    //// check for victorious players
    //std::map<int, std::set<std::string> > new_victors; // map from player ID to set of victory reason strings

    //// marked by Victory effect?
    //const std::multimap<int, std::string>& marked_for_victory = m_universe.GetMarkedForVictory();
    //for (std::multimap<int, std::string>::const_iterator it = marked_for_victory.begin(); it != marked_for_victory.end(); ++it) {
    //    const UniverseObject* obj = objects.Object(it->first);
    //    if (!obj || obj->Unowned()) continue; // perhaps it was destroyed?
    //    int empire_id = obj->Owner();
    //    if (empires.Lookup(empire_id))
    //        new_victors[EmpirePlayerID(empire_id)].insert(it->second);
    //}

    //// all enemies eliminated?
    //if (eliminations.size() == m_networking.NumEstablishedPlayers() - 1) {
    //    // only one player not eliminated.  treat this as a win for the remaining player
    //    ServerNetworking::established_iterator player_it = m_networking.established_begin();
    //    if (player_it != m_networking.established_end()) {
    //        boost::shared_ptr<PlayerConnection> player_connection = *player_it;
    //        int cur_player_id = player_connection->PlayerID();
    //        if (eliminations.find(cur_player_id) == eliminations.end())
    //            new_victors[cur_player_id].insert("ALL_ENEMIES_ELIMINATED_VICTORY");
    //    }
    //}


    //// check if any victors are new.  (don't want to re-announce old victors each subsequent turn)
    //if (!new_victors.empty()) {
    //    for (std::map<int, std::set<std::string> >::const_iterator it = new_victors.begin(); it != new_victors.end(); ++it) {
    //        int victor_player_id = it->first;

    //        const std::set<std::string>& reasons = it->second;
    //        for (std::set<std::string>::const_iterator reason_it = reasons.begin(); reason_it != reasons.end(); ++reason_it) {
    //            std::string reason_string = *reason_it;

    //            // see if player has already won the game...
    //            bool new_victory = false;
    //            std::map<int, std::set<std::string> >::const_iterator vict_it = m_victors.find(victor_player_id);
    //            if (vict_it == m_victors.end()) {
    //                // player hasn't yet won, so victory is new
    //                new_victory = true;
    //            } else {
    //                // player has won at least once, but also need to check of the type of victory is new
    //                std::set<std::string>::const_iterator vict_type_it = vict_it->second.find(reason_string);
    //                if (vict_type_it == vict_it->second.end())
    //                    new_victory = true;
    //            }

    //            if (new_victory) {
    //                // record victory
    //                m_victors[victor_player_id].insert(reason_string);

    //                Empire* empire = GetPlayerEmpire(victor_player_id);
    //                if (!empire) {
    //                    Logger().errorStream() << "Trying to grant victory to a missing empire!";
    //                    continue;
    //                }
    //                const std::string& victor_empire_name = empire->Name();
    //                int victor_empire_id = empire->EmpireID();


    //                // notify all players of victory
    //                for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
    //                    boost::shared_ptr<PlayerConnection> player_connection = *player_it;
    //                    int recipient_player_id = player_connection->PlayerID();
    //                    player_connection->SendMessage(VictoryDefeatMessage(recipient_player_id, Message::VICTORY, reason_string, victor_empire_id));
    //                    if (Empire* recipient_empire = GetPlayerEmpire(recipient_player_id))
    //                        recipient_empire->AddSitRepEntry(CreateVictorySitRep(reason_string, victor_empire_name));
    //                }
    //            }
    //        }
    //    }
    //}


    //if (eliminations.empty())
    //    return;


    //Sleep(1000); // time for elimination messages to propegate


    //// inform all players of eliminations
    //for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
    //    int elim_player_id = it->first;
    //    int elim_empire_id = it->second;
    //    Empire* empire = empires.Lookup(elim_empire_id);
    //    if (!empire) {
    //        Logger().errorStream() << "Trying to eliminate a missing empire!";
    //        continue;
    //    }
    //    const std::string& elim_empire_name = empire->Name();

    //    // notify all players of disconnection, and end game of eliminated player
    //    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
    //        boost::shared_ptr<PlayerConnection> player_connection = *player_it;
    //        int recipient_player_id = player_connection->PlayerID();
    //        if (recipient_player_id == elim_player_id) {
    //            player_connection->SendMessage(EndGameMessage(recipient_player_id, Message::YOU_ARE_ELIMINATED));
    //            m_ai_client_processes.erase(recipient_player_id);   // done now so that PlayerConnection doesn't need to be re-retreived when dumping connections
    //        } else {
    //            player_connection->SendMessage(PlayerEliminatedMessage(recipient_player_id, elim_empire_id, elim_empire_name));    // PlayerEliminatedMessage takes the eliminated empire id, not the eliminated player id, for unknown reasons, as of this writing
    //            if (Empire* recipient_empire = GetPlayerEmpire(recipient_player_id))
    //                recipient_empire->AddSitRepEntry(CreateEmpireEliminatedSitRep(elim_empire_id));
    //        }
    //    }
    //}

    //// dump connections to eliminated players, and remove server-side empire data
    //for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
    //    int elim_empire_id = it->second;
    //    // remove eliminated empire's ownership of UniverseObjects
    //    std::vector<UniverseObject*> object_vec = objects.FindObjects(OwnedVisitor<UniverseObject>(elim_empire_id));
    //    for (std::vector<UniverseObject*>::iterator obj_it = object_vec.begin(); obj_it != object_vec.end(); ++obj_it)
    //        (*obj_it)->SetOwner(ALL_EMPIRES);

    //    Logger().debugStream() << "ServerApp::ProcessTurns : Player " << it->first << " is eliminated and dumped";
    //    m_eliminated_players.insert(it->first);
    //    m_networking.Disconnect(it->first);

    //    empires.EliminateEmpire(it->second);
    //    RemoveEmpireTurn(it->second);
    //}
}

void ServerApp::AddEmpireCombatTurn(int empire_id)
{ m_combat_turn_sequence[empire_id] = 0; }

void ServerApp::ClearEmpireCombatTurns()
{
    ClearEmpireCombatTurnOrders();
    m_combat_turn_sequence.clear();
}

void ServerApp::SetEmpireCombatTurnOrders(int empire_id, CombatOrderSet* order_set)
{ m_combat_turn_sequence[empire_id] = order_set; }

void ServerApp::ClearEmpireCombatTurnOrders()
{
    for (std::map<int, CombatOrderSet*>::iterator it = m_combat_turn_sequence.begin();
         it != m_combat_turn_sequence.end();
         ++it) {
        delete it->second;
        it->second = 0;
    }
}

bool ServerApp::AllCombatOrdersReceived()
{
    for (std::map<int, CombatOrderSet*>::iterator it = m_combat_turn_sequence.begin();
         it != m_combat_turn_sequence.end();
         ++it) {
        if (!it->second)
            return false;
    }
    return true;
}

void ServerApp::ProcessCombatTurn()
{
    PathingEngine& pathing_engine = m_current_combat->m_pathing_engine;

    // apply combat orders
    for (std::map<int, CombatOrderSet*>::iterator it = m_combat_turn_sequence.begin();
         it != m_combat_turn_sequence.end();
         ++it) {
        for (std::size_t i = 0; i < it->second->size(); ++i) {
            const CombatOrder& order = (*it->second)[i];
            switch (order.Type()) {
            case CombatOrder::SHIP_ORDER: {
                CombatShipPtr combat_ship = pathing_engine.FindShip(order.ID());
                if (order.Append())
                    combat_ship->ClearMissions();
                combat_ship->AppendMission(order.GetShipMission());
                break;
            }
            case CombatOrder::FIGHTER_ORDER: {
                CombatFighterPtr combat_fighter = pathing_engine.FindLeader(order.ID());
                if (order.Append())
                    combat_fighter->ClearMissions();
                combat_fighter->AppendMission(order.GetFighterMission());
                break;
            }
            case CombatOrder::SETUP_PLACEMENT_ORDER: {
                CombatShipPtr combat_ship = pathing_engine.FindShip(order.ID());
                assert(combat_ship);
                combat_ship->setPosition(order.GetPositionAndDirection().first);
                combat_ship->regenerateOrthonormalBasis(-order.GetPositionAndDirection().first,
                                                        OpenSteer::Vec3(0.0, 0.0, 1.0));
                break;
            }
            }
        }
    }

    // clear applied combat orders
    ClearEmpireCombatTurnOrders();

    // process combat turn
    if (m_current_combat->m_combat_turn_number) {
        pathing_engine.TurnStarted(m_current_combat->m_combat_turn_number);
        const std::size_t MIN_ITERATIONS = 60;
        const std::size_t ITERATIONS =
            std::max(MIN_ITERATIONS,
                     PathingEngine::SECONDS_PER_TURN * PathingEngine::TARGET_OBJECT_UPDATES_PER_SEC);
        const double ITERATION_DURATION = 1.0 * PathingEngine::SECONDS_PER_TURN / ITERATIONS;
        for (std::size_t i = 0; i < ITERATIONS; ++i) {
            pathing_engine.Update(ITERATION_DURATION, true);
        }
    }

    // increment combat turn number
    ++m_current_combat->m_combat_turn_number;
}

bool ServerApp::CombatTerminated()
{
    // TODO
    return false;
}
