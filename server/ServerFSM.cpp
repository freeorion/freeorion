#include "ServerFSM.h"

#include "SaveLoad.h"
#include "ServerApp.h"
#include "../Empire/Empire.h"
#include "../network/ServerNetworking.h"
#include "../network/Message.h"
#include "../util/Directories.h"
#include "../util/OrderSet.h"

#include <GG/Font.h>

#include <iostream>


namespace {
    void RebuildSaveGameEmpireData(std::vector<SaveGameEmpireData>& save_game_empire_data, const std::string& save_game_filename)
    {
        save_game_empire_data.clear();
        std::vector<PlayerSaveGameData> player_save_game_data;
        int current_turn;
        Universe universe;
        LoadGame((GetLocalDir() / "save" / save_game_filename).native_file_string().c_str(),
                 current_turn, player_save_game_data, universe);
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

////////////////////////////////////////////////////////////
// MessageEventBase
////////////////////////////////////////////////////////////
Disconnection::Disconnection(PlayerConnectionPtr& player_connection) :
    m_player_connection(player_connection)
{}

////////////////////////////////////////////////////////////
// MessageEventBase
////////////////////////////////////////////////////////////
MessageEventBase::MessageEventBase(const Message& message, PlayerConnectionPtr& player_connection) :
    m_message(message),
    m_player_connection(player_connection)
{}


////////////////////////////////////////////////////////////
// ServerFSM
////////////////////////////////////////////////////////////
ServerFSM::ServerFSM(ServerApp &server) :
    m_server(server)
{}

void ServerFSM::unconsumed_event(const boost::statechart::event_base &event)
{ std::cout << "Invalid Event! TODO: disconnect offender" << std::endl; }

ServerApp& ServerFSM::Server()
{ return m_server; }

void ServerFSM::HandleNonLobbyDisconnection(const Disconnection& d)
{
    PlayerConnectionPtr& player_connection = d.m_player_connection;

    int id = player_connection->ID();
    // this will not usually happen, since the host client process usually owns the server process, and will usually take it down if it fails
    if (player_connection->Host()) {
        // if the host dies, there's really nothing else we can do -- the game's over
        std::string message = player_connection->PlayerName();
        for (ServerNetworking::const_iterator it = m_server.m_networking.begin(); it != m_server.m_networking.end(); ++it) {
            if ((*it)->ID() != id) {
                (*it)->SendMessage(PlayerDisconnectedMessage((*it)->ID(), message));
                (*it)->SendMessage(EndGameMessage(-1, (*it)->ID()));
            }
        }
        m_server.m_state = SERVER_DYING;
        m_server.m_log_category.debugStream() << "ServerFSM::HandleNonLobbyDisconnection : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        m_server.m_networking.DisconnectAll();
        m_server.Exit(1);
    } else if (m_server.m_losers.find(id) == m_server.m_losers.end()) { // player abnormally disconnected during a regular game
        m_server.m_state = SERVER_DISCONNECT;
        m_server.m_log_category.debugStream() << "ServerFSM::HandleNonLobbyDisconnection : Lost connection to player #" << boost::lexical_cast<std::string>(id) 
                                              << ", named \"" << player_connection->PlayerName() << "\"; server now in mode " << SERVER_DISCONNECT << " (SERVER_DISCONNECT).";
        std::string message = player_connection->PlayerName();
        for (ServerNetworking::const_iterator it = m_server.m_networking.begin(); it != m_server.m_networking.end(); ++it) {
            if ((*it)->ID() != id) {
                (*it)->SendMessage(PlayerDisconnectedMessage((*it)->ID(), message));
                // in the future we may find a way to recover from this, but for now we will immediately send a game ending message as well
                (*it)->SendMessage(EndGameMessage(-1, (*it)->ID()));
            }
        }
    }

    // independently of everything else, if there are no humans left, it's time to terminate
    if (m_server.m_networking.empty() || m_server.m_ai_clients.size() == m_server.m_networking.size()) {
        m_server.m_state = SERVER_DYING;
        m_server.m_log_category.debugStream() << "ServerFSM::HandleNonLobbyDisconnection : All human players disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        m_server.m_networking.DisconnectAll();
        m_server.Exit(1);
    }
}


////////////////////////////////////////////////////////////
// Idle
////////////////////////////////////////////////////////////
Idle::Idle()
{ std::cout << "Idle" << std::endl; }

Idle::~Idle()
{ std::cout << "~Idle" << std::endl; }

boost::statechart::result Idle::react(const HostMPGame& msg)
{
    std::cout << "Idle.HostMPGame" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;
    PlayerConnectionPtr& player_connection = msg.m_player_connection;

    if (server.m_networking.empty() && server.m_expected_ai_players.empty()) { // TODO: superfluous
        std::string host_player_name = message.Text();
        int player_id = Networking::HOST_PLAYER_ID;

        // start an MP lobby situation so that game settings can be established
        server.m_single_player_game = false;
        server.m_state = SERVER_MP_LOBBY;
        server.m_lobby_data = MultiplayerLobbyData(true);
        server.m_log_category.debugStream() << "Idle.HostMPGame : Server now in mode " << SERVER_MP_LOBBY << " (SERVER_MP_LOBBY).";
        player_connection->EstablishPlayer(player_id, host_player_name, true);
        player_connection->SendMessage(HostMPAckMessage(player_id));
        player_connection->SendMessage(JoinAckMessage(player_id));
        server.m_lobby_data.m_players.push_back(PlayerSetupData());
        server.m_lobby_data.m_players.back().m_player_id = player_id;
        server.m_lobby_data.m_players.back().m_player_name = host_player_name;
        server.m_lobby_data.m_players.back().m_empire_color = EmpireColors().at(0);
        player_connection->SendMessage(ServerLobbyUpdateMessage(player_id, server.m_lobby_data));
    } else {
        server.m_log_category.errorStream() << "Idle.HostMPGame : A human player attempted to host "
            "a new game but there was already one in progress or one being setup.  Terminating connection.";
        server.m_networking.Disconnect(player_connection);
    }

    return transit<MPLobby>();
}

boost::statechart::result Idle::react(const HostSPGame& msg)
{
    std::cout << "Idle.HostSPGame" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;
    PlayerConnectionPtr& player_connection = msg.m_player_connection;

    // TODO: if new game, do this: ...

    if (server.m_networking.empty() && server.m_expected_ai_players.empty()) { // TODO: superfluous
        SinglePlayerSetupData setup_data;
        ExtractMessageData(message, setup_data);

        int player_id = Networking::HOST_PLAYER_ID;

        // immediately start a new game with the given parameters
        server.m_single_player_game = true;
        server.m_expected_players = setup_data.m_AIs + 1;
        server.m_galaxy_size = setup_data.m_size;
        server.m_galaxy_shape = setup_data.m_shape;
        server.m_galaxy_age = setup_data.m_age;
        server.m_starlane_freq = setup_data.m_starlane_freq;
        server.m_planet_density = setup_data.m_planet_density;
        server.m_specials_freq = setup_data.m_specials_freq;
        server.CreateAIClients(std::vector<PlayerSetupData>(setup_data.m_AIs));
        server.m_player_save_game_data.clear();
        server.m_lobby_data.m_players.clear();
        server.m_lobby_data.m_players.push_back(PlayerSetupData());
        server.m_lobby_data.m_players.back().m_player_id = player_id;
        server.m_lobby_data.m_players.back().m_player_name = setup_data.m_host_player_name;
        server.m_lobby_data.m_players.back().m_empire_name = setup_data.m_empire_name;
        server.m_lobby_data.m_players.back().m_empire_color = setup_data.m_empire_color;
        server.m_state = SERVER_GAME_SETUP;
        player_connection->EstablishPlayer(player_id, setup_data.m_host_player_name, true);
        player_connection->SendMessage(HostSPAckMessage(player_id));
        player_connection->SendMessage(JoinAckMessage(player_id));
        server.m_log_category.debugStream() << "Idle.HostSPGame : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
        server.m_log_category.debugStream() << "Idle.HostSPGame : Universe size set to " << server.m_galaxy_size << " systems (SERVER_GAME_SETUP).";
        server.m_log_category.debugStream() << "Idle.HostSPGame : Universe shape set to " << server.m_galaxy_shape << " (SERVER_GAME_SETUP).";
    } else {
        server.m_log_category.errorStream() << "Idle.HostSPGame : A human player attempted to host "
            "a new game but there was already one in progress or one being setup.  Terminating connection.";
        server.m_networking.Disconnect(player_connection);
    }

    // TODO: if loaded game, do this (same as LoadGame -- needs to be consolidated):

    ServerNetworking::const_iterator sender_it = server.m_networking.GetPlayer(message.SendingPlayer());
    if (sender_it != server.m_networking.end() && (*sender_it)->Host()) {
        server.m_empires.RemoveAllEmpires();
        server.m_single_player_game = true;
        LoadGame(message.Text(), server.m_current_turn, server.m_player_save_game_data, server.m_universe);
        server.m_expected_players = server.m_player_save_game_data.size();
        server.CreateAIClients(std::vector<PlayerSetupData>(server.m_expected_players - 1));
        server.m_state = SERVER_GAME_SETUP;
    } else {
        server.m_log_category.errorStream() << "WaitingForTurnEnd.LoadSPGame : Player #" << message.SendingPlayer()
                                            << " attempted to initiate a game save, but is not the host, or is not found a valid player.";
    }

    return transit<WaitingForJoiners>();
}


////////////////////////////////////////////////////////////
// MPLobby
////////////////////////////////////////////////////////////
MPLobby::MPLobby()
{ std::cout << "MPLobby" << std::endl; }

MPLobby::~MPLobby()
{ std::cout << "~MPLobby" << std::endl; }

boost::statechart::result MPLobby::react(const Disconnection& d)
{
    std::cout << "MPLobby.Disconnection" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    PlayerConnectionPtr& player_connection = d.m_player_connection;

    int id = player_connection->ID();
    // this will not usually happen, since the host process usually owns the server process, and will usually take it down if it fails
    if (player_connection->Host()) {
        for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
            if ((*it)->ID() != id) {
                (*it)->SendMessage(ServerLobbyHostAbortMessage((*it)->ID()));
                server.m_networking.Disconnect((*it)->ID());
            }
        }
        server.m_networking.Disconnect(id);
        server.m_state = SERVER_DYING;
        server.m_log_category.debugStream() << "MPLobby.Disconnection : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        server.m_networking.DisconnectAll();
        server.Exit(1);
    } else {
        unsigned int i = 0;
        for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it, ++i) {
            if ((*it)->ID() == id) {
                if (i < server.m_lobby_data.m_players.size())
                    server.m_lobby_data.m_players.erase(server.m_lobby_data.m_players.begin() + i); // remove the exiting player's PlayerSetupData struct
            } else {
                (*it)->SendMessage(ServerLobbyExitMessage(id, (*it)->ID()));
            }
        }
        server.m_networking.Disconnect(id);
    }

    // independently of everything else, if there are no humans left, it's time to terminate
    if (server.m_networking.empty() || server.m_ai_clients.size() == server.m_networking.size()) {
        server.m_state = SERVER_DYING;
        server.m_log_category.debugStream() << "MPLobby.Disconnection : All human players disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        server.m_networking.DisconnectAll();
        server.Exit(1);
    }

    return discard_event();
}

boost::statechart::result MPLobby::react(const JoinGame& msg)
{
    std::cout << "MPLobby.JoinGame" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;
    PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name = message.Text();
    int player_id = (*server.m_networking.rbegin())->ID() + 1;
    player_connection->EstablishPlayer(player_id, player_name, false);
    player_connection->SendMessage(JoinAckMessage(player_id));
    player_connection->SendMessage(ServerLobbyUpdateMessage(player_id, server.m_lobby_data));
    server.m_lobby_data.m_players.push_back(PlayerSetupData());
    server.m_lobby_data.m_players.back().m_player_id = player_id;
    server.m_lobby_data.m_players.back().m_player_name = player_name;
    server.m_lobby_data.m_players.back().m_empire_color = EmpireColors().at(0);
    for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
        (*it)->SendMessage(ServerLobbyUpdateMessage((*it)->ID(), server.m_lobby_data));
    }

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyUpdate& msg)
{
    std::cout << "MPLobby.LobbyUpdate" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    MultiplayerLobbyData mp_lobby_data;
    ExtractMessageData(message, mp_lobby_data);

    // NOTE: The client is only allowed to update certain of these, so those are the only ones we'll copy into m_lobby_data.
    server.m_lobby_data.m_new_game = mp_lobby_data.m_new_game;
    server.m_lobby_data.m_size = mp_lobby_data.m_size;
    server.m_lobby_data.m_shape = mp_lobby_data.m_shape;
    server.m_lobby_data.m_age = mp_lobby_data.m_age;
    server.m_lobby_data.m_starlane_freq = mp_lobby_data.m_starlane_freq;
    server.m_lobby_data.m_planet_density = mp_lobby_data.m_planet_density;
    server.m_lobby_data.m_specials_freq = mp_lobby_data.m_specials_freq;

    bool new_save_file_selected = false;
    if (mp_lobby_data.m_save_file_index != server.m_lobby_data.m_save_file_index &&
        0 <= mp_lobby_data.m_save_file_index && mp_lobby_data.m_save_file_index < static_cast<int>(server.m_lobby_data.m_save_games.size())) {
        server.m_lobby_data.m_save_file_index = mp_lobby_data.m_save_file_index;
        RebuildSaveGameEmpireData(server.m_lobby_data.m_save_game_empire_data, server.m_lobby_data.m_save_games[server.m_lobby_data.m_save_file_index]);
        // reset the current choice of empire for each player, since the new save game's empires may not have the same IDs
        for (unsigned int i = 0; i < server.m_lobby_data.m_players.size(); ++i) {
            server.m_lobby_data.m_players[i].m_save_game_empire_id = -1;
        }
        new_save_file_selected = true;
    }
    server.m_lobby_data.m_players = mp_lobby_data.m_players;

    for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
        if ((*it)->ID() != message.SendingPlayer() || new_save_file_selected)
            (*it)->SendMessage(ServerLobbyUpdateMessage((*it)->ID(), server.m_lobby_data));
    }

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyChat& msg)
{
    std::cout << "MPLobby.LobbyChat" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    if (message.ReceivingPlayer() == -1) { // the receiver is everyone (except the sender)
        for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
            if ((*it)->ID() != message.SendingPlayer())
                (*it)->SendMessage(ServerLobbyChatMessage(message.SendingPlayer(), (*it)->ID(), message.Text()));
        }
    } else {
        server.m_networking.SendMessage(ServerLobbyChatMessage(message.SendingPlayer(), message.ReceivingPlayer(), message.Text()));
    }

    return discard_event();
}

boost::statechart::result MPLobby::react(const LobbyHostAbort& msg)
{
    std::cout << "MPLobby.LobbyHostAbort" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
        if ((*it)->ID() != message.SendingPlayer()) {
            (*it)->SendMessage(ServerLobbyHostAbortMessage((*it)->ID()));
            server.m_networking.Disconnect((*it)->ID());
        }
    }

    return transit<Idle>(); // TODO: Is this right?
}

boost::statechart::result MPLobby::react(const LobbyNonHostExit& msg)
{
    std::cout << "MPLobby.LobbyNonHostExit" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    unsigned int i = 0;
    for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it, ++i) {
        if ((*it)->ID() == message.SendingPlayer()) {
            if (i < server.m_lobby_data.m_players.size())
                server.m_lobby_data.m_players.erase(server.m_lobby_data.m_players.begin() + i); // remove the exiting player's PlayerSetupData struct
        } else {
            (*it)->SendMessage(ServerLobbyExitMessage(message.SendingPlayer(), (*it)->ID()));
        }
    }
    server.m_networking.Disconnect(message.SendingPlayer());

    return discard_event();
}

boost::statechart::result MPLobby::react(const StartMPGame& msg)
{
    std::cout << "MPLobby.StartMPGame" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    if (server.m_lobby_data.m_new_game) { // new game
        server.m_galaxy_size = server.m_lobby_data.m_size;
        server.m_galaxy_shape = server.m_lobby_data.m_shape;
        server.m_galaxy_age = server.m_lobby_data.m_age;
        server.m_starlane_freq = server.m_lobby_data.m_starlane_freq;
        server.m_planet_density = server.m_lobby_data.m_planet_density;
        server.m_specials_freq = server.m_lobby_data.m_specials_freq;
        server.m_expected_players = server.m_networking.size() + server.m_lobby_data.m_AIs.size();
        for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
            if ((*it)->ID() != message.SendingPlayer())
                (*it)->SendMessage(Message(Message::GAME_START, -1, (*it)->ID(), Message::CLIENT_LOBBY_MODULE, ""));
        }
        server.m_state = SERVER_GAME_SETUP;
        server.CreateAIClients(server.m_lobby_data.m_AIs);
        server.m_player_save_game_data.clear();
        server.m_log_category.debugStream() << "MPLobby.StartMPGame : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
        if (server.m_expected_players == static_cast<int>(server.m_networking.size())) {
            server.NewGameInit();
            server.m_state = SERVER_WAITING;
            server.m_log_category.debugStream() << "MPLobby.StartMPGame : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
        }
    } else { // load game
        ServerNetworking::const_iterator sender_it = server.m_networking.GetPlayer(message.SendingPlayer());
        if (sender_it != server.m_networking.end() && (*sender_it)->Host()) {
            server.m_empires.RemoveAllEmpires();
            server.m_single_player_game = false;
            LoadGame((GetLocalDir() / "save" / server.m_lobby_data.m_save_games[server.m_lobby_data.m_save_file_index]).native_file_string(),
                     server.m_current_turn, server.m_player_save_game_data, server.m_universe);
            server.m_expected_players = server.m_player_save_game_data.size();
            server.m_empires.RemoveAllEmpires();
            for (unsigned int i = 0; i < server.m_player_save_game_data.size(); ++i) {
                for (unsigned int j = 0; j < server.m_lobby_data.m_players.size(); ++j) {
                    assert(server.m_player_save_game_data[i].m_empire);
                    if (server.m_lobby_data.m_players[j].m_save_game_empire_id == server.m_player_save_game_data[i].m_empire->EmpireID()) {
                        ServerNetworking::const_iterator player_it = server.m_networking.begin();
                        std::advance(player_it, j);  // TODO: This is probably broken now
                        server.m_player_save_game_data[i].m_name = (*player_it)->PlayerName();
                        server.m_player_save_game_data[i].m_empire->SetPlayerName((*player_it)->PlayerName());
                    }
                }
                server.m_empires.InsertEmpire(server.m_player_save_game_data[i].m_empire);
            }

            for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
                if (it != sender_it)
                    (*it)->SendMessage(Message(Message::GAME_START, -1, (*it)->ID(), Message::CLIENT_LOBBY_MODULE, ""));
            }

            int AI_clients = server.m_expected_players - server.m_networking.size();
            server.CreateAIClients(std::vector<PlayerSetupData>(AI_clients));
            server.m_state = SERVER_GAME_SETUP;

            if (!AI_clients)
                server.LoadGameInit();
        } else {
            server.m_log_category.errorStream() << "MPLobby.StartMPGame : Player #" << message.SendingPlayer()
                                                << " attempted to initiate a game load, but is not the host, or is not found a valid player.";
        }
    }

    return transit<WaitingForJoiners>();
}


////////////////////////////////////////////////////////////
// WaitingForJoiners
////////////////////////////////////////////////////////////
WaitingForJoiners::WaitingForJoiners()
{ std::cout << "WaitingForJoiners" << std::endl; }

WaitingForJoiners::~WaitingForJoiners()
{ std::cout << "~WaitingForJoiners" << std::endl; }

boost::statechart::result WaitingForJoiners::react(const JoinGame& msg)
{
    std::cout << "WaitingForJoiners.JoinGame" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;
    PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name = message.Text();
    int player_id = (*server.m_networking.rbegin())->ID() + 1;
    std::set<std::string>::iterator it = server.m_expected_ai_players.find(player_name);
    if (it != server.m_expected_ai_players.end()) { // incoming AI player connection
        // let the networking system know what socket this player is on
        player_connection->EstablishPlayer(player_id, player_name, false);
        player_connection->SendMessage(JoinAckMessage(player_id));
        server.m_expected_ai_players.erase(player_name); // only allow one connection per AI
        server.m_ai_IDs.insert(player_id);
    } else { // non-AI player connection
        if (static_cast<int>(server.m_expected_ai_players.size() + server.m_networking.size()) < server.m_expected_players) {
            player_connection->EstablishPlayer(player_id, player_name, false);
            player_connection->SendMessage(JoinAckMessage(player_id));
        } else {
            server.m_log_category.errorStream() << "WaitingForJoiners.JoinGame : A human player attempted to join "
                "the game but there was not enough room.  Terminating connection.";
            server.m_networking.Disconnect(player_connection);
        }
    }

    if (static_cast<int>(server.m_networking.size()) == server.m_expected_players) { // if we've gotten all the players joined up
        if (server.m_player_save_game_data.empty())
            server.NewGameInit();
        else
            server.LoadGameInit();
        server.m_state = SERVER_WAITING;
        server.m_log_category.debugStream() << "WaitingForJoiners.JoinGame : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
    }

#if 0
    if (/*TODO*/)
        return transit<PlayingGame>();
    else
#endif
    return discard_event();
}


////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame()
{ std::cout << "PlayingGame" << std::endl; }

PlayingGame::~PlayingGame()
{ std::cout << "~PlayingGame" << std::endl; }


////////////////////////////////////////////////////////////
// WaitingForTurnEnd
////////////////////////////////////////////////////////////
WaitingForTurnEnd::WaitingForTurnEnd()
{ std::cout << "WaitingForTurnEnd" << std::endl; }

WaitingForTurnEnd::~WaitingForTurnEnd()
{ std::cout << "~WaitingForTurnEnd" << std::endl; }

boost::statechart::result WaitingForTurnEnd::react(const LoadSPGame& msg)
{
    std::cout << "WaitingForTurnEnd.LoadSPGame" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    // TODO: Same as load case of HostSPGame -- needs to be consolidated.
    ServerNetworking::const_iterator sender_it = server.m_networking.GetPlayer(message.SendingPlayer());
    if (sender_it != server.m_networking.end() && (*sender_it)->Host()) {
        server.m_empires.RemoveAllEmpires();
        server.m_single_player_game = true;
        LoadGame(message.Text(), server.m_current_turn, server.m_player_save_game_data, server.m_universe);
        server.m_expected_players = server.m_player_save_game_data.size();
        server.CreateAIClients(std::vector<PlayerSetupData>(server.m_expected_players - 1));
        server.m_state = SERVER_GAME_SETUP;
    } else {
        server.m_log_category.errorStream() << "WaitingForTurnEnd.LoadSPGame : Player #" << message.SendingPlayer()
                                            << " attempted to initiate a game save, but is not the host, or is not found a valid player.";
    }

    return discard_event();
}

boost::statechart::result WaitingForTurnEnd::react(const TurnOrders& msg)
{
    std::cout << "WaitingForTurnEnd.TurnOrders" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    OrderSet* order_set = new OrderSet;
    ExtractMessageData(message, *order_set);

    // check order validity -- all orders must originate from this empire in order to be considered valid
    Empire* empire = server.GetPlayerEmpire(message.SendingPlayer());
    assert(empire);
    for (OrderSet::const_iterator it = order_set->begin(); it != order_set->end(); ++it) {
        Order* order = it->second;
        assert(order);
        if (empire->EmpireID() != order->EmpireID()) {
            throw std::runtime_error(
                "WaitingForTurnEnd.TurnOrders : Player \"" + empire->PlayerName() + "\""
                " attempted to issue an order for player "
                "\"" + Empires().Lookup(order->EmpireID())->PlayerName() + "\"!  Terminating...");
        }
    }

    server.m_networking.SendMessage(TurnProgressMessage(message.SendingPlayer(), Message::WAITING_FOR_PLAYERS, -1));

    server.m_log_category.debugStream() << "WaitingForTurnEnd.TurnOrders : Received orders from player " << message.SendingPlayer();

    /* if all orders are received already, do nothing as we are processing a turn */
    if (server.AllOrdersReceived())
        return discard_event(); // TODO: Huh?

    /* add orders to turn sequence */    
    server.SetEmpireTurnOrders(server.GetPlayerEmpire(message.SendingPlayer())->EmpireID(), order_set);

    /* look to see if all empires are done */
    if (server.AllOrdersReceived()) {
        server.m_log_category.debugStream() << "WaitingForTurnEnd.TurnOrders : All orders received; processing turn...";
        server.ProcessTurns();
    }

#if 0
    if (/*TODO*/) {
        // ProcessTurns(), etc.
        return transit<WaitingForTurnEnd>();
    } else {
#endif
    return discard_event();
}

boost::statechart::result WaitingForTurnEnd::react(const RequestObjectID& msg)
{
    std::cout << "WaitingForTurnEnd.RequestObjectID" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;
    server.m_networking.SendMessage(DispatchObjectIDMessage(message.SendingPlayer(), GetUniverse().GenerateObjectID()));
    return discard_event();
}

boost::statechart::result WaitingForTurnEnd::react(const PlayerChat& msg)
{
    std::cout << "WaitingForTurnEnd.PlayerChat" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    std::string text = message.Text();

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
            for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
                if (tokens[i] == (*it)->PlayerName()) {
                    token_is_name = true;
                    break;
                }
            }
            if (token_is_name)
                target_player_names.insert(tokens[i]);
            else
                return discard_event(); // TODO: What to return here?
        }
    }
    if (!target_player_names.empty()) {
        text = text.substr(colon_position + 1);
        if (text == "")
            return discard_event(); // TODO: What to return here?
    }
    Empire* sender_empire = server.GetPlayerEmpire(message.SendingPlayer());
    std::string final_text = RgbaTag(Empires().Lookup(sender_empire->EmpireID())->Color()) + (*server.m_networking.GetPlayer(message.SendingPlayer()))->PlayerName() +
        (target_player_names.empty() ? ": " : " (whisper):") + text + "</rgba>\n";
    for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
        if (target_player_names.empty() || target_player_names.find((*it)->PlayerName()) != target_player_names.end())
            (*it)->SendMessage(ChatMessage(message.SendingPlayer(), (*it)->ID(), final_text));
    }

    return discard_event();
}

boost::statechart::result WaitingForTurnEnd::react(const EndGame& msg)
{
    std::cout << "WaitingForTurnEnd.EndGame" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    ServerNetworking::const_iterator it = server.m_networking.GetPlayer(message.SendingPlayer());
    if (it != server.m_networking.end() && (*it)->Host()) {
        for (ServerNetworking::const_iterator it2 = server.m_networking.begin(); it2 != server.m_networking.end(); ++it2) {
            if ((*it)->ID() != (*it2)->ID())
                (*it2)->SendMessage(EndGameMessage(-1, (*it2)->ID()));
        }
        server.m_state = SERVER_DYING;
        server.m_log_category.debugStream() << "WaitingForTurnEnd.EndGame : Server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        server.m_networking.SendMessage(Message(Message::SERVER_STATUS, -1, message.SendingPlayer(), Message::CORE, boost::lexical_cast<std::string>(server.m_state)));
        server.m_networking.DisconnectAll();
        server.Exit(0);
    }

    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForTurnEndIdle
////////////////////////////////////////////////////////////
WaitingForTurnEndIdle::WaitingForTurnEndIdle()
{ std::cout << "WaitingForTurnEndIdle" << std::endl; }

WaitingForTurnEndIdle::~WaitingForTurnEndIdle()
{ std::cout << "~WaitingForTurnEndIdle" << std::endl; }

boost::statechart::result WaitingForTurnEndIdle::react(const SaveGameRequest& msg)
{
    std::cout << "WaitingForTurnEnd.SaveGameRequest" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    if (server.m_networking.GetPlayer(message.SendingPlayer()) != server.m_networking.end()) { // TODO: superfluous
        // send out all save game data requests
        std::set<int> needed_reponses;
        server.m_players_responded.clear();
        server.m_player_save_game_data.clear();
        for (ServerNetworking::const_iterator it = server.m_networking.begin(); it != server.m_networking.end(); ++it) {
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
        if (server.m_players_responded == needed_reponses) {
            SaveGame(message.Text(), server.m_current_turn, server.m_player_save_game_data, server.m_universe);
            server.m_networking.SendMessage(ServerSaveGameMessage(message.SendingPlayer(), true));
        }
    } else {
        server.m_log_category.errorStream() << "WaitingForTurnEnd.SaveGameRequest : Player #" << message.SendingPlayer()
                                            << " attempted to initiate a game save, but is not found a valid player.";
    }

    return transit<WaitingForSaveData>();
}


////////////////////////////////////////////////////////////
// WaitingForSaveData
////////////////////////////////////////////////////////////
WaitingForSaveData::WaitingForSaveData()
{ std::cout << "WaitingForSaveData" << std::endl; }

WaitingForSaveData::~WaitingForSaveData()
{ std::cout << "~WaitingForSaveData" << std::endl; }

boost::statechart::result WaitingForSaveData::react(const ClientSaveData& msg)
{
    std::cout << "WaitingForSaveData.ClientSaveData" << std::endl;
    ServerApp& server = context<ServerFSM>().Server();
    const Message& message = msg.m_message;

    boost::shared_ptr<OrderSet> order_set(new OrderSet);
    boost::shared_ptr<SaveGameUIData> ui_data(new SaveGameUIData);
    if (!ExtractMessageData(message, *order_set, *ui_data))
        ui_data.reset();
    ServerNetworking::const_iterator player_it = server.m_networking.GetPlayer(message.SendingPlayer());
    assert(player_it != server.m_networking.end());
    server.m_player_save_game_data.push_back(PlayerSaveGameData((*player_it)->PlayerName(), server.GetPlayerEmpire(message.SendingPlayer()), order_set, ui_data));
    server.m_players_responded.insert(message.SendingPlayer());

#if 0
    if (/*TODO*/)
        return transit<WaitingForTurnEndIdle>();
    else
#endif
    return discard_event();
}
