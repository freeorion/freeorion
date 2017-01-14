#include "ServerFSM.h"

#include "SaveLoad.h"
#include "ServerApp.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/System.h"
#include "../universe/Species.h"
#include "../network/ServerNetworking.h"
#include "../network/Message.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OrderSet.h"
#include "../util/Random.h"
#include "../util/ModeratorAction.h"
#include "../util/MultiplayerCommon.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

class CombatLogManager;
CombatLogManager&   GetCombatLogManager();

namespace {
    const bool TRACE_EXECUTION = true;

    void SendMessageToAllPlayers(const Message& message) {
        ServerApp* server = ServerApp::GetApp();
        if (!server) {
            ErrorLogger() << "SendMessageToAllPlayers couldn't get server.";
            return;
        }
        ServerNetworking& networking = server->Networking();

        for (ServerNetworking::const_established_iterator player_it = networking.established_begin();
             player_it != networking.established_end();
             ++player_it)
        {
            PlayerConnectionPtr player = *player_it;
            player->SendMessage(message);
        }
    }

    void SendMessageToHost(const Message& message) {
        ServerApp* server = ServerApp::GetApp();
        if (!server) {
            ErrorLogger() << "SendMessageToHost couldn't get server.";
            return;
        }
        ServerNetworking& networking = server->Networking();

        ServerNetworking::established_iterator host_it = networking.GetPlayer(networking.HostPlayerID());
        if (host_it == networking.established_end()) {
            ErrorLogger() << "SendMessageToHost couldn't get host player.";
            return;
        }

        PlayerConnectionPtr host = *host_it;
        host->SendMessage(message);
    }

    std::string GetHostNameFromSinglePlayerSetupData(const SinglePlayerSetupData& single_player_setup_data) {
        if (single_player_setup_data.m_new_game) {
            // for new games, get host player's name from PlayerSetupData for the
            // (should be only) human player
            for (const PlayerSetupData& psd : single_player_setup_data.m_players) {
                // In a single player game, the host player is always the human player, so
                // this is just a matter of finding which player setup data is for
                // a human player, and assigning that setup data to the host player id
                if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                    return psd.m_player_name;
            }

        } else {
            // for loading saved games, get host / human player's name from save file
            if (!single_player_setup_data.m_players.empty())
                ErrorLogger() << "GetHostNameFromSinglePlayerSetupData got single player setup data to load a game, but also player setup data for a new game.  Ignoring player setup data";


            std::vector<PlayerSaveHeaderData> player_save_header_data;
            LoadPlayerSaveHeaderData(single_player_setup_data.m_filename, player_save_header_data);

            // find which player was the human (and thus the host) in the saved game
            for (const PlayerSaveHeaderData& psgd : player_save_header_data) {
                if (psgd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                    return psgd.m_name;
            }
        }
        return "";
    }

    void LogPlayerSetupData(const std::list<std::pair<int, PlayerSetupData> >& psd) {
        DebugLogger() << "PlayerSetupData:";
        for (const std::pair<int, PlayerSetupData>& entry : psd) {
            std::stringstream ss;
            ss << boost::lexical_cast<std::string>(entry.first) << " : "
               << entry.second.m_player_name << ", ";
            switch (entry.second.m_client_type) {
            case Networking::CLIENT_TYPE_AI_PLAYER:         ss << "AI_PLAYER, ";    break;
            case Networking::CLIENT_TYPE_HUMAN_MODERATOR:   ss << "MODERATOR, ";    break;
            case Networking::CLIENT_TYPE_HUMAN_OBSERVER:    ss << "OBSERVER, ";     break;
            case Networking::CLIENT_TYPE_HUMAN_PLAYER:      ss << "HUMAN_PLAYER, "; break;
            default:                                        ss << "<invalid client type>, ";
            }
            ss << entry.second.m_starting_species_name;
            DebugLogger() << " ... " << ss.str();
        }
    }

    std::string GenerateEmpireName(std::list<std::pair<int, PlayerSetupData> >& players) {
        // load default empire names
        static std::list<std::string> empire_names;
        if (empire_names.empty())
            UserStringList("EMPIRE_NAMES", empire_names);
        std::set<std::string> validNames(empire_names.begin(), empire_names.end());
        for (const std::pair<int, PlayerSetupData>& psd : players) {
            std::set<std::string>::iterator name_it = validNames.find(psd.second.m_empire_name);
            if (name_it != validNames.end())
                validNames.erase(name_it);
        }
        if (!validNames.empty()) {
            // pick a name from the list of empire names
            int empire_name_idx = RandSmallInt(0, static_cast<int>(validNames.size()) - 1);
            std::set<std::string>::iterator it = validNames.begin();
            std::advance(it, empire_name_idx);
            return *it;
        } else {
            // use a generic name
            return UserString("EMPIRE");
        }
    }


    /** Note:  This Exits on fatal errors and does not return.*/
    void HandleErrorMessage(const Error& msg, ServerApp &server) {
        std::string problem;
        bool fatal;
        ExtractMessageData(msg.m_message, problem, fatal);

        std::stringstream ss;

        ss << "Server received from player "
           << msg.m_player_connection->PlayerName() << "("
           << msg.m_player_connection->PlayerID() << ")"
           << (fatal?" a fatal":" an")
           << " error message: " << problem;

        if (fatal) {
            FatalLogger() << ss.str();
            SendMessageToAllPlayers(msg.m_message);
            boost::this_thread::sleep_for(boost::chrono::seconds(2));
            server.Exit(1);
        }

        ErrorLogger() << ss.str();
    }
}

////////////////////////////////////////////////////////////
// Disconnection
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

void ServerFSM::unconsumed_event(const sc::event_base &event) {
    std::string most_derived_message_type_str = "[ERROR: Unknown Event]";
    const sc::event_base* event_ptr = &event;
    if (dynamic_cast<const Disconnection*>(event_ptr))
        most_derived_message_type_str = "Disconnection";
#define MESSAGE_EVENT_CASE(r, data, name)                               \
    else if (dynamic_cast<const name*>(event_ptr))                      \
        most_derived_message_type_str = BOOST_PP_STRINGIZE(name);
    BOOST_PP_SEQ_FOR_EACH(MESSAGE_EVENT_CASE, _, MESSAGE_EVENTS);
#undef MESSAGE_EVENT_CASE
    ErrorLogger() << "ServerFSM : A " << most_derived_message_type_str << " event was passed to "
        "the ServerFSM.  This event is illegal in the FSM's current state.  It is being ignored.";
}

ServerApp& ServerFSM::Server()
{ return m_server; }

void ServerFSM::HandleNonLobbyDisconnection(const Disconnection& d) {
    PlayerConnectionPtr& player_connection = d.m_player_connection;
    int id = player_connection->PlayerID();

    // Did an active player (AI or Human) disconnect?  If so, game is over
    if (player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
        player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
    {
        // can continue.  Select new host if necessary.
        if (m_server.m_networking.PlayerIsHost(player_connection->PlayerID()))
            m_server.SelectNewHost();

    } else if (player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
               player_connection->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER)
    {
        const Empire* empire = GetEmpire(m_server.PlayerEmpireID(id));
        // eliminated and non-empire players can leave safely
        if (empire && !empire->Eliminated()) {
            // player abnormally disconnected during a regular game
            DebugLogger() << "ServerFSM::HandleNonLobbyDisconnection : Lost connection to player #" << id
                          << ", named \"" << player_connection->PlayerName() << "\"; server terminating.";
            std::string message = player_connection->PlayerName();
            for (ServerNetworking::const_established_iterator it = m_server.m_networking.established_begin(); it != m_server.m_networking.established_end(); ++it) {
                if ((*it)->PlayerID() == id)
                    continue;
                // in the future we may find a way to recover from this, but for now we will immediately send a game ending message as well
                (*it)->SendMessage(EndGameMessage((*it)->PlayerID(), Message::PLAYER_DISCONNECT, player_connection->PlayerName()));
            }
        }
    }

    // independently of everything else, if there are no humans left, it's time to terminate
    if (m_server.m_networking.empty() || m_server.m_ai_client_processes.size() == m_server.m_networking.NumEstablishedPlayers()) {
        DebugLogger() << "ServerFSM::HandleNonLobbyDisconnection : All human players disconnected; server terminating.";
        // HACK! Pause for a bit to let the player disconnected and end game messages propogate.
        boost::this_thread::sleep_for(boost::chrono::seconds(2));
        m_server.Exit(1);
    }
}


////////////////////////////////////////////////////////////
// Idle
////////////////////////////////////////////////////////////
Idle::Idle(my_context c) :
    my_base(c)
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) Idle"; }

Idle::~Idle()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~Idle"; }

sc::result Idle::react(const HostMPGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) Idle.HostMPGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string host_player_name;
    std::string client_version_string;
    ExtractMessageData(message, host_player_name, client_version_string);

    // validate host name (was found and wasn't empty)
    if (host_player_name.empty()) {
        ErrorLogger() << "Idle::react(const HostMPGame& msg) got an empty host player name";
        return discard_event();
    }

    DebugLogger() << "Idle::react(HostMPGame) about to establish host";

    int host_player_id = server.m_networking.NewPlayerID();
    player_connection->EstablishPlayer(host_player_id, host_player_name, Networking::CLIENT_TYPE_HUMAN_PLAYER, client_version_string);
    server.m_networking.SetHostPlayerID(host_player_id);

    DebugLogger() << "Idle::react(HostMPGame) about to send acknowledgement to host";
    player_connection->SendMessage(HostMPAckMessage(host_player_id));

    server.m_single_player_game = false;

    DebugLogger() << "Idle::react(HostMPGame) about to transit to MPLobby";

    return transit<MPLobby>();
}

sc::result Idle::react(const HostSPGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) Idle.HostSPGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    boost::shared_ptr<SinglePlayerSetupData> single_player_setup_data(new SinglePlayerSetupData);
    std::string client_version_string;
    ExtractMessageData(message, *single_player_setup_data, client_version_string);


    // get host player's name from setup data or saved file
    std::string host_player_name;
    try {
        host_player_name = GetHostNameFromSinglePlayerSetupData(*single_player_setup_data);
    } catch (const std::exception& e) {
        const PlayerConnectionPtr& player_connection = msg.m_player_connection;
        player_connection->SendMessage(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), true));
        return discard_event();
    }
    // validate host name (was found and wasn't empty)
    if (host_player_name.empty()) {
        ErrorLogger() << "Idle::react(const HostSPGame& msg) got an empty host player name or couldn't find a human player";
        return discard_event();
    }


    int host_player_id = server.m_networking.NewPlayerID();
    player_connection->EstablishPlayer(host_player_id, host_player_name, Networking::CLIENT_TYPE_HUMAN_PLAYER, client_version_string);
    server.m_networking.SetHostPlayerID(host_player_id);
    player_connection->SendMessage(HostSPAckMessage(host_player_id));

    server.m_single_player_game = true;

    context<ServerFSM>().m_single_player_setup_data = single_player_setup_data;

    return transit<WaitingForSPGameJoiners>();
}

sc::result Idle::react(const Error& msg) {
    HandleErrorMessage(msg, Server());
    return discard_event();
}

////////////////////////////////////////////////////////////
// MPLobby
////////////////////////////////////////////////////////////
MPLobby::MPLobby(my_context c) :
    my_base(c),
    m_lobby_data(new MultiplayerLobbyData()),
    m_server_save_game_data(new ServerSaveGameData())
{
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby";
    ServerApp& server = Server();
    const SpeciesManager& sm = GetSpeciesManager();
    const PlayerConnectionPtr& player_connection = *(server.m_networking.GetPlayer(server.m_networking.HostPlayerID()));

    int host_id = server.m_networking.HostPlayerID();
    ClockSeed();

    // create player setup data for host, and store in list
    m_lobby_data->m_players.push_back(std::make_pair(host_id, PlayerSetupData()));

    PlayerSetupData& player_setup_data = m_lobby_data->m_players.begin()->second;

    player_setup_data.m_player_name =           player_connection->PlayerName();
    player_setup_data.m_empire_name =           (player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER) ? player_connection->PlayerName() : GenerateEmpireName(m_lobby_data->m_players);
    player_setup_data.m_empire_color =          EmpireColors().at(0);               // since the host is the first joined player, it can be assumed that no other player is using this colour (unlike subsequent join game message responses)
    player_setup_data.m_starting_species_name = sm.RandomPlayableSpeciesName();
    // leaving save game empire id as default
    player_setup_data.m_client_type =           player_connection->GetClientType();

    server.m_networking.SendMessage(ServerLobbyUpdateMessage(server.m_networking.HostPlayerID(), *m_lobby_data));
}

MPLobby::~MPLobby()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~MPLobby"; }

sc::result MPLobby::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.Disconnection";
    ServerApp& server = Server();
    PlayerConnectionPtr& player_connection = d.m_player_connection;

    DebugLogger() << "MPLobby::react(Disconnection) player id: " << player_connection->PlayerID();
    DebugLogger() << "Remaining player ids: ";
    for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        DebugLogger() << " ... " << (*it)->PlayerID();
    }

    // if there are no humans left, it's time to terminate
    if (server.m_networking.empty() || server.m_ai_client_processes.size() == server.m_networking.NumEstablishedPlayers()) {
        DebugLogger() << "MPLobby.Disconnection : All human players disconnected; server terminating.";
        server.Exit(1);
    }

    if (server.m_networking.PlayerIsHost(player_connection->PlayerID()))
        server.SelectNewHost();

    // if the disconnected player wasn't in the lobby, don't need to do anything more.
    // if player is in lobby, need to remove it
    int id = player_connection->PlayerID();
    bool player_was_in_lobby = false;
    for (std::list<std::pair<int, PlayerSetupData> >::iterator it = m_lobby_data->m_players.begin();
         it != m_lobby_data->m_players.end(); ++it)
    {
        if (it->first == id) {
            player_was_in_lobby = true;
            m_lobby_data->m_players.erase(it);
            break;
        }
    }
    if (!player_was_in_lobby) {
        DebugLogger() << "MPLobby.Disconnection : Disconnecting player (" << id << ") was not in lobby";
        return discard_event();
    }

    // send updated lobby data to players after disconnection-related changes
    for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        (*it)->SendMessage(ServerLobbyUpdateMessage((*it)->PlayerID(), *m_lobby_data));
    }

    return discard_event();
}

namespace {
    GG::Clr GetUnusedEmpireColour(const std::list<std::pair<int, PlayerSetupData> >& psd) {
        //DebugLogger() << "finding colours for empire of player " << player_name;
        GG::Clr empire_colour = GG::Clr(192, 192, 192, 255);
        for (const GG::Clr& possible_colour : EmpireColors()) {
            //DebugLogger() << "trying colour " << possible_colour.r << ", " << possible_colour.g << ", " << possible_colour.b;

            // check if any other player / empire is using this colour
            bool colour_is_new = true;
            for (const std::pair<int, PlayerSetupData>& entry : psd) {
                const GG::Clr& player_colour = entry.second.m_empire_color;
                if (player_colour == possible_colour) {
                    colour_is_new = false;
                    break;
                }
            }

            // use colour and exit loop if no other empire is using the colour
            if (colour_is_new) {
                empire_colour = possible_colour;
                break;
            }

            //DebugLogger() << " ... colour already used.";
        }
        return empire_colour;
    }
}

sc::result MPLobby::react(const JoinGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.JoinGame";
    ServerApp& server = Server();
    const SpeciesManager& sm = GetSpeciesManager();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name;
    Networking::ClientType client_type;
    std::string client_version_string;
    ExtractMessageData(message, player_name, client_type, client_version_string);
    // TODO: check if player name is unique.  If not, modify it slightly to be unique.

    // assign unique player ID to newly connected player
    int player_id = server.m_networking.NewPlayerID();

    // establish player with requested client type and acknowldge via connection
    player_connection->EstablishPlayer(player_id, player_name, client_type, client_version_string);
    player_connection->SendMessage(JoinAckMessage(player_id));

    // inform player of host
    player_connection->SendMessage(HostIDMessage(server.m_networking.HostPlayerID()));

    // assign player info from defaults or from connection to lobby data players list
    PlayerSetupData player_setup_data;
    player_setup_data.m_player_name =           player_name;
    player_setup_data.m_client_type =           client_type;
    player_setup_data.m_empire_name =           (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) ? player_name : GenerateEmpireName(m_lobby_data->m_players);
    player_setup_data.m_empire_color =          GetUnusedEmpireColour(m_lobby_data->m_players);
    if (m_lobby_data->m_seed!="")
        player_setup_data.m_starting_species_name = sm.RandomPlayableSpeciesName();
    else
        player_setup_data.m_starting_species_name = sm.SequentialPlayableSpeciesName(player_id);

    // after setting all details, push into lobby data
    m_lobby_data->m_players.push_back(std::make_pair(player_id, player_setup_data));

    for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    { (*it)->SendMessage(ServerLobbyUpdateMessage((*it)->PlayerID(), *m_lobby_data)); }

    return discard_event();
}

sc::result MPLobby::react(const LobbyUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.LobbyUpdate";
    ServerApp& server = Server();
    const Message& message = msg.m_message;

    MultiplayerLobbyData incoming_lobby_data;
    ExtractMessageData(message, incoming_lobby_data);

    // check if new lobby data changed player setup data.  if so, need to echo
    // back to sender with updated lobby details.
    bool player_setup_data_changed = (incoming_lobby_data.m_players != m_lobby_data->m_players);

    // store incoming lobby data.  clients can only change some of
    // this information (galaxy setup data, whether it is a new game and what
    // save file index to load) directly, so other data is skipped (list of
    // save files, save game empire data from the save file, player data)
    // during this copying and is updated below from the save file(s)

    // TODO: ensure only the host can change anything other than a player's
    // own details.  non-hosts should not be able to edit other players' info
    // or the galaxy setup.

    // GalaxySetupData
    m_lobby_data->m_seed =          incoming_lobby_data.m_seed;
    m_lobby_data->m_size =          incoming_lobby_data.m_size;
    m_lobby_data->m_shape =         incoming_lobby_data.m_shape;
    m_lobby_data->m_age =           incoming_lobby_data.m_age;
    m_lobby_data->m_starlane_freq = incoming_lobby_data.m_starlane_freq;
    m_lobby_data->m_planet_density =incoming_lobby_data.m_planet_density;
    m_lobby_data->m_specials_freq = incoming_lobby_data.m_specials_freq;
    m_lobby_data->m_monster_freq =  incoming_lobby_data.m_monster_freq;
    m_lobby_data->m_native_freq =   incoming_lobby_data.m_native_freq;
    m_lobby_data->m_ai_aggr     =   incoming_lobby_data.m_ai_aggr;
    
    // directly configurable lobby data
    m_lobby_data->m_new_game =      incoming_lobby_data.m_new_game;
    m_lobby_data->m_players =       incoming_lobby_data.m_players;

    LogPlayerSetupData(m_lobby_data->m_players);

    // update player connection types according to modified lobby selections,
    // while recording connections that are to be dropped
    std::vector<PlayerConnectionPtr> player_connections_to_drop;
    for (ServerNetworking::established_iterator player_connection_it = server.m_networking.established_begin();
         player_connection_it != server.m_networking.established_end(); ++player_connection_it)
    {
        PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        if (player_id == Networking::INVALID_PLAYER_ID)
            continue;

        // get lobby data for this player connection
        bool found_player_lobby_data = false;
        std::list<std::pair<int, PlayerSetupData> >::iterator player_setup_it = m_lobby_data->m_players.begin();
        while (player_setup_it != m_lobby_data->m_players.end()) {
            if (player_setup_it->first == player_id) {
                found_player_lobby_data = true;
                break;
            }
            ++player_setup_it;
        }

        // drop connections which have no lobby data
        if (!found_player_lobby_data) {
            ErrorLogger() << "No player setup data for player " << player_id << " in MPLobby::react(const LobbyUpdate& msg)";
            player_connections_to_drop.push_back(player_connection);
            continue;
        }

        // get client type, and drop connections with invalid type, as that indicates a requested drop
        Networking::ClientType client_type = player_setup_it->second.m_client_type;

        if (client_type != Networking::INVALID_CLIENT_TYPE) {
            // update player connection type for lobby change
            player_connection->SetClientType(client_type);
        } else {
            // drop connections for players who were dropped from lobby
            m_lobby_data->m_players.erase(player_setup_it);
            player_connections_to_drop.push_back(player_connection);
        }
    }

    // drop players connections.  Doing this in separate loop to avoid messing
    // up iteration above.  these disconnections will lead to Disconnect events
    // being generated and MPLobby::react(Disconnect) being called.  If this
    // disconnects the host, then a new host will be selected within that function.
    for (PlayerConnectionPtr drop_con : player_connections_to_drop) {
        server.m_networking.Disconnect(drop_con);
    }

    // remove empty lobby player entries.  these will occur if AIs are dropped
    // from the lobby.  this will also occur when humans are dropped, but those
    // cases should have been handled above when checking the lobby data for
    // each player connection.
    std::list<std::pair<int, PlayerSetupData> >::iterator player_setup_it = m_lobby_data->m_players.begin();
    while (player_setup_it != m_lobby_data->m_players.end()) {
        if (player_setup_it->second.m_client_type == Networking::INVALID_CLIENT_TYPE) {
            std::list<std::pair<int, PlayerSetupData> >::iterator erase_it = player_setup_it;
            ++player_setup_it;
            m_lobby_data->m_players.erase(erase_it);
        } else {
            ++player_setup_it;
        }
    }

    static int AI_count = 1;
    static int nameless_player_count = 1;

    // assign unique names / colours to any lobby entry that lacks them, or
    // remove empire / colours from observers
    for (std::pair<int, PlayerSetupData>& entry : m_lobby_data->m_players) {
        PlayerSetupData& psd = entry.second;
        if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
            psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        {
            psd.m_empire_color = GG::Clr(0, 0, 0, 0);
            // On OSX the following two lines must not be included.
            // Clearing empire name and starting species name from
            // PlayerSetupData causes a weird crash (bus error) deep
            // in GG code on OSX in the Multiplayer Lobby when selecting
            // Observer or Moderator as client type.
#ifndef FREEORION_MACOSX
            psd.m_empire_name.clear();
            psd.m_starting_species_name.clear();
#endif
            psd.m_save_game_empire_id = ALL_EMPIRES;

        } else if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
            if (psd.m_empire_color == GG::Clr(0, 0, 0, 0))
                psd.m_empire_color = GetUnusedEmpireColour(m_lobby_data->m_players);
            if (psd.m_player_name.empty())
                psd.m_player_name = UserString("AI_PLAYER") + "_" + boost::lexical_cast<std::string>(AI_count++);
            if (psd.m_empire_name.empty())
                psd.m_empire_name = GenerateEmpireName(m_lobby_data->m_players);
            if (psd.m_starting_species_name.empty()) {
                if (m_lobby_data->m_seed!="")
                    psd.m_starting_species_name = GetSpeciesManager().RandomPlayableSpeciesName();
                else
                    psd.m_starting_species_name = GetSpeciesManager().SequentialPlayableSpeciesName(AI_count);
            }

        } else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
            if (psd.m_empire_color == GG::Clr(0, 0, 0, 0))
                psd.m_empire_color = GetUnusedEmpireColour(m_lobby_data->m_players);
            if (psd.m_player_name.empty())
                psd.m_player_name = UserString("PLAYER") + "_" + boost::lexical_cast<std::string>(nameless_player_count++);
            if (psd.m_empire_name.empty())
                psd.m_empire_name = psd.m_player_name;
            if (psd.m_starting_species_name.empty())
                psd.m_starting_species_name = GetSpeciesManager().RandomPlayableSpeciesName();
        }
    }


    // to determine if a new save file was selected, check if the selected file
    // index is different, and the new file index is in the valid range
    bool new_save_file_selected = false;
    std::string new_file = incoming_lobby_data.m_save_game;
    std::string old_file = m_lobby_data->m_save_game;
    if (new_file != old_file) {
        new_save_file_selected = true;

        // update selected file index
        m_lobby_data->m_save_game = new_file;

        // reset assigned empires in save game for all players.  new loaded game may not have the same set of empire IDs to choose from
        for (std::pair<int, PlayerSetupData>& psd : m_lobby_data->m_players) {
            psd.second.m_save_game_empire_id = ALL_EMPIRES;
        }

        // refresh save game empire data
        boost::filesystem::path save_dir(GetSaveDir());
        try {
            LoadEmpireSaveGameData((save_dir / m_lobby_data->m_save_game).string(),
                                   m_lobby_data->m_save_game_empire_data);
        } catch (const std::exception&) {
            // inform player who attempted to change the save file that there was a problem
            const PlayerConnectionPtr& player_connection = msg.m_player_connection;
            player_connection->SendMessage(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), false));
            // revert to old save file
            m_lobby_data->m_save_game = old_file;
        }
    }

    // propagate lobby changes to players, so everyone has the latest updated
    // version of the lobby data
    for (ServerNetworking::const_established_iterator player_connection_it = server.m_networking.established_begin();
         player_connection_it != server.m_networking.established_end(); ++player_connection_it)
    {
        PlayerConnectionPtr player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        // new save file update needs to be sent to everyone, as does an update
        // after a player is added or dropped.  otherwise, messages can just go
        // to players who didn't send the message that this function is
        // responding to.  TODO: check for add/drop
        if (new_save_file_selected || player_setup_data_changed || player_id != message.SendingPlayer())
            player_connection->SendMessage(ServerLobbyUpdateMessage(player_id, *m_lobby_data));
    }

    return discard_event();
}

sc::result MPLobby::react(const LobbyChat& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.LobbyChat";
    ServerApp& server = Server();
    const Message& message = msg.m_message;

    if (message.ReceivingPlayer() == Networking::INVALID_PLAYER_ID) { // the receiver is everyone (except the sender)
        for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin(); it != server.m_networking.established_end(); ++it) {
            if ((*it)->PlayerID() != message.SendingPlayer())
                (*it)->SendMessage(ServerLobbyChatMessage(message.SendingPlayer(), (*it)->PlayerID(), message.Text()));
        }
    } else {
        server.m_networking.SendMessage(ServerLobbyChatMessage(message.SendingPlayer(), message.ReceivingPlayer(), message.Text()));
    }

    return discard_event();
}

sc::result MPLobby::react(const StartMPGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.StartMPGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    if (server.m_networking.PlayerIsHost(player_connection->PlayerID())) {
        if (m_lobby_data->m_new_game) {
            // if all expected player already connected, can skip waiting for
            // MP joiners and go directly to playing game
            int expected_players = m_lobby_data->m_players.size();
            if (expected_players == static_cast<int>(server.m_networking.NumEstablishedPlayers())) {
                server.NewMPGameInit(*m_lobby_data);
                return transit<PlayingGame>();
            }
            // otherwise, transit to waiting for MP joiners

        } else {
            // Load game...
            std::string save_filename = (GetSaveDir() / m_lobby_data->m_save_game).string();

            try {
                LoadGame(save_filename,             *m_server_save_game_data,
                         m_player_save_game_data,   GetUniverse(),
                         Empires(),                 GetSpeciesManager(),
                         GetCombatLogManager(),     server.m_galaxy_setup_data);
                int seed = 0;
                try {
                    DebugLogger() << "Seeding with loaded galaxy seed: " << server.m_galaxy_setup_data.m_seed;
                    seed = boost::lexical_cast<unsigned int>(server.m_galaxy_setup_data.m_seed);
                } catch (...) {
                    try {
                        boost::hash<std::string> string_hash;
                        std::size_t h = string_hash(server.m_galaxy_setup_data.m_seed);
                        seed = static_cast<unsigned int>(h);
                    } catch (...) {}
                }
                Seed(seed);

            } catch (const std::exception&) {
                SendMessageToAllPlayers(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), true));
                return discard_event();
            }

            // if no AI clients need to be started, can go directly to playing game
            int expected_players = m_player_save_game_data.size();
            int needed_AI_clients = expected_players - server.m_networking.NumEstablishedPlayers();
            if (needed_AI_clients < 1) {
                server.LoadMPGameInit(*m_lobby_data,
                                      m_player_save_game_data,
                                      m_server_save_game_data);
                return transit<PlayingGame>();
            }
            // othewrise, transit to waiting for mp joiners
        }
    } else {
        ErrorLogger() << "(ServerFSM) MPLobby.StartMPGame : Player #" << message.SendingPlayer()
                               << " attempted to initiate a game load, but is not the host.  Terminating connection.";
        server.m_networking.Disconnect(player_connection);
        return discard_event();
    }

    // copy locally stored data to common server fsm context so it can be
    // retreived in WaitingForMPGameJoiners
    context<ServerFSM>().m_lobby_data = m_lobby_data;
    context<ServerFSM>().m_player_save_game_data = m_player_save_game_data;
    context<ServerFSM>().m_server_save_game_data = m_server_save_game_data;

    return transit<WaitingForMPGameJoiners>();
}

sc::result MPLobby::react(const HostMPGame& msg) {
    ErrorLogger() << "MPLobby::react(const HostMPGame& msg) recived HostMPGame message but is already in the MP Lobby.  Aborting connection";
    msg.m_player_connection->SendMessage(ErrorMessage(UserStringNop("SERVER_ALREADY_HOSTING_GAME"), true));
    Server().m_networking.Disconnect(msg.m_player_connection);
    return discard_event();
}

sc::result MPLobby::react(const HostSPGame& msg) {
    ErrorLogger() << "MPLobby::react(const HostSPGame& msg) recived HostSPGame message but is already in the MP Lobby.  Aborting connection";
    msg.m_player_connection->SendMessage(ErrorMessage(UserStringNop("SERVER_ALREADY_HOSTING_GAME"), true));
    Server().m_networking.Disconnect(msg.m_player_connection);
    return discard_event();
}

sc::result MPLobby::react(const Error& msg) {
    HandleErrorMessage(msg, Server());
    return discard_event();
}

////////////////////////////////////////////////////////////
// WaitingForSPGameJoiners
////////////////////////////////////////////////////////////
WaitingForSPGameJoiners::WaitingForSPGameJoiners(my_context c) :
    my_base(c),
    m_single_player_setup_data(context<ServerFSM>().m_single_player_setup_data),
    m_server_save_game_data(new ServerSaveGameData()),
    m_num_expected_players(0)
{
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForSPGameJoiners";

    context<ServerFSM>().m_single_player_setup_data.reset();
    ServerApp& server = Server();
    std::vector<PlayerSetupData>& players = m_single_player_setup_data->m_players;

    // Ensure all players have unique non-empty names   // TODO: the uniqueness part...
    unsigned int player_num = 1;
    for (PlayerSetupData& psd : players) {
        if (psd.m_player_name.empty()) {
            if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
                psd.m_player_name = "AI_" + boost::lexical_cast<std::string>(player_num++);
            else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                psd.m_player_name = "Human_Player_" + boost::lexical_cast<std::string>(player_num++);
            else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
                psd.m_player_name = "Observer_" + boost::lexical_cast<std::string>(player_num++);
            else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
                psd.m_player_name = "Moderator_" + boost::lexical_cast<std::string>(player_num++);
            else
                psd.m_player_name = "Player_" + boost::lexical_cast<std::string>(player_num++);
        }
    }

    if (m_single_player_setup_data->m_new_game) {
        // DO NOTHING

        // for new games, single player setup data contains full m_players
        // vector, so can just use the contents of that to create AI
        // clients

    } else {
        // for loaded games, all that is specified is the filename, and the
        // server needs to populate single player setup data's m_players
        // with data from the save file.
        if (!players.empty()) {
            ErrorLogger() << "WaitingForSPGameJoiners::WaitingForSPGameJoiners got single player setup data to load a game, but also player setup data for a new game.  Ignoring player setup data";
            players.clear();
        }

        std::vector<PlayerSaveHeaderData> player_save_header_data;
        try {
            LoadPlayerSaveHeaderData(m_single_player_setup_data->m_filename, player_save_header_data);
        } catch (const std::exception& e) {
            SendMessageToHost(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), true));
            post_event(LoadSaveFileFailed());
            return;
        }

        // add player setup data for each player in saved gamed
        for (const PlayerSaveHeaderData& psgd : player_save_header_data) {
            if (psgd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
                psgd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            {
                PlayerSetupData psd;
                psd.m_player_name =         psgd.m_name;
                //psd.m_empire_name // left default
                //psd.m_empire_color // left default
                //psd.m_starting_species_name // left default
                psd.m_save_game_empire_id = psgd.m_empire_id;
                psd.m_client_type =         psgd.m_client_type;
                players.push_back(psd);
            }
        }
    }

    m_num_expected_players = players.size();
    m_expected_ai_player_names.clear();
    for (const PlayerSetupData& psd : players)
        if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            m_expected_ai_player_names.insert(psd.m_player_name);

    server.CreateAIClients(players, m_single_player_setup_data->m_ai_aggr);    // also disconnects any currently-connected AI clients

    // force immediate check if all expected AIs are present, so that the FSM
    // won't get stuck in this state waiting for JoinGame messages that will
    // never come since no other AIs are left to join
    post_event(CheckStartConditions());
}

WaitingForSPGameJoiners::~WaitingForSPGameJoiners()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~WaitingForSPGameJoiners"; }

sc::result WaitingForSPGameJoiners::react(const JoinGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForSPGameJoiners.JoinGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name("Default_Player_Name_in_WaitingForSPGameJoiners::react(const JoinGame& msg)");
    Networking::ClientType client_type(Networking::INVALID_CLIENT_TYPE);
    std::string client_version_string;
    ExtractMessageData(message, player_name, client_type, client_version_string);

    int player_id = server.m_networking.NewPlayerID();

    // is this an AI?
    if (client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
        // verify that player name was expected
        if (m_expected_ai_player_names.find(player_name) == m_expected_ai_player_names.end()) {
            // unexpected ai player
            ErrorLogger() << "WaitingForSPGameJoiners::react(const JoinGame& msg) received join game message for player \"" << player_name << "\" which was not an expected AI player name.    Terminating connection.";
            server.m_networking.Disconnect(player_connection);
        } else {
            // expected player
            // let the networking system know what socket this player is on
            player_connection->EstablishPlayer(player_id, player_name, client_type, client_version_string);
            player_connection->SendMessage(JoinAckMessage(player_id));

            // remove name from expected names list, so as to only allow one connection per AI
            m_expected_ai_player_names.erase(player_name);
        }

    } else if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
        // verify that there is room left for this player
        int already_connected_players = m_expected_ai_player_names.size() + server.m_networking.NumEstablishedPlayers();
        if (already_connected_players >= m_num_expected_players) {
            // too many human players
            ErrorLogger() << "WaitingForSPGameJoiners::react(const JoinGame& msg): A human player attempted to join the game but there was not enough room.  Terminating connection.";
            // TODO: send message to attempted joiner saying game is full
            server.m_networking.Disconnect(player_connection);
        } else {
            // expected human player
            player_connection->EstablishPlayer(player_id, player_name, client_type, client_version_string);
            player_connection->SendMessage(JoinAckMessage(player_id));
        }
    } else {
        ErrorLogger() << "WaitingForSPGameJoiners::react(const JoinGame& msg): Received JoinGame message with invalid client type: " << client_type;
        return discard_event();
    }

    post_event(CheckStartConditions());
    return discard_event();
}

sc::result WaitingForSPGameJoiners::react(const CheckStartConditions& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForSPGameJoiners.CheckStartConditions";
    ServerApp& server = Server();

    // if all expected players have connected, proceed to start new or load game
    if (static_cast<int>(server.m_networking.NumEstablishedPlayers()) == m_num_expected_players) {
        DebugLogger() << "WaitingForSPGameJoiners::react(const CheckStartConditions& u) : have all " << m_num_expected_players << " expected players connected.";
        if (m_single_player_setup_data->m_new_game) {
            DebugLogger() << "Initializing new SP game...";
            server.NewSPGameInit(*m_single_player_setup_data);
        } else {
            DebugLogger() << "Loading SP game save file: " << m_single_player_setup_data->m_filename;
            try {
                LoadGame(m_single_player_setup_data->m_filename,            *m_server_save_game_data,
                         m_player_save_game_data,   GetUniverse(),          Empires(),
                         GetSpeciesManager(),       GetCombatLogManager(),  server.m_galaxy_setup_data);

            } catch (const std::exception&) {
                SendMessageToHost(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), true));
                return transit<Idle>();
            }

            server.LoadSPGameInit(m_player_save_game_data, m_server_save_game_data);
        }
        return transit<PlayingGame>();
    }

    return discard_event();
}

sc::result WaitingForSPGameJoiners::react(const LoadSaveFileFailed& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForSPGameJoiners.LoadSaveFileFailed";
    return transit<Idle>();
}

sc::result WaitingForSPGameJoiners::react(const Error& msg) {
    HandleErrorMessage(msg, Server());
    return discard_event();
}

////////////////////////////////////////////////////////////
// WaitingForMPGameJoiners
////////////////////////////////////////////////////////////
WaitingForMPGameJoiners::WaitingForMPGameJoiners(my_context c) :
    my_base(c),
    m_lobby_data(context<ServerFSM>().m_lobby_data),
    m_player_save_game_data(context<ServerFSM>().m_player_save_game_data),
    m_server_save_game_data(context<ServerFSM>().m_server_save_game_data),
    m_num_expected_players(0)
{
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForMPGameJoiners";
    context<ServerFSM>().m_lobby_data.reset();
    context<ServerFSM>().m_player_save_game_data.clear();
    context<ServerFSM>().m_server_save_game_data.reset();
    ServerApp& server = Server();

    m_num_expected_players = m_lobby_data->m_players.size();

    std::vector<PlayerSetupData> player_setup_data;
    m_expected_ai_player_names.clear();

    for (std::pair<int, PlayerSetupData>& psd : m_lobby_data->m_players) {
        player_setup_data.push_back(psd.second);
        if (psd.second.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            m_expected_ai_player_names.insert(psd.second.m_player_name);
    }

    server.CreateAIClients(player_setup_data, m_lobby_data->m_ai_aggr);

    // force immediate check if all expected AIs are present, so that the FSM
    // won't get stuck in this state waiting for JoinGame messages that will
    // never come since no other AIs are left to join
    post_event(CheckStartConditions());
}

WaitingForMPGameJoiners::~WaitingForMPGameJoiners()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~WaitingForMPGameJoiners"; }

sc::result WaitingForMPGameJoiners::react(const JoinGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForMPGameJoiners.JoinGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name("Default_Player_Name_in_WaitingForMPGameJoiners::react(const JoinGame& msg)");
    Networking::ClientType client_type(Networking::INVALID_CLIENT_TYPE);
    std::string client_version_string;
    ExtractMessageData(message, player_name, client_type, client_version_string);

    int player_id = server.m_networking.NewPlayerID();

    // is this an AI?
    if (client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
        // verify that player name was expected
        if (m_expected_ai_player_names.find(player_name) == m_expected_ai_player_names.end()) {
            // unexpected ai player
            ErrorLogger() << "WaitingForMPGameJoiners::react(const JoinGame& msg) received join game message for player \"" << player_name << "\" which was not an expected AI player name.    Terminating connection.";
            server.m_networking.Disconnect(player_connection);
        } else {
            // expected player
            // let the networking system know what socket this player is on
            player_connection->EstablishPlayer(player_id, player_name, client_type, client_version_string);
            player_connection->SendMessage(JoinAckMessage(player_id));

            // remove name from expected names list, so as to only allow one connection per AI
            m_expected_ai_player_names.erase(player_name);
        }

    } else if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
        // verify that there is room left for this player
        int already_connected_players = m_expected_ai_player_names.size() + server.m_networking.NumEstablishedPlayers();
        if (already_connected_players >= m_num_expected_players) {
            // too many human players
            ErrorLogger() << "WaitingForSPGameJoiners.JoinGame : A human player attempted to join the game but there was not enough room.  Terminating connection.";
            server.m_networking.Disconnect(player_connection);
        } else {
            // expected human player
            player_connection->EstablishPlayer(player_id, player_name, client_type, client_version_string);
            player_connection->SendMessage(JoinAckMessage(player_id));
        }
    } else {
        ErrorLogger() << "WaitingForMPGameJoiners::react(const JoinGame& msg): Received JoinGame message with invalid client type: " << client_type;
        return discard_event();
    }

    // force immediate check if all expected AIs are present, so that the FSM
    // won't get stuck in this state waiting for JoinGame messages that will
    // never come since no other AIs are left to join
    post_event(CheckStartConditions());

    return discard_event();
}

sc::result WaitingForMPGameJoiners::react(const CheckStartConditions& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForMPGameJoiners.CheckStartConditions";
    ServerApp& server = Server();

    if (static_cast<int>(server.m_networking.NumEstablishedPlayers()) == m_num_expected_players) {
        if (m_player_save_game_data.empty()) {
            DebugLogger() << "Initializing new MP game...";
            server.NewMPGameInit(*m_lobby_data);
        } else {
            DebugLogger() << "Initializing loaded MP game";
            server.LoadMPGameInit(*m_lobby_data, m_player_save_game_data, m_server_save_game_data);
        }
        return transit<PlayingGame>();
    }

    return discard_event();
}

sc::result WaitingForMPGameJoiners::react(const Error& msg) {
    HandleErrorMessage(msg, Server());
    return discard_event();
}


////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame(my_context c) :
    my_base(c)
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame"; }

PlayingGame::~PlayingGame()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~PlayingGame"; }

sc::result PlayingGame::react(const PlayerChat& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.PlayerChat";
    ServerApp& server = Server();
    for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        if (msg.m_message.ReceivingPlayer() == Networking::INVALID_PLAYER_ID ||
            msg.m_message.ReceivingPlayer() == (*it)->PlayerID())
        {
            (*it)->SendMessage(SingleRecipientChatMessage(msg.m_message.SendingPlayer(),
                                                          (*it)->PlayerID(),
                                                          msg.m_message.Text()));
        }
    }
    return discard_event();
}

sc::result PlayingGame::react(const Diplomacy& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.Diplomacy";
    const Message& message = msg.m_message;

    DiplomaticMessage diplo_message;
    ExtractMessageData(message, diplo_message);
    Empires().HandleDiplomaticMessage(diplo_message);

    return discard_event();
}

sc::result PlayingGame::react(const ModeratorAct& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.ModeratorAct";
    const Message& message = msg.m_message;
    int player_id = message.SendingPlayer();
    ServerApp& server = Server();

    const PlayerConnectionPtr& player_connection = msg.m_player_connection;
    Networking::ClientType client_type = player_connection->GetClientType();

    if (client_type != Networking::CLIENT_TYPE_HUMAN_MODERATOR) {
        ErrorLogger() << "PlayingGame::react(ModeratorAct): Non-moderator player sent moderator action, ignorning";
        return discard_event();
    }

    Moderator::ModeratorAction* action = 0;
    ExtractMessageData(message, action);

    DebugLogger() << "PlayingGame::react(ModeratorAct): " << (action ? action->Dump() : "(null)");

    if (action) {
        // execute action
        action->Execute();

        // update player(s) of changed gamestate as result of action
        bool use_binary_serialization = player_connection->ClientVersionStringMatchesThisServer();
        server.m_networking.SendMessage(TurnProgressMessage(Message::DOWNLOADING, player_id));
        server.m_networking.SendMessage(TurnPartialUpdateMessage(player_id, server.PlayerEmpireID(player_id),
                                                                 GetUniverse(), use_binary_serialization));
    }

    delete action;

    return discard_event();
}

sc::result PlayingGame::react(const Error& msg) {
    HandleErrorMessage(msg, Server());
    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForTurnEnd
////////////////////////////////////////////////////////////
WaitingForTurnEnd::WaitingForTurnEnd(my_context c) :
    my_base(c)
{
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEnd";
}

WaitingForTurnEnd::~WaitingForTurnEnd()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~WaitingForTurnEnd"; }

sc::result WaitingForTurnEnd::react(const TurnOrders& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEnd.TurnOrders";
    ServerApp& server = Server();
    const Message& message = msg.m_message;

    OrderSet* order_set = new OrderSet;
    ExtractMessageData(message, *order_set);

    assert(message.SendingPlayer() == msg.m_player_connection->PlayerID());

    int player_id = message.SendingPlayer();
    Networking::ClientType client_type = msg.m_player_connection->GetClientType();

    if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER) {
        // observers cannot submit orders. ignore.
        ErrorLogger() << "WaitingForTurnEnd::react(TurnOrders&) received orders from player "
                               << msg.m_player_connection->PlayerName()
                               << "(player id: " << message.SendingPlayer() << ") "
                               << "who is an observer and should not be sending orders. Orders being ignored.";
        server.m_networking.SendMessage(ErrorMessage(message.SendingPlayer(), UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();

    } else if (client_type == Networking::INVALID_CLIENT_TYPE) {
        // ??? lingering connection? shouldn't get to here. ignore.
        ErrorLogger() << "WaitingForTurnEnd::react(TurnOrders&) received orders from player "
                               << msg.m_player_connection->PlayerName()
                               << "(player id: " << message.SendingPlayer() << ") "
                               << "who has an invalid player type. The server is confused, and the orders being ignored.";
        server.m_networking.SendMessage(ErrorMessage(message.SendingPlayer(), UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();

    } else if (client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR) {
        // if the moderator ends the turn, it is done, regardless of what
        // players are doing or haven't done
        if (TRACE_EXECUTION) DebugLogger() << "WaitingForTurnEnd.TurnOrders : Moderator ended turn.";
        post_event(ProcessTurn());
        return transit<ProcessingTurn>();

    } else if (client_type == Networking::CLIENT_TYPE_AI_PLAYER ||
               client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
    {
        // store empire orders and resume waiting for more
        const Empire* empire = GetEmpire(server.PlayerEmpireID(player_id));
        if (!empire) {
            ErrorLogger() << "WaitingForTurnEnd::react(TurnOrders&) couldn't get empire for player with id:" << player_id;
            server.m_networking.SendMessage(ErrorMessage(message.SendingPlayer(), UserStringNop("EMPIRE_NOT_FOUND_CANT_HANDLE_ORDERS"), false));
            return discard_event();
        }

        for (const std::map<int, OrderPtr>::value_type& entry : *order_set) {
            OrderPtr order = entry.second;
            if (!order) {
                ErrorLogger() << "WaitingForTurnEnd::react(TurnOrders&) couldn't get order from order set!";
                continue;
            }
            if (empire->EmpireID() != order->EmpireID()) {
                ErrorLogger() << "WaitingForTurnEnd::react(TurnOrders&) received orders from player " << empire->PlayerName() << "(id: "
                                       << message.SendingPlayer() << ") who controls empire " << empire->EmpireID()
                                       << " but those orders were for empire " << order->EmpireID() << ".  Orders being ignored.";
                server.m_networking.SendMessage(ErrorMessage(message.SendingPlayer(), UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
                return discard_event();
            }
        }

        if (TRACE_EXECUTION) DebugLogger() << "WaitingForTurnEnd.TurnOrders : Received orders from player " << message.SendingPlayer();

        server.SetEmpireTurnOrders(empire->EmpireID(), order_set);
    }


    // notify other player that this player submitted orders
    for (ServerNetworking::const_established_iterator player_it = server.m_networking.established_begin();
         player_it != server.m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player_ctn = *player_it;
        player_ctn->SendMessage(PlayerStatusMessage(player_ctn->PlayerID(),
                                                    message.SendingPlayer(),
                                                    Message::WAITING));
    }

    // inform player who just submitted of their new status.  Note: not sure why
    // this only needs to be send to the submitting player and not all others as
    // well ...
    server.m_networking.SendMessage(TurnProgressMessage(Message::WAITING_FOR_PLAYERS,
                                                        message.SendingPlayer()));

    // if player who just submitted is the local human player, raise AI process priority
    // as raising process priority requires superuser privileges on OSX and Linux AFAIK,
    // this piece of code only makes sense on Windows systems
#ifdef FREEORION_WIN32
    if (server.IsLocalHumanPlayer(player_id)) {
        if (TRACE_EXECUTION) DebugLogger() << "WaitingForTurnEnd.TurnOrders : Orders received from local human player, raising AI process priority";
        server.SetAIsProcessPriorityToLow(false);
    }
#endif

    // check conditions for ending this turn
    post_event(CheckTurnEndConditions());

    return discard_event();
}

sc::result WaitingForTurnEnd::react(const RequestObjectID& msg) {
    //if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEnd.RequestObjectID";
    Server().m_networking.SendMessage(DispatchObjectIDMessage(msg.m_message.SendingPlayer(), GetUniverse().GenerateObjectID()));
    return discard_event();
}

sc::result WaitingForTurnEnd::react(const RequestDesignID& msg) {
    //if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEnd.RequestDesignID";
    Server().m_networking.SendMessage(DispatchDesignIDMessage(msg.m_message.SendingPlayer(), GetUniverse().GenerateDesignID()));
    return discard_event();
}

sc::result WaitingForTurnEnd::react(const CheckTurnEndConditions& c) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEnd.CheckTurnEndConditions";
    ServerApp& server = Server();
    // is there a moderator in the game?  If so, do nothing, as the turn does
    // not proceed until the moderator orders it
    if (server.m_networking.ModeratorsInGame())
        return discard_event();

    // no moderator; wait for all player orders to be submitted before
    // processing turn.
    if (server.AllOrdersReceived()) {
        // if all players have submitted orders, proceed to turn processing
        if (TRACE_EXECUTION) DebugLogger() << "WaitingForTurnEnd.TurnOrders : All orders received.";
        post_event(ProcessTurn());
        return transit<ProcessingTurn>();
    }

    return discard_event();
}


////////////////////////////////////////////////////////////
// WaitingForTurnEndIdle
////////////////////////////////////////////////////////////
WaitingForTurnEndIdle::WaitingForTurnEndIdle(my_context c) :
    my_base(c)
{
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEndIdle";
}

WaitingForTurnEndIdle::~WaitingForTurnEndIdle()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~WaitingForTurnEndIdle"; }

sc::result WaitingForTurnEndIdle::react(const SaveGameRequest& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEndIdle.SaveGameRequest";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    if (!server.m_networking.PlayerIsHost(player_connection->PlayerID())) {
        ErrorLogger() << "WaitingForTurnEndIdle.SaveGameRequest : Player #" << message.SendingPlayer()
                      << " attempted to initiate a game save, but is not the host.  Ignoring request connection.";
        player_connection->SendMessage(ErrorMessage(UserStringNop("NON_HOST_SAVE_REQUEST_IGNORED"), false));
        return discard_event();
    }

    context<WaitingForTurnEnd>().m_save_filename = message.Text();  // store requested save file name in Base state context so that sibling state can retreive it

    return transit<WaitingForSaveData>();
}


////////////////////////////////////////////////////////////
// WaitingForSaveData
////////////////////////////////////////////////////////////
WaitingForSaveData::WaitingForSaveData(my_context c) :
    my_base(c)
{
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForSaveData";

    ServerApp& server = Server();
    for (ServerNetworking::const_established_iterator player_it = server.m_networking.established_begin();
         player_it != server.m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player = *player_it;
        int player_id = player->PlayerID();
        player->SendMessage(ServerSaveGameDataRequestMessage(player_id, false));
        m_needed_reponses.insert(player_id);
    }
}

WaitingForSaveData::~WaitingForSaveData()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~WaitingForSaveData"; }

sc::result WaitingForSaveData::react(const ClientSaveData& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForSaveData.ClientSaveData";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    int player_id = player_connection->PlayerID();

    // extract client save information in message
    OrderSet received_orders;
    boost::shared_ptr<SaveGameUIData> ui_data(new SaveGameUIData);
    bool ui_data_available = false;
    std::string save_state_string;
    bool save_state_string_available = false;

    try {
        ExtractMessageData(message, received_orders, ui_data_available, *ui_data, save_state_string_available, save_state_string);
    } catch (const std::exception& e) {
        DebugLogger() << "WaitingForSaveData::react(const ClientSaveData& msg) received invalid save data from player " << player_connection->PlayerName();
        player_connection->SendMessage(ErrorMessage(UserStringNop("INVALID_CLIENT_SAVE_DATA_RECEIVED"), false));

        // TODO: use whatever portion of message data was extracted, and leave the rest as defaults.
    }

    // store recieved orders or already existing orders.  I'm not sure what's
    // going on here with the two possible sets of orders.  apparently the
    // received orders are ignored if there are already existing orders?
    boost::shared_ptr<OrderSet> order_set;
    if (const Empire* empire = GetEmpire(server.PlayerEmpireID(player_id))) {
        OrderSet* existing_orders = server.m_turn_sequence[empire->EmpireID()];
        if (existing_orders)
            order_set.reset(new OrderSet(*existing_orders));
        else
            order_set.reset(new OrderSet(received_orders));
    } else {
        ErrorLogger() << "WaitingForSaveData::react(const ClientSaveData& msg) couldn't get empire for player " << player_id;
        order_set.reset(new OrderSet(received_orders));
    }

    // ensure ui data availability flag is consistent with ui data
    if (!ui_data_available)
        ui_data.reset();

    // what type of client is this?
    Networking::ClientType client_type = player_connection->GetClientType();


    // pack data into struct
    m_player_save_game_data.push_back(
        PlayerSaveGameData(player_connection->PlayerName(),
                           server.PlayerEmpireID(message.SendingPlayer()),
                           order_set,       ui_data,    save_state_string,
                           client_type));


    // if all players have responded, proceed with save and continue game
    m_players_responded.insert(message.SendingPlayer());
    if (m_players_responded == m_needed_reponses) {
        ServerSaveGameData server_data(server.m_current_turn);

        // retreive requested save name from Base state, which should have been
        // set in WaitingForTurnEndIdle::react(const SaveGameRequest& msg)
        const std::string& save_filename = context<WaitingForTurnEnd>().m_save_filename;
        int bytes_written = 0;

        // save game...
        try {
            bytes_written = SaveGame(save_filename,     server_data,    m_player_save_game_data,
                                     GetUniverse(),     Empires(),      GetSpeciesManager(),
                                     GetCombatLogManager(),             server.m_galaxy_setup_data,
                                     !server.m_single_player_game);

        } catch (const std::exception&) {
            DebugLogger() << "Catch std::exception&";
            SendMessageToAllPlayers(ErrorMessage(UserStringNop("UNABLE_TO_WRITE_SAVE_FILE"), false));
        }

        // inform players that save is complete
        SendMessageToAllPlayers(ServerSaveGameCompleteMessage(save_filename, bytes_written));

        DebugLogger() << "Finished ClientSaveData from within if.";
        context<WaitingForTurnEnd>().m_save_filename = "";
        return transit<WaitingForTurnEndIdle>();
    }

    DebugLogger() << "Finished ClientSaveData from outside of if.";
    return discard_event();
}


////////////////////////////////////////////////////////////
// ProcessingTurn
////////////////////////////////////////////////////////////
ProcessingTurn::ProcessingTurn(my_context c) :
    my_base(c)
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ProcessingTurn"; }

ProcessingTurn::~ProcessingTurn()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~ProcessingTurn"; }

sc::result ProcessingTurn::react(const ProcessTurn& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ProcessingTurn.ProcessTurn";

    ServerApp& server = Server();

    // make sure all AI client processes are running with low priority
    server.SetAIsProcessPriorityToLow(true);

    server.PreCombatProcessTurns();
    server.ProcessCombats();
    server.PostCombatProcessTurns();

    // update players that other players are now playing their turn
    for (ServerNetworking::const_established_iterator player_it = server.m_networking.established_begin();
         player_it != server.m_networking.established_end();
         ++player_it)
    {
        PlayerConnectionPtr player_ctn = *player_it;
        if (player_ctn->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER ||
            player_ctn->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER ||
            player_ctn->GetClientType() == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
            player_ctn->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
        {
            // inform all players that this player is playing a turn
            for (ServerNetworking::const_established_iterator recipient_player_it = server.m_networking.established_begin();
                recipient_player_it != server.m_networking.established_end();
                ++recipient_player_it)
            {
                PlayerConnectionPtr recipient_player_ctn = *recipient_player_it;
                server.m_networking.SendMessage(
                    PlayerStatusMessage(recipient_player_ctn->PlayerID(), player_ctn->PlayerID(), Message::PLAYING_TURN));
            }
        }
    }

    return transit<WaitingForTurnEnd>();
}

sc::result ProcessingTurn::react(const CheckTurnEndConditions& c) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ProcessingTurn.CheckTurnEndConditions";
    return discard_event();
}
