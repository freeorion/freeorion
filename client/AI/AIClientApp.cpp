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

    InitLogger(AICLIENT_LOG_FILENAME, "%d %p AI : %m%n");
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
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));
        }
    }
}

void AIClientApp::HandleMessage(const Message& msg) {
    //Logger().debugStream() << "AIClientApp::HandleMessage " << msg.Type();
    switch (msg.Type()) {
    case Message::ERROR_MSG : {
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
                Logger().debugStream() << "AIClientApp::HandleMessage : Received JOIN_GAME acknowledgement";
                m_networking.SetPlayerID(msg.ReceivingPlayer());
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

            ExtractMessageData(msg,                     single_player_game,     m_empire_id,
                               m_current_turn,          m_empires,              m_universe,
                               GetSpeciesManager(),     GetCombatLogManager(),  m_player_info,
                               m_orders,                loaded_game_data,       ui_data_available,
                               ui_data,                 state_string_available, save_state_string,
                               m_galaxy_setup_data);

            Logger().debugStream() << "Extracted GameStart message for turn: " << m_current_turn << " with empire: " << m_empire_id;

            GetUniverse().InitializeSystemGraph(m_empire_id);

            Logger().debugStream() << "Message::GAME_START loaded_game_data: " << loaded_game_data;
            if (loaded_game_data) {
                Logger().debugStream() << "Message::GAME_START save_state_string: " << save_state_string;
                m_AI->ResumeLoadedGame(save_state_string);
                Orders().ApplyOrders();
            } else {
                Logger().debugStream() << "Message::GAME_START Starting New Game!";
                // % Distributions   using m_max_aggression range for RandSmallInt
                // Aggression   :  0   1   2   3   4   5   (0=Beginner, 5=Maniacal)
                //                __  __  __  __  __  __
                //Max 0         :100   0   0   0   0   0
                //Max 1         : 25  75   0   0   0   0
                //Max 2         : 11  56  33   0   0   0
                //Max 3         :  6  31  44  19   0   0
                //Max 4         :  4  20  36  28  12   0
                //Max 5         :  3  14  25  31  19   8
                //unsigned mySeed = m_player_name.c_str()[0] + m_player_name.size();
                //
                // % Distributions   using range [0,1] for RandSmallInt
                // Aggression   :  0   1   2   3   4   5   (0=Beginner, 5=Maniacal)
                //                __  __  __  __  __  __
                //Max 0         :100   0   0   0   0   0
                //Max 1         : 25  75   0   0   0   0
                //Max 2         :  0  25  75   0   0   0
                //Max 3         :  0   0  25  75   0   0
                //Max 4         :  0   0   0  25  75   0
                //Max 5         :  0   0   0   0  25  75
                void* seedPtr = &m_player_name;
                unsigned int mySeed = static_cast<unsigned int>(reinterpret_cast<unsigned long>(seedPtr));
                Seed(mySeed);
                int rand1 = 0;
                int rand2 = 0;
                int thisAggr = 0;
                if ( m_max_aggression > 0 ) {
                    rand1 = RandSmallInt(0, 1);
                    rand2 = RandSmallInt(0, 1);
                    thisAggr = m_max_aggression -1 + int(( rand1+rand2+1)/2);
                }
                Logger().debugStream() << "Message::GAME_START getting AI aggression, max is  "<< m_max_aggression;
                Logger().debugStream() << "Message::GAME_START starting new game with AI aggression set to "<< thisAggr <<" from int("<< rand1<<"+"<<rand2<<"+1)/2";

                m_AI->SetAggression(thisAggr);
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
            ExtractMessageData(msg,                     m_empire_id,        m_current_turn,
                               m_empires,               m_universe,         GetSpeciesManager(),
                               GetCombatLogManager(),   m_player_info);
            //Logger().debugStream() << "AIClientApp::HandleMessage : generating orders";
            GetUniverse().InitializeSystemGraph(m_empire_id);
            m_AI->GenerateOrders();
            //Logger().debugStream() << "AIClientApp::HandleMessage : done handling turn update message";
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
        Logger().debugStream() << "Message::END_GAME : Exiting";
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
        Logger().errorStream() << "AIClientApp::HandleMessage : Received unknown Message type code " << msg.Type();
        break;
    }
    }
    //Logger().debugStream() << "AIClientApp::HandleMessage done";
}
