#include "AIClientApp.h"

#include "../../python/AI/AIFramework.h"
#include "../../util/Logger.h"
#include "../../util/LoggerWithOptionsDB.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Serialize.h"
#include "../../util/i18n.h"
#include "../util/AppInterface.h"
#include "../../network/Message.h"
#include "../../network/ClientNetworking.h"
#include "../util/Random.h"
#include "../util/Version.h"


#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../universe/Universe.h"
#include "../../util/OrderSet.h"
#include "../../util/Order.h"
#include "../../Empire/Empire.h"
#include "../../Empire/Diplomacy.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/functional/hash.hpp>
#include <boost/uuid/nil_generator.hpp>

#include <thread>
#include <chrono>

class CombatLogManager;
CombatLogManager&   GetCombatLogManager();


namespace {

    /** AddTraitBypassOption creates a set of options for debugging of
        the form:
        ai.trait.\<trait name\>.force.enabled  -- If true use the following options to bypass the trait
        ai.trait.\<trait name\>.default        -- If present use this value for all of the AIs not individually set
        ai.trait.\<trait name\>.ai_1           -- Use for AI_1
        ...
        ai.trait.\<trait name\>.ai_40          -- Use for AI_40
     */
    template <typename T>
    void AddTraitBypassOption(OptionsDB& db, std::string const & root, std::string ROOT,
                                 T def, ValidatorBase const & validator) {
        std::string option_root = "ai.trait." + root + ".";
        std::string user_string_root = "OPTIONS_DB_AI_CONFIG_TRAIT_"+ROOT;
        db.Add<bool>(option_root + "force.enabled", UserStringNop(user_string_root + "_FORCE"), false, Validator<bool>());
        db.Add<T>(option_root + "default", UserStringNop(user_string_root + "_FORCE_VALUE"), def, validator);

        for (int ii = 1; ii <= IApp::MAX_AI_PLAYERS(); ++ii) {
            std::stringstream ss;
            ss << option_root << "ai_" << std::to_string(ii);
            db.Add<T>(ss.str(), UserStringNop(user_string_root + "_FORCE_VALUE"), def, validator);
        }
    }

    void AddOptions(OptionsDB& db) {
        // Create options to allow bypassing the traits of the AI
        // character and forcing them all to one value for testing
        // purposes.

        const int max_aggression = 5;
        const int no_value = -1;
        AddTraitBypassOption<int>(db, "aggression", "AGGRESSION", no_value, RangedValidator<int>(no_value, max_aggression));
        AddTraitBypassOption<int>(db, "empire-id", "EMPIREID", no_value, RangedValidator<int>(no_value, IApp::MAX_AI_PLAYERS()));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

}

// static member(s)
AIClientApp::AIClientApp(const std::vector<std::string>& args) :
    m_player_name(""),
    m_max_aggression(0)
{
    if (args.size() < 2) {
        std::cerr << "The AI client should not be executed directly!  Run freeorion to start the game.";
        ExitApp(1);
    }

    // read command line args

    m_player_name = args.at(1);
    if (args.size() >=3) {
        m_max_aggression = boost::lexical_cast<int>(args.at(2));
    }

    // Force the log file if requested.
    if (GetOptionsDB().Get<std::string>("log-file").empty()) {
        const std::string AICLIENT_LOG_FILENAME((GetUserDataDir() / (m_player_name + ".log")).string());
        GetOptionsDB().Set("log-file", AICLIENT_LOG_FILENAME);
    }
    // Force the log threshold if requested.
    auto force_log_level = GetOptionsDB().Get<std::string>("log-level");
    if (!force_log_level.empty())
        OverrideAllLoggersThresholds(to_LogLevel(force_log_level));

    InitLoggingSystem(GetOptionsDB().Get<std::string>("log-file"), "AI");
    InitLoggingOptionsDBSystem();

    InfoLogger() << FreeOrionVersionString();
    DebugLogger() << PlayerName() + " ai client initialized.";
}

AIClientApp::~AIClientApp() {
    Networking().DisconnectFromServer();

    DebugLogger() << "AIClientApp exited cleanly for ai client " << PlayerName();
}

void AIClientApp::operator()()
{ Run(); }

void AIClientApp::ExitApp(int code) {
    DebugLogger() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    if (code)
        exit(code);
    throw NormalExitException();
}

int AIClientApp::EffectsProcessingThreads() const
{ return GetOptionsDB().Get<int>("effects.ai.threads"); }

AIClientApp* AIClientApp::GetApp()
{ return static_cast<AIClientApp*>(s_app); }

const AIBase* AIClientApp::GetAI()
{ return m_AI.get(); }

void AIClientApp::Run() {
    ConnectToServer();

    try {
        StartPythonAI();

        // join game
        Networking().SendMessage(JoinGameMessage(PlayerName(),
                                                 Networking::CLIENT_TYPE_AI_PLAYER,
                                                 boost::uuids::nil_uuid()));

        // Start parsing content
        StartBackgroundParsing();

        // respond to messages until disconnected
        while (1) {
            try {

                if (!Networking().IsRxConnected())
                    break;
                if (const auto msg = Networking().GetMessage())
                    HandleMessage(*msg);
                else
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

            } catch (const boost::python::error_already_set&) {
                /* If the python interpreter is still running then keep
                   going, otherwise exit.*/
                m_AI->HandleErrorAlreadySet();
                if (!m_AI->IsPythonRunning())
                    throw;
            }
        }
    } catch (const NormalExitException&) {
        // intentionally empty.
    } catch (const boost::python::error_already_set&) {
        HandlePythonAICrash();
    }

    Networking().DisconnectFromServer();
}

void AIClientApp::ConnectToServer() {
    if (!Networking().ConnectToLocalHostServer())
        ExitApp(1);
}

void AIClientApp::StartPythonAI() {
    m_AI.reset(new PythonAI());
    if (!(m_AI.get())->Initialize()) {
        HandlePythonAICrash();
        throw std::runtime_error("PythonAI failed to initialize.");
    }
}

void AIClientApp::HandlePythonAICrash() {
    // Note: If python crashed during initialization then the AI has not
    // been associated with a PlayerConnection so the server will not
    // know the AI's PlayerName.
    std::stringstream err_msg;
    err_msg << "AIClientApp failed due to error in python AI code for " << PlayerName() << ".  Exiting Soon.";
    ErrorLogger() << err_msg.str() << " id = " << PlayerID();
    Networking().SendMessage(
        ErrorMessage(str(FlexibleFormat(UserString("ERROR_PYTHON_AI_CRASHED")) % PlayerName()) , true));
}


void AIClientApp::HandleMessage(const Message& msg) {
    //DebugLogger() << "AIClientApp::HandleMessage " << msg.Type();
    switch (msg.Type()) {
    case Message::ERROR_MSG: {
        ErrorLogger() << "AIClientApp::HandleMessage : Received ERROR message from server: " << msg.Text();
        break;
    }

    case Message::HOST_ID: {
        const std::string& text = msg.Text();
        int host_id = Networking::INVALID_PLAYER_ID;
        if (text.empty()) {
            ErrorLogger() << "AIClientApp::HandleMessage for HOST_ID : Got empty message text?!";
        } else {
            try {
                host_id = boost::lexical_cast<int>(text);
            } catch(const boost::bad_lexical_cast& ex) {
                ErrorLogger() << "AIClientApp::HandleMessage for HOST_ID : Couldn't parse message text \"" << text << "\": " << ex.what();
            }
        }
        m_networking->SetHostPlayerID(host_id);
        break;
    }

    case Message::JOIN_GAME: {
        if (PlayerID() == Networking::INVALID_PLAYER_ID) {
            DebugLogger() << "AIClientApp::HandleMessage : Received JOIN_GAME acknowledgement";
            try {
                int player_id;
                boost::uuids::uuid cookie; // ignore
                ExtractJoinAckMessageData(msg, player_id, cookie);
                m_networking->SetPlayerID(player_id);
            } catch(const boost::bad_lexical_cast& ex) {
                ErrorLogger() << "AIClientApp::HandleMessage for JOIN_GAME : Couldn't parse message text \"" << msg.Text() << "\": " << ex.what();
            }
        } else {
            ErrorLogger() << "AIClientApp::HandleMessage : Received erroneous JOIN_GAME acknowledgement when already in a game";
        }
        break;
    }

    case Message::GAME_START: {
        DebugLogger() << "AIClientApp::HandleMessage : Received GAME_START message; starting AI turn...";
        bool single_player_game;        // ignored
        bool loaded_game_data;
        bool ui_data_available;         // ignored
        SaveGameUIData ui_data;         // ignored
        bool state_string_available;    // ignored, as save_state_string is sent even if not set by ExtractMessageData
        std::string save_state_string;
        m_empire_status.clear();

        ExtractGameStartMessageData(msg,                     single_player_game,     m_empire_id,
                                    m_current_turn,          m_empires,              m_universe,
                                    GetSpeciesManager(),     GetCombatLogManager(),  GetSupplyManager(),
                                    m_player_info,           m_orders,               loaded_game_data,
                                    ui_data_available,       ui_data,                state_string_available,
                                    save_state_string,       m_galaxy_setup_data);

        DebugLogger() << "Extracted GameStart message for turn: " << m_current_turn << " with empire: " << m_empire_id;

        GetUniverse().InitializeSystemGraph(m_empire_id);

        DebugLogger() << "Message::GAME_START loaded_game_data: " << loaded_game_data;
        if (loaded_game_data) {
            TraceLogger() << "Message::GAME_START save_state_string: " << save_state_string;
            m_AI->ResumeLoadedGame(save_state_string);
            Orders().ApplyOrders();
        } else {
            DebugLogger() << "Message::GAME_START Starting New Game!";
            // % Distribution of aggression levels
            // Aggression   :  0   1   2   3   4   5   (0=Beginner, 5=Maniacal)
            //                __  __  __  __  __  __
            //Max 0         :100   0   0   0   0   0
            //Max 1         : 25  75   0   0   0   0
            //Max 2         :  0  25  75   0   0   0
            //Max 3         :  0   0  25  75   0   0
            //Max 4         :  0   0   0  25  75   0
            //Max 5         :  0   0   0   0  25  75

            // Optional aggression table, possibly for 0.4.4+?
            // Aggression   :  0   1   2   3   4   5   (0=Beginner, 5=Maniacal)
            //                __  __  __  __  __  __
            //Max 0         :100   0   0   0   0   0
            //Max 1         : 25  75   0   0   0   0
            //Max 2         :  8  17  75   0   0   0
            //Max 3         :  0   8  17  75   0   0
            //Max 4         :  0   0   8  17  75   0
            //Max 5         :  0   0   0   8  17  75

            const std::string g_seed = GetGalaxySetupData().m_seed;
            const std::string emp_name = GetEmpire(m_empire_id)->Name();
            unsigned int my_seed = 0;

            try {
                // generate consistent my_seed values from galaxy seed & empire name.
                boost::hash<std::string> string_hash;
                std::size_t h = string_hash(g_seed);
                my_seed = 3 * static_cast<unsigned int>(h) * static_cast<unsigned int>(string_hash(emp_name));
                DebugLogger() << "Message::GAME_START getting " << emp_name << " AI aggression, RNG Seed: " << my_seed;
            } catch (...) {
                DebugLogger() << "Message::GAME_START getting " << emp_name << " AI aggression, could not initialise RNG.";
            }

            int rand_num = 0;
            int this_aggr = m_max_aggression;

            if (this_aggr > 0  && my_seed > 0) {
                Seed(my_seed);
                rand_num = RandSmallInt(0, 99);
                // if it's in the top 25% then decrease aggression.
                if (rand_num > 74) this_aggr--;
                // Leaving the following as commented out code for now. Possibly for 0.4.4+?
                // in the top 8% ? decrease aggression again, unless it's already as low as it gets.
                // if (rand_num > 91 && this_aggr > 0) this_aggr--;
            }

            DebugLogger() << "Message::GAME_START setting AI aggression as " << this_aggr << " (from rnd " << rand_num << "; max aggression " << m_max_aggression << ")";

            m_AI->SetAggression(this_aggr);
            m_AI->StartNewGame();
        }
        m_AI->GenerateOrders();
        break;
    }

    case Message::SAVE_GAME_COMPLETE:
        break;

    case Message::TURN_UPDATE: {
        //DebugLogger() << "AIClientApp::HandleMessage : extracting turn update message data";
        ExtractTurnUpdateMessageData(msg,                     m_empire_id,        m_current_turn,
                                     m_empires,               m_universe,         GetSpeciesManager(),
                                     GetCombatLogManager(),   GetSupplyManager(), m_player_info);
        //DebugLogger() << "AIClientApp::HandleMessage : generating orders";
        GetUniverse().InitializeSystemGraph(m_empire_id);
        m_AI->GenerateOrders();
        //DebugLogger() << "AIClientApp::HandleMessage : done handling turn update message";
        break;
    }

    case Message::TURN_PARTIAL_UPDATE:
        ExtractTurnPartialUpdateMessageData(msg, m_empire_id, m_universe);
        break;

    case Message::TURN_PROGRESS: {
        Message::TurnProgressPhase phase_id;
        ExtractTurnProgressMessageData(msg, phase_id);
        ClientApp::HandleTurnPhaseUpdate(phase_id);
        break;
    }

    case Message::PLAYER_STATUS:
        break;

    case Message::END_GAME: {
        DebugLogger() << "Message::END_GAME : Exiting";
        DebugLogger() << "Acknowledge server shutdown message.";
        Networking().SendMessage(AIEndGameAcknowledgeMessage());
        ExitApp(0);
        break;
    }

    case Message::PLAYER_CHAT: {
        std::string data;
        int player_id;
        boost::posix_time::ptime timestamp;
        ExtractServerPlayerChatMessageData(msg, player_id, timestamp, data);
        m_AI->HandleChatMessage(player_id, data);
        break;
    }

    case Message::DIPLOMACY: {
        DiplomaticMessage diplo_message;
        ExtractDiplomacyMessageData(msg, diplo_message);
        m_AI->HandleDiplomaticMessage(diplo_message);
        break;
    }

    case Message::DIPLOMATIC_STATUS: {
        DiplomaticStatusUpdateInfo diplo_update;
        ExtractDiplomaticStatusMessageData(msg, diplo_update);
        m_AI->HandleDiplomaticStatusUpdate(diplo_update);
        break;
    }

    case Message::LOGGER_CONFIG: {
         std::set<std::tuple<std::string, std::string, LogLevel>> options;
         ExtractLoggerConfigMessageData(msg, options);

         SetLoggerThresholds(options);
         break;
    }

    case Message::CHECKSUM: {
        TraceLogger() << "(AIClientApp) CheckSum.";
        VerifyCheckSum(msg);
        break;
    }

    default: {
        ErrorLogger() << "AIClientApp::HandleMessage : Received unknown Message type code " << msg.Type();
        break;
    }
    }
    //DebugLogger() << "AIClientApp::HandleMessage done";
}
