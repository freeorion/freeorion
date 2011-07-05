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
#include "../../UI/IntroScreen.h"
#include "../../UI/GalaxySetupWnd.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../UI/ServerConnectWnd.h"
#include "../../UI/Sound.h"
#include "../../network/Message.h"
#include "../../network/Networking.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../util/Process.h"
#include "../../util/Serialize.h"
#include "../../util/SitRepEntry.h"
#include "../../util/Directories.h"
#include "../../universe/Planet.h"
#include "../../universe/Species.h"
#include "../../Empire/Empire.h"

#include <GG/BrowseInfoWnd.h>
#include <GG/Cursor.h>

#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <OgreRoot.h>
#include <OgreVector3.h>
#include <OgreRenderWindow.h>
#include <OgreRenderSystem.h>

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
                               Ogre::Viewport* viewport,
                               const std::string& ois_input_cfg_file_name) :
    ClientApp(),
    OgreGUI(window, ois_input_cfg_file_name),
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

    const std::string LOG_FILENAME((GetUserDir() / "freeorion.log").string());

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

    if ((GetOptionsDB().Get<bool>("UI.sound.music-enabled")))
        Sound::GetSound().PlayMusic(GetOptionsDB().Get<std::string>("UI.sound.bg-music"), -1);

    Sound::GetSound().SetMusicVolume(GetOptionsDB().Get<int>("UI.sound.music-volume"));
    Sound::GetSound().SetUISoundsVolume(GetOptionsDB().Get<int>("UI.sound.volume"));

    EnableFPS();
    UpdateFPSLimit();
    GG::Connect(GetOptionsDB().OptionChangedSignal("show-fps"), &HumanClientApp::UpdateFPSLimit, this);

    boost::shared_ptr<GG::BrowseInfoWnd> default_browse_info_wnd(
        new GG::TextBoxBrowseInfoWnd(GG::X(400), ClientUI::GetFont(),
                                     GG::Clr(0, 0, 0, 200), ClientUI::WndOuterBorderColor(), ClientUI::TextColor(),
                                     GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK, 1));
    GG::Wnd::SetDefaultBrowseInfoWnd(default_browse_info_wnd);

    boost::shared_ptr<GG::Texture> cursor_texture = m_ui->GetTexture(ClientUI::ArtDir() / "cursors" / "default_cursor.png");
    SetCursor(boost::shared_ptr<GG::TextureCursor>(new GG::TextureCursor(cursor_texture, GG::Pt(GG::X(6), GG::Y(3)))));
    RenderCursor(true);


    GG::Connect(WindowResizedSignal,    &HumanClientApp::HandleWindowResize,    this);
    GG::Connect(WindowClosedSignal,     &HumanClientApp::HandleWindowClose,     this);


    unsigned int width, height, c;
    int left, top;
    window->getMetrics(width, height, c, left, top);
    Logger().debugStream() << "HumanClientApp::HumanClientApp window size: " << width << "x" << height << " at " << left << "x" << top;


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
    if (m_networking.Connected())
        m_networking.DisconnectFromServer();
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
    const std::string SERVER_CLIENT_EXE = (GetBinDir() / "freeoriond.exe").string();
#else
    const std::string SERVER_CLIENT_EXE = (GetBinDir() / "freeoriond").string();
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
    m_networking.SetPlayerID(Networking::INVALID_PLAYER_ID);
    m_networking.SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    SetEmpireID(ALL_EMPIRES);
}

void HumanClientApp::KillServer()
{
    m_server_process.Kill();
    m_networking.SetPlayerID(Networking::INVALID_PLAYER_ID);
    m_networking.SetHostPlayerID(Networking::INVALID_PLAYER_ID);
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
        while (!m_networking.ConnectToLocalHostServer()) {
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

            human_player_setup_data.m_save_game_empire_id = ALL_EMPIRES; // not used for new games
            human_player_setup_data.m_client_type = Networking::CLIENT_TYPE_HUMAN_PLAYER;

            // add to setup data players
            setup_data.m_players.push_back(human_player_setup_data);

            // AI player setup data.  One entry for each requested AI

            int num_AIs = GetOptionsDB().Get<int>("GameSetup.ai-players");
            for (int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
                PlayerSetupData ai_setup_data;

                ai_setup_data.m_player_name = "AI_" + boost::lexical_cast<std::string>(ai_i);
                ai_setup_data.m_empire_name.clear();                // leave blank, to be set by server in Universe::GenerateEmpires
                ai_setup_data.m_empire_color = GG::CLR_ZERO;        // to be set by server
                ai_setup_data.m_starting_species_name.clear();      // leave blank, to be set by server
                ai_setup_data.m_save_game_empire_id = ALL_EMPIRES;  // not used for new games
                ai_setup_data.m_client_type = Networking::CLIENT_TYPE_AI_PLAYER;

                setup_data.m_players.push_back(ai_setup_data);
            }

            m_networking.SendMessage(HostSPGameMessage(setup_data));
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

void HumanClientApp::MultiPlayerGame()
{
    if (m_networking.Connected()) {
        Logger().errorStream() << "HumanClientApp::MultiPlayerGame aborting because already connected to a server";
        return;
    }

    ServerConnectWnd server_connect_wnd;
    bool failed = false;
    server_connect_wnd.Run();

    std::string server_name = server_connect_wnd.Result().second;

    if (server_name.empty()) {
        failed = true;
    } else {
        if (server_name == "HOST GAME SELECTED") {
            if (!GetOptionsDB().Get<bool>("force-external-server")) {
                HumanClientApp::GetApp()->StartServer();
                HumanClientApp::GetApp()->FreeServer();
                server_name = "localhost";
            }
            server_name = GetOptionsDB().Get<std::string>("external-server-address");
        }
        unsigned int start_time = Ticks();
        while (!m_networking.ConnectToServer(server_name)) {
            if (SERVER_CONNECT_TIMEOUT < Ticks() - start_time) {
                ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                if (server_connect_wnd.Result().second == "HOST GAME SELECTED")
                    KillServer();
                failed = true;
                break;
            }
        }
    }

    if (!failed) {
        if (server_connect_wnd.Result().second == "HOST GAME SELECTED") {
            m_networking.SendMessage(HostMPGameMessage(server_connect_wnd.Result().first));
            m_fsm->process_event(HostMPGameRequested());
        } else {
            m_networking.SendMessage(JoinGameMessage(server_connect_wnd.Result().first, Networking::CLIENT_TYPE_HUMAN_PLAYER));
            m_fsm->process_event(JoinMPGameRequested());
        }
        m_connected = true;
    }
}

void HumanClientApp::StartMultiPlayerGameFromLobby()
{
    m_fsm->process_event(StartMPGameClicked());
}

void HumanClientApp::CancelMultiplayerGameFromLobby()
{
    m_fsm->process_event(CancelMPGameClicked());
}

void HumanClientApp::SaveGame(const std::string& filename)
{
    Message response_msg;
    m_networking.SendSynchronousMessage(HostSaveGameMessage(PlayerID(), filename), response_msg);
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

            FileDlg dlg(GetSaveDir().string(), "", false, false, save_file_types);
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
        while (!m_networking.ConnectToLocalHostServer()) {
            if (SERVER_CONNECT_TIMEOUT < Ticks() - start_time) {
                ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                KillServer();
                return;
            }
        }

        Logger().debugStream() << "HumanClientApp::LoadSinglePlayerGame() connected to server";

        m_connected = true;
        m_networking.SetPlayerID(Networking::INVALID_PLAYER_ID);
        m_networking.SetHostPlayerID(Networking::INVALID_PLAYER_ID);
        SetEmpireID(ALL_EMPIRES);

        SinglePlayerSetupData setup_data;
        // leving GalaxySetupData information default / blank : not used when loading a game
        setup_data.m_new_game = false;
        setup_data.m_filename = filename;
        // leving setup_data.m_players empty : not specified when loading a game, as server will generate from save file


        m_networking.SendMessage(HostSPGameMessage(setup_data));
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

std::pair<int, int> HumanClientApp::GetWindowWidthHeight(Ogre::RenderSystem* render_system)
{
    int width(800), height(600);
    if (!render_system)
        return std::make_pair(width, height);

    bool fullscreen = GetOptionsDB().Get<bool>("fullscreen");
    if (!fullscreen) {
        width = GetOptionsDB().Get<int>("app-width-windowed");
        height = GetOptionsDB().Get<int>("app-height-windowed");
        return std::make_pair(width, height);
    }

    bool reset_fullscreen = GetOptionsDB().Get<bool>("reset-fullscreen-size");
    if (!reset_fullscreen) {
        width = GetOptionsDB().Get<int>("app-width");
        height = GetOptionsDB().Get<int>("app-height");
        return std::make_pair(width, height);
    }

    GetOptionsDB().Set<bool>("reset-fullscreen-size", false);

    // parse list of available resolutions, pick the largest,
    // which should be the monitor size
    Ogre::StringVector possible_modes;
    Ogre::ConfigOptionMap& renderer_options = render_system->getConfigOptions();
    for (Ogre::ConfigOptionMap::iterator it = renderer_options.begin(); it != renderer_options.end(); ++it) {
        if (it->first != "Video Mode")
            continue;
        possible_modes = it->second.possibleValues;
    }
    // for each most, parse the text and check if it is the biggest
    // yet seen.
    for (Ogre::StringVector::iterator it = possible_modes.begin(); it != possible_modes.end(); ++it) {
        std::istringstream iss(*it);
        char x;
        int cur_width(-1), cur_height(-1);
        iss >> cur_width >> std::ws >> x >> std::ws >> cur_height;

        //std::cout << cur_width << ", " << cur_height << std::endl;

        if (cur_width > width || cur_height > height) {
            width = cur_width;
            height = cur_height;
            GetOptionsDB().Set<int>("app-width", width);
            GetOptionsDB().Set<int>("app-height", height);
        }
    }

    return std::make_pair(width, height);
}

void HumanClientApp::Reinitialize()
{
    Ogre::RenderSystem* render_system = m_root->getRenderSystem();
    if (!render_system)
        return;

    //int colour_depth = GetOptionsDB().Get<int>("color-depth");
    std::pair<unsigned int, unsigned int> width_height = GetWindowWidthHeight(render_system);
    unsigned int width(width_height.first), height(width_height.second);
    bool fullscreen = GetOptionsDB().Get<bool>("fullscreen");

    Ogre::RenderWindow* window = m_root->getAutoCreatedWindow();

    if (fullscreen != window->isFullScreen()) {
        window->setFullscreen(fullscreen, width, height);
    } else if (width != window->getWidth() || height != window->getHeight()) {
        if (fullscreen)
            window->setFullscreen(fullscreen, width, height);
        else
            window->resize(width, height);
    }
}

float HumanClientApp::GLVersion() const
{
    return GetGLVersion();
}

namespace {
    static bool enter_2d_mode_log_done(false);
}

void HumanClientApp::Enter2DMode()
{
    Ogre::RenderWindow* window = m_root->getAutoCreatedWindow();
    unsigned int width, height, c;
    int left, top;
    window->getMetrics(width, height, c, left, top);

    if (!enter_2d_mode_log_done) {
        enter_2d_mode_log_done = true;
        Logger().debugStream() << "HumanClientApp::Enter2DMode()";
    }
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

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // set up coordinates with origin in upper-left and +x and +y directions right and down, respectively
    // the depth of the viewing volume is only 1 (from 0.0 to 1.0)
    glOrtho(0.0, width, height, 0.0, 0.0, width);

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
    if (m_connected && !m_networking.Connected()) {
        m_connected = false;
        DisconnectedFromServer();
    } else if (m_networking.MessageAvailable()) {
        Message msg;
        m_networking.GetMessage(msg);
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
    case Message::ERROR:                m_fsm->process_event(Error(msg));               break;
    case Message::HOST_MP_GAME:         m_fsm->process_event(HostMPGame(msg));          break;
    case Message::HOST_SP_GAME:         m_fsm->process_event(HostSPGame(msg));          break;
    case Message::JOIN_GAME:            m_fsm->process_event(JoinGame(msg));            break;
    case Message::HOST_ID:              m_fsm->process_event(HostID(msg));              break;
    case Message::LOBBY_UPDATE:         m_fsm->process_event(LobbyUpdate(msg));         break;
    case Message::LOBBY_CHAT:           m_fsm->process_event(LobbyChat(msg));           break;
    case Message::SAVE_GAME:            m_fsm->process_event(::SaveGame(msg));          break;
    case Message::GAME_START:           m_fsm->process_event(GameStart(msg));           break;
    case Message::TURN_UPDATE:          m_fsm->process_event(TurnUpdate(msg));          break;
    case Message::TURN_PARTIAL_UPDATE:  m_fsm->process_event(TurnPartialUpdate(msg));   break;
    case Message::TURN_PROGRESS:        m_fsm->process_event(TurnProgress(msg));        break;
    case Message::PLAYER_STATUS:        m_fsm->process_event(PlayerStatus(msg));        break;
    case Message::COMBAT_START:         m_fsm->process_event(CombatStart(msg));         break;
    case Message::COMBAT_TURN_UPDATE:   m_fsm->process_event(CombatRoundUpdate(msg));   break;
    case Message::COMBAT_END:           m_fsm->process_event(CombatEnd(msg));           break;
    case Message::PLAYER_CHAT:          m_fsm->process_event(PlayerChat(msg));          break;
    case Message::VICTORY_DEFEAT :      m_fsm->process_event(VictoryDefeat(msg));       break;
    case Message::PLAYER_ELIMINATED:    m_fsm->process_event(PlayerEliminated(msg));    break;
    case Message::END_GAME:             m_fsm->process_event(::EndGame(msg));           break;
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
    m_networking.SendMessage(ClientSaveDataMessage(PlayerID(), Orders(), ui_data));
}

void HumanClientApp::HandleWindowResize(GG::X w, GG::Y h)
{
    Logger().debugStream() << "HumanClientApp::HandleWindowResize(" << Value(w) << ", " << Value(h) << ")";
    if (ClientUI* ui = ClientUI::GetClientUI()) {
        if (MapWnd* map_wnd = ui->GetMapWnd())
            map_wnd->DoLayout();
        if (IntroScreen* intro_screen = ui->GetIntroScreen()) {
            intro_screen->Resize(GG::Pt(w, h));
            intro_screen->DoLayout();
        }
    }

    // store resize if window is not full-screen (so that fullscreen
    // resolution doesn't overwrite windowed resolution)
    Ogre::RenderWindow* window = m_root->getAutoCreatedWindow();
    if (window && !window->isFullScreen()) {
        GetOptionsDB().Set<int>("app-width-windowed", Value(w));
        GetOptionsDB().Set<int>("app-height-windowed", Value(h));

        // Save the changes:
        {
            boost::filesystem::ofstream ofs(GetConfigPath());
            if (ofs) {
                GetOptionsDB().GetXML().WriteDoc(ofs);
            } else {
                std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
                std::cerr << GetConfigPath().string() << std::endl;
                Logger().errorStream() << UserString("UNABLE_TO_WRITE_CONFIG_XML");
                Logger().errorStream() << GetConfigPath().string();
            }
        }
    }
}

void HumanClientApp::HandleWindowClose()
{
    Logger().debugStream() << "HumanClientApp::HandleWindowClose()";
    EndGame();
    //Exit(0);  // want to call Exit here, to cleanly quit, but doing so doesn't work on Win7
    exit(0);
}

void HumanClientApp::StartGame()
{
    m_game_started = true;
    Orders().Reset();
}

void HumanClientApp::Autosave()
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

    Logger().debugStream() << "Autosaving to: " << (save_dir / save_filename).string();

    try {
        SaveGame((save_dir / save_filename).string());
    } catch (const std::exception& e) {
        Logger().errorStream() << "Autosave failed: " << e.what();
        std::cerr << "Autosave failed: " << e.what() << std::endl;
    }
}

void HumanClientApp::EndGame(bool suppress_FSM_reset)
{
    Logger().debugStream() << "HumanClientApp::EndGame";
    m_game_started = false;
    m_networking.DisconnectFromServer();
    m_server_process.RequestTermination();
    m_networking.SetPlayerID(Networking::INVALID_PLAYER_ID);
    m_networking.SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    SetEmpireID(ALL_EMPIRES);
    m_ui->GetMapWnd()->Sanitize();

    m_universe.Clear();
    m_empires.Clear();
    m_orders.Reset();
    m_combat_orders.clear();

    if (!suppress_FSM_reset)
        m_fsm->process_event(ResetToIntroMenu());
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

void HumanClientApp::DisconnectedFromServer()
{
    Logger().debugStream() << "HumanClientApp::DisconnectedFromServer";
    m_fsm->process_event(Disconnection());
}

void HumanClientApp::Exit(int code)
{
    if (code) {
        Logger().debugStream() << "Initiating Exit (code " << code << " - error termination)";
        exit(code);
    } else {
        Logger().debugStream() << "Initiating Exit (code " << code << " - normal termination)";
#ifdef FREEORION_MACOSX
        // FIXME - terminate is called during the stack unwind if CleanQuit is thrown,
        //  so use exit() for now (this appears to be OS X specific)
        exit(code);
#else
        throw CleanQuit();
#endif
    }
}

HumanClientApp* HumanClientApp::GetApp()
{ return dynamic_cast<HumanClientApp*>(GG::GUI::GetGUI()); }
