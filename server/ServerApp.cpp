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

#include <GG/Font.h>

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <SDL/SDL_timer.h>
// for dummy video driver setenv-hack
#include <SDL/SDL_getenv.h>

#include <ctime>


namespace fs = boost::filesystem;

namespace {
#ifdef FREEORION_WIN32
    const std::string AI_CLIENT_EXE = "freeorionca.exe";
#else
    const fs::path BIN_DIR = GetBinDir();
    const std::string AI_CLIENT_EXE = (BIN_DIR / "freeorionca").native_file_string();
#endif
}


////////////////////////////////////////////////
// PlayerSaveGameData
////////////////////////////////////////////////
PlayerSaveGameData::PlayerSaveGameData() :
    m_empire(0)
{}

PlayerSaveGameData::PlayerSaveGameData(const std::string& name, Empire* empire, const boost::shared_ptr<OrderSet>& orders, const boost::shared_ptr<SaveGameUIData>& ui_data) :
    m_name(name),
    m_empire(empire),
    m_orders(orders),
    m_ui_data(ui_data)
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
    m_current_turn(INVALID_GAME_TURN)
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of singleton class ServerApp");

    s_app = this;

    const std::string SERVER_LOG_FILENAME((GetLocalDir() / "freeoriond.log").native_file_string());

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
    m_log_category.setPriority(PriorityValue(GetOptionsDB().Get<std::string>("log-level")));

    m_fsm.initiate();
}

ServerApp::~ServerApp()
{ log4cpp::Category::shutdown(); }

void ServerApp::operator()()
{
    Run();
}

void ServerApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    exit(code);
}

log4cpp::Category& ServerApp::Logger()
{
    return m_log_category;
}

void ServerApp::CreateAIClients(const std::vector<PlayerSetupData>& AIs, std::set<std::string>& expected_ai_player_names)
{
    for (std::set<int>::iterator it = m_ai_IDs.begin(); it != m_ai_IDs.end(); ++it) {
        m_networking.Disconnect(*it);
    }
    m_ai_clients.clear();
    m_ai_IDs.clear();

    int ai_client_base_number = time(0) % 999; // get a random number from which to start numbering the AI clients
    int i = 0;
    for (std::vector<PlayerSetupData>::const_iterator it = AIs.begin(); it != AIs.end(); ++it, ++i) {
        // TODO: add other command line args to AI client invocation as needed
        std::string player_name = "AI_" + boost::lexical_cast<std::string>(ai_client_base_number + i);
        expected_ai_player_names.insert(player_name);
        std::vector<std::string> args;
        args.push_back(AI_CLIENT_EXE);
        args.push_back(player_name);
        args.push_back("--settings-dir");
        args.push_back("\"" + GetOptionsDB().Get<std::string>("settings-dir") + "\"");
        args.push_back("--log-level");
        args.push_back(GetOptionsDB().Get<std::string>("log-level"));
        Logger().debugStream() << "starting " << AI_CLIENT_EXE;
        m_ai_clients.push_back(Process(AI_CLIENT_EXE, args));
        Logger().debugStream() << "done starting " << AI_CLIENT_EXE;
    }
}

ServerApp* ServerApp::GetApp()
{
    return s_app;
}

Universe& ServerApp::GetUniverse()
{
    return ServerApp::GetApp()->m_universe;
}

EmpireManager& ServerApp::Empires()
{
    return ServerApp::GetApp()->m_empires;
}

CombatModule* ServerApp::CurrentCombat()
{
    return ServerApp::GetApp()->m_current_combat;
}

ServerNetworking& ServerApp::Networking()
{
    return ServerApp::GetApp()->m_networking;
}

void ServerApp::Run()
{
    try {
        m_io_service.run();
    } catch (...) {
        CleanupAIs();
        throw;
    }
    CleanupAIs();
}

void ServerApp::CleanupAIs()
{
    for (unsigned int i = 0; i < m_ai_clients.size(); ++i) {
        m_ai_clients[i].Kill();
    }
}

void ServerApp::HandleMessage(Message& msg, PlayerConnectionPtr player_connection)
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
    case Message::CLIENT_SAVE_DATA:      m_fsm.process_event(ClientSaveData(msg, player_connection)); break;
    case Message::HUMAN_PLAYER_CHAT:     m_fsm.process_event(PlayerChat(msg, player_connection)); break;
    case Message::REQUEST_NEW_OBJECT_ID: m_fsm.process_event(RequestObjectID(msg, player_connection)); break;
    default:
        m_log_category.errorStream() << "ServerApp::HandleMessage : Received an unknown message type \""
                                     << msg.Type() << "\".  Terminating connection.";
        m_networking.Disconnect(player_connection);
        break;
    }
}

void ServerApp::HandleNonPlayerMessage(Message& msg, PlayerConnectionPtr player_connection)
{
    switch (msg.Type()) {
    case Message::HOST_SP_GAME: m_fsm.process_event(HostSPGame(msg, player_connection)); break;
    case Message::HOST_MP_GAME: m_fsm.process_event(HostMPGame(msg, player_connection)); break;
    case Message::JOIN_GAME:    m_fsm.process_event(JoinGame(msg, player_connection)); break;
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

void ServerApp::LoadGameInit(const std::vector<PlayerSaveGameData>& player_save_game_data)
{
    m_turn_sequence.clear();

    std::set<int> unused_save_game_data;
    int index_of_human_player = 0;
    for (unsigned int i = 0; i < player_save_game_data.size(); ++i) {
        assert(player_save_game_data[i].m_empire);
        // This works because the empire ID is always the same as the player ID in single player games, at least for the
        // host (human) player.
        if (player_save_game_data[i].m_empire->EmpireID() == Networking::HOST_PLAYER_ID)
            index_of_human_player = i;
        else
            unused_save_game_data.insert(i);
    }

    assert(m_networking.NumPlayers() == player_save_game_data.size());

    std::map<Empire*, const PlayerSaveGameData*> player_data_by_empire;
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        int save_game_data_index = -1;
        if ((*it)->ID() == Networking::HOST_PLAYER_ID) {
            save_game_data_index = index_of_human_player;
        } else {
            save_game_data_index = *unused_save_game_data.begin();
            unused_save_game_data.erase(save_game_data_index);
        }
        const PlayerSaveGameData& save_game_data = player_save_game_data[save_game_data_index];
        save_game_data.m_empire->SetPlayerName((*it)->PlayerName());
        Empires().InsertEmpire(save_game_data.m_empire);
        AddEmpireTurn(save_game_data.m_empire->EmpireID());
        player_data_by_empire[save_game_data.m_empire] = &save_game_data;
    }

    // This is a bit odd, but since Empires() is built from the data stored in m_player_save_game_data, and the universe
    // is loaded long before that, the universe's empire-specific views of the systems is not properly initialized when
    // the universe is loaded.  That means we must do it here.
    m_universe.RebuildEmpireViewSystemGraphs();

    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        Empire* empire = GetPlayerEmpire((*it)->ID());
        (*it)->SendMessage(GameStartMessage((*it)->ID(), m_single_player_game, empire->EmpireID(), m_current_turn, m_empires, m_universe,
                                            *player_data_by_empire[empire]->m_orders, player_data_by_empire[empire]->m_ui_data.get()));
    }

    m_losers.clear();
}

void ServerApp::NewGameInit(boost::shared_ptr<MultiplayerLobbyData> lobby_data)
{
    NewGameInit(lobby_data->m_size, lobby_data->m_shape, lobby_data->m_age, lobby_data->m_starlane_freq, lobby_data->m_planet_density, lobby_data->m_specials_freq, lobby_data->m_players);
}

void ServerApp::LoadGameInit(boost::shared_ptr<MultiplayerLobbyData> lobby_data, const std::vector<PlayerSaveGameData>& player_save_game_data)
{
    assert(!player_save_game_data.empty());

    m_turn_sequence.clear();

#if 1
#else
    std::map<int, PlayerSaveGameData> player_data_by_empire;
    for (unsigned int i = 0; i < player_save_game_data.size(); ++i) {
        assert(player_save_game_data[i].m_empire);
        player_data_by_empire[player_save_game_data[i].m_empire->EmpireID()] = player_save_game_data[i];
    }

    if (m_single_player_game) {
        while (lobby_data->m_players.size() < m_networking.size()) {
            lobby_data->m_players.push_back(PlayerSetupData());
        }
    }

    std::map<int, int> player_to_empire_ids;
    std::set<int> already_chosen_empire_ids;
    unsigned int i = 0;
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it, ++i) {
        player_to_empire_ids[(*it)->ID()] = lobby_data->m_players[i].m_save_game_empire_id;
        already_chosen_empire_ids.insert(lobby_data->m_players[i].m_save_game_empire_id);
    }

    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        const int INVALID_EMPIRE_ID = -5000;
        int empire_id = INVALID_EMPIRE_ID;
        if (player_to_empire_ids[(*it)->ID()] != -1) {
            empire_id = player_to_empire_ids[(*it)->ID()];
        } else {
            for (std::map<int, PlayerSaveGameData>::iterator player_data_it = player_data_by_empire.begin(); player_data_it != player_data_by_empire.end(); ++player_data_it) {
                if (already_chosen_empire_ids.find(player_data_it->first) == already_chosen_empire_ids.end()) {
                    empire_id = player_data_it->first;
                    already_chosen_empire_ids.insert(empire_id);
                    player_to_empire_ids[(*it)->ID()] = empire_id;
                    // since this must be an AI player, it does not have the correct player name set in its Empire yet, so we need to do so now
                    player_data_it->second.m_empire->SetPlayerName((*it)->PlayerName());
                    Empires().InsertEmpire(player_data_it->second.m_empire);
                    break;
                }
            }
        }
        assert(empire_id != INVALID_EMPIRE_ID);
        m_turn_sequence[empire_id] = 0;
    }

    // This is a bit odd, but since Empires() is built from the data stored in player_save_game_data, and the universe
    // is loaded long before that, the universe's empire-specific views of the systems is not properly initialized when
    // the universe is loaded.  That means we must do it here.
    m_universe.RebuildEmpireViewSystemGraphs();

    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        int empire_id = player_to_empire_ids[(*it)->ID()];
        (*it)->SendMessage(GameStartMessage((*it)->ID(), m_single_player_game, empire_id, m_current_turn, m_empires, m_universe,
                                            *player_data_by_empire[empire_id].m_orders, player_data_by_empire[empire_id].m_ui_data.get()));
    }
#endif

    m_losers.clear();
}

void ServerApp::NewGameInit(int size, Shape shape, Age age, StarlaneFrequency starlane_freq, PlanetDensity planet_density, SpecialsFrequency specials_freq,
                            const std::map<int, PlayerSetupData>& player_setup_data)
{
    m_turn_sequence.clear();

    m_current_turn = BEFORE_FIRST_TURN;     // every UniverseObject created before game starts will have m_created_on_turn BEFORE_FIRST_TURN
    m_universe.CreateUniverse(size, shape, age, starlane_freq, planet_density, specials_freq,
                              m_networking.NumPlayers() - m_ai_clients.size(), m_ai_clients.size(), player_setup_data);
    m_current_turn = 1;                     // after all game initialization stuff has been created, can set current turn to 1 for start of game

    // TODO: here we add empires to turn sequence map -- according to spec this should be done randomly; for now, it's not
    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        AddEmpireTurn((*it)->ID());
    }

    for (ServerNetworking::const_established_iterator it = m_networking.established_begin(); it != m_networking.established_end(); ++it) {
        int player_id = (*it)->ID();
        int empire_id = GetPlayerEmpire(player_id)->EmpireID();
        (*it)->SendMessage(GameStartMessage(player_id, m_single_player_game, empire_id, m_current_turn, m_empires, m_universe));
    }

    m_losers.clear();
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


void ServerApp::ProcessTurns()
{
    Empire                    *pEmpire;
    OrderSet                  *pOrderSet;
    OrderSet::const_iterator  order_it;

    // Now all orders, then process turns
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        // broadcast UI message to all players
        for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
            (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::PROCESSING_ORDERS, it->first));
        }

        pEmpire = Empires().Lookup(it->first);
        pEmpire->ClearSitRep();
        pOrderSet = it->second;
     
        // execute order set
        for (order_it = pOrderSet->begin(); order_it != pOrderSet->end(); ++order_it) {
            // TODO: Consider adding exeption handling here 
            order_it->second->Execute();
        }
    }    

    // filter FleetColonizeOrder for later processing
    std::map<int,std::vector<FleetColonizeOrder*> > colonize_order_map;
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        pOrderSet = it->second;

        // filter FleetColonizeOrder and sort them per planet
        FleetColonizeOrder *order;
        for ( order_it = pOrderSet->begin(); order_it != pOrderSet->end(); ++order_it)
            if((order=dynamic_cast<FleetColonizeOrder*>(order_it->second)))
            {
                std::map<int,std::vector<FleetColonizeOrder*> >::iterator it = colonize_order_map.find(order->PlanetID());
                if(it == colonize_order_map.end())
                {
                    colonize_order_map.insert(std::pair<int,std::vector<FleetColonizeOrder*> >(order->PlanetID(),std::vector<FleetColonizeOrder*>()));
                    it = colonize_order_map.find(order->PlanetID());
                }
                it->second.push_back(order);
            }
    }

    // colonization apply be the following rules
    // 1 - if there is only own empire which tries to colonize a planet, is allowed to do so
    // 2 - if there are more than one empire then
    // 2.a - if only one empire which tries to colonize (empire who don't are ignored) is armed, this empire wins the race
    // 2.b - if more than one empire is armed or all forces are unarmed, no one can colonize the planet
    for (std::map<int,std::vector<FleetColonizeOrder*> >::iterator it = colonize_order_map.begin(); it != colonize_order_map.end(); ++it)
    {
        Planet *planet = GetUniverse().Object<Planet>(it->first);

        // only one empire?
        if (it->second.size()==1) {
            it->second[0]->ServerExecute();
            pEmpire = Empires().Lookup( it->second[0]->EmpireID() );
            pEmpire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet->SystemID(), planet->ID()));
        } else {
            const System *system = GetUniverse().Object<System>(planet->SystemID());

            std::vector<const Fleet*> vec_fleet = system->FindObjects<Fleet>();
            std::set<int> set_empire_with_military;
            for (unsigned int i=0;i<vec_fleet.size();i++)
                for (Fleet::const_iterator ship_it=vec_fleet[i]->begin();ship_it!=vec_fleet[i]->end();++ship_it)
                    if (GetUniverse().Object<Ship>(*ship_it)->IsArmed())
                    {
                        set_empire_with_military.insert(*vec_fleet[i]->Owners().begin());
                        break;
                    }

            // set the first empire as winner for now
            int winner = 0;
            // is the current winner armed?
            bool winner_is_armed = set_empire_with_military.find(it->second[0]->EmpireID()) != set_empire_with_military.end();
            for (unsigned int i=1;i<it->second.size();i++)
                // is this empire armed?
                if (set_empire_with_military.find(it->second[i]->EmpireID()) != set_empire_with_military.end())
                {
                    // if this empire is armed and the former winner too, noone can win
                    if (winner_is_armed)
                    {
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

            for (int i=0;i<static_cast<int>(it->second.size());i++)
                if (winner==i) {
                    it->second[i]->ServerExecute();
                    pEmpire = Empires().Lookup( it->second[i]->EmpireID() );
                    pEmpire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet->SystemID(), planet->ID()));
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
        
    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it) {
        // save for possible SitRep generation after moving...
        const Fleet* fleet = GetUniverse().Object<Fleet>(it->first);
        int eta = -1;
        if (fleet)
            eta = fleet->ETA().first;
        
        it->second->MovementPhase();
        
        // SitRep for fleets having arrived at destinations, to all owners of those fleets
        if (fleet) {
            if (eta == 1) {
                std::set<int> owners_set = fleet->Owners();
                for (std::set<int>::const_iterator owners_it = owners_set.begin(); owners_it != owners_set.end(); ++owners_it) {
                    pEmpire = Empires().Lookup( *owners_it );
                    pEmpire->AddSitRepEntry(CreateFleetArrivedAtDestinationSitRep(fleet->SystemID(), fleet->ID()));
                }
            }
        }
    }

    // find planets which have starved to death
    std::vector<Planet*> plt_vec = GetUniverse().FindObjects<Planet>();
    for (std::vector<Planet*>::iterator it = plt_vec.begin();it!=plt_vec.end();++it)
        if ((*it)->Owners().size()>0 && (*it)->PopPoints()==0.0)
        {
            // add some information to sitrep
            Empire *empire = Empires().Lookup(*(*it)->Owners().begin());
            empire->AddSitRepEntry(CreatePlanetStarvedToDeathSitRep((*it)->SystemID(),(*it)->ID()));
            (*it)->Reset();
        }

    // check for combats, and resolve them.
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::COMBAT, -1));
    }

    std::vector<System*> sys_vec = GetUniverse().FindObjects<System>();
    bool combat_happend = false;
    for (std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it)
    {
        std::vector<CombatAssets> empire_combat_forces;
        System* system = *it;
      
        std::vector<Fleet*> flt_vec = system->FindObjects<Fleet>();
        if (flt_vec.empty()) continue;  // skip systems with not fleets, as these can't have combat

        for (std::vector<Fleet*>::iterator flt_it = flt_vec.begin();flt_it != flt_vec.end(); ++flt_it)
        {
            Fleet* flt = *flt_it;
            // a fleet should belong only to one empire!?
            if (1==flt->Owners().size())
            {
                std::vector<CombatAssets>::iterator ecf_it = std::find(empire_combat_forces.begin(),empire_combat_forces.end(),CombatAssetsOwner(Empires().Lookup(*flt->Owners().begin())));

                if (ecf_it==empire_combat_forces.end())
                {
                    CombatAssets ca(Empires().Lookup(*flt->Owners().begin()));
                    ca.fleets.push_back(flt);
                    empire_combat_forces.push_back(ca);
                }
                else
                    (*ecf_it).fleets.push_back(flt);
            }
        }
        std::vector<Planet*> plt_vec = system->FindObjects<Planet>();
        for (std::vector<Planet*>::iterator plt_it = plt_vec.begin();plt_it != plt_vec.end(); ++plt_it)
        {
            Planet* plt = *plt_it;
            // a planet should belong only to one empire!?
            if (1==plt->Owners().size())
            {           
                std::vector<CombatAssets>::iterator ecf_it = std::find(empire_combat_forces.begin(),empire_combat_forces.end(),CombatAssetsOwner(Empires().Lookup(*plt->Owners().begin())));

                if (ecf_it==empire_combat_forces.end())
                {
                    CombatAssets ca(Empires().Lookup(*plt->Owners().begin()));
                    ca.planets.push_back(plt);
                    empire_combat_forces.push_back(ca);
                }
                else
                    (*ecf_it).planets.push_back(plt);
            }
        }

        if (empire_combat_forces.size()>1)
        {
            combat_happend=true;
            CombatSystem combat_system;
            combat_system.ResolveCombat(system->ID(),empire_combat_forces);
        }
    }

    // if a combat happened, give the human user a chance to look at the results
    if (combat_happend)
        SDL_Delay(1500); // TODO: Put this delay client-side.

    // process production and growth phase
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::EMPIRE_PRODUCTION, -1));
    }

    for (EmpireManager::iterator it = Empires().begin(); it != Empires().end(); ++it)
        it->second->UpdateResourcePool();
    
    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it) {
        it->second->ResetMaxMeters();
        it->second->AdjustMaxMeters();
    }

    GetUniverse().ApplyEffects();
    GetUniverse().RebuildEmpireViewSystemGraphs();

    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it) {
        it->second->PopGrowthProductionResearchPhase(); // Population growth / starvation, health meter growth, resource current meter growth
        it->second->ClampMeters();  // limit current meters by max meters
        for (MeterType i = MeterType(0); i != NUM_METER_TYPES; i = MeterType(i + 1)) {
            if (Meter* meter = it->second->GetMeter(i)) {
                meter->m_previous_current = meter->m_initial_current;
                meter->m_previous_max = meter->m_initial_max;
                meter->m_initial_current = meter->m_current;
                meter->m_initial_max = meter->m_max;
            }
        }
    }

    // check for completed research, production or social projects, pay maintenance.  Update stockpiles.
    // doesn't do actual population growth, which occurs above when PopGrowthProductionResearchPhase() is called
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        Empire* empire = Empires().Lookup(it->first);
        empire->CheckResearchProgress();
        empire->CheckProductionProgress();
        empire->CheckTradeSocialProgress();
        empire->CheckGrowthFoodProgress();
    }

    // loop and free all orders
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }   
    
    ++m_current_turn;

    // indicate that the clients are waiting for their new Universes
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::DOWNLOADING, -1));
    }

    // check if all empires are still alive
    std::map<int, int> eliminations; // map from player ids to empire ids
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        if (GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(it->first)).empty()) { // when you're out of planets, your game is over
            std::string player_name = it->second->PlayerName();
            for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
                if ((*player_it)->PlayerName() == player_name) {
                    // record this player/empire so we can send out messages about it
                    eliminations[(*player_it)->ID()] = it->first;
                    break;
                }
            }
        } 
    }

    // clean up defeated empires
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        // remove the empire from play
        Universe::ObjectVec object_vec = GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(it->second));
        for (unsigned int j = 0; j < object_vec.size(); ++j)
            object_vec[j]->RemoveOwner(it->second);
    }

    // send new-turn updates to all players
    for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
        pEmpire = GetPlayerEmpire((*player_it)->ID());
        (*player_it)->SendMessage(TurnUpdateMessage((*player_it)->ID(), pEmpire->EmpireID(), m_current_turn, m_empires, m_universe));
    }

    // notify all players of the eliminated players
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        for (ServerNetworking::const_established_iterator player_it = m_networking.established_begin(); player_it != m_networking.established_end(); ++player_it) {
            if ((*player_it)->ID() == it->first)
                (*player_it)->SendMessage(EndGameMessage((*player_it)->ID(), Message::YOU_ARE_DEFEATED));
            else
                (*player_it)->SendMessage(PlayerEliminatedMessage((*player_it)->ID(), Empires().Lookup(it->second)->Name()));
        }
    }

    // dump connections to eliminated players, and remove server-side empire data
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        m_log_category.debugStream() << "ServerApp::ProcessTurns : Player " << it->first << " is marked as a loser and dumped";
        m_losers.insert(it->first);
        m_networking.Disconnect(it->first);
        m_ai_IDs.erase(it->first);
        Empires().EliminateEmpire(it->second);
        RemoveEmpireTurn(it->second);
    }

    // determine if victory conditions exist
    if (m_networking.NumPlayers() == 1) { // if there is only one player left, that player is the winner
        m_log_category.debugStream() << "ServerApp::ProcessTurns : One player left -- sending victory notification and terminating.";
        while (m_networking.NumPlayers() == 1) {
            m_networking.SendMessage(EndGameMessage((*m_networking.established_begin())->ID(), Message::LAST_OPPONENT_DEFEATED));
            SDL_Delay(100); // TODO: It should be possible to eliminate this by using linger.
        }
        m_networking.DisconnectAll();
        Exit(0);
    } else if (m_ai_IDs.size() == m_networking.NumPlayers()) { // if there are none but AI players left, we're done
        m_log_category.debugStream() << "ServerApp::ProcessTurns : No human players left -- server terminating.";
        m_networking.DisconnectAll();
        Exit(0);
    }
}
