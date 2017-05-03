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
#include <boost/asio/high_resolution_timer.hpp>

#include <iterator>


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

    void LogPlayerSetupData(const std::list<std::pair<int, PlayerSetupData>>& psd) {
        DebugLogger() << "PlayerSetupData:";
        for (const std::pair<int, PlayerSetupData>& entry : psd) {
            std::stringstream ss;
            ss << std::to_string(entry.first) << " : "
               << entry.second.m_player_name << ", ";
            switch (entry.second.m_client_type) {
            case Networking::CLIENT_TYPE_AI_PLAYER:         ss << "AI_PLAYER, ";    break;
            case Networking::CLIENT_TYPE_HUMAN_MODERATOR:   ss << "MODERATOR, ";    break;
            case Networking::CLIENT_TYPE_HUMAN_OBSERVER:    ss << "OBSERVER, ";     break;
            case Networking::CLIENT_TYPE_HUMAN_PLAYER:      ss << "HUMAN_PLAYER, "; break;
            default:                                        ss << "<invalid client type>, ";
            }
            ss << entry.second.m_starting_species_name;
            if (entry.second.m_player_ready)
                ss << ", Ready";
            DebugLogger() << " ... " << ss.str();
        }
    }

    std::string GenerateEmpireName(const std::string& player_name,
                                   std::list<std::pair<int, PlayerSetupData>>& players)
    {
        // load default empire names
        std::vector<std::string> empire_names = UserStringList("EMPIRE_NAMES");
        std::set<std::string> valid_names(empire_names.begin(), empire_names.end());
        for (const std::pair<int, PlayerSetupData>& psd : players) {
            std::set<std::string>::iterator name_it = valid_names.find(psd.second.m_empire_name);
            if (name_it != valid_names.end())
                valid_names.erase(name_it);
            name_it = valid_names.find(psd.second.m_player_name);
            if (name_it != valid_names.end())
                valid_names.erase(name_it);
        }
        if (!valid_names.empty()) {
            // pick a name from the list of empire names
            int empire_name_idx = RandSmallInt(0, static_cast<int>(valid_names.size()) - 1);
            return *std::next(valid_names.begin(), empire_name_idx);
        }
        // use a player_name as it unique among players
        return player_name;
    }


    /** Return true for fatal errors.*/
    bool HandleErrorMessage(const Error& msg, ServerApp &server) {
        std::string problem;
        bool fatal;
        int player_id;
        ExtractErrorMessageData(msg.m_message, player_id, problem, fatal);

        std::stringstream ss;

        ss << "Server received from player "
           << msg.m_player_connection->PlayerName() << "("
           << msg.m_player_connection->PlayerID() << ")"
           << (fatal?" a fatal":" an")
           << " error message: " << problem;

        if (fatal) {
            ErrorLogger() << ss.str();
            SendMessageToAllPlayers(ErrorMessage(problem, fatal, player_id));
        } else {
            ErrorLogger() << ss.str();
        }

        return fatal;
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
                (*it)->SendMessage(EndGameMessage(Message::PLAYER_DISCONNECT, player_connection->PlayerName()));
            }
        }
    }

    // independently of everything else, if there are no humans left, it's time to terminate
    if (m_server.m_networking.empty() || m_server.m_ai_client_processes.size() == m_server.m_networking.NumEstablishedPlayers()) {
        DebugLogger() << "ServerFSM::HandleNonLobbyDisconnection : All human players disconnected; server terminating.";
        m_server.m_fsm->process_event(ShutdownServer());
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
    ExtractHostMPGameMessageData(message, host_player_name, client_version_string);

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

    auto single_player_setup_data = std::make_shared<SinglePlayerSetupData>();
    std::string client_version_string;
    ExtractHostSPGameMessageData(message, *single_player_setup_data, client_version_string);


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

sc::result Idle::react(const ShutdownServer& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.ShutdownServer";

    return transit<ShuttingDownServer>();
}

sc::result Idle::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal)
        return transit<ShuttingDownServer>();
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
    int host_id = server.m_networking.HostPlayerID();
    const PlayerConnectionPtr& player_connection = *(server.m_networking.GetPlayer(host_id));

    ClockSeed();

    // create player setup data for host, and store in list
    m_lobby_data->m_players.push_back(std::make_pair(host_id, PlayerSetupData()));

    PlayerSetupData& player_setup_data = m_lobby_data->m_players.begin()->second;

    player_setup_data.m_player_name =           player_connection->PlayerName();
    player_setup_data.m_empire_name =           (player_connection->GetClientType() == Networking::CLIENT_TYPE_HUMAN_PLAYER) ? player_connection->PlayerName() : GenerateEmpireName(player_setup_data.m_player_name, m_lobby_data->m_players);
    player_setup_data.m_empire_color =          EmpireColors().at(0);               // since the host is the first joined player, it can be assumed that no other player is using this colour (unlike subsequent join game message responses)
    player_setup_data.m_starting_species_name = sm.RandomPlayableSpeciesName();
    // leaving save game empire id as default
    player_setup_data.m_client_type =           player_connection->GetClientType();

    player_connection->SendMessage(ServerLobbyUpdateMessage(*m_lobby_data));
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
        return transit<ShuttingDownServer>();
    }

    if (server.m_networking.PlayerIsHost(player_connection->PlayerID()))
        server.SelectNewHost();

    // if the disconnected player wasn't in the lobby, don't need to do anything more.
    // if player is in lobby, need to remove it
    int id = player_connection->PlayerID();
    bool player_was_in_lobby = false;
    for (std::list<std::pair<int, PlayerSetupData>>::iterator it = m_lobby_data->m_players.begin();
         it != m_lobby_data->m_players.end(); ++it)
    {
        if (it->first == id) {
            player_was_in_lobby = true;
            m_lobby_data->m_players.erase(it);
            break;
        }
    }
    if (player_was_in_lobby) {
        // drop ready flag as player list changed
        for (std::pair<int, PlayerSetupData>& plrs : m_lobby_data->m_players) {
            plrs.second.m_player_ready = false;
        }
    } else {
        DebugLogger() << "MPLobby.Disconnection : Disconnecting player (" << id << ") was not in lobby";
        return discard_event();
    }

    // send updated lobby data to players after disconnection-related changes
    for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        (*it)->SendMessage(ServerLobbyUpdateMessage(*m_lobby_data));
    }

    return discard_event();
}

namespace {
    GG::Clr GetUnusedEmpireColour(const std::list<std::pair<int, PlayerSetupData>>& psd) {
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

    // return true if player has important changes.
    bool IsPlayerChanged(const PlayerSetupData& lhs, const PlayerSetupData& rhs) {
        return (lhs.m_client_type != rhs.m_client_type) ||
            (lhs.m_starting_species_name != rhs.m_starting_species_name) ||
            (lhs.m_save_game_empire_id != rhs.m_save_game_empire_id) ||
            (lhs.m_empire_color != rhs.m_empire_color);
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
    ExtractJoinGameMessageData(message, player_name, client_type, client_version_string);

    std::string original_player_name = player_name;

    // Remove AI prefix to distinguish Human from AI.
    std::string ai_prefix = UserString("AI_PLAYER") + "_";
    if (client_type != Networking::CLIENT_TYPE_AI_PLAYER) {
        while (player_name.compare(0, ai_prefix.size(), ai_prefix) == 0)
            player_name.erase(0, ai_prefix.size());
    }
    if(player_name.empty())
        player_name = "_";

    std::string new_player_name = player_name;

    bool collision = true;
    std::size_t t = 1;
    while (t <= m_lobby_data->m_players.size() + 1 && collision) {
        collision = false;
        if (!server.IsAvailableName(new_player_name)) {
            collision = true;
        } else {
            for (std::pair<int, PlayerSetupData>& plr : m_lobby_data->m_players) {
                if (plr.second.m_empire_name == new_player_name) {
                    collision = true;
                    break;
                }
            }
        }

        if (collision)
            new_player_name = player_name + std::to_string(++t); // start alternative names from 2
    }

    if (collision) {
        player_connection->SendMessage(ErrorMessage(str(FlexibleFormat(UserString("ERROR_PLAYER_NAME_ALREADY_USED")) % original_player_name),
                                                    true));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }

    player_name = new_player_name;

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
    player_setup_data.m_empire_name =           (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) ? player_name : GenerateEmpireName(player_name, m_lobby_data->m_players);
    player_setup_data.m_empire_color =          GetUnusedEmpireColour(m_lobby_data->m_players);
    if (m_lobby_data->m_seed!="")
        player_setup_data.m_starting_species_name = sm.RandomPlayableSpeciesName();
    else
        player_setup_data.m_starting_species_name = sm.SequentialPlayableSpeciesName(player_id);

    // after setting all details, push into lobby data
    m_lobby_data->m_players.push_back(std::make_pair(player_id, player_setup_data));

    // drop ready player flag at new player
    for (std::pair<int, PlayerSetupData>& plr : m_lobby_data->m_players) {
        if (plr.second.m_empire_name == player_name) {
            // change empire name
            plr.second.m_empire_name = (plr.second.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) ? plr.second.m_player_name : GenerateEmpireName(plr.second.m_player_name, m_lobby_data->m_players);
        }

        plr.second.m_player_ready = false;
    }

    for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    { (*it)->SendMessage(ServerLobbyUpdateMessage(*m_lobby_data)); }

    return discard_event();
}

sc::result MPLobby::react(const LobbyUpdate& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.LobbyUpdate";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    MultiplayerLobbyData incoming_lobby_data;
    ExtractLobbyUpdateMessageData(message, incoming_lobby_data);

    // check if new lobby data changed player setup data.  if so, need to echo
    // back to sender with updated lobby details.
    bool player_setup_data_changed = false;
    // if got important lobby changes so players shoul reset their ready status
    bool has_important_changes = false;

    // store incoming lobby data.  clients can only change some of
    // this information (galaxy setup data, whether it is a new game and what
    // save file index to load) directly, so other data is skipped (list of
    // save files, save game empire data from the save file, player data)
    // during this copying and is updated below from the save file(s)

    if (server.m_networking.PlayerIsHost(sender->PlayerID())) {

        DebugLogger() << "Get message from host.";

        static int AI_count = 1;
        const GG::Clr CLR_NONE = GG::Clr(0, 0, 0, 0);

        // assign unique names / colours to any lobby entry that lacks them, or
        // remove empire / colours from observers
        for (std::pair<int, PlayerSetupData>& entry : incoming_lobby_data.m_players) {
            PlayerSetupData& psd = entry.second;
            if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
            {
                psd.m_empire_color = CLR_NONE;
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
                if (psd.m_empire_color == CLR_NONE)
                    psd.m_empire_color = GetUnusedEmpireColour(incoming_lobby_data.m_players);
                if (psd.m_player_name.empty())
                    // ToDo: Should we translate player_name?
                    psd.m_player_name = UserString("AI_PLAYER") + "_" + std::to_string(AI_count++);
                if (psd.m_empire_name.empty())
                    psd.m_empire_name = GenerateEmpireName(psd.m_player_name, incoming_lobby_data.m_players);
                if (psd.m_starting_species_name.empty()) {
                    if (m_lobby_data->m_seed != "")
                        psd.m_starting_species_name = GetSpeciesManager().RandomPlayableSpeciesName();
                    else
                        psd.m_starting_species_name = GetSpeciesManager().SequentialPlayableSpeciesName(AI_count);
                }

            } else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
                if (psd.m_empire_color == CLR_NONE)
                    psd.m_empire_color = GetUnusedEmpireColour(incoming_lobby_data.m_players);
                if (psd.m_empire_name.empty())
                    psd.m_empire_name = psd.m_player_name;
                if (psd.m_starting_species_name.empty())
                    psd.m_starting_species_name = GetSpeciesManager().RandomPlayableSpeciesName();
            }
        }

        bool has_collision = false;
        // check for color and names
        std::set<GG::Clr> psd_colors;
        std::set<std::string> psd_names;
        for (std::pair<int, PlayerSetupData>& player : incoming_lobby_data.m_players) {
            if (psd_colors.count(player.second.m_empire_color) ||
                psd_names.count(player.second.m_empire_name) ||
                psd_names.count(player.second.m_player_name))
            {
                has_collision = true;
                break;
            } else {
                psd_colors.emplace(player.second.m_empire_color);
                psd_names.emplace(player.second.m_empire_name);
                psd_names.emplace(player.second.m_player_name);
            }
        }

        if (has_collision) {
            player_setup_data_changed = true;
            for (std::pair<int, PlayerSetupData>& player : m_lobby_data->m_players) {
                if (player.first == sender->PlayerID()) {
                    player.second.m_player_ready = false;
                    break;
                }
            }
        } else {

            player_setup_data_changed = (incoming_lobby_data.m_players != m_lobby_data->m_players);

            // check if galaxy setup data changed
            has_important_changes = has_important_changes || (m_lobby_data->m_seed != incoming_lobby_data.m_seed) ||
                (m_lobby_data->m_size != incoming_lobby_data.m_size) ||
                (m_lobby_data->m_shape != incoming_lobby_data.m_shape) ||
                (m_lobby_data->m_age != incoming_lobby_data.m_age) ||
                (m_lobby_data->m_starlane_freq != incoming_lobby_data.m_starlane_freq) ||
                (m_lobby_data->m_planet_density != incoming_lobby_data.m_planet_density) ||
                (m_lobby_data->m_specials_freq != incoming_lobby_data.m_specials_freq) ||
                (m_lobby_data->m_monster_freq != incoming_lobby_data.m_monster_freq) ||
                (m_lobby_data->m_native_freq != incoming_lobby_data.m_native_freq) ||
                (m_lobby_data->m_ai_aggr != incoming_lobby_data.m_ai_aggr) ||
                (m_lobby_data->m_new_game != incoming_lobby_data.m_new_game);

            if (player_setup_data_changed) {
                if (m_lobby_data->m_players.size() != incoming_lobby_data.m_players.size()) {
                    has_important_changes = true; // drop ready at number of players changed
                } else {
                    for (std::pair<int, PlayerSetupData>& i_player : m_lobby_data->m_players) {
                        if (i_player.first < 0) // ignore changes in AI.
                            continue;
                        int player_id = i_player.first;
                        bool is_found_player = false;
                        for (std::pair<int, PlayerSetupData>& j_player : incoming_lobby_data.m_players) {
                            if (player_id == j_player.first) {
                                has_important_changes = has_important_changes || IsPlayerChanged(i_player.second, j_player.second);
                                is_found_player = true;
                                break;
                            }
                        }
                        has_important_changes = has_important_changes || (!is_found_player);
                    }
                }
            }

            // GalaxySetupData
            m_lobby_data->m_seed           = incoming_lobby_data.m_seed;
            m_lobby_data->m_size           = incoming_lobby_data.m_size;
            m_lobby_data->m_shape          = incoming_lobby_data.m_shape;
            m_lobby_data->m_age            = incoming_lobby_data.m_age;
            m_lobby_data->m_starlane_freq  = incoming_lobby_data.m_starlane_freq;
            m_lobby_data->m_planet_density = incoming_lobby_data.m_planet_density;
            m_lobby_data->m_specials_freq  = incoming_lobby_data.m_specials_freq;
            m_lobby_data->m_monster_freq   = incoming_lobby_data.m_monster_freq;
            m_lobby_data->m_native_freq    = incoming_lobby_data.m_native_freq;
            m_lobby_data->m_ai_aggr        = incoming_lobby_data.m_ai_aggr;

            // directly configurable lobby data
            m_lobby_data->m_new_game       = incoming_lobby_data.m_new_game;
            m_lobby_data->m_players        = incoming_lobby_data.m_players;

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
                std::list<std::pair<int, PlayerSetupData>>::iterator player_setup_it = m_lobby_data->m_players.begin();
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
            std::list<std::pair<int, PlayerSetupData>>::iterator player_setup_it = m_lobby_data->m_players.begin();
            while (player_setup_it != m_lobby_data->m_players.end()) {
                if (player_setup_it->second.m_client_type == Networking::INVALID_CLIENT_TYPE) {
                    std::list<std::pair<int, PlayerSetupData>>::iterator erase_it = player_setup_it;
                    ++player_setup_it;
                    m_lobby_data->m_players.erase(erase_it);
                } else {
                    ++player_setup_it;
                }
            }

        }
    } else {
        // can change only himself
        for (std::pair<int, PlayerSetupData>& i_player : m_lobby_data->m_players) {
            if (i_player.first != sender->PlayerID())
                continue;

            // found sender at m_lobby_data
            for (std::pair<int, PlayerSetupData>& j_player : incoming_lobby_data.m_players) {
                if (j_player.first != sender->PlayerID())
                    continue;

                // found sender at incoming_lobby_data

                // check for color and names
                std::set<GG::Clr> psd_colors;
                std::set<std::string> psd_names;
                for (std::pair<int, PlayerSetupData>& k_player : m_lobby_data->m_players) {
                    if (k_player.first == sender->PlayerID())
                        continue;

                    psd_colors.emplace(k_player.second.m_empire_color);
                    psd_names.emplace(k_player.second.m_empire_name);
                    psd_names.emplace(k_player.second.m_player_name);
                }

                // if we have collision unset ready flag and ignore changes
                if (psd_colors.count(j_player.second.m_empire_color) ||
                    psd_names.count(j_player.second.m_empire_name) ||
                    psd_names.count(j_player.second.m_player_name))
                {
                    i_player.second.m_player_ready = false;
                    player_setup_data_changed = true;
                } else {
                    has_important_changes = IsPlayerChanged(i_player.second, j_player.second);
                    player_setup_data_changed = ! (i_player.second == j_player.second);

                    i_player.second = std::move(j_player.second);
                }

                break;
            }
            break;
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
            sender->SendMessage(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), false));
            // revert to old save file
            m_lobby_data->m_save_game = old_file;
        }
    }

    if (has_important_changes) {
        for (std::pair<int, PlayerSetupData>& player : m_lobby_data->m_players)
            player.second.m_player_ready = false;
    } else {
        // check if all established human players ready to play
        bool is_all_ready = true;
        for (std::pair<int, PlayerSetupData>& player : m_lobby_data->m_players) {
            if ((player.first >= 0) && (player.second.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER ||
                player.second.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR ||
                player.second.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER))
            {
                if (! player.second.m_player_ready)
                    is_all_ready = false;
            }
        }

        if (is_all_ready) {
            // TODO: merge this code with MPLobby::react(const StartMPGame& msg)
            // start game

            if (!m_lobby_data->m_new_game) {
                // Load game ...
                std::string save_filename = (GetSaveDir() / m_lobby_data->m_save_game).string();

                try {
                    LoadGame(save_filename,             *m_server_save_game_data,
                             m_player_save_game_data,   GetUniverse(),
                             Empires(),                 GetSpeciesManager(),
                             GetCombatLogManager(),     server.m_galaxy_setup_data);
                    int seed = 0;
                    try {
                        seed = boost::lexical_cast<unsigned int>(server.m_galaxy_setup_data.m_seed);
                    } catch (...) {
                        try {
                            boost::hash<std::string> string_hash;
                            std::size_t h = string_hash(server.m_galaxy_setup_data.m_seed);
                            seed = static_cast<unsigned int>(h);
                        } catch (...) {}
                    }
                    DebugLogger() << "Seeding with loaded galaxy seed: " << server.m_galaxy_setup_data.m_seed << "  interpreted as actual seed: " << seed;
                    Seed(seed);

                } catch (const std::exception&) {
                    SendMessageToAllPlayers(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), true));
                    return discard_event();
                }
            }

            // copy locally stored data to common server fsm context so it can be
            // retreived in WaitingForMPGameJoiners
            context<ServerFSM>().m_lobby_data = m_lobby_data;
            context<ServerFSM>().m_player_save_game_data = m_player_save_game_data;
            context<ServerFSM>().m_server_save_game_data = m_server_save_game_data;

            return transit<WaitingForMPGameJoiners>();
        }
    }

    // propagate lobby changes to players, so everyone has the latest updated
    // version of the lobby data
    for (ServerNetworking::const_established_iterator player_connection_it = server.m_networking.established_begin();
         player_connection_it != server.m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr& player_connection = *player_connection_it;
        int player_id = player_connection->PlayerID();
        // new save file update needs to be sent to everyone, as does an update
        // after a player is added or dropped.  otherwise, messages can just go
        // to players who didn't send the message that this function is
        // responding to.  TODO: check for add/drop
        if (new_save_file_selected || player_setup_data_changed ||
            player_id != sender->PlayerID() || has_important_changes )
            player_connection->SendMessage(ServerLobbyUpdateMessage(*m_lobby_data));
    }

    return discard_event();
}

sc::result MPLobby::react(const PlayerChat& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.LobbyChat";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    std::string data;
    int receiver;
    ExtractPlayerChatMessageData(message, receiver, data);

    if (receiver == Networking::INVALID_PLAYER_ID) { // the receiver is everyone (except the sender)
        for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin(); it != server.m_networking.established_end(); ++it) {
            if ((*it)->PlayerID() != sender->PlayerID())
                (*it)->SendMessage(ServerPlayerChatMessage(sender->PlayerID(), data));
        }
    } else {
        ServerNetworking::const_established_iterator it = server.m_networking.GetPlayer(receiver);
        if (it != server.m_networking.established_end())
            (*it)->SendMessage(ServerPlayerChatMessage(sender->PlayerID(), data));
    }

    return discard_event();
}

sc::result MPLobby::react(const StartMPGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) MPLobby.StartMPGame";
    ServerApp& server = Server();
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    if (server.m_networking.PlayerIsHost(sender->PlayerID())) {
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
                    seed = boost::lexical_cast<unsigned int>(server.m_galaxy_setup_data.m_seed);
                } catch (...) {
                    try {
                        boost::hash<std::string> string_hash;
                        std::size_t h = string_hash(server.m_galaxy_setup_data.m_seed);
                        seed = static_cast<unsigned int>(h);
                    } catch (...) {}
                }
                DebugLogger() << "Seeding with loaded galaxy seed: " << server.m_galaxy_setup_data.m_seed << "  interpreted as actual seed: " << seed;
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
        ErrorLogger() << "(ServerFSM) MPLobby.StartMPGame : Player #" << sender->PlayerID()
                               << " attempted to initiate a game load, but is not the host.  Terminating connection.";
        server.m_networking.Disconnect(sender);
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

sc::result MPLobby::react(const ShutdownServer& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.ShutdownServer";

    return transit<ShuttingDownServer>();
}

sc::result MPLobby::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal)
        return transit<ShuttingDownServer>();
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
    int player_id = server.m_networking.NewPlayerID();

    if (m_single_player_setup_data->m_new_game) {
        // for new games, single player setup data contains full m_players
        // vector, so can just use the contents of that to create AI
        // clients
        for (auto& psd : players) {
            if (psd.m_player_name.empty()) {
                if (psd.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
                    psd.m_player_name = "AI_" + std::to_string(player_num++);
                else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
                    psd.m_player_name = "Human_Player_" + std::to_string(player_num++);
                else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
                    psd.m_player_name = "Observer_" + std::to_string(player_num++);
                else if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
                    psd.m_player_name = "Moderator_" + std::to_string(player_num++);
                else
                    psd.m_player_name = "Player_" + std::to_string(player_num++);
            }

            if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
                psd.m_player_id = server.Networking().HostPlayerID();
            } else {
                psd.m_player_id = player_id++;
            }
        }

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

                if (psd.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
                    psd.m_player_id = server.Networking().HostPlayerID();
                } else {
                    psd.m_player_id = player_id++;
                }

                players.push_back(psd);
            }
        }
    }

    m_num_expected_players = players.size();
    m_expected_ai_names_and_ids.clear();
    for (const auto& player_data : players) {
        if (player_data.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            m_expected_ai_names_and_ids.insert(std::make_pair(player_data.m_player_name, player_data.m_player_id));
    }

    server.CreateAIClients(players, m_single_player_setup_data->m_ai_aggr);    // also disconnects any currently-connected AI clients

    server.InitializePython();

    if (m_single_player_setup_data->m_new_game) {
        // For SP game start inializaing while waiting for AI callbacks.
        DebugLogger() << "Initializing new SP game...";
        server.NewSPGameInit(*m_single_player_setup_data);
    }

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
    ExtractJoinGameMessageData(message, player_name, client_type, client_version_string);

    // is this an AI?
    if (client_type == Networking::CLIENT_TYPE_AI_PLAYER) {
        const auto& expected_it = m_expected_ai_names_and_ids.find(player_name);
        // verify that player name was expected
        if (expected_it == m_expected_ai_names_and_ids.end()) {
            // unexpected ai player
            ErrorLogger() << "WaitingForSPGameJoiners::react(const JoinGame& msg) received join game message for player \"" << player_name << "\" which was not an expected AI player name.    Terminating connection.";
            server.m_networking.Disconnect(player_connection);
        } else {
            // expected player
            // let the networking system know what socket this player is on
            player_connection->EstablishPlayer(expected_it->second, player_name, client_type, client_version_string);
            player_connection->SendMessage(JoinAckMessage(expected_it->second));

            // remove name from expected names list, so as to only allow one connection per AI
            m_expected_ai_names_and_ids.erase(player_name);
        }

    } else if (client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER) {
        // verify that there is room left for this player
        int already_connected_players = m_expected_ai_names_and_ids.size() + server.m_networking.NumEstablishedPlayers();
        if (already_connected_players >= m_num_expected_players) {
            // too many human players
            ErrorLogger() << "WaitingForSPGameJoiners::react(const JoinGame& msg): A human player attempted to join the game but there was not enough room.  Terminating connection.";
            // TODO: send message to attempted joiner saying game is full
            server.m_networking.Disconnect(player_connection);
        } else {
            // unexpected but welcome human player
            int host_id = server.Networking().HostPlayerID();
            player_connection->EstablishPlayer(host_id, player_name, client_type, client_version_string);
            player_connection->SendMessage(JoinAckMessage(host_id));

            DebugLogger() << "Initializing new SP game...";
            server.NewSPGameInit(*m_single_player_setup_data);
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
            DebugLogger() << "Verify AIs SP game...";
            if (server.VerifySPGameAIs(*m_single_player_setup_data))
                server. SendNewGameStartMessages();

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
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal) {
        DebugLogger() << "fatal in joiners";
        return transit<ShuttingDownServer>();
    }
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

    server.InitializePython();

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
    ExtractJoinGameMessageData(message, player_name, client_type, client_version_string);

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
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal)
        return transit<ShuttingDownServer>();
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
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    std::string data;
    int receiver;
    ExtractPlayerChatMessageData(message, receiver, data);

    for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        if (receiver == Networking::INVALID_PLAYER_ID ||
            receiver == (*it)->PlayerID())
        {
            (*it)->SendMessage(ServerPlayerChatMessage(sender->PlayerID(),
                                                       data));
        }
    }
    return discard_event();
}

sc::result PlayingGame::react(const Diplomacy& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.Diplomacy";
    const Message& message = msg.m_message;

    DiplomaticMessage diplo_message;
    ExtractDiplomacyMessageData(message, diplo_message);
    Empires().HandleDiplomaticMessage(diplo_message);

    return discard_event();
}

sc::result PlayingGame::react(const ModeratorAct& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.ModeratorAct";
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;
    int player_id = sender->PlayerID();
    ServerApp& server = Server();

    Networking::ClientType client_type = sender->GetClientType();

    if (client_type != Networking::CLIENT_TYPE_HUMAN_MODERATOR) {
        ErrorLogger() << "PlayingGame::react(ModeratorAct): Non-moderator player sent moderator action, ignorning";
        return discard_event();
    }

    Moderator::ModeratorAction* action = nullptr;
    ExtractModeratorActionMessageData(message, action);

    DebugLogger() << "PlayingGame::react(ModeratorAct): " << (action ? action->Dump() : "(null)");

    if (action) {
        // execute action
        action->Execute();

        // update player(s) of changed gamestate as result of action
        bool use_binary_serialization = sender->ClientVersionStringMatchesThisServer();
        sender->SendMessage(TurnProgressMessage(Message::DOWNLOADING));
        sender->SendMessage(TurnPartialUpdateMessage(server.PlayerEmpireID(player_id),
                                                     GetUniverse(), use_binary_serialization));
    }

    delete action;

    return discard_event();
}

sc::result PlayingGame::react(const ShutdownServer& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) PlayingGame.ShutdownServer";

    return transit<ShuttingDownServer>();
}

sc::result PlayingGame::react(const RequestCombatLogs& msg) {
    DebugLogger() << "(ServerFSM) PlayingGame::RequestCombatLogs message received";
    Server().UpdateCombatLogs(msg.m_message, msg.m_player_connection);
    return discard_event();
}

sc::result PlayingGame::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal) {
        DebugLogger() << "Fatal received.";
        return transit<ShuttingDownServer>();
    }
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
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    OrderSet* order_set = new OrderSet;
    ExtractTurnOrdersMessageData(message, *order_set);

    int player_id = sender->PlayerID();
    Networking::ClientType client_type = sender->GetClientType();

    if (client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER) {
        // observers cannot submit orders. ignore.
        ErrorLogger() << "WaitingForTurnEnd::react(TurnOrders&) received orders from player "
                               << sender->PlayerName()
                               << "(player id: " << player_id << ") "
                               << "who is an observer and should not be sending orders. Orders being ignored.";
        sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();

    } else if (client_type == Networking::INVALID_CLIENT_TYPE) {
        // ??? lingering connection? shouldn't get to here. ignore.
        ErrorLogger() << "WaitingForTurnEnd::react(TurnOrders&) received orders from player "
                               <<sender->PlayerName()
                               << "(player id: " << player_id << ") "
                               << "who has an invalid player type. The server is confused, and the orders being ignored.";
        sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
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
            sender->SendMessage(ErrorMessage(UserStringNop("EMPIRE_NOT_FOUND_CANT_HANDLE_ORDERS"), false));
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
                                       << player_id << ") who controls empire " << empire->EmpireID()
                                       << " but those orders were for empire " << order->EmpireID() << ".  Orders being ignored.";
                sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
                return discard_event();
            }
        }

        if (TRACE_EXECUTION) DebugLogger() << "WaitingForTurnEnd.TurnOrders : Received orders from player " << player_id;

        server.SetEmpireTurnOrders(empire->EmpireID(), order_set);
    }


    // notify other player that this player submitted orders
    for (ServerNetworking::const_established_iterator player_it = server.m_networking.established_begin();
         player_it != server.m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player_ctn = *player_it;
        player_ctn->SendMessage(PlayerStatusMessage(player_id, Message::WAITING));
    }

    // inform player who just submitted of their new status.  Note: not sure why
    // this only needs to be send to the submitting player and not all others as
    // well ...
    sender->SendMessage(TurnProgressMessage(Message::WAITING_FOR_PLAYERS));

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
    msg.m_player_connection->SendMessage(DispatchObjectIDMessage(GetUniverse().GenerateObjectID()));
    return discard_event();
}

sc::result WaitingForTurnEnd::react(const RequestDesignID& msg) {
    //if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) WaitingForTurnEnd.RequestDesignID";
    msg.m_player_connection->SendMessage(DispatchDesignIDMessage(GetUniverse().GenerateDesignID()));
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
        ErrorLogger() << "WaitingForTurnEndIdle.SaveGameRequest : Player #" << player_connection->PlayerID()
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
        player->SendMessage(ServerSaveGameDataRequestMessage(false));
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
    auto ui_data = std::make_shared<SaveGameUIData>();
    bool ui_data_available = false;
    std::string save_state_string;
    bool save_state_string_available = false;

    try {
        ExtractClientSaveDataMessageData(message, received_orders, ui_data_available, *ui_data, save_state_string_available, save_state_string);
    } catch (const std::exception& e) {
        DebugLogger() << "WaitingForSaveData::react(const ClientSaveData& msg) received invalid save data from player " << player_connection->PlayerName();
        player_connection->SendMessage(ErrorMessage(UserStringNop("INVALID_CLIENT_SAVE_DATA_RECEIVED"), false));

        // TODO: use whatever portion of message data was extracted, and leave the rest as defaults.
    }

    // store recieved orders or already existing orders.  I'm not sure what's
    // going on here with the two possible sets of orders.  apparently the
    // received orders are ignored if there are already existing orders?
    std::shared_ptr<OrderSet> order_set;
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
                           server.PlayerEmpireID(player_connection->PlayerID()),
                           order_set,       ui_data,    save_state_string,
                           client_type));


    // if all players have responded, proceed with save and continue game
    m_players_responded.insert(player_connection->PlayerID());
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
                const PlayerConnectionPtr& recipient_player_ctn = *recipient_player_it;
                recipient_player_ctn->SendMessage(PlayerStatusMessage(player_ctn->PlayerID(),
                                                                      Message::PLAYING_TURN));
            }
        }
    }

    return transit<WaitingForTurnEnd>();
}

sc::result ProcessingTurn::react(const CheckTurnEndConditions& c) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ProcessingTurn.CheckTurnEndConditions";
    return discard_event();
}


////////////////////////////////////////////////////////////
// ShuttingDownServer
////////////////////////////////////////////////////////////
const auto SHUTDOWN_POLLING_TIME = std::chrono::milliseconds(5000);

ShuttingDownServer::ShuttingDownServer(my_context c) :
    my_base(c),
    m_player_id_ack_expected(),
    m_timeout(Server().m_io_service, Clock::now() + SHUTDOWN_POLLING_TIME)
{
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ShuttingDownServer";

    ServerApp& server = Server();

    if (server.m_ai_client_processes.empty() && server.m_networking.empty())
        throw ServerApp::NormalExitException();

    DebugLogger() << "ShuttingDownServer informing AIs game is ending";

    // Inform all players that the game is ending.  Only check the AIs for acknowledgement, because
    // they are the server's child processes.
    for (PlayerConnectionPtr player : server.m_networking) {
        auto good_connection = player->SendMessage(EndGameMessage(Message::PLAYER_DISCONNECT));
        if (player->GetClientType() == Networking::CLIENT_TYPE_AI_PLAYER) {
            if (good_connection) {
                // Only expect acknowledgement from sockets that are up.
                m_player_id_ack_expected.insert(player->PlayerID());
            }
        }
    }

    DebugLogger() << "ShuttingDownServer expecting " << m_player_id_ack_expected.size() << " AIs to ACK shutdown.";

    // Set the timeout.  If all clients have not responded then kill the remainder and exit
    m_timeout.async_wait(boost::bind(&ServerApp::ShutdownTimedoutHandler,
                                     &server,
                                     boost::asio::placeholders::error));

    post_event(CheckEndConditions());
}

ShuttingDownServer::~ShuttingDownServer()
{ if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ~ShuttingDownServer"; }

sc::result ShuttingDownServer::react(const LeaveGame& msg) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ShuttingDownServer.LeaveGame";
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;
    int player_id = player_connection->PlayerID();

    auto ack_found = m_player_id_ack_expected.find(player_id);

    if (ack_found != m_player_id_ack_expected.end()) {
        DebugLogger() << "Shutdown ACK received for AI " << player_id;
        m_player_id_ack_expected.erase(ack_found);
    } else {
        WarnLogger() << "Unexpected shutdown ACK received for AI " << player_id;
    }

    post_event(CheckEndConditions());
    return discard_event();
}

sc::result ShuttingDownServer::react(const Disconnection& d) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ShuttingDownServer.Disconnection";
    PlayerConnectionPtr& player_connection = d.m_player_connection;
    int player_id = player_connection->PlayerID();

    // Treat disconnection as an implicit ACK.  Otherwise ignore it.
    auto ack_found = m_player_id_ack_expected.find(player_id);

    if (ack_found != m_player_id_ack_expected.end()) {
        DebugLogger() << "Disconnect received for AI " << player_id << ".  Treating it as shutdown ACK.";
        m_player_id_ack_expected.erase(ack_found);
    }

    post_event(CheckEndConditions());
    return discard_event();
}

sc::result ShuttingDownServer::react(const CheckEndConditions& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ShuttingDownServer.CheckEndConditions";
    ServerApp& server = Server();

    auto all_acked = m_player_id_ack_expected.empty();

    if (all_acked) {
        DebugLogger() << "All " << server.m_ai_client_processes.size() << " AIs acknowledged shutdown request.";

        // Free the processes so that they can complete their shutdown.
        for (Process& process : server.m_ai_client_processes)
        { process.Free(); }

        post_event(DisconnectClients());
    }

    return discard_event();
}

sc::result ShuttingDownServer::react(const DisconnectClients& u) {
    if (TRACE_EXECUTION) DebugLogger() << "(ServerFSM) ShuttingDownServer.DisconnectClients";
    ServerApp& server = Server();

    // Remove the ai processes.  They either all acknowledged the shutdown and are free or were all killed.
    server.m_ai_client_processes.clear();

    // Disconnect
    server.m_networking.DisconnectAll();

    throw ServerApp::NormalExitException();

    // Never reached.
    return discard_event();
}

sc::result ShuttingDownServer::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal)
        return transit<ShuttingDownServer>();
    return discard_event();
}
