#include "AIClientApp.h"

#include "AIFramework.h"
#include "../ClientNetworking.h"
#include "../../util/Logger.h"
#include "../../util/LoggerWithOptionsDB.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/i18n.h"
#include "../../util/GameRules.h"
#include "../../util/AppInterface.h"
#include "../../network/Message.h"
#include "../../util/Random.h"
#include "../../util/Version.h"


#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../universe/Universe.h"
#include "../../util/OrderSet.h"
#include "../../util/Order.h"
#include "../../Empire/Empire.h"
#include "../../Empire/Diplomacy.h"

#include "../../parse/PythonParser.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/functional/hash.hpp>
#include <boost/uuid/nil_generator.hpp>

#include <thread>
#include <chrono>

class CombatLogManager;
[[nodiscard]] CombatLogManager& GetCombatLogManager();


namespace {
    /** AddTraitBypassOption creates a set of options for debugging of
        the form:
        ai.trait.\<trait name\>.force.enabled  -- If true use the following options to bypass the trait
        ai.trait.\<trait name\>.default        -- If present use this value for all of the AIs not individually set
        ai.trait.\<trait name\>.ai_1           -- Use for AI_1
        ...
        ai.trait.\<trait name\>.ai_40          -- Use for AI_40
     */
    void AddTraitBypassOption(OptionsDB& db, std::string const & root, std::string ROOT,
                              auto def, const ValidatorBase& validator)
    {
        const std::string option_root = "ai.trait." + root + ".";
        const std::string user_string_root = "OPTIONS_DB_AI_CONFIG_TRAIT_" + ROOT;
        db.Add(option_root + "force.enabled", UserStringNop(user_string_root + "_FORCE"),       false);
        db.Add(option_root + "default",       UserStringNop(user_string_root + "_FORCE_VALUE"), def,    validator.Clone());

        for (int ii = 1; ii <= IApp::MAX_AI_PLAYERS(); ++ii) {
            db.Add(option_root + "ai_" + std::to_string(ii),
                   UserStringNop(user_string_root + "_FORCE_VALUE"), def, validator.Clone());
        }
    }

    void AddOptions(OptionsDB& db) {
        // Create options to allow bypassing the traits of the AI
        // character and forcing them all to one value for testing
        // purposes.

        static constexpr int max_aggression = 5;
        static constexpr int no_value = -1;
        AddTraitBypassOption(db, "aggression", "AGGRESSION", no_value, RangedValidator<int>(no_value, max_aggression));
        AddTraitBypassOption(db, "empire-id",  "EMPIREID",   no_value, RangedValidator<int>(no_value, IApp::MAX_AI_PLAYERS()));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

}

// static member(s)
AIClientApp::AIClientApp(const std::vector<std::string>& args) {
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
        std::string ai_log_dir = GetOptionsDB().Get<std::string>("ai-log-dir");
        std::string AICLIENT_LOG_FILENAME(PathToString((ai_log_dir.empty() ? GetUserDataDir() : FilenameToPath(ai_log_dir)) / (m_player_name + ".log")));
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

void AIClientApp::ExitApp(int code) {
    DebugLogger() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    if (code)
        exit(code);
    throw NormalExitException();
}

int AIClientApp::EffectsProcessingThreads() const
{ return GetOptionsDB().Get<int>("effects.ai.threads"); }

void AIClientApp::Run() {
    ConnectToServer();

    try {
        InitializePythonAI();

        // join game
        Networking().SendMessage(JoinGameMessage(PlayerName(),
                                                 Networking::ClientType::CLIENT_TYPE_AI_PLAYER,
                                                 DependencyVersions(),
                                                 boost::uuids::nil_uuid()));

        // Start parsing content
        std::promise<void> barrier;
        std::future<void> barrier_future = barrier.get_future();
        StartBackgroundParsing(PythonParser(*m_AI, GetResourceDir() / "scripting"), std::move(barrier));
        barrier_future.wait();

        // Import python main module only after game content has been parsed, allowing
        // python to query e.g. NamedReals during module initialization.
        StartPythonAI();

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
        m_AI->HandleErrorAlreadySet();
        HandlePythonAICrash();
    }

    Networking().DisconnectFromServer();
}

void AIClientApp::ConnectToServer() {
    if (!Networking().ConnectToLocalHostServer())
        ExitApp(1);
}

void AIClientApp::InitializePythonAI() {
    m_AI = std::make_unique<PythonAI>();
    if (!(m_AI.get())->Initialize()) {
        HandlePythonAICrash();
        throw std::runtime_error("PythonAI failed to initialize.");
    }
}

void AIClientApp::StartPythonAI() {
    m_AI.get()->Start();
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
    case Message::MessageType::ERROR_MSG: {
        ErrorLogger() << "AIClientApp::HandleMessage : Received ERROR message from server: " << msg.Text();
        break;
    }

    case Message::MessageType::HOST_ID: {
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

    case Message::MessageType::JOIN_GAME: {
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

    case Message::MessageType::GAME_START: {
        DebugLogger() << "AIClientApp::HandleMessage : Received GAME_START message; starting AI turn...";
        bool single_player_game;        // ignored
        bool loaded_game_data;
        bool ui_data_available;         // ignored
        SaveGameUIData ui_data;         // ignored
        bool state_string_available;    // ignored, as save_state_string is sent even if not set by ExtractMessageData
        std::string save_state_string;

        ExtractGameStartMessageData(msg,                     single_player_game,     m_empire_id,
                                    m_current_turn,          m_empires,              m_universe,
                                    m_species_manager,       GetCombatLogManager(),  m_supply_manager,
                                    m_player_info,           m_orders,               loaded_game_data,
                                    ui_data_available,       ui_data,                state_string_available,
                                    save_state_string,       m_galaxy_setup_data);
        m_context.current_turn = m_current_turn;

        DebugLogger() << "Extracted GameStart message for turn: " << m_current_turn << " with empire: " << m_empire_id;

        m_universe.InitializeSystemGraph(m_empires, m_universe.Objects());
        m_universe.UpdateCommonFilteredSystemGraphsWithMainObjectMap(m_empires);

        GetGameRules().SetFromStrings(m_galaxy_setup_data.GetGameRules());

        DebugLogger() << "Message::GAME_START loaded_game_data: " << loaded_game_data;
        if (loaded_game_data) {
            TraceLogger() << "Message::GAME_START save_state_string: " << save_state_string;
            m_AI->ResumeLoadedGame(save_state_string);
            m_orders.ApplyOrders(m_context);
        } else {
            DebugLogger() << "Message::GAME_START Starting New Game!";
            m_AI->StartNewGame();
        }
        m_AI->GenerateOrders();
        break;
    }

    case Message::MessageType::PLAYER_INFO:
        ExtractPlayerInfoMessageData(msg, m_player_info);
        break;

    case Message::MessageType::SAVE_GAME_COMPLETE:
        break;

    case Message::MessageType::TURN_UPDATE: {
        m_orders.Reset();
        //DebugLogger() << "AIClientApp::HandleMessage : extracting turn update message data";
        ExtractTurnUpdateMessageData(msg,                   m_empire_id,      m_current_turn,
                                     m_empires,             m_universe,       m_species_manager,
                                     GetCombatLogManager(), m_supply_manager, m_player_info);
        m_context.current_turn = m_current_turn;
        //DebugLogger() << "AIClientApp::HandleMessage : generating orders";
        m_universe.InitializeSystemGraph(m_empires, m_universe.Objects());
        m_universe.UpdateCommonFilteredSystemGraphsWithMainObjectMap(m_empires);
        m_AI->GenerateOrders();
        //DebugLogger() << "AIClientApp::HandleMessage : done handling turn update message";
        break;
    }

    case Message::MessageType::TURN_PARTIAL_UPDATE:
        ExtractTurnPartialUpdateMessageData(msg, m_empire_id, m_universe);
        break;

    case Message::MessageType::TURN_PROGRESS: {
        Message::TurnProgressPhase phase_id;
        ExtractTurnProgressMessageData(msg, phase_id);
        ClientApp::HandleTurnPhaseUpdate(phase_id);
        break;
    }

    case Message::MessageType::PLAYER_STATUS:
        break;

    case Message::MessageType::END_GAME: {
        DebugLogger() << "Message::END_GAME : Exiting";
        DebugLogger() << "Acknowledge server shutdown message.";
        Networking().SendMessage(AIEndGameAcknowledgeMessage());
        ExitApp(0);
        break;
    }

    case Message::MessageType::PLAYER_CHAT: {
        std::string data;
        int player_id;
        boost::posix_time::ptime timestamp;
        bool pm;
        ExtractServerPlayerChatMessageData(msg, player_id, timestamp, data, pm);
        m_AI->HandleChatMessage(player_id, data);
        break;
    }

    case Message::MessageType::DIPLOMACY: {
        DiplomaticMessage diplo_message;
        ExtractDiplomacyMessageData(msg, diplo_message);
        m_AI->HandleDiplomaticMessage(diplo_message);
        break;
    }

    case Message::MessageType::DIPLOMATIC_STATUS: {
        DiplomaticStatusUpdateInfo diplo_update;
        ExtractDiplomaticStatusMessageData(msg, diplo_update);
        m_AI->HandleDiplomaticStatusUpdate(diplo_update);
        break;
    }

    case Message::MessageType::LOGGER_CONFIG: {
         auto options = ExtractLoggerConfigMessageData(msg);
         SetLoggerThresholds(options);
         break;
    }

    case Message::MessageType::CHECKSUM: {
        TraceLogger() << "(AIClientApp) CheckSum.";
        bool result = VerifyCheckSum(msg);
        if (!result) {
            ErrorLogger() << "Wrong checksum";
            throw std::runtime_error("AI got incorrect checksum.");
        }
        break;
    }

    case Message::MessageType::TURN_TIMEOUT:
        break;

    default: {
        ErrorLogger() << "AIClientApp::HandleMessage : Received unknown Message type code " << msg.Type();
        break;
    }
    }
    //DebugLogger() << "AIClientApp::HandleMessage done";
}
