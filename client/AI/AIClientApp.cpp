#include "AIClientApp.h"

#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Serialize.h"
#include "../../network/Message.h"

#include <GG/net/fastevents.h>
#include <GG/net/net2.h>

#include <boost/lexical_cast.hpp>
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include "../../universe/Universe.h"
#include "../../universe/System.h"
#include "../../universe/Fleet.h"
#include "../../universe/Ship.h"
#include "../../universe/ShipDesign.h"
#include "../../util/OrderSet.h"
#include "../../util/Order.h"
#include "../../empire/Empire.h"

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
    m_player_name = argv[1];

    const std::string AICLIENT_LOG_FILENAME((GetLocalDir() / (m_player_name + ".log")).native_file_string());

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
    m_log_category.setPriority(log4cpp::Priority::DEBUG);
    m_log_category.debug(m_player_name + " logger initialized.");
}

AIClientApp::~AIClientApp()
{
    m_log_category.debug("Shutting down " + m_player_name + " logger...");
    log4cpp::Category::shutdown();
}

void AIClientApp::operator()()
{
    Run();
}

void AIClientApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    SDLQuit();
    exit(code);
}

log4cpp::Category& AIClientApp::Logger()
{
    return m_log_category;
}

AIClientApp* AIClientApp::GetApp()
{
    return s_app;
}

void AIClientApp::Run()
{
    try {
        SDLInit();
        Initialize();
        while (1)
            Poll();
    } catch (const std::invalid_argument& exception) {
        m_log_category.fatal("std::invalid_argument Exception caught in AIClientApp::Run(): " + std::string(exception.what()));
        Exit(1);
    } catch (const std::runtime_error& exception) {
        m_log_category.fatal("std::runtime_error Exception caught in AIClientApp::Run(): " + std::string(exception.what()));
        Exit(1);
    }
}

void AIClientApp::SDLInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
        Logger().errorStream() << "SDL initialization failed: " << SDL_GetError();
        Exit(1);
    }

    if (SDLNet_Init() < 0) {
        Logger().errorStream() << "SDL Net initialization failed: " << SDLNet_GetError();
        Exit(1);
    }

    if (FE_Init() < 0) {
        Logger().errorStream() << "FastEvents initialization failed: " << FE_GetError();
        Exit(1);
    }

    if (NET2_Init() < 0) {
        Logger().errorStream() << "SDL Net2 initialization failed: " << NET2_GetError();
        Exit(1);
    }

    Logger().debugStream() << "SDLInit() complete.";
}

void AIClientApp::Initialize()
{
    // join game at server
    const int MAX_TRIES = 5;
    int tries = 0;
    bool connected = false;
    while (tries < MAX_TRIES && !connected) {
        connected = m_network_core.ConnectToLocalhostServer();
        ++tries;
    }
    if (!connected) {
        Logger().fatalStream() << "AIClientApp::Initialize : Failed to connect to localhost server after " << MAX_TRIES << " tries.  Exiting.";
        Exit(1);
    }
    m_network_core.SendMessage(JoinGameMessage(m_player_name));
}

void AIClientApp::Poll()
{
    // handle events
    SDL_Event event;
    while (FE_WaitEvent(&event)) {
        int net2_type = NET2_GetEventType(&event);
        if (event.type == SDL_USEREVENT && 
            (net2_type == NET2_ERROREVENT || 
            net2_type == NET2_TCPACCEPTEVENT || 
            net2_type == NET2_TCPRECEIVEEVENT || 
            net2_type == NET2_TCPCLOSEEVENT || 
            net2_type == NET2_UDPRECEIVEEVENT)) { // an SDL_net2 event
            m_network_core.HandleNetEvent(event);
        } else {
            switch (event.type) {
            case SDL_QUIT:
                Exit(0);
                break;
            }
        }
    }
}

void AIClientApp::FinalCleanup()
{
}

void AIClientApp::SDLQuit()
{
    FinalCleanup();
    NET2_Quit();
    FE_Quit();
    SDLNet_Quit();
    SDL_Quit();
    Logger().debugStream() << "SDLQuit() complete.";
}

void AIClientApp::HandleMessageImpl(const Message& msg)
{
    switch (msg.Type()) {
    case Message::SERVER_STATUS: {
        Logger().debugStream() << "AIClientApp::HandleMessageImpl : Received SERVER_STATUS (status code " << msg.GetText() << ")";
        break;
    }
          
    case Message::JOIN_GAME: {
        if (msg.Sender() == -1) {
            if (m_player_id == -1) {
                m_player_id = boost::lexical_cast<int>(msg.GetText());
                Logger().debugStream() << "AIClientApp::HandleMessageImpl : Received JOIN_GAME acknowledgement";
            } else {
                Logger().errorStream() << "AIClientApp::HandleMessageImpl : Received erroneous JOIN_GAME acknowledgement when already in a game";
            }
        }
        break;
    }

    case Message::RENAME_PLAYER: {
        m_player_name = msg.GetText();
        Logger().debugStream() << "AIClientApp::HandleMessageImpl : Received RENAME_PLAYER -- server has renamed this player \"" << 
            m_player_name  << "\"";
        break;
    }

    case Message::GAME_START: {
        if (msg.Sender() == -1) {
            Logger().debugStream() << "AIClientApp::HandleMessageImpl : Received GAME_START message; "
                "starting AI turn...";
            bool single_player_game; // note that this is ignored
            ExtractMessageData(msg, single_player_game, m_empire_id, m_current_turn, Empires(), GetUniverse());

            Logger().debugStream() << "Got Turn Update message, extracted message data";

            // ... copied from HumanClientApp.cpp.  Not sure if / why it's necessary.
            // if this is the last turn, the TCP message handling inherent in Autosave()'s synchronous message may have
            // processed an end-of-game message, in which case we need *not* to execute these last two lines below
            if (!NetworkCore().Connected()) break;
            Logger().debugStream() << "Still connected, starting to generate AI orders";

            AIGenerateOrders();

            Logger().debugStream() << "Generated AI Orders, starting turn update";

            StartTurn();

            Logger().debugStream() << "Done dealing with turn update message";
        }
        break;
    }

    case Message::SAVE_GAME: {
        NetworkCore().SendMessage(ClientSaveDataMessage(m_player_id, m_orders));
        break;
    }

    case Message::LOAD_GAME: {
        // HACK! We're just ignoring the rest of the message, since we only care about the orders
        ExtractMessageData(msg, Orders());
        Orders().ApplyOrders();
        break;
    }

    case Message::TURN_UPDATE: {
        if (msg.Sender() == -1) {
            ExtractMessageData(msg, m_empire_id, m_current_turn, Empires(), GetUniverse());

            Logger().debugStream() << "Got Turn Update message, extracted message data";

            // ... copied from HumanClientApp.cpp.  Not sure if / why it's necessary.
            // if this is the last turn, the TCP message handling inherent in Autosave()'s synchronous message may have
            // processed an end-of-game message, in which case we need *not* to execute these last two lines below
            if (!NetworkCore().Connected()) break;
            Logger().debugStream() << "Still connected, starting to generate AI orders";

            AIGenerateOrders();

            Logger().debugStream() << "Generated AI Orders, starting turn update";

            StartTurn();

            Logger().debugStream() << "Done dealing with turn update message";
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
        Logger().errorStream() << "AIClientApp::HandleMessageImpl : Received unknown Message type code " << msg.Type();
        break;
    }
    }
}

void AIClientApp::HandleServerDisconnectImpl()
{
    Exit(1);
}

void AIClientApp::StartTurn()
{
    ClientApp::StartTurn();
}

void AIClientApp::AIGenerateOrders()
{
    Universe& universe = ClientApp::GetUniverse();
    int empire_id = ClientApp::EmpireID();

    Fleet* fleet;

    // 1) Split stationary multi-ship fleets into multiple single-ship fleets
    Universe::ObjectVec stat_fleets = universe.FindObjects(StationaryFleetVisitor(empire_id));    
    Universe::ObjectVec::iterator fleet_it;

    for (fleet_it = stat_fleets.begin(); fleet_it != stat_fleets.end(); ++fleet_it) {
        if(!(fleet = dynamic_cast<Fleet*>(*fleet_it))) continue;

        // split fleet into single-ship fleets if it presently has more than one ship
        if (fleet->NumShips() > 1) {
            SplitFleet(fleet);
        }
    }

    // 2) Give stationary fleets orders
    stat_fleets = universe.FindObjects(StationaryFleetVisitor(empire_id));  // redo to get any newly created fleets from above

    for (fleet_it = stat_fleets.begin(); fleet_it != stat_fleets.end(); ++fleet_it) {
        if(!(fleet = dynamic_cast<Fleet*>(*fleet_it))) continue;

        if (fleet->NumShips() < 1) continue;    // shouldn't be possible... but to be safe...

        // get ship, design
        Ship* ship = universe.Object<Ship>(*(fleet->begin()));  if (!ship) continue;
        const ShipDesign *design = ship->Design();
        
        // give orders according to type of ship in fleet
        if (design->name == "Scout") {
            Explore(fleet);

        } else if (design->name == "Colony Ship") {
            ColonizeSomewhere(fleet);

        }
    }
}

void AIClientApp::Explore(Fleet* fleet) {
    if (!fleet) return;

    Logger().debugStream() << "telling fleet to explore";

    Universe& universe = ClientApp::GetUniverse();
    int empire_id = ClientApp::EmpireID();
     
    // ensure this player owns this fleet
    const std::set<int>& owners = fleet->Owners();
    if (owners.size() != 1 || *(owners.begin()) != empire_id) return; // don't own fleet

    const Empire* empire = ClientApp::Empires().Lookup(empire_id);
    if (!empire) throw std::runtime_error("Couldn't get pointer to empire when telling fleet to Explore");


    int start_id = fleet->SystemID();   // system where fleet is presently

    Logger().debugStream() << "telling fleet to explore2";
    
    // attempt to find an unexplored system that can be explored (fleet can get to)
    int explorable_system = UniverseObject::INVALID_OBJECT_ID;
    std::vector<System*> systems = universe.FindObjects<System>();
    for (std::vector<System*>::const_iterator system_it = systems.begin(); system_it != systems.end(); ++system_it) {
        System* system = *system_it;
        int dest_id = system->ID();   // system to go to
        if (empire->HasExploredSystem(dest_id)) continue;   // already explored system
        if (m_fleet_exploration_targets_map.find(dest_id) != m_fleet_exploration_targets_map.end()) continue;   // another fleet has been dispatched
        
        Logger().debugStream() << "telling fleet to explore3";
        
        // get path to destination.  don't care that it's short, but just that it exists
        std::list<System*> route = universe.ShortestPath(start_id, dest_id, empire_id).first;
        
        if (route.empty()) continue; // can't get to system (with present starlanes knowledge)

        Logger().debugStream() << "telling fleet to explore4";
        
        // order ship to go ot system
        GetApp()->Orders().IssueOrder(new FleetMoveOrder(empire_id, fleet->ID(), start_id, dest_id));

        Logger().debugStream() << "telling fleet to explore5";
        
        // mark system as targeted for exploration, so another ship isn't sent to it redundantly
        m_fleet_exploration_targets_map.insert(std::pair<int, int>(dest_id, fleet->ID()));

        return; // don't need to keep looping at this point
    }
}

void AIClientApp::ColonizeSomewhere(Fleet* fleet) {

}

void AIClientApp::SplitFleet(Fleet* fleet)
{
    if (!fleet) return; // no fleet to process...
    if (fleet->NumShips() < 2) return;    // can't split fleet with one (or no?) ships
 
    Universe& universe = ClientApp::GetUniverse();
    int empire_id = ClientApp::EmpireID();
     
    // ensure this player owns this fleet
    const std::set<int>& owners = fleet->Owners();

    if (owners.size() != 1 || *(owners.begin()) != empire_id) return; // don't own fleet

    // starting with second ship, pick ships to transfer to new fleets
    std::set<int> ship_ids_to_remove;
    for (Fleet::iterator ship_it = ++(fleet->begin()); ship_it != fleet->end(); ++ship_it) {

        Ship *ship = universe.Object<Ship>(*ship_it);
        const std::set<int>& ship_owners = ship->Owners();

        if (ship_owners.size() != 1 || *(ship_owners.begin()) != empire_id) continue; // don't own ship
    
        ship_ids_to_remove.insert(*ship_it);
    }

    if (ship_ids_to_remove.empty()) return;  // nothing more to do

    // info from source fleet that may be copied to new fleets
    System* system = fleet->GetSystem();
    double fleet_x = fleet->X();
    double fleet_y = fleet->Y();

    // order transfers of ships from old fleet to new fleets 
    for (std::set<int>::iterator ship_it = ship_ids_to_remove.begin(); ship_it != ship_ids_to_remove.end(); ++ship_it) {
        std::vector<int> ship_ids;
        ship_ids.push_back(*ship_it);

        int new_fleet_id = ClientApp::GetNewObjectID();
        if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID) 
            throw std::runtime_error("Couldn't get new object ID when transferring ship to new fleet");

        std::string fleet_name = UserString("FW_NEW_FLEET_NAME") + boost::lexical_cast<std::string>(new_fleet_id);

        Fleet* new_fleet = 0;
        if (system) {
            GetApp()->Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system->ID(), ship_ids));

        } else {
            GetApp()->Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, fleet_x, fleet_y, ship_ids));
        }
    }
}