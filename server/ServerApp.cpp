#include "ServerApp.h"

#include "fastevents.h"
#include "../network/Message.h"
#include "XMLDoc.h"

#include "../network/XDiff.hpp"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>

// for dummy video driver setenv-hack
#include "SDL_getenv.h"

#include <ctime>

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

const unsigned int HOST_PLAYER_ID = 0;
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
    m_state(SERVER_IDLE)
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
}

ServerApp::~ServerApp()
{
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
    int ai_client_base_number = time(0) % 999; // get a random number from which to start numbering the AI clients
    const std::string AI_CLIENT_EXE = "freeorionca.exe";
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
            m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_GAME_SETUP << " (SERVER_GAME_SETUP).";
            if (m_expected_players == static_cast<int>(m_network_core.Players().size())) {
                GameInit();
                m_state = SERVER_WAITING;
                m_log_category.debugStream() << "ServerApp::HandleMessage : Server now in mode " << SERVER_WAITING << " (SERVER_WAITING).";
            }
        } else {
            const char* socket_hostname = SDLNet_ResolveIP(const_cast<IPaddress*>(&m_network_core.Players().find(msg.Sender())->second.address));
            m_log_category.errorStream() << "ServerApp::HandleMessage : A human player attempted to host "
                "a new MP game with the wrong player name, or while one was not being setup.  Terminating connection to " << 
                (socket_hostname ? socket_hostname : "[unknown host]") << " (player #" << msg.Sender() << ")";
            std::cout << "**** TEXT= " << host_player_name << " **** ";
            std::cout << ((it == m_network_core.Players().end()) ? "NOT" : "FOUND") << " in m_network_core ";
            if (it != m_network_core.Players().end())
                std::cout << " name in m_network_core: " << it->second.name << "\n";
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
            int player_id = HOST_PLAYER_ID;
            if (doc.root_node.NumChildren() == 1) { // start an MP lobby situation so that game settings can be established
                m_state = SERVER_MP_LOBBY;
                g_lobby_data = LobbyModeData();
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_MP_LOBBY << " (SERVER_MP_LOBBY).";
                if (m_network_core.EstablishPlayer(connection.socket, player_id, host_player_info)) {
                    m_network_core.SendMessage(HostAckMessage(player_id));
                    m_network_core.SendMessage(JoinAckMessage(player_id));
                }
            } else { // immediately start a new game with the given parameters
                m_expected_players = boost::lexical_cast<int>(doc.root_node.Child("num_players").Attribute("value"));
                m_galaxy_size = boost::lexical_cast<int>(doc.root_node.Child("universe_params").Attribute("size"));
                m_galaxy_shape = Universe::Shape(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Attribute("shape")));
                if (m_galaxy_shape == Universe::FROM_FILE)
                    m_galaxy_file = doc.root_node.Child("universe_params").Child("file").Text();
                CreateAIClients(doc.root_node);
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
        int player_id = std::max(HOST_PLAYER_ID + 1, m_network_core.Players().size());
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
                GameInit();
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
    m_state = SERVER_DISCONNECT;
    m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Server now in mode " << SERVER_DISCONNECT << " (SERVER_DISCONNECT).";
// TODO: try to reconnect
// TODO: send request to host player to ask what to do
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
    // Dirty hack to active the dummy video handler of SDL; if the user has already set SDL_VIDEODRIVER, we trust him
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

void ServerApp::GameInit()
{

    m_universe.CreateUniverse(m_galaxy_shape, m_galaxy_size, m_network_core.Players().size() - m_ai_clients.size(), m_ai_clients.size());
    m_log_category.debugStream() << "ServerApp::GameInit : Created universe " << 
        (m_galaxy_shape == Universe::FROM_FILE ? ("from file " + m_galaxy_file) : "") << " (SERVER_GAME_SETUP).";


    // the universe creation caused the creation of empires.  But now we
    // need to assign the empires to players.
    int i = 0;
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it, ++i) {
        GG::XMLDoc doc;
        doc.root_node.AppendChild(m_universe.XMLEncode());
        doc.root_node.AppendChild(m_empires.CreateClientEmpireUpdate(i));
        m_network_core.SendMessage(GameStartMessage(it->first, doc));
    }
}

GG::XMLDoc ServerApp::CreateTurnUpdate(int empire_id)
{
    using GG::XMLElement;

    GG::XMLDoc this_turn;

    // generate new data for this turn
    XMLElement universe_data = m_universe.XMLEncode(empire_id);
    XMLElement empire_data = m_empires.CreateClientEmpireUpdate(empire_id);

    // build the new turn doc
    this_turn.root_node.AppendChild(universe_data);
    this_turn.root_node.AppendChild(empire_data);

    std::map<int, GG::XMLDoc>::iterator itr =  m_last_turn_update_msg.find(empire_id);
    if (itr == m_last_turn_update_msg.end()) {
        // This empire has not been added to the map yet. Full data will be sent
        // to the client and this turn will be the first entry for this empire.
        m_last_turn_update_msg[empire_id] = this_turn;
        return this_turn;
    }

    // valid entry in the map for this empire
    GG::XMLDoc last_turn = itr->second;
   
    GG::XMLDoc update_patch;

    // diff this turn with previous turn
    XDiff(last_turn, this_turn, update_patch);

    // clear the previous entry and store this turn into the map
    m_last_turn_update_msg.erase(itr);
    m_last_turn_update_msg[empire_id] = this_turn;

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
