#include "ServerApp.h"

#include "fastevents.h"
#include "../network/Message.h"
#include "XMLDoc.h"

#include "../network/XDiff.hpp"
#include "../util/OrderSet.h"
#include "../util/GZStream.h"
#include "../universe/Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Predicates.h"
#include "../Empire/TechManager.h"

#include "../combat/CombatSystem.h"

#include <GGFont.h>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#define TEST_VALUE_REF_VARIABLE 0
#if TEST_VALUE_REF_VARIABLE
#include <ValueRef.h>
#endif

#define TEST_CONDITIONS_CLASS 0
#if TEST_CONDITIONS_CLASS
#define TEST_CONDITIONS_CLASS_CONCISE_OUTPUT 1
#include <Condition.h>
namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("condition-test-set", "Selects the test of the Condition class to perform.", 0, Validator<int>());
        db.Add("condition-test-source", "Selects source object (id) for the Condition class tests.", 528, Validator<int>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}
#endif

// for dummy video driver setenv-hack
#include "SDL_getenv.h"

#include <ctime>

// The compression level of savegames. Range 0-9.
// define this as nonzero to save games in gzip-compressed form;
// define this as zero when this is inconvenient, such as when testing
// and debugging
#define GZIP_SAVE_FILES_COMPRESSION_LEVEL 0


namespace {
    const std::string SAVE_FILE_EXTENSION = ".mps";
    const std::string SAVE_DIR_NAME = "save/";

    struct MultiplayerLobbyData
    {
        MultiplayerLobbyData (bool build_save_game_list = true) :
            new_game(true),
            size(100),
            shape(Universe::SPIRAL_2),
            age(Universe::AGE_MATURE),
            starlane_freq(Universe::LANES_SEVERAL),
            planet_density(Universe::PD_AVERAGE),
            specials_freq(Universe::SPECIALS_UNCOMMON),
            save_file(-1)
        {
            if (build_save_game_list) {
                // build a list of save files
                namespace fs = boost::filesystem;
                fs::path save_dir = boost::filesystem::initial_path() / SAVE_DIR_NAME;
                fs::directory_iterator end_it;
                for (fs::directory_iterator it(save_dir); it != end_it; ++it) {
                    if (fs::exists(*it) && !fs::is_directory(*it) && it->leaf()[0] != '.') {
                        std::string filename = it->leaf();
                        // disallow filenames that begin with a dot, and filenames with spaces in them
                        if (filename.find('.') != 0 && filename.find(' ') == std::string::npos && 
                            filename.find(SAVE_FILE_EXTENSION) == filename.size() - SAVE_FILE_EXTENSION.size()) {
                            save_games.push_back(filename);
                        }
                    }
                }

                // build list of empire colors
                empire_colors = EmpireColors();
            }
        }

        void RebuildSaveGameEmpireData()
        {
            save_game_empire_data.clear();
            if (0 <= save_file && save_file < static_cast<int>(save_games.size())) {
                GG::XMLDoc doc;
                GZStream::igzstream ifs((SAVE_DIR_NAME + save_games[save_file]).c_str());
                doc.ReadDoc(ifs);
                ifs.close();

                for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
                    if (doc.root_node.Child(i).Tag() == "Player") {
                        const GG::XMLElement& elem = doc.root_node.Child(i).Child("Empire");
                        SaveGameEmpireData data;
                        data.id = boost::lexical_cast<int>(elem.Child("m_id").Text());
                        data.name = elem.Child("m_name").Text();
                        data.player_name = elem.Child("m_player_name").Text();
                        data.color = GG::Clr(elem.Child("m_color").Child("GG::Clr"));
                        save_game_empire_data.push_back(data);
                    }
                }
            }
        }

        bool                            new_game;
        int                             size;
        Universe::Shape                 shape;
        Universe::Age                   age;
        Universe::StarlaneFrequency     starlane_freq;
        Universe::PlanetDensity         planet_density;
        Universe::SpecialsFrequency     specials_freq;
        int                             save_file;
        std::vector<PlayerSetupData>    players;
        std::vector<PlayerSetupData>    AIs;

        std::vector<std::string>        save_games;
        std::vector<GG::Clr>            empire_colors;
        std::vector<SaveGameEmpireData> save_game_empire_data;

    } g_lobby_data(false);

#ifdef FREEORION_WIN32
    const std::string AI_CLIENT_EXE = "freeorionca.exe";
#else
    const std::string AI_CLIENT_EXE = "freeorionca";
#endif    
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

void ServerApp::CreateAIClients(const std::vector<PlayerSetupData>& AIs)
{
    m_expected_ai_players.clear();
    for (std::set<int>::iterator it = m_ai_IDs.begin(); it != m_ai_IDs.end(); ++it) {
        m_network_core.DumpPlayer(*it);
    }
    m_ai_clients.clear();
    m_ai_IDs.clear();

    int ai_client_base_number = time(0) % 999; // get a random number from which to start numbering the AI clients
    int i = 0;
    for (std::vector<PlayerSetupData>::const_iterator it = AIs.begin(); it != AIs.end(); ++it, ++i) {
        // TODO: add other command line args to AI client invocation as needed
        std::string player_name = "AI_" + boost::lexical_cast<std::string>(ai_client_base_number + i);
        m_expected_ai_players.insert(player_name);
        std::vector<std::string> args;
        args.push_back(AI_CLIENT_EXE);
        args.push_back(player_name);
        args.push_back("--data-dir"); args.push_back(GetOptionsDB().Get<std::string>("data-dir"));
        m_ai_clients.push_back(Process(AI_CLIENT_EXE, args));
    }
}

void ServerApp::CreateAIClients(const GG::XMLElement& elem)
{
    std::vector<PlayerSetupData> AIs;
    for (int i = 0; i < elem.NumChildren(); ++i) {
        if (elem.Child(i).Tag() == "AI_client") {
            AIs.push_back(PlayerSetupData());
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
            if (g_lobby_data.new_game) { // new game
                m_galaxy_size = g_lobby_data.size;
                m_galaxy_shape = g_lobby_data.shape;
                m_galaxy_age = g_lobby_data.age;
                m_starlane_freq = g_lobby_data.starlane_freq;
                m_planet_density = g_lobby_data.planet_density;
                m_specials_freq = g_lobby_data.specials_freq;
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
            } else { // load game
                std::map<int, PlayerInfo>::const_iterator sender_it = m_network_core.Players().find(msg.Sender());
                if (sender_it != m_network_core.Players().end() && sender_it->second.host) {
                    m_empires.RemoveAllEmpires();
                    m_single_player_game = false;

                    std::string load_filename = SAVE_DIR_NAME + g_lobby_data.save_games[g_lobby_data.save_file];
                    GG::XMLDoc doc;
                    GZStream::igzstream ifs(load_filename.c_str());
                    doc.ReadDoc(ifs);
                    ifs.close();

                    // set the player names of all empires to match the selected player name/empire name pairs in the lobby
                    // (AI players' names will replace the applicable old names when the AIs connect later.)
                    for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
                        if (doc.root_node.Child(i).Tag() == "Player") {
                            int player_empire_id = boost::lexical_cast<int>(doc.root_node.Child(i).Child("Empire").Child("m_id").Text());
                            for (unsigned int j = 0; j < g_lobby_data.players.size(); ++j) {
                                if (g_lobby_data.players[j].save_game_empire_id == player_empire_id) {
                                    std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin();
                                    std::advance(player_it, j);
                                    doc.root_node.Child(i).Child("name").SetText(player_it->second.name);
                                    doc.root_node.Child(i).Child("Empire").Child("m_player_name").SetText(player_it->second.name);
                                    break;
                                }
                            }
                        }
                    }

                    m_universe.SetUniverse(doc.root_node.Child("Universe"));
                    m_expected_players = 0;
                    for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
                        if (doc.root_node.Child(i).Tag() == "Player") {
                            m_empires.InsertEmpire(new Empire(doc.root_node.Child(i).Child("Empire")));
                            ++m_expected_players;
                        }
                    }
                    LoadGameVars(doc);

                    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                        if (it != sender_it)
                            m_network_core.SendMessage(Message(Message::GAME_START, -1, it->first, Message::CLIENT_LOBBY_MODULE, ""));
                    }

                    int AI_clients = m_expected_players - m_network_core.Players().size();
                    CreateAIClients(std::vector<PlayerSetupData>(AI_clients));
                    g_load_doc = doc;
                    m_state = SERVER_GAME_SETUP;

                    if (!AI_clients)
                        LoadGameInit();
                } else {
                    m_log_category.errorStream() << "Player #" << msg.Sender() << " attempted to initiate a game load, but is not the host, or is "
                        "not found in the player list.";
                }
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
            int receiver = boost::lexical_cast<int>(doc.root_node.Child("receiver").Text());
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
            doc.root_node.Child("exit_lobby").AppendChild(GG::XMLElement("id", boost::lexical_cast<std::string>(msg.Sender())));
            unsigned int i = 0;
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it, ++i) {
                if (it->first != msg.Sender()) {
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, doc));
                } else {
                    if (i < g_lobby_data.players.size())
                        g_lobby_data.players.erase(g_lobby_data.players.begin() + i); // remove the exiting player's PlayerSetupData struct
                }
            }
            m_network_core.DumpPlayer(msg.Sender());
        } else { // normal lobby data update
            if (doc.root_node.ContainsChild("new_game"))
                g_lobby_data.new_game = true;
            else if (doc.root_node.ContainsChild("load_game"))
                g_lobby_data.new_game = false;

            if (doc.root_node.ContainsChild("universe_params")) {
                g_lobby_data.size = boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("size").Text());
                g_lobby_data.shape = Universe::Shape(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("shape").Text()));
                g_lobby_data.age = Universe::Age(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("age").Text()));
                g_lobby_data.starlane_freq = Universe::StarlaneFrequency(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("starlane_freq").Text()));
                g_lobby_data.planet_density = Universe::PlanetDensity(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("planet_density").Text()));
                g_lobby_data.specials_freq = Universe::SpecialsFrequency(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("specials_freq").Text()));
            }

            bool new_save_file_selected = false;
            if (doc.root_node.ContainsChild("save_file")) {
                int old_save_file = g_lobby_data.save_file;
                g_lobby_data.save_file = boost::lexical_cast<int>(doc.root_node.Child("save_file").Text());
                if (g_lobby_data.save_file != old_save_file) {
                    g_lobby_data.RebuildSaveGameEmpireData();
                    new_save_file_selected = true;
                }
            }

            if (doc.root_node.ContainsChild("players")) {
                g_lobby_data.players.clear();
                for (GG::XMLElement::child_iterator it = doc.root_node.Child("players").child_begin(); 
                    it != doc.root_node.Child("players").child_end(); 
                    ++it) {
                    g_lobby_data.players.push_back(PlayerSetupData(*it));
                }
            }

            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first == msg.Sender()) {
                    if (new_save_file_selected)
                        m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, SaveGameUpdateDoc()));
                } else {
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, LobbyUpdateDoc()));
                }
            }
        }
        break;
    }

    case Message::SAVE_GAME: {
        if (m_network_core.Players().find(msg.Sender()) != m_network_core.Players().end()) {
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
                    Empire* player_empire = GetPlayerEmpire(it->first);
                    player_element.AppendChild(m_empires.Lookup(player_empire->EmpireID())->XMLEncode());
                    for (GG::XMLElement::const_child_iterator elem_it = it->second.child_begin(); elem_it != it->second.child_end(); ++elem_it) {
                        player_element.AppendChild(*elem_it);
                    }
                    doc.root_node.AppendChild(player_element);
                }
                doc.root_node.AppendChild(m_universe.XMLEncode(Universe::ALL_EMPIRES));

#if GZIP_SAVE_FILES_COMPRESSION_LEVEL
                GZStream::ogzstream ofs(save_filename.c_str());
                /* For now, we use the standard compression settings,
	                but later we could let the compression settings be
	                customizable in the save-dialog */
                // The default is: ofs.set_gzparams(6, Z_DEFAULT_STRATEGY);
                ofs.set_gzparams(GZIP_SAVE_FILES_COMPRESSION_LEVEL, Z_DEFAULT_STRATEGY);
#else
                std::ofstream ofs(save_filename.c_str());
#endif
                doc.WriteDoc(ofs, GZIP_SAVE_FILES_COMPRESSION_LEVEL ? false : true);
                ofs.close();
                m_network_core.SendMessage(ServerSaveGameMessage(msg.Sender(), true));
            }
        } else {
            m_log_category.errorStream() << "Player #" << msg.Sender() << " attempted to initiate a game save, but is not found in the player list.";
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
            GZStream::igzstream ifs(load_filename.c_str());
            doc.ReadDoc(ifs);
            ifs.close();

            m_universe.SetUniverse(doc.root_node.Child("Universe"));
            m_expected_players = 0;
            for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
                if (doc.root_node.Child(i).Tag() == "Player") {
                    m_empires.InsertEmpire(new Empire(doc.root_node.Child(i).Child("Empire")));
                    ++m_expected_players;
                }
            }
            LoadGameVars(doc);

#if TEST_VALUE_REF_VARIABLE
            std::ofstream ofs("ValueRefTest.txt");

            // first, create the XML representation of an arbitrary UniverseObject
            UniverseObject* object = m_universe.Object(559);
            GG::XMLDoc debug_doc;
            debug_doc.root_node.AppendChild(object->XMLEncode());
            debug_doc.WriteDoc(ofs);

            // now, read all the ValueRef-accessible items from this object
            const char* field_names[] = {
                "CurrentFarming",
                "CurrentIndustry",
                "MaxIndustry",
                "CurrentResearch",
                "MaxResearch",
                "CurrentTrade",
                "MaxTrade",
                "CurrentMining",
                "MaxMining",
                "CurrentConstruction",
                "MaxConstruction",
                "CurrentHealth",
                "MaxHealth",
                "CurrentPopulation",
                "MaxPopulation",
                "MoneyStockpile",
                "MineralStockpile",
                "FoodStockpile",
                "MoneyProduction",
                "FoodProduction",
                "MineralProduction",
                "IndustryProduction",
                "ScienceProduction",
                0
            };

            try {
                ofs << "\nOBJECT:\n" << std::endl;
                // first the source object version...
                const char** curr_name = field_names;
                while (*curr_name) {
                    ofs << *curr_name << "(source)= " << ValueRef::Variable<double>(true, *curr_name).Eval(object, 0) << std::endl;
                    curr_name++;
                }
                ofs << std::endl;

                ofs << "PlanetSize(source)= " << ValueRef::Variable<PlanetSize>(true, "PlanetSize").Eval(object, 0) << std::endl;
                ofs << "PlanetType(source)= " << ValueRef::Variable<PlanetType>(true, "PlanetType").Eval(object, 0) << std::endl;
                ofs << "PlanetEnvironment(source)= " << ValueRef::Variable<PlanetEnvironment>(true, "PlanetEnvironment").Eval(object, 0) << std::endl;
                ofs << "UniverseObjectType(source)= " << ValueRef::Variable<UniverseObjectType>(true, "ObjectType").Eval(object, 0) << std::endl;
                ofs << "StarType(source)= " << ValueRef::Variable<StarType>(true, "StarType").Eval(object, 0) << std::endl;
                ofs << "PrimaryFocus(source)= " << ValueRef::Variable<FocusType>(true, "PrimaryFocus").Eval(object, 0) << std::endl;
                ofs << "SecondaryFocus(source)= " << ValueRef::Variable<FocusType>(true, "SecondaryFocus").Eval(object, 0) << std::endl;

                ofs << "Owner(source)= " << ValueRef::Variable<int>(true, "Owner").Eval(object, 0) << std::endl;
                ofs << "ID(source)= " << ValueRef::Variable<int>(true, "ID").Eval(object, 0) << std::endl;
                ofs << "\n\n";

                // now the target object version
                curr_name = field_names;
                while (*curr_name) {
                    ofs << *curr_name << "(target)= " << ValueRef::Variable<double>(false, *curr_name).Eval(0, object) << std::endl;
                    curr_name++;
                }
                ofs << std::endl;

                ofs << "PlanetSize(target)= " << ValueRef::Variable<PlanetSize>(false, "PlanetSize").Eval(0, object) << std::endl;
                ofs << "PlanetType(target)= " << ValueRef::Variable<PlanetType>(false, "PlanetType").Eval(0, object) << std::endl;
                ofs << "PlanetEnvironment(target)= " << ValueRef::Variable<PlanetEnvironment>(false, "PlanetEnvironment").Eval(0, object) << std::endl;
                ofs << "UniverseObjectType(target)= " << ValueRef::Variable<UniverseObjectType>(false, "ObjectType").Eval(0, object) << std::endl;
                ofs << "StarType(target)= " << ValueRef::Variable<StarType>(false, "StarType").Eval(0, object) << std::endl;
                ofs << "PrimaryFocus(target)= " << ValueRef::Variable<FocusType>(false, "PrimaryFocus").Eval(0, object) << std::endl;
                ofs << "SecondaryFocus(target)= " << ValueRef::Variable<FocusType>(false, "SecondaryFocus").Eval(0, object) << std::endl;

                ofs << "Owner(target)= " << ValueRef::Variable<int>(false, "Owner").Eval(0, object) << std::endl;
                ofs << "ID(target)= " << ValueRef::Variable<int>(false, "ID").Eval(0, object) << std::endl;

                // NOW, re-run the same tests, using a reference to the object's system
                ofs << "\nOBJECT'S SYSTEM:\n" << std::endl;
                // first the source object version...
                curr_name = field_names;
                while (*curr_name) {
                    ofs << "System." << *curr_name << "(source)= " << ValueRef::Variable<double>(true, std::string("System.") + *curr_name).Eval(object, 0) << std::endl;
                    curr_name++;
                }
                ofs << std::endl;

                ofs << "System.PlanetSize(source)= " << ValueRef::Variable<PlanetSize>(true, "System.PlanetSize").Eval(object, 0) << std::endl;
                ofs << "System.PlanetType(source)= " << ValueRef::Variable<PlanetType>(true, "System.PlanetType").Eval(object, 0) << std::endl;
                ofs << "System.PlanetEnvironment(source)= " << ValueRef::Variable<PlanetEnvironment>(true, "System.PlanetEnvironment").Eval(object, 0) << std::endl;
                ofs << "System.UniverseObjectType(source)= " << ValueRef::Variable<UniverseObjectType>(true, "System.ObjectType").Eval(object, 0) << std::endl;
                ofs << "System.StarType(source)= " << ValueRef::Variable<StarType>(true, "System.StarType").Eval(object, 0) << std::endl;
                ofs << "System.PrimaryFocus(source)= " << ValueRef::Variable<FocusType>(true, "System.PrimaryFocus").Eval(object, 0) << std::endl;
                ofs << "System.SecondaryFocus(source)= " << ValueRef::Variable<FocusType>(true, "System.SecondaryFocus").Eval(object, 0) << std::endl;

                ofs << "System.Owner(source)= " << ValueRef::Variable<int>(true, "System.Owner").Eval(object, 0) << std::endl;
                ofs << "System.ID(source)= " << ValueRef::Variable<int>(true, "System.ID").Eval(object, 0) << std::endl;
                ofs << "\n\n";

                // (skip the target object version; the first round of tests determined that that works alright)
            } catch (...) {
                ofs.close();
                throw;
            }

            ofs.close();
#endif
#if TEST_CONDITIONS_CLASS
            for (int i = 0; i < 32; ++i) {
            std::ofstream ofs2(("ConditionTest" + boost::lexical_cast<std::string>(i) + ".txt").c_str());
            try {
                Condition::ConditionBase* condition = 0;
                switch (i) {
                    case 0:
                    default:
                        condition = new Condition::All();
                        break;
                    case 1:
                        condition = new Condition::EmpireAffiliation(new ValueRef::Constant<int>(0), AFFIL_SELF, true);
                        break;
                    case 2:
                        condition = new Condition::EmpireAffiliation(new ValueRef::Constant<int>(0), AFFIL_SELF, false);
                        break;
                    case 3:
                        condition = new Condition::Self();
                        break;
                    case 4:
                        condition = new Condition::Type(new ValueRef::Constant<UniverseObjectType>(OBJ_SHIP));
                        break;
                    case 5:
                        // TODO : put a valid building name in here.
                        // condition = new Condition::Building("building_name");
                        break;
                    case 6:
                        // TODO : put a valid special name in here.
                        // condition = new Condition::HasSpecial("special_name");
                        break;
                    case 7:
                        // TODO : put a valid condition name in here.
                        // condition = new Condition::Contains(/*condition*/);
                        break;
                    case 8:
                        condition = new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL)));
                        break;
                    case 9:
                        condition = new Condition::PlanetSize(std::vector<const ValueRef::ValueRefBase<PlanetSize>*>(1, new ValueRef::Constant<PlanetSize>(SZ_ASTEROIDS)));
                        break;
                    case 10:
                        condition = new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), true);
                        break;
                    case 11:
                        condition = new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), false);
                        break;
                    case 12:
                        condition = new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE)));
                        break;
                    case 13:
                        condition = new Condition::Chance(new ValueRef::Constant<double>(0.05));
                        break;
                    case 14:
                        // TODO : test this
                        condition = new Condition::MeterValue(METER_POPULATION,
                                                              new ValueRef::Constant<double>(10.0),
                                                              new ValueRef::Constant<double>(90.0),
                                                              true);
                        break;
                    case 15:
                        // TODO : test this
                        condition = new Condition::MeterValue(METER_POPULATION,
                                                              new ValueRef::Constant<double>(10.0),
                                                              new ValueRef::Constant<double>(90.0),
                                                              false);
                        break;
                    case 16:
                        // TODO : put a valid stockpile type in here.
                        /*condition = new Condition::StockpileValue(new ValueRef::Constant<StockpileType>(stockpile type value),
                                                              new ValueRef::Constant<double>(10.0),
                                                              new ValueRef::Constant<double>(90.0));*/
                        break;
                    case 17:
                        condition = new Condition::VisibleToEmpire(std::vector<const ValueRef::ValueRefBase<int>*>(1, new ValueRef::Constant<int>(0)));
                        break;
                    case 18:
                        // using condition from FocusType #2
                        condition = new Condition::WithinDistance(new ValueRef::Constant<double>(50.0),
                                                                  new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), false));
                        break;
                    case 19:
                        // using condition from FocusType #2
                        // TODO : test more extensively with Fleets at various positions (moving between Systems, etc.)
                        condition = new Condition::WithinStarlaneJumps(new ValueRef::Constant<int>(2),
                                                                       new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), false));
                        break;
                    case 20:
                        // TODO (EffectTarget is currently unimplemented)
                        //condition = new Condition::EffectTarget();
                        break;
                    /* the rest of these test And Or and Not, and their interactions; A = case 8, B = case 12, C = case 10 */
                    case 21: { // A and B
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        operands.push_back(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE))));
                        condition = new Condition::And(operands);
                        break;
                    }
                    case 22: { // A or B
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        operands.push_back(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE))));
                        condition = new Condition::Or(operands);
                        break;
                    }
                    case 23: { // not A
                        condition = new Condition::Not(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE))));
                        break;
                    }
                    case 24: { // A and (B or C)
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        std::vector<const Condition::ConditionBase*> or_operands;
                        or_operands.push_back(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE))));
                        or_operands.push_back(new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), true));
                        operands.push_back(new Condition::Or(or_operands));
                        condition = new Condition::And(operands);
                        break;
                    }
                    case 25: { // (A and B) or C
                        std::vector<const Condition::ConditionBase*> operands;
                        std::vector<const Condition::ConditionBase*> and_operands;
                        and_operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        and_operands.push_back(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE))));
                        operands.push_back(new Condition::And(and_operands));
                        operands.push_back(new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), true));
                        condition = new Condition::Or(operands);
                        break;
                    }
                    case 26: { // A or (B and C)
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        std::vector<const Condition::ConditionBase*> and_operands;
                        and_operands.push_back(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE))));
                        and_operands.push_back(new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), true));
                        operands.push_back(new Condition::And(and_operands));
                        condition = new Condition::Or(operands);
                        break;
                    }
                    case 27: { // (A or B) and C
                        std::vector<const Condition::ConditionBase*> operands;
                        std::vector<const Condition::ConditionBase*> or_operands;
                        or_operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        or_operands.push_back(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE))));
                        operands.push_back(new Condition::Or(or_operands));
                        operands.push_back(new Condition::FocusType(std::vector<const ValueRef::ValueRefBase<FocusType>*>(1, new ValueRef::Constant<FocusType>(FOCUS_INDUSTRY)), true));
                        condition = new Condition::And(operands);
                        break;
                    }
                    case 28: { // A or not(B)
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        operands.push_back(new Condition::Not(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE)))));
                        condition = new Condition::Or(operands);
                        break;
                    }
                    case 29: { // A and not(B)
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        operands.push_back(new Condition::Not(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE)))));
                        condition = new Condition::And(operands);
                        break;
                    }
                    case 30: { // not(B) or A
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::Not(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE)))));
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        condition = new Condition::Or(operands);
                        break;
                    }
                    case 31: { // not(B) and A
                        std::vector<const Condition::ConditionBase*> operands;
                        operands.push_back(new Condition::Not(new Condition::StarType(std::vector<const ValueRef::ValueRefBase<StarType>*>(1, new ValueRef::Constant<StarType>(STAR_WHITE)))));
                        operands.push_back(new Condition::PlanetEnvironment(std::vector<const ValueRef::ValueRefBase<PlanetEnvironment>*>(1, new ValueRef::Constant<PlanetEnvironment>(PE_OPTIMAL))));
                        condition = new Condition::And(operands);
                        break;
                    }
                }
                // get the list of all UniverseObjects that satisfy m_condition
                Condition::ConditionBase::ObjectSet condition_targets;
                Condition::ConditionBase::ObjectSet condition_non_targets;
                for (Universe::const_iterator it = m_universe.begin(); it != m_universe.end(); ++it) {
                    condition_non_targets.insert(it->second);
                }
#if !TEST_CONDITIONS_CLASS_CONCISE_OUTPUT
                ofs2 << "SOURCE:\n" << std::endl;
#endif
                UniverseObject* source = m_universe.Object(GetOptionsDB().Get<int>("condition-test-source"));
#if !TEST_CONDITIONS_CLASS_CONCISE_OUTPUT
                GG::XMLDoc debug_doc;
                debug_doc.root_node.AppendChild(source->XMLEncode());
                debug_doc.WriteDoc(ofs2);
#endif
                if (condition) {
                    condition->Eval(source, condition_targets, condition_non_targets);
#if !TEST_CONDITIONS_CLASS_CONCISE_OUTPUT
                    ofs2 << "\n\nTARGETS:\n" << std::endl;
#endif
                    for (Condition::ConditionBase::ObjectSet::const_iterator it = condition_targets.begin(); it != condition_targets.end(); ++it) {
                        ofs2 << "  " << (*it)->ID() << " \"" << (*it)->Name() << "\"" << std::endl;
                    }
                } else {
                    ofs2 << "Test #" << i << " not implemented yet.";
                }
#if !TEST_CONDITIONS_CLASS_CONCISE_OUTPUT
                ofs2 << "\n" << std::endl;
#endif
            } catch (...) {
                ofs2.close();
            }
            ofs2.close();
            }
#endif

            CreateAIClients(std::vector<PlayerSetupData>(m_expected_players - 1));
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

            m_log_category.debugStream() << "ServerApp::HandleMessage : Received orders from player " << msg.Sender();

            /* if all orders are received already, do nothing as we are processing a turn */
            if ( AllOrdersReceived( ) )
                break;

            /* add orders to turn sequence */    
            SetEmpireTurnOrders( GetPlayerEmpire( msg.Sender() )->EmpireID(), p_order_set );
            
            /* look to see if all empires are done */
            if ( AllOrdersReceived( ) ) {
                m_log_category.debugStream() << "ServerApp::HandleMessage : All orders received; processing turn...";
                ProcessTurns( );
            }
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
        Empire* sender_empire = GetPlayerEmpire(msg.Sender());
        std::string final_text = GG::RgbaTag(Empires().Lookup(sender_empire->EmpireID())->Color()) + m_network_core.Players().find(msg.Sender())->second.name + 
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
                g_lobby_data = MultiplayerLobbyData();
                m_log_category.debugStream() << "ServerApp::HandleNonPlayerMessage : Server now in mode " << SERVER_MP_LOBBY << " (SERVER_MP_LOBBY).";
                if (m_network_core.EstablishPlayer(connection.socket, player_id, host_player_info)) {
                    m_network_core.SendMessage(HostAckMessage(player_id));
                    m_network_core.SendMessage(JoinAckMessage(player_id));
                }
                m_network_core.SendMessage(ServerLobbyUpdateMessage(player_id, LobbyStartDoc()));
            } else { // immediately start a new game with the given parameters
                m_single_player_game = true;
                m_expected_players = boost::lexical_cast<int>(doc.root_node.Child("num_players").Text());
                m_galaxy_size = boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("size").Text());
                m_galaxy_shape = Universe::Shape(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("shape").Text()));
                m_galaxy_age = Universe::Age(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("age").Text()));
                m_starlane_freq = Universe::StarlaneFrequency(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("starlane_freq").Text()));
                m_planet_density = Universe::PlanetDensity(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("planet_density").Text()));
                m_specials_freq = Universe::SpecialsFrequency(boost::lexical_cast<int>(doc.root_node.Child("universe_params").Child("specials_freq").Text()));
                CreateAIClients(doc.root_node);
                g_load_doc.root_node = GG::XMLElement();
                g_lobby_data.players.clear();
                g_lobby_data.players.push_back(PlayerSetupData());
                g_lobby_data.players.back().empire_name = doc.root_node.Child("empire_name").Text();
                g_lobby_data.players.back().empire_color = GG::Clr(doc.root_node.Child("empire_color").Child("GG::Clr"));
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
                m_network_core.SendMessage(ServerLobbyUpdateMessage(player_id, LobbyStartDoc()));
                for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, LobbyUpdateDoc()));
                }
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
    // this will not usually happen, since the host process usually owns the server process, and will usually take it down if it fails
    if (id == NetworkCore::HOST_PLAYER_ID) {
        if (m_state == SERVER_MP_LOBBY) { // host disconnected in MP lobby
            GG::XMLDoc doc;
            doc.root_node.AppendChild("abort_game");
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != id) {
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, doc));
                    m_network_core.DumpPlayer(it->first);
                }
            }
            m_network_core.DumpPlayer(id);
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_network_core.DumpAllConnections();
            Exit(1);
        } else if (m_losers.find(id) == m_losers.end()) { // host abnormally disconnected during a regular game
            // if the host dies, there's really nothing else we can do
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != id)
                    m_network_core.SendMessage(EndGameMessage(-1, it->first));
            }
            m_state = SERVER_DYING;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Host player disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
            m_network_core.DumpAllConnections();
            Exit(1);
        }
    } else {
        if (m_state == SERVER_MP_LOBBY) { // player disconnected in MP lobby
            GG::XMLDoc doc;
            doc.root_node.AppendChild("exit_lobby");
            doc.root_node.LastChild().AppendChild(GG::XMLElement("id", boost::lexical_cast<std::string>(id)));
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != id)
                    m_network_core.SendMessage(ServerLobbyUpdateMessage(it->first, doc));
            }
            m_network_core.DumpPlayer(id);
        } else if (m_losers.find(id) == m_losers.end()) { // player abnormally disconnected during a regular game
            m_state = SERVER_DISCONNECT;
            const PlayerInfo& disconnected_player_info = m_network_core.Players().find(id)->second;
            m_log_category.debugStream() << "ServerApp::PlayerDisconnected : Lost connection to player #" << boost::lexical_cast<std::string>(id) 
                << ", named \"" << disconnected_player_info.name << "\"; server now in mode " << SERVER_DISCONNECT << " (SERVER_DISCONNECT).";
            std::string message = disconnected_player_info.name;
            for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
                if (it->first != id) {
                    m_network_core.SendMessage(PlayerDisconnectedMessage(it->first, message));
                    // in the future we may find a way to recover from this, but for now we will immediately send a game ending message as well
                    m_network_core.SendMessage(EndGameMessage(-1, it->first));
                }
            }
        }
    }

    // if there are no humans left, it's time to terminate
    if (m_network_core.Players().empty() || m_ai_clients.size() == m_network_core.Players().size()) {
        m_state = SERVER_DYING;
        m_log_category.debugStream() << "ServerApp::PlayerDisconnected : All human players disconnected; server now in mode " << SERVER_DYING << " (SERVER_DYING).";
        m_network_core.DumpAllConnections();
        Exit(1);
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
#ifdef FREEORION_LINUX
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

    m_universe.CreateUniverse(m_galaxy_size, m_galaxy_shape, m_galaxy_age, m_starlane_freq, m_planet_density, m_specials_freq, 
                              m_network_core.Players().size() - m_ai_clients.size(), m_ai_clients.size(), g_lobby_data.players);
    m_log_category.debugStream() << "ServerApp::GameInit : Created universe " << " (SERVER_GAME_SETUP).";

    // add empires to turn sequence map
    // according to spec this should be done randomly
    // for now it's not
    for (it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it) {
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
        doc.root_node.AppendChild(GG::XMLElement("empire_id", boost::lexical_cast<std::string>(it->first)));

        // turn number is an attribute of the document
        doc.root_node.SetAttribute("turn_number", boost::lexical_cast<std::string>(m_current_turn));

        m_network_core.SendMessage(GameStartMessage(it->first, doc));
    }

    m_losers.clear();
}

void ServerApp::LoadGameInit()
{
    m_turn_sequence.clear();

    std::map<int, GG::XMLDoc> player_docs; // indexed by empire id
    for (int i = 0; i < g_load_doc.root_node.NumChildren(); ++i) {
        if (g_load_doc.root_node.Child(i).Tag() == "Player") {
            GG::XMLDoc player_doc;
            player_doc.root_node.SetTag("GG::XMLDoc");
            player_doc.root_node.AppendChild(g_load_doc.root_node.Child(i).Child("Orders"));
            if (g_load_doc.root_node.Child(i).ContainsChild("UI"))
                player_doc.root_node.AppendChild(g_load_doc.root_node.Child(i).Child("UI"));
            player_docs[boost::lexical_cast<int>(g_load_doc.root_node.Child(i).Child("Empire").Child("m_id").Text())] = player_doc;
        }
    }

    std::map<int, int> player_to_empire_ids;
    std::set<int> already_chosen_empire_ids;
    unsigned int i = 0;
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it, ++i) {
        if (i <= g_lobby_data.players.size())
            g_lobby_data.players.push_back(PlayerSetupData());
        player_to_empire_ids[it->first] = g_lobby_data.players[i].save_game_empire_id;
        already_chosen_empire_ids.insert(g_lobby_data.players[i].save_game_empire_id);
    }

    std::map<int, GG::XMLDoc>::iterator player_doc_it = player_docs.begin();
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it, ++player_doc_it) {
        GG::XMLDoc doc;
        int empire_id;
        if (m_single_player_game) {
            doc.root_node.AppendChild("single_player_game");
        }
        if (player_to_empire_ids[it->first] != -1) {
            empire_id = player_to_empire_ids[it->first];
        } else {
            for (std::map<int, GG::XMLDoc>::iterator doc_it = player_docs.begin(); doc_it != player_docs.end(); ++doc_it) {
                if (already_chosen_empire_ids.find(doc_it->first) == already_chosen_empire_ids.end()) {
                    empire_id = doc_it->first;
                    already_chosen_empire_ids.insert(empire_id);
                    // since this must be an AI player, it does not have the correct player name set in its Empire yet, so we need to do so now
                    Empires().Lookup(empire_id)->SetPlayerName(it->second.name);
                    break;
                }
            }
        }
        doc.root_node.AppendChild(m_universe.XMLEncode(ALL_OBJECTS_VISIBLE ? Universe::ALL_EMPIRES : empire_id));
        doc.root_node.AppendChild(m_empires.CreateClientEmpireUpdate(empire_id));
        doc.root_node.AppendChild(GG::XMLElement("empire_id", boost::lexical_cast<std::string>(empire_id)));
        doc.root_node.SetAttribute("turn_number", boost::lexical_cast<std::string>(m_current_turn));
        m_network_core.SendMessage(GameStartMessage(it->first, doc));

#if 0
        std::ofstream ofs(("LoadGameInit-doc-empire" + boost::lexical_cast<std::string>(empire_id) + ".xml").c_str());
        doc.WriteDoc(ofs);
        ofs.close();
#endif

        // send saved pending orders to player
        m_network_core.SendMessage(ServerLoadGameMessage(it->first, player_docs[empire_id]));

        m_turn_sequence[empire_id] = 0;
    }

    m_losers.clear();
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

    retval.root_node.AppendChild(GG::XMLElement(g_lobby_data.new_game ? "new_game" : "load_game"));

    retval.root_node.AppendChild(GG::XMLElement("universe_params"));
    retval.root_node.LastChild().AppendChild(GG::XMLElement("size", boost::lexical_cast<std::string>(g_lobby_data.size)));
    retval.root_node.LastChild().AppendChild(GG::XMLElement("shape", boost::lexical_cast<std::string>(g_lobby_data.shape)));
    retval.root_node.LastChild().AppendChild(GG::XMLElement("age", boost::lexical_cast<std::string>(g_lobby_data.age)));
    retval.root_node.LastChild().AppendChild(GG::XMLElement("starlane_freq", boost::lexical_cast<std::string>(g_lobby_data.starlane_freq)));
    retval.root_node.LastChild().AppendChild(GG::XMLElement("planet_density", boost::lexical_cast<std::string>(g_lobby_data.planet_density)));
    retval.root_node.LastChild().AppendChild(GG::XMLElement("specials_freq", boost::lexical_cast<std::string>(g_lobby_data.specials_freq)));

    if (!g_lobby_data.save_games.empty()) {
        retval.root_node.AppendChild(GG::XMLElement("save_games"));
        std::string save_games;
        for (unsigned int i = 0; i < g_lobby_data.save_games.size(); ++i) {
            save_games += g_lobby_data.save_games[i] + " ";
        }
        retval.root_node.LastChild().SetText(save_games);
    }

    retval.root_node.AppendChild(GG::XMLElement("save_file", boost::lexical_cast<std::string>(g_lobby_data.save_file)));

    if (!g_lobby_data.save_game_empire_data.empty()) {
        retval.root_node.AppendChild(GG::XMLElement("save_game_empire_data"));
        for (unsigned int i = 0; i < g_lobby_data.save_game_empire_data.size(); ++i) {
            retval.root_node.LastChild().AppendChild(g_lobby_data.save_game_empire_data[i].XMLEncode());
        }
    }

    retval.root_node.AppendChild(GG::XMLElement("players"));
    unsigned int i = 0;
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it, ++i) {
        retval.root_node.LastChild().AppendChild(GG::XMLElement(it->second.name));
        retval.root_node.LastChild().LastChild().AppendChild(GG::XMLElement("id", boost::lexical_cast<std::string>(it->first)));
        if (i <= g_lobby_data.players.size())
            g_lobby_data.players.push_back(PlayerSetupData());
        retval.root_node.LastChild().LastChild().AppendChild(g_lobby_data.players[i].XMLEncode());
    }

    return retval;
}

GG::XMLDoc ServerApp::LobbyStartDoc() const
{
    GG::XMLDoc retval;

    if (!g_lobby_data.save_game_empire_data.empty()) {
        retval.root_node.AppendChild(GG::XMLElement("save_game_empire_data"));
        for (unsigned int i = 0; i < g_lobby_data.save_game_empire_data.size(); ++i) {
            retval.root_node.LastChild().AppendChild(g_lobby_data.save_game_empire_data[i].XMLEncode());
        }
    }

    if (!g_lobby_data.save_games.empty()) {
        retval.root_node.AppendChild(GG::XMLElement("save_games"));
        std::string save_games;
        for (unsigned int i = 0; i < g_lobby_data.save_games.size(); ++i) {
            save_games += g_lobby_data.save_games[i] + " ";
        }
        retval.root_node.LastChild().SetText(save_games);
    }

    retval.root_node.AppendChild(GG::XMLElement("empire_colors"));
    for (unsigned int i = 0; i < g_lobby_data.empire_colors.size(); ++i) {
        retval.root_node.LastChild().AppendChild(g_lobby_data.empire_colors[i].XMLEncode());
    }

    retval.root_node.AppendChild(GG::XMLElement("players"));
    unsigned int i = 0;
    for (std::map<int, PlayerInfo>::const_iterator it = m_network_core.Players().begin(); it != m_network_core.Players().end(); ++it, ++i) {
        retval.root_node.LastChild().AppendChild(GG::XMLElement(it->second.name));
        retval.root_node.LastChild().LastChild().AppendChild(GG::XMLElement("id", boost::lexical_cast<std::string>(it->first)));
        if (i <= g_lobby_data.players.size())
            g_lobby_data.players.push_back(PlayerSetupData());
        retval.root_node.LastChild().LastChild().AppendChild(g_lobby_data.players[i].XMLEncode());
    }

    return retval;
}

GG::XMLDoc ServerApp::SaveGameUpdateDoc() const
{
    GG::XMLDoc retval;

    retval.root_node.AppendChild(GG::XMLElement("load_game"));

    if (!g_lobby_data.save_game_empire_data.empty()) {
        retval.root_node.AppendChild(GG::XMLElement("save_game_empire_data"));
        for (unsigned int i = 0; i < g_lobby_data.save_game_empire_data.size(); ++i) {
            retval.root_node.LastChild().AppendChild(g_lobby_data.save_game_empire_data[i].XMLEncode());
        }
    }

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


Empire* ServerApp::GetPlayerEmpire(int player_id) const
{
    Empire* retval = 0;
    std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().find(player_id);
    if (player_it != m_network_core.Players().end()) {
        std::string player_name = player_it->second.name;
        for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
            if (it->second->PlayerName() == player_name) {
                retval = it->second;
                break;
            }
        }
    }
    return retval;
}


void ServerApp::AddEmpireTurn( int empire_id )
{
    // add empire
    m_turn_sequence[ empire_id ] = NULL;
}


void ServerApp::RemoveEmpireTurn( int empire_id )
{
    m_turn_sequence.erase( empire_id );
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
 
    // filter FleetColonizeOrder for later processing
    std::map<int,std::vector<FleetColonizeOrder*> > map_planet_colonize_order_list;
    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it)
    {
        pOrderSet = it->second;

        /// filter FleetColonizeOrder and sort them per planet
        FleetColonizeOrder *order;
        for ( order_it = pOrderSet->begin(); order_it != pOrderSet->end(); ++order_it)
            if((order=dynamic_cast<FleetColonizeOrder*>(order_it->second)))
            {
                std::map<int,std::vector<FleetColonizeOrder*> >::iterator it = map_planet_colonize_order_list.find(order->PlanetID());
                if(it == map_planet_colonize_order_list.end())
                {
                  map_planet_colonize_order_list.insert(std::pair<int,std::vector<FleetColonizeOrder*> >(order->PlanetID(),std::vector<FleetColonizeOrder*>()));
                  it = map_planet_colonize_order_list.find(order->PlanetID());
                }
                it->second.push_back(order);
            }
    }

    // colonization apply be the following rules
    // 1 - if there is only own empire which tries to colonize a planet, is allowed to do so
    // 2 - if there are more than one empire then
    // 2.a - if only one empire which tries to colonize (empire who doesn't are ignored) is armed, this empire wins the race
    // 2.b - if more than one empire is armed or all forces are unarmed, noone can colonize the planet
    for (std::map<int,std::vector<FleetColonizeOrder*> >::iterator it = map_planet_colonize_order_list.begin(); it != map_planet_colonize_order_list.end(); ++it)
    {
      // only one empire?
      if(it->second.size()==1)
        it->second[0]->ExecuteServerApply();
      else
      {
        const Planet *planet = GetUniverse().Object<Planet>(it->first);
        const System *system = GetUniverse().Object<System>(planet->SystemID());

        std::vector<const Fleet*> vec_fleet = system->FindObjects<Fleet>();
        std::set<int> set_empire_with_military;
        for(unsigned int i=0;i<vec_fleet.size();i++)
          for(Fleet::const_iterator ship_it=vec_fleet[i]->begin();ship_it!=vec_fleet[i]->end();++ship_it)
            if(GetUniverse().Object<Ship>(*ship_it)->IsArmed())
            {
              set_empire_with_military.insert(*vec_fleet[i]->Owners().begin());
              break;
            }

        // set the first empire as winner for now
        int winner = 0;
        // is the current winner armed?
        bool winner_is_armed = set_empire_with_military.find(it->second[0]->EmpireID()) != set_empire_with_military.end();
        for(unsigned int i=1;i<it->second.size();i++)
          // is this empire armed?
          if(set_empire_with_military.find(it->second[i]->EmpireID()) != set_empire_with_military.end())
          {
            // if this empire is armed and the former winner too, noone can win
            if(winner_is_armed)
            {
              winner = -1; // no winner!!
              break;       // won't find a winner!
            }
            winner = i; // this empire is the winner for now
            winner_is_armed = true; // and has armed forces
          }
          else
            // this empire isn't armed
            if(!winner_is_armed)
              winner = -1; // if the current winner isn't armed, a winner must be armed!!!!

        for(int i=0;i<static_cast<int>(it->second.size());i++)
          if(winner==i) 
            it->second[i]->ExecuteServerApply();
          else
            it->second[i]->ExecuteServerRevoke();
      }
    }

    /// process movement phase
    for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage( player_it->first, Message::FLEET_MOVEMENT, -1));

    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it)
        it->second->MovementPhase();


    /// process production and growth phase
    for (ServerEmpireManager::iterator it = Empires().begin(); it != Empires().end(); ++it)
        it->second->UpdateResourcePool();

    for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it) 
        m_network_core.SendMessage(TurnProgressMessage( player_it->first, Message::EMPIRE_PRODUCTION, -1));

    for (Universe::const_iterator it = GetUniverse().begin(); it != GetUniverse().end(); ++it)
        it->second->PopGrowthProductionResearchPhase();

    for (std::map<int, OrderSet*>::iterator it = m_turn_sequence.begin(); it != m_turn_sequence.end(); ++it) {
        ///< check now for completed research
        Empire* empire = Empires().Lookup(it->first);
        empire->CheckResearchProgress();
    }

    // find planets which have starved to death
    std::vector<Planet*> plt_vec = GetUniverse().FindObjects<Planet>();
    for(std::vector<Planet*>::iterator it = plt_vec.begin();it!=plt_vec.end();++it)
      if((*it)->Owners().size()>0 && (*it)->PopPoints()==0.0)
      {
        // add some information to sitreport
        Empire *empire = Empires().Lookup(*(*it)->Owners().begin());
        empire->AddSitRepEntry(CreatePlanetStarvedToDeathSitRep((*it)->SystemID(),(*it)->ID()));
        (*it)->RemoveOwner(*(*it)->Owners().begin());
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
    bool combat_happend = false;
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
        combat_happend=true;
        CombatSystem combat_system;
        combat_system.ResolveCombat(system->ID(),empire_combat_forces);
      }
    }

    // if a combat happend give the human user a chance to look at the results
    if(combat_happend)
      SDL_Delay(1000);

    ++m_current_turn;

    // check if all empires are still alive
    std::map<int, int> eliminations; // map from player ids to empire ids
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        if (GetUniverse().FindObjects(IsOwnedObjectFunctor<Planet>(it->first)).empty()) { // when you're out of planets, your game over
            std::string player_name = it->second->PlayerName();
            for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); 
                 player_it != m_network_core.Players().end(); ++player_it) {
                if (player_it->second.name == player_name) {
                    // record this player/empire so we can send out messages about it
                    eliminations[player_it->first] = it->first;
                    break;
                }
            }
        } 
    }

    // clean up defeated empires
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        // remove the empire from play
        Universe::ObjectVec object_vec = GetUniverse().FindObjects(IsOwnedByFunctor(it->second));
        for (unsigned int j = 0; j < object_vec.size(); ++j) {
            if (object_vec[j]->WhollyOwnedBy(it->second)) {
                GetUniverse().Remove(object_vec[j]->ID());
            } else { // the object is owned by this empire, but also by at least one other empire as well
                object_vec[j]->RemoveOwner(it->second);
            }
        }
    }

    // send new-turn updates to all players
    for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it)
    {
        pEmpire = GetPlayerEmpire( player_it->first );
        GG::XMLDoc doc = CreateTurnUpdate( pEmpire->EmpireID() );
        m_network_core.SendMessage( TurnUpdateMessage( player_it->first, doc ) );
    }

    // notify all players of the eliminated players
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        for (std::map<int, PlayerInfo>::const_iterator player_it = m_network_core.Players().begin(); player_it != m_network_core.Players().end(); ++player_it) {
            m_network_core.SendMessage(PlayerEliminatedMessage(player_it->first, Empires().Lookup(it->second)->Name()));
        }
    }

    // dump connections to eliminated players, and remove server-side empire data
    for (std::map<int, int>::iterator it = eliminations.begin(); it != eliminations.end(); ++it) {
        m_log_category.debugStream() << "ServerApp::ProcessTurns : Player " << it->first << " is marked as a loser and dumped";
        m_losers.insert(it->first);
        m_network_core.DumpPlayer(it->first);
        m_ai_IDs.erase(it->first);
        Empires().EliminateEmpire(it->second);
        RemoveEmpireTurn(it->second);
    }

    // determine if victory conditions exist
    if (m_network_core.Players().size() == 1) { // if there is only one player left, that player is the winner
        m_log_category.debugStream() << "ServerApp::ProcessTurns : One player left -- sending victory notification and terminating.";
        m_network_core.SendMessage(VictoryMessage(m_network_core.Players().begin()->first));
        // TODO: flush pending messages before Exit(0), instead of waiting 5 seconds
        SDL_Delay(5000);
        m_network_core.DumpAllConnections();
        Exit(0);
    } else if (m_ai_IDs.size() == m_network_core.Players().size()) { // if there are none but AI players left, we're done
        m_log_category.debugStream() << "ServerApp::ProcessTurns : No human players left -- server terminating.";
        // TODO: flush pending messages before Exit(0), instead of waiting 5 seconds
        SDL_Delay(5000);
        m_network_core.DumpAllConnections();
        Exit(0);
    }
}
