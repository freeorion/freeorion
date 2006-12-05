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
    m_networking(m_io_service,
                 boost::bind(&ServerApp::HandleMessage, this, _1, _2),
                 boost::bind(&ServerApp::HandleNonPlayerMessage, this, _1, _2),
                 boost::bind(&ServerApp::PlayerDisconnected, this, _1)),
    m_log_category(log4cpp::Category::getRoot()),
    m_state(SERVER_IDLE),
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
        m_networking.Disconnect(*it);
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

void ServerApp::HandleMessage(const Message& msg, PlayerConnectionPtr player_connection)
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
            m_expected_players = m_networking.size() + m_lobby_data.m_AIs.size();
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                if ((*it)->ID() != msg.SendingPlayer())
                    (*it)->SendMessage(Message(Message::GAME_START, -1, (*it)->ID(), Message::CLIENT_LOBBY_MODULE, ""));
            }
            m_state = SERVER_GAME_SETUP;
            CreateAIClients(m_lobby_data.m_AIs);
            m_player_save_game_data.clear();
            m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
            if (m_expected_players == static_cast<int>(m_networking.size())) {
                NewGameInit();
                m_state = SERVER_WAITING;
                m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
            }
        } else { // load game
            ServerNetworking::const_iterator sender_it = m_networking.GetPlayer(msg.SendingPlayer());
            if (sender_it != m_networking.end() && (*sender_it)->Host()) {
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
                            ServerNetworking::const_iterator player_it = m_networking.begin();
                            std::advance(player_it, j);  // TODO: This is probably broken now
                            m_player_save_game_data[i].m_name = (*player_it)->PlayerName();
                            m_player_save_game_data[i].m_empire->SetPlayerName((*player_it)->PlayerName());
                        }
                    }
                    m_empires.InsertEmpire(m_player_save_game_data[i].m_empire);
                }

                for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                    if (it != sender_it)
                        (*it)->SendMessage(Message(Message::GAME_START, -1, (*it)->ID(), Message::CLIENT_LOBBY_MODULE, ""));
                }

                int AI_clients = m_expected_players - m_networking.size();
                CreateAIClients(std::vector<PlayerSetupData>(AI_clients));
                m_state = SERVER_GAME_SETUP;

                if (!AI_clients)
                    LoadGameInit();
            } else {
                m_log_category.errorStream() << "Player #" << msg.SendingPlayer() << " attempted to initiate a game load, but is not the host, or is "
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

        for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
            if ((*it)->ID() != msg.SendingPlayer() || new_save_file_selected)
                (*it)->SendMessage(ServerLobbyUpdateMessage((*it)->ID(), m_lobby_data));
        }
        break;
    }

    case Message::LOBBY_CHAT: {
        if (msg.ReceivingPlayer() == -1) { // the receiver is everyone (except the sender)
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                if ((*it)->ID() != msg.SendingPlayer())
                    (*it)->SendMessage(ServerLobbyChatMessage(msg.SendingPlayer(), (*it)->ID(), msg.Text()));
            }
        } else {
            m_networking.SendMessage(ServerLobbyChatMessage(msg.SendingPlayer(), msg.ReceivingPlayer(), msg.Text()));
        }
        break;
    }

    case Message::LOBBY_HOST_ABORT: {
        for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
            if ((*it)->ID() != msg.SendingPlayer()) {
                (*it)->SendMessage(ServerLobbyHostAbortMessage((*it)->ID()));
                m_networking.Disconnect((*it)->ID());
            }
        }
        break;
    }

    case Message::LOBBY_EXIT: {
        unsigned int i = 0;
        for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it, ++i) {
            if ((*it)->ID() == msg.SendingPlayer()) {
                if (i < m_lobby_data.m_players.size())
                    m_lobby_data.m_players.erase(m_lobby_data.m_players.begin() + i); // remove the exiting player's PlayerSetupData struct
            } else {
                (*it)->SendMessage(ServerLobbyExitMessage(msg.SendingPlayer(), (*it)->ID()));
            }
        }
        m_networking.Disconnect(msg.SendingPlayer());
        break;
    }

    case Message::SAVE_GAME: {
        if (m_networking.GetPlayer(msg.SendingPlayer()) != m_networking.end()) {
            // send out all save game data requests
            std::set<int> needed_reponses;
            m_players_responded.clear();
            m_player_save_game_data.clear();
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                (*it)->SendMessage(ServerSaveGameMessage((*it)->ID()));
                needed_reponses.insert((*it)->ID());
            }

#if 0 // TODO: Block or something here, or just have the handler for the incoming events call the code that follows when m_players_responded == needed_reponses
            // wait for them all to come in
            SDL_Event ev;
            const unsigned int SYCHRONOUS_TIMEOUT = 15000; // give up after this many ms without any valid responses
            unsigned int start_time = SDL_GetTicks();
            while (1) {
                unsigned int starting_responses = m_players_responded.size();
                FE_PollEvent(&ev);
                if (ev.type == SDL_USEREVENT) {
                    int net2_type = NET2_GetEventType(&ev);
                    if (net2_type == NET2_ERROREVENT || 
                        net2_type == NET2_TCPACCEPTEVENT || 
                        net2_type == NET2_TCPRECEIVEEVENT || 
                        net2_type == NET2_TCPCLOSEEVENT || 
                        net2_type == NET2_UDPRECEIVEEVENT) {
                        m_networking.HandleNetEvent(ev);
                    }
                }
                if (starting_responses < m_players_responded.size())
                    start_time = SDL_GetTicks(); // reset timeout whenever there's a valid response
                if (m_players_responded == needed_reponses || SYCHRONOUS_TIMEOUT < SDL_GetTicks() - start_time)
                    break;
            }
#endif
            if (m_players_responded == needed_reponses) {
                SaveGame(msg.Text(), m_current_turn, m_player_save_game_data, m_universe);
                m_networking.SendMessage(ServerSaveGameMessage(msg.SendingPlayer(), true));
            }
        } else {
            m_log_category.errorStream() << "Player #" << msg.SendingPlayer() << " attempted to initiate a game save, but is not found in the player list.";
        }
        break;
    }

    case Message::LOAD_GAME: { // single-player loading (multiplayer loading is handled through the lobby interface)
        ServerNetworking::const_iterator sender_it = m_networking.GetPlayer(msg.SendingPlayer());
        if (sender_it != m_networking.end() && (*sender_it)->Host()) {
            m_empires.RemoveAllEmpires();
            m_single_player_game = true;
            LoadGame(msg.Text(), m_current_turn, m_player_save_game_data, m_universe);
            m_expected_players = m_player_save_game_data.size();
            CreateAIClients(std::vector<PlayerSetupData>(m_expected_players - 1));
            m_state = SERVER_GAME_SETUP;
        } else {
            m_log_category.errorStream() << "Player #" << msg.SendingPlayer() << " attempted to initiate a game save, but is not the host, or is "
                "not found in the player list.";
        }
        break;
    }

    case Message::TURN_ORDERS: {
        OrderSet* order_set = new OrderSet;
        ExtractMessageData(msg, *order_set);

        // check order validity -- all orders must originate from this empire in order to be considered valid
        Empire* empire = GetPlayerEmpire(msg.SendingPlayer());
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

        m_networking.SendMessage(TurnProgressMessage(msg.SendingPlayer(), Message::WAITING_FOR_PLAYERS, -1));

        m_log_category.debugStream() << "ServerApp::HandleMessage : Received orders from player " << msg.SendingPlayer();

        /* if all orders are received already, do nothing as we are processing a turn */
        if (AllOrdersReceived())
            break;

        /* add orders to turn sequence */    
        SetEmpireTurnOrders(GetPlayerEmpire(msg.SendingPlayer())->EmpireID(), order_set);

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
        ServerNetworking::const_iterator player_it = m_networking.GetPlayer(msg.SendingPlayer());
        assert(player_it != m_networking.end());
        m_player_save_game_data.push_back(PlayerSaveGameData((*player_it)->PlayerName(), GetPlayerEmpire(msg.SendingPlayer()), order_set, ui_data));
        m_players_responded.insert(msg.SendingPlayer());
        break;
    }

    case Message::HUMAN_PLAYER_MSG: {
        std::string text = msg.Text();

        // if there's a colon in the message, treat all tokens before the colon as player names.
        // if there are tokens before the colon, but at least one of them *is not* a valid player names, assume there has been a typo,
        // and don't send the message at all, since we can't decipher which parts are message and which parts are names
        std::string::size_type colon_position = text.find(':');
        // target_player_names.empty() implies that all players should be sent the message; otherwise, only the indicated players will receive the message
        std::set<std::string> target_player_names;
        if (colon_position != std::string::npos) {
            std::vector<std::string> tokens = Tokenize(text.substr(0, colon_position));
            for (unsigned int i = 0; i < tokens.size(); ++i) {
                bool token_is_name = false;
                for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                    if (tokens[i] == (*it)->PlayerName()) {
                        token_is_name = true;
                        break;
                    }
                }
                if (token_is_name)
                    target_player_names.insert(tokens[i]);
                else
                    return;
            }
        }
        if (!target_player_names.empty()) {
            text = text.substr(colon_position + 1);
            if (text == "")
                return;
        }
        Empire* sender_empire = GetPlayerEmpire(msg.SendingPlayer());
        std::string final_text = RgbaTag(Empires().Lookup(sender_empire->EmpireID())->Color()) + (*m_networking.GetPlayer(msg.SendingPlayer()))->PlayerName() +
            (target_player_names.empty() ? ": " : " (whisper):") + text + "</rgba>\n";
        for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
            if (target_player_names.empty() || target_player_names.find((*it)->PlayerName()) != target_player_names.end())
                (*it)->SendMessage(ChatMessage(msg.SendingPlayer(), (*it)->ID(), final_text));
        }
        break;
    }

    case Message::REQUEST_NEW_OBJECT_ID: {
        /* get get ID and send back to client, it's waiting for this */
        m_networking.SendMessage(DispatchObjectIDMessage(msg.SendingPlayer(), GetUniverse().GenerateObjectID()));
        break;
    }

    case Message::END_GAME: {
        ServerNetworking::const_iterator it = m_networking.GetPlayer(msg.SendingPlayer());
        if (it != m_networking.end() && (*it)->Host()) {
            for (ServerNetworking::const_iterator it2 = m_networking.begin(); it2 != m_networking.end(); ++it2) {
                if ((*it)->ID() != (*it2)->ID())
                    (*it2)->SendMessage(EndGameMessage(-1, (*it2)->ID()));
            }
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_networking.SendMessage(Message(Message::SERVER_STATUS, -1, msg.SendingPlayer(), Message::CORE, boost::lexical_cast<std::string>(m_state)));
            m_networking.DisconnectAll();
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

void ServerApp::HandleNonPlayerMessage(const Message& msg, PlayerConnectionPtr player_connection)
{
    switch (msg.Type()) {
    case Message::HOST_SP_GAME: {
        if (m_networking.empty() && m_expected_ai_players.empty()) {
            SinglePlayerSetupData setup_data;
            ExtractMessageData(msg, setup_data);

            int player_id = Networking::HOST_PLAYER_ID;

            // immediately start a new game with the given parameters
            m_single_player_game = true;
            m_expected_players = setup_data.m_AIs + 1;
            m_galaxy_size = setup_data.m_size;
            m_galaxy_shape = setup_data.m_shape;
            m_galaxy_age = setup_data.m_age;
            m_starlane_freq = setup_data.m_starlane_freq;
            m_planet_density = setup_data.m_planet_density;
            m_specials_freq = setup_data.m_specials_freq;
            CreateAIClients(std::vector<PlayerSetupData>(setup_data.m_AIs));
            m_player_save_game_data.clear();
            m_lobby_data.m_players.clear();
            m_lobby_data.m_players.push_back(PlayerSetupData());
            m_lobby_data.m_players.back().m_player_id = player_id;
            m_lobby_data.m_players.back().m_player_name = setup_data.m_host_player_name;
            m_lobby_data.m_players.back().m_empire_name = setup_data.m_empire_name;
            m_lobby_data.m_players.back().m_empire_color = setup_data.m_empire_color;
            m_state = SERVER_GAME_SETUP;
            player_connection->EstablishPlayer(player_id, setup_data.m_host_player_name, true);
            player_connection->SendMessage(HostSPAckMessage(player_id));
            player_connection->SendMessage(JoinAckMessage(player_id));
            m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
            m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Universe size set to " << m_galaxy_size << " systems (SERVER_GAME_SETUP).";
            m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Universe shape set to " << m_galaxy_shape << " (SERVER_GAME_SETUP).";
        } else {
            m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to host "
                "a new game but there was already one in progress or one being setup.  Terminating connection.";
            m_networking.Disconnect(player_connection);
        }
        break;
    }

    case Message::HOST_MP_GAME: {
        if (m_networking.empty() && m_expected_ai_players.empty()) {
            std::string host_player_name = msg.Text();
            int player_id = Networking::HOST_PLAYER_ID;

            // start an MP lobby situation so that game settings can be established
            m_single_player_game = false;
            m_state = SERVER_MP_LOBBY;
            m_lobby_data = MultiplayerLobbyData(true);
            m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_MP_LOBBY << " (SERVER_MP_LOBBY).";
            player_connection->EstablishPlayer(player_id, host_player_name, true);
            player_connection->SendMessage(HostMPAckMessage(player_id));
            player_connection->SendMessage(JoinAckMessage(player_id));
            m_lobby_data.m_players.push_back(PlayerSetupData());
            m_lobby_data.m_players.back().m_player_id = player_id;
            m_lobby_data.m_players.back().m_player_name = host_player_name;
            m_lobby_data.m_players.back().m_empire_color = EmpireColors().at(0);
            player_connection->SendMessage(ServerLobbyUpdateMessage(player_id, m_lobby_data));
        } else {
            m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to host "
                "a new game but there was already one in progress or one being setup.  Terminating connection.";
            m_networking.Disconnect(player_connection);
        }
        break;
    }

    case Message::JOIN_GAME: {
        std::string player_name = msg.Text();

        int player_id = (*m_networking.rbegin())->ID() + 1;

        if (m_state == SERVER_MP_LOBBY) { // enter an MP lobby
            player_connection->EstablishPlayer(player_id, player_name, false);
            player_connection->SendMessage(JoinAckMessage(player_id));
            player_connection->SendMessage(ServerLobbyUpdateMessage(player_id, m_lobby_data));
            m_lobby_data.m_players.push_back(PlayerSetupData());
            m_lobby_data.m_players.back().m_player_id = player_id;
            m_lobby_data.m_players.back().m_player_name = player_name;
            m_lobby_data.m_players.back().m_empire_color = EmpireColors().at(0);
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                (*it)->SendMessage(ServerLobbyUpdateMessage((*it)->ID(), m_lobby_data));
            }
        } else { // immediately join a game that is about to start
            std::set<std::string>::iterator it = m_expected_ai_players.find(player_name);
            if (it != m_expected_ai_players.end()) { // incoming AI player connection
                // let the networking system know what socket this player is on
                player_connection->EstablishPlayer(player_id, player_name, false);
                player_connection->SendMessage(JoinAckMessage(player_id));
                m_expected_ai_players.erase(player_name); // only allow one connection per AI
                m_ai_IDs.insert(player_id);
            } else { // non-AI player connection
                if (static_cast<int>(m_expected_ai_players.size() + m_networking.size()) < m_expected_players) {
                    player_connection->EstablishPlayer(player_id, player_name, false);
                    player_connection->SendMessage(JoinAckMessage(player_id));
                } else {
                    m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to join "
                        "the game but there was not enough room.  Terminating connection.";
                    m_networking.Disconnect(player_connection);
                }
            }

            if (static_cast<int>(m_networking.size()) == m_expected_players) { // if we've gotten all the players joined up
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
        m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : Received an invalid message type \"" <<
            msg.Type() << "\" for a non-player Message.  Terminating connection.";
        m_networking.Disconnect(player_connection);
        break;
    }
    }
}

void ServerApp::PlayerDisconnected(PlayerConnectionPtr player_connection)
{
    int id = player_connection->ID();
    // this will not usually happen, since the host process usually owns the server process, and will usually take it down if it fails
    if (id == Networking::HOST_PLAYER_ID) {
        if (m_state == SERVER_MP_LOBBY) { // host disconnected in MP lobby
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                if ((*it)->ID() != id) {
                    (*it)->SendMessage(ServerLobbyHostAbortMessage((*it)->ID()));
                    m_networking.Disconnect((*it)->ID());
                }
            }
            m_networking.Disconnect(id);
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_networking.DisconnectAll();
            Exit(1);
        } else if (m_losers.find(id) == m_losers.end()) { // host abnormally disconnected during a regular game
            // if the host dies, there's really nothing else we can do
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                if ((*it)->ID() != id)
                    (*it)->SendMessage(EndGameMessage(-1, (*it)->ID()));
            }
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_networking.DisconnectAll();
            Exit(1);
        }
    } else {
        if (m_state == SERVER_MP_LOBBY) { // player disconnected in MP lobby
            unsigned int i = 0;
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it, ++i) {
                if ((*it)->ID() == id) {
                    if (i < m_lobby_data.m_players.size())
                        m_lobby_data.m_players.erase(m_lobby_data.m_players.begin() + i); // remove the exiting player's PlayerSetupData struct
                } else {
                    (*it)->SendMessage(ServerLobbyExitMessage(id, (*it)->ID()));
                }
            }
            m_networking.Disconnect(id);
        } else if (m_losers.find(id) == m_losers.end()) { // player abnormally disconnected during a regular game
            m_state = SERVER_DISCONNECT;
            PlayerConnectionPtr disconnected_player = *m_networking.GetPlayer(id);
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Lost connection to player #" << boost::lexical_cast<std::string>(id) 
                                         << ", named \"" << disconnected_player->PlayerName() << "\"; server now in mode " << SERVER_DISCONNECT << " (SERVER_DISCONNECT).";
            std::string message = disconnected_player->PlayerName();
            for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
                if ((*it)->ID() != id) {
                    (*it)->SendMessage(PlayerDisconnectedMessage((*it)->ID(), message));
                    // in the future we may find a way to recover from this, but for now we will immediately send a game ending message as well
                    (*it)->SendMessage(EndGameMessage(-1, (*it)->ID()));
                }
            }
        }
    }

    // if there are no humans left, it's time to terminate
    if (m_networking.empty() || m_ai_clients.size() == m_networking.size()) {
        m_state = SERVER_DYING;
        m_log_category.debugStream() << "ServerApp::PlayerDisconnected : All human players disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        m_networking.DisconnectAll();
        Exit(1);
    }
}

void ServerApp::NewGameInit()
{
    m_current_turn = BEFORE_FIRST_TURN;     // every UniverseObject created before game starts will have m_created_on_turn BEFORE_FIRST_TURN
    m_universe.CreateUniverse(m_galaxy_size, m_galaxy_shape, m_galaxy_age, m_starlane_freq, m_planet_density, m_specials_freq, 
                              m_networking.size() - m_ai_clients.size(), m_ai_clients.size(), m_lobby_data.m_players);
    m_current_turn = 1;                     // after all game initialization stuff has been created, can set current turn to 1 for start of game
    m_log_category.debugStream() << "ServerApp::GameInit : Created universe " << " (SERVER_GAME_SETUP).";

    // add empires to turn sequence map according to spec this should be done randomly for now it's not
    for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
        AddEmpireTurn((*it)->ID());
    }

    // the universe creation caused the creation of empires.  But now we need to assign the empires to players.
    for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
        (*it)->SendMessage(GameStartMessage((*it)->ID(), m_single_player_game, (*it)->ID(), m_current_turn, m_empires, m_universe));
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
        while (m_lobby_data.m_players.size() < m_networking.size()) {
            m_lobby_data.m_players.push_back(PlayerSetupData());
        }
    }

    std::map<int, int> player_to_empire_ids;
    std::set<int> already_chosen_empire_ids;
    unsigned int i = 0;
    for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it, ++i) {
        player_to_empire_ids[(*it)->ID()] = m_lobby_data.m_players[i].m_save_game_empire_id;
        already_chosen_empire_ids.insert(m_lobby_data.m_players[i].m_save_game_empire_id);
    }

    for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
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

    // This is a bit odd, but since Empires() is built from the data stored in m_player_save_game_data, and the universe
    // is loaded long before that, the universe's empire-specific views of the systems is not properly initialized when
    // the universe is loaded.  That means we must do it here.
    m_universe.RebuildEmpireViewSystemGraphs();

    for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
        int empire_id = player_to_empire_ids[(*it)->ID()];
        (*it)->SendMessage(GameStartMessage((*it)->ID(), m_single_player_game, empire_id, m_current_turn, m_empires, m_universe));
        (*it)->SendMessage(ServerLoadGameMessage((*it)->ID(), *player_data_by_empire[empire_id].m_orders, player_data_by_empire[empire_id].m_ui_data.get()));
    }

    m_losers.clear();
}

Empire* ServerApp::GetPlayerEmpire(int player_id) const
{
    Empire* retval = 0;
    ServerNetworking::const_iterator player_it = m_networking.GetPlayer(player_id);
    if (player_it != m_networking.end()) {
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
    for (ServerNetworking::const_iterator it = m_networking.begin(); it != m_networking.end(); ++it) {
        if ((*it)->PlayerName() == player_name) {
            retval = (*it)->ID();
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
        for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
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
    for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
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
    for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::COMBAT, -1));
    }

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
        SDL_Delay(1500); // TODO: Put this delay client-side.

    // process production and growth phase
    for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
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
    for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
        (*player_it)->SendMessage(TurnProgressMessage((*player_it)->ID(), Message::DOWNLOADING, -1));
    }

    // check if all empires are still alive
    std::map<int, int> eliminations; // map from player ids to empire ids
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        if (GetUniverse().FindObjects(OwnedVisitor<UniverseObject>(it->first)).empty()) { // when you're out of planets, your game is over
            std::string player_name = it->second->PlayerName();
            for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
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
    for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
        pEmpire = GetPlayerEmpire((*player_it)->ID());
        (*player_it)->SendMessage(TurnUpdateMessage((*player_it)->ID(), pEmpire->EmpireID(), m_current_turn, m_empires, m_universe));
    }

    // notify all players of the eliminated players
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        for (ServerNetworking::const_iterator player_it = m_networking.begin(); player_it != m_networking.end(); ++player_it) {
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
    if (m_networking.size() == 1) { // if there is only one player left, that player is the winner
        m_log_category.debugStream() << "ServerApp::ProcessTurns : One player left -- sending victory notification and terminating.";
        while (m_networking.size() == 1) {
            m_networking.SendMessage(VictoryMessage((*m_networking.begin())->ID()));
            SDL_Delay(100); // TODO: It should be possible to eliminate this by using linger.
        }
        m_networking.DisconnectAll();
        Exit(0);
    } else if (m_ai_IDs.size() == m_networking.size()) { // if there are none but AI players left, we're done
        m_log_category.debugStream() << "ServerApp::ProcessTurns : No human players left -- server terminating.";
        m_networking.DisconnectAll();
        Exit(0);
    }
}
