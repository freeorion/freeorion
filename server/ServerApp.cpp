#include "ServerApp.h"

#include "fastevents.h"
#include "../network/Message.h"
#include "XMLDoc.h"

#include "../network/XDiff.hpp"
#include "../util/OrderSet.h"
#include "../util/GZStream.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Predicates.h"
#include "../Empire/TechManager.h"

#include "../combat/CombatSystem.h"

#include <GGFont.h>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>

// for dummy video driver setenv-hack
#include "SDL_getenv.h"

#include <ctime>

// define this as nonzero to save games in gzip-compressed form; define this as zero when this is inconvenient, such as when testing and debugging
#define GZIP_SAVE_FILES 1


struct AISetupData
{
    // TODO
};

namespace {
    struct LobbyModeData
    {
        LobbyModeData () :
            galaxy_size(150),
            galaxy_type(Universe::SPIRAL_2)
        {}

        int                      galaxy_size; // number of stars
        Universe::Shape          galaxy_type;
        std::string              galaxy_image_filename;

        std::vector<AISetupData> AIs;

    } g_lobby_data;
    const std::string AI_CLIENT_EXE = "freeorionca.exe";
    const std::string LAST_TURN_UPDATE_SAVE_ELEM_PREFIX = "empire_";
    GG::XMLDoc g_load_doc;
    const bool ALL_OBJECTS_VISIBLE = false; // set this to true to turn off visibility for debugging purposes
}

////////////////////////////////////////////////
// PlayerInfo
////////////////////////////////////////////////
PlayerInfo::PlayerInfo() : 
    socket(-1)
{
}

PlayerInfo::PlayerInfo(int sock, const IPaddress& addr, const std::string& player_name/* = ""*/, bool host_/* = false*/) : 
    socket(sock),
    address(addr),
    name(player_name),
    host(host_)
{
}



////////////////////////////////////////////////
// ServerApp
////////////////////////////////////////////////
// static member(s)
ServerApp*  ServerApp::s_app = 0;

ServerApp::ServerApp(int argc, char* argv[]) : 
    m_current_combat(0), 
    m_log_category(log4cpp::Category::getRoot()),
    m_state(SERVER_IDLE),
    m_current_turn(1)
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of singleton class ServerApp");
   
    s_app = this;
   
    const std::string SERVER_LOG_FILENAME("freeoriond.log");
   
    // a platform-independent way to erase the old log
    std::ofstream temp(SERVER_LOG_FILENAME.c_str());
    temp.close();
   
    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", SERVER_LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p : %m%n");
    appender->setLayout(layout);
    m_log_category.setAdditivity(false);  // make appender the only appender used...
    m_log_category.setAppender(appender);
    m_log_category.setAdditivity(true);   // ...but allow the addition of others later
    m_log_category.setPriority(log4cpp::Priority::DEBUG);
    m_log_category.debug("freeoriond logger initialized.");
    m_log_category.errorStream() << "ServerApp::ServerApp : Server now in mode " << SERVER_IDLE << " (SERVER_IDLE).";

    // initialize tech manager
    TechManager::instance().LoadTechTree( "" );
}

ServerApp::~ServerApp()
{
    // shutdown tech tree
    TechManager::instance().ClearAll();

    m_log_category.debug("Shutting down freeoriond logger...");
	log4cpp::Category::shutdown();
}

GG::XMLDoc ServerApp::ServerStatusDoc() const
{
    GG::XMLDoc retval;
    GG::XMLElement elem("server_state");
    elem.SetAttribute("value", boost::lexical_cast<std::string>(m_state));
    retval.root_node.AppendChild(elem);
    return retval;
}

void ServerApp::operator()()
{
    Run();
}

void ServerApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    SDLQuit();
    exit(code);
}

log4cpp::Category& ServerApp::Logger()
{
    return m_log_category;
}

void ServerApp::CreateAIClients(const std::vector<AISetupData>& AIs)
{
    m_expected_ai_players.clear();
    for (std::set<int>::iterator it = m_ai_IDs.begin(); it != m_ai_IDs.end(); ++it) {
        m_network_core.DumpPlayer(*it);
    }
    m_ai_clients.clear();
    m_ai_IDs.clear();

    int ai_client_base_number = time(0) % 999; // get a random number from which to start numbering the AI clients
    int i = 0;
    for (std::vector<AISetupData>::const_iterator it = AIs.begin(); it != AIs.end(); ++it, ++i) {
// TODO: add other command line args to AI client invocation as needed
        std::string player_name = "AI_" + boost::lexical_cast<std::string>(ai_client_base_number + i); // AI player's "name"
        m_expected_ai_players.insert(player_name);
        std::vector<std::string> args;
        args.push_back(AI_CLIENT_EXE);
        args.push_back(player_name);
        m_ai_clients.push_back(Process(AI_CLIENT_EXE, args));
    }
}

void ServerApp::CreateAIClients(const GG::XMLElement& elem)
{
    std::vector<AISetupData> AIs;
    for (int i = 0; i < elem.NumChildren(); ++i) {
        if (elem.Child(i).Tag() == "AI_client") {
            AIs.push_back(AISetupData());
        }
    }
    CreateAIClients(AIs);
}

void ServerApp::HandleMessage(const Message& msg)
{
    switch (msg.Type()) {
    case Message::HOST_GAME: { // this should only be received at the end of MP setup
        std::string host_player_name = msg.GetText();
        std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().find(msg.Sender());
        bool spoofed_host = (m_state == SERVER_MP_LOBBY && it != m_network_core.Players().end() && it->second.name == host_player_name);
        if (!spoofed_host) {
            m_galaxy_size = g_lobby_data.galaxy_size;
            m_galaxy_shape = g_lobby_data.galaxy_type;
            m_galaxy_file = g_lobby_data.galaxy_image_filename;
            m_expected_players = m_network_core.Players().size() + g_lobby_data.AIs.size();
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != msg.Sender())
                    m_network_core.SendMessage(Message(Message::GAME_START, -1, it->first, Message::CLIENT_LOBBY_MODULE, ""));
            }
            m_state = SERVER_GAME_SETUP;
            CreateAIClients(g_lobby_data.AIs);
            g_load_doc.root_node = GG::XMLElement();
            m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
            if (m_expected_players == static_cast<int>(m_network_core.Players().size())) {
                if (g_load_doc.root_node.NumChildren())
                    LoadGameInit();
                else
                    NewGameInit();
                m_state = SERVER_WAITING;
                m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
            }
        } else {
            const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&m_network_core.Players().find(msg.Sender())->second.address));
            m_log_category.errorStream() << "ServerApp::HandleMessage : A human player attempted to host "
                "a new MP game with the wrong player name, or while one was not being setup.  Terminating connection to " << 
                (socket_hostname ? socket_hostname : "[unknown host]") << " (player #" << msg.Sender() << ")";
            m_network_core.DumpPlayer(msg.Sender());
        }
        break;
    }

    case Message::LOBBY_UPDATE: {
        std::stringstream stream(msg.GetText());
        GG::XMLDoc doc;
        doc.ReadDoc(stream);
        if (doc.root_node.ContainsChild("receiver")) { // chat message
            int receiver = boost::lexical_cast<int>(doc.root_node.Child("receiver").Attribute("value"));
            const std::string& text = doc.root_node.Child("text").Text();
            if (receiver == -1) { // the receiver is everyone (except the sender)
                for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                    if (it->first != msg.Sender())
                        m_network_core.SendMessage(ServerLobbyChatMessage(msg.Sender(), it->first, text));
                }
            } else {
                m_network_core.SendMessage(ServerLobbyChatMessage(msg.Sender(), receiver, text));
            }
        } else if (doc.root_node.ContainsChild("abort_game")) { // host is aborting the game (must be sent by the host to be valid)
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != msg.Sender()) {
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, doc));
                    m_network_core.DumpPlayer(it->first);
                }
            }
        } else if (doc.root_node.ContainsChild("exit_lobby")) { // player is exiting the lobby (must be a non-host to be valid)
            doc.root_node.Child("exit_lobby").SetAttribute("id", boost::lexical_cast<std::string>(msg.Sender()));
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != msg.Sender())
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, doc));
            }
            m_network_core.DumpPlayer(msg.Sender());
        } else { // normal lobby data update
            if (doc.root_node.ContainsChild("galaxy_size")) {
                g_lobby_data.galaxy_size = boost::lexical_cast<int>(doc.root_node.Child("galaxy_size").Attribute("value"));
            }
            if (doc.root_node.ContainsChild("galaxy_type")) {
                g_lobby_data.galaxy_type = Universe::Shape(boost::lexical_cast<int>(doc.root_node.Child("galaxy_type").Attribute("value")));
            }
            if (doc.root_node.ContainsChild("galaxy_image_filename")) {
                g_lobby_data.galaxy_image_filename = doc.root_node.Child("galaxy_image_filename").Text();
            }
            if (doc.root_node.ContainsChild("players")) {
                // TODO : handle player updates when there are options for players
            }
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != msg.Sender())
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, doc));
            }
        }
        break;
    }

    case Message::SAVE_GAME: {
        if (m_network_core.Players().find(msg.Sender())->second.host) {
            std::string save_filename = msg.GetText();
            GG::XMLDoc doc;

            // send out all save game data requests
            std::set<int> needed_reponses;
            m_players_responded.clear();
            m_player_save_game_data.clear();
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                m_network_core.SendMessage(ServerSaveGameMessage(it->first));
                needed_reponses.insert(it->first);
            }

            // wait for them all to come in
            SDL_Event ev;
            const unsigned int SYCHRONOUS_TIMEOUT = 15000; // give up after this many ms without any valid responses
            unsigned int start_time = SDL_GetTicks();
            while (1) {
                unsigned int starting_responses = m_players_responded.size();
                FE_PollEvent(&ev);
                if (ev.type == SDL_USEREVENT) {
                    int net2_type = NET2_GetEventType(&ev);
                    if (net2_type == NET2_ERROREVENT || 
                        net2_type == NET2_TCPACCEPTEVENT || 
                        net2_type == NET2_TCPRECEIVEEVENT || 
                        net2_type == NET2_TCPCLOSEEVENT || 
                        net2_type == NET2_UDPRECEIVEEVENT) {
                        m_network_core.HandleNetEvent(ev);
                    }
                }
                if (starting_responses < m_players_responded.size())
                    start_time = SDL_GetTicks(); // reset timeout whenever there's a valid response
                if (m_players_responded == needed_reponses || SYCHRONOUS_TIMEOUT < SDL_GetTicks() - start_time)
                    break;
            }
            if (m_players_responded == needed_reponses) {
                SaveGameVars(doc);
                for (std::map<int, GG::XMLElement>::iterator it = m_player_save_game_data.begin(); it != m_player_save_game_data.end(); ++it) {
                    GG::XMLElement player_element("Player");
                    player_element.AppendChild(GG::XMLElement("name", m_network_core.Players().find(it->first)->second.name));
                    player_element.AppendChild(m_empires.Lookup(it->first)->XMLEncode());
                    for (GG::XMLElement::const_child_iterator elem_it = it->second.child_begin(); elem_it != it->second.child_end(); ++elem_it) {
                        player_element.AppendChild(*elem_it);
                    }
                    doc.root_node.AppendChild(player_element);
                }
                doc.root_node.AppendChild(m_universe.XMLEncode(Universe::ALL_EMPIRES));
#if GZIP_SAVE_FILES
                GZStream::ogzstream ofs(save_filename.c_str());
                /* For now, we use the standard compression settings,
	                but later we could let the compression settings be
	                customizable in the save-dialog */
                // The default is: ofs.set_gzparams(6, Z_DEFAULT_STRATEGY);
                doc.WriteDoc(ofs, false);
                ofs.close();
#else
                std::ofstream ofs(save_filename.c_str());
                doc.WriteDoc(ofs);
                ofs.close();
#endif
                m_network_core.SendMessage(ServerSaveGameMessage(msg.Sender(), true));
            }
        } else {
            m_log_category.errorStream() << "Player #" << msg.Sender() << " attempted to initiate a game save, but is not the host player, or is "
                "not found in the player list.";
        }
        break;
    }

    case Message::LOAD_GAME: { // single-player loading (multiplayer loading is handled through the lobby interface)
        std::map<int, PlayerInfo>::const_iterator sender_it = m_network_core.Players().find(msg.Sender());
        if (sender_it != m_network_core.Players().end() && sender_it->second.host) {
            m_empires.RemoveAllEmpires();
            m_single_player_game = true;

            std::string load_filename = msg.GetText();
            GG::XMLDoc doc;
#if GZIP_SAVE_FILES
            GZStream::igzstream ifs(load_filename.c_str());
            doc.ReadDoc(ifs);
            ifs.close();
#else
            std::ifstream ifs(load_filename.c_str());
            doc.ReadDoc(ifs);
            ifs.close();
#endif

            m_universe.SetUniverse(doc.root_node.Child("Universe"));
            m_expected_players = 0;
            for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
                if (doc.root_node.Child(i).Tag() == "Player") {
                    m_empires.InsertEmpire(new Empire(doc.root_node.Child(i).Child("Empire")));
                    ++m_expected_players;
                }
            }
            LoadGameVars(doc);

            CreateAIClients(std::vector<AISetupData>(m_expected_players - 1));
            g_load_doc = doc;
            m_state = SERVER_GAME_SETUP;
        } else {
            m_log_category.errorStream() << "Player #" << msg.Sender() << " attempted to initiate a game save, but is not the host, or is "
                "not found in the player list.";
        }
        break;
    }

    case Message::TURN_ORDERS: {
        /* decode order set */
	    std::stringstream stream(msg.GetText());
	    GG::XMLDoc doc;
	    doc.ReadDoc(stream);

        if (doc.root_node.ContainsChild("save_game_data")) { // the Orders were in answer to a save game data request
            doc.root_node.RemoveChild("save_game_data");
            m_player_save_game_data[msg.Sender()].AppendChild(doc.root_node.Child("Orders"));
            if (doc.root_node.ContainsChild("UI"))
                m_player_save_game_data[msg.Sender()].AppendChild(doc.root_node.Child("UI"));
            m_players_responded.insert(msg.Sender());
        } else { // the Orders were sent from a Player who has finished her turn
#if 0
            /* debug information */
            std::string dbg_file( "turn_orders_server_" );
            dbg_file += boost::lexical_cast<std::string>(msg.Sender());
            dbg_file += ".txt";
            std::ofstream output( dbg_file.c_str() );
            doc.WriteDoc(output);
            output.close();
#endif

            OrderSet *p_order_set;
            p_order_set = new OrderSet( );
            GG::XMLObjectFactory<Order> order_factory;
            Order::InitOrderFactory(order_factory);
            const GG::XMLElement& root = doc.root_node.Child("Orders");

            for (int i = 0; i < root.NumChildren(); ++i) {
                Order *p_order = order_factory.GenerateObject(root.Child(i));

                if ( p_order ) {
                    p_order_set->AddOrder( p_order );
                } else {
                    // log error
                    m_log_category.errorStream() << "An Order has been received that has no factory - ignoring.";        
                }
            }

            /* if all orders are received already, do nothing as we are processing a turn */
            if ( AllOrdersReceived( ) )
                break;

            /* add orders to turn sequence */    
            SetEmpireTurnOrders( msg.Sender(), p_order_set );        
            
            /* look to see if all empires are done */
            if ( AllOrdersReceived( ) )
                ProcessTurns( );
        }
        break;
    }

    case Message::HUMAN_PLAYER_MSG: {
        std::string text = msg.GetText();

        // if there's a colon in the message, treat all tokens before the colon as player names.
        // if there are tokens before the colon, but at least one of them *is not* a valid player names, assume there has been a typo,
        // and don't send the message at all, since we can't decipher which parts are message and which parts are names
        unsigned int colon_position = text.find(':');
        // target_player_names.empty() implies that all players should be sent the message; otherwise, only the indicated players will receive the message
        std::set<std::string> target_player_names;
        if (colon_position != std::string::npos) {
            std::vector<std::string> tokens = GG::Tokenize(text.substr(0, colon_position));
            for (unsigned int i = 0; i < tokens.size(); ++i) {
                bool token_is_name = false;
                for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                    if (tokens[i] == it->second.name) {
                        token_is_name = true;
                        break;
                    }
                }
                if (token_is_name)
                    target_player_names.insert(tokens[i]);
                else
                    return;
            }
        }
        if (!target_player_names.empty()) {
            text = text.substr(colon_position + 1);
            if (text == "")
                return;
        }
        std::string final_text = GG::RgbaTag(Empires().Lookup(msg.Sender())->Color()) + m_network_core.Players().find(msg.Sender())->second.name + 
            (target_player_names.empty() ? ": " : " (whisper):") + text + "</rgba>\n";
        for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
            if (target_player_names.empty() || target_player_names.find(it->second.name) != target_player_names.end())
                m_network_core.SendMessage(ChatMessage(msg.Sender(), it->first, final_text));
        }
        break;
    }

    case Message::REQUEST_NEW_OBJECT_ID: {
        /* get get ID and send back to client, it's waiting for this */
        m_network_core.SendMessage(DispatchObjectIDMessage(msg.Sender(), GetUniverse().GenerateObjectID( ) ) );
        break;
	}

    case Message::END_GAME: {
        std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().find(msg.Sender());
        if (it != m_network_core.Players().end() && it->second.host) {
            for (std::map<int, PlayerInfo>::const_iterator it2 = m_network_core.Players().begin(); it2 != m_network_core.Players().end(); ++it2) {
                if (it->first != it2->first)
                    m_network_core.SendMessage(EndGameMessage(-1, it2->first));
            }
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_network_core.SendMessage(Message(Message::SERVER_STATUS, -1, msg.Sender(), Message::CORE, ServerStatusDoc()));
            m_network_core.DumpAllConnections();
            Exit(0);
        }
        break;
    }

    default: {
        m_log_category.errorStream() << "ServerApp::HandleMessage : Received an unknown message type \"" << msg.Type() << "\".";
        break;
    }
    }
}

void ServerApp::HandleNonPlayerMessage(const Message& msg, const PlayerInfo& connection)
{
    switch (msg.Type()) {
    case Message::HOST_GAME: {
        if (m_network_core.Players().empty() && m_expected_ai_players.empty()) {
            std::stringstream stream(msg.GetText());
            GG::XMLDoc doc;
            doc.ReadDoc(stream);
            std::string host_player_name = doc.root_node.Child("host_player_name").Text();

            PlayerInfo host_player_info(connection.socket, connection.address, host_player_name, true);
            int player_id = NetworkCore::HOST_PLAYER_ID;
            if (doc.root_node.NumChildren() == 1) { // start an MP lobby situation so that game settings can be established
                m_single_player_game = false;
                m_state = SERVER_MP_LOBBY;
                g_lobby_data = LobbyModeData();
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_MP_LOBBY << " (SERVER_MP_LOBBY).";
                if (m_network_core.EstablishPlayer(connection.socket, player_id, host_player_info)) {
                    m_network_core.SendMessage(HostAckMessage(player_id));
                    m_network_core.SendMessage(JoinAckMessage(player_id));
                }
            } else { // immediately start a new game with the given parameters
                m_single_player_game = true;
                m_expected_players = boost::lexical_cast<int>(doc.root_node.Child("num_players").Attribute("value"));
                m_galaxy_size = boost::lexical_cast<int>(doc.root_node.Child("universe_params").Attribute("size"));
                m_galaxy_shape = Universe::Shape(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Attribute("shape")));
                if (m_galaxy_shape == Universe::FROM_FILE)
                    m_galaxy_file = doc.root_node.Child("universe_params").Child("file").Text();
                CreateAIClients(doc.root_node);
                g_load_doc.root_node = GG::XMLElement();
                m_state = SERVER_GAME_SETUP;
                if (m_network_core.EstablishPlayer(connection.socket, player_id, host_player_info)) {
                    m_network_core.SendMessage(HostAckMessage(player_id));
                    m_network_core.SendMessage(JoinAckMessage(player_id));
                }
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Universe size set to " << m_galaxy_size << " systems (SERVER_GAME_SETUP).";
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Universe shape set to " << m_galaxy_shape << " (SERVER_GAME_SETUP).";
            }
        } else {
            const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
            m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to host "
                "a new game but there was already one in progress or one being setup.  Terminating connection to " << 
                (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
            m_network_core.DumpConnection(connection.socket);
        }

        break;
    }
      
    case Message::JOIN_GAME: {
        std::string player_name = msg.GetText(); // the player name should be the entire text
        PlayerInfo player_info(connection.socket, connection.address, player_name, false);
        int player_id = std::max(NetworkCore::HOST_PLAYER_ID + 1, static_cast<int>(m_network_core.Players().size()));
        if (player_id) {
            player_id = m_network_core.Players().rbegin()->first + 1;
        }
        if (m_state == SERVER_MP_LOBBY) { // enter an MP lobby
            if (m_network_core.EstablishPlayer(connection.socket, player_id, player_info)) {
                m_network_core.SendMessage(JoinAckMessage(player_id));
                for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                    if (it->first != player_id)
                        m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, LobbyPlayerUpdateDoc()));
                }
                m_network_core.SendMessage(ServerLobbyUpdateMessage(player_id, LobbyUpdateDoc()));
            }
        } else { // immediately join a game that is about to start
            std::set<std::string>::iterator it = m_expected_ai_players.find(player_name);
            if (it != m_expected_ai_players.end()) { // incoming AI player connection
                // let the server network core know what socket this player is on
                if (m_network_core.EstablishPlayer(connection.socket, player_id, player_info)) {
                    m_network_core.SendMessage(JoinAckMessage(player_id));
                    m_expected_ai_players.erase(player_name); // only allow one connection per AI
                    m_ai_IDs.insert(player_id);
                }
            } else { // non-AI player connection
                if (static_cast<int>(m_expected_ai_players.size() + m_network_core.Players().size()) < m_expected_players) {
                    if (m_network_core.EstablishPlayer(connection.socket, player_id, player_info)) {
                        m_network_core.SendMessage(JoinAckMessage(player_id));
                    }
                } else {
                    const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
                    m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : A human player attempted to join "
                        "the game but there was not enough room.  Terminating connection to " << 
                        (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
                    m_network_core.DumpConnection(connection.socket);
                }
            }

            if (static_cast<int>(m_network_core.Players().size()) == m_expected_players) { // if we've gotten all the players joined up
                if (g_load_doc.root_node.NumChildren())
                    LoadGameInit();
                else
                    NewGameInit();
                m_state = SERVER_WAITING;
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
            }
        }
        break;
    }

    default: {
        const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&connection.address));
        m_log_category.errorStream() << "ServerApp::HandleNonPlayerMessage : Received an invalid message type \"" <<
            msg.Type() << "\" for a non-player Message.  Terminating connection to " << 
            (socket_hostname ? socket_hostname : "[unknown host]") << " on socket " << connection.socket;
        m_network_core.DumpConnection(connection.socket);
        break;
    }
    }
}

void ServerApp::PlayerDisconnected(int id)
{
    if (id == NetworkCore::HOST_PLAYER_ID) {
        // if the host dies, there's really nothing else we can do
        for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
            if (it->first != id)
                m_network_core.SendMessage(EndGameMessage(-1, it->first));
        }
        m_state = SERVER_DYING;
        m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        m_network_core.DumpAllConnections();
        Exit(1);
    } else {
        // possibly try to recover
        m_state = SERVER_DISCONNECT;
        m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Server now in mode " << SERVER_DISCONNECT << " (SERVER_DISCONNECT).";
// TODO: try to reconnect
// TODO: send request to host player to ask what to do
    }
}

ServerApp* ServerApp::GetApp()
{
    return s_app;
}

Universe& ServerApp::GetUniverse()
{
    return ServerApp::GetApp()->m_universe;
}

ServerEmpireManager& ServerApp::Empires()
{
    return ServerApp::GetApp()->m_empires;
}

CombatModule* ServerApp::CurrentCombat()
{
    return ServerApp::GetApp()->m_current_combat;
}

ServerNetworkCore& ServerApp::NetworkCore()
{
    return ServerApp::GetApp()->m_network_core;
}

void ServerApp::Run()
{
    try {
        SDLInit();
        Initialize();
        while (1)
            Poll();
    } catch (const std::invalid_argument& exception) {
        m_log_category.fatal("std::invalid_argument Exception caught in ServerApp::Run(): " + std::string(exception.what()));
        Exit(1);
    } catch (const std::runtime_error& exception) {
        m_log_category.fatal("std::runtime_error Exception caught in ServerApp::Run(): " + std::string(exception.what()));
        Exit(1);
    }
}

void ServerApp::SDLInit()
{
#ifndef FREEORION_WIN32
    // Dirty hack to active the dummy video handler of SDL; if the user has already set SDL_VIDEODRIVER, we'll trust him
    if (getenv("SDL_VIDEODRIVER") == NULL) {
        putenv("SDL_VIDEODRIVER=dummy");
    }
#endif

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

void ServerApp::Initialize()
{
    m_network_core.ListenToPorts();
}

void ServerApp::Poll()
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
        } else { // some other SDL event
            switch (event.type) {
			case SDL_QUIT:
                Exit(0);
                break;
			}
// TODO: handle other relevant SDL events here
        }
    }
}

void ServerApp::FinalCleanup()
{
	NetworkCore().DumpAllConnections();
	for (unsigned int i = 0; i < m_ai_clients.size(); ++i)
        m_ai_clients[i].Kill();
}

void ServerApp::SDLQuit()
{
    FinalCleanup();
    NET2_Quit();
    FE_Quit();
    SDLNet_Quit();
    SDL_Quit();
    Logger().debugStream() << "SDLQuit() complete.";
}

void ServerApp::NewGameInit()
{
    std::map<int, PlayerInfo>::const_iterator it;

    m_universe.CreateUniverse(m_galaxy_shape, m_galaxy_size, m_network_core.Players().size() - m_ai_clients.size(), m_ai_clients.size());
    m_log_category.debugStream() << "ServerApp::GameInit : Created universe " << 
        (m_galaxy_shape == Universe::FROM_FILE ? ("from file " + m_galaxy_file) : "") << " (SERVER_GAME_SETUP).";

    // add empires to turn sequence map
    // according to spec this should be done randomly
    // for now it's not
    for ( it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
        AddEmpireTurn( it->first );
    }

    // the universe creation caused the creation of empires.  But now we
    // need to assign the empires to players.
    for (it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
        GG::XMLDoc doc;
        if (m_single_player_game)
            doc.root_node.AppendChild("single_player_game");
        doc.root_node.AppendChild(m_universe.XMLEncode(ALL_OBJECTS_VISIBLE ? Universe::ALL_EMPIRES : it->first));
        doc.root_node.AppendChild(m_empires.CreateClientEmpireUpdate(it->first));

        // turn number is an attribute of the document
        doc.root_node.SetAttribute("turn_number", boost::lexical_cast<std::string>(m_current_turn));

        m_network_core.SendMessage(GameStartMessage(it->first, doc));
    }
}

void ServerApp::LoadGameInit()
{
    m_turn_sequence.clear();
    std::vector<GG::XMLDoc> player_docs;
    for (int i = 0; i < g_load_doc.root_node.NumChildren(); ++i) {
        if (g_load_doc.root_node.Child(i).Tag() == "Player") {
            GG::XMLDoc player_doc;
            player_doc.root_node.SetTag("GG::XMLDoc");
            player_doc.root_node.AppendChild(g_load_doc.root_node.Child(i).Child("Orders"));
            if (g_load_doc.root_node.Child(i).ContainsChild("UI"))
                player_doc.root_node.AppendChild(g_load_doc.root_node.Child(i).Child("UI"));
            player_docs.push_back(player_doc);
        }
    }
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
        GG::XMLDoc doc;
        if (m_single_player_game)
            doc.root_node.AppendChild("single_player_game");
        doc.root_node.AppendChild(m_universe.XMLEncode(ALL_OBJECTS_VISIBLE ? Universe::ALL_EMPIRES : it->first));
        doc.root_node.AppendChild(m_empires.CreateClientEmpireUpdate(it->first));
        doc.root_node.SetAttribute("turn_number", boost::lexical_cast<std::string>(m_current_turn));
        m_network_core.SendMessage(GameStartMessage(it->first, doc));

#if 0
        std::ofstream ofs(("LoadGameInit-doc-empire" + boost::lexical_cast<std::string>(it->first) + ".xml").c_str());
        doc.WriteDoc(ofs);
        ofs.close();
#endif

        // send saved pending orders to player
        m_network_core.SendMessage(ServerLoadGameMessage(it->first, player_docs[it->first]));

        m_turn_sequence[it->first] = 0;
    }
}

GG::XMLDoc ServerApp::CreateTurnUpdate(int empire_id)
{
    using GG::XMLElement;
    GG::XMLDoc this_turn;
    GG::XMLDoc update_patch;

    // generate new data for this turn
    // for the final game, we'd have visibility
    // but for now we are able to see the whole universe
    XMLElement universe_data = m_universe.XMLEncode(ALL_OBJECTS_VISIBLE ? Universe::ALL_EMPIRES : empire_id);
    XMLElement empire_data = m_empires.CreateClientEmpireUpdate(empire_id);

    // build the new turn doc
    this_turn.root_node.AppendChild(universe_data);
    this_turn.root_node.AppendChild(empire_data);

    std::map<int, GG::XMLDoc>::iterator itr =  m_last_turn_update_msg.find(empire_id);

    GG::XMLDoc last_turn = itr->second;
   
    // diff this turn with previous turn
    XDiff(last_turn, this_turn, update_patch);

    // turn number is an attribute of the document
    update_patch.root_node.SetAttribute("turn_number", boost::lexical_cast<std::string>(m_current_turn));

    // return the results of the diff
    return update_patch;
}

GG::XMLDoc ServerApp::LobbyUpdateDoc() const
{
    GG::XMLDoc retval;
    GG::XMLElement temp("galaxy_size");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(g_lobby_data.galaxy_size));
    retval.root_node.AppendChild(temp);

    temp = GG::XMLElement("galaxy_type");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(g_lobby_data.galaxy_type));
    retval.root_node.AppendChild(temp);

    retval.root_node.AppendChild(GG::XMLElement("galaxy_image_filename", g_lobby_data.galaxy_image_filename));

    temp = GG::XMLElement("players");
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
        GG::XMLElement temp2(it->second.name);
        temp2.SetAttribute("id", boost::lexical_cast<std::string>(it->first));
        temp.AppendChild(temp2);
    }
    retval.root_node.AppendChild(temp);

    return retval;
}

GG::XMLDoc ServerApp::LobbyPlayerUpdateDoc() const

{
    GG::XMLDoc retval;
    GG::XMLElement temp("players");
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
        GG::XMLElement temp2(it->second.name);
        temp2.SetAttribute("id", boost::lexical_cast<std::string>(it->first));
        temp.AppendChild(temp2);
    }
    retval.root_node.AppendChild(temp);
    return retval;
}

void ServerApp::SaveGameVars(GG::XMLDoc& doc) const
{
    doc.root_node.AppendChild(GG::XMLElement("turn_number", boost::lexical_cast<std::string>(m_current_turn)));

    GG::XMLElement temp("m_last_turn_update_msg");
    for (std::map<int, GG::XMLDoc>::const_iterator it = m_last_turn_update_msg.begin(); it != m_last_turn_update_msg.end(); ++it) {
	temp.AppendChild(GG::XMLElement(LAST_TURN_UPDATE_SAVE_ELEM_PREFIX + boost::lexical_cast<std::string>(it->first), it->second.root_node));
    }
    doc.root_node.AppendChild(temp);
}

void ServerApp::LoadGameVars(const GG::XMLDoc& doc)
{
    m_current_turn = boost::lexical_cast<int>(doc.root_node.Child("turn_number").Text());

    const GG::XMLElement& last_turn_update_elem = doc.root_node.Child("m_last_turn_update_msg");
    for (GG::XMLElement::const_child_iterator it = last_turn_update_elem.child_begin(); it != last_turn_update_elem.child_end(); ++it) {
	m_last_turn_update_msg[boost::lexical_cast<int>(it->Tag().substr(LAST_TURN_UPDATE_SAVE_ELEM_PREFIX.size()))].root_node = it->Child(0);
    }
}


void ServerApp::AddEmpireTurn( int empire_id )
{
    // add empire
    m_turn_sequence[ empire_id ] = NULL;
}


void ServerApp::RemoveEmpireTurn( int empire_id )
{
    // Loop through to find empire ID and remove
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        if ( it->first == empire_id )
        {
            m_turn_sequence.erase( it );
            break;
        }
    }
}

void ServerApp::SetEmpireTurnOrders( int empire_id , OrderSet *order_set )
{
    m_turn_sequence[ empire_id ] = order_set;
}


bool ServerApp::AllOrdersReceived( )
{
    // Loop through to find empire ID and check for valid orders pointer
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        if ( !it->second )
            return false; 
    } 
    return true;
}


void ServerApp::ProcessTurns( )
{
    Empire                    *pEmpire;
    OrderSet                  *pOrderSet;
    OrderSet::const_iterator  order_it;
    Fleet                     *the_fleet;
    Planet                    *the_planet;
    UniverseObject            *the_object;

    /// First process all orders, then process turns
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        /// broadcast UI message to all players
        for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it) 
        {
            m_network_core.SendMessage( TurnProgressMessage( player_it->first, Message::PROCESSING_ORDERS, it->first ) );
        }

        pEmpire = Empires().Lookup( it->first );
        pEmpire->ClearSitRep( );
        pOrderSet = it->second;
     
        /// execute order set
        for ( order_it = pOrderSet->begin(); order_it != pOrderSet->end(); ++order_it)
        {
            // Add exeption handling here 
            order_it->second->Execute( );               
        }
    }    

    // now that orders are executed, universe and empire data are the same as on the client, since 
    // clients execute orders locally
    // here we encode the states so that we can diff later
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        GG::XMLDoc game_state;

        GG::XMLElement universe_data = m_universe.XMLEncode(ALL_OBJECTS_VISIBLE ? Universe::ALL_EMPIRES : it->first);
        GG::XMLElement empire_data = m_empires.CreateClientEmpireUpdate( it->first );

        // build the new turn doc
        game_state.root_node.AppendChild(universe_data);
        game_state.root_node.AppendChild(empire_data);

        // erase previous saved data
        std::map<int, GG::XMLDoc>::iterator itr =  m_last_turn_update_msg.find( it->first );
        if (itr != m_last_turn_update_msg.end()) {
            m_last_turn_update_msg.erase(itr);
        }
        m_last_turn_update_msg[ it->first ] = game_state;
    }


    /// process movement phase
    for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage( player_it->first, Message::FLEET_MOVEMENT, -1));

    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it)
        it->second->MovementPhase();


    /// process production and growth phase
    for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage( player_it->first, Message::EMPIRE_PRODUCTION, -1));

    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it)
        it->second->PopGrowthProductionResearchPhase();

    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        ///< check now for completed research
        Empire* empire = Empires().Lookup(it->first);
        empire->CheckResearchProgress();
    }


    /// loop and free all orders
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }   
    
    // check for combats, and resolve them.
    for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it) 
       m_network_core.SendMessage( TurnProgressMessage( player_it->first, Message::COMBAT, -1) );

    std::vector<System*> sys_vec = GetUniverse().FindObjects<System>();
    for(std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it)
    {
      std::vector<CombatAssets> empire_combat_forces;
      System* system = *it;
      
      std::vector<Fleet*> flt_vec = system->FindObjects<Fleet>();
      for(std::vector<Fleet*>::iterator flt_it = flt_vec.begin();flt_it != flt_vec.end(); ++flt_it)
      {
	        Fleet* flt = *flt_it;
	        // a fleet should belong only to one empire!?
 	        if(1==flt->Owners().size())
          {
            std::vector<CombatAssets>::iterator ecf_it = std::find(empire_combat_forces.begin(),empire_combat_forces.end(),CombatAssetsOwner(Empires().Lookup(*flt->Owners().begin())));

            if(ecf_it==empire_combat_forces.end())
            {
              CombatAssets ca(Empires().Lookup(*flt->Owners().begin()));
              ca.fleets.push_back(flt);
              empire_combat_forces.push_back(ca);
            }
            else
              (*ecf_it).fleets.push_back(flt);
          }
      }
      std::vector<Planet*> plt_vec = system->FindObjects<Planet>();
      for(std::vector<Planet*>::iterator plt_it = plt_vec.begin();plt_it != plt_vec.end(); ++plt_it)
      {
	        Planet* plt = *plt_it;
	        // a planet should belong only to one empire!?
          if(1==plt->Owners().size())
          {           
            std::vector<CombatAssets>::iterator ecf_it = std::find(empire_combat_forces.begin(),empire_combat_forces.end(),CombatAssetsOwner(Empires().Lookup(*plt->Owners().begin())));

            if(ecf_it==empire_combat_forces.end())
            {
              CombatAssets ca(Empires().Lookup(*plt->Owners().begin()));
              ca.planets.push_back(plt);
              empire_combat_forces.push_back(ca);
            }
            else
              (*ecf_it).planets.push_back(plt);
          }
     }

      if(empire_combat_forces.size()>1)
      {
        CombatSystem combat_system;
        combat_system.ResolveCombat(system->ID(),empire_combat_forces);
      }
    }
    // TODO : check if all empires are still alive

    /// Increment turn
    ++m_current_turn;
   
    // broadcast UI message to all players
    for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it)
    {
        pEmpire = Empires().Lookup( player_it->first );

        GG::XMLDoc doc = CreateTurnUpdate( player_it->first );
        
        m_network_core.SendMessage( TurnUpdateMessage( player_it->first, doc ) );
    }
}


