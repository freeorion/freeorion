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
#include <GG/net/fastevents.h>

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

// for dummy video driver setenv-hack
#include "SDL_getenv.h"

#include <ctime>


namespace fs = boost::filesystem;

namespace {
#ifdef FREEORION_WIN32
    const std::string AI_CLIENT_EXE = "freeorionca.exe";
#else
    const fs::path BIN_DIR = GetBinDir();
    const std::string AI_CLIENT_EXE = (BIN_DIR / "freeorionca").native_file_string();
#endif    

    void RebuildSaveGameEmpireData(std::vector<SaveGameEmpireData>& save_game_empire_data, const std::string& save_game_filename)
    {
        save_game_empire_data.clear();
        std::vector<PlayerSaveGameData> player_save_game_data;
        {
            int current_turn;
            Universe universe;
            LoadGame((GetLocalDir() / "save" / save_game_filename).native_file_string().c_str(),
                     current_turn, player_save_game_data, universe);
        }
        for (unsigned int i = 0; i < player_save_game_data.size(); ++i) {
            SaveGameEmpireData data;
            Empire* empire = player_save_game_data[i].m_empire;
            data.m_id = empire->EmpireID();
            data.m_name = empire->Name();
            data.m_player_name = empire->PlayerName();
            data.m_color = empire->Color();
            save_game_empire_data.push_back(data);
            delete empire;
        }
    }
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

ServerApp::ServerApp(int argc, char* argv[]) : 
    m_current_combat(0), 
    m_log_category(log4cpp::Category::getRoot()),
    m_state(SERVER_IDLE),
    m_current_turn(INVALID_GAME_TURN)
{
    for (int n = 0; n < 10000; n++) {
        int x = n/(n+1);
        std::cerr << "\n" << n;
    }
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
    m_log_category.setPriority(log4cpp::Priority::DEBUG);
    m_log_category.debug("freeoriond logger initialized.");
    m_log_category.debugStream() << "ServerApp::ServerApp : Server now in mode " << SERVER_IDLE << " (SERVER_IDLE).";
}

ServerApp::~ServerApp()
{
    m_log_category.debug("Shutting down freeoriond logger...");
    log4cpp::Category::shutdown();
}

void ServerApp::operator()()
{
    Run();
}

void ServerApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    SDLQuit();
    exit(code);
}

log4cpp::Category& ServerApp::Logger()
{
    return m_log_category;
}

void ServerApp::CreateAIClients(const std::vector<PlayerSetupData>& AIs)
{
    m_expected_ai_players.clear();
    for (std::set<int>::iterator it = m_ai_IDs.begin(); it != m_ai_IDs.end(); ++it) {
        m_network_core.DumpPlayer(*it);
    }
    m_ai_clients.clear();
    m_ai_IDs.clear();

    int ai_client_base_number = time(0) % 999; // get a random number from which to start numbering the AI clients
    int i = 0;
    for (std::vector<PlayerSetupData>::const_iterator it = AIs.begin(); it != AIs.end(); ++it, ++i) {
        // TODO: add other command line args to AI client invocation as needed
        std::string player_name = "AI_" + boost::lexical_cast<std::string>(ai_client_base_number + i);
        m_expected_ai_players.insert(player_name);
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

void ServerApp::HandleMessage(const Message& msg)
{
    switch (msg.Type()) {
    case Message::START_MP_GAME: {
        if (m_lobby_data.m_new_game) { // new game
            m_galaxy_size = m_lobby_data.m_size;
            m_galaxy_shape = m_lobby_data.m_shape;
            m_galaxy_age = m_lobby_data.m_age;
            m_starlane_freq = m_lobby_data.m_starlane_freq;
            m_planet_density = m_lobby_data.m_planet_density;
            m_specials_freq = m_lobby_data.m_specials_freq;
            m_expected_players = m_network_core.PlayerConnections().size() + m_lobby_data.m_AIs.size();
            for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                if (it->first != msg.Sender())
                    m_network_core.SendMessage(Message(Message::GAME_START, -1, it->first, Message::CLIENT_LOBBY_MODULE, ""));
            }
            m_state = SERVER_GAME_SETUP;
            CreateAIClients(m_lobby_data.m_AIs);
            m_player_save_game_data.clear();
            m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
            if (m_expected_players == static_cast<int>(m_network_core.PlayerConnections().size())) {
                NewGameInit();
                m_state = SERVER_WAITING;
                m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
            }
        } else { // load game
            std::map<int, PlayerConnection>::const_iterator sender_it = m_network_core.PlayerConnections().find(msg.Sender());
            if (sender_it != m_network_core.PlayerConnections().end() && sender_it->second.host) {
                m_empires.RemoveAllEmpires();
                m_single_player_game = false;
                LoadGame((GetLocalDir() / "save" / m_lobby_data.m_save_games[m_lobby_data.m_save_file_index]).native_file_string(),
                         m_current_turn, m_player_save_game_data, m_universe);
                m_expected_players = m_player_save_game_data.size();
                m_empires.RemoveAllEmpires();
                for (unsigned int i = 0; i < m_player_save_game_data.size(); ++i) {
                    for (unsigned int j = 0; j < m_lobby_data.m_players.size(); ++j) {
                        assert(m_player_save_game_data[i].m_empire);
                        if (m_lobby_data.m_players[j].m_save_game_empire_id == m_player_save_game_data[i].m_empire->EmpireID()) {
                            std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin();
                            std::advance(player_it, j);
                            m_player_save_game_data[i].m_name = player_it->second.name;
                            m_player_save_game_data[i].m_empire->SetPlayerName(player_it->second.name);
                        }
                    }
                    m_empires.InsertEmpire(m_player_save_game_data[i].m_empire);
                }

                for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                    if (it != sender_it)
                        m_network_core.SendMessage(Message(Message::GAME_START, -1, it->first, Message::CLIENT_LOBBY_MODULE, ""));
                }

                int AI_clients = m_expected_players - m_network_core.PlayerConnections().size();
                CreateAIClients(std::vector<PlayerSetupData>(AI_clients));
                m_state = SERVER_GAME_SETUP;

                if (!AI_clients)
                    LoadGameInit();
            } else {
                m_log_category.errorStream() << "Player #" << msg.Sender() << " attempted to initiate a game load, but is not the host, or is "
                    "not found in the player list.";
            }
        }
        break;
    }

    case Message::LOBBY_UPDATE: {
        MultiplayerLobbyData mp_lobby_data;
        ExtractMessageData(msg, mp_lobby_data);

        // NOTE: The client is only allowed to update certain of these, so those are the only ones we'll copy into m_lobby_data.
        m_lobby_data.m_new_game = mp_lobby_data.m_new_game;
        m_lobby_data.m_size = mp_lobby_data.m_size;
        m_lobby_data.m_shape = mp_lobby_data.m_shape;
        m_lobby_data.m_age = mp_lobby_data.m_age;
        m_lobby_data.m_starlane_freq = mp_lobby_data.m_starlane_freq;
        m_lobby_data.m_planet_density = mp_lobby_data.m_planet_density;
        m_lobby_data.m_specials_freq = mp_lobby_data.m_specials_freq;

        bool new_save_file_selected = false;
        if (mp_lobby_data.m_save_file_index != m_lobby_data.m_save_file_index &&
            0 <= mp_lobby_data.m_save_file_index && mp_lobby_data.m_save_file_index < static_cast<int>(m_lobby_data.m_save_games.size())) {
            m_lobby_data.m_save_file_index = mp_lobby_data.m_save_file_index;
            RebuildSaveGameEmpireData(m_lobby_data.m_save_game_empire_data, m_lobby_data.m_save_games[m_lobby_data.m_save_file_index]);
            // reset the current choice of empire for each player, since the new save game's empires may not have the same IDs
            for (unsigned int i = 0; i < m_lobby_data.m_players.size(); ++i) {
                m_lobby_data.m_players[i].m_save_game_empire_id = -1;
            }
            new_save_file_selected = true;
        }
        m_lobby_data.m_players = mp_lobby_data.m_players;

        for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
            if (it->first != msg.Sender() || new_save_file_selected)
                m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, m_lobby_data));
        }
        break;
    }

    case Message::LOBBY_CHAT: {
        if (msg.Receiver() == -1) { // the receiver is everyone (except the sender)
            for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                if (it->first != msg.Sender())
                    m_network_core.SendMessage(ServerLobbyChatMessage(msg.Sender(), it->first, msg.GetText()));
            }
        } else {
            m_network_core.SendMessage(ServerLobbyChatMessage(msg.Sender(), msg.Receiver(), msg.GetText()));
        }
        break;
    }

    case Message::LOBBY_HOST_ABORT: {
        for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
            if (it->first != msg.Sender()) {
                m_network_core.SendMessage(ServerLobbyHostAbortMessage(it->first));
                m_network_core.DumpPlayer(it->first);
            }
        }
        break;
    }

    case Message::LOBBY_EXIT: {
        unsigned int i = 0;
        for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it, ++i) {
            if (it->first == msg.Sender()) {
                if (i < m_lobby_data.m_players.size())
                    m_lobby_data.m_players.erase(m_lobby_data.m_players.begin() + i); // remove the exiting player's PlayerSetupData struct
            } else {
                m_network_core.SendMessage(ServerLobbyExitMessage(msg.Sender(), it->first));
            }
        }
        m_network_core.DumpPlayer(msg.Sender());
        break;
    }

    case Message::SAVE_GAME: {
        if (m_network_core.PlayerConnections().find(msg.Sender()) != m_network_core.PlayerConnections().end()) {
            // send out all save game data requests
            std::set<int> needed_reponses;
            m_player_connections_responded.clear();
            m_player_save_game_data.clear();
            for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                m_network_core.SendMessage(ServerSaveGameMessage(it->first));
                needed_reponses.insert(it->first);
            }

            // wait for them all to come in
            SDL_Event ev;
            const unsigned int SYCHRONOUS_TIMEOUT = 15000; // give up after this many ms without any valid responses
            unsigned int start_time = SDL_GetTicks();
            while (1) {
                unsigned int starting_responses = m_player_connections_responded.size();
                FE_PollEvent(&ev);
                if (ev.type == SDL_USEREVENT) {
                    int net2_type = NET2_GetEventType(&ev);
                    if (net2_type == NET2_ERROREVENT || 
                        net2_type == NET2_TCPACCEPTEVENT || 
                        net2_type == NET2_TCPRECEIVEEVENT || 
                        net2_type == NET2_TCPCLOSEEVENT || 
                        net2_type == NET2_UDPRECEIVEEVENT) {
                        m_network_core.HandleNetEvent(ev);
                    }
                }
                if (starting_responses < m_player_connections_responded.size())
                    start_time = SDL_GetTicks(); // reset timeout whenever there's a valid response
                if (m_player_connections_responded == needed_reponses || SYCHRONOUS_TIMEOUT < SDL_GetTicks() - start_time)
                    break;
            }
            if (m_player_connections_responded == needed_reponses) {
                SaveGame(msg.GetText(), m_current_turn, m_player_save_game_data, m_universe);
                m_network_core.SendMessage(ServerSaveGameMessage(msg.Sender(), true));
            }
        } else {
            m_log_category.errorStream() << "Player #" << msg.Sender() << " attempted to initiate a game save, but is not found in the player list.";
        }
        break;
    }

    case Message::LOAD_GAME: { // single-player loading (multiplayer loading is handled through the lobby interface)
        std::map<int, PlayerConnection>::const_iterator sender_it = m_network_core.PlayerConnections().find(msg.Sender());
        if (sender_it != m_network_core.PlayerConnections().end() && sender_it->second.host) {
            m_empires.RemoveAllEmpires();
            m_single_player_game = true;
            LoadGame(msg.GetText(), m_current_turn, m_player_save_game_data, m_universe);
            m_expected_players = m_player_save_game_data.size();
            CreateAIClients(std::vector<PlayerSetupData>(m_expected_players - 1));
            m_state = SERVER_GAME_SETUP;
        } else {
            m_log_category.errorStream() << "Player #" << msg.Sender() << " attempted to initiate a game save, but is not the host, or is "
                "not found in the player list.";
        }
        break;
    }

    case Message::TURN_ORDERS: {
        OrderSet* order_set = new OrderSet;
        ExtractMessageData(msg, *order_set);

        // check order validity -- all orders must originate from this empire in order to be considered valid
        Empire* empire = GetPlayerEmpire(msg.Sender());
        assert(empire);
        for (OrderSet::const_iterator it = order_set->begin(); it != order_set->end(); ++it) {
            Order* order = it->second;
            assert(order);
            if (empire->EmpireID() != order->EmpireID()) {
                throw std::runtime_error(
                    "ServerApp::HandleMessage : Player \"" + empire->PlayerName() + "\""
                    " attempted to issue an order for player "
                    "\"" + Empires().Lookup(order->EmpireID())->PlayerName() + "\"!  Terminating...");
            }
        }

        m_network_core.SendMessage(TurnProgressMessage(msg.Sender(), Message::WAITING_FOR_PLAYERS, -1));

        m_log_category.debugStream() << "ServerApp::HandleMessage : Received orders from player " << msg.Sender();

        /* if all orders are received already, do nothing as we are processing a turn */
        if (AllOrdersReceived())
            break;

        /* add orders to turn sequence */    
        SetEmpireTurnOrders(GetPlayerEmpire(msg.Sender())->EmpireID(), order_set);

        /* look to see if all empires are done */
        if (AllOrdersReceived()) {
            m_log_category.debugStream() << "ServerApp::HandleMessage : All orders received; processing turn...";
            ProcessTurns();
        }
        break;
    }

    case Message::CLIENT_SAVE_DATA: {
        boost::shared_ptr<OrderSet> order_set(new OrderSet);
        boost::shared_ptr<SaveGameUIData> ui_data(new SaveGameUIData);
        if (!ExtractMessageData(msg, *order_set, *ui_data))
            ui_data.reset();
        std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().find(msg.Sender());
        assert(player_it != m_network_core.PlayerConnections().end());
        m_player_save_game_data.push_back(PlayerSaveGameData(player_it->second.name, GetPlayerEmpire(msg.Sender()), order_set, ui_data));
        m_player_connections_responded.insert(msg.Sender());
        break;
    }

    case Message::CHAT_MSG: {
        if (msg.Receiver() == -1) {
            // rebroadcast message to all players
            std::map<int, PlayerConnection> info_map = m_network_core.PlayerConnections();
            for (std::map<int, PlayerConnection>::const_iterator it = info_map.begin(); it != info_map.end(); ++it)
                m_network_core.SendMessage(SingleRecipientChatMessage(msg.Sender(), it->first, msg.GetText()));
        } else {
            /* shouldn't get any messages addressed to -1 (the server) here, as these should have been rebroadcast
               to players in ServerNetworkCore::DispatchMessage, but just in case... */
            m_network_core.SendMessage(msg);
        }
        break;
    }

    case Message::REQUEST_NEW_OBJECT_ID: {
        /* get get ID and send back to client, it's waiting for this */
        m_network_core.SendMessage(DispatchObjectIDMessage(msg.Sender(), GetUniverse().GenerateObjectID()));
        break;
    }

    case Message::END_GAME: {
        std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().find(msg.Sender());
        if (it != m_network_core.PlayerConnections().end() && it->second.host) {
            for (std::map<int, PlayerConnection>::const_iterator it2 = m_network_core.PlayerConnections().begin(); it2 != m_network_core.PlayerConnections().end(); ++it2) {
                if (it->first != it2->first)
                    m_network_core.SendMessage(EndGameMessage(-1, it2->first));
            }
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_network_core.SendMessage(Message(Message::SERVER_STATUS, -1, msg.Sender(), Message::CORE, boost::lexical_cast<std::string>(m_state)));
            m_network_core.DumpAllConnections();
            Exit(0);
        }
        break;
    }

    default: {
        m_log_category.errorStream() << "ServerApp::HandleMessage : Received an unknown message type \"" << msg.Type() << "\".";
        break;
    }
    }
}

void ServerApp::HandleNonPlayerMessage(const Message& msg, const PlayerConnection& connection)
{
    switch (msg.Type()) {
    case Message::HOST_SP_GAME: {
        Logger().errorStream() << "pc size: " << m_network_core.PlayerConnections().size() << " ais: " << m_expected_ai_players.size();
        if (m_network_core.PlayerConnections().empty() && m_expected_ai_players.empty()) {
            SinglePlayerSetupData setup_data;
            ExtractMessageData(msg, setup_data);

            PlayerConnection host_player_info(connection.socket, connection.address, setup_data.m_host_player_name, true);
            int player_id = NetworkCore::HOST_PLAYER_ID;

            // players settings
            m_single_player_game = true;
            m_expected_players = setup_data.m_AIs + 1;
            
            // universe settings
            m_galaxy_size = setup_data.m_size;
            m_galaxy_shape = setup_data.m_shape;
            m_galaxy_age = setup_data.m_age;
            m_starlane_freq = setup_data.m_starlane_freq;
            m_planet_density = setup_data.m_planet_density;
            m_specials_freq = setup_data.m_specials_freq;

            // ...?
            m_player_save_game_data.clear();
            
            // remove any existing players data from lobby
            m_lobby_data.m_players.clear();
            // and replace with just the single player
            m_lobby_data.m_players.push_back(PlayerSetupData());
            m_lobby_data.m_players.back().m_player_id = 0;
            m_lobby_data.m_players.back().m_player_name = setup_data.m_host_player_name;
            m_lobby_data.m_players.back().m_empire_name = setup_data.m_empire_name;
            m_lobby_data.m_players.back().m_empire_color = setup_data.m_empire_color;

            // Create AI client processes
            CreateAIClients(std::vector<PlayerSetupData>(setup_data.m_AIs));

            m_state = SERVER_GAME_SETUP;
            if (m_network_core.EstablishPlayer(connection.socket, player_id, host_player_info)) {
                m_network_core.SendMessage(HostSPAckMessage(player_id));
                m_network_core.SendMessage(JoinAckMessage(player_id));
            }

        } else {
            const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
            m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to host "
                "a new game but there was already one in progress or one being setup.  Terminating connection to " << 
                (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
            m_network_core.DumpConnection(connection.socket);
        }
        break;
    }

    case Message::HOST_MP_GAME: {
        if (m_network_core.PlayerConnections().empty() && m_expected_ai_players.empty()) {
            std::string host_player_name = msg.GetText();
            PlayerConnection host_player_info(connection.socket, connection.address, host_player_name, true);
            int player_id = NetworkCore::HOST_PLAYER_ID;

            // start an MP lobby situation so that game settings can be established
            m_single_player_game = false;
            m_state = SERVER_MP_LOBBY;
            m_lobby_data = MultiplayerLobbyData(true);
            m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_MP_LOBBY << " (SERVER_MP_LOBBY).";
            if (m_network_core.EstablishPlayer(connection.socket, player_id, host_player_info)) {
                m_network_core.SendMessage(HostMPAckMessage(player_id));
                m_network_core.SendMessage(JoinAckMessage(player_id));
                m_lobby_data.m_players.push_back(PlayerSetupData());
                m_lobby_data.m_players.back().m_player_id = player_id;
                m_lobby_data.m_players.back().m_player_name = host_player_name;
                m_lobby_data.m_players.back().m_empire_color = EmpireColors().at(0);
            }
            m_network_core.SendMessage(ServerLobbyUpdateMessage(player_id, m_lobby_data));
        } else {
            const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
            m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to host "
                "a new game but there was already one in progress or one being setup.  Terminating connection to " << 
                (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
            m_network_core.DumpConnection(connection.socket);
        }
        break;
    }

    case Message::JOIN_GAME: {
        std::string player_name = msg.GetText();

        PlayerConnection player_info(connection.socket, connection.address, player_name, false);
        int player_id = std::max(NetworkCore::HOST_PLAYER_ID + 1, static_cast<int>(m_network_core.PlayerConnections().size()));
        if (player_id) {
            player_id = m_network_core.PlayerConnections().rbegin()->first + 1;
        }

        if (m_state == SERVER_MP_LOBBY) { // enter an MP lobby
            if (m_network_core.EstablishPlayer(connection.socket, player_id, player_info)) {
                m_network_core.SendMessage(JoinAckMessage(player_id));
                m_network_core.SendMessage(ServerLobbyUpdateMessage(player_id, m_lobby_data));
                m_lobby_data.m_players.push_back(PlayerSetupData());
                m_lobby_data.m_players.back().m_player_id = player_id;
                m_lobby_data.m_players.back().m_player_name = player_name;
                m_lobby_data.m_players.back().m_empire_color = EmpireColors().at(0);
                for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, m_lobby_data));
                }
            }
        } else { // immediately join a game that is about to start
            std::set<std::string>::iterator it = m_expected_ai_players.find(player_name);
            if (it != m_expected_ai_players.end()) { // incoming AI player connection
                // let the server network core know what socket this player is on
                if (m_network_core.EstablishPlayer(connection.socket, player_id, player_info)) {
                    m_network_core.SendMessage(JoinAckMessage(player_id));
                    m_expected_ai_players.erase(player_name); // only allow one connection per AI
                    m_ai_IDs.insert(player_id);
                }
            } else { // non-AI player connection
                if (static_cast<int>(m_expected_ai_players.size() + m_network_core.PlayerConnections().size()) < m_expected_players) {
                    if (m_network_core.EstablishPlayer(connection.socket, player_id, player_info)) {
                        m_network_core.SendMessage(JoinAckMessage(player_id));
                    }
                } else {
                    const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
                    m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to join "
                        "the game but there was not enough room.  Terminating connection to " << 
                        (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
                    m_network_core.DumpConnection(connection.socket);
                }
            }

            if (static_cast<int>(m_network_core.PlayerConnections().size()) == m_expected_players) { // if we've gotten all the players joined up
                if (m_player_save_game_data.empty())
                    NewGameInit();
                else
                    LoadGameInit();
                m_state = SERVER_WAITING;
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
            }
        }
        break;
    }

    default: {
        const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
        m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : Received an invalid message type \"" <<
            msg.Type() << "\" for a non-player Message.  Terminating connection to " << 
            (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
        m_network_core.DumpConnection(connection.socket);
        break;
    }
    }
}

void ServerApp::PlayerDisconnected(int id)
{
    // this will not usually happen, since the host process usually owns the server process, and will usually take it down if it fails
    if (id == NetworkCore::HOST_PLAYER_ID) {
        if (m_state == SERVER_MP_LOBBY) { // host disconnected in MP lobby
            for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                if (it->first != id) {
                    m_network_core.SendMessage(ServerLobbyHostAbortMessage(it->first));
                    m_network_core.DumpPlayer(it->first);
                }
            }
            m_network_core.DumpPlayer(id);
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_network_core.DumpAllConnections();
            Exit(1);
        } else if (m_losers.find(id) == m_losers.end()) { // host abnormally disconnected during a regular game
            // if the host dies, there's really nothing else we can do
            for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                if (it->first != id)
                    m_network_core.SendMessage(EndGameMessage(-1, it->first));
            }
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_network_core.DumpAllConnections();
            Exit(1);
        }
    } else {
        if (m_state == SERVER_MP_LOBBY) { // player disconnected in MP lobby
            unsigned int i = 0;
            for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it, ++i) {
                if (it->first == id) {
                    if (i < m_lobby_data.m_players.size())
                        m_lobby_data.m_players.erase(m_lobby_data.m_players.begin() + i); // remove the exiting player's PlayerSetupData struct
                } else {
                    m_network_core.SendMessage(ServerLobbyExitMessage(id, it->first));
                }
            }
            m_network_core.DumpPlayer(id);
        } else if (m_losers.find(id) == m_losers.end()) { // player abnormally disconnected during a regular game
            m_state = SERVER_DISCONNECT;
            const PlayerConnection& disconnected_player_info = m_network_core.PlayerConnections().find(id)->second;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Lost connection to player #" << boost::lexical_cast<std::string>(id) 
                                         << ", named \"" << disconnected_player_info.name << "\"; server now in mode " << SERVER_DISCONNECT << " (SERVER_DISCONNECT).";
            std::string message = disconnected_player_info.name;
            for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
                if (it->first != id) {
                    m_network_core.SendMessage(PlayerDisconnectedMessage(it->first, message));
                    // in the future we may find a way to recover from this, but for now we will immediately send a game ending message as well
                    m_network_core.SendMessage(EndGameMessage(-1, it->first));
                }
            }
        }
    }

    // if there are no humans left, it's time to terminate
    if (m_network_core.PlayerConnections().empty() || m_ai_clients.size() == m_network_core.PlayerConnections().size()) {
        m_state = SERVER_DYING;
        m_log_category.debugStream() << "ServerApp::PlayerDisconnected : All human players disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        m_network_core.DumpAllConnections();
        Exit(1);
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

ServerNetworkCore& ServerApp::NetworkCore()
{
    return ServerApp::GetApp()->m_network_core;
}

void ServerApp::Run()
{
    try {
        SDLInit();
        Initialize();
        while (1)
            Poll();
    } catch (const std::invalid_argument& exception) {
        m_log_category.fatal("std::invalid_argument Exception caught in ServerApp::Run(): " + std::string(exception.what()));
        Exit(1);
    } catch (const std::runtime_error& exception) {
        m_log_category.fatal("std::runtime_error Exception caught in ServerApp::Run(): " + std::string(exception.what()));
        Exit(1);
    }
}

#ifdef FREEORION_LINUX
#  include <iostream> // for informal dummy videodriver message
#endif

void ServerApp::SDLInit()
{
#ifdef FREEORION_LINUX
    // Dirty hack to active the dummy video handler of SDL; if the user has already set SDL_VIDEODRIVER, we'll trust him
    if (getenv("SDL_VIDEODRIVER") == NULL) {
        putenv("SDL_VIDEODRIVER=dummy");
        std::cerr << "NOTE: All warnings about \"using the SDL dummy video driver\" can safely be ignored." << std::endl;
    }
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
        Logger().errorStream() << "SDL initialization failed: " << SDL_GetError();
        Exit(1);
    }

    if (SDLNet_Init() < 0) {
        Logger().errorStream() << "SDL Net initialization failed: " << SDLNet_GetError();
        Exit(1);
    }
  
    if (FE_Init() < 0) {
        Logger().errorStream() << "FastEvents initialization failed: " << FE_GetError();
        Exit(1);
    }

    if (NET2_Init() < 0) {
        Logger().errorStream() << "SDL Net2 initialization failed: " << NET2_GetError();
        Exit(1);
    }
  
    Logger().debugStream() << "SDLInit() complete.";
}

void ServerApp::Initialize()
{
    m_network_core.ListenToPorts();
}

void ServerApp::Poll()
{
    // handle events
    SDL_Event event;
    while (FE_WaitEvent(&event)) {
        int net2_type = NET2_GetEventType(&event);
        if (event.type == SDL_USEREVENT && 
            (net2_type == NET2_ERROREVENT || 
             net2_type == NET2_TCPACCEPTEVENT || 
             net2_type == NET2_TCPRECEIVEEVENT || 
             net2_type == NET2_TCPCLOSEEVENT || 
             net2_type == NET2_UDPRECEIVEEVENT)) { // an SDL_net2 event
            m_network_core.HandleNetEvent(event);
        } else { // some other SDL event
            switch (event.type) {
            case SDL_QUIT:
                Exit(0);
                break;
            }
        }
    }
}

void ServerApp::FinalCleanup()
{
    NetworkCore().DumpAllConnections();
    for (unsigned int i = 0; i < m_ai_clients.size(); ++i)
        m_ai_clients[i].Kill();
}

void ServerApp::SDLQuit()
{
    FinalCleanup();
    NET2_Quit();
    FE_Quit();
    SDLNet_Quit();
    SDL_Quit();
    Logger().debugStream() << "SDLQuit() complete.";
}

void ServerApp::NewGameInit()
{
    m_current_turn = BEFORE_FIRST_TURN;     // every UniverseObject created before game starts will have m_created_on_turn BEFORE_FIRST_TURN
    m_universe.CreateUniverse(m_galaxy_size, m_galaxy_shape, m_galaxy_age, m_starlane_freq, m_planet_density, m_specials_freq, 
                              m_network_core.PlayerConnections().size() - m_ai_clients.size(), m_ai_clients.size(), m_lobby_data.m_players);
    m_current_turn = 1;                     // after all game initialization stuff has been created, can set current turn to 1 for start of game
    m_log_category.debugStream() << "ServerApp::GameInit : Created universe " << " (SERVER_GAME_SETUP).";

    // add empires to turn sequence map according to spec this should be done randomly for now it's not
    for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
        AddEmpireTurn(it->first);
    }

    // compile map of PlayerInfo for each player, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) {
        std::string name = player_it->second.name;
        bool host = player_it->second.host;
        bool AI = (m_ai_IDs.find(player_it->first) != m_ai_IDs.end());
        int empire_id = GetPlayerEmpire(player_it->first)->EmpireID();
        players.insert(std::pair<int, PlayerInfo>(player_it->first, PlayerInfo(name, empire_id, AI, host)));
    }
    // send initial gamestate and players information to players
    for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
        m_network_core.SendMessage(GameStartMessage(it->first, m_single_player_game, it->first, m_current_turn, m_empires, m_universe, players));
    }

    m_losers.clear();
}

void ServerApp::LoadGameInit()
{
    assert(!m_player_save_game_data.empty());

    m_turn_sequence.clear();

    std::map<int, PlayerSaveGameData> player_data_by_empire;
    for (unsigned int i = 0; i < m_player_save_game_data.size(); ++i) {
        assert(m_player_save_game_data[i].m_empire);
        player_data_by_empire[m_player_save_game_data[i].m_empire->EmpireID()] = m_player_save_game_data[i];
    }

    if (m_single_player_game) {
        while (m_lobby_data.m_players.size() < m_network_core.PlayerConnections().size()) {
            m_lobby_data.m_players.push_back(PlayerSetupData());
        }
    }

    std::map<int, int> player_to_empire_ids;
    std::set<int> already_chosen_empire_ids;
    unsigned int i = 0;
    for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it, ++i) {
        player_to_empire_ids[it->first] = m_lobby_data.m_players[i].m_save_game_empire_id;
        already_chosen_empire_ids.insert(m_lobby_data.m_players[i].m_save_game_empire_id);
    }

    for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
        const int INVALID_EMPIRE_ID = -5000;
        int empire_id = INVALID_EMPIRE_ID;
        if (player_to_empire_ids[it->first] != -1) {
            empire_id = player_to_empire_ids[it->first];
        } else {
            for (std::map<int, PlayerSaveGameData>::iterator player_data_it = player_data_by_empire.begin(); player_data_it != player_data_by_empire.end(); ++player_data_it) {
                if (already_chosen_empire_ids.find(player_data_it->first) == already_chosen_empire_ids.end()) {
                    empire_id = player_data_it->first;
                    already_chosen_empire_ids.insert(empire_id);
                    player_to_empire_ids[it->first] = empire_id;
                    // since this must be an AI player, it does not have the correct player name set in its Empire yet, so we need to do so now
                    player_data_it->second.m_empire->SetPlayerName(it->second.name);
                    Empires().InsertEmpire(player_data_it->second.m_empire);
                    break;
                }
            }
        }
        assert(empire_id != INVALID_EMPIRE_ID);
        m_turn_sequence[empire_id] = 0;
    }

    // This is a bit odd, but since Empires() is built from the data stored in m_player_save_game_data, and the universe
    // is loaded long before that, the universe's empire-specific views of the systems is not properly initialized when
    // the universe is loaded.  That means we must do it here.
    m_universe.RebuildEmpireViewSystemGraphs();

    // compile map of PlayerInfo for each player, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) {
        std::string name = player_it->second.name;
        bool host = player_it->second.host;
        bool AI = (m_ai_IDs.find(player_it->first) != m_ai_IDs.end());
        int empire_id = GetPlayerEmpire(player_it->first)->EmpireID();
        players.insert(std::pair<int, PlayerInfo>(player_it->first, PlayerInfo(name, empire_id, AI, host)));
    }
    // send loaded gamestate and players info to players
    for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin(); it != m_network_core.PlayerConnections().end(); ++it) {
        int empire_id = player_to_empire_ids[it->first];
        m_network_core.SendMessage(GameStartMessage(it->first, m_single_player_game, empire_id, m_current_turn, m_empires, m_universe, players));
        m_network_core.SendMessage(ServerLoadGameMessage(it->first, *player_data_by_empire[empire_id].m_orders, player_data_by_empire[empire_id].m_ui_data.get()));
    }

    m_losers.clear();
}

Empire* ServerApp::GetPlayerEmpire(int player_id) const
{
    Empire* retval = 0;
    std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().find(player_id);
    if (player_it != m_network_core.PlayerConnections().end()) {
        std::string player_name = player_it->second.name;
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
    for (std::map<int, PlayerConnection>::const_iterator it = m_network_core.PlayerConnections().begin();
         it != m_network_core.PlayerConnections().end();
         ++it) {
        if (it->second.name == player_name) {
            retval = it->first;
            break;
        }
    }
    return retval;
}

void ServerApp::AddEmpireTurn(int empire_id)
{
    // add empire
    m_turn_sequence[empire_id] = NULL;
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
        for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin();
             player_it != m_network_core.PlayerConnections().end();
             ++player_it) {
            m_network_core.SendMessage(TurnProgressMessage(player_it->first, Message::PROCESSING_ORDERS, it->first));
        }

        pEmpire = Empires().Lookup( it->first );
        pEmpire->ClearSitRep( );
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
        if(it->second.size()==1) {
            it->second[0]->ServerExecute();
            pEmpire = Empires().Lookup( it->second[0]->EmpireID() );
            pEmpire->AddSitRepEntry(CreatePlanetColonizedSitRep(planet->SystemID(), planet->ID()));
        }
        else
        {
            const System *system = GetUniverse().Object<System>(planet->SystemID());

            std::vector<const Fleet*> vec_fleet = system->FindObjects<Fleet>();
            std::set<int> set_empire_with_military;
            for(unsigned int i=0;i<vec_fleet.size();i++)
                for(Fleet::const_iterator ship_it=vec_fleet[i]->begin();ship_it!=vec_fleet[i]->end();++ship_it)
                    if(GetUniverse().Object<Ship>(*ship_it)->IsArmed())
                    {
                        set_empire_with_military.insert(*vec_fleet[i]->Owners().begin());
                        break;
                    }

            // set the first empire as winner for now
            int winner = 0;
            // is the current winner armed?
            bool winner_is_armed = set_empire_with_military.find(it->second[0]->EmpireID()) != set_empire_with_military.end();
            for(unsigned int i=1;i<it->second.size();i++)
                // is this empire armed?
                if(set_empire_with_military.find(it->second[i]->EmpireID()) != set_empire_with_military.end())
                {
                    // if this empire is armed and the former winner too, noone can win
                    if(winner_is_armed)
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

            for(int i=0;i<static_cast<int>(it->second.size());i++)
                if(winner==i) {
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
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage(player_it->first, Message::FLEET_MOVEMENT, -1));
        
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

    // check for combats, and resolve them.
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage(player_it->first, Message::COMBAT, -1));

    std::vector<System*> sys_vec = GetUniverse().FindObjects<System>();
    bool combat_happend = false;
    for(std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it)
    {
        std::vector<CombatAssets> empire_combat_forces;
        System* system = *it;
      
        std::vector<Fleet*> flt_vec = system->FindObjects<Fleet>();
        if (flt_vec.empty()) continue;  // skip systems with not fleets, as these can't have combat

        for(std::vector<Fleet*>::iterator flt_it = flt_vec.begin();flt_it != flt_vec.end(); ++flt_it)
        {
            Fleet* flt = *flt_it;
            // a fleet should belong only to one empire!?
            if(1==flt->Owners().size())
            {
                std::vector<CombatAssets>::iterator ecf_it = std::find(empire_combat_forces.begin(),empire_combat_forces.end(),CombatAssetsOwner(Empires().Lookup(*flt->Owners().begin())));

                if(ecf_it==empire_combat_forces.end())
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
        for(std::vector<Planet*>::iterator plt_it = plt_vec.begin();plt_it != plt_vec.end(); ++plt_it)
        {
            Planet* plt = *plt_it;
            // a planet should belong only to one empire!?
            if(1==plt->Owners().size())
            {           
                std::vector<CombatAssets>::iterator ecf_it = std::find(empire_combat_forces.begin(),empire_combat_forces.end(),CombatAssetsOwner(Empires().Lookup(*plt->Owners().begin())));

                if(ecf_it==empire_combat_forces.end())
                {
                    CombatAssets ca(Empires().Lookup(*plt->Owners().begin()));
                    ca.planets.push_back(plt);
                    empire_combat_forces.push_back(ca);
                }
                else
                    (*ecf_it).planets.push_back(plt);
            }
        }

        if(empire_combat_forces.size()>1)
        {
            combat_happend=true;
            CombatSystem combat_system;
            combat_system.ResolveCombat(system->ID(),empire_combat_forces);
        }
    }

    // if a combat happened, give the human user a chance to look at the results
    if (combat_happend)
        SDL_Delay(1500);


    // inform players that production processing is starting...
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage(player_it->first, Message::EMPIRE_PRODUCTION, -1));


    // Update meters, do other effects stuff
    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it) {
        it->second->ResetMaxMeters();   // zero all meters
        it->second->AdjustMaxMeters();  // apply non-effects max meter modifications, including focus mods
    }
    GetUniverse().ApplyEffects();       // apply effects, futher altering meters (and also non-meter effects)


    // Determine how much of each resource is available, and determine how to distribute it to planets or on queues
    for (EmpireManager::iterator it = Empires().begin(); it != Empires().end(); ++it)
        it->second->UpdateResourcePool();

    // consume distributed resources on queues
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        Empire* empire = Empires().Lookup(it->first);
        empire->CheckResearchProgress();
        empire->CheckProductionProgress();
        empire->CheckTradeSocialProgress();
        empire->CheckGrowthFoodProgress();
    }


    // regenerate empire system visibility, which is needed for some UniverseObject subclasses' PopGrowthProductionResearchPhase()
    GetUniverse().RebuildEmpireViewSystemGraphs();


    // Population growth or loss, health meter growth, resource current meter growth
    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it) {
        it->second->PopGrowthProductionResearchPhase();
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

    
    // loop and free all orders
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }   

    ++m_current_turn;

    // indicate that the clients are waiting for their new Universes
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage(player_it->first, Message::DOWNLOADING, -1));

    // check if all empires are still alive
    std::map<int, int> eliminations; // map from player ids to empire ids
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        if (GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(it->first)).empty()) { // when you're out of planets, your game is over
            std::string player_name = it->second->PlayerName();
            for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); 
                 player_it != m_network_core.PlayerConnections().end(); ++player_it) {
                if (player_it->second.name == player_name) {
                    // record this player/empire so we can send out messages about it
                    eliminations[player_it->first] = it->first;
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

    // compile map of PlayerInfo for each player, indexed by player ID
    std::map<int, PlayerInfo> players;
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) {
        std::string name = player_it->second.name;
        bool host = player_it->second.host;
        bool AI = (m_ai_IDs.find(player_it->first) != m_ai_IDs.end());
        int empire_id = GetPlayerEmpire(player_it->first)->EmpireID();
        players.insert(std::pair<int, PlayerInfo>(player_it->first, PlayerInfo(name, empire_id, AI, host)));
    }
    // send new-turn updates to all players
    for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) {
        int empire_id = GetPlayerEmpire(player_it->first)->EmpireID();
        m_network_core.SendMessage(TurnUpdateMessage(player_it->first, empire_id, m_current_turn, m_empires, m_universe, players));
    }

    // notify all players of the eliminated players
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        for (std::map<int, PlayerConnection>::const_iterator player_it = m_network_core.PlayerConnections().begin(); player_it != m_network_core.PlayerConnections().end(); ++player_it) {
            m_network_core.SendMessage(PlayerEliminatedMessage(player_it->first, Empires().Lookup(it->second)->Name()));
        }
    }

    // dump connections to eliminated players, and remove server-side empire data
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        m_log_category.debugStream() << "ServerApp::ProcessTurns : Player " << it->first << " is marked as a loser and dumped";
        m_losers.insert(it->first);
        m_network_core.DumpPlayer(it->first);
        m_ai_IDs.erase(it->first);
        Empires().EliminateEmpire(it->second);
        RemoveEmpireTurn(it->second);
    }

    // determine if victory conditions exist
    if (m_network_core.PlayerConnections().size() == 1) { // if there is only one player left, that player is the winner
        m_log_category.debugStream() << "ServerApp::ProcessTurns : One player left -- sending victory notification and terminating.";
        while (m_network_core.PlayerConnections().size() == 1) {
            m_network_core.SendMessage(VictoryMessage(m_network_core.PlayerConnections().begin()->first));
            SDL_Delay(100);
        }
        m_network_core.DumpAllConnections();
        Exit(0);
    } else if (m_ai_IDs.size() == m_network_core.PlayerConnections().size()) { // if there are none but AI players left, we're done
        m_log_category.debugStream() << "ServerApp::ProcessTurns : No human players left -- server terminating.";
        m_network_core.DumpAllConnections();
        Exit(0);
    }
}
