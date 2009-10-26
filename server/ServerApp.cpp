#include "ServerApp.h"

#include "SaveLoad.h"
#include "../combat/CombatSystem.h"
#include "../network/Message.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Special.h"
#include "../universe/System.h"
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

namespace {
    const bool TEST_3D_COMBAT = false;

    void PopulateCombatUniverse(const System& system, std::map<int, UniverseObject*>& combat_universe)
    {
        for (System::const_orbit_iterator it = system.begin(); it != system.end(); ++it) {
            int object_id = it->second;
            combat_universe[object_id] = GetUniverse().Object(object_id);
        }
    }
}


////////////////////////////////////////////////
// PlayerSaveGameData
////////////////////////////////////////////////
PlayerSaveGameData::PlayerSaveGameData() :
    m_empire(0)
{}

PlayerSaveGameData::PlayerSaveGameData(const std::string& name, Empire* empire, const boost::shared_ptr<OrderSet>& orders, const boost::shared_ptr<SaveGameUIData>& ui_data, const std::string& save_state_string) :
    m_name(name),
    m_empire(empire),
    m_orders(orders),
    m_ui_data(ui_data),
    m_save_state_string(save_state_string)
{}

////////////////////////////////////////////////
// ServerSaveGameData
////////////////////////////////////////////////
ServerSaveGameData::ServerSaveGameData() :
    m_current_turn(-1)
{}

ServerSaveGameData::ServerSaveGameData(const int& current_turn, const std::map<int, std::set<std::string> >& victors) :
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
    m_log_category(log4cpp::Category::getRoot()),
    m_fsm(*this),
    m_current_turn(INVALID_GAME_TURN),
    m_single_player_game(false)
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of singleton class ServerApp");

    s_app = this;

    const std::string SERVER_LOG_FILENAME((GetUserDir() / "freeoriond.log").file_string());

    // a platform-independent way to erase the old log
    std::ofstream temp(SERVER_LOG_FILENAME.c_str());
    temp.close();

    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", SERVER_LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p Server : %m%n");
    appender->setLayout(layout);
    m_log_category.setAdditivity(false);  // make appender the only appender used...
    m_log_category.setAppender(appender);
    m_log_category.setAdditivity(true);   // ...but allow the addition of others later
    m_log_category.setPriority(log4cpp::Priority::DEBUG);

    m_fsm.initiate();
}

ServerApp::~ServerApp()
{ log4cpp::Category::shutdown(); }

void ServerApp::operator()()
{ Run(); }

void ServerApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    exit(code);
}

log4cpp::Category& ServerApp::Logger()
{ return m_log_category; }

void ServerApp::CreateAIClients(const std::vector<PlayerSetupData>& AIs, std::set<std::string>& expected_ai_player_names)
{
    for (std::set<int>::iterator it = m_ai_IDs.begin(); it != m_ai_IDs.end(); ++it) {
        m_networking.Disconnect(*it);
    }
    m_ai_clients.clear();
    m_ai_IDs.clear();

    int ai_client_base_number = 1;
    int i = 0;

#ifdef FREEORION_WIN32
    const std::string AI_CLIENT_EXE = (GetBinDir() / "freeorionca.exe").file_string();
#else
    const std::string AI_CLIENT_EXE = (GetBinDir() / "freeorionca").file_string();
#endif
    for (std::vector<PlayerSetupData>::const_iterator it = AIs.begin(); it != AIs.end(); ++it, ++i) {
        // TODO: add other command line args to AI client invocation as needed
        std::string player_name = "AI_" + boost::lexical_cast<std::string>(ai_client_base_number + i);
        expected_ai_player_names.insert(player_name);
        std::vector<std::string> args;
        args.push_back(AI_CLIENT_EXE);
        args.push_back(player_name);
        args.push_back("--resource-dir");
        args.push_back("\"" + GetOptionsDB().Get<std::string>("resource-dir") + "\"");
        args.push_back("--log-level");
        args.push_back(GetOptionsDB().Get<std::string>("log-level"));
        Logger().debugStream() << "starting " << AI_CLIENT_EXE;
        m_ai_clients[player_name] = Process(AI_CLIENT_EXE, args);
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
    try {
        while (1) {
            if (m_io_service.run_one())
                m_networking.HandleNextEvent();
            else
                break;
        }
    } catch (...) {
        CleanupAIs();
        throw;
    }
    CleanupAIs();
}

void ServerApp::CleanupAIs()
{
    for (std::map<std::string, Process>::iterator it = m_ai_clients.begin(); it != m_ai_clients.end(); ++it) {
        it->second.Kill();
    }
}

void ServerApp::HandleMessage(Message msg, PlayerConnectionPtr player_connection)
{
    if (msg.SendingPlayer() != player_connection->ID()) {
        m_log_category.errorStream() << "ServerApp::HandleMessage : Received an message with a sender ID that differs "
            "from the sending player's ID.  Terminating connection.";
        m_networking.Disconnect(player_connection);
        return;
    }

    switch (msg.Type()) {
    case Message::HOST_SP_GAME:          m_fsm.process_event(HostSPGame(msg, player_connection)); break;
    case Message::START_MP_GAME:         m_fsm.process_event(StartMPGame(msg, player_connection)); break;
    case Message::LOBBY_UPDATE:          m_fsm.process_event(LobbyUpdate(msg, player_connection)); break;
    case Message::LOBBY_CHAT:            m_fsm.process_event(LobbyChat(msg, player_connection)); break;
    case Message::LOBBY_HOST_ABORT:      m_fsm.process_event(LobbyHostAbort(msg, player_connection)); break;
    case Message::LOBBY_EXIT:            m_fsm.process_event(LobbyNonHostExit(msg, player_connection)); break;
    case Message::SAVE_GAME:             m_fsm.process_event(SaveGameRequest(msg, player_connection)); break;
    case Message::TURN_ORDERS:           m_fsm.process_event(TurnOrders(msg, player_connection)); break;
    case Message::COMBAT_TURN_ORDERS:    m_fsm.process_event(CombatTurnOrders(msg, player_connection)); break;
    case Message::CLIENT_SAVE_DATA:      m_fsm.process_event(ClientSaveData(msg, player_connection)); break;
    case Message::HUMAN_PLAYER_CHAT:     m_fsm.process_event(PlayerChat(msg, player_connection)); break;
    case Message::REQUEST_NEW_OBJECT_ID: m_fsm.process_event(RequestObjectID(msg, player_connection)); break;
    case Message::REQUEST_NEW_DESIGN_ID: m_fsm.process_event(RequestDesignID(msg, player_connection)); break;

    // TODO: For prototyping only.
    case Message::COMBAT_END:            m_fsm.process_event(CombatComplete()); break;

#ifndef FREEORION_RELEASE
    case Message::DEBUG:                 break;
#endif
    default:
        m_log_category.errorStream() << "ServerApp::HandleMessage : Received an unknown message type \""
                                     << msg.Type() << "\".  Terminating connection.";
        m_networking.Disconnect(player_connection);
        break;
    }
}

void ServerApp::HandleNonPlayerMessage(Message msg, PlayerConnectionPtr player_connection)
{
    switch (msg.Type()) {
    case Message::HOST_SP_GAME: m_fsm.process_event(HostSPGame(msg, player_connection)); break;
    case Message::HOST_MP_GAME: m_fsm.process_event(HostMPGame(msg, player_connection)); break;
    case Message::JOIN_GAME:    m_fsm.process_event(JoinGame(msg, player_connection)); break;
#ifndef FREEORION_RELEASE
    case Message::DEBUG:                 break;
#endif
    default:
        m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : Received an invalid message type \""
                                     << msg.Type() << "\" for a non-player Message.  Terminating connection.";
        m_networking.Disconnect(player_connection);
        break;
    }
}

void ServerApp::PlayerDisconnected(PlayerConnectionPtr player_connection)
{ m_fsm.process_event(Disconnection(player_connection)); }

void ServerApp::NewGameInit(boost::shared_ptr<SinglePlayerSetupData> setup_data)
{
    std::map<int, PlayerSetupData> player_setup_data;
    PlayerSetupData& data = player_setup_data[Networking::HOST_PLAYER_ID];
    data.m_player_id = Networking::HOST_PLAYER_ID;
    data.m_player_name = setup_data->m_host_player_name;
    data.m_empire_name = setup_data->m_empire_name;
    data.m_empire_color = setup_data->m_empire_color;
    NewGameInit(setup_data->m_size, setup_data->m_shape, setup_data->m_age, setup_data->m_starlane_freq, setup_data->m_planet_density, setup_data->m_specials_freq, player_setup_data);
}

void ServerApp::LoadGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data, boost::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    std::set<int> unused_save_game_data;
    std::map<int, int> player_id_to_save_game_data_index;
    for (unsigned int i = 0; i < player_save_game_data.size(); ++i) {
        assert(player_save_game_data[i].m_empire);
        // This works because the empire ID is always the same as the player ID in single player games, at least for the
        // host (human) player.
        if (player_save_game_data[i].m_empire->EmpireID() == Networking::HOST_PLAYER_ID)
            player_id_to_save_game_data_index[Networking::HOST_PLAYER_ID] = i;
        else
            unused_save_game_data.insert(i);
    }
    LoadGameInit(player_save_game_data, player_id_to_save_game_data_index, unused_save_game_data, server_save_game_data);
}

void ServerApp::NewGameInit(boost::shared_ptr<MultiplayerLobbyData> lobby_data)
{
    NewGameInit(lobby_data->m_size, lobby_data->m_shape, lobby_data->m_age, lobby_data->m_starlane_freq, lobby_data->m_planet_density, lobby_data->m_specials_freq, lobby_data->m_players);
}

void ServerApp::LoadGameInit(boost::shared_ptr<MultiplayerLobbyData> lobby_data, const std::vector<PlayerSaveGameData>& player_save_game_data, boost::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    // multiplayer load
    std::set<int> used_save_game_data;
    std::map<int, int> player_id_to_save_game_data_index;
    for (std::map<int, PlayerSetupData>::const_iterator it = lobby_data->m_players.begin(); it != lobby_data->m_players.end(); ++it) {
        int save_game_empire_id = it->second.m_save_game_empire_id;
        for (unsigned int i = 0; i < player_save_game_data.size(); ++i) {
            assert(player_save_game_data[i].m_empire);
            if (player_save_game_data[i].m_empire->EmpireID() == save_game_empire_id) {
                player_id_to_save_game_data_index[it->first] = i;
                used_save_game_data.insert(i);
            }
        }
    }

    std::set<int> unused_save_game_data;
    for (unsigned int i = 0; i < player_save_game_data.size(); ++i) {
        if (used_save_game_data.find(i) == used_save_game_data.end())
            unused_save_game_data.insert(i);
    }

    LoadGameInit(player_save_game_data, player_id_to_save_game_data_index, unused_save_game_data, server_save_game_data);
}

void ServerApp::NewGameInit(int size, Shape shape, Age age, StarlaneFrequency starlane_freq, PlanetDensity planet_density, SpecialsFrequency specials_freq,
                            const std::map<int, PlayerSetupData>& player_setup_data)
{
    Logger().debugStream() << "ServerApp::NewGameInit";
    m_turn_sequence.clear();

    m_victors.clear();
    m_eliminated_players.clear();

    m_current_turn = BEFORE_FIRST_TURN;     // every UniverseObject created before game starts will have m_created_on_turn BEFORE_FIRST_TURN
    m_universe.CreateUniverse(size, shape, age, starlane_freq, planet_density, specials_freq,
                              m_networking.NumPlayers() - m_ai_clients.size(), m_ai_clients.size(), player_setup_data);
    m_current_turn = 1;                     // after all game initialization stuff has been created, can set current turn to 1 for start of game

    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();

    // Determine initial supply distribution and exchanging and resource pools for empires
    EmpireManager& empires = Empires();
    for (EmpireManager::iterator it = empires.begin(); it != empires.end(); ++it) {
        Empire* empire = it->second;

        empire->UpdateSupplyUnobstructedSystems();  // determines which systems can propegate fleet and resource (same for both)
        empire->UpdateSystemSupplyRanges();         // sets range systems can propegate fleet and resourse supply (separately)
        empire->UpdateFleetSupply();                // determines which systems can access fleet supply, and starlane traversals used to do this
        empire->UpdateResourceSupply();             // determines the separate groups of systems within which (but not between which) resources can be shared
        empire->InitResourcePools();                // determines population centers and resource centers of empire, tells resource pools the centers and groups of systems that can share resources (note that being able to share resources doesn't mean a system produces resources)
        empire->UpdateResourcePools();              // determines how much of each resources is available in each resource sharing group
    }

    Logger().debugStream() << "Universe Created.  Adding empires to turn processing list";

    std::vector<PlayerConnectionPtr> shuffled_players;
    std::copy(m_networking.established_begin(), m_networking.established_end(), std::back_inserter(shuffled_players));
    std::random_shuffle(shuffled_players.begin(), shuffled_players.end());
    for (std::vector<PlayerConnectionPtr>::const_iterator it = shuffled_players.begin(); it != shuffled_players.end(); ++it) {
        AddEmpireTurn((*it)->ID());
    }

    // compile map of PlayerInfo for each player, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        players[(*it)->ID()] = PlayerInfo((*it)->PlayerName(),
                                          GetPlayerEmpire((*it)->ID())->EmpireID(),
                                          m_ai_IDs.find((*it)->ID()) != m_ai_IDs.end(),
                                          (*it)->Host());
    }

    Logger().debugStream() << "Sending GameStartMessages to players";

    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        int player_id = (*it)->ID();
        int empire_id = GetPlayerEmpire(player_id)->EmpireID();
        (*it)->SendMessage(GameStartMessage(player_id, m_single_player_game, empire_id, m_current_turn, m_empires, m_universe, players));
    }
}

void ServerApp::LoadGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data,
                             const std::map<int, int>& player_id_to_save_game_data_index,
                             std::set<int>& unused_save_game_data, boost::shared_ptr<ServerSaveGameData> server_save_game_data)
{
    assert(!player_save_game_data.empty());

    m_turn_sequence.clear();

    m_victors = server_save_game_data->m_victors;
    m_eliminated_players.clear();

    m_current_turn = server_save_game_data->m_current_turn;

    assert(m_networking.NumPlayers() == player_save_game_data.size());
    assert(player_id_to_save_game_data_index.size() + unused_save_game_data.size() == player_save_game_data.size());

    std::map<Empire*, const PlayerSaveGameData*> player_data_by_empire;
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        int save_game_data_index = -1;
        std::map<int, int>::const_iterator id_to_index_it = player_id_to_save_game_data_index.find((*it)->ID());
        if (id_to_index_it == player_id_to_save_game_data_index.end()) {
            save_game_data_index = *unused_save_game_data.begin();
            unused_save_game_data.erase(save_game_data_index);
        } else {
            save_game_data_index = id_to_index_it->second;
        }
        const PlayerSaveGameData& save_game_data = player_save_game_data[save_game_data_index];
        save_game_data.m_empire->SetPlayerName((*it)->PlayerName());
        Empires().InsertEmpire(save_game_data.m_empire);
        AddEmpireTurn(save_game_data.m_empire->EmpireID());
        player_data_by_empire[save_game_data.m_empire] = &save_game_data;
    }

    // This is a bit odd, but since Empires() is built from the data stored in player_save_game_data, and the universe
    // is loaded long before that, the universe's empire-specific views of the systems is not properly initialized when
    // the universe is loaded.  That means we must do it here.
    m_universe.RebuildEmpireViewSystemGraphs();


    // restore in-turn state data potentially lost during serialization..
    m_universe.ApplyMeterEffectsAndUpdateMeters();
    for (EmpireManager::iterator it = Empires().begin(); it != Empires().end(); ++it) {
        Empire* empire = it->second;

        empire->UpdateSupplyUnobstructedSystems();  // determines which systems can propegate fleet and resource (same for both)
        empire->UpdateSystemSupplyRanges();         // sets range systems can propegate fleet and resourse supply (separately)
        empire->UpdateFleetSupply();                // determines which systems can access fleet supply, and starlane traversals used to do this
        empire->UpdateResourceSupply();             // determines the separate groups of systems within which (but not between which) resources can be shared
        empire->InitResourcePools();                // determines population centers and resource centers of empire, tells resource pools the centers and groups of systems that can share resources (note that being able to share resources doesn't mean a system produces resources)
        empire->UpdateResourcePools();              // determines how much of each resources is available in each resource sharing group
    }


    // compile map of PlayerInfo for each player, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        // extract connection struct and info from it
        boost::shared_ptr<PlayerConnection> player_connection = *it;
        std::string player_name =           player_connection->PlayerName();
        int player_id =                     player_connection->ID();
        const Empire* player_empire =       GetPlayerEmpire(player_id);
        int player_empire_id =              player_empire->EmpireID();
        bool player_is_host =               player_connection->Host();
        bool player_is_AI =                 m_ai_IDs.find(player_id) != m_ai_IDs.end();
        // store in PlayerInfo struct
        players[player_id] = PlayerInfo(player_name, player_empire_id, player_is_AI, player_is_host);
    }

    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        // extract info needed for GameStartMessage for this player
        int player_id =                             (*it)->ID();
        Empire* empire =                            GetPlayerEmpire(player_id);
        int empire_id =                             empire->EmpireID();
        bool player_is_AI =                         players[player_id].AI;
        boost::shared_ptr<OrderSet> orders =        player_data_by_empire[empire]->m_orders;
        boost::shared_ptr<SaveGameUIData> ui_data = player_data_by_empire[empire]->m_ui_data;
        std::string save_state_string =             player_data_by_empire[empire]->m_save_state_string;
        // send load game messages to human and AI players.  AIs might have something useful in save_state_string,
        // but clients are assumed to only use the ui_data struct
        if (player_is_AI) {
            std::string* sss = 0;   // I first tried making PlayerSaveGameData::m_save_string_data a boost::shared_ptr<std::string>, but the compiler didn't like this, and refused to allow the struct to be serialized.  Consequently, the raw data is now contained in the struct, and this test to see if there is anything in the string is done instead of the more elegant system used to check of ui_data is available.
            if (!save_state_string.empty())
                sss = &save_state_string;
            (*it)->SendMessage(GameStartMessage(player_id, m_single_player_game, empire_id, m_current_turn, m_empires, m_universe,
                                                players, *orders, sss));
        } else {
            (*it)->SendMessage(GameStartMessage(player_id, m_single_player_game, empire_id, m_current_turn, m_empires, m_universe,
                                                players, *orders, ui_data.get()));
        }
    }
}

Empire* ServerApp::GetPlayerEmpire(int player_id) const
{
    Empire* retval = 0;
    ServerNetworking::const_established_iterator player_it = m_networking.GetPlayer(player_id);
    if (player_it != m_networking.established_end()) {
        std::string player_name = (*player_it)->PlayerName();
        for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
            if (it->second->PlayerName() == player_name) {
                retval = it->second;
                break;
            }
        }
    }
    return retval;
}

int ServerApp::GetEmpirePlayerID(int empire_id) const
{
    int retval = -1;
    std::string player_name = Empires().Lookup(empire_id)->PlayerName();
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        if ((*it)->PlayerName() == player_name) {
            retval = (*it)->ID();
            break;
        }
    }
    return retval;
}

void ServerApp::AddEmpireTurn(int empire_id)
{
    m_turn_sequence[empire_id] = 0;
}

void ServerApp::RemoveEmpireTurn(int empire_id)
{
    m_turn_sequence.erase(empire_id);
}

void ServerApp::SetEmpireTurnOrders(int empire_id, OrderSet *order_set)
{
    m_turn_sequence[empire_id] = order_set;
}

bool ServerApp::AllOrdersReceived()
{
    // Loop through to find empire ID and check for valid orders pointer
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        if (!it->second)
            return false;
    } 
    return true;
}

namespace {
    /** returns true if \a empire has been eliminated by the applicable
      * definition of elimination.  As of this writing, elimination means
      * having no ships and no fleets. */
    bool EmpireEliminated(const Empire* empire, const Universe& universe) {
        if (!empire)
            return false;
        int empire_id = empire->EmpireID();
        return (universe.FindObjects(OwnedVisitor<Planet>(empire_id)).empty() &&    // no planets
                universe.FindObjects(OwnedVisitor<Fleet>(empire_id)).empty());      // no fleets
    }
}

void ServerApp::ProcessTurns()
{
    EmpireManager& empires = Empires();


    // Now all orders, then process turns
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        // broadcast UI message to all players
        for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
            (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::PROCESSING_ORDERS, it->first));
        }

        Empire* empire = empires.Lookup(it->first);
        empire->ClearSitRep();
        OrderSet* order_set = it->second;

        // execute order set
        for (OrderSet::const_iterator order_it = order_set->begin(); order_it != order_set->end(); ++order_it) {
            order_it->second->Execute();
        }
    }

    // filter FleetColonizeOrder for later processing
    typedef std::map<int, std::vector<boost::shared_ptr<FleetColonizeOrder> > > ColonizeOrderMap;
    ColonizeOrderMap colonize_order_map;
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        OrderSet* order_set = it->second;

        // filter FleetColonizeOrder and sort them per planet
        boost::shared_ptr<FleetColonizeOrder> order;
        for (OrderSet::const_iterator order_it = order_set->begin(); order_it != order_set->end(); ++order_it) {
            if ((order = boost::dynamic_pointer_cast<FleetColonizeOrder>(order_it->second))) {
                ColonizeOrderMap::iterator it = colonize_order_map.find(order->PlanetID());
                if (it == colonize_order_map.end()) {
                    colonize_order_map.insert(std::make_pair(order->PlanetID(),std::vector<boost::shared_ptr<FleetColonizeOrder> >()));
                    it = colonize_order_map.find(order->PlanetID());
                }
                it->second.push_back(order);
            }
        }
    }


    // colonization apply be the following rules
    // 1 - if there is only own empire which tries to colonize a planet, is allowed to do so
    // 2 - if there are more than one empire then
    // 2.a - if only one empire which tries to colonize (empire who don't are ignored) is armed, this empire wins the race
    // 2.b - if more than one empire is armed or all forces are unarmed, no one can colonize the planet
    for (ColonizeOrderMap::iterator it = colonize_order_map.begin(); it != colonize_order_map.end(); ++it) {
        Planet* planet = m_universe.Object<Planet>(it->first);

        // only one empire?
        if (it->second.size() == 1) {
            it->second[0]->ServerExecute();
            Empire* empire = empires.Lookup( it->second[0]->EmpireID() );
            empire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet->SystemID(), planet->ID()));
        } else {
            const System* system = m_universe.Object<System>(planet->SystemID());

            std::vector<const Fleet*> vec_fleet = system->FindObjects<Fleet>();
            std::set<int> set_empire_with_military;
            for (unsigned int i=0;i<vec_fleet.size();i++)
                for (Fleet::const_iterator ship_it=vec_fleet[i]->begin();ship_it!=vec_fleet[i]->end();++ship_it)
                    if (m_universe.Object<Ship>(*ship_it)->IsArmed()) {
                        set_empire_with_military.insert(*vec_fleet[i]->Owners().begin());
                        break;
                    }

            // set the first empire as winner for now
            int winner = 0;
            // is the current winner armed?
            bool winner_is_armed = set_empire_with_military.find(it->second[0]->EmpireID()) != set_empire_with_military.end();
            for (unsigned int i = 1; i < it->second.size(); i++)
                // is this empire armed?
                if (set_empire_with_military.find(it->second[i]->EmpireID()) != set_empire_with_military.end()) {
                    // if this empire is armed and the former winner too, noone can win
                    if (winner_is_armed) {
                        winner = -1; // no winner!!
                        break;       // won't find a winner!
                    }
                    winner = i; // this empire is the winner for now
                    winner_is_armed = true; // and has armed forces
                }
                else
                    // this empire isn't armed
                    if(!winner_is_armed)
                        winner = -1; // if the current winner isn't armed, a winner must be armed!!!!

            for (int i = 0; i < static_cast<int>(it->second.size()); i++)
                if (winner == i) {
                    it->second[i]->ServerExecute();
                    Empire* empire = empires.Lookup(it->second[i]->EmpireID());
                    empire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet->SystemID(), planet->ID()));
                }
                else
                    it->second[i]->Undo();
        }

        planet->ResetIsAboutToBeColonized();
    }


    // process movement phase
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::FLEET_MOVEMENT, -1));
    }


    // scrap orders
    std::vector<int> objects_to_scrap;
    for (Universe::iterator it = m_universe.begin(); it != m_universe.end(); ++it) {
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
    std::vector<Fleet*> fleets = m_universe.FindObjects<Fleet>();
    for (std::vector<Fleet*>::iterator it = fleets.begin(); it != fleets.end(); ++it) {
        if (Fleet* fleet = *it)
            if (fleet->Empty())
                m_universe.Destroy(fleet->ID());
    }


    // fleet movement
    for (Universe::const_iterator it = m_universe.begin(); it != m_universe.end(); ++it) {
        // save for possible SitRep generation after moving...
        const Fleet* fleet = m_universe.Object<Fleet>(it->first);
        int eta = -1;
        if (fleet)
            eta = fleet->ETA().first;

        // TODO: Do movement incrementally, and if the moving fleet encounters
        // stationary combat fleets or planetary defenses that can hurt it, it
        // must be resolved as a combat.
        it->second->MovementPhase();

        // SitRep for fleets having arrived at destinations, to all owners of those fleets
        if (fleet) {
            if (eta == 1) {
                std::set<int> owners_set = fleet->Owners();
                for (std::set<int>::const_iterator owners_it = owners_set.begin(); owners_it != owners_set.end(); ++owners_it) {
                    if (Empire* empire = empires.Lookup(*owners_it))
                        empire->AddSitRepEntry(CreateFleetArrivedAtDestinationSitRep(fleet->SystemID(), fleet->ID()));
                    else
                        Logger().errorStream() << "ServerApp::ProcessTurns couldn't find empire with id " << *owners_it << " to send a fleet arrival sitrep to for fleet " << fleet->ID();
                }
            }
        }
    }


    // post-movement visibility update
    m_universe.UpdateEmpireObjectVisibilities();


    // check for combats, and resolve them.
    for (ServerNetworking::const_established_iterator player_it =
             m_networking.established_begin();
         player_it != m_networking.established_end();
         ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::COMBAT, -1));
    }

    std::vector<System*> sys_vec = m_universe.FindObjects<System>();
    for (std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
        System* system = *it;
        if (TEST_3D_COMBAT) {
            bool combat_conditions_exist = false;
            std::vector<Fleet*> fleets = system->FindObjects<Fleet>();
            std::set<int> ids_of_empires_with_combat_fleets_here;
            std::set<int> ids_of_empires_with_fleets_here;
            for (std::vector<Fleet*>::iterator it = fleets.begin(); it != fleets.end(); ++it) {
                assert((*it)->Owners().size() == 1);
                int owner = *(*it)->Owners().begin();
                if ((*it)->HasArmedShips())
                    ids_of_empires_with_combat_fleets_here.insert(owner);
                ids_of_empires_with_fleets_here.insert(owner);
                if (2 <= ids_of_empires_with_combat_fleets_here.size() ||
                    ids_of_empires_with_combat_fleets_here.size() < ids_of_empires_with_fleets_here.size()) {
                    // TODO: Determine whether the empires are enemies.
                    combat_conditions_exist = true;
                    break;
                }
            }
            if (!combat_conditions_exist) {
                // TODO: Find base and planetary defenses that can harm the
                // fleets here.
            }
            if (!combat_conditions_exist) {
                // TODO: Find space monsters.
            }
            if (combat_conditions_exist
                // TODO: This second part of this condition is for prototyping only.
                && ids_of_empires_with_fleets_here.find(GetEmpirePlayerID(Networking::HOST_PLAYER_ID)) !=
                ids_of_empires_with_fleets_here.end()) {
                // TODO: Include the empire ID's of empires with base and
                // planetary defenses found above.
                m_fsm.process_event(ResolveCombat(system, ids_of_empires_with_fleets_here));
                while (m_current_combat) {
                    m_io_service.run_one();
                    m_networking.HandleNextEvent();
                }
            }
        } else {
            std::vector<CombatAssets> empire_combat_forces;

            std::vector<Fleet*> flt_vec = system->FindObjects<Fleet>();
            if (flt_vec.empty())
                continue;

            for (std::vector<Fleet*>::iterator flt_it = flt_vec.begin();
                 flt_it != flt_vec.end();
                 ++flt_it) {
                Fleet* flt = *flt_it;
                // a fleet should belong only to one empire!?
                if (1 == flt->Owners().size()) {
                    std::vector<CombatAssets>::iterator ecf_it =
                        std::find(empire_combat_forces.begin(), empire_combat_forces.end(),
                                  CombatAssetsOwner(empires.Lookup(*flt->Owners().begin())));

                    if (ecf_it == empire_combat_forces.end()) {
                        CombatAssets ca(empires.Lookup(*flt->Owners().begin()));
                        ca.fleets.push_back(flt);
                        empire_combat_forces.push_back(ca);
                    } else {
                        (*ecf_it).fleets.push_back(flt);
                    }
                }
            }
            std::vector<Planet*> plt_vec = system->FindObjects<Planet>();
            for (std::vector<Planet*>::iterator plt_it = plt_vec.begin();
                 plt_it != plt_vec.end();
                 ++plt_it) {
                Planet* plt = *plt_it;
                // a planet should belong only to one empire!?
                if (1 == plt->Owners().size()) {
                    std::vector<CombatAssets>::iterator ecf_it =
                        std::find(empire_combat_forces.begin(), empire_combat_forces.end(),
                                  CombatAssetsOwner(empires.Lookup(*plt->Owners().begin())));

                    if (ecf_it == empire_combat_forces.end()) {
                        CombatAssets ca(empires.Lookup(*plt->Owners().begin()));
                        ca.planets.push_back(plt);
                        empire_combat_forces.push_back(ca);
                    } else {
                        (*ecf_it).planets.push_back(plt);
                    }
                }
            }

            if (empire_combat_forces.size() > 1) {
                CombatSystem combat_system;
                combat_system.ResolveCombat(system->ID(), empire_combat_forces);
            }
        }
    }


    // post-combat visibility update
    m_universe.UpdateEmpireObjectVisibilities();


    // process production and growth phase

    // notify players that production and growth is being processed
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::EMPIRE_PRODUCTION, -1));
    }

    // execute all effects and update meters prior to production, research, etc.
    m_universe.ApplyAllEffectsAndUpdateMeters();



    // Determine how much of each resource is available, and determine how to distribute it to planets or on queues
    for (EmpireManager::iterator it = empires.begin(); it != empires.end(); ++it) {
        Empire* empire = it->second;

        empire->UpdateSupplyUnobstructedSystems();  // determines which systems can propegate fleet and resource (same for both)
        empire->UpdateSystemSupplyRanges();         // sets range systems can propegate fleet and resourse supply (separately)
        empire->UpdateFleetSupply();                // determines which systems can access fleet supply, and starlane traversals used to do this
        empire->UpdateResourceSupply();             // determines the separate groups of systems within which (but not between which) resources can be shared
        empire->InitResourcePools();                // determines population centers and resource centers of empire, tells resource pools the centers and groups of systems that can share resources (note that being able to share resources doesn't mean a system produces resources)
        empire->UpdateResourcePools();              // determines how much of each resources is available in each resource sharing group
    }


    // Consume distributed resources to planets and on queues, create new objects for completed production and
    // give techs to empires that have researched them
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        Empire* empire = empires.Lookup(it->first);
        empire->CheckResearchProgress();
        empire->CheckProductionProgress();
        empire->CheckTradeSocialProgress();
        empire->CheckGrowthFoodProgress();
    }


    // re-execute all meter-related effects after production, so that new
    // UniverseObjects created during production will have effects applied to
    // them this turn, allowing (for example) ships to have max fuel meters
    // greater than 0 on the turn they are created.
    m_universe.ApplyMeterEffectsAndUpdateMeters();


    // post-production and meter-effects visibility update
    m_universe.UpdateEmpireObjectVisibilities();


    // regenerate empire system graphs based on latest visibility information.
    // this is needed for some UniverseObject subclasses'
    // PopGrowthProductionResearchPhase()
    m_universe.RebuildEmpireViewSystemGraphs();


    // Population growth or loss, health meter growth, resource current meter
    // growth
    for (Universe::const_iterator it = m_universe.begin(); it != m_universe.end(); ++it) {
        it->second->PopGrowthProductionResearchPhase();
        it->second->ClampMeters();  // ensures growth doesn't leave meters over MAX.  should otherwise be redundant with ClampMeters() in Universe::ApplyMeterEffectsAndUpdateMeters()
    }


    // post-growth visibility update
    m_universe.UpdateEmpireObjectVisibilities();


    // copy latest updated current meter values to initial current values, and
    // initial current values to previous values, so that clients will have
    // this information based on values after all changes that occured this turn
    for (Universe::const_iterator it = m_universe.begin(); it != m_universe.end(); ++it) {
        for (MeterType i = MeterType(0); i != NUM_METER_TYPES; i = MeterType(i + 1))
            if (Meter* meter = it->second->GetMeter(i))
                meter->BackPropegate();
    }


    // create sitreps for starved planets
    std::vector<Planet*> plt_vec = m_universe.FindObjects<Planet>();
    for (std::vector<Planet*>::iterator it = plt_vec.begin(); it!=plt_vec.end(); ++it) {
        if ((*it)->Owners().size() > 0 && (*it)->GetMeter(METER_POPULATION)->Current() <= 0.0) {
            // add some information to sitrep
            Empire* empire = empires.Lookup(*(*it)->Owners().begin());
            empire->AddSitRepEntry(CreatePlanetStarvedToDeathSitRep((*it)->SystemID(), (*it)->ID()));
            (*it)->Reset();
        }
    }


    // loop and free all orders
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        delete it->second;
        it->second = 0;
    }


    ++m_current_turn;


    m_universe.UpdateEmpireLatestKnownObjectsAndVisibilityTurns();


    // compile map of PlayerInfo for each player, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        players[(*it)->ID()] = PlayerInfo((*it)->PlayerName(),
                                          GetPlayerEmpire((*it)->ID())->EmpireID(),
                                          m_ai_IDs.find((*it)->ID()) != m_ai_IDs.end(),
                                          (*it)->Host());
    }


    // check for eliminated empires and players
    std::map<int, int> eliminations; // map from player id to empire id of eliminated players
    for (EmpireManager::const_iterator it = empires.begin(); it != empires.end(); ++it) {
        Empire* empire = it->second;
        if (!EmpireEliminated(empire, m_universe))
            continue;

        int elim_empire_id = it->first;
        int elim_player_id = GetEmpirePlayerID(elim_empire_id);
        eliminations[elim_player_id] = elim_empire_id;
    }


    // check for victorious players
    std::map<int, std::set<std::string> > new_victors; // map from player ID to set of victory reason strings

    // marked by Victory effect?
    const std::multimap<int, std::string>& marked_for_victory = m_universe.GetMarkedForVictory();
    for (std::multimap<int, std::string>::const_iterator it = marked_for_victory.begin(); it != marked_for_victory.end(); ++it) {
        const UniverseObject* obj = m_universe.Object(it->first);
        if (!obj) continue; // perhaps it was destroyed?
        const std::set<int>& owners = obj->Owners();
        if (owners.size() == 1) {
            int empire_id = *owners.begin();
            if (empires.Lookup(empire_id))
                new_victors[GetEmpirePlayerID(empire_id)].insert(it->second);
        }
    }

    // all enemies eliminated?
    if (eliminations.size() == m_networking.NumPlayers() - 1) {
        // only one player not eliminated.  treat this as a win for the remaining player
        ServerNetworking::established_iterator player_it = m_networking.established_begin();
        if (player_it != m_networking.established_end()) {
            boost::shared_ptr<PlayerConnection> pc = *player_it;
            int cur_player_id = pc->ID();
            if (eliminations.find(cur_player_id) == eliminations.end())
                new_victors[cur_player_id].insert("ALL_ENEMIES_ELIMINATED_VICTORY");
        }
    }


    // check if any victors are new.  (don't want to re-announce old victors each subsequent turn)
    if (!new_victors.empty()) {
        for (std::map<int, std::set<std::string> >::const_iterator it = new_victors.begin(); it != new_victors.end(); ++it) {
            int victor_player_id = it->first;

            const std::set<std::string>& reasons = it->second;
            for (std::set<std::string>::const_iterator reason_it = reasons.begin(); reason_it != reasons.end(); ++reason_it) {
                std::string reason_string = *reason_it;

                // see if player has already won the game...
                bool new_victory = false;
                std::map<int, std::set<std::string> >::const_iterator vict_it = m_victors.find(victor_player_id);
                if (vict_it == m_victors.end()) {
                    // player hasn't yet won, so victory is new
                    new_victory = true;
                } else {
                    // player has won at least once, but also need to check of the type of victory is new
                    std::set<std::string>::const_iterator vict_type_it = vict_it->second.find(reason_string);
                    if (vict_type_it == vict_it->second.end())
                        new_victory = true;
                }

                if (new_victory) {
                    // record victory
                    m_victors[victor_player_id].insert(reason_string);

                    Empire* empire = GetPlayerEmpire(victor_player_id);
                    if (!empire) {
                        Logger().errorStream() << "Trying to grant victory to a missing empire!";
                        continue;
                    }
                    const std::string& victor_empire_name = empire->Name();
                    int victor_empire_id = empire->EmpireID();


                    // notify all players of victory
                    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
                        boost::shared_ptr<PlayerConnection> pc = *player_it;
                        int recipient_player_id = pc->ID();
                        pc->SendMessage(VictoryDefeatMessage(recipient_player_id, Message::VICTORY, reason_string, victor_empire_id));
                        if (Empire* recipient_empire = GetPlayerEmpire(recipient_player_id))
                            recipient_empire->AddSitRepEntry(CreateVictorySitRep(reason_string, victor_empire_name));
                    }
                }
            }
        }
    }


    if (!eliminations.empty()) {
        Sleep(1000); // time for elimination messages to propegate

        // inform all players of eliminations
        for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
            int elim_player_id = it->first;
            int elim_empire_id = it->second;
            Empire* empire = empires.Lookup(elim_empire_id);
            if (!empire) {
                Logger().errorStream() << "Trying to eliminate a missing empire!";
                continue;
            }
            const std::string& elim_empire_name = empire->Name();

            // notify all players of disconnection, and end game of eliminated player
            for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
                boost::shared_ptr<PlayerConnection> pc = *player_it;
                int recipient_player_id = pc->ID();
                if (recipient_player_id == elim_player_id) {
                    pc->SendMessage(EndGameMessage(recipient_player_id, Message::YOU_ARE_ELIMINATED));
                    m_ai_clients.erase(pc->PlayerName());   // done now so that PlayerConnection doesn't need to be re-retreived when dumping connections
                } else {
                    pc->SendMessage(PlayerEliminatedMessage(recipient_player_id, elim_empire_id, elim_empire_name));    // PlayerEliminatedMessage takes the eliminated empire id, not the eliminated player id, for unknown reasons, as of this writing
                    if (Empire* recipient_empire = GetPlayerEmpire(recipient_player_id))
                        recipient_empire->AddSitRepEntry(CreateEmpireEliminatedSitRep(elim_empire_name));
                }
            }
        }

        // dump connections to eliminated players, and remove server-side empire data
        for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
            int elim_empire_id = it->second;
            // remove eliminated empire's ownership of UniverseObjects
            Universe::ObjectVec object_vec = GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(elim_empire_id));
            for (Universe::ObjectVec::iterator obj_it = object_vec.begin(); obj_it != object_vec.end(); ++obj_it)
                (*obj_it)->RemoveOwner(elim_empire_id);

            m_log_category.debugStream() << "ServerApp::ProcessTurns : Player " << it->first << " is eliminated and dumped";
            m_eliminated_players.insert(it->first);
            m_networking.Disconnect(it->first);
            m_ai_IDs.erase(it->first);

            empires.EliminateEmpire(it->second);
            RemoveEmpireTurn(it->second);
        }
    }

    // indicate that the clients are waiting for their new Universes
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::DOWNLOADING, -1));
    }

    // send new-turn updates to all players
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        int empire_id = GetPlayerEmpire((*player_it)->ID())->EmpireID();
        (*player_it)->SendMessage(TurnUpdateMessage((*player_it)->ID(), empire_id, m_current_turn, m_empires, m_universe, players));
    }
}
