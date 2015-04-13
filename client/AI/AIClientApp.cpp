#include "AIClientApp.h"

#include "../../AI/PythonAI.h"
#include "../../util/Logger.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Serialize.h"
#include "../../network/Message.h"
#include "../util/Random.h"

#include "../../universe/System.h"
#include "../../universe/Species.h"
#include "../../universe/Universe.h"
#include "../../util/OrderSet.h"
#include "../../util/Order.h"
#include "../../Empire/Empire.h"
#include "../../Empire/Diplomacy.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>

class CombatLogManager;
CombatLogManager&   GetCombatLogManager();

// static member(s)
AIClientApp::AIClientApp(const std::vector<std::string>& args) :
    m_AI(0),
    m_player_name(""),
    m_max_aggression(0)
{
    if (args.size() < 2) {
        std::cerr << "The AI client should not be executed directly!  Run freeorion to start the game.";
        Exit(1);
    }

    // read command line args

    m_player_name = args.at(1);
    const std::string AICLIENT_LOG_FILENAME((GetUserDir() / (m_player_name + ".log")).string());
    if (args.size() >=3) {
        m_max_aggression = boost::lexical_cast<int>(args.at(2));
    }

    InitLogger(AICLIENT_LOG_FILENAME, "AI");
    DebugLogger() << PlayerName() + " logger initialized.";
}

AIClientApp::~AIClientApp() {
    delete m_AI;
    DebugLogger() << "Shutting down " + PlayerName() + " logger...";
}

void AIClientApp::operator()()
{ Run(); }

void AIClientApp::Exit(int code) {
    DebugLogger() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    exit(code);
}

AIClientApp* AIClientApp::GetApp()
{ return static_cast<AIClientApp*>(s_app); }

const AIBase* AIClientApp::GetAI()
{ return m_AI; }

void AIClientApp::Run() {
    m_AI = new PythonAI();

    // connect
    const int MAX_TRIES = 10;
    int tries = 0;
    volatile bool connected = false;
    while (tries < MAX_TRIES) {
        DebugLogger() << "Attempting to contact server";
        connected = Networking().ConnectToLocalHostServer();
        if (!connected) {
            std::cerr << "FreeOrion AI client server contact attempt " << tries + 1 << " failed." << std::endl;
            ErrorLogger() << "Server contact attempt " << tries + 1 << " failed";
        } else {
            break;
        }
        ++tries;
    }
    if (!connected) {
        DebugLogger() << "AIClientApp::Initialize : Failed to connect to localhost server after " << MAX_TRIES << " tries.  Exiting.";
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
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));
        }
    }
}

void AIClientApp::HandleMessage(const Message& msg) {
    //DebugLogger() << "AIClientApp::HandleMessage " << msg.Type();
    switch (msg.Type()) {
    case Message::ERROR_MSG : {
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
            } catch (...) {
                ErrorLogger() << "AIClientApp::HandleMessage for HOST_ID : Couldn't parese message text: " << text;
            }
        }
        m_networking.SetHostPlayerID(host_id);
        break;
    }

    case Message::JOIN_GAME: {
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID) {
            if (PlayerID() == Networking::INVALID_PLAYER_ID) {
                DebugLogger() << "AIClientApp::HandleMessage : Received JOIN_GAME acknowledgement";
                m_networking.SetPlayerID(msg.ReceivingPlayer());
            } else {
                ErrorLogger() << "AIClientApp::HandleMessage : Received erroneous JOIN_GAME acknowledgement when already in a game";
            }
        }
        break;
    }

    case Message::GAME_START: {
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID) {
            DebugLogger() << "AIClientApp::HandleMessage : Received GAME_START message; starting AI turn...";
            bool single_player_game;        // ignored
            bool loaded_game_data;
            bool ui_data_available;         // ignored
            SaveGameUIData ui_data;         // ignored
            bool state_string_available;    // ignored, as save_state_string is sent even if not set by ExtractMessageData
            std::string save_state_string;
            m_player_status.clear();

            ExtractMessageData(msg,                     single_player_game,     m_empire_id,
                               m_current_turn,          m_empires,              m_universe,
                               GetSpeciesManager(),     GetCombatLogManager(),  m_player_info,
                               m_orders,                loaded_game_data,       ui_data_available,
                               ui_data,                 state_string_available, save_state_string,
                               m_galaxy_setup_data);

            DebugLogger() << "Extracted GameStart message for turn: " << m_current_turn << " with empire: " << m_empire_id;

            GetUniverse().InitializeSystemGraph(m_empire_id);

            DebugLogger() << "Message::GAME_START loaded_game_data: " << loaded_game_data;
            if (loaded_game_data) {
                DebugLogger() << "Message::GAME_START save_state_string: " << save_state_string;
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
        }
        break;
    }

    case Message::SAVE_GAME: {
        //DebugLogger() << "AIClientApp::HandleMessage Message::SAVE_GAME";
        Networking().SendMessage(ClientSaveDataMessage(PlayerID(), Orders(), m_AI->GetSaveStateString()));
        //DebugLogger() << "AIClientApp::HandleMessage sent save data message";
        break;
    }

    case Message::TURN_UPDATE: {
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID) {
            //DebugLogger() << "AIClientApp::HandleMessage : extracting turn update message data";
            ExtractMessageData(msg,                     m_empire_id,        m_current_turn,
                               m_empires,               m_universe,         GetSpeciesManager(),
                               GetCombatLogManager(),   m_player_info);
            //DebugLogger() << "AIClientApp::HandleMessage : generating orders";
            GetUniverse().InitializeSystemGraph(m_empire_id);
            m_AI->GenerateOrders();
            //DebugLogger() << "AIClientApp::HandleMessage : done handling turn update message";
        }
        break;
    }

    case Message::TURN_PARTIAL_UPDATE:
        if (msg.SendingPlayer() == Networking::INVALID_PLAYER_ID)
            ExtractMessageData(msg, m_empire_id, m_universe);
        break;

    case Message::TURN_PROGRESS:
    case Message::PLAYER_STATUS:
        break;

    case Message::END_GAME: {
        DebugLogger() << "Message::END_GAME : Exiting";
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
        m_AI->HandleDiplomaticStatusUpdate(diplo_update);
        break;
    }

    default: {
        ErrorLogger() << "AIClientApp::HandleMessage : Received unknown Message type code " << msg.Type();
        break;
    }
    }
    //DebugLogger() << "AIClientApp::HandleMessage done";
}
