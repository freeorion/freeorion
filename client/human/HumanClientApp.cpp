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
    const unsigned int SERVER_CONNECT_TIMEOUT = 30000; // in ms

    const bool INSTRUMENT_MESSAGE_HANDLING = false;

    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("autosave.single-player", "OPTIONS_DB_AUTOSAVE_SINGLE_PLAYER", true, Validator<bool>());
        db.Add("autosave.multiplayer", "OPTIONS_DB_AUTOSAVE_MULTIPLAYER", false, Validator<bool>());
        db.Add("autosave.turns", "OPTIONS_DB_AUTOSAVE_TURNS", 5, RangedValidator<int>(1, 50));
        db.Add("autosave.saves", "OPTIONS_DB_AUTOSAVE_SAVES", 10, RangedValidator<int>(1, 50));
        db.Add("music-volume", "OPTIONS_DB_MUSIC_VOLUME", 255, RangedValidator<int>(1, 255));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

}

HumanClientApp::HumanClientApp(Ogre::Root* root,
                               Ogre::RenderWindow* window,
                               Ogre::SceneManager* scene_manager,
                               Ogre::Camera* camera,
                               Ogre::Viewport* viewport) : 
    ClientApp(), 
    OgreGUI(window, "OISInput.cfg"),
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

    const std::string LOG_FILENAME((GetLocalDir() / "freeorion.log").native_file_string());

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

std::map<int, int> HumanClientApp::PendingColonizationOrders() const
{
    std::map<int, int> retval;
    for (OrderSet::const_iterator it = Orders().begin(); it != Orders().end(); ++it) {
        if (boost::shared_ptr<FleetColonizeOrder> order =
            boost::dynamic_pointer_cast<FleetColonizeOrder>(it->second)) {
            retval[order->PlanetID()] = it->first;
        }
    }
    return retval;
}

void HumanClientApp::StartServer()
{
#ifdef FREEORION_WIN32
    const std::string SERVER_CLIENT_EXE = "freeoriond.exe";
#else
    const std::string SERVER_CLIENT_EXE = (GetBinDir() / "freeoriond").native_file_string();
#endif
    std::vector<std::string> args(1, SERVER_CLIENT_EXE);
    args.push_back("--settings-dir");
    args.push_back("\"" + GetOptionsDB().Get<std::string>("settings-dir") + "\"");
    args.push_back("--log-level");
    args.push_back(GetOptionsDB().Get<std::string>("log-level"));
    m_server_process = Process(SERVER_CLIENT_EXE, args);
}

void HumanClientApp::FreeServer()
{
    m_server_process.Free();
    SetPlayerID(-1);
    SetEmpireID(-1);
    SetPlayerName("");
}

void HumanClientApp::KillServer()
{
    m_server_process.Kill();
    SetPlayerID(-1);
    SetEmpireID(-1);
    SetPlayerName("");
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
            if (quickstart) {
                // get values stored in options from previous time game was run

                setup_data.m_size = GetOptionsDB().Get<int>("GameSetup.stars");
                setup_data.m_shape = GetOptionsDB().Get<Shape>("GameSetup.galaxy-shape");
                setup_data.m_age = GetOptionsDB().Get<Age>("GameSetup.galaxy-age");

                // GalaxySetupWnd doesn't allow LANES_NON, but I'll assume the value in the OptionsDB is valid here
                // since this is quickstart, and should be based on previous acceptable value stored, unless
                // the options file has be corrupted or edited
                setup_data.m_starlane_freq = GetOptionsDB().Get<StarlaneFrequency>("GameSetup.starlane-frequency");

                setup_data.m_planet_density = GetOptionsDB().Get<PlanetDensity>("GameSetup.planet-density");
                setup_data.m_specials_freq = GetOptionsDB().Get<SpecialsFrequency>("GameSetup.specials-frequency");
                setup_data.m_empire_name = GetOptionsDB().Get<std::string>("GameSetup.empire-name");

                // DB stores index into array of available colours, so need to get that array to look up value of index.
                // if stored value is invalid, use a default colour
                const std::vector<GG::Clr>& empire_colours = EmpireColors();
                int colour_index = GetOptionsDB().Get<int>("GameSetup.empire-color");
                if (colour_index >= 0 && colour_index < static_cast<int>(empire_colours.size()))
                    setup_data.m_empire_color = empire_colours[colour_index];
                else
                    setup_data.m_empire_color = GG::CLR_GREEN;

                setup_data.m_AIs = GetOptionsDB().Get<int>("GameSetup.ai-players");

            } else {
                galaxy_wnd.Panel().GetSetupData(setup_data);
                setup_data.m_empire_name = galaxy_wnd.EmpireName();
                setup_data.m_empire_color = galaxy_wnd.EmpireColor();
                setup_data.m_AIs = galaxy_wnd.NumberAIs();
            }
            setup_data.m_new_game = true;
            setup_data.m_host_player_name = SinglePlayerName();
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
            Networking().SendMessage(JoinGameMessage(server_connect_wnd.Result().first));
            m_fsm->process_event(JoinMPGameRequested());
        }
        m_connected = true;
    }
}

void HumanClientApp::SaveGame(const std::string& filename)
{
    Message response_msg;
    Networking().SendSynchronousMessage(HostSaveGameMessage(PlayerID(), filename), response_msg);
    assert(response_msg.Type() == Message::SAVE_GAME);
    HandleSaveGameDataRequest();
}

void HumanClientApp::EndGame()
{ EndGame(false); }

void HumanClientApp::LoadSinglePlayerGame()
{
    std::vector<std::pair<std::string, std::string> > save_file_types;
    save_file_types.push_back(std::pair<std::string, std::string>(UserString("GAME_MENU_SAVE_FILES"), "*.sav"));

    try {
        FileDlg dlg(GetOptionsDB().Get<std::string>("save-dir"), "", false, false, save_file_types);
        dlg.Run();
        if (!dlg.Result().empty()) {
            if (m_game_started)
                EndGame();

            if (!GetOptionsDB().Get<bool>("force-external-server"))
                StartServer();
            unsigned int start_time = Ticks();
            while (!Networking().ConnectToLocalHostServer()) {
                if (SERVER_CONNECT_TIMEOUT < Ticks() - start_time) {
                    ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
                    KillServer();
                    return;
                }
            }

            m_connected = true;
            SetPlayerID(Networking::HOST_PLAYER_ID);
            SetEmpireID(-1);
            SetPlayerName(SinglePlayerName());

            SinglePlayerSetupData setup_data;
            setup_data.m_new_game = false;
            setup_data.m_filename = *dlg.Result().begin();
            setup_data.m_host_player_name = SinglePlayerName();
            Networking().SendMessage(HostSPGameMessage(setup_data));
            m_fsm->process_event(HostSPGameRequested(WAITING_FOR_LOADED_GAME));
        }
    } catch (const FileDlg::BadInitialDirectory& e) {
        ClientUI::MessageBox(e.what(), true);
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

void HumanClientApp::Enter2DMode()
{
    OgreGUI::Enter2DMode();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

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
    case Message::HOST_MP_GAME:          m_fsm->process_event(HostMPGame(msg)); break;
    case Message::HOST_SP_GAME:          m_fsm->process_event(HostSPGame(msg)); break;
    case Message::JOIN_GAME:             m_fsm->process_event(JoinGame(msg)); break;
    case Message::LOBBY_UPDATE:          m_fsm->process_event(LobbyUpdate(msg)); break;
    case Message::LOBBY_CHAT:            m_fsm->process_event(LobbyChat(msg)); break;
    case Message::LOBBY_HOST_ABORT:      m_fsm->process_event(LobbyHostAbort(msg)); break;
    case Message::LOBBY_EXIT:            m_fsm->process_event(LobbyNonHostExit(msg)); break;
    case Message::SAVE_GAME:             m_fsm->process_event(::SaveGame(msg)); break;
    case Message::GAME_START:            m_fsm->process_event(GameStart(msg)); break;
    case Message::TURN_UPDATE:           m_fsm->process_event(TurnUpdate(msg)); break;
    case Message::TURN_PROGRESS:         m_fsm->process_event(TurnProgress(msg)); break;
    case Message::COMBAT_START:          m_fsm->process_event(CombatStart(msg)); break;
    case Message::COMBAT_ROUND_UPDATE:   m_fsm->process_event(CombatRoundUpdate(msg)); break;
    case Message::COMBAT_END:            m_fsm->process_event(CombatEnd(msg)); break;
    case Message::HUMAN_PLAYER_CHAT:     m_fsm->process_event(PlayerChat(msg)); break;
    case Message::VICTORY_DEFEAT :       m_fsm->process_event(VictoryDefeat(msg)); break;
    case Message::PLAYER_ELIMINATED:     m_fsm->process_event(PlayerEliminated(msg)); break;
    case Message::END_GAME:              m_fsm->process_event(::EndGame(msg)); break;
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
    for (Empire::SitRepItr it = Empires().Lookup(EmpireID())->SitRepBegin(); it != Empires().Lookup(EmpireID())->SitRepEnd(); ++it) {
        m_ui->GenerateSitRepText(*it);
    }
}

void HumanClientApp::Autosave(bool new_game)
{
    if (((m_single_player_game && GetOptionsDB().Get<bool>("autosave.single-player")) || 
         (!m_single_player_game && GetOptionsDB().Get<bool>("autosave.multiplayer"))) &&
        (m_turns_since_autosave++ % GetOptionsDB().Get<int>("autosave.turns")) == 0) {
        const char* legal_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
        std::string empire_name = Empires().Lookup(EmpireID())->Name();
        std::string::size_type first_good_empire_char = empire_name.find_first_of(legal_chars);
        if (first_good_empire_char == std::string::npos) {
            empire_name = "";
        } else {
            std::string::size_type first_bad_empire_char = empire_name.find_first_not_of(legal_chars, first_good_empire_char);
            empire_name = empire_name.substr(first_good_empire_char, first_bad_empire_char - first_good_empire_char);
        }

        std::string save_filename;
        if (m_single_player_game) {
            save_filename = boost::io::str(boost::format("AS_%s_%04d.sav") % empire_name % CurrentTurn());
        } else {
            std::string::size_type first_good_player_char = PlayerName().find_first_of(legal_chars);
            if (first_good_player_char == std::string::npos) {
                save_filename = boost::io::str(boost::format("AS_%s_%04d.mps") % empire_name % CurrentTurn());
            } else {
                std::string::size_type first_bad_player_char = PlayerName().find_first_not_of(legal_chars, first_good_player_char);
                std::string player_name = PlayerName().substr(first_good_player_char, first_bad_player_char - first_good_player_char);
                save_filename = boost::io::str(boost::format("AS_%s_%s_%04d.mps") % player_name % empire_name % CurrentTurn());
            }
        }

        std::set<std::string> similar_save_files;
        std::set<std::string> old_save_files;
        std::string extension = m_single_player_game ? ".sav" : ".mps";
        namespace fs = boost::filesystem;
        fs::path save_dir(GetOptionsDB().Get<std::string>("save-dir"));
        fs::directory_iterator end_it;
        for (fs::directory_iterator it(save_dir); it != end_it; ++it) {
            if (!fs::is_directory(*it)) {
                std::string filename = it->filename();
                if (!new_game &&
                    filename.find(extension) == filename.size() - extension.size() && 
                    filename.find(save_filename.substr(0, save_filename.size() - 7)) == 0) {
                    similar_save_files.insert(filename);
                } else if (filename.find("AS_") == 0) {
                    // this simple condition means that at the beginning of an autosave run, we'll clear out all old autosave files,
                    // even if they don't match the current empire name, or if they are MP vs. SP games, or whatever
                    old_save_files.insert(filename);
                }
            }
        }

        for (std::set<std::string>::iterator it = old_save_files.begin(); it != old_save_files.end(); ++it) {
            fs::remove(save_dir / *it);
        }

        unsigned int max_autosaves = GetOptionsDB().Get<int>("autosave.saves");
        std::set<std::string>::reverse_iterator rit = similar_save_files.rbegin();
        std::advance(rit, std::min(similar_save_files.size(), (size_t)(max_autosaves - 1)));
        for (; rit != similar_save_files.rend(); ++rit) {
            fs::remove(save_dir / *rit);
        }

        SaveGame((save_dir / save_filename).native_file_string());
    }
}

void HumanClientApp::EndGame(bool suppress_FSM_reset)
{
    if (!suppress_FSM_reset)
        m_fsm->process_event(ResetToIntroMenu());
    m_game_started = false;
    Networking().DisconnectFromServer();
    m_server_process.RequestTermination();
    SetPlayerID(-1);
    SetEmpireID(-1);
    SetPlayerName("");
    m_ui->GetMapWnd()->Sanitize();
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
        throw CleanQuit();
}

HumanClientApp* HumanClientApp::GetApp()
{ return dynamic_cast<HumanClientApp*>(GG::GUI::GetGUI()); }
