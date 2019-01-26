#include "ClientAppFixture.h"

#include "combat/CombatLogManager.h"
#include "network/ClientNetworking.h"
#include "universe/Species.h"
#include "util/Directories.h"
#include "util/GameRules.h"
#include "util/Version.h"

#include "GG/GG/ClrConstants.h"

#include <boost/format.hpp>
#include <boost/uuid/nil_generator.hpp>

ClientAppFixture::ClientAppFixture() :
    m_game_started(false),
    m_cookie(boost::uuids::nil_uuid())
{
#ifdef FREEORION_LINUX
    // Dirty hack to output log to console.
    InitLoggingSystem("/proc/self/fd/1", "Test");
#else
    InitLoggingSystem((GetUserDataDir() / "test.log").string(), "Test");
#endif
    //InitLoggingOptionsDBSystem();

    InfoLogger() << FreeOrionVersionString();
    DebugLogger() << "Test client initialized";

    StartBackgroundParsing();
}

int ClientAppFixture::EffectsProcessingThreads() const
{ return 1; }

bool ClientAppFixture::PingLocalHostServer()
{ return m_networking->PingLocalHostServer(std::chrono::milliseconds(100)); }

bool ClientAppFixture::ConnectToLocalHostServer()
{ return m_networking->ConnectToLocalHostServer(); }

bool ClientAppFixture::ConnectToServer(const std::string& ip_address)
{ return m_networking->ConnectToServer(ip_address); }

void ClientAppFixture::DisconnectFromServer()
{ return m_networking->DisconnectFromServer(); }

void ClientAppFixture::HostSPGame(unsigned int num_AIs) {
    auto game_rules = GetGameRules().GetRulesAsStrings();

    SinglePlayerSetupData setup_data;
    setup_data.m_new_game = true;
    setup_data.m_filename.clear();  // not used for new game

    // GalaxySetupData
    setup_data.SetSeed("TestSeed1");
    setup_data.m_size =             100;
    setup_data.m_shape =            Shape::SPIRAL_4;
    setup_data.m_age =              GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_starlane_freq =    GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_planet_density =   GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_specials_freq =    GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_monster_freq =     GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_native_freq =      GalaxySetupOption::GALAXY_SETUP_MEDIUM;
    setup_data.m_ai_aggr =          Aggression::MANIACAL;
    setup_data.m_game_rules =       game_rules;

    // SinglePlayerSetupData contains a map of PlayerSetupData, for
    // the human and AI players.  Need to compile this information
    // from the specified human options and number of requested AIs

    // Human player setup data
    PlayerSetupData human_player_setup_data;
    human_player_setup_data.m_player_name = "TestPlayer";
    human_player_setup_data.m_empire_name = "TestEmpire";
    human_player_setup_data.m_empire_color = GG::CLR_GREEN;

    human_player_setup_data.m_starting_species_name = "SP_HUMAN";
    human_player_setup_data.m_save_game_empire_id = ALL_EMPIRES; // not used for new games
    human_player_setup_data.m_client_type = Networking::CLIENT_TYPE_HUMAN_PLAYER;

    // add to setup data players
    setup_data.m_players.push_back(human_player_setup_data);

    // AI player setup data.  One entry for each requested AI

    for (unsigned int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
        PlayerSetupData ai_setup_data;

        ai_setup_data.m_player_name = "AI_" + std::to_string(ai_i);
        ai_setup_data.m_empire_name.clear();                // leave blank, to be set by server in Universe::GenerateEmpires
        ai_setup_data.m_empire_color = GG::CLR_ZERO;        // to be set by server
        ai_setup_data.m_starting_species_name.clear();      // leave blank, to be set by server
        ai_setup_data.m_save_game_empire_id = ALL_EMPIRES;  // not used for new games
        ai_setup_data.m_client_type = Networking::CLIENT_TYPE_AI_PLAYER;

        setup_data.m_players.push_back(ai_setup_data);
    }

    m_networking->SendMessage(HostSPGameMessage(setup_data));
}

void ClientAppFixture::JoinGame() {
    m_lobby_updated = false;
    m_networking->SendMessage(JoinGameMessage("TestPlayer",
                                              Networking::CLIENT_TYPE_HUMAN_PLAYER,
                                              m_cookie));
}

bool ClientAppFixture::ProcessMessages(const boost::posix_time::ptime& start_time, int max_seconds) {
    bool to_process = true;
    while ((boost::posix_time::microsec_clock::local_time() - start_time).total_seconds() < max_seconds) {
        if (!m_networking->IsConnected()) {
            ErrorLogger() << "Disconnected";
            return false;
        }
        auto opt_msg = m_networking->GetMessage();
        if (opt_msg) {
            Message msg = *opt_msg;
            if (! HandleMessage(msg)) {
                return false;
            }
            to_process = true;
        } else {
            if (to_process) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                to_process = false;
            } else {
                return true;
            }
        }
    }
    ErrorLogger() << "Timeout";
    return false; // return on timeout
}

bool ClientAppFixture::HandleMessage(Message& msg) {
    InfoLogger() << "Handle message " << msg.Type();
    switch (msg.Type()) {
    case Message::CHECKSUM: {
        bool result = VerifyCheckSum(msg);
        if (!result)
            ErrorLogger() << "Wrong checksum";
        //return result;
        return true;
    }
    case Message::SET_AUTH_ROLES:
        ExtractSetAuthorizationRolesMessage(msg, m_networking->AuthorizationRoles());
        return true;
    case Message::HOST_SP_GAME:
        try {
            int host_id = boost::lexical_cast<int>(msg.Text());
            m_networking->SetPlayerID(host_id);
            m_networking->SetHostPlayerID(host_id);
            return true;
        } catch (const boost::bad_lexical_cast& ex) {
            ErrorLogger() << "Host id " << msg.Text() << " is not a number: " << ex.what();
            return false;
        }
    case Message::TURN_PROGRESS: {
        Message::TurnProgressPhase phase_id;
        ExtractTurnProgressMessageData(msg, phase_id);
        InfoLogger() << "Turn progress: " << phase_id;
        return true;
    }
    case Message::GAME_START: {
        bool single_player_game;     // ignored
        bool loaded_game_data;       // ignored
        bool ui_data_available;      // ignored
        SaveGameUIData ui_data;      // ignored
        bool state_string_available; // ignored
        std::string save_state_string;
        m_empire_status.clear();

        ExtractGameStartMessageData(msg,                     single_player_game,     m_empire_id,
                                    m_current_turn,          m_empires,              m_universe,
                                    GetSpeciesManager(),     GetCombatLogManager(),  GetSupplyManager(),
                                    m_player_info,           m_orders,               loaded_game_data,
                                    ui_data_available,       ui_data,                state_string_available,
                                    save_state_string,       m_galaxy_setup_data);

        InfoLogger() << "Extracted GameStart message for turn: " << m_current_turn << " with empire: " << m_empire_id;

        for (const auto& empire : Empires()) {
            if (GetEmpireClientType(empire.first) == Networking::CLIENT_TYPE_AI_PLAYER)
                m_ai_empires.insert(empire.first);
        }
        m_ai_waiting = m_ai_empires;

        m_game_started = true;
        return true;
    }
    case Message::DIPLOMATIC_STATUS:
    case Message::PLAYER_CHAT:
    case Message::CHAT_HISTORY:
        return true; // ignore
    case Message::PLAYER_STATUS: {
        int ignore_about_player_id;
        int about_empire_id;
        Message::PlayerStatus status;
        ExtractPlayerStatusMessageData(msg, ignore_about_player_id, status, about_empire_id);

        if (status == Message::WAITING) {
            m_ai_waiting.erase(ignore_about_player_id);
        }
        return true;
    }
    case Message::TURN_PARTIAL_UPDATE: {
        ExtractTurnPartialUpdateMessageData(msg, EmpireID(), GetUniverse());
        return true;
    }
    case Message::TURN_UPDATE: {
        ExtractTurnUpdateMessageData(msg,                   EmpireID(),         m_current_turn,
                                     Empires(),             GetUniverse(),      GetSpeciesManager(),
                                     GetCombatLogManager(), GetSupplyManager(), Players());
        m_turn_done = true;
        return true;
    }
    case Message::SAVE_GAME_COMPLETE:
        m_save_completed = true;
        return true;
    case Message::JOIN_GAME: {
        int player_id;
        ExtractJoinAckMessageData(msg, player_id, m_cookie);
        m_networking->SetPlayerID(player_id);
        return true;
    }
    case Message::HOST_ID: {
        int host_id = Networking::INVALID_PLAYER_ID;
        try {
            host_id = boost::lexical_cast<int>(msg.Text());
            m_networking->SetHostPlayerID(host_id);
        } catch (const boost::bad_lexical_cast& ex) {
            ErrorLogger() << "HOST_ID: Could not convert \"" << msg.Text() << "\" to host id";
            return false;
        }
        return true;
    }
    case Message::LOBBY_UPDATE:
        m_lobby_updated = true;
        ExtractLobbyUpdateMessageData(msg, m_lobby_data);
        return true;
    default:
        ErrorLogger() << "Unknown message type: " << msg.Type();
        return false;
    }
}

void ClientAppFixture::SaveGame() {
    std::string save_filename = boost::io::str(boost::format("FreeOrionTestGame_%04d_%s%s") % CurrentTurn() % FilenameTimestamp() % SP_SAVE_FILE_EXTENSION);
    boost::filesystem::path save_dir_path(GetSaveDir() / "test");
    boost::filesystem::path save_path(save_dir_path / save_filename);
    if (!exists(save_dir_path))
        boost::filesystem::create_directories(save_dir_path);

    auto path_string = PathToString(save_path);
    m_save_completed = false;
    m_networking->SendMessage(HostSaveGameInitiateMessage(path_string));

}

void ClientAppFixture::UpdateLobby() {
    m_lobby_updated = false;
    m_networking->SendMessage(LobbyUpdateMessage(m_lobby_data));
}

unsigned int ClientAppFixture::GetLobbyAICount() const {
    unsigned int res = 0;
    for (const auto& plr: m_lobby_data.m_players) {
        if (plr.second.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            ++ res;
    }
    return res;
}

