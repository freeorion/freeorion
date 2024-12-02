#include "ServerFSM.h"

#include "SaveLoad.h"
#include "ServerApp.h"
#include "ServerNetworking.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/System.h"
#include "../universe/Species.h"
#include "../network/Message.h"
#include "../util/Directories.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/LoggerWithOptionsDB.h"
#include "../util/Order.h"
#include "../util/OrderSet.h"
#include "../util/Random.h"
#include "../util/ModeratorAction.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iterator>


class CombatLogManager;
CombatLogManager&   GetCombatLogManager();

namespace {
    DeclareThreadSafeLogger(FSM);

    constexpr EmpireColor CLR_SERVER{{255, 255, 255, 255}};
    constexpr EmpireColor CLR_ZERO{{0, 0, 0, 0}};

    void SendMessageToAllPlayers(const Message& message) {
        ServerApp* server = ServerApp::GetApp();
        if (!server) {
            ErrorLogger(FSM) << "SendMessageToAllPlayers couldn't get server.";
            return;
        }
        ServerNetworking& networking = server->Networking();

        for (auto player_it = networking.established_begin();
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
            ErrorLogger(FSM) << "SendMessageToHost couldn't get server.";
            return;
        }
        ServerNetworking& networking = server->Networking();

        auto host_it = networking.GetPlayer(networking.HostPlayerID());
        if (host_it == networking.established_end()) {
            ErrorLogger(FSM) << "SendMessageToHost couldn't get host player.";
            return;
        }

        PlayerConnectionPtr host = *host_it;
        host->SendMessage(message);
    }

    std::string GetHostNameFromSinglePlayerSetupData(const SinglePlayerSetupData& single_player_setup_data) {
        if (single_player_setup_data.new_game) {
            // for new games, get host player's name from PlayerSetupData for the
            // (should be only) human player
            for (const PlayerSetupData& psd : single_player_setup_data.players) {
                // In a single player game, the host player is always the human player, so
                // this is just a matter of finding which player setup data is for
                // a human player, and assigning that setup data to the host player id
                if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
                    return psd.player_name;
            }

        } else {
            // for loading saved games, get host / human player's name from save file
            if (!single_player_setup_data.players.empty())
                ErrorLogger(FSM) << "GetHostNameFromSinglePlayerSetupData got single player setup data to load a game, but also player setup data for a new game.  Ignoring player setup data";


            std::vector<PlayerSaveHeaderData> player_save_header_data;
            LoadPlayerSaveHeaderData(single_player_setup_data.filename, player_save_header_data);

            // find which player was the human (and thus the host) in the saved game
            for (const PlayerSaveHeaderData& psgd : player_save_header_data) {
                if (psgd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
                    return psgd.name;
            }
        }
        return "";
    }

    void LogPlayerSetupData(const std::list<std::pair<int, PlayerSetupData>>& psd) {
        DebugLogger(FSM) << "PlayerSetupData:";
        for (const std::pair<int, PlayerSetupData>& entry : psd) {
            std::stringstream ss;
            ss << std::to_string(entry.first) << " : "
               << entry.second.player_name << ", ";
            switch (entry.second.client_type) {
            case Networking::ClientType::CLIENT_TYPE_AI_PLAYER:         ss << "AI_PLAYER, ";    break;
            case Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR:   ss << "MODERATOR, ";    break;
            case Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER:    ss << "OBSERVER, ";     break;
            case Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER:      ss << "PLAYER, "; break;
            default:                                                    ss << "<invalid client type>, ";
            }
            EmpireColor empire_color = entry.second.empire_color;
            ss << "(" << static_cast<unsigned int>(std::get<0>(empire_color))
               << ", " << static_cast<unsigned int>(std::get<1>(empire_color))
               << ", " << static_cast<unsigned int>(std::get<2>(empire_color))
               << ", " << static_cast<unsigned int>(std::get<3>(empire_color)) << "), ";
            ss << entry.second.starting_species_name;
            if (entry.second.player_ready)
                ss << ", Ready";
            DebugLogger(FSM) << " ... " << ss.str();
        }
    }

    std::string GenerateEmpireName(const std::string& player_name,
                                   std::list<std::pair<int, PlayerSetupData>>& players)
    {
        // load default empire names
        auto empire_names = UserStringList("EMPIRE_NAMES");
        std::set<std::string> valid_names(empire_names.begin(), empire_names.end());
        for (const auto& psd : players) {
            auto name_it = valid_names.find(psd.second.empire_name);
            if (name_it != valid_names.end())
                valid_names.erase(name_it);
            name_it = valid_names.find(psd.second.player_name);
            if (name_it != valid_names.end())
                valid_names.erase(name_it);
        }
        if (!valid_names.empty()) {
            // pick a name from the list of empire names
            int empire_name_idx = RandInt(0, static_cast<int>(valid_names.size()) - 1);
            return *std::next(valid_names.begin(), empire_name_idx);
        }
        // use a player_name as it unique among players
        return player_name;
    }


    /** Return true for fatal errors.*/
    bool HandleErrorMessage(const Error& msg, ServerApp& server) {
        std::stringstream ss;
        std::string problem_key, unlocalized_info;
        bool fatal = false;
        int player_id = Networking::INVALID_PLAYER_ID;
        try {
            ExtractErrorMessageData(msg.m_message, player_id, problem_key, unlocalized_info, fatal);
        } catch (...) {
            problem_key = UserString("UNKNOWN");
        }
        ss << "Server received from player "
           << msg.m_player_connection->PlayerName() << "("
           << msg.m_player_connection->PlayerID() << ")"
           << (fatal?" a fatal":" an")
           << " error message: " << problem_key;
        if (!unlocalized_info.empty())
            ss << " info: " << unlocalized_info;
        ErrorLogger(FSM) << ss.str();

        if (fatal) {
            if (unlocalized_info.empty())
                SendMessageToAllPlayers(ErrorMessage(problem_key, fatal, player_id));
            else
                SendMessageToAllPlayers(ErrorMessage(problem_key, unlocalized_info, fatal, player_id));
        }

        return fatal;
    }

    std::string GetAutoSaveFileName(int current_turn, const GalaxySetupData& gsd) {
        const auto& subdir = gsd.GetGameUID();
        boost::filesystem::path autosave_dir_path = GetServerSaveDir() / (subdir.empty() ? "auto" : subdir);
        const auto& extension = MP_SAVE_FILE_EXTENSION;
        // Add timestamp to autosave generated files
        std::string datetime_str = FilenameTimestamp();

        std::string save_filename = boost::io::str(boost::format("FreeOrion_%04d_%s%s") % current_turn % datetime_str % extension);
        boost::filesystem::path save_path(autosave_dir_path / save_filename);
        return PathToString(save_path);
    }

    bool IsMultiplayerSaveFile(const boost::filesystem::path& path)
    { return IsExistingFile(path) && MP_SAVE_FILE_EXTENSION == path.extension(); }

    EmpireColor GetUnusedEmpireColour(const std::list<std::pair<int, PlayerSetupData>>& psd,
                                      const std::map<int, SaveGameEmpireData>& sged = std::map<int, SaveGameEmpireData>())
    {
        //DebugLogger(FSM) << "finding colours for empire of player " << player_name;
        EmpireColor empire_colour{{192, 192, 192, 255}};
        for (const EmpireColor possible_colour : EmpireColors()) {
            //DebugLogger(FSM) << "trying colour " << possible_colour.r << ", " << possible_colour.g << ", " << possible_colour.b;

            // check if any other player / empire is using this colour
            bool colour_is_new = true;
            for (const std::pair<int, PlayerSetupData>& entry : psd) {
                const auto player_colour = entry.second.empire_color;
                if (player_colour == possible_colour) {
                    colour_is_new = false;
                    break;
                }
            }

            if (colour_is_new) {
                for (const auto& entry : sged) {
                    const auto player_colour = entry.second.color;
                    if (player_colour == possible_colour) {
                        colour_is_new = false;
                        break;
                    }
                }
            }

            // use colour and exit loop if no other empire is using the colour
            if (colour_is_new) {
                empire_colour = possible_colour;
                break;
            }

            //DebugLogger(FSM) << " ... colour already used.";
        }
        return empire_colour;
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
MessageEventBase::MessageEventBase(const Message& message, const PlayerConnectionPtr& player_connection) :
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
    BOOST_PP_SEQ_FOR_EACH(MESSAGE_EVENT_CASE, _, MESSAGE_EVENTS)
    BOOST_PP_SEQ_FOR_EACH(MESSAGE_EVENT_CASE, _, NON_MESSAGE_SERVER_FSM_EVENTS)
#undef MESSAGE_EVENT_CASE

    if (terminated()) {
        ErrorLogger(FSM) << "A " << most_derived_message_type_str << " event was passed to "
            "the ServerFSM.  The FSM has terminated.  The event is being ignored.";
        return;
    }

    std::stringstream ss;
    ss << "[";
    for (auto leaf_state_it = state_begin(); leaf_state_it != state_end();) {
        // The following use of typeid assumes that
        // BOOST_STATECHART_USE_NATIVE_RTTI is defined
        const auto& leaf_state = *leaf_state_it;
        ss << typeid(leaf_state).name();
        ++leaf_state_it;
        if (leaf_state_it != state_end())
            ss << ", ";
    }
    ss << "]";

    ErrorLogger(FSM) << "A " << most_derived_message_type_str
                     << " event was not handled by any of these states : "
                     << ss.str() << ".  It is being ignored.";
}

void ServerFSM::HandleNonLobbyDisconnection(const Disconnection& d) {
    PlayerConnectionPtr& player_connection = d.m_player_connection;
    bool must_quit = false;
    int id = Networking::INVALID_PLAYER_ID;

    if (player_connection->IsEstablished()) {
        // update cookie expire date
        // so player could reconnect within 15 minutes
        m_server.Networking().UpdateCookie(player_connection->Cookie());

        id = player_connection->PlayerID();
        DebugLogger(FSM) << "ServerFSM::HandleNonLobbyDisconnection : Lost connection to player #" << id
                         << ", named \"" << player_connection->PlayerName() << "\".";

        if (player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            // AI could safely disconnect only if empire was eliminated
            auto empire = m_server.Empires().GetEmpire(m_server.PlayerEmpireID(id));
            if (empire) {
                if (!empire->Eliminated()) {
                    must_quit = true;
                    // AI abnormally disconnected during a regular game
                    ErrorLogger(FSM) << "AI Player #" << id << ", named \""
                                     << player_connection->PlayerName() << "\" quit before empire #"
                                     << empire->EmpireID() << " "
                                     << empire->Name() << " of player "
                                     << empire->PlayerName() << " was eliminated.";
                } else {
                    InfoLogger(FSM) << "AI Player #" << id << ", named \""
                                    << player_connection->PlayerName() << "\" killed after empire #"
                                    << empire->EmpireID() << " "
                                    << empire->Name() << " of player "
                                    << empire->PlayerName() << " was eliminated.";
                    // detach player from empire
                    m_server.DropPlayerEmpireLink(id);
                }
            }
        } else if (player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
            auto empire = m_server.Empires().GetEmpire(m_server.PlayerEmpireID(id));
            // eliminated and non-empire players can leave safely
            if (empire && !empire->Eliminated()) {
                // player abnormally disconnected during a regular game
                WarnLogger(FSM) << "Player #" << id << ", named \""
                                 << player_connection->PlayerName() << "\"quit before empire was eliminated.";
                // detach player from empire
                m_server.DropPlayerEmpireLink(id);
            }
        }
        // add "player left game" message
        boost::posix_time::ptime timestamp = boost::posix_time::second_clock::universal_time();
        std::string data = std::string("[[") + UserStringNop("PLAYER_LEFT_GAME") + "," + player_connection->PlayerName() + "]]";
        m_server.PushChatMessage(data, "", CLR_SERVER, timestamp);

        // send message to other players
        for (auto it = m_server.m_networking.established_begin();
             it != m_server.m_networking.established_end(); ++it)
        {
            if (player_connection != (*it))
                (*it)->SendMessage(ServerPlayerChatMessage(Networking::INVALID_PLAYER_ID, timestamp, data));
        }
    } else {
        DebugLogger(FSM) << "Client quit before id was assigned.";
    }

    // count of active (non-eliminated) empires, which currently have a connected human players
    int empire_connected_plr_cnt = 0;
    // count of active (non-eliminated) empires, which currently have a unconnected human players
    int empire_unconnected_plr_cnt = 0;
    for (const auto& empire : m_server.Empires()) {
        if (!empire.second->Eliminated()) {
            switch (m_server.GetEmpireClientType(empire.first)) {
            case Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER:
                empire_connected_plr_cnt++;
                break;
            case Networking::ClientType::INVALID_CLIENT_TYPE:
                empire_unconnected_plr_cnt++;
                break;
            case Networking::ClientType::CLIENT_TYPE_AI_PLAYER:
                // ignore
                break;
            default:
                ErrorLogger(FSM) << "Incorrect client type " << m_server.GetEmpireClientType(empire.first)
                                 << " for empire #" << empire.first;
                break;
            }
        }
    }

    // Stop server if connected human player empires count is less than minimum
    if (empire_connected_plr_cnt < GetOptionsDB().Get<int>("network.server.conn-human-empire-players.min")) {
        ErrorLogger(FSM) << "Too low connected human player " << empire_connected_plr_cnt
                         << " expected " << GetOptionsDB().Get<int>("network.server.conn-human-empire-players.min")
                         << "; server terminating.";
        must_quit = true;
    }

    // Stop server if unconnected human player empires count exceeds maximum and maximum is set
    if (GetOptionsDB().Get<int>("network.server.unconn-human-empire-players.max") > 0 &&
        empire_unconnected_plr_cnt >= GetOptionsDB().Get<int>("network.server.unconn-human-empire-players.max"))
    {
        ErrorLogger(FSM) << "Too high unconnected human player " << empire_unconnected_plr_cnt
                         << " expected " << GetOptionsDB().Get<int>("network.server.unconn-human-empire-players.max")
                         << "; server terminating.";
        must_quit = true;
    }

    m_server.Networking().CleanupCookies();

    if (must_quit) {
        ErrorLogger(FSM) << "Unable to recover server terminating.";
        if (m_server.IsHostless()) {
            if (GetOptionsDB().Get<bool>("save.auto.exit.enabled") &&
                m_server.CurrentTurn() > 0)
            {
                // save game on exit
                std::string save_filename = GetAutoSaveFileName(m_server.CurrentTurn(),
                                                                m_server.GetGalaxySetupData());
                ServerSaveGameData server_data{m_server.CurrentTurn()};
                int bytes_written = 0;
                // save game...
                try {
                    bytes_written = SaveGame(save_filename,             server_data,        m_server.GetPlayerSaveGameData(),
                                             m_server.GetUniverse(),    m_server.Empires(), m_server.GetSpeciesManager(),
                                             GetCombatLogManager(),     m_server.m_galaxy_setup_data,
                                             !m_server.m_single_player_game);
                } catch (const std::exception& error) {
                    ErrorLogger(FSM) << "While saving, catch std::exception: " << error.what();
                    SendMessageToAllPlayers(ErrorMessage(UserStringNop("UNABLE_TO_WRITE_SAVE_FILE"), false));
                }

                // inform players that save is complete
                SendMessageToAllPlayers(ServerSaveGameCompleteMessage(save_filename, bytes_written));
            }
            m_server.m_fsm->process_event(Hostless());
        } else {
            m_server.m_fsm->process_event(ShutdownServer());
        }
    } else {
        // can continue.  Select new host if necessary.
        if (m_server.m_networking.PlayerIsHost(id))
            m_server.SelectNewHost();

        // player list changed
        // notify those in ingame lobby
        UpdateIngameLobby();
    }
}

void ServerFSM::UpdateIngameLobby() {
    GalaxySetupData galaxy_data = m_server.GetGalaxySetupData();
    MultiplayerLobbyData dummy_lobby_data(std::move(galaxy_data));
    dummy_lobby_data.any_can_edit = false;
    dummy_lobby_data.new_game = false;
    dummy_lobby_data.in_game = true;
    dummy_lobby_data.start_locked = true;
    dummy_lobby_data.save_game_current_turn = m_server.CurrentTurn();
    dummy_lobby_data.save_game_empire_data = CompileSaveGameEmpireData(m_server.Empires());
    for (auto player_it = m_server.m_networking.established_begin();
        player_it != m_server.m_networking.established_end(); ++player_it)
    {
        PlayerSetupData player_setup_data;
        int player_id = (*player_it)->PlayerID();
        player_setup_data.player_id =   player_id;
        player_setup_data.player_name = (*player_it)->PlayerName();
        player_setup_data.client_type = (*player_it)->GetClientType();
        if (auto empire = m_server.Empires().GetEmpire(m_server.PlayerEmpireID(player_id))) {
            player_setup_data.save_game_empire_id = empire->EmpireID();
            player_setup_data.empire_name = empire->Name();
            player_setup_data.empire_color = empire->Color();
        } else {
            player_setup_data.empire_color = {{255, 255, 255, 255}};
        }
        player_setup_data.authenticated = (*player_it)->IsAuthenticated();
        dummy_lobby_data.players.emplace_back(player_id, player_setup_data);
    }
    dummy_lobby_data.start_lock_cause = UserStringNop("SERVER_ALREADY_PLAYING_GAME");

    auto player_info_map = m_server.GetPlayerInfoMap();

    // send it to all either in the ingame lobby
    // or in the playing game
    for (auto player_it = m_server.m_networking.established_begin();
        player_it != m_server.m_networking.established_end(); ++player_it)
    {
        if ((*player_it)->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER &&
            !m_server.GetEmpire(m_server.PlayerEmpireID((*player_it)->PlayerID())))
        {
            (*player_it)->SendMessage(ServerLobbyUpdateMessage(dummy_lobby_data));
        } else {
            (*player_it)->SendMessage(PlayerInfoMessage(player_info_map));
        }
    }
}

bool ServerFSM::EstablishPlayer(PlayerConnectionPtr player_connection,
                                std::string player_name, Networking::ClientType client_type,
                                std::string client_version_string, Networking::AuthRoles roles)
{
    std::vector<PlayerConnectionPtr> to_disconnect;
    to_disconnect.reserve(m_server.m_networking.size());

    // set and test roles
    player_connection->SetAuthRoles(roles);

    if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER &&
        !player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER))
    { client_type = Networking::ClientType::INVALID_CLIENT_TYPE; }
    if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR &&
        !player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_MODERATOR))
    { client_type = Networking::ClientType::INVALID_CLIENT_TYPE; }
    if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER &&
        !player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER))
    { client_type = Networking::ClientType::INVALID_CLIENT_TYPE; }

    if (player_connection->IsAuthenticated() || !player_connection->Cookie().is_nil()) {
        // drop other connection with same name
        for (auto it = m_server.m_networking.established_begin();
             it != m_server.m_networking.established_end(); ++it)
        {
            const auto& est_player = *it;
            if (est_player->PlayerName() == player_name && player_connection != est_player) {
                est_player->SendMessage(ErrorMessage(UserStringNop("ERROR_CONNECTION_WAS_REPLACED"), true));
                to_disconnect.push_back(est_player);

                // If we're going to establish Human Player
                // it will be better to break link with previous connection
                // so game won't be stopped on disconnection of previous connection.
                if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
                    m_server.DropPlayerEmpireLink(est_player->PlayerID());
                    est_player->SetClientType(Networking::ClientType::INVALID_CLIENT_TYPE);
                }
            }
        }
    }

    if (client_type == Networking::ClientType::INVALID_CLIENT_TYPE) {
        InfoLogger() << "ServerFSM::EstablishPlayer player " << player_name
                     << " has client type " << client_type
                     << " version string: " << client_version_string
                     << " and roles: " << roles.Text()
                     << " and is to be disconnected due to the invalid client type";
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CLIENT_TYPE_NOT_ALLOWED"), true));
        to_disconnect.push_back(std::move(player_connection));

    } else {
        // assign unique player ID to newly connected player
        int player_id = m_server.m_networking.NewPlayerID();
        DebugLogger() << "ServerFSM.EstablishPlayer Assign new player id " << player_id;

        // establish player with requested client type and acknowldge via connection
        player_connection->EstablishPlayer(player_id, player_name, client_type,
                                           std::move(client_version_string));

        // save cookie for player name
        boost::uuids::uuid cookie = player_connection->Cookie();
        // Don't generate cookie if player already have it
        if (cookie.is_nil())
            cookie = m_server.m_networking.GenerateCookie(player_name,
                                                          roles,
                                                          player_connection->IsAuthenticated());
        DebugLogger() << "ServerFSM.EstablishPlayer player " << player_name << " get cookie: " << cookie;
        player_connection->SetCookie(cookie);

        player_connection->SendMessage(JoinAckMessage(player_id, cookie));
        if (!GetOptionsDB().Get<bool>("skip-checksum"))
            player_connection->SendMessage(ContentCheckSumMessage(m_server.GetSpeciesManager()));

        // inform player of host
        player_connection->SendMessage(HostIDMessage(m_server.m_networking.HostPlayerID()));

        // send chat history
        if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR ||
            client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
            client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
        {
            // add "player enter game" message
            boost::posix_time::ptime timestamp = boost::posix_time::second_clock::universal_time();
            std::string data = std::string("[[").append(UserStringNop("PLAYER_ENTERED_GAME"))
                                                .append(",").append(player_connection->PlayerName()).append("]]");
            m_server.PushChatMessage(data, "", CLR_SERVER, timestamp);

            // send message to other players
            for (auto it = m_server.m_networking.established_begin();
                 it != m_server.m_networking.established_end(); ++it)
            {
                if (player_connection != (*it))
                    (*it)->SendMessage(ServerPlayerChatMessage(Networking::INVALID_PLAYER_ID, timestamp, data));
            }

            std::vector<std::reference_wrapper<const ChatHistoryEntity>> chat_history;
            for (const auto& elem : m_server.GetChatHistory())
                chat_history.push_back(std::cref(elem));
            if (chat_history.size() > 0)
                player_connection->SendMessage(ChatHistoryMessage(chat_history, !player_connection->IsLocalConnection()));
        }
    }

    // disconnect "ghost" connection after establishing new
    for (auto& conn : to_disconnect)
        m_server.Networking().Disconnect(std::move(conn));

    return client_type != Networking::ClientType::INVALID_CLIENT_TYPE;
}



////////////////////////////////////////////////////////////
// Idle
////////////////////////////////////////////////////////////
Idle::Idle(my_context c) :
    my_base(c)
{
    TraceLogger(FSM) << "(ServerFSM) Idle";
    if (Server().IsHostless())
        post_event(Hostless());
    else if (!GetOptionsDB().Get<std::string>("load").empty())
        throw std::invalid_argument("Autostart load file was chosen but the server wasn't started in a hostless mode");
    else if (GetOptionsDB().Get<bool>("quickstart"))
        throw std::invalid_argument("Quickstart was chosen but the server wasn't started in a hostless mode");
    else if (GetOptionsDB().Get<bool>("load-or-quickstart"))
        throw std::invalid_argument("Load or quickstart was chosen but the server wasn't started in a hostless mode");
}

Idle::~Idle()
{ TraceLogger(FSM) << "(ServerFSM) ~Idle"; }

sc::result Idle::react(const HostMPGame& msg) {
    TraceLogger(FSM) << "(ServerFSM) Idle.HostMPGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string host_player_name;
    std::string client_version_string;
    std::map<std::string, std::string> dependencies;
    try {
        ExtractHostMPGameMessageData(message, host_player_name, client_version_string, dependencies);
    } catch (...) {}

    // validate host name (was found and wasn't empty)
    if (host_player_name.empty()) {
        ErrorLogger(FSM) << "Idle::react(const HostMPGame& msg) got an empty host player name";
        return discard_event();
    }

    DebugLogger(FSM) << "Idle::react(HostMPGame) about to establish host";

    const int host_player_id = server.m_networking.NewPlayerID();
    player_connection->EstablishPlayer(host_player_id, std::move(host_player_name),
                                       Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER,
                                       std::move(client_version_string));
    server.m_networking.SetHostPlayerID(host_player_id);

    if (!GetOptionsDB().Get<bool>("skip-checksum"))
        player_connection->SendMessage(ContentCheckSumMessage(server.GetSpeciesManager()));

    DebugLogger(FSM) << "Idle::react(HostMPGame) about to send acknowledgement to host";
    player_connection->SetAuthRoles(Networking::AuthRoles{
                                        Networking::RoleType::ROLE_HOST,
                                        Networking::RoleType::ROLE_CLIENT_TYPE_MODERATOR,
                                        Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER,
                                        Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER,
                                        Networking::RoleType::ROLE_GALAXY_SETUP
                                    });
    player_connection->SendMessage(HostMPAckMessage(host_player_id));

    server.m_single_player_game = false;

    DebugLogger(FSM) << "Idle::react(HostMPGame) about to transit to MPLobby";

    return transit<MPLobby>();
}

sc::result Idle::react(const HostSPGame& msg) {
    TraceLogger(FSM) << "(ServerFSM) Idle.HostSPGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    auto single_player_setup_data = std::make_shared<SinglePlayerSetupData>();
    std::string client_version_string;
    std::map<std::string, std::string> dependencies;
    try {
        ExtractHostSPGameMessageData(message, *single_player_setup_data, client_version_string, dependencies);
    } catch (...) {}


    // get host player's name from setup data or saved file
    std::string host_player_name;
    try {
        host_player_name = GetHostNameFromSinglePlayerSetupData(*single_player_setup_data);
    } catch (const std::exception&) {
        player_connection->SendMessage(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), true));
        return discard_event();
    }
    // validate host name (was found and wasn't empty)
    if (host_player_name.empty()) {
        ErrorLogger(FSM) << "Idle::react(const HostSPGame& msg) got an empty host player name or couldn't find a human player";
        return discard_event();
    }


    const int host_player_id = server.m_networking.NewPlayerID();
    player_connection->EstablishPlayer(host_player_id, std::move(host_player_name),
                                       Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER,
                                       std::move(client_version_string));
    server.m_networking.SetHostPlayerID(host_player_id);
    if (!GetOptionsDB().Get<bool>("skip-checksum"))
        player_connection->SendMessage(ContentCheckSumMessage(server.GetSpeciesManager()));
    player_connection->SetAuthRoles(Networking::AuthRoles{
                                        Networking::RoleType::ROLE_HOST,
                                        Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER,
                                        Networking::RoleType::ROLE_GALAXY_SETUP
                                    });
    player_connection->SendMessage(HostSPAckMessage(host_player_id));

    server.m_single_player_game = true;

    context<ServerFSM>().m_single_player_setup_data = std::move(single_player_setup_data);

    return transit<WaitingForSPGameJoiners>();
}

sc::result Idle::react(const ShutdownServer& msg) {
    TraceLogger(FSM) << "(ServerFSM) Idle.ShutdownServer";

    return transit<ShuttingDownServer>();
}

sc::result Idle::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal && !Server().IsHostless())
        return transit<ShuttingDownServer>();
    return discard_event();
}

sc::result Idle::react(const Hostless&) {
    TraceLogger(FSM) << "(ServerFSM) Idle.Hostless";
    std::string autostart_load_filename = GetOptionsDB().Get<std::string>("load");
    bool quickstart = GetOptionsDB().Get<bool>("quickstart");
    bool load_or_quickstart = GetOptionsDB().Get<bool>("load-or-quickstart");
    if (!quickstart && !load_or_quickstart && autostart_load_filename.empty())
        return transit<MPLobby>();

    if (GetOptionsDB().Get<int>("network.server.conn-human-empire-players.min") > 0) {
        throw std::invalid_argument("A save file to load or quickstart and autostart in hostless mode was specified, but the server has a non-zero minimum number of connected players, so cannot be started without a connected player.");
    }

    ServerApp& server = Server();
    auto lobby_data = std::make_shared<MultiplayerLobbyData>(server.m_galaxy_setup_data);
    auto server_save_game_data = std::make_shared<ServerSaveGameData>();
    std::vector<PlayerSaveGameData> player_save_game_data;
    server.InitializePython();
    server.LoadChatHistory();

    if (load_or_quickstart) {
        // Search save games in a subfolder with game UID or "auto" formed in
        // `GetAutoSaveFileName`.
        std::string subdir = server.m_galaxy_setup_data.GetGameUID();
        boost::filesystem::path autosave_dir_path = GetServerSaveDir() / (subdir.empty() ? "auto" : subdir);
        if (IsExistingDir(autosave_dir_path)) {
            auto saves = ListDir(autosave_dir_path, IsMultiplayerSaveFile);
            for (const auto& save : saves) {
                // Filenames of saves lexicographically sorted with turns and timestamps so latest
                // file will have greater string representation.
                if (PathToString(save) > autostart_load_filename)
                    autostart_load_filename = PathToString(save);
            }
        }
    }

    if (autostart_load_filename.empty()) {
        DebugLogger(FSM) << "Start new game";

        auto human_players = server.FillListPlayers();
        for (auto& player_setup_data : human_players) {
            DebugLogger(FSM) << "Create player " << player_setup_data.player_name;
            player_setup_data.player_id =     Networking::INVALID_PLAYER_ID;
            player_setup_data.client_type =   Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER;
            player_setup_data.empire_color =  GetUnusedEmpireColour(lobby_data->players);
            player_setup_data.authenticated = true;
            lobby_data->players.emplace_back(Networking::INVALID_PLAYER_ID, std::move(player_setup_data));
        }

        const SpeciesManager& sm = server.GetSpeciesManager();
        auto max_ai = GetOptionsDB().Get<int>("network.server.ai.max");
        const int ai_count = GetOptionsDB().Get<int>("setup.ai.player.count");
        int ai_next_index = 1;
        while (ai_next_index <= ai_count && (ai_next_index <= max_ai || max_ai < 0)) {
            PlayerSetupData player_setup_data;
            player_setup_data.player_id =     Networking::INVALID_PLAYER_ID;
            player_setup_data.player_name =   UserString("AI_PLAYER") + "_" + std::to_string(ai_next_index++);
            player_setup_data.client_type =   Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
            player_setup_data.empire_name =   GenerateEmpireName(player_setup_data.player_name, lobby_data->players);
            player_setup_data.empire_color =  GetUnusedEmpireColour(lobby_data->players);
            if (lobby_data->seed != "")
                player_setup_data.starting_species_name = sm.RandomPlayableSpeciesName();
            else
                player_setup_data.starting_species_name = sm.SequentialPlayableSpeciesName(ai_next_index);

            lobby_data->players.emplace_back(Networking::INVALID_PLAYER_ID, std::move(player_setup_data));
        }
    } else {
        DebugLogger(FSM) << "Loading file " << autostart_load_filename;
        try {
            LoadGame(autostart_load_filename,   *server_save_game_data,
                     player_save_game_data,     server.GetUniverse(),
                     server.Empires(),          server.GetSpeciesManager(),
                     GetCombatLogManager(),     server.m_galaxy_setup_data);
            int seed = 0;
            try {
                seed = boost::lexical_cast<unsigned int>(server.m_galaxy_setup_data.seed);
            } catch (...) {
                try {
                    boost::hash<std::string> string_hash;
                    std::size_t h = string_hash(server.m_galaxy_setup_data.seed);
                    seed = static_cast<unsigned int>(h);
                } catch (...) {}
            }
            DebugLogger(FSM) << "Seeding with loaded galaxy seed: " << server.m_galaxy_setup_data.seed
                             << "  interpreted as: " << seed;
            Seed(seed);

            // fill lobby data with AI to start them with server
            int ai_next_index = 1;
            for (const auto& psgd : player_save_game_data) {
                if (psgd.client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
                    continue;
                PlayerSetupData player_setup_data;
                player_setup_data.player_id =     Networking::INVALID_PLAYER_ID;
                player_setup_data.player_name =   UserString("AI_PLAYER") + "_" + std::to_string(ai_next_index++);
                player_setup_data.client_type =   Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
                player_setup_data.save_game_empire_id = psgd.empire_id;
                lobby_data->players.emplace_back(Networking::INVALID_PLAYER_ID, std::move(player_setup_data));
            }
        } catch (const std::exception& e) {
            throw e;
        }
    }

    lobby_data->game_rules = GetGameRules().GetRulesAsStrings();

    // copy locally stored data to common server fsm context so it can be
    // retreived in WaitingForMPGameJoiners
    context<ServerFSM>().m_lobby_data = lobby_data;
    context<ServerFSM>().m_player_save_game_data = player_save_game_data;
    context<ServerFSM>().m_server_save_game_data = server_save_game_data;

    return transit<WaitingForMPGameJoiners>();
}

////////////////////////////////////////////////////////////
// MPLobby
////////////////////////////////////////////////////////////
namespace {
    // return true if player has important changes.
    bool IsPlayerChanged(const PlayerSetupData& lhs, const PlayerSetupData& rhs) {
        return (lhs.client_type != rhs.client_type) ||
            (lhs.starting_species_name != rhs.starting_species_name) ||
            (lhs.save_game_empire_id != rhs.save_game_empire_id) ||
            (lhs.empire_color != rhs.empire_color);
    }
}

MPLobby::MPLobby(my_context c) :
    my_base(c),
    m_lobby_data(std::make_shared<MultiplayerLobbyData>(std::move(Server().m_galaxy_setup_data))),
    m_server_save_game_data(std::make_shared<ServerSaveGameData>()),
    m_ai_next_index(1)
{
    TraceLogger(FSM) << "(ServerFSM) MPLobby";
    ClockSeed();
    ServerApp& server = Server();
    server.LoadChatHistory();
    m_lobby_data->game_rules = GetGameRules().GetRulesAsStrings();
    const SpeciesManager& sm = server.GetSpeciesManager();
    if (server.IsHostless()) {
        DebugLogger(FSM) << "(ServerFSM) MPLobby. Fill MPLobby data from the previous game.";

        m_lobby_data->any_can_edit = true;

        auto max_ai = GetOptionsDB().Get<int>("network.server.ai.max");
        std::vector<PlayerConnectionPtr> to_disconnect;
        to_disconnect.reserve(server.m_networking.size());
        // Try to use connections:
        for (const auto& player_connection : server.m_networking) {
            // If connection was not established disconnect it.
            if (!player_connection->IsEstablished()) {
                to_disconnect.push_back(player_connection);
                continue;
            }

            int player_id = player_connection->PlayerID();
            DebugLogger(FSM) << "(ServerFSM) MPLobby. Fill MPLobby player " << player_id;
            if (player_connection->GetClientType() != Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
                PlayerSetupData player_setup_data;
                player_setup_data.player_id =     player_id;
                player_setup_data.player_name =   player_connection->PlayerName();
                player_setup_data.client_type =   player_connection->GetClientType();
                player_setup_data.empire_name =   (player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) ? player_connection->PlayerName() : GenerateEmpireName(player_setup_data.player_name, m_lobby_data->players);
                player_setup_data.empire_color =  GetUnusedEmpireColour(m_lobby_data->players);
                if (m_lobby_data->seed != "")
                    player_setup_data.starting_species_name = sm.RandomPlayableSpeciesName();
                else
                    player_setup_data.starting_species_name = sm.SequentialPlayableSpeciesName(player_id);
                player_setup_data.authenticated = player_connection->IsAuthenticated();

                m_lobby_data->players.emplace_back(player_id, std::move(player_setup_data));

            } else if (player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
                if (m_ai_next_index <= max_ai || max_ai < 0) {
                    PlayerSetupData player_setup_data;
                    player_setup_data.player_id =     Networking::INVALID_PLAYER_ID;
                    player_setup_data.player_name =   UserString("AI_PLAYER") + "_" + std::to_string(m_ai_next_index++);
                    player_setup_data.client_type =   Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
                    player_setup_data.empire_name =   GenerateEmpireName(player_setup_data.player_name, m_lobby_data->players);
                    player_setup_data.empire_color =  GetUnusedEmpireColour(m_lobby_data->players);
                    if (m_lobby_data->seed != "")
                        player_setup_data.starting_species_name = sm.RandomPlayableSpeciesName();
                    else
                        player_setup_data.starting_species_name = sm.SequentialPlayableSpeciesName(m_ai_next_index);

                    m_lobby_data->players.emplace_back(Networking::INVALID_PLAYER_ID,
                                                       std::move(player_setup_data));
                }
                // disconnect AI
                to_disconnect.push_back(player_connection);
            }
        }

        for (const auto& player_connection : to_disconnect)
            server.Networking().Disconnect(player_connection);


        // check if there weren't previous AIs
        if (m_ai_next_index == 1) {
            // use AI count from option
            const int ai_count = GetOptionsDB().Get<int>("setup.ai.player.count");
            while (m_ai_next_index <= ai_count && (m_ai_next_index <= max_ai || max_ai < 0)) {
                PlayerSetupData player_setup_data;
                player_setup_data.player_id =     Networking::INVALID_PLAYER_ID;
                player_setup_data.player_name =   UserString("AI_PLAYER") + "_" + std::to_string(m_ai_next_index++);
                player_setup_data.client_type =   Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
                player_setup_data.empire_name =   GenerateEmpireName(player_setup_data.player_name, m_lobby_data->players);
                player_setup_data.empire_color =  GetUnusedEmpireColour(m_lobby_data->players);
                if (!m_lobby_data->seed.empty())
                    player_setup_data.starting_species_name = sm.RandomPlayableSpeciesName();
                else
                    player_setup_data.starting_species_name = sm.SequentialPlayableSpeciesName(m_ai_next_index);

                m_lobby_data->players.emplace_back(Networking::INVALID_PLAYER_ID,
                                                   std::move(player_setup_data));
            }
        }

        ValidateClientLimits();

        server.Networking().SendMessageAll(ServerLobbyUpdateMessage(*m_lobby_data));
    } else {
        int host_id = server.m_networking.HostPlayerID();
        const PlayerConnectionPtr& player_connection = *(server.m_networking.GetPlayer(host_id));

        // create player setup data for host, and store in list
        m_lobby_data->players.push_back({host_id, PlayerSetupData()});

        PlayerSetupData& player_setup_data = m_lobby_data->players.begin()->second;

        player_setup_data.player_name =           player_connection->PlayerName();
        player_setup_data.empire_name =           (player_connection->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) ? player_connection->PlayerName() : GenerateEmpireName(player_setup_data.player_name, m_lobby_data->players);
        player_setup_data.empire_color =          EmpireColors().at(0);               // since the host is the first joined player, it can be assumed that no other player is using this colour (unlike subsequent join game message responses)
        player_setup_data.starting_species_name = sm.RandomPlayableSpeciesName();
        // leaving save game empire id as default
        player_setup_data.client_type =           player_connection->GetClientType();
        player_setup_data.authenticated =         player_connection->IsAuthenticated();

        player_connection->SendMessage(ServerLobbyUpdateMessage(*m_lobby_data));
    }
}

MPLobby::~MPLobby()
{ TraceLogger(FSM) << "(ServerFSM) ~MPLobby"; }

void MPLobby::ValidateClientLimits() {
    int human_count = 0;
    int ai_count = 0;
    for (const auto& plr : m_lobby_data->players | range_values) {
        if (plr.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
            human_count++;
        else if (plr.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            ai_count++;
    }

    const int human_connected_count = human_count;

    // for load game consider as human all non-AI empires
    // because human player could connect later in game
    int non_eliminated_empires_count = 0;
    if (!m_lobby_data->new_game) {
        for (const auto& empire_data : m_lobby_data->save_game_empire_data) {
            if (!empire_data.second.eliminated)
                non_eliminated_empires_count ++;
        }

        human_count = non_eliminated_empires_count - ai_count;
    }

    int min_ai = GetOptionsDB().Get<int>("network.server.ai.min");
    int max_ai = GetOptionsDB().Get<int>("network.server.ai.max");
    if (max_ai >= 0 && max_ai < min_ai) {
        WarnLogger(FSM) << "Maximum ai clients less than minimum, setting max to min";
        max_ai = min_ai;
        GetOptionsDB().Set("network.server.ai.max", max_ai);
    }
    int min_human = GetOptionsDB().Get<int>("network.server.human.min");
    int max_human = GetOptionsDB().Get<int>("network.server.human.max");
    if (max_human >= 0 && max_human < min_human) {
        WarnLogger(FSM) << "Maximum human clients less than minimum, setting max to min";
        max_human = min_human;
        GetOptionsDB().Set("network.server.human.max", max_human);
    }
    int min_connected_human_empire_players = GetOptionsDB().Get<int>("network.server.conn-human-empire-players.min");
    int max_unconnected_human_empire_players = GetOptionsDB().Get<int>("network.server.unconn-human-empire-players.max");

    // restrict minimun number of human and ai players
    if (human_count < min_human) {
        m_lobby_data->start_locked = true;
        m_lobby_data->start_lock_cause = UserStringNop("ERROR_NOT_ENOUGH_HUMAN_PLAYERS");
    } else if (human_connected_count < min_connected_human_empire_players) {
        m_lobby_data->start_locked = true;
        m_lobby_data->start_lock_cause = UserStringNop("ERROR_NOT_ENOUGH_CONNECTED_HUMAN_PLAYERS");
    } else if (max_unconnected_human_empire_players > 0 &&
        !m_lobby_data->new_game &&
        non_eliminated_empires_count - ai_count - human_connected_count >= max_unconnected_human_empire_players)
    {
        m_lobby_data->start_locked = true;
        m_lobby_data->start_lock_cause = UserStringNop("ERROR_TOO_MANY_UNCONNECTED_HUMAN_PLAYERS");
    } else if (max_human >= 0 && human_count > max_human) {
        m_lobby_data->start_locked = true;
        m_lobby_data->start_lock_cause = UserStringNop("ERROR_TOO_MANY_HUMAN_PLAYERS");
    } else if (ai_count < min_ai) {
        m_lobby_data->start_locked = true;
        m_lobby_data->start_lock_cause = UserStringNop("ERROR_NOT_ENOUGH_AI_PLAYERS");
    } else if (max_ai >= 0 && ai_count > max_ai) {
        m_lobby_data->start_locked = true;
        m_lobby_data->start_lock_cause = UserStringNop("ERROR_TOO_MANY_AI_PLAYERS");
    } else {
        m_lobby_data->start_locked = false;
        m_lobby_data->start_lock_cause.clear();
    }
}

sc::result MPLobby::react(const Disconnection& d) {
    TraceLogger(FSM) << "(ServerFSM) MPLobby.Disconnection";
    ServerApp& server = Server();
    PlayerConnectionPtr& player_connection = d.m_player_connection;

    DebugLogger(FSM) << "MPLobby::react(Disconnection) player id: " << player_connection->PlayerID();
    DebugLogger(FSM) << "Remaining player ids: ";
    for (auto it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        DebugLogger(FSM) << " ... " << (*it)->PlayerID() << " (" << (*it)->PlayerName() << ")";
    }

    if (!server.IsHostless()) {
        // if there are no humans left, it's time to terminate
        if (server.m_networking.empty() || server.m_ai_client_processes.size() == server.m_networking.NumEstablishedPlayers()) {
            DebugLogger(FSM) << "MPLobby.Disconnection : All human players disconnected; server terminating.";
            return transit<ShuttingDownServer>();
        }
    }

    if (server.m_networking.PlayerIsHost(player_connection->PlayerID()))
        server.SelectNewHost();

    // if the disconnected player wasn't in the lobby, don't need to do anything more.
    // if player is in lobby, need to remove it
    int id = player_connection->PlayerID();
    // Non-established player shouldn't be processed because it wasn't in lobby and AI entries have same id (INVALID_PLAYER_ID).
    if (id == Networking::INVALID_PLAYER_ID) {
        DebugLogger(FSM) << "MPLobby.Disconnection : Disconnecting player (" << id << ") was not established";
        return discard_event();
    }
    bool player_was_in_lobby = false;
    for (auto it = m_lobby_data->players.begin();
         it != m_lobby_data->players.end(); ++it)
    {
        if (it->first == id) {
            player_was_in_lobby = true;
            m_lobby_data->players.erase(it);
            break;
        }
    }
    if (player_was_in_lobby) {
        // update cookie's expire date
        // so player could reconnect within 15 minutes
        Server().Networking().UpdateCookie(player_connection->Cookie());

        // drop ready flag as player list changed
        for (auto& plrs : m_lobby_data->players) {
            plrs.second.player_ready = false;
        }

        // add "player left game" message
        boost::posix_time::ptime timestamp = boost::posix_time::second_clock::universal_time();
        std::string data = std::string("[[") + UserStringNop("PLAYER_LEFT_GAME") + "," + player_connection->PlayerName() + "]]";
        server.PushChatMessage(data, "", CLR_SERVER, timestamp);

        // send message to other players
        for (auto it = server.m_networking.established_begin();
             it != server.m_networking.established_end(); ++it)
        {
            if (player_connection != (*it))
                (*it)->SendMessage(ServerPlayerChatMessage(Networking::INVALID_PLAYER_ID, timestamp, data));
        }
    } else {
        DebugLogger(FSM) << "MPLobby.Disconnection : Disconnecting player (" << id << ") was not in lobby";
        return discard_event();
    }

    ValidateClientLimits();

    // send updated lobby data to players after disconnection-related changes
    for (auto it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        (*it)->SendMessage(ServerLobbyUpdateMessage(*m_lobby_data));
    }

    return discard_event();
}

void MPLobby::EstablishPlayer(PlayerConnectionPtr player_connection,
                              std::string player_name, Networking::ClientType client_type,
                              std::string client_version_string, Networking::AuthRoles roles)
{
    ServerApp& server = Server();
    const SpeciesManager& sm = server.GetSpeciesManager();

    if (context<ServerFSM>().EstablishPlayer(player_connection, player_name, client_type,
                                             client_version_string, roles))
    {
        const int player_id = player_connection->PlayerID();

        // Inform AI of logging configuration.
        if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            player_connection->SendMessage(
                LoggerConfigMessage(Networking::INVALID_PLAYER_ID, LoggerOptionsLabelsAndLevels(LoggerTypes::both)));

        // assign player info from defaults or from connection to lobby data players list
        PlayerSetupData player_setup_data;
        player_setup_data.player_name = player_name;
        player_setup_data.client_type = client_type;
        player_setup_data.empire_name = (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) ?
                                         player_name : GenerateEmpireName(player_name, m_lobby_data->players);
        player_setup_data.empire_color = GetUnusedEmpireColour(m_lobby_data->players);
        if (m_lobby_data->seed.empty())
            player_setup_data.starting_species_name = sm.RandomPlayableSpeciesName();
        else
            player_setup_data.starting_species_name = sm.SequentialPlayableSpeciesName(player_id);
        player_setup_data.authenticated =  player_connection->IsAuthenticated();

        // after setting all details, push into lobby data
        m_lobby_data->players.push_back({player_id, player_setup_data});

        // drop ready player flag at new player
        for (auto& plr : m_lobby_data->players) {
            if (plr.second.empire_name == player_name) {
                // change empire name
                plr.second.empire_name = (plr.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) ?
                                          plr.second.player_name : GenerateEmpireName(plr.second.player_name, m_lobby_data->players);
            }

            plr.second.player_ready = false;
        }

        ValidateClientLimits();

        for (auto it = server.m_networking.established_begin();
             it != server.m_networking.established_end(); ++it)
        { (*it)->SendMessage(ServerLobbyUpdateMessage(*m_lobby_data)); }
    }
}

namespace {
    std::string StringifyDependencies(const std::map<std::string, std::string>& deps) {
        std::string retval;
        for (auto& [dep, version] : deps)
            retval.append(dep).append(": ").append(version).append("   ");
        return retval;
    }
}

sc::result MPLobby::react(const JoinGame& msg) {
    TraceLogger(FSM) << "(ServerFSM) MPLobby.JoinGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name;
    Networking::ClientType client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    std::string client_version_string;
    std::map<std::string, std::string> dependencies;
    boost::uuids::uuid cookie = boost::uuids::nil_generator{}();
    try {
        ExtractJoinGameMessageData(message, player_name, client_type, client_version_string,
                                   dependencies, cookie);
    } catch (const std::exception&) {
        ErrorLogger(FSM) << "MPLobby::react(const JoinGame& msg): couldn't extract data from join game message";
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION"), true));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }

    InfoLogger() << "Player " << player_name << " has dependencies: " << StringifyDependencies(dependencies);

    Networking::AuthRoles roles;
    bool authenticated = false;

    DebugLogger() << "MPLobby.JoinGame Try to login player " << player_name << " with cookie: " << cookie;
    if (server.Networking().CheckCookie(cookie, player_name, roles, authenticated)) {
        // if player have correct and non-expired cookies simply establish him
        player_connection->SetCookie(cookie);
        if (authenticated)
            player_connection->SetAuthenticated();
    } else {
        const bool relaxed_auth = player_connection->IsLocalConnection() && client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
        if (!relaxed_auth && server.IsAuthRequiredOrFillRoles(player_name, player_connection->GetIpAddress(), roles)) {
            // send authentication request
            player_connection->AwaitPlayer(client_type, client_version_string);
            player_connection->SendMessage(AuthRequestMessage(player_name, "PLAIN-TEXT"));
            return discard_event();
        }

        std::string original_player_name = player_name;

        // Remove AI prefix to distinguish Human from AI.
        std::string ai_prefix = UserString("AI_PLAYER") + "_";
        if (client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            while (player_name.compare(0, ai_prefix.size(), ai_prefix) == 0)
                player_name.erase(0, ai_prefix.size());
        }
        if (player_name.empty())
            player_name = "_";

        std::string new_player_name = player_name;

        // Try numeric suffixes to get unique name which isn't equal to other player or empire name
        // or registered for authentication.
        bool collision = true;
        std::size_t t = 1;
        while (collision &&
               t <= m_lobby_data->players.size() + server.Networking().GetCookiesSize() + 1)
        {
            collision = false;
            roles.Clear();
            if (!server.IsAvailableName(new_player_name) || (!relaxed_auth && server.IsAuthRequiredOrFillRoles(new_player_name, player_connection->GetIpAddress(), roles))) {
                collision = true;
            } else {
                for (auto& plr : m_lobby_data->players) {
                    if (plr.second.empire_name == new_player_name) {
                        collision = true;
                        break;
                    }
                }
            }

            if (collision)
                new_player_name = player_name + std::to_string(++t); // start alternative names from 2
        }

        // There is a small chance of unsolveable collision in case of sequential player names are
        // registered for authentication
        if (collision) {
            ErrorLogger(FSM) << "MPLobby::react(const JoinGame& msg): couldn't find free player name for \"" << original_player_name << "\"";
            player_connection->SendMessage(ErrorMessage(str(FlexibleFormat(UserString("ERROR_PLAYER_NAME_ALREADY_USED")) % original_player_name),
                                                        true));
            server.Networking().Disconnect(player_connection);
            return discard_event();
        }

        player_name = std::move(new_player_name);
    }

    EstablishPlayer(player_connection, std::move(player_name), client_type,
                    std::move(client_version_string), roles);

    return discard_event();
}

sc::result MPLobby::react(const AuthResponse& msg) {
    TraceLogger(FSM) << "(ServerFSM) MPLobby.AuthResponse";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name;
    std::string auth;
    try {
        ExtractAuthResponseMessageData(message, player_name, auth);
    } catch (...) {}

    Networking::AuthRoles roles;

    if (!server.IsAuthSuccessAndFillRoles(player_name, auth, roles)) {
        // wrong password
        player_connection->SendMessage(ErrorMessage(
            str(FlexibleFormat(UserString("ERROR_WRONG_PASSWORD")) % player_name), true));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }
    player_connection->SetAuthenticated();

    Networking::ClientType client_type = player_connection->GetClientType();

    EstablishPlayer(player_connection, std::move(player_name), client_type,
                    player_connection->ClientVersionString(), roles);

    return discard_event();
}

sc::result MPLobby::react(const LobbyUpdate& msg) {
    TraceLogger(FSM) << "(ServerFSM) MPLobby.LobbyUpdate";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    MultiplayerLobbyData incoming_lobby_data;
    try {
        ExtractLobbyUpdateMessageData(message, incoming_lobby_data);
    } catch (...) {}

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

    if (sender->HasAuthRole(Networking::RoleType::ROLE_HOST)) {
        if (m_lobby_data->any_can_edit != incoming_lobby_data.any_can_edit) {
            has_important_changes = true;
            m_lobby_data->any_can_edit = incoming_lobby_data.any_can_edit;

            // change role ROLE_GALAXY_SETUP for all non-host players
            for (const auto& player_connection : server.Networking()) {
                if (!player_connection->HasAuthRole(Networking::RoleType::ROLE_HOST)) {
                    player_connection->SetAuthRole(Networking::RoleType::ROLE_GALAXY_SETUP,
                                                   m_lobby_data->any_can_edit);
                }
            }
        }
    }

    if (sender->HasAuthRole(Networking::RoleType::ROLE_GALAXY_SETUP)) {

        DebugLogger(FSM) << "Get message from host or allowed player " << sender->PlayerID();

        // assign unique names / colours to any lobby entry that lacks them, or
        // remove empire / colours from observers
        for (auto& entry : incoming_lobby_data.players) {
            PlayerSetupData& psd = entry.second;
            if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
                psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
            {
                psd.empire_color = CLR_ZERO;
                // On OSX the following two lines must not be included.
                // Clearing empire name and starting species name from
                // PlayerSetupData causes a weird crash (bus error) deep
                // in GG code on OSX in the Multiplayer Lobby when selecting
                // Observer or Moderator as client type.
#ifndef FREEORION_MACOSX
                psd.empire_name.clear();
                psd.starting_species_name.clear();
#endif
                psd.save_game_empire_id = ALL_EMPIRES;

            } else if (psd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
                if (psd.empire_color == CLR_ZERO)
                    psd.empire_color = GetUnusedEmpireColour(incoming_lobby_data.players);
                if (psd.player_name.empty())
                    // ToDo: Should we translate player_name?
                    psd.player_name = UserString("AI_PLAYER") + "_" + std::to_string(m_ai_next_index++);
                if (psd.empire_name.empty())
                    psd.empire_name = GenerateEmpireName(psd.player_name, incoming_lobby_data.players);
                if (psd.starting_species_name.empty()) {
                    if (m_lobby_data->seed != "")
                        psd.starting_species_name = server.m_species_manager.RandomPlayableSpeciesName();
                    else
                        psd.starting_species_name = server.m_species_manager.SequentialPlayableSpeciesName(m_ai_next_index);
                }

            } else if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
                if (psd.empire_color == CLR_ZERO)
                    psd.empire_color = GetUnusedEmpireColour(incoming_lobby_data.players);
                if (psd.empire_name.empty())
                    psd.empire_name = psd.player_name;
                if (psd.starting_species_name.empty())
                    psd.starting_species_name = server.m_species_manager.RandomPlayableSpeciesName();
            }
        }

        bool has_collision = false;
        // check for color, names, and IDs
        std::set<EmpireColor> psd_colors;
        std::set<std::string> psd_names;
        std::set<int> psd_ids;
        for (auto& player : incoming_lobby_data.players) {
            if (psd_colors.contains(player.second.empire_color) ||
                psd_names.contains(player.second.empire_name) ||
                psd_names.contains(player.second.player_name))
            {
                has_collision = true;
                WarnLogger(FSM) << "Got color, empire's name or player's name collision for player "
                                << player.second.player_name << "(" << player.first << ")";
                break;
            } else {
                psd_colors.emplace(player.second.empire_color);
                psd_names.emplace(player.second.empire_name);
                psd_names.emplace(player.second.player_name);
            }

            if (player.first != Networking::INVALID_PLAYER_ID) {
                const auto& player_it = server.Networking().GetPlayer(player.first);
                if (player_it != server.Networking().established_end()) {
                    // check for roles and client types
                    if ((player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER &&
                        !(*player_it)->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER)) ||
                        (player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR &&
                        !(*player_it)->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_MODERATOR)) ||
                        (player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER &&
                        !(*player_it)->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER)))
                    {
                        has_collision = true;
                        WarnLogger(FSM) << "Got unallowed client types.";
                        break;
                    }
                    // set correct authentication status
                    player.second.authenticated = (*player_it)->IsAuthenticated();
                } else {
                    // player wasn't found
                    // don't allow "ghost" records
                    has_collision = true;
                    WarnLogger(FSM) << "Got missing player.";
                    break;
                }
                if (!psd_ids.emplace(player.first).second) {
                    // player id was already used
                    // don't allow ID collision
                    has_collision = true;
                    WarnLogger(FSM) << "Got player's id collision.";
                    break;
                }
            }
        }

        if (has_collision) {
            player_setup_data_changed = true;
            for (auto& player : m_lobby_data->players) {
                if (player.first == sender->PlayerID()) {
                    player.second.player_ready = false;
                    break;
                }
            }
        } else {

            player_setup_data_changed = (incoming_lobby_data.players != m_lobby_data->players);

            // check if galaxy setup data changed
            has_important_changes = has_important_changes || (m_lobby_data->seed != incoming_lobby_data.seed) ||
                (m_lobby_data->size != incoming_lobby_data.size) ||
                (m_lobby_data->shape != incoming_lobby_data.shape) ||
                (m_lobby_data->age != incoming_lobby_data.age) ||
                (m_lobby_data->starlane_freq != incoming_lobby_data.starlane_freq) ||
                (m_lobby_data->planet_density != incoming_lobby_data.planet_density) ||
                (m_lobby_data->specials_freq != incoming_lobby_data.specials_freq) ||
                (m_lobby_data->monster_freq != incoming_lobby_data.monster_freq) ||
                (m_lobby_data->native_freq != incoming_lobby_data.native_freq) ||
                (m_lobby_data->ai_aggr != incoming_lobby_data.ai_aggr) ||
                (m_lobby_data->new_game != incoming_lobby_data.new_game) ||
                (m_lobby_data->game_rules != incoming_lobby_data.game_rules);

            if (player_setup_data_changed) {
                if (m_lobby_data->players.size() != incoming_lobby_data.players.size()) {
                    has_important_changes = true; // drop ready at number of players changed
                } else {
                    for (auto& i_player : m_lobby_data->players) {
                        if (i_player.first < 0) // ignore changes in AI.
                            continue;
                        int player_id = i_player.first;
                        bool is_found_player = false;
                        for (auto& j_player : incoming_lobby_data.players) {
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
            m_lobby_data->SetSeed(incoming_lobby_data.seed);
            m_lobby_data->size           = incoming_lobby_data.size;
            m_lobby_data->shape          = incoming_lobby_data.shape;
            m_lobby_data->age            = incoming_lobby_data.age;
            m_lobby_data->starlane_freq  = incoming_lobby_data.starlane_freq;
            m_lobby_data->planet_density = incoming_lobby_data.planet_density;
            m_lobby_data->specials_freq  = incoming_lobby_data.specials_freq;
            m_lobby_data->monster_freq   = incoming_lobby_data.monster_freq;
            m_lobby_data->native_freq    = incoming_lobby_data.native_freq;
            m_lobby_data->ai_aggr        = incoming_lobby_data.ai_aggr;

            // copy rules from incoming lobby data to server lobby data, only if those rules are
            // not locked by the server
            for (const auto& incoming_rule : incoming_lobby_data.game_rules) {
                if (GetOptionsDB().OptionExists("setup.rules.server-locked." + incoming_rule.first) &&
                    !GetOptionsDB().Get<bool>("setup.rules.server-locked." + incoming_rule.first))
                {
                    m_lobby_data->game_rules[incoming_rule.first] = incoming_rule.second;
                }
            }

            // directly configurable lobby data
            m_lobby_data->new_game       = incoming_lobby_data.new_game;
            if (m_lobby_data->new_game) {
                // empty save data
                m_lobby_data->save_game = "";
                m_lobby_data->save_game_empire_data.clear();
                // prevent updating lobby by having old and new file name equal
                incoming_lobby_data.save_game.clear();
            }

            int ai_count = 0;
            for (const auto& plr : incoming_lobby_data.players) {
                if (plr.second.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
                    ai_count++;
                }
            }

            // limit count of AI
            auto max_ai = GetOptionsDB().Get<int>("network.server.ai.max");
            if (ai_count <= max_ai || max_ai < 0) {
                if (sender->HasAuthRole(Networking::RoleType::ROLE_HOST)) {
                    // don't check host player
                    // treat host player as superuser or administrator
                    m_lobby_data->players = incoming_lobby_data.players;
                } else {
                    // check players shouldn't set protected empire to themselves
                    // if they don't have required player's name
                    bool incorrect_empire = false;
                    for (const auto& plr : incoming_lobby_data.players) {
                        if (plr.first != sender->PlayerID())
                            continue;
                        if (plr.second.save_game_empire_id != ALL_EMPIRES) {
                            const auto empire_it = m_lobby_data->save_game_empire_data.find(plr.second.save_game_empire_id);
                            if (empire_it != m_lobby_data->save_game_empire_data.end()) {
                                if (empire_it->second.eliminated) {
                                    WarnLogger(FSM) << "Trying to take over eliminated empire \"" << empire_it->second.empire_name << "\"";
                                    incorrect_empire = true;
                                } else if (empire_it->second.authenticated) {
                                    if (empire_it->second.player_name != sender->PlayerName()) {
                                        WarnLogger(FSM) << "Unauthorized access to protected empire \"" << empire_it->second.empire_name << "\"."
                                                        << " Expected player \"" << empire_it->second.player_name << "\""
                                                        << " got \"" << sender->PlayerName() << "\"";
                                        incorrect_empire = true;
                                    }
                                }
                            } else {
                                WarnLogger(FSM) << "Unknown empire #" << plr.second.save_game_empire_id;
                                incorrect_empire = true;
                            }
                        }
                        break;
                    }
                    if (incorrect_empire) {
                        has_important_changes = true;
                        player_setup_data_changed = true;
                    } else {
                        // ToDo: non-host player should change only AI and himself
                        m_lobby_data->players = incoming_lobby_data.players;
                    }
                }
            } else {
                has_important_changes = true;
            }

            LogPlayerSetupData(m_lobby_data->players);

            // update player connection types according to modified lobby selections,
            // while recording connections that are to be dropped
            std::vector<PlayerConnectionPtr> player_connections_to_drop;
            for (auto player_connection_it = server.m_networking.established_begin();
                 player_connection_it != server.m_networking.established_end();
                 ++player_connection_it)
            {
                PlayerConnectionPtr player_connection = *player_connection_it;
                if (!player_connection->IsEstablished())
                    continue;
                int player_id = player_connection->PlayerID();

                // get lobby data for this player connection
                bool found_player_lobby_data = false;
                auto player_setup_it = m_lobby_data->players.begin();
                while (player_setup_it != m_lobby_data->players.end()) {
                    if (player_setup_it->first == player_id) {
                        found_player_lobby_data = true;
                        break;
                    }
                    ++player_setup_it;
                }

                // drop connections which have no lobby data
                if (!found_player_lobby_data) {
                    ErrorLogger(FSM) << "No player setup data for player " << player_id << " in MPLobby::react(const LobbyUpdate& msg)";
                    player_connections_to_drop.push_back(player_connection);
                    continue;
                }

                // get client type, and drop connections with invalid type, as that indicates a requested drop
                auto client_type = player_setup_it->second.client_type;

                if (client_type != Networking::ClientType::INVALID_CLIENT_TYPE) {
                    // update player connection type for lobby change
                    player_connection->SetClientType(client_type);
                } else {
                    // drop connections for players who were dropped from lobby
                    m_lobby_data->players.erase(player_setup_it);
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
            auto player_setup_it = m_lobby_data->players.begin();
            while (player_setup_it != m_lobby_data->players.end()) {
                if (player_setup_it->second.client_type == Networking::ClientType::INVALID_CLIENT_TYPE) {
                    auto erase_it = player_setup_it;
                    ++player_setup_it;
                    m_lobby_data->players.erase(erase_it);
                } else {
                    ++player_setup_it;
                }
            }

        }
    } else {
        // can change only himself
        for (auto& i_player : m_lobby_data->players) {
            if (i_player.first != sender->PlayerID())
                continue;

            // found sender at m_lobby_data
            for (auto& j_player : incoming_lobby_data.players) {
                if (j_player.first != sender->PlayerID())
                    continue;

                // found sender at incoming_lobby_data

                // check for color and names
                std::set<EmpireColor> psd_colors;
                std::set<std::string> psd_names;
                for (auto& k_player : m_lobby_data->players) {
                    if (k_player.first == sender->PlayerID())
                        continue;

                    psd_colors.emplace(k_player.second.empire_color);
                    psd_names.emplace(k_player.second.empire_name);
                    psd_names.emplace(k_player.second.player_name);
                }

                // if we have collision or unallowed client type
                // unset ready flag and ignore changes
                if (psd_colors.contains(j_player.second.empire_color) ||
                    psd_names.contains(j_player.second.empire_name) ||
                    psd_names.contains(j_player.second.player_name) ||
                    (j_player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER &&
                        !sender->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER)) ||
                    (j_player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR &&
                        !sender->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_MODERATOR)) ||
                    (j_player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER &&
                        !sender->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER)))
                {
                    i_player.second.player_ready = false;
                    player_setup_data_changed = true;
                } else {
                    // check loaded empire
                    bool incorrect_empire = false;
                    if (j_player.second.save_game_empire_id != ALL_EMPIRES) {
                        const auto empire_it = m_lobby_data->save_game_empire_data.find(j_player.second.save_game_empire_id);
                        if (empire_it != m_lobby_data->save_game_empire_data.end()) {
                            if (empire_it->second.eliminated) {
                                WarnLogger(FSM) << "Trying to take over eliminated empire \"" << empire_it->second.empire_name << "\"";
                                incorrect_empire = true;
                            } else if (empire_it->second.authenticated) {
                                if (empire_it->second.player_name != sender->PlayerName()) {
                                    WarnLogger(FSM) << "Unauthorized access to protected empire \"" << empire_it->second.empire_name << "\"."
                                                    << " Expected player \"" << empire_it->second.player_name << "\""
                                                    << " got \"" << sender->PlayerName() << "\"";
                                    incorrect_empire = true;
                                }
                            }
                        } else {
                            WarnLogger(FSM) << "Unknown empire #" << j_player.second.save_game_empire_id;
                            incorrect_empire = true;
                        }
                    }
                    if (incorrect_empire) {
                        i_player.second.player_ready = false;
                        player_setup_data_changed = true;
                        break;
                    }

                    has_important_changes = IsPlayerChanged(i_player.second, j_player.second);
                    player_setup_data_changed = ! (i_player.second == j_player.second);

                    i_player.second = std::move(j_player.second);
                }

                break;
            }
            break;
        }
    }

    ValidateClientLimits();
    if (m_lobby_data->start_locked) {
        has_important_changes = true;
    }

    // to determine if a new save file was selected, check if the selected file
    // index is different, and the new file index is in the valid range
    bool new_save_file_selected = false;
    std::string new_file = incoming_lobby_data.save_game;
    std::string old_file = m_lobby_data->save_game;
    if (new_file != old_file) {
        new_save_file_selected = true;

        // update selected file index
        m_lobby_data->save_game = new_file;

        // remove all AIs from current lobby data,
        // so that when the save is loaded no AI state as appropriate,
        // without having potential extra AIs lingering from the previous
        m_lobby_data->players.remove_if([](const std::pair<int, PlayerSetupData>& plr) {
            return plr.second.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
        });
        m_ai_next_index = 1;

        // reset assigned empires in save game for all players.  new loaded game may not have the same set of empire IDs to choose from
        for (auto& psd : m_lobby_data->players) {
            psd.second.save_game_empire_id = ALL_EMPIRES;
            psd.second.empire_color = CLR_ZERO;
        }

        // refresh save game empire data
        boost::filesystem::path save_dir(GetServerSaveDir());
        std::vector<PlayerSaveHeaderData> player_save_header_data;
        try {
            LoadEmpireSaveGameData((save_dir / m_lobby_data->save_game).string(),
                                   m_lobby_data->save_game_empire_data,
                                   player_save_header_data,
                                   *m_lobby_data,
                                   m_lobby_data->save_game_current_turn);

            // read all AI players from save game and add them into current lobby
            // with appropriate empire's data
            for (const auto& pshd : player_save_header_data) {
                if (pshd.client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
                    continue;
                const auto& empire_data_it = m_lobby_data->save_game_empire_data.find(pshd.empire_id);
                if (empire_data_it == m_lobby_data->save_game_empire_data.end())
                    continue;

                PlayerSetupData player_setup_data;
                player_setup_data.player_id =     Networking::INVALID_PLAYER_ID;
                // don't use original names to prevent collision with manually added AIs
                player_setup_data.player_name =   UserString("AI_PLAYER") + "_" + std::to_string(m_ai_next_index++);
                player_setup_data.client_type =   Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
                player_setup_data.save_game_empire_id = pshd.empire_id;
                player_setup_data.empire_name =   empire_data_it->second.empire_name;
                player_setup_data.empire_color =  empire_data_it->second.color;
                if (m_lobby_data->seed != "")
                    player_setup_data.starting_species_name = server.GetSpeciesManager().RandomPlayableSpeciesName();
                else
                    player_setup_data.starting_species_name = server.GetSpeciesManager().SequentialPlayableSpeciesName(m_ai_next_index);
                m_lobby_data->players.push_back({Networking::INVALID_PLAYER_ID, player_setup_data});
            }

            // reset empire color of non-AI player to unused
            for (auto& psd : m_lobby_data->players) {
                if (psd.second.save_game_empire_id == ALL_EMPIRES)
                    psd.second.empire_color = GetUnusedEmpireColour(m_lobby_data->players, m_lobby_data->save_game_empire_data);
            }
        } catch (...) {
            // inform player who attempted to change the save file that there was a problem
            sender->SendMessage(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), false));
            // revert to old save file
            m_lobby_data->save_game = old_file;
        }
    }

    if (has_important_changes) {
        for (auto& player : m_lobby_data->players)
            player.second.player_ready = false;
    } else {
        // check if all established human players ready to play
        bool is_all_ready = true;
        for (auto& player : m_lobby_data->players) {
            if ((player.first >= 0) && (player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
                player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR ||
                player.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER))
            {
                if (! player.second.player_ready)
                    is_all_ready = false;
            }
        }

        if (is_all_ready) {
            // TODO: merge this code with MPLobby::react(const StartMPGame& msg)
            // start game

            if (!m_lobby_data->new_game) {
                // Load game ...
                std::string save_filename = PathToString((GetServerSaveDir() / m_lobby_data->save_game));

                try {
                    LoadGame(save_filename,             *m_server_save_game_data,
                             m_player_save_game_data,   server.GetUniverse(),
                             server.Empires(),          server.GetSpeciesManager(),
                             GetCombatLogManager(),     server.m_galaxy_setup_data);
                    int seed = 0;
                    try {
                        seed = boost::lexical_cast<unsigned int>(server.m_galaxy_setup_data.seed);
                    } catch (...) {
                        try {
                            boost::hash<std::string> string_hash;
                            std::size_t h = string_hash(server.m_galaxy_setup_data.seed);
                            seed = static_cast<unsigned int>(h);
                        } catch (...) {}
                    }
                    DebugLogger(FSM) << "Seeding with loaded galaxy seed: " << server.m_galaxy_setup_data.seed << "  interpreted as actual seed: " << seed;
                    Seed(seed);

                } catch (...) {
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
    for (auto player_connection_it = server.m_networking.established_begin();
         player_connection_it != server.m_networking.established_end(); ++player_connection_it)
    {
        const PlayerConnectionPtr& player_connection = *player_connection_it;
        const int player_id = player_connection->PlayerID();
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
    TraceLogger(FSM) << "(ServerFSM) MPLobby.LobbyChat";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    std::string data;
    std::set<int> recipients;
    bool pm;
    try  {
        ExtractPlayerChatMessageData(message, recipients, data, pm);
    } catch (...) {}

    boost::posix_time::ptime timestamp = boost::posix_time::second_clock::universal_time();

    if (recipients.empty() && sender->GetClientType() != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
    {
        EmpireColor text_color = CLR_SERVER;
        for (const auto& player : m_lobby_data->players) {
            if (player.first != sender->PlayerID())
                continue;
            text_color = player.second.empire_color;
        }
        server.PushChatMessage(data, sender->PlayerName(), text_color, timestamp);
    }

    if (recipients.empty()) { // the receiver is everyone
        for (auto it = server.m_networking.established_begin(); it != server.m_networking.established_end(); ++it) {
            (*it)->SendMessage(ServerPlayerChatMessage(sender->PlayerID(), timestamp, data, pm));
        }
    } else {
        for (auto it = server.m_networking.established_begin(); it != server.m_networking.established_end(); ++it) {
            if (recipients.find((*it)->PlayerID()) != recipients.end())
                (*it)->SendMessage(ServerPlayerChatMessage(sender->PlayerID(), timestamp, data, pm));
        }
    }

    return discard_event();
}

sc::result MPLobby::react(const StartMPGame& msg) {
    TraceLogger(FSM) << "(ServerFSM) MPLobby.StartMPGame";
    ServerApp& server = Server();
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    if (server.m_networking.PlayerIsHost(sender->PlayerID())) {
        if (m_lobby_data->new_game) {
            // if all expected player already connected, can skip waiting for
            // MP joiners and go directly to playing game
            int expected_players = m_lobby_data->players.size();
            if (expected_players == static_cast<int>(server.m_networking.NumEstablishedPlayers())) {
                server.NewMPGameInit(*m_lobby_data);
                return transit<PlayingGame>();
            }
            // otherwise, transit to waiting for MP joiners

        } else {
            // Load game...
            std::string save_filename = (GetServerSaveDir() / m_lobby_data->save_game).string();

            try {
                LoadGame(save_filename,             *m_server_save_game_data,
                         m_player_save_game_data,   server.GetUniverse(),
                         server.Empires(),          server.GetSpeciesManager(),
                         GetCombatLogManager(),     server.m_galaxy_setup_data);
                int seed = 0;
                try {
                    seed = boost::lexical_cast<unsigned int>(server.m_galaxy_setup_data.seed);
                } catch (...) {
                    try {
                        boost::hash<std::string> string_hash;
                        std::size_t h = string_hash(server.m_galaxy_setup_data.seed);
                        seed = static_cast<unsigned int>(h);
                    } catch (...) {}
                }
                DebugLogger(FSM) << "Seeding with loaded galaxy seed: " << server.m_galaxy_setup_data.seed << "  interpreted as actual seed: " << seed;
                Seed(seed);

            } catch (...) {
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
        ErrorLogger(FSM) << "(ServerFSM) MPLobby.StartMPGame : Player #" << sender->PlayerID()
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
    ErrorLogger(FSM) << "MPLobby::react(const HostMPGame& msg) received HostMPGame message but is already in the MP Lobby.  Aborting connection";
    msg.m_player_connection->SendMessage(ErrorMessage(UserStringNop("SERVER_ALREADY_HOSTING_GAME"), true));
    Server().m_networking.Disconnect(msg.m_player_connection);
    return discard_event();
}

sc::result MPLobby::react(const HostSPGame& msg) {
    ErrorLogger(FSM) << "MPLobby::react(const HostSPGame& msg) received HostSPGame message but is already in the MP Lobby.  Aborting connection";
    msg.m_player_connection->SendMessage(ErrorMessage(UserStringNop("SERVER_ALREADY_HOSTING_GAME"), true));
    Server().m_networking.Disconnect(msg.m_player_connection);
    return discard_event();
}

sc::result MPLobby::react(const ShutdownServer& msg) {
    TraceLogger(FSM) << "(ServerFSM) MPLobby.ShutdownServer";

    return transit<ShuttingDownServer>();
}

sc::result MPLobby::react(const Hostless& msg) {
    TraceLogger(FSM) << "(ServerFSM) MPLobby.Hostless";

    return discard_event();
}

sc::result MPLobby::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal && !Server().IsHostless())
        return transit<ShuttingDownServer>();
    return discard_event();
}

////////////////////////////////////////////////////////////
// WaitingForSPGameJoiners
////////////////////////////////////////////////////////////
WaitingForSPGameJoiners::WaitingForSPGameJoiners(my_context c) :
    my_base(c),
    m_single_player_setup_data(context<ServerFSM>().m_single_player_setup_data),
    m_server_save_game_data(std::make_shared<ServerSaveGameData>()),
    m_num_expected_players(0)
{
    TraceLogger(FSM) << "(ServerFSM) WaitingForSPGameJoiners";

    context<ServerFSM>().m_single_player_setup_data.reset();
    ServerApp& server = Server();
    std::vector<PlayerSetupData>& players = m_single_player_setup_data->players;

    // Ensure all players have unique non-empty names   // TODO: the uniqueness part...
    unsigned int player_num = 1;
    int player_id = server.m_networking.NewPlayerID();

    if (m_single_player_setup_data->new_game) {
        // for new games, single player setup data contains full m_players
        // vector, so can just use the contents of that to create AI
        // clients
        for (auto& psd : players) {
            if (psd.player_name.empty()) {
                if (psd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
                    psd.player_name = "AI_" + std::to_string(player_num++);
                else if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
                    psd.player_name = "Human_Player_" + std::to_string(player_num++);
                else if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER)
                    psd.player_name = "Observer_" + std::to_string(player_num++);
                else if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
                    psd.player_name = "Moderator_" + std::to_string(player_num++);
                else
                    psd.player_name = "Player_" + std::to_string(player_num++);
            }

            if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
                psd.player_id = server.Networking().HostPlayerID();
            } else {
                psd.player_id = player_id++;
            }
        }

    } else {
        // for loaded games, all that is specified is the filename, and the
        // server needs to populate single player setup data's m_players
        // with data from the save file.
        if (!players.empty()) {
            ErrorLogger(FSM) << "WaitingForSPGameJoiners::WaitingForSPGameJoiners got single player setup data to load a game, but also player setup data for a new game.  Ignoring player setup data";
            players.clear();
        }

        std::vector<PlayerSaveHeaderData> player_save_header_data;
        try {
            LoadPlayerSaveHeaderData(m_single_player_setup_data->filename, player_save_header_data);
        } catch (const std::exception&) {
            SendMessageToHost(ErrorMessage(UserStringNop("UNABLE_TO_READ_SAVE_FILE"), true));
            post_event(LoadSaveFileFailed());
            return;
        }

        // add player setup data for each player in saved gamed
        for (PlayerSaveHeaderData& psgd : player_save_header_data) {
            if (psgd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER ||
                psgd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            {
                players.emplace_back();
                auto& psd = players.back();

                psd.player_name = std::move(psgd.name);
                //psd.empire_name // left default
                //psd.empire_color // left default
                //psd.starting_species_name // left default
                psd.save_game_empire_id = psgd.empire_id;
                psd.client_type = psgd.client_type;

                if (psd.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
                    psd.player_id = server.Networking().HostPlayerID();
                } else {
                    psd.player_id = player_id++;
                }
            }
        }
    }

    m_num_expected_players = players.size();
    m_expected_ai_names_and_ids.clear();
    for (const auto& player_data : players) {
        if (player_data.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            m_expected_ai_names_and_ids.emplace(player_data.player_name, player_data.player_id);
    }

    server.CreateAIClients(players, int(m_single_player_setup_data->ai_aggr));    // also disconnects any currently-connected AI clients

    // if Python fails to initialize, don't bother initializing SP game.
    // Still check start conditions, which will abort the server after all
    // the expected players have joined if Python is (still) not initialized.
    if (server.m_python_server.IsPythonRunning()) {
        if (m_single_player_setup_data->new_game) {
            // For new SP game start inializaing while waiting for AI callbacks.
            DebugLogger(FSM) << "Initializing new SP game...";
            server.NewSPGameInit(*m_single_player_setup_data);
        }
    }

    // force immediate check if all expected AIs are present, so that the FSM
    // won't get stuck in this state waiting for JoinGame messages that will
    // never come since no other AIs are left to join
    post_event(CheckStartConditions());
}

WaitingForSPGameJoiners::~WaitingForSPGameJoiners()
{ TraceLogger(FSM) << "(ServerFSM) ~WaitingForSPGameJoiners"; }

sc::result WaitingForSPGameJoiners::react(const JoinGame& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForSPGameJoiners.JoinGame";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    auto& player_connection = msg.m_player_connection;

    std::string player_name("Default_Player_Name_in_WaitingForSPGameJoiners::react(const JoinGame& msg)");
    Networking::ClientType client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    std::string client_version_string;
    std::map<std::string, std::string> dependencies;
    boost::uuids::uuid cookie = boost::uuids::nil_generator{}();
    try {
        ExtractJoinGameMessageData(message, player_name, client_type, client_version_string,
                                   dependencies, cookie);
    } catch (...) {}

    DebugLogger() << "Player " << player_name << " has dependencies: " << StringifyDependencies(dependencies);

    // is this an AI?
    if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
        const auto& expected_it = m_expected_ai_names_and_ids.find(player_name);
        // verify that player name was expected
        if (expected_it == m_expected_ai_names_and_ids.end()) {
            // unexpected ai player
            ErrorLogger(FSM) << "WaitingForSPGameJoiners::react(const JoinGame& msg) received join game message for player \"" << player_name << "\" which was not an expected AI player name.    Terminating connection.";
            server.m_networking.Disconnect(std::move(player_connection));

        } else {
            // expected player
            // let the networking system know what socket this player is on
            player_connection->EstablishPlayer(expected_it->second, player_name, client_type, // player_name used below
                                               std::move(client_version_string));
            player_connection->SendMessage(JoinAckMessage(expected_it->second, boost::uuids::nil_uuid()));
            if (!GetOptionsDB().Get<bool>("skip-checksum"))
                player_connection->SendMessage(ContentCheckSumMessage(server.GetSpeciesManager()));

            // Inform AI of logging configuration.
            player_connection->SendMessage(
                LoggerConfigMessage(Networking::INVALID_PLAYER_ID,
                                    LoggerOptionsLabelsAndLevels(LoggerTypes::both)));

            // remove name from expected names list, so as to only allow one connection per AI
            m_expected_ai_names_and_ids.erase(std::move(player_name));
        }

    } else if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
        // verify that there is room left for this player
        int already_connected_players = m_expected_ai_names_and_ids.size() + server.m_networking.NumEstablishedPlayers();
        if (already_connected_players >= m_num_expected_players) {
            // too many human players
            ErrorLogger(FSM) << "WaitingForSPGameJoiners::react(const JoinGame& msg): A human player attempted to join the game but there was not enough room.  Terminating connection.";
            // TODO: send message to attempted joiner saying game is full
            server.m_networking.Disconnect(std::move(player_connection));

        } else {
            // unexpected but welcome human player
            const int host_id = server.Networking().HostPlayerID();
            player_connection->EstablishPlayer(host_id, std::move(player_name), client_type,
                                               std::move(client_version_string));
            player_connection->SendMessage(JoinAckMessage(host_id, boost::uuids::nil_uuid()));
            if (!GetOptionsDB().Get<bool>("skip-checksum"))
                player_connection->SendMessage(ContentCheckSumMessage(server.GetSpeciesManager()));

            DebugLogger(FSM) << "Initializing new SP game...";
            server.NewSPGameInit(*m_single_player_setup_data);
        }
    } else {
        ErrorLogger(FSM) << "WaitingForSPGameJoiners::react(const JoinGame& msg): Received JoinGame message with invalid client type: " << client_type;
        return discard_event();
    }

    post_event(CheckStartConditions());
    return discard_event();
}

sc::result WaitingForSPGameJoiners::react(const CheckStartConditions& u) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForSPGameJoiners.CheckStartConditions";
    ServerApp& server = Server();

    // if all expected players have connected, proceed to start new or load game
    if (static_cast<int>(server.m_networking.NumEstablishedPlayers()) == m_num_expected_players) {
        DebugLogger(FSM) << "WaitingForSPGameJoiners::react(const CheckStartConditions& u) : have all " << m_num_expected_players << " expected players connected.";

        if (!server.m_python_server.IsPythonRunning()) {
            post_event(ShutdownServer());
            return discard_event();
        }

        if (m_single_player_setup_data->new_game) {
            DebugLogger(FSM) << "Verify AIs SP game...";
            if (server.VerifySPGameAIs(*m_single_player_setup_data))
                server.SendNewGameStartMessages();

        } else {
            DebugLogger(FSM) << "Loading SP game save file: " << m_single_player_setup_data->filename;
            try {
                LoadGame(m_single_player_setup_data->filename,                  *m_server_save_game_data,
                         m_player_save_game_data,       server.GetUniverse(),   server.Empires(),
                         server.GetSpeciesManager(),    GetCombatLogManager(),  server.m_galaxy_setup_data);

            } catch (...) {
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
    TraceLogger(FSM) << "(ServerFSM) WaitingForSPGameJoiners.LoadSaveFileFailed";
    return transit<Idle>();
}

sc::result WaitingForSPGameJoiners::react(const ShutdownServer& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForSPGameJoiners.ShutdownServer";
    return transit<ShuttingDownServer>();
}

sc::result WaitingForSPGameJoiners::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal) {
        DebugLogger(FSM) << "fatal in joiners";
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
    m_server_save_game_data(context<ServerFSM>().m_server_save_game_data)
{
    TraceLogger(FSM) << "(ServerFSM) WaitingForMPGameJoiners";
    context<ServerFSM>().m_lobby_data.reset();
    context<ServerFSM>().m_player_save_game_data.clear();
    context<ServerFSM>().m_server_save_game_data.reset();
    ServerApp& server = Server();

    std::vector<PlayerSetupData> player_setup_data;
    m_expected_ai_player_names.clear();

    for (std::pair<int, PlayerSetupData>& psd : m_lobby_data->players) {
        player_setup_data.push_back(psd.second);
        if (psd.second.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            m_expected_ai_player_names.emplace(psd.second.player_name);
    }

    server.CreateAIClients(player_setup_data, int(m_lobby_data->ai_aggr));

    server.InitializePython();

    // force immediate check if all expected AIs are present, so that the FSM
    // won't get stuck in this state waiting for JoinGame messages that will
    // never come since no other AIs are left to join
    post_event(CheckStartConditions());
}

WaitingForMPGameJoiners::~WaitingForMPGameJoiners()
{ TraceLogger(FSM) << "(ServerFSM) ~WaitingForMPGameJoiners"; }

sc::result WaitingForMPGameJoiners::react(const JoinGame& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForMPGameJoiners.JoinGame";
    ServerApp& server = Server();
    // due disconnection could cause `delete this` in transition
    // to MPLobby or ShuttingDownServer gets context before disconnection
    ServerFSM& fsm = context<ServerFSM>();
    const Message& message = msg.m_message;
    auto& player_connection = msg.m_player_connection;

    std::string player_name("Default_Player_Name_in_WaitingForMPGameJoiners::react(const JoinGame& msg)");
    Networking::ClientType client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    std::string client_version_string;
    std::map<std::string, std::string> dependencies;
    boost::uuids::uuid cookie = boost::uuids::nil_generator{}();
    try {
        ExtractJoinGameMessageData(message, player_name, client_type, client_version_string,
                                   dependencies, cookie);
    } catch (...) {}

    DebugLogger() << "Player " << player_name << " has dependencies: " << StringifyDependencies(dependencies);

    // is this an AI?
    if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
        // verify that player name was expected
        if (!m_expected_ai_player_names.contains(player_name)) {
            // unexpected ai player
            ErrorLogger(FSM) << "WaitingForMPGameJoiners::react(const JoinGame& msg) received join game message for player \"" << player_name << "\" which was not an expected AI player name.    Terminating connection.";
            server.m_networking.Disconnect(player_connection);
        } else {
            // expected player

            // remove name from expected names list, so as to only allow one connection per AI
            m_expected_ai_player_names.erase(player_name);

            // let the networking system know what socket this player is on
            const int player_id = server.m_networking.NewPlayerID();
            player_connection->EstablishPlayer(player_id, std::move(player_name), client_type,
                                               std::move(client_version_string));
            player_connection->SendMessage(JoinAckMessage(player_id, boost::uuids::nil_uuid()));

            // Inform AI of logging configuration.
            player_connection->SendMessage(
                LoggerConfigMessage(Networking::INVALID_PLAYER_ID,
                                    LoggerOptionsLabelsAndLevels(LoggerTypes::both)));
        }

    } else if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
        // if we don't need to authenticate player we got default roles here
        Networking::AuthRoles roles;
        bool authenticated;

        DebugLogger() << "WaitingForMPGameJoiners.JoinGame Try to login player " << player_name
                      << " with cookie: " << cookie;
        if (server.Networking().CheckCookie(cookie, player_name, roles, authenticated)) {
            // if player has correct and non-expired cookies simply establish him
            player_connection->SetCookie(cookie);
            if (authenticated)
                player_connection->SetAuthenticated();

            // drop other connection with same name before checks for expected players
            std::vector<PlayerConnectionPtr> to_disconnect;
            to_disconnect.reserve(server.m_networking.size());
            for (auto it = server.m_networking.established_begin();
                 it != server.m_networking.established_end(); ++it)
            {
                if ((*it)->PlayerName() == player_name && player_connection != (*it)) {
                    (*it)->SendMessage(ErrorMessage(UserStringNop("ERROR_CONNECTION_WAS_REPLACED"), true));
                    to_disconnect.push_back(*it);
                }
            }
            for (const auto& conn : to_disconnect)
                server.Networking().Disconnect(conn);
        } else {
            if (server.IsAuthRequiredOrFillRoles(player_name, player_connection->GetIpAddress(), roles)) {
                // send authentication request
                player_connection->AwaitPlayer(client_type, client_version_string);
                player_connection->SendMessage(AuthRequestMessage(player_name, "PLAIN-TEXT"));
                return discard_event();
            }
        }

        std::string original_player_name = player_name;

        // Remove AI prefix to distinguish Human from AI.
        std::string ai_prefix = UserString("AI_PLAYER") + "_";
        while (player_name.compare(0, ai_prefix.size(), ai_prefix) == 0)
            player_name.erase(0, ai_prefix.size());
        if (player_name.empty())
            player_name = "_";

        std::string new_player_name = player_name;

        // Try numeric suffixes to get unique name which isn't equal to other player or empire name
        // or registered for authentication.
        bool collision = true;
        std::size_t t = 1;
        while (collision &&
               t <= m_lobby_data->players.size() + server.Networking().GetCookiesSize() + 1)
        {
            collision = false;
            roles.Clear();
            if (!server.IsAvailableName(new_player_name) || server.IsAuthRequiredOrFillRoles(new_player_name, player_connection->GetIpAddress(), roles)) {
                collision = true;
            } else {
                for (std::pair<int, PlayerSetupData>& plr : m_lobby_data->players) {
                    if (plr.second.empire_name == new_player_name) {
                        collision = true;
                        break;
                    }
                }
            }

            if (collision)
                new_player_name = player_name + std::to_string(++t); // start alternative names from 2
        }

        // There is a small chance of unsolveable collision in case of sequential player names are
        // registered for authentication
        if (collision) {
            ErrorLogger(FSM) << "WaitingForMPGameJoiners::react(const JoinGame& msg): couldn't find free player name for \"" << original_player_name << "\"";
            player_connection->SendMessage(ErrorMessage(str(FlexibleFormat(UserString("ERROR_PLAYER_NAME_ALREADY_USED")) % original_player_name),
                                                        true));
            server.Networking().Disconnect(player_connection);
            return discard_event();
        }

        if (!player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER)) {
            if (player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER))
                client_type = Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER;
            else if (player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_MODERATOR))
                client_type = Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR;
            else {
                player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CLIENT_TYPE_NOT_ALLOWED"), true));
                InfoLogger() << "WaitingForMPGameJoiners::react(const JoinGame& msg) player " << player_name
                             << " has client type " << client_type
                             << " version string: " << client_version_string
                             << " and roles: " << roles.Text()
                             << " and is to be disconnected due to the invalid client type";
                server.Networking().Disconnect(player_connection);
                return discard_event();
            }
        }

        fsm.EstablishPlayer(player_connection, std::move(new_player_name), client_type,
                            std::move(client_version_string), roles);
    } else {
        ErrorLogger(FSM) << "WaitingForMPGameJoiners::react(const JoinGame& msg): Received JoinGame message with invalid client type: " << client_type;
        return discard_event();
    }

    // force immediate check if all expected AIs are present, so that the FSM
    // won't get stuck in this state waiting for JoinGame messages that will
    // never come since no other AIs are left to join
    post_event(CheckStartConditions());

    return discard_event();
}

sc::result WaitingForMPGameJoiners::react(const AuthResponse& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForMPGameJoiners.AuthResponse";
    ServerApp& server = Server();
    // due disconnection could cause `delete this` in transition
    // to MPLobby or ShuttingDownServer gets context before disconnection
    ServerFSM& fsm = context<ServerFSM>();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    std::string player_name;
    std::string auth;
    try {
        ExtractAuthResponseMessageData(message, player_name, auth);
    } catch (...) {
        // unable to read message
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION"), true));
        return discard_event();
    }

    Networking::AuthRoles roles;

    if (!server.IsAuthSuccessAndFillRoles(player_name, auth, roles)) {
        // wrong password
        player_connection->SendMessage(ErrorMessage(str(FlexibleFormat(UserString("ERROR_WRONG_PASSWORD")) % player_name),
                                                    true));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }

    player_connection->SetAuthenticated();
    Networking::ClientType client_type = player_connection->GetClientType();

    if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
        // drop other connection with same name before checks for expected players
        std::vector<PlayerConnectionPtr> to_disconnect;
        to_disconnect.reserve(server.m_networking.size());
        for (ServerNetworking::const_established_iterator it = server.m_networking.established_begin();
             it != server.m_networking.established_end(); ++it)
        {
            if ((*it)->PlayerName() == player_name && player_connection != (*it)) {
                (*it)->SendMessage(ErrorMessage(UserStringNop("ERROR_CONNECTION_WAS_REPLACED"), true));
                to_disconnect.push_back(*it);
            }
        }
        for (auto& conn : to_disconnect)
            server.Networking().Disconnect(std::move(conn));

        // expected human player
        if (!player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_PLAYER)) {
            if (player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_OBSERVER))
                client_type = Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER;
            else if (player_connection->HasAuthRole(Networking::RoleType::ROLE_CLIENT_TYPE_MODERATOR))
                client_type = Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR;
            else {
                player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_CLIENT_TYPE_NOT_ALLOWED"), true));
                InfoLogger() << "WaitingForMPGameJoiners::react(const AuthResponse& msg) player " << player_name
                             << " has client type " << client_type
                             << " and roles: " << roles.Text()
                             << " and is to be disconnected due to the invalid client type";
                server.Networking().Disconnect(player_connection);
                return discard_event();
            }
        }

        fsm.EstablishPlayer(std::move(player_connection), std::move(player_name), client_type,
                            player_connection->ClientVersionString(), roles);
    } else {
        // non-human player
        ErrorLogger(FSM) << "WaitingForMPGameJoiners.AuthResponse : A non-human player attempted to join the game.";
        server.m_networking.Disconnect(std::move(player_connection));
    }

    // force immediate check if all expected AIs are present, so that the FSM
    // won't get stuck in this state waiting for JoinGame messages that will
    // never come since no other AIs are left to join
    post_event(CheckStartConditions());

    return discard_event();
}

sc::result WaitingForMPGameJoiners::react(const CheckStartConditions& u) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForMPGameJoiners.CheckStartConditions";
    ServerApp& server = Server();

    if (m_expected_ai_player_names.empty()) {
        if (!server.m_python_server.IsPythonRunning()) {
            post_event(ShutdownServer());
            return discard_event();
        }

        if (m_player_save_game_data.empty()) {
            DebugLogger(FSM) << "Initializing new MP game...";
            server.NewMPGameInit(*m_lobby_data);
        } else {
            DebugLogger(FSM) << "Initializing loaded MP game";
            server.LoadMPGameInit(*m_lobby_data, m_player_save_game_data, m_server_save_game_data);
        }
        return transit<PlayingGame>();
    }

    return discard_event();
}

sc::result WaitingForMPGameJoiners::react(const Hostless& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForMPGameJoiners.Hostless";
    ServerApp& server = Server();

    // Remove the ai processes.  They either all acknowledged the shutdown and are free or were all killed.
    server.m_ai_client_processes.clear();

    // Don't DisconnectAll. It cause segfault here.
    return transit<MPLobby>();
}

sc::result WaitingForMPGameJoiners::react(const ShutdownServer& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForMPGameJoiners.ShutdownServer";
    return transit<ShuttingDownServer>();
}

sc::result WaitingForMPGameJoiners::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal && !Server().IsHostless())
        return transit<ShuttingDownServer>();
    return discard_event();
}

////////////////////////////////////////////////////////////
// PlayingGame
////////////////////////////////////////////////////////////
PlayingGame::PlayingGame(my_context c) :
    my_base(c),
    m_turn_timeout(Server().m_io_context),
    m_start(std::chrono::high_resolution_clock::now())
{
    TraceLogger(FSM) << "(ServerFSM) PlayingGame";

    if (!GetOptionsDB().Get<std::string>("network.server.turn-timeout.first-turn-time").empty()) {
        // Set first turn advance to absolute time point
        try {
            m_turn_timeout.expires_at(boost::posix_time::time_from_string(GetOptionsDB().Get<std::string>("network.server.turn-timeout.first-turn-time")));
            m_turn_timeout.async_wait(boost::bind(&PlayingGame::TurnTimedoutHandler,
                                                  this,
                                                  boost::asio::placeholders::error));
            return;
        } catch (...) {
            WarnLogger(FSM) << "(ServerFSM) PlayingGame: Cann't parse first turn time: "
                            << GetOptionsDB().Get<std::string>("network.server.turn-timeout.first-turn-time");
        }
    }

    if (GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval") > 0 && !Server().IsHaveWinner()) {
        // Set turn advance after time interval
        m_turn_timeout.expires_from_now(boost::posix_time::seconds(GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval")));
        m_turn_timeout.async_wait(boost::bind(&PlayingGame::TurnTimedoutHandler,
                                              this,
                                              boost::asio::placeholders::error));
    }
}

PlayingGame::~PlayingGame() {
    auto duration = std::chrono::high_resolution_clock::now() - m_start;
    DebugLogger(FSM) << "PlayingGame time: " << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << " s";

    TraceLogger(FSM) << "(ServerFSM) ~PlayingGame";
    m_turn_timeout.cancel();
}

sc::result PlayingGame::react(const PlayerChat& msg) {
    TraceLogger(FSM) << "(ServerFSM) PlayingGame.PlayerChat";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    std::string data;
    std::set<int> recipients;
    bool pm = true;
    try {
        ExtractPlayerChatMessageData(message, recipients, data, pm);
    } catch (...) {
        // unable to read message
        sender->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION"), true));
        return discard_event();
    }

    boost::posix_time::ptime timestamp = boost::posix_time::second_clock::universal_time();

    if (recipients.empty() && sender->GetClientType() != Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
        EmpireColor text_color = CLR_SERVER;
        if (auto empire = server.Empires().GetEmpire(server.PlayerEmpireID(sender->PlayerID())))
            text_color = empire->Color();

        server.PushChatMessage(data, sender->PlayerName(), text_color, timestamp);
    }

    for (auto it = server.m_networking.established_begin();
         it != server.m_networking.established_end(); ++it)
    {
        // send message to: (1) specified recipients, (2) all if no recipient  specified, (3) sender
        if (recipients.find((*it)->PlayerID()) != recipients.end() || recipients.empty()
            || ((*it)->PlayerID() == sender->PlayerID()))
        {
            (*it)->SendMessage(ServerPlayerChatMessage(sender->PlayerID(), timestamp,
                                                       data, pm));
        }
    }
    return discard_event();
}

sc::result PlayingGame::react(const Diplomacy& msg) {
    TraceLogger(FSM) << "(ServerFSM) PlayingGame.Diplomacy";
    const Message& message = msg.m_message;
    ServerApp& server = Server();

    DiplomaticMessage diplo_message;
    try {
        ExtractDiplomacyMessageData(message, diplo_message);
        server.Empires().HandleDiplomaticMessage(diplo_message);
    } catch (...) {}

    return discard_event();
}

sc::result PlayingGame::react(const ModeratorAct& msg) {
    TraceLogger(FSM) << "(ServerFSM) PlayingGame.ModeratorAct";
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;
    int player_id = sender->PlayerID();
    ServerApp& server = Server();

    Networking::ClientType client_type = sender->GetClientType();

    if (client_type != Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR) {
        ErrorLogger(FSM) << "PlayingGame::react(ModeratorAct): Non-moderator player sent moderator action, ignoring";
        return discard_event();
    }

    std::unique_ptr<Moderator::ModeratorAction> action;
    try {
        ExtractModeratorActionMessageData(message, action);
    } catch (...) {}

    DebugLogger(FSM) << "PlayingGame::react(ModeratorAct): " << (action ? action->Dump() : "(null)");

    if (action) {
        // execute action
        action->Execute();

        // update player(s) of changed gamestate as result of action
        bool use_binary_serialization = sender->IsBinarySerializationUsed();
        sender->SendMessage(TurnProgressMessage(Message::TurnProgressPhase::DOWNLOADING));
        sender->SendMessage(TurnPartialUpdateMessage(server.PlayerEmpireID(player_id),
                                                     server.GetUniverse(), use_binary_serialization,
                                                     !sender->IsLocalConnection()));
    }

    return discard_event();
}

sc::result PlayingGame::react(const ShutdownServer& msg) {
    TraceLogger(FSM) << "(ServerFSM) PlayingGame.ShutdownServer";

    ServerApp& server = Server();

    if (server.IsHostless() &&
        GetOptionsDB().Get<bool>("save.auto.exit.enabled") &&
        server.CurrentTurn() > 0)
    {
        // save game on exit
        std::string save_filename = GetAutoSaveFileName(server.CurrentTurn(), server.GetGalaxySetupData());
        ServerSaveGameData server_data{server.CurrentTurn()};
        int bytes_written = 0;
        // save game...
        try {
            bytes_written = SaveGame(save_filename,         server_data,        server.GetPlayerSaveGameData(),
                                     server.GetUniverse(),  server.Empires(),   server.GetSpeciesManager(),
                                     GetCombatLogManager(), server.m_galaxy_setup_data,
                                     !server.m_single_player_game);
        } catch (const std::exception& error) {
            ErrorLogger(FSM) << "While saving, catch std::exception: " << error.what();
            SendMessageToAllPlayers(ErrorMessage(UserStringNop("UNABLE_TO_WRITE_SAVE_FILE"), false));
        }

        // inform players that save is complete
        SendMessageToAllPlayers(ServerSaveGameCompleteMessage(save_filename, bytes_written));
    }

    return transit<ShuttingDownServer>();
}

sc::result PlayingGame::react(const Hostless& msg) {
    TraceLogger(FSM) << "(ServerFSM) PlayingGame.Hostless";

    ServerApp& server = Server();

    // Remove the ai processes.  They either all acknowledged the shutdown and are free or were all killed.
    server.m_ai_client_processes.clear();

    // Don't DisconnectAll. It cause segfault here.

    return transit<MPLobby>();
}

sc::result PlayingGame::react(const RequestCombatLogs& msg) {
    DebugLogger(FSM) << "(ServerFSM) PlayingGame::RequestCombatLogs message received";
    Server().UpdateCombatLogs(msg.m_message, msg.m_player_connection);
    return discard_event();
}

void PlayingGame::EstablishPlayer(PlayerConnectionPtr player_connection, std::string player_name,
                                  Networking::ClientType client_type, std::string client_version_string,
                                  Networking::AuthRoles roles)
{
    ServerApp& server = Server();
    // due disconnection could cause `delete this` in transition
    // to MPLobby or ShuttingDownServer gets context before disconnection
    ServerFSM& fsm = context<ServerFSM>();

    if (fsm.EstablishPlayer(player_connection, std::move(player_name), client_type,
                            player_connection->ClientVersionString(), roles))
    {
        // it possible to be not in PlayingGame here
        bool is_in_mplobby = false;
        std::stringstream ss;
        for (auto leaf_state_it = fsm.state_begin(); leaf_state_it != fsm.state_end();) {
            // The following use of typeid assumes that
            // BOOST_STATECHART_USE_NATIVE_RTTI is defined
            const auto& leaf_state = *leaf_state_it;
            ss << typeid(leaf_state).name();
            if (typeid(leaf_state) == typeid(MPLobby))
                is_in_mplobby = true;
            ++leaf_state_it;
            if (leaf_state_it != fsm.state_end())
                ss << ", ";
        }
        ss << "]";
        DebugLogger(FSM) << "(ServerFSM) PlayingGame.EstablishPlayer at " << ss.str();

        if (!is_in_mplobby) {
            if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER ||
                client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
            {
                // send playing game
                server.AddObserverPlayerIntoGame(player_connection);
            } else if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER ||
                       client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
                // previous connection was dropped
                // set empire link to new connection by name
                // send playing game
                server.AddPlayerIntoGame(player_connection, ALL_EMPIRES);
            }
            // In both cases update ingame lobby
            fsm.UpdateIngameLobby();

            // send timeout data
            if (GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval") > 0 &&
                !Server().IsHaveWinner())
            {
                const auto remaining = m_turn_timeout.expires_from_now();
                player_connection->SendMessage(TurnTimeoutMessage(static_cast<int>(remaining.total_seconds())));
            } else {
                player_connection->SendMessage(TurnTimeoutMessage(0));
            }
        }
    }
}

sc::result PlayingGame::react(const JoinGame& msg) {
    DebugLogger(FSM) << "(ServerFSM) PlayingGame::JoinGame message received";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    PlayerConnectionPtr player_connection = msg.m_player_connection;

    std::string player_name;
    Networking::ClientType client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    std::string client_version_string;
    std::map<std::string, std::string> dependencies;
    boost::uuids::uuid cookie = boost::uuids::nil_generator{}();
    try {
        ExtractJoinGameMessageData(message, player_name, client_type, client_version_string,
                                   dependencies, cookie);
    } catch (const std::exception&) {
        ErrorLogger(FSM) << "PlayingGame::react(const JoinGame& msg): couldn't extract data from join game message";
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION"), true));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }

    DebugLogger() << "Player " << player_name << " has dependencies: " << StringifyDependencies(dependencies);

    Networking::AuthRoles roles;
    bool authenticated = false;

    DebugLogger() << "PlayingGame.JoinGame Try to login player " << player_name << " with cookie: " << cookie;
    if (server.Networking().CheckCookie(cookie, player_name, roles, authenticated)) {
        // if player have correct and non-expired cookies simply establish him
        player_connection->SetCookie(cookie);
        if (authenticated)
            player_connection->SetAuthenticated();
    } else {
        const bool relaxed_auth = player_connection->IsLocalConnection() && client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER;
        if (!relaxed_auth && server.IsAuthRequiredOrFillRoles(player_name, player_connection->GetIpAddress(), roles)) {
            // send authentication request
            player_connection->AwaitPlayer(client_type, client_version_string);
            player_connection->SendMessage(AuthRequestMessage(player_name, "PLAIN-TEXT"));
            return discard_event();
        }

        std::string original_player_name = player_name;
        // Remove AI prefix to distinguish Human from AI.
        std::string ai_prefix = UserString("AI_PLAYER") + "_";
        if (client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            while (player_name.compare(0, ai_prefix.size(), ai_prefix) == 0)
                player_name.erase(0, ai_prefix.size());
        }
        if (player_name.empty())
            player_name = "_";

        std::string new_player_name = player_name;

        bool collision = true;
        std::size_t t = 1;
        while (collision &&
               t <= server.Networking().NumEstablishedPlayers() + server.Networking().GetCookiesSize() + 1)
        {
            collision = false;
            roles.Clear();
            if (!server.IsAvailableName(new_player_name) ||
                (!relaxed_auth && server.IsAuthRequiredOrFillRoles(new_player_name,
                                                                   player_connection->GetIpAddress(), roles)))
            {
                collision = true;
            } else {
                for (auto& plr : server.Empires() ) {
                    if (plr.second->Name() == new_player_name) {
                        collision = true;
                        break;
                    }
                }
            }

            if (collision)
                new_player_name = player_name + std::to_string(++t); // start alternative names from 2
        }

        if (collision) {
            WarnLogger() << "Reject player " << original_player_name;
            player_connection->SendMessage(ErrorMessage(str(FlexibleFormat(UserString("ERROR_PLAYER_NAME_ALREADY_USED")) % original_player_name),
                                                        true));
            server.Networking().Disconnect(player_connection);
            return discard_event();
        }

        player_name = std::move(new_player_name);
    }

    EstablishPlayer(std::move(player_connection), std::move(player_name), client_type,
                    std::move(client_version_string), roles);

    return discard_event();
}

sc::result PlayingGame::react(const AuthResponse& msg) {
    DebugLogger(FSM) << "(ServerFSM) PlayingGame::AuthResponse message received";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    PlayerConnectionPtr player_connection = msg.m_player_connection;

    std::string player_name;
    std::string auth;
    try {
        ExtractAuthResponseMessageData(message, player_name, auth);
    } catch (...) {}

    Networking::AuthRoles roles;

    if (!server.IsAuthSuccessAndFillRoles(player_name, auth, roles)) {
        // wrong password
        player_connection->SendMessage(ErrorMessage(str(FlexibleFormat(UserString("ERROR_WRONG_PASSWORD")) % player_name),
                                                    true));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }

    player_connection->SetAuthenticated();
    const Networking::ClientType client_type = player_connection->GetClientType();

    EstablishPlayer(player_connection, std::move(player_name), client_type,
                    player_connection->ClientVersionString(), roles);

    return discard_event();
}

sc::result PlayingGame::react(const EliminateSelf& msg) {
    DebugLogger(FSM) << "(ServerFSM) PlayingGame::EliminateSelf message received";
    ServerApp& server = Server();
    PlayerConnectionPtr player_connection = msg.m_player_connection;

    if (player_connection->GetClientType() != Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER
        && player_connection->GetClientType() != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
    {
        WarnLogger(FSM) << "(ServerFSM) PlayingGame::EliminateSelf non-player connection " << player_connection->PlayerID();
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_NONPLAYER_CANNOT_CONCEDE"), true));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }

    if (!server.EliminatePlayer(player_connection)) {
        WarnLogger(FSM) << "(ServerFSM) PlayingGame::EliminateSelf player " << player_connection->PlayerID()
                        << " not allowed to concede";
        return discard_event();
    }

    server.Networking().Disconnect(std::move(player_connection));

    // check conditions for ending this turn
    post_event(CheckTurnEndConditions());

    return discard_event();
}

sc::result PlayingGame::react(const AutoTurn& msg) {
    DebugLogger(FSM) << "(ServerFSM) PlayingGame::AutoTurn message received";
    ServerApp& server = Server();
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    int player_id = player_connection->PlayerID();

    if (player_connection->GetClientType() != Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER) {
        ErrorLogger(FSM) << "PlayingGame::react(AutoTurn&) Only human client can set empire to auto-turn. Got auto-turn issue from " << player_id;
        player_connection->SendMessage(ErrorMessage(UserStringNop("WRONG_CLIENT_TYPE_AUTOTURN"), false));
        return discard_event();
    }

    int turns_count = 0;

    try {
        turns_count = boost::lexical_cast<int>(msg.m_message.Text());
    } catch (const boost::bad_lexical_cast& ex) {
        ErrorLogger(FSM) << "PlayingGame::react(AutoTurn&) turns count " << msg.m_message.Text() << " is not a number: " << ex.what();
        // incorrect turns count. disconnect player with wrong client.
        player_connection->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION")));
        server.Networking().Disconnect(player_connection);
        return discard_event();
    }

    auto empire = server.Empires().GetEmpire(server.PlayerEmpireID(player_id));
    if (!empire) {
        ErrorLogger(FSM) << "PlayingGame::react(AutoTurn&) couldn't get empire for player with id:" << player_id;
        player_connection->SendMessage(ErrorMessage(UserStringNop("EMPIRE_NOT_FOUND_CANT_HANDLE_ORDERS"), false));
        return discard_event();
    }

    int empire_id = empire->EmpireID();
    if (empire->Eliminated()) {
        ErrorLogger(FSM) << "PlayingGame::react(AutoTurn&) received orders from player " << empire->PlayerName() << "(id: "
                         << player_id << ") who controls empire " << empire_id
                         << " but empire was eliminated";
        player_connection->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();
    }

    empire->SetAutoTurn(turns_count);
    empire->SetReady(turns_count != 0);

    // notify other player that this empire submitted orders
    for (auto player_it = server.m_networking.established_begin();
         player_it != server.m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player_ctn = *player_it;
        player_ctn->SendMessage(PlayerStatusMessage(empire->Ready() ?
                                                    Message::PlayerStatus::WAITING :
                                                    Message::PlayerStatus::PLAYING_TURN,
                                                    empire_id));
    }

    if (empire->Ready()) {
        // check conditions for ending this turn
        post_event(CheckTurnEndConditions());
    }

    return discard_event();
}

sc::result PlayingGame::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal && !Server().IsHostless()) {
        DebugLogger(FSM) << "Fatal received.";
        return transit<ShuttingDownServer>();
    }
    return discard_event();
}

sc::result PlayingGame::react(const LobbyUpdate& msg) {
    TraceLogger(FSM) << "(ServerFSM) PlayingGame.LobbyUpdate";
    ServerApp& server = Server();
    const PlayerConnectionPtr& sender = msg.m_player_connection;
    const Message& message = msg.m_message;

    MultiplayerLobbyData incoming_lobby_data;
    try {
        ExtractLobbyUpdateMessageData(message, incoming_lobby_data);
    } catch (...) {
        sender->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION")));
        return discard_event();
    }

    // try to add the player into the game if he choose empire
    for (const auto& player : incoming_lobby_data.players) {
        if (player.first == sender->PlayerID() && player.second.save_game_empire_id != ALL_EMPIRES) {
            int empire_id = server.AddPlayerIntoGame(sender, player.second.save_game_empire_id);
            if (empire_id != ALL_EMPIRES) {
                server.m_fsm->UpdateIngameLobby();
                return discard_event();
            }
        }
    }

    // ignore data
    sender->SendMessage(ErrorMessage(UserStringNop("SERVER_ALREADY_PLAYING_GAME")));
    server.Networking().Disconnect(sender);

    return discard_event();
}

void PlayingGame::TurnTimedoutHandler(boost::system::error_code error) {
    DebugLogger(FSM) << "(ServerFSM) PlayingGame::TurnTimedoutHandler";

    if (error) {
        DebugLogger() << "Turn timed out cancelled";
        return;
    }

    if (GetOptionsDB().Get<bool>("network.server.turn-timeout.fixed-interval") &&
        GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval") > 0 &&
        !Server().IsHaveWinner())
    {
        auto turn_expired_time = m_turn_timeout.expires_at();
        turn_expired_time += boost::posix_time::seconds(GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval"));
        m_turn_timeout.expires_at(turn_expired_time);
        m_turn_timeout.async_wait(boost::bind(&PlayingGame::TurnTimedoutHandler,
                                              this,
                                              boost::asio::placeholders::error));
    }
    Server().ExpireTurn();
    // check if AI players made their orders and advance turn
    Server().m_fsm->process_event(CheckTurnEndConditions());
}

////////////////////////////////////////////////////////////
// WaitingForTurnEnd
////////////////////////////////////////////////////////////
WaitingForTurnEnd::WaitingForTurnEnd(my_context c) :
    my_base(c),
    m_timeout(Server().m_io_context),
    m_start(std::chrono::high_resolution_clock::now())
{
    TraceLogger(FSM) << "(ServerFSM) WaitingForTurnEnd";
    if (GetOptionsDB().Get<int>("save.auto.interval") > 0) {
        m_timeout.expires_after(std::chrono::seconds(GetOptionsDB().Get<int>("save.auto.interval")));
        m_timeout.async_wait(boost::bind(&WaitingForTurnEnd::SaveTimedoutHandler,
                                         this,
                                         boost::asio::placeholders::error));
    }

    auto& playing_game = context<PlayingGame>();

    // reset turn timer if there no fixed interval and no first turn time set
    if (!GetOptionsDB().Get<bool>("network.server.turn-timeout.fixed-interval")
        && GetOptionsDB().Get<std::string>("network.server.turn-timeout.first-turn-time").empty())
    {
        playing_game.m_turn_timeout.cancel();
        if (GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval") > 0 && !Server().IsHaveWinner()) {
            playing_game.m_turn_timeout.expires_from_now(boost::posix_time::seconds(GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval")));
            playing_game.m_turn_timeout.async_wait(boost::bind(&PlayingGame::TurnTimedoutHandler,
                                                               &playing_game,
                                                               boost::asio::placeholders::error));
        }
    } else {
        // Cleanup time of first turn so it won't be applied second time
        GetOptionsDB().Set<std::string>("network.server.turn-timeout.first-turn-time", "");
    }

    if (GetOptionsDB().Get<int>("network.server.turn-timeout.max-interval") > 0 && !Server().IsHaveWinner()) {
        auto remaining = playing_game.m_turn_timeout.expires_from_now();
        Server().Networking().SendMessageAll(TurnTimeoutMessage(static_cast<int>(remaining.total_seconds())));
    } else {
        Server().Networking().SendMessageAll(TurnTimeoutMessage(0));
    }
}

WaitingForTurnEnd::~WaitingForTurnEnd() {
    const auto duration = std::chrono::high_resolution_clock::now() - m_start;
    DebugLogger(FSM) << "WaitingForTurnEnd time: " << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << " s";

    TraceLogger(FSM) << "(ServerFSM) ~WaitingForTurnEnd";
    m_timeout.cancel();
}

sc::result WaitingForTurnEnd::react(const TurnOrders& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForTurnEnd.TurnOrders";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    OrderSet order_set;
    SaveGameUIData ui_data;
    bool ui_data_available = false;
    std::string save_state_string;
    bool save_state_string_available = false;
    try {
        ExtractTurnOrdersMessageData(message, order_set, ui_data_available, ui_data,
                                     save_state_string_available, save_state_string);
    } catch (const std::exception&) {
        // incorrect turn orders. disconnect player with wrong client.
        sender->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION")));
        server.Networking().Disconnect(sender);
        return discard_event();
    }

    int player_id = sender->PlayerID();
    const Networking::ClientType client_type = sender->GetClientType();

    // clear if it shouldn't have been available...
    if (!ui_data_available)
        ui_data = SaveGameUIData();

    if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER) {
        // observers cannot submit orders. ignore.
        ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnOrders&) received orders from player "
                         << sender->PlayerName()
                         << "(player id: " << player_id << ") "
                               << "who is an observer and should not be sending orders. Orders being ignored.";
        sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();

    } else if (client_type == Networking::ClientType::INVALID_CLIENT_TYPE) {
        // ??? lingering connection? shouldn't get to here. ignore.
        ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnOrders&) received orders from player "
                         << sender->PlayerName()
                         << "(player id: " << player_id << ") "
                               << "who has an invalid player type. The server is confused, and the orders being ignored.";
        sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();

    } else if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR) {
        // if the moderator ends the turn, it is done, regardless of what
        // players are doing or haven't done
        TraceLogger(FSM) << "WaitingForTurnEnd.TurnOrders : Moderator ended turn.";
        post_event(ProcessTurn());
        return transit<ProcessingTurn>();

    } else if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER ||
               client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
    {
        // store empire orders and resume waiting for more
        auto empire = server.Empires().GetEmpire(server.PlayerEmpireID(player_id));
        if (!empire) {
            ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnOrders&) couldn't get empire for player with id:" << player_id;
            sender->SendMessage(ErrorMessage(UserStringNop("EMPIRE_NOT_FOUND_CANT_HANDLE_ORDERS"), false));
            return discard_event();
        }

        int empire_id = empire->EmpireID();
        if (empire->Eliminated()) {
            ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnOrders&) received orders from player " << empire->PlayerName() << "(id: "
                             << player_id << ") who controls empire " << empire_id
                             << " but empire was eliminated";
            sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
            return discard_event();
        }

        for (const auto& order : order_set | range_values) {
            if (!order) {
                ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnOrders&) couldn't get order from order set!";
                continue;
            }
            if (empire_id != order->EmpireID()) {
                ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnOrders&) received orders from player " << empire->PlayerName()
                                 << "(id: " << player_id << ") who controls empire " << empire_id
                                 << " but those orders were for empire " << order->EmpireID() << ".  Orders being ignored.";
                sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
                return discard_event();
            }
        }

        DebugLogger(FSM) << "WaitingForTurnEnd.TurnOrders : Received orders from player " << player_id
                         << " for empire " << empire_id << " count of " << order_set.size();

        server.AddEmpireData(PlayerSaveGameData(sender->PlayerName(), empire_id, std::move(order_set),
                                                std::move(ui_data), std::move(save_state_string), client_type));
        empire->SetAutoTurn(0);
        empire->SetReady(true);

        // notify other player that this empire submitted orders
        for (auto player_it = server.m_networking.established_begin();
             player_it != server.m_networking.established_end(); ++player_it)
        {
            PlayerConnectionPtr player_ctn = *player_it;
            player_ctn->SendMessage(PlayerStatusMessage(Message::PlayerStatus::WAITING, empire_id));
        }
    }

    // inform player who just submitted of their new status.  Note: not sure why
    // this only needs to be send to the submitting player and not all others as
    // well ...
    sender->SendMessage(TurnProgressMessage(Message::TurnProgressPhase::WAITING_FOR_PLAYERS));

    // if player who just submitted is the local human player, raise AI process priority
    // as raising process priority requires superuser privileges on OSX and Linux AFAIK,
    // this piece of code only makes sense on Windows systems
#ifdef FREEORION_WIN32
    if (server.IsLocalHumanPlayer(player_id)) {
        TraceLogger(FSM) << "WaitingForTurnEnd.TurnOrders : Orders received from local human player, raising AI process priority";
        server.SetAIsProcessPriorityToLow(false);
    }
#endif

    // check conditions for ending this turn
    post_event(CheckTurnEndConditions());

    return discard_event();
}

sc::result WaitingForTurnEnd::react(const TurnPartialOrders& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForTurnEnd.TurnPartialOrders";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    OrderSet added;
    std::set<int> deleted;
    try {
        ExtractTurnPartialOrdersMessageData(message, added, deleted);
    } catch (const std::exception&) {
        // incorrect turn orders. disconnect player with wrong client.
        sender->SendMessage(ErrorMessage(UserStringNop("ERROR_INCOMPATIBLE_VERSION")));
        server.Networking().Disconnect(sender);
        return discard_event();
    }

    const int player_id = sender->PlayerID();
    const Networking::ClientType client_type = sender->GetClientType();

    if (client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER) {
        // observers cannot submit orders. ignore.
        ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnPartialOrders&) received orders from player "
                         << sender->PlayerName()
                         << "(player id: " << player_id << ") "
                         << "who is an observer and should not be sending orders. Orders being ignored.";
        sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();

    } else if (client_type == Networking::ClientType::INVALID_CLIENT_TYPE) {
        // ??? lingering connection? shouldn't get to here. ignore.
        ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnPartialOrders&) received orders from player "
                         << sender->PlayerName()
                         << "(player id: " << player_id << ") "
                         << "who has an invalid player type. The server is confused, and the orders being ignored.";
        sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();

    } else if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER ||
               client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
    {
        // store empire orders and resume waiting for more
        auto empire = server.Empires().GetEmpire(server.PlayerEmpireID(player_id));
        if (!empire) {
            ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnPartialOrders&) couldn't get empire for player with id: " << player_id;
            sender->SendMessage(ErrorMessage(UserStringNop("EMPIRE_NOT_FOUND_CANT_HANDLE_ORDERS"), false));
            return discard_event();
        }

        int empire_id = empire->EmpireID();
        if (empire->Eliminated()) {
            ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnPartialOrders&) received orders from player " << empire->PlayerName()
                             << " (id: " << player_id << ") who controls empire " << empire_id
                             << " but empire was eliminated";
            sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
            return discard_event();
        }

        for (const auto& [order_id, order] : added) {
            if (!order) {
                ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnPartialOrders&) couldn't get order from order set!";
                continue;
            }
            if (empire_id != order->EmpireID()) {
                ErrorLogger(FSM) << "WaitingForTurnEnd::react(TurnPartialOrders&) received orders from player " << empire->PlayerName() << "(id: "
                                 << player_id << ") who controls empire " << empire_id
                                 << " but those orders were for empire " << order->EmpireID() << ".  Orders being ignored.";
                sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
                return discard_event();
            }
        }

        TraceLogger(FSM) << "WaitingForTurnEnd.TurnPartialOrders : Received partial orders from player " << player_id;

        server.UpdatePartialOrders(empire_id, std::move(added), deleted);
    }

    return discard_event();
}

sc::result WaitingForTurnEnd::react(const RevertOrders& msg) {
    DebugLogger(FSM) << "(ServerFSM) WaitingForTurnEnd::RevertOrders message received";
    ServerApp& server = Server();
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    const int player_id = sender->PlayerID();

    if (sender->GetClientType() != Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER &&
        sender->GetClientType() != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
    {
        ErrorLogger(FSM) << "WaitingForTurnEnd::react(RevertOrders&) Only player clients can revert orders. Got revert order issue from " << player_id;
        sender->SendMessage(ErrorMessage(UserStringNop("WRONG_CLIENT_TYPE_REVERT_ORDERS"), false));
        return discard_event();
    }

    const auto empire = server.Empires().GetEmpire(server.PlayerEmpireID(player_id));
    if (!empire) {
        ErrorLogger(FSM) << "WaitingForTurnEnd::react(RevertOrders&) couldn't get empire for player with id:" << player_id;
        sender->SendMessage(ErrorMessage(UserStringNop("EMPIRE_NOT_FOUND_CANT_HANDLE_ORDERS"), false));
        return discard_event();
    }

    const int empire_id = empire->EmpireID();
    if (empire->Eliminated()) {
        ErrorLogger(FSM) << "WaitingForTurnEnd::react(RevertOrders&) received orders from player " << empire->PlayerName() << "(id: "
            << player_id << ") who controls empire " << empire_id << " but empire was eliminated";
        sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
        return discard_event();
    }


    empire->SetReady(false);
    server.ClearEmpireTurnOrders(empire_id);

    // re-send player initial turn update
    const auto& context = server.GetContext();

    for (auto& loop_empire : server.Empires() | range_values) {
        loop_empire->UpdateOwnedObjectCounters(server.GetUniverse());
        loop_empire->PrepQueueAvailabilityInfoForSerialization(context);
        loop_empire->PrepPolicyInfoForSerialization(context);
    }

    const bool use_binary_serialization = sender->IsBinarySerializationUsed();
    sender->SendMessage(TurnUpdateMessage(empire_id,                  server.CurrentTurn(),
                                          server.Empires(),           server.GetUniverse(),
                                          server.GetSpeciesManager(), GetCombatLogManager(),
                                          server.GetSupplyManager(),  server.GetPlayerInfoMap(),
                                          use_binary_serialization,   !sender->IsLocalConnection()));


    // notify other players that this empire has not submitted orders
    for (auto player_it = server.m_networking.established_begin();
         player_it != server.m_networking.established_end(); ++player_it)
    {
        PlayerConnectionPtr player_ctn = *player_it;
        player_ctn->SendMessage(PlayerStatusMessage(Message::PlayerStatus::PLAYING_TURN, empire_id));
    }

    return discard_event();
}

sc::result WaitingForTurnEnd::react(const RevokeReadiness& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForTurnEnd.RevokeReadiness";
    ServerApp& server = Server();
    const PlayerConnectionPtr& sender = msg.m_player_connection;

    const int player_id = sender->PlayerID();
    Networking::ClientType client_type = sender->GetClientType();

    if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER ||
        client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
    {
        // store empire orders and resume waiting for more
        auto empire = server.Empires().GetEmpire(server.PlayerEmpireID(player_id));
        if (!empire) {
            ErrorLogger(FSM) << "WaitingForTurnEnd::react(RevokeReadiness&) couldn't get empire for player with id:" << player_id;
            sender->SendMessage(ErrorMessage(UserStringNop("EMPIRE_NOT_FOUND_CANT_HANDLE_ORDERS"), false));
            return discard_event();
        }

        const int empire_id = empire->EmpireID();
        if (empire->Eliminated()) {
            ErrorLogger(FSM) << "WaitingForTurnEnd::react(RevokeReadiness&) received orders from player " << empire->PlayerName() << "(id: "
                             << player_id << ") who controls empire " << empire_id
                             << " but empire was eliminated";
            sender->SendMessage(ErrorMessage(UserStringNop("ORDERS_FOR_WRONG_EMPIRE"), false));
            return discard_event();
        }

        TraceLogger(FSM) << "WaitingForTurnEnd.RevokeReadiness : Revoke orders from player " << player_id;

        empire->SetAutoTurn(0);
        empire->SetReady(false);

        // inform player who just submitted of acknowledge revoking status.
        sender->SendMessage(msg.m_message);

        // notify other players that this empire revoked readiness
        for (auto player_it = server.m_networking.established_begin();
             player_it != server.m_networking.established_end(); ++player_it)
        {
            PlayerConnectionPtr player_ctn = *player_it;
            player_ctn->SendMessage(PlayerStatusMessage(Message::PlayerStatus::PLAYING_TURN, empire_id));
        }
    }

    return discard_event();
}

sc::result WaitingForTurnEnd::react(const CheckTurnEndConditions& c) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForTurnEnd.CheckTurnEndConditions";
    ServerApp& server = Server();
    // is there a moderator in the game?  If so, do nothing, as the turn does
    // not proceed until the moderator orders it
    if (server.m_networking.ModeratorsInGame())
        return discard_event();

    // no moderator; wait for all player orders to be submitted before
    // processing turn.
    if (server.AllOrdersReceived() &&
        (server.IsTurnExpired() ||
        !GetOptionsDB().Get<bool>("network.server.turn-timeout.fixed-interval")))
    {
        // if all players have submitted orders and the server doesn't wait until fixed turn timeout
        // expires, proceed to turn processing
        TraceLogger(FSM) << "WaitingForTurnEnd.TurnOrders : All orders received.";
        post_event(ProcessTurn());
        return transit<ProcessingTurn>();
    }

    // save game so orders from the player will be backuped
    if (server.IsHostless() && GetOptionsDB().Get<bool>("save.auto.hostless.each-player.enabled") && server.CurrentTurn() > 0) {
        PlayerConnectionPtr dummy_connection = nullptr;
        post_event(SaveGameRequest(HostSaveGameInitiateMessage(
            GetAutoSaveFileName(server.CurrentTurn(), server.GetGalaxySetupData())),
            dummy_connection));
    }

    return discard_event();
}

sc::result WaitingForTurnEnd::react(const SaveGameRequest& msg) {
    TraceLogger(FSM) << "(ServerFSM) WaitingForTurnEnd.SaveGameRequest";
    ServerApp& server = Server();
    const Message& message = msg.m_message;
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;

    if (player_connection && !server.m_networking.PlayerIsHost(player_connection->PlayerID())) {
        ErrorLogger(FSM) << "WaitingForTurnEnd.SaveGameRequest : Player #" << player_connection->PlayerID()
                         << " attempted to initiate a game save, but is not the host.  Ignoring request connection.";
        player_connection->SendMessage(ErrorMessage(UserStringNop("NON_HOST_SAVE_REQUEST_IGNORED"), false));
        return discard_event();
    }


    // check if AIs have valid state info
    auto server_player_save_data = server.GetPlayerSaveGameData();
    bool ais_all_have_state = true;
    for (const auto& spsd : server_player_save_data) {
        if (spsd.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER &&
            spsd.save_state_string.empty())
        {
            ais_all_have_state = false;
            break;
        }
    }
    if (!ais_all_have_state) {
        // lacking state info from AIs...
        ErrorLogger() << "WaitingForTurnEnd::react(const SaveGameRequest& msg): don't have save state string for AI(s). Loaded save will lack AI state info, such as aggression setting.";
    }


    // save game...
    try {
        ServerSaveGameData server_data{server.m_current_turn};
        const auto& save_filename = message.Text();
        int bytes_written = SaveGame(save_filename,         server_data,        server_player_save_data,
                                     server.GetUniverse(),  server.Empires(),   server.GetSpeciesManager(),
                                     GetCombatLogManager(), server.m_galaxy_setup_data,
                                     !server.m_single_player_game);
        // inform players that save is complete
        SendMessageToAllPlayers(ServerSaveGameCompleteMessage(save_filename, bytes_written));

    } catch (const std::exception& error) {
        ErrorLogger(FSM) << "While saving, catch std::exception: " << error.what();
        SendMessageToAllPlayers(ErrorMessage(UserStringNop("UNABLE_TO_WRITE_SAVE_FILE"), false));
    }


    return discard_event();
}

void WaitingForTurnEnd::SaveTimedoutHandler(const boost::system::error_code& error) {
    if (error) {
        DebugLogger() << "Save timed out cancelled";
        return;
    }

    if (Server().CurrentTurn() <= 0)
        return;


    DebugLogger() << "Save timed out.";
    PlayerConnectionPtr dummy_connection = nullptr;
    Server().m_fsm->process_event(SaveGameRequest(HostSaveGameInitiateMessage(
        GetAutoSaveFileName(Server().CurrentTurn(), Server().GetGalaxySetupData())),
        dummy_connection));
    if (GetOptionsDB().Get<int>("save.auto.interval") > 0) {
        m_timeout.expires_after(std::chrono::seconds(GetOptionsDB().Get<int>("save.auto.interval")));
        m_timeout.async_wait(boost::bind(&WaitingForTurnEnd::SaveTimedoutHandler,
                                         this,
                                         boost::asio::placeholders::error));
    }
}

////////////////////////////////////////////////////////////
// ProcessingTurn
////////////////////////////////////////////////////////////
ProcessingTurn::ProcessingTurn(my_context c) :
    my_base(c),
    m_start(std::chrono::high_resolution_clock::now())
{ TraceLogger(FSM) << "(ServerFSM) ProcessingTurn"; }

ProcessingTurn::~ProcessingTurn() {
    auto duration = std::chrono::high_resolution_clock::now() - m_start;
    DebugLogger(FSM) << "ProcessingTurn time: " << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << " s";
    TraceLogger(FSM) << "(ServerFSM) ~ProcessingTurn";
}

sc::result ProcessingTurn::react(const ProcessTurn& u) {
    TraceLogger(FSM) << "(ServerFSM) ProcessingTurn.ProcessTurn";

    ServerApp& server = Server();

    // make sure all AI client processes are running with low priority
    server.SetAIsProcessPriorityToLow(true);

    server.PreCombatProcessTurns();
    server.ProcessCombats();
    server.PostCombatProcessTurns();

    // update players that other empires are now playing their turn
    for (const auto& empire : server.Empires()) {
        // inform all players that this empire is playing a turn if not eliminated
        for (auto recipient_player_it = server.m_networking.established_begin();
            recipient_player_it != server.m_networking.established_end();
            ++recipient_player_it)
        {
            const PlayerConnectionPtr& recipient_player_ctn = *recipient_player_it;
            recipient_player_ctn->SendMessage(PlayerStatusMessage(empire.second->Eliminated() || empire.second->Ready() ?
                                                                      Message::PlayerStatus::WAITING :
                                                                      Message::PlayerStatus::PLAYING_TURN,
                                                                  empire.first));
        }
    }

    if (server.IsHostless() && GetOptionsDB().Get<bool>("save.auto.hostless.enabled") && server.CurrentTurn() > 0) {
        PlayerConnectionPtr dummy_connection = nullptr;
        post_event(SaveGameRequest(HostSaveGameInitiateMessage(GetAutoSaveFileName(
            server.CurrentTurn(), server.GetGalaxySetupData())),
            dummy_connection));
    }
    return transit<WaitingForTurnEnd>();
}

sc::result ProcessingTurn::react(const CheckTurnEndConditions& c) {
    TraceLogger(FSM) << "(ServerFSM) ProcessingTurn.CheckTurnEndConditions";
    return discard_event();
}


////////////////////////////////////////////////////////////
// ShuttingDownServer
////////////////////////////////////////////////////////////
const auto SHUTDOWN_POLLING_TIME = std::chrono::milliseconds(5000);

ShuttingDownServer::ShuttingDownServer(my_context c) :
    my_base(c),
    m_player_id_ack_expected(),
    m_timeout(Server().m_io_context, Clock::now() + SHUTDOWN_POLLING_TIME)
{
    TraceLogger(FSM) << "(ServerFSM) ShuttingDownServer";

    ServerApp& server = Server();

    if (server.m_ai_client_processes.empty() && server.m_networking.empty())
        throw ServerApp::NormalExitException();

    DebugLogger(FSM) << "ShuttingDownServer informing AIs game is ending";

    // Inform all players that the game is ending.  Only check the AIs for acknowledgement, because
    // they are the server's child processes.
    for (PlayerConnectionPtr player : server.m_networking) {
        player->SendMessage(EndGameMessage(Message::EndGameReason::PLAYER_DISCONNECT));
        if (player->GetClientType() == Networking::ClientType::CLIENT_TYPE_AI_PLAYER) {
            // If sending message will result in error it will be processed in Disconnection
            // handle.
            m_player_id_ack_expected.emplace(player->PlayerID());
        }
    }

    DebugLogger(FSM) << "ShuttingDownServer expecting " << m_player_id_ack_expected.size() << " AIs to ACK shutdown.";

    // Set the timeout.  If all clients have not responded then kill the remainder and exit
    m_timeout.async_wait(boost::bind(&ServerApp::ShutdownTimedoutHandler,
                                     &server,
                                     boost::asio::placeholders::error));

    post_event(CheckEndConditions());
}

ShuttingDownServer::~ShuttingDownServer()
{ TraceLogger(FSM) << "(ServerFSM) ~ShuttingDownServer"; }

sc::result ShuttingDownServer::react(const LeaveGame& msg) {
    TraceLogger(FSM) << "(ServerFSM) ShuttingDownServer.LeaveGame";
    const PlayerConnectionPtr& player_connection = msg.m_player_connection;
    const int player_id = player_connection->PlayerID();

    const auto ack_found = m_player_id_ack_expected.find(player_id);

    if (ack_found != m_player_id_ack_expected.end()) {
        DebugLogger(FSM) << "Shutdown ACK received for AI " << player_id;
        m_player_id_ack_expected.erase(ack_found);
    } else {
        WarnLogger(FSM) << "Unexpected shutdown ACK received for AI " << player_id;
    }

    post_event(CheckEndConditions());
    return discard_event();
}

sc::result ShuttingDownServer::react(const Disconnection& d) {
    TraceLogger(FSM) << "(ServerFSM) ShuttingDownServer.Disconnection";
    PlayerConnectionPtr& player_connection = d.m_player_connection;
    const int player_id = player_connection->PlayerID();

    // Treat disconnection as an implicit ACK.  Otherwise ignore it.
    const auto ack_found = m_player_id_ack_expected.find(player_id);

    if (ack_found != m_player_id_ack_expected.end()) {
        DebugLogger(FSM) << "Disconnect received for AI " << player_id << ".  Treating it as shutdown ACK.";
        m_player_id_ack_expected.erase(ack_found);
    }

    post_event(CheckEndConditions());
    return discard_event();
}

sc::result ShuttingDownServer::react(const CheckEndConditions& u) {
    TraceLogger(FSM) << "(ServerFSM) ShuttingDownServer.CheckEndConditions";
    ServerApp& server = Server();

    const auto all_acked = m_player_id_ack_expected.empty();

    if (all_acked) {
        DebugLogger(FSM) << "All " << server.m_ai_client_processes.size() << " AIs acknowledged shutdown request.";

        // Free the processes so that they can complete their shutdown.
        for (auto& process : server.m_ai_client_processes)
            process.second.Free();

        post_event(DisconnectClients());
    }

    return discard_event();
}

sc::result ShuttingDownServer::react(const DisconnectClients&) {
    TraceLogger(FSM) << "(ServerFSM) ShuttingDownServer.DisconnectClients";
    ServerApp& server = Server();

    // Remove the ai processes.  They either all acknowledged the shutdown and are free or were all killed.
    server.m_ai_client_processes.clear();

    // Disconnect
    server.m_networking.DisconnectAll();

    throw ServerApp::NormalExitException();

    // Never reached.
}

sc::result ShuttingDownServer::react(const Error& msg) {
    auto fatal = HandleErrorMessage(msg, Server());
    if (fatal)
        return transit<ShuttingDownServer>();
    return discard_event();
}
