#include "AIClientApp.h"

#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Serialize.h"
#include "../../network/Message.h"

#include <boost/lexical_cast.hpp>
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <boost/filesystem/fstream.hpp>


// static member(s)
AIClientApp*  AIClientApp::s_app = 0;

AIClientApp::AIClientApp(int argc, char* argv[]) : 
   m_log_category(log4cpp::Category::getRoot())
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of singleton class AIClientApp");

    s_app = this;

    if (argc < 2) {
        m_log_category.fatal("The AI client should not be called directly!");
        Exit(1);
    }
        
    // read command line args
    SetPlayerName(argv[1]);

    const std::string AICLIENT_LOG_FILENAME((GetLocalDir() / (PlayerName() + ".log")).native_file_string());

    // a platform-independent way to erase the old log
    std::ofstream temp(AICLIENT_LOG_FILENAME.c_str());
    temp.close();

    // establish debug logging
    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", AICLIENT_LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p AI : %m%n");
    appender->setLayout(layout);
    m_log_category.setAdditivity(false);  // make appender the only appender used...
    m_log_category.setAppender(appender);
    m_log_category.setAdditivity(true);   // ...but allow the addition of others later
    m_log_category.setPriority(PriorityValue(GetOptionsDB().Get<std::string>("log-level")));
    m_log_category.debug(PlayerName() + " logger initialized.");
}

AIClientApp::~AIClientApp()
{
    m_log_category.debug("Shutting down " + PlayerName() + " logger...");
    log4cpp::Category::shutdown();
}

void AIClientApp::operator()()
{ Run(); }

void AIClientApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    exit(code);
}

log4cpp::Category& AIClientApp::Logger()
{ return m_log_category; }

AIClientApp* AIClientApp::GetApp()
{ return s_app; }

void AIClientApp::Run()
{
    // connect
    const int MAX_TRIES = 5;
    int tries = 0;
    bool connected = false;
    while (tries < MAX_TRIES && !connected) {
        connected = Networking().ConnectToLocalHostServer();
        ++tries;
    }
    if (!connected) {
        Logger().fatalStream() << "AIClientApp::Initialize : Failed to connect to localhost server after " << MAX_TRIES << " tries.  Exiting.";
        Exit(1);
    }

    // join game
    Networking().SendMessage(JoinGameMessage(PlayerName()));

    // respond to messages until disconnected
    while (1) {
        if (!Networking().Connected())
            break;
        if (Networking().MessageAvailable()) {
            Message msg;
            Networking().GetMessage(msg);
            HandleMessage(msg);
        }
    }
}

void AIClientApp::HandleMessage(const Message& msg)
{
    switch (msg.Type()) {
    case Message::JOIN_GAME: {
        if (msg.SendingPlayer() == -1) {
            if (PlayerID() == -1) {
                SetPlayerID(msg.ReceivingPlayer());
                Logger().debugStream() << "AIClientApp::HandleMessage : Received JOIN_GAME acknowledgement";
            } else {
                Logger().errorStream() << "AIClientApp::HandleMessage : Received erroneous JOIN_GAME acknowledgement when already in a game";
            }
        }
        break;
    }

    case Message::GAME_START: {
        if (msg.SendingPlayer() == -1) {
            Logger().debugStream() << "AIClientApp::HandleMessage : Received GAME_START message; "
                "starting AI turn...";
            bool single_player_game; // note that this is ignored
            SaveGameUIData ui_data; // note that this is ignored
            bool loaded_game_data;
            bool ui_data_available;
            ExtractMessageData(msg, single_player_game, EmpireIDRef(), CurrentTurnRef(), Empires(), GetUniverse(), Orders(), ui_data, loaded_game_data, ui_data_available);
            if (loaded_game_data)
                Orders().ApplyOrders();
            StartTurn();
        }
        break;
    }

    case Message::SAVE_GAME: {
        Networking().SendMessage(ClientSaveDataMessage(PlayerID(), Orders()));
        break;
    }

    case Message::TURN_UPDATE: {
        if (msg.SendingPlayer() == -1) {
            ExtractMessageData(msg, EmpireIDRef(), CurrentTurnRef(), Empires(), GetUniverse());
            StartTurn();
        }
        break;
    }

    case Message::TURN_PROGRESS: 
        break;
          
    case Message::END_GAME: {
        Exit(0);
        break;
    }
       
    default: {
        Logger().errorStream() << "AIClientApp::HandleMessage : Received unknown Message type code " << msg.Type();
        break;
    }
    }
}

void AIClientApp::StartTurn()
{ ClientApp::StartTurn(); }
