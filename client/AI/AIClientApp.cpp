#include "AIClientApp.h"

#include "../../AI/PythonAI.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Serialize.h"
#include "../../network/Message.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../universe/Universe.h"
#include "../../util/OrderSet.h"
#include "../../util/Order.h"
#include "../../Empire/Empire.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>


// static member(s)
AIClientApp*  AIClientApp::s_app = 0;

AIClientApp::AIClientApp(int argc, char* argv[]) :
    m_AI(0),
    m_player_name("")
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of singleton class AIClientApp");

    s_app = this;

    if (argc < 2) {
        std::cerr << "The AI client should not be executed directly!  Run freeorion to start the game.";
        Exit(1);
    }

    // read command line args

    m_player_name = argv[1];
    const std::string AICLIENT_LOG_FILENAME((GetUserDir() / (m_player_name + ".log")).string());

    // a platform-independent way to erase the old log
    std::ofstream temp(AICLIENT_LOG_FILENAME.c_str());
    temp.close();

    // establish debug logging
    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", AICLIENT_LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p AI : %m%n");
    appender->setLayout(layout);
    Logger().setAdditivity(false);  // make appender the only appender used...
    Logger().setAppender(appender);
    Logger().setAdditivity(true);   // ...but allow the addition of others later
    Logger().setPriority(log4cpp::Priority::DEBUG);
    Logger().debug(PlayerName() + " logger initialized.");
}

AIClientApp::~AIClientApp() {
    delete m_AI;
    Logger().debug("Shutting down " + PlayerName() + " logger...");
}

void AIClientApp::operator()()
{ Run(); }

void AIClientApp::Exit(int code) {
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    exit(code);
}

AIClientApp* AIClientApp::GetApp()
{ return s_app; }

const AIBase* AIClientApp::GetAI()
{ return m_AI; }

void AIClientApp::Run() {
    m_AI = new PythonAI();

    // connect
    const int MAX_TRIES = 10;
    int tries = 0;
    volatile bool connected = false;
    while (tries < MAX_TRIES) {
        Logger().debugStream() << "Attempting to contact server";
        connected = Networking().ConnectToLocalHostServer();
        if (!connected) {
            std::cerr << "FreeOrion AI client server contact attempt " << tries + 1 << " failed." << std::endl;
            Logger().errorStream() << "Server contact attempt " << tries + 1 << " failed";
        } else {
            break;
        }
        ++tries;
    }
    if (!connected) {
        Logger().fatalStream() << "AIClientApp::Initialize : Failed to connect to localhost server after " << MAX_TRIES << " tries.  Exiting.";
        Exit(1);
    }

    // join game
    Networking().SendMessage(JoinGameMessage(PlayerName(), Networking::CLIENT_TYPE_AI_PLAYER));

    // respond to messages until disconnected
    while (1) {
        if (!Networking().Connected())
            break;
        if (Networking().MessageAvailable()) {
            Message msg;
            Networking().GetMessage(msg);
            HandleMessage(msg);
        } else {
            Sleep(250);
        }
    }
}

void AIClientApp::HandleMessage(const Message& msg) {
    //Logger().debugStream() << "AIClientApp::HandleMessage " << msg.Type();
    switch (msg.Type()) {
    case Message::ERROR : {
        Logger().errorStream() << "AIClientApp::HandleMessage : Received ERROR message from server: " << msg.Text();
        break;
    }

    case Message::HOST_ID: {
        const std::string& text = msg.Text();
        int host_id = Networking::INVALID_PLAYER_ID;
        if (text.empty()) {
            Logger().errorStream() << "AIClientApp::HandleMessage for HOST_ID : Got empty message text?!";
        } else {
            try {
                host_id = boost::lexical_cast<int>(text);
            } catch (...) {
                Logger().errorStream() << "AIClientApp::HandleMessage for HOST_ID : Couldn't parese message text: " << text;
            }
        }
        m_networking.SetHostPlayerID(host_id);
        break;
    }

    case Message::JOIN_GAME: {
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID) {
            if (PlayerID() == Networking::INVALID_PLAYER_ID) {
                m_networking.SetPlayerID(msg.ReceivingPlayer());
                Logger().debugStream() << "AIClientApp::HandleMessage : Received JOIN_GAME acknowledgement";
            } else {
                Logger().errorStream() << "AIClientApp::HandleMessage : Received erroneous JOIN_GAME acknowledgement when already in a game";
            }
        }
        break;
    }

    case Message::GAME_START: {
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID) {
            Logger().debugStream() << "AIClientApp::HandleMessage : Received GAME_START message; starting AI turn...";
            bool single_player_game;        // ignored
            bool loaded_game_data;
            bool ui_data_available;         // ignored
            SaveGameUIData ui_data;         // ignored
            bool state_string_available;    // ignored, as save_state_string is sent even if not set by ExtractMessageData
            std::string save_state_string;

            ExtractMessageData(msg,                     single_player_game,     EmpireIDRef(),
                               CurrentTurnRef(),        Empires(),              GetUniverse(),
                               GetSpeciesManager(),     m_player_info,          Orders(),
                               loaded_game_data,        ui_data_available,      ui_data,
                               state_string_available,  save_state_string);

            Logger().debugStream() << "Message::GAME_START loaded_game_data: " << loaded_game_data;
            if (loaded_game_data) {
                Logger().debugStream() << "Message::GAME_START save_state_string: " << save_state_string;
                m_AI->ResumeLoadedGame(save_state_string);
                Orders().ApplyOrders();
            } else {
                Logger().debugStream() << "Message::GAME_START Starting New Game!";
                m_AI->StartNewGame();
            }
            m_AI->GenerateOrders();
        }
        break;
    }

    case Message::SAVE_GAME: {
        //Logger().debugStream() << "AIClientApp::HandleMessage Message::SAVE_GAME";
        Networking().SendMessage(ClientSaveDataMessage(PlayerID(), Orders(), m_AI->GetSaveStateString()));
        //Logger().debugStream() << "AIClientApp::HandleMessage sent save data message";
        break;
    }

    case Message::TURN_UPDATE: {
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID) {
            //Logger().debugStream() << "AIClientApp::HandleMessage : extracting turn update message data";
            ExtractMessageData(msg,
                               EmpireIDRef(),
                               CurrentTurnRef(),
                               Empires(),
                               GetUniverse(),
                               GetSpeciesManager(),
                               m_player_info);
            //Logger().debugStream() << "AIClientApp::HandleMessage : generating orders";
            GetUniverse().InitializeSystemGraph(m_empire_id);
            m_AI->GenerateOrders();
            //Logger().debugStream() << "AIClientApp::HandleMessage : done handling turn update message";
        }
        break;
    }

    case Message::TURN_PARTIAL_UPDATE:
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID) {
            ExtractMessageData(msg,
                               EmpireIDRef(),
                               GetUniverse());
        }
        break;

    case Message::TURN_PROGRESS:
    case Message::PLAYER_STATUS:
        break;

    case Message::COMBAT_START: {
        CombatData combat_data;
        std::vector<CombatSetupGroup> setup_groups;
        Universe::ShipDesignMap foreign_designs;
        ExtractMessageData(msg, combat_data, setup_groups, foreign_designs);
        // TODO: Do we need to do anything here, like decide on overall goals
        // for this combat, so we don't have to figure such things out each
        // turn?
        m_AI->GenerateCombatSetupOrders(combat_data);
        break;
    }

    case Message::COMBAT_TURN_UPDATE: {
        CombatData combat_data;
        ExtractMessageData(msg, combat_data);
        m_AI->GenerateCombatOrders(combat_data);
        break;
    }

    case Message::COMBAT_END:
        // TODO: If we grabbed any other resources on COMBAT_START, release them here.
        break;

    case Message::END_GAME: {
        Exit(0);
        break;
    }

    case Message::PLAYER_CHAT:
        m_AI->HandleChatMessage(msg.SendingPlayer(), msg.Text());
        break;

    case Message::DIPLOMACY: {
        DiplomaticMessage diplo_message;
        ExtractMessageData(msg, diplo_message);
        m_AI->HandleDiplomaticMessage(diplo_message);
        break;
    }

    case Message::DIPLOMATIC_STATUS: {
        DiplomaticStatusUpdateInfo diplo_update;
        ExtractMessageData(msg, diplo_update);
        // TODO: inform AI state of change
        break;
    }

    default: {
        Logger().errorStream() << "AIClientApp::HandleMessage : Received unknown Message type code " << msg.Type();
        break;
    }
    }
    //Logger().debugStream() << "AIClientApp::HandleMessage done";
}
