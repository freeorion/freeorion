#ifdef FREEORION_WIN32
#include <GL/glew.h>
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "HumanClientApp.h"

#include "HumanClientFSM.h"
#include "../../UI/CUIControls.h"
#include "../../UI/CUIStyle.h"
#include "../../UI/MapWnd.h"
#include "../../network/Message.h"
#include "../../network/Networking.h"
#include "../../UI/GalaxySetupWnd.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../UI/ServerConnectWnd.h"
#include "../../UI/Sound.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../universe/Planet.h"
#include "../../universe/Species.h"
#include "../../util/Process.h"
#include "../../util/Serialize.h"
#include "../../util/SitRepEntry.h"
#include "../../util/Directories.h"
#include "../../Empire/Empire.h"

#include <GG/BrowseInfoWnd.h>
#include <GG/Cursor.h>

#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/serialization/vector.hpp>

#include <sstream>

#ifdef ENABLE_CRASH_BACKTRACE
# include <signal.h>
# include <execinfo.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>

void SigHandler(int sig)
{
    void* backtrace_buffer[100];
    int num;
    int fd;

    signal(sig, SIG_DFL);
    fd = open("crash.txt",O_WRONLY|O_CREAT|O_APPEND|O_SYNC,0666);
    if (fd != -1) {
        write(fd, "--- New crash backtrace begins here ---\n", 24);
        num = backtrace(backtrace_buffer, 100);
        backtrace_symbols_fd(backtrace_buffer, num, fd);
        backtrace_symbols_fd(backtrace_buffer, num, 2);
        close(fd);
    }

    // Now we try to display a MessageBox; this might fail and also
    // corrupt the heap, but since we're dying anyway that's no big
    // deal

    ClientUI::MessageBox("The client has just crashed!\nFile a bug report and\nattach the file called 'crash.txt'\nif necessary", true);

    raise(sig);
}
#endif //ENABLE_CRASH_BACKTRACE

namespace {
    const unsigned int  SERVER_CONNECT_TIMEOUT = 10000; // in ms

    const bool          INSTRUMENT_MESSAGE_HANDLING = false;

    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("autosave.single-player",    "OPTIONS_DB_AUTOSAVE_SINGLE_PLAYER",    true,   Validator<bool>());
        db.Add("autosave.multiplayer",      "OPTIONS_DB_AUTOSAVE_MULTIPLAYER",      false,  Validator<bool>());
        db.Add("autosave.turns",            "OPTIONS_DB_AUTOSAVE_TURNS",            1,      RangedValidator<int>(1, 50));
        db.Add("music-volume",              "OPTIONS_DB_MUSIC_VOLUME",              255,    RangedValidator<int>(1, 255));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /* Sets the value of options that need language-dependent default values.*/
    void SetStringtableDependentOptionDefaults() {
        if (GetOptionsDB().Get<std::string>("GameSetup.empire-name").empty())
            GetOptionsDB().Set("GameSetup.empire-name", UserString("DEFAULT_EMPIRE_NAME"));

        if (GetOptionsDB().Get<std::string>("GameSetup.player-name").empty())
            GetOptionsDB().Set("GameSetup.player-name", UserString("DEFAULT_PLAYER_NAME"));

        if (GetOptionsDB().Get<std::string>("multiplayersetup.player-name").empty())
            GetOptionsDB().Set("multiplayersetup.player-name", UserString("DEFAULT_PLAYER_NAME"));
    }

    static float stored_gl_version = -1.0f;  // to be replaced when gl version first checked

    float GetGLVersion() {
        if (stored_gl_version != -1.0f)
            return stored_gl_version;

        // get OpenGL version string and parse to get version number
        const GLubyte* gl_version = glGetString(GL_VERSION);
        std::string gl_version_string = boost::lexical_cast<std::string>(gl_version);

        float version_number = 0.0f;
        std::istringstream iss(gl_version_string);
        iss >> version_number;
        version_number += 0.05f;    // ensures proper rounding of 1.1 digit number

        stored_gl_version = version_number;

        return stored_gl_version;
    }

    void SetGLVersionDependentOptionDefaults() {
        // get OpenGL version string and parse to get version number
        float version_number = GetGLVersion();
        Logger().debugStream() << "OpenGL Version Number: " << DoubleToString(version_number, 2, false);    // combination of floating point precision and DoubleToString preferring to round down means the +0.05 is needed to round properly
        if (version_number < 1.5) {
            Logger().errorStream() << "OpenGL Version is less than 2.0 (official required) or 1.5 (usually works).  FreeOrion may crash when trying to start a game.";
        }

        // only execute default option setting once
        if (GetOptionsDB().Get<bool>("checked-gl-version"))
            return;
        GetOptionsDB().Set<bool>("checked-gl-version", true);

        // if GL version is too low, set various map rendering options to
        // disabled, so as to prevent crashes when running on systems that
        // don't support these GL features.  these options are added to the
        // DB in MapWnd.cpp's AddOptions and all default to true.
        if (version_number < 2.0) {
            GetOptionsDB().Set<bool>("UI.galaxy-gas-background",        false);
            GetOptionsDB().Set<bool>("UI.galaxy-starfields",            false);
            GetOptionsDB().Set<bool>("UI.optimized-system-rendering",   false);
            GetOptionsDB().Set<bool>("UI.system-fog-of-war",            false);
        }
    }
}

HumanClientApp::HumanClientApp(Ogre::Root* root,
                               Ogre::RenderWindow* window,
                               Ogre::SceneManager* scene_manager,
                               Ogre::Camera* camera,
                               Ogre::Viewport* viewport) :
    ClientApp(),
    OgreGUI(window, (GetBinDir() / "OISInput.cfg").string()),
    m_fsm(0),
    m_single_player_game(true),
    m_game_started(false),
    m_turns_since_autosave(0),
    m_connected(false),
    m_root(root),
    m_scene_manager(scene_manager),
    m_camera(camera),
    m_viewport(viewport)
{
#ifdef ENABLE_CRASH_BACKTRACE
    signal(SIGSEGV, SigHandler);
#endif
    m_fsm = new HumanClientFSM(*this);

    const std::string LOG_FILENAME((GetUserDir() / "freeorion.log").file_string());

    // a platform-independent way to erase the old log We cannot use
    // boost::filesystem::ofstream here, as stupid b::f won't allow us
    // to have a dot in the directory name, which is where local data
    // is kept under unix.
    std::ofstream temp(LOG_FILENAME.c_str());
    temp.close();

    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p Client : %m%n");
    appender->setLayout(layout);
    Logger().setAdditivity(false);  // make appender the only appender used...
    Logger().setAppender(appender);
    Logger().setAdditivity(true);   // ...but allow the addition of others later
    Logger().setPriority(PriorityValue(GetOptionsDB().Get<std::string>("log-level")));

    boost::shared_ptr<GG::StyleFactory> style(new CUIStyle());
    SetStyleFactory(style);

    SetMinDragTime(0);
    EnableMouseButtonDownRepeat(250, 15);

    m_ui = boost::shared_ptr<ClientUI>(new ClientUI());

    if (!(GetOptionsDB().Get<bool>("music-off")))
        Sound::GetSound().PlayMusic(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("bg-music"), -1);

    Sound::GetSound().SetMusicVolume(GetOptionsDB().Get<int>("music-volume"));
    Sound::GetSound().SetUISoundsVolume(GetOptionsDB().Get<int>("UI.sound.volume"));

    EnableFPS();
    UpdateFPSLimit();
    GG::Connect(GetOptionsDB().OptionChangedSignal("show-fps"), &HumanClientApp::UpdateFPSLimit, this);

    boost::shared_ptr<GG::BrowseInfoWnd> default_browse_info_wnd(
        new GG::TextBoxBrowseInfoWnd(GG::X(400), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                     GG::Clr(0, 0, 0, 200), ClientUI::WndOuterBorderColor(), ClientUI::TextColor(),
                                     GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK, 1));
    GG::Wnd::SetDefaultBrowseInfoWnd(default_browse_info_wnd);

    boost::shared_ptr<GG::Texture> cursor_texture = m_ui->GetTexture(ClientUI::ArtDir() / "cursors" / "default_cursor.png");
    SetCursor(boost::shared_ptr<GG::TextureCursor>(new GG::TextureCursor(cursor_texture, GG::Pt(GG::X(6), GG::Y(3)))));
    RenderCursor(true);

#ifdef FREEORION_WIN32
    GLenum error = glewInit();
    assert(error == GLEW_OK);
#endif

    SetStringtableDependentOptionDefaults();
    SetGLVersionDependentOptionDefaults();

    m_fsm->initiate();
}

HumanClientApp::~HumanClientApp()
{
    if (Networking().Connected())
        Networking().DisconnectFromServer();
    m_server_process.RequestTermination();
    delete m_fsm;
}

const std::string& HumanClientApp::SaveFileName() const
{ return m_save_filename; }

bool HumanClientApp::SinglePlayerGame() const
{ return m_single_player_game; }

void HumanClientApp::StartServer()
{
#ifdef FREEORION_WIN32
    const std::string SERVER_CLIENT_EXE = (GetBinDir() / "freeoriond.exe").file_string();
#else
    const std::string SERVER_CLIENT_EXE = (GetBinDir() / "freeoriond").file_string();
#endif
    std::vector<std::string> args;
    args.push_back("\"" + SERVER_CLIENT_EXE + "\"");
    args.push_back("--resource-dir");
    args.push_back("\"" + GetOptionsDB().Get<std::string>("resource-dir") + "\"");
    args.push_back("--log-level");
    args.push_back(GetOptionsDB().Get<std::string>("log-level"));
    if (GetOptionsDB().Get<bool>("test-3d-combat"))
        args.push_back("--test-3d-combat");
    m_server_process = Process(SERVER_CLIENT_EXE, args);
}

void HumanClientApp::FreeServer()
{
    m_server_process.Free();
    SetPlayerID(-1);
    SetEmpireID(ALL_EMPIRES);
}

void HumanClientApp::KillServer()
{
    m_server_process.Kill();
    SetPlayerID(-1);
    SetEmpireID(ALL_EMPIRES);
}

void HumanClientApp::NewSinglePlayerGame(bool quickstart)
{
    if (!GetOptionsDB().Get<bool>("force-external-server")) {
        try {
            StartServer();
        } catch (std::runtime_error err) {
            Logger().errorStream() << "Couldn't start server.  Got error message: " << err.what();
            ClientUI::MessageBox(UserString("SERVER_WONT_START"), true);
            return;
        }
    }

    GalaxySetupWnd galaxy_wnd;
    if (!quickstart) {
        galaxy_wnd.Run();
    }

    bool failed = false;
    if (quickstart || galaxy_wnd.EndedWithOk()) {
        unsigned int start_time = Ticks();
        while (!Networking().ConnectToLocalHostServer()) {
            if (SERVER_CONNECT_TIMEOUT < Ticks() - start_time) {
                ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                failed = true;
                break;
            }
        }

        if (!failed) {
            SinglePlayerSetupData setup_data;

            setup_data.m_new_game = true;
            setup_data.m_filename.clear();  // not used for new game


            // get values stored in options from previous time game was run or
            // from just having run GalaxySetupWnd

            // GalaxySetupData
            setup_data.m_size =             GetOptionsDB().Get<int>("GameSetup.stars");
            setup_data.m_shape =            GetOptionsDB().Get<Shape>("GameSetup.galaxy-shape");
            setup_data.m_age =              GetOptionsDB().Get<Age>("GameSetup.galaxy-age");
            setup_data.m_starlane_freq =    GetOptionsDB().Get<StarlaneFrequency>("GameSetup.starlane-frequency");
            setup_data.m_planet_density =   GetOptionsDB().Get<PlanetDensity>("GameSetup.planet-density");
            setup_data.m_specials_freq =    GetOptionsDB().Get<SpecialsFrequency>("GameSetup.specials-frequency");


            // SinglePlayerSetupData contains a map of PlayerSetupData, for
            // the human and AI players.  Need to compile this information
            // from the specified human options and number of requested AIs

            // Human player setup data first

            PlayerSetupData human_player_setup_data;
            human_player_setup_data.m_player_name = GetOptionsDB().Get<std::string>("GameSetup.player-name");
            human_player_setup_data.m_empire_name = GetOptionsDB().Get<std::string>("GameSetup.empire-name");

            // DB stores index into array of available colours, so need to get that array to look up value of index.
            // if stored value is invalid, use a default colour
            const std::vector<GG::Clr>& empire_colours = EmpireColors();
            int colour_index = GetOptionsDB().Get<int>("GameSetup.empire-color");
            if (colour_index >= 0 && colour_index < static_cast<int>(empire_colours.size()))
                human_player_setup_data.m_empire_color = empire_colours[colour_index];
            else
                human_player_setup_data.m_empire_color = GG::CLR_GREEN;

            human_player_setup_data.m_starting_species_name = GetOptionsDB().Get<std::string>("GameSetup.starting-species");
            if (!GetSpecies(human_player_setup_data.m_starting_species_name)) {
                const SpeciesManager& sm = GetSpeciesManager();
                if (sm.empty())
                    human_player_setup_data.m_starting_species_name.clear();
                else
                    human_player_setup_data.m_starting_species_name = sm.begin()->first;
             }

            human_player_setup_data.m_save_game_empire_id = -1; // not used for new games
            human_player_setup_data.m_client_type = Networking::CLIENT_TYPE_HUMAN_PLAYER;

            // add to setup data players
            setup_data.m_players.push_back(human_player_setup_data);

            // AI player setup data.  One entry for each requested AI

            int num_AIs = GetOptionsDB().Get<int>("GameSetup.ai-players");
            for (int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
                PlayerSetupData ai_setup_data;

                ai_setup_data.m_player_name = "AI_" + boost::lexical_cast<std::string>(ai_i);
                ai_setup_data.m_empire_name.clear();            // leave blank, to be set by server in Universe::GenerateEmpires
                ai_setup_data.m_empire_color = GG::CLR_ZERO;    // to be set by server
                ai_setup_data.m_starting_species_name.clear();  // leave blank, to be set by server
                ai_setup_data.m_save_game_empire_id = -1;       // not used for new games
                ai_setup_data.m_client_type = Networking::CLIENT_TYPE_AI_PLAYER;

                setup_data.m_players.push_back(ai_setup_data);
            }

            Networking().SendMessage(HostSPGameMessage(setup_data));
            m_fsm->process_event(HostSPGameRequested(WAITING_FOR_NEW_GAME));
        }
    } else {
        failed = true;
    }

    if (failed)
        KillServer();
    else
        m_connected = true;
}

void HumanClientApp::MulitplayerGame()
{
    ServerConnectWnd server_connect_wnd;
    bool failed = false;
    while (!failed && !Networking().Connected()) {
        server_connect_wnd.Run();

        if (server_connect_wnd.Result().second == "") {
            failed = true;
        } else {
            std::string server_name = server_connect_wnd.Result().second;
            if (server_connect_wnd.Result().second == "HOST GAME SELECTED") {
                if (!GetOptionsDB().Get<bool>("force-external-server")) {
                    HumanClientApp::GetApp()->StartServer();
                    HumanClientApp::GetApp()->FreeServer();
                    server_name = "localhost";
                }                
                server_name = GetOptionsDB().Get<std::string>("external-server-address");
            }
            unsigned int start_time = Ticks();
            while (!Networking().ConnectToServer(server_name)) {
                if (SERVER_CONNECT_TIMEOUT < Ticks() - start_time) {
                    ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                    if (server_connect_wnd.Result().second == "HOST GAME SELECTED")
                        KillServer();
                    failed = true;
                    break;
                }
            }
        }
    }

    if (!failed) {
        if (server_connect_wnd.Result().second == "HOST GAME SELECTED") {
            Networking().SendMessage(HostMPGameMessage(server_connect_wnd.Result().first));
            m_fsm->process_event(HostMPGameRequested());
        } else {
            Networking().SendMessage(JoinGameMessage(server_connect_wnd.Result().first, Networking::CLIENT_TYPE_HUMAN_PLAYER));
            m_fsm->process_event(JoinMPGameRequested());
        }
        m_connected = true;
    }
}

void HumanClientApp::SaveGame(const std::string& filename)
{
    Message response_msg;
    Networking().SendSynchronousMessage(HostSaveGameMessage(PlayerID(), filename), response_msg);
    if (response_msg.Type() != Message::SAVE_GAME) {
        Logger().errorStream() << "HumanClientApp::SaveGame sent synchronous HostSaveGameMessage, but received back message of wrong type: " << response_msg.Type();
        throw std::runtime_error("HumanClientApp::SaveGame synchronous message received invalid response message type");
    }
    HandleSaveGameDataRequest();
}

void HumanClientApp::EndGame()
{ EndGame(false); }

void HumanClientApp::LoadSinglePlayerGame(std::string filename/* = ""*/)
{
    if (filename != "") {
        if (!exists(boost::filesystem::path(filename))) {
            std::string msg =
                "HumanClientApp::LoadSinglePlayerGame() given a nonexistent file \"" + filename + "\" to load; aborting.";
            Logger().fatalStream() << msg;
            std::cerr << msg << '\n';
            abort();
        }
    } else {
        try {
            std::vector<std::pair<std::string, std::string> > save_file_types;
            save_file_types.push_back(std::pair<std::string, std::string>(UserString("GAME_MENU_SAVE_FILES"), "*.sav"));

            FileDlg dlg(GetSaveDir().native_file_string(), "", false, false, save_file_types);
            dlg.Run();
            if (!dlg.Result().empty())
                filename = *dlg.Result().begin();
        } catch (const FileDlg::BadInitialDirectory& e) {
            ClientUI::MessageBox(e.what(), true);
        }
    }

    if (!filename.empty()) {
        // end any currently-playing game before loading new one
        if (m_game_started) {
            EndGame();
            Sleep(1500);    // delay to make sure old game is fully cleaned up before attempting to start a new one
        } else {
            Logger().debugStream() << "HumanClientApp::LoadSinglePlayerGame() not already in a game, so don't need to end it";
        }

        if (!GetOptionsDB().Get<bool>("force-external-server")) {
            Logger().debugStream() << "HumanClientApp::LoadSinglePlayerGame() Starting server";
            StartServer();
        } else {
            Logger().debugStream() << "HumanClientApp::LoadSinglePlayerGame() assuming external server will be available";
        }

        unsigned int start_time = Ticks();
        while (!Networking().ConnectToLocalHostServer()) {
            if (SERVER_CONNECT_TIMEOUT < Ticks() - start_time) {
                ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                KillServer();
                return;
            }
        }

        Logger().debugStream() << "HumanClientApp::LoadSinglePlayerGame() connected to server";

        m_connected = true;
        SetPlayerID(Networking::HOST_PLAYER_ID);
        SetEmpireID(ALL_EMPIRES);

        SinglePlayerSetupData setup_data;
        // leving GalaxySetupData information default / blank : not used when loading a game
        setup_data.m_new_game = false;
        setup_data.m_filename = filename;
        // leving setup_data.m_players empty : not specified when loading a game, as server will generate from save file


        Networking().SendMessage(HostSPGameMessage(setup_data));
        m_fsm->process_event(HostSPGameRequested(WAITING_FOR_LOADED_GAME));
    }
}

void HumanClientApp::SetSaveFileName(const std::string& filename)
{ m_save_filename = filename; }

Ogre::SceneManager* HumanClientApp::SceneManager()
{ return m_scene_manager; }

Ogre::Camera* HumanClientApp::Camera()
{ return m_camera; }

Ogre::Viewport* HumanClientApp::Viewport()
{ return m_viewport; }

float HumanClientApp::GLVersion() const
{
    return GetGLVersion();
}

void HumanClientApp::Enter2DMode()
{
    OgreGUI::Enter2DMode();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);

    float ambient_light[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // set up coordinates with origin in upper-left and +x and +y directions right and down, respectively
    // the depth of the viewing volume is only 1 (from 0.0 to 1.0)
    glOrtho(0.0, Value(AppWidth()), Value(AppHeight()), 0.0, 0.0, Value(AppWidth()));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void HumanClientApp::Exit2DMode()
{ OgreGUI::Exit2DMode(); }

void HumanClientApp::StartTurn()
{
    ClientApp::StartTurn();
    m_fsm->process_event(TurnEnded());
}

void HumanClientApp::HandleSystemEvents()
{
    OgreGUI::HandleSystemEvents();
    if (m_connected && !Networking().Connected()) {
        m_connected = false;
        // Note that Disconnections are handled with a post_event instead of a process_event.  This is because a
        // Disconnection inherently precipitates a transition out of any state S that handles it, and if another event
        // that also causes a transition out of S is currently active (e.g. MPLobby), a double-destruction of S will
        // occur.
        m_fsm->post_event(Disconnection());
    } else if (Networking().MessageAvailable()) {
        Message msg;
        Networking().GetMessage(msg);
        HandleMessage(msg);
    }
}

void HumanClientApp::RenderBegin()
{
    OgreGUI::RenderBegin();
    Sound::GetSound().DoFrame();
}

void HumanClientApp::HandleMessage(Message& msg)
{
    if (INSTRUMENT_MESSAGE_HANDLING)
        std::cerr << "HumanClientApp::HandleMessage(" << MessageTypeStr(msg.Type()) << ")\n";

    switch (msg.Type()) {
    case Message::ERROR:                m_fsm->process_event(Error(msg)); break;
    case Message::HOST_MP_GAME:         m_fsm->process_event(HostMPGame(msg)); break;
    case Message::HOST_SP_GAME:         m_fsm->process_event(HostSPGame(msg)); break;
    case Message::JOIN_GAME:            m_fsm->process_event(JoinGame(msg)); break;
    case Message::LOBBY_UPDATE:         m_fsm->process_event(LobbyUpdate(msg)); break;
    case Message::LOBBY_CHAT:           m_fsm->process_event(LobbyChat(msg)); break;
    case Message::LOBBY_HOST_ABORT:     m_fsm->process_event(LobbyHostAbort(msg)); break;
    case Message::LOBBY_EXIT:           m_fsm->process_event(LobbyNonHostExit(msg)); break;
    case Message::SAVE_GAME:            m_fsm->process_event(::SaveGame(msg)); break;
    case Message::GAME_START:           m_fsm->process_event(GameStart(msg)); break;
    case Message::TURN_UPDATE:          m_fsm->process_event(TurnUpdate(msg)); break;
    case Message::TURN_PROGRESS:        m_fsm->process_event(TurnProgress(msg)); break;
    case Message::COMBAT_START:         m_fsm->process_event(CombatStart(msg)); break;
    case Message::COMBAT_TURN_UPDATE:   m_fsm->process_event(CombatRoundUpdate(msg)); break;
    case Message::COMBAT_END:           m_fsm->process_event(CombatEnd(msg)); break;
    case Message::HUMAN_PLAYER_CHAT:    m_fsm->process_event(PlayerChat(msg)); break;
    case Message::VICTORY_DEFEAT :      m_fsm->process_event(VictoryDefeat(msg)); break;
    case Message::PLAYER_ELIMINATED:    m_fsm->process_event(PlayerEliminated(msg)); break;
    case Message::END_GAME:             m_fsm->process_event(::EndGame(msg)); break;
    default:
        Logger().errorStream() << "HumanClientApp::HandleMessage : Received an unknown message type \""
                               << msg.Type() << "\".";
    }
}

void HumanClientApp::HandleSaveGameDataRequest()
{
    if (INSTRUMENT_MESSAGE_HANDLING)
        std::cerr << "HumanClientApp::HandleSaveGameDataRequest(" << MessageTypeStr(Message::SAVE_GAME) << ")\n";
    SaveGameUIData ui_data;
    m_ui->GetSaveGameUIData(ui_data);
    Networking().SendMessage(ClientSaveDataMessage(PlayerID(), Orders(), ui_data));
}

void HumanClientApp::StartGame()
{
    m_game_started = true;
    Orders().Reset();
    if (const Empire* empire = Empires().Lookup(EmpireID()))
        for (Empire::SitRepItr it = empire->SitRepBegin(); it != empire->SitRepEnd(); ++it)
            m_ui->GenerateSitRepText(*it);
    else
        Logger().errorStream() << "HumanClientApp::StartGame couldn't get empire with id " << EmpireID();
}

void HumanClientApp::Autosave(bool new_game)
{
    // autosave only on appropriate turn numbers, and when enabled for current
    // game type (single vs. multiplayer)
    int autosave_turns = GetOptionsDB().Get<int>("autosave.turns");
    if (autosave_turns < 1)
        return;     // avoid divide by zero
    if (CurrentTurn() % autosave_turns != 0)
        return;     // turns divisible by autosave_turns have autosaves done
    if (m_single_player_game && !GetOptionsDB().Get<bool>("autosave.single-player"))
        return;
    if (!m_single_player_game && !GetOptionsDB().Get<bool>("autosave.multiplayer"))
        return;

    const char* legal_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

    // get empire name, filtered for filename acceptability
    std::string empire_name = Empires().Lookup(EmpireID())->Name();
    std::string::size_type first_good_empire_char = empire_name.find_first_of(legal_chars);
    if (first_good_empire_char == std::string::npos) {
        empire_name = "";
    } else {
        std::string::size_type first_bad_empire_char = empire_name.find_first_not_of(legal_chars, first_good_empire_char);
        empire_name = empire_name.substr(first_good_empire_char, first_bad_empire_char - first_good_empire_char);
    }

    // get player name, also filtered
    std::string player_name = Empires().Lookup(EmpireID())->PlayerName();
    std::string::size_type first_good_player_char = player_name.find_first_of(legal_chars);
    if (first_good_player_char == std::string::npos) {
        player_name = "";
    } else {
        std::string::size_type first_bad_player_char = player_name.find_first_not_of(legal_chars, first_good_player_char);
        player_name = player_name.substr(first_good_player_char, first_bad_player_char - first_good_player_char);
    }

    // select filename extension
    std::string extension;
    if (m_single_player_game)
        extension = SP_SAVE_FILE_EXTENSION;
    else
        extension = MP_SAVE_FILE_EXTENSION;

    std::string save_filename = boost::io::str(boost::format("FreeOrion_%s_%s_%04d%s") % player_name % empire_name % CurrentTurn() % extension);

    namespace fs = boost::filesystem;
    fs::path save_dir(GetSaveDir());

    Logger().debugStream() << "Autosaving to: " << (save_dir / save_filename).file_string();

    try {
        SaveGame((save_dir / save_filename).file_string());
    } catch (const std::exception& e) {
        Logger().errorStream() << "Autosave failed: " << e.what();
        std::cerr << "Autosave failed: " << e.what() << std::endl;
    }
}

void HumanClientApp::EndGame(bool suppress_FSM_reset)
{
    Logger().debugStream() << "HumanClientApp::EndGame";
    if (!suppress_FSM_reset)
        m_fsm->process_event(ResetToIntroMenu());
    m_game_started = false;
    Networking().DisconnectFromServer();
    m_server_process.RequestTermination();
    SetPlayerID(-1);
    SetEmpireID(-1);
    m_ui->GetMapWnd()->Sanitize();

    m_universe.Clear();
    m_empires.Clear();
    m_orders.Reset();
    m_combat_orders.clear();
}

void HumanClientApp::UpdateFPSLimit()
{
    if (GetOptionsDB().Get<bool>("limit-fps")) {
        double fps = GetOptionsDB().Get<double>("max-fps");
        SetMaxFPS(fps);
        Logger().debugStream() << "Limited FPS to " << fps;
    } else {
        SetMaxFPS(0.0); // disable fps limit
        Logger().debugStream() << "Disabled FPS limit";
    }
}

void HumanClientApp::Exit(int code)
{
    if (code)
        Logger().debugStream() << "Initiating Exit (code " << code << " - error termination)";
    if (code)
        exit(code);
    else
#ifdef FREEORION_MACOSX
        // FIXME - terminate is called during the stack unwind if CleanQuit is thrown,
        //  so use exit() for now (this appears to be OS X specific)
        exit(code);
#else
        throw CleanQuit();
#endif
}

HumanClientApp* HumanClientApp::GetApp()
{ return dynamic_cast<HumanClientApp*>(GG::GUI::GetGUI()); }
