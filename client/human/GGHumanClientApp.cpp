#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "GGHumanClientApp.h"

#include "HumanClientFSM.h"
#include "../../UI/ChatWnd.h"
#include "../../UI/CUIControls.h"
#include "../../UI/CUIStyle.h"
#include "../../UI/MapWnd.h"
#include "../../UI/DesignWnd.h"
#include "../../UI/Hotkeys.h"
#include "../../UI/IntroScreen.h"
#include "../../UI/GalaxySetupWnd.h"
#include "../../UI/MultiplayerLobbyWnd.h"
#include "../../UI/SaveFileDialog.h"
#include "../../UI/ServerConnectWnd.h"
#include "../../UI/Sound.h"
#include "../../network/Message.h"
#include "../ClientNetworking.h"
#include "../../util/i18n.h"
#include "../../util/LoggerWithOptionsDB.h"
#include "../../util/GameRules.h"
#include "../../util/OptionsDB.h"
#include "../../util/Process.h"
#include "../../util/PythonCommon.h"
#include "../../util/SaveGamePreviewUtils.h"
#include "../../util/SitRepEntry.h"
#include "../../util/Directories.h"
#include "../../util/Version.h"
#include "../../util/ScopedTimer.h"
#include "../../universe/Planet.h"
#include "../../universe/Species.h"
#include "../../Empire/Empire.h"
#include "../../combat/CombatLogManager.h"
#include "../../parse/Parse.h"
#include "../../parse/PythonParser.h"

#include <GG/BrowseInfoWnd.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/Cursor.h>
#include <GG/utf8/checked.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>

#include <chrono>
#include <thread>
#include <sstream>
#include <utility>
#if !defined(__cpp_lib_integer_comparison_functions)
namespace std {
    inline auto cmp_less(auto&& lhs, auto&& rhs) { return lhs < rhs; }
}
#endif


namespace fs = boost::filesystem;

#ifdef ENABLE_CRASH_BACKTRACE
# include <signal.h>
# include <execinfo.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>

void SigHandler(int sig) {
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
    constexpr bool INSTRUMENT_MESSAGE_HANDLING = false;

    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add("save.auto.turn.start.enabled",              UserStringNop("OPTIONS_DB_AUTOSAVE_SINGLE_PLAYER_TURN_START"),  true,               Validator<bool>());
        db.Add("save.auto.turn.end.enabled",                UserStringNop("OPTIONS_DB_AUTOSAVE_SINGLE_PLAYER_TURN_END"),    false,              Validator<bool>());
        db.Add("save.auto.turn.multiplayer.start.enabled",  UserStringNop("OPTIONS_DB_AUTOSAVE_MULTIPLAYER_TURN_START"),    true,               Validator<bool>());
        db.Add("save.auto.turn.interval",                   UserStringNop("OPTIONS_DB_AUTOSAVE_TURNS"),                     1,                  RangedValidator<int>(1, 50));
        db.Add("save.auto.file.limit",                      UserStringNop("OPTIONS_DB_AUTOSAVE_LIMIT"),                     10,                 RangedValidator<int>(1, 10000));
        db.Add("save.auto.initial.enabled",                 UserStringNop("OPTIONS_DB_AUTOSAVE_GALAXY_CREATION"),           true,               Validator<bool>());
        db.Add("ui.input.mouse.button.swap.enabled",        UserStringNop("OPTIONS_DB_UI_MOUSE_LR_SWAP"),                   false);
        db.Add("ui.input.keyboard.repeat.delay",            UserStringNop("OPTIONS_DB_KEYPRESS_REPEAT_DELAY"),              360,                RangedValidator<int>(0, 1000));
        db.Add("ui.input.keyboard.repeat.interval",         UserStringNop("OPTIONS_DB_KEYPRESS_REPEAT_INTERVAL"),           20,                 RangedValidator<int>(0, 1000));
        db.Add("ui.input.mouse.button.repeat.delay",        UserStringNop("OPTIONS_DB_MOUSE_REPEAT_DELAY"),                 360,                RangedValidator<int>(0, 1000));
        db.Add("ui.input.mouse.button.repeat.interval",     UserStringNop("OPTIONS_DB_MOUSE_REPEAT_INTERVAL"),              15,                 RangedValidator<int>(0, 1000));
        db.Add("ui.map.messages.timestamp.shown",           UserStringNop("OPTIONS_DB_DISPLAY_TIMESTAMP"),                  true,               Validator<bool>());

        Hotkey::AddHotkey("exit",                           UserStringNop("HOTKEY_EXIT"),                                   GG::Key::GGK_NONE,  GG::MOD_KEY_NONE);
        Hotkey::AddHotkey("quit",                           UserStringNop("HOTKEY_QUIT"),                                   GG::Key::GGK_NONE,  GG::MOD_KEY_NONE);
        Hotkey::AddHotkey("video.fullscreen",               UserStringNop("HOTKEY_FULLSCREEN"),                             GG::Key::GGK_RETURN,GG::MOD_KEY_ALT);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /** These options can only be validated after the graphics system (SDL) is initialized,
        so that display size can be detected
     */
    constexpr int DEFAULT_WIDTH = 1280;
    constexpr int DEFAULT_HEIGHT = 800;
    constexpr int DEFAULT_LEFT = static_cast<int>(SDL_WINDOWPOS_CENTERED);
    constexpr int DEFAULT_TOP = 50;
    constexpr int MIN_WIDTH = 800;
    constexpr int MIN_HEIGHT = 600;

    /** Sets the default and current values for the string option @p option_name to @p option_value if initially empty */
    void SetEmptyStringDefaultOption(std::string_view option_name, std::string option_value) {
        OptionsDB& db = GetOptionsDB();
        if (db.Get<std::string>(option_name).empty()) {
            db.SetDefault(option_name, option_value);
            db.Set(option_name, std::move(option_value));
        }
    }

    /* Sets the value of options that need language-dependent default values.*/
    void SetStringtableDependentOptionDefaults() {
        SetEmptyStringDefaultOption("setup.empire.name", UserString("DEFAULT_EMPIRE_NAME"));
        std::string player_name = UserString("DEFAULT_PLAYER_NAME");
        SetEmptyStringDefaultOption("setup.player.name", player_name);
        SetEmptyStringDefaultOption("setup.multiplayer.player.name", player_name);
    }

    std::string GetGLVersionString() {
        std::array<std::string::value_type, 64> buff{};
        auto* v = glGetString(GL_VERSION);
        for (auto buff_it = buff.begin(); v && *v && buff_it != buff.end();)
            *buff_it++ = *v++;
        return buff.data();
    }


    float GetGLVersion() {
        static float stored_gl_version = -1.0f; // to be replaced when gl version first checked
        if (stored_gl_version != -1.0f)
            return stored_gl_version;

        // get OpenGL version string and parse to get version number
        std::string gl_version_string = GetGLVersionString();

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
        DebugLogger() << "OpenGL Version Number: " << DoubleToString(version_number, 2, false);
        if (version_number < 2.0) {
            ErrorLogger() << "OpenGL Version is less than 2.0. FreeOrion may crash when trying to start a game.";
        }

        // only execute default option setting once
        if (GetOptionsDB().Get<bool>("version.gl.check.done"))
            return;
        GetOptionsDB().Set<bool>("version.gl.check.done", true);

        // if GL version is too low, set various map rendering options to
        // disabled, to hopefully improve frame rate.
        if (version_number < 2.0) {
            GetOptionsDB().Set<bool>("ui.map.background.gas.shown", false);
            GetOptionsDB().Set<bool>("ui.map.background.starfields.shown", false);
            GetOptionsDB().Set<bool>("ui.map.scanlines.shown", false);
        }
    }
}

void GGHumanClientApp::AddWindowSizeOptionsAfterMainStart(OptionsDB& db) {
    const int max_width_plus_one = GGHumanClientApp::MaximumPossibleWidth() + 1;
    const int max_height_plus_one = GGHumanClientApp::MaximumPossibleHeight() + 1;

    db.Add("video.fullscreen.width", UserStringNop("OPTIONS_DB_APP_WIDTH"),             DEFAULT_WIDTH,  RangedValidator<int>(MIN_WIDTH, max_width_plus_one));
    db.Add("video.fullscreen.height", UserStringNop("OPTIONS_DB_APP_HEIGHT"),           DEFAULT_HEIGHT, RangedValidator<int>(MIN_HEIGHT, max_height_plus_one));
    db.Add("video.windowed.width", UserStringNop("OPTIONS_DB_APP_WIDTH_WINDOWED"),      DEFAULT_WIDTH,  RangedValidator<int>(MIN_WIDTH, max_width_plus_one));
    db.Add("video.windowed.height", UserStringNop("OPTIONS_DB_APP_HEIGHT_WINDOWED"),    DEFAULT_HEIGHT, RangedValidator<int>(MIN_HEIGHT, max_height_plus_one));
    db.Add("video.windowed.left", UserStringNop("OPTIONS_DB_APP_LEFT_WINDOWED"),        DEFAULT_LEFT,   OrValidator<int>( RangedValidator<int>(-max_width_plus_one, max_width_plus_one), DiscreteValidator<int>(DEFAULT_LEFT) ));
    db.Add("video.windowed.top", UserStringNop("OPTIONS_DB_APP_TOP_WINDOWED"),          DEFAULT_TOP,    RangedValidator<int>(-max_height_plus_one, max_height_plus_one));
}

std::string GGHumanClientApp::EncodeServerAddressOption(const std::string& server) {
    std::string server_encoded = boost::replace_all_copy(server, ".", "_");
    boost::replace_all(server_encoded, ":", "_");
    return "network.known-servers._" + server_encoded;
}

GGHumanClientApp::GGHumanClientApp(int width, int height, bool calculate_fps, std::string name,
                                   int x, int y, bool fullscreen, bool fake_mode_change) :
    ClientApp(),
    SDLGUI(width, height, calculate_fps, std::move(name), x, y, fullscreen, fake_mode_change),
    m_fsm(*this)
{
#ifdef ENABLE_CRASH_BACKTRACE
    signal(SIGSEGV, SigHandler);
#endif
#ifdef FREEORION_MACOSX
    SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");
#endif

    // Force the log file if requested.
    if (GetOptionsDB().Get<std::string>("log-file").empty()) {
        const std::string HUMAN_CLIENT_LOG_FILENAME(PathToString(GetUserDataDir() / "freeorion.log"));
        GetOptionsDB().Set("log-file", HUMAN_CLIENT_LOG_FILENAME);
    }
    // Force the log threshold if requested.
    auto force_log_level = GetOptionsDB().Get<std::string>("log-level");
    if (!force_log_level.empty())
        OverrideAllLoggersThresholds(to_LogLevel(force_log_level));

    InitLoggingSystem(GetOptionsDB().Get<std::string>("log-file"), "Client");
    InitLoggingOptionsDBSystem();

    // Force loggers to always appear in the config.xml and OptionsWnd even before their
    // initialization on first use.
    // This is not needed for the loggers to work correctly.
    // this is not needed for the loggers to automatically be added to the config.xml on
    // first use.
    // This only needs to be done in one of the executables connected to the same config.xml
    RegisterLoggerWithOptionsDB("ai", true);
    RegisterLoggerWithOptionsDB("client", true);
    RegisterLoggerWithOptionsDB("server", true);
    RegisterLoggerWithOptionsDB("combat_log");
    RegisterLoggerWithOptionsDB("combat");
    RegisterLoggerWithOptionsDB("supply");
    RegisterLoggerWithOptionsDB("effects");
    RegisterLoggerWithOptionsDB("conditions");
    RegisterLoggerWithOptionsDB("FSM");
    RegisterLoggerWithOptionsDB("network");
    RegisterLoggerWithOptionsDB("parsing");
    RegisterLoggerWithOptionsDB("python");
    RegisterLoggerWithOptionsDB("timer");
    RegisterLoggerWithOptionsDB("IDallocator");

    InfoLogger() << FreeOrionVersionString();

    try {
        InfoLogger() << "GL Version String: " << GetGLVersionString();
    } catch (...) {
        ErrorLogger() << "Unable to get GL Version String?";
    }

    LogDependencyVersions();

    float version_number = GetGLVersion();
    if (version_number < 2.0f) {
        ErrorLogger() << "OpenGL version is less than 2; FreeOrion will likely crash while starting up...";
        if (!GetOptionsDB().Get<bool>("version.gl.check.done")) {
            auto mb_result = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, UserString("OPENGL_VERSION_LOW_TITLE").c_str(),
                                                      UserString("OPENGL_VERSION_LOW_TEXT").c_str(), nullptr);
            if (mb_result)
                std::cerr << "OpenGL version is less than 2; FreeOrion will likely crash during initialization";
        }
    } else if (version_number < 2.1f) {
        ErrorLogger() << "OpenGL version is less than 2.1; FreeOrion may crash during initialization";
    }

    SetStyleFactory(std::make_unique<CUIStyle>());

    SetMinDragTime(0);

    bool inform_user_sound_failed(false);
    try {
        if (GetOptionsDB().Get<bool>("audio.effects.enabled") || GetOptionsDB().Get<bool>("audio.music.enabled"))
            Sound::GetSound().Enable();

        if ((GetOptionsDB().Get<bool>("audio.music.enabled")))
            Sound::GetSound().PlayMusic(GetOptionsDB().Get<std::string>("audio.music.path"), -1);

        Sound::GetSound().SetMusicVolume(GetOptionsDB().Get<int>("audio.music.volume"));
        Sound::GetSound().SetUISoundsVolume(GetOptionsDB().Get<int>("audio.effects.volume"));
    } catch (const Sound::InitializationFailureException&) {
        inform_user_sound_failed = true;
    }

    m_ui = std::make_unique<ClientUI>();

    EnableFPS();
    UpdateFPSLimit();
    GetOptionsDB().OptionChangedSignal("video.fps.shown").connect(
        boost::bind(&GGHumanClientApp::UpdateFPSLimit, this));
    GetOptionsDB().OptionChangedSignal("video.fps.max").connect(
        boost::bind(&GGHumanClientApp::UpdateFPSLimit, this));

    auto default_browse_info_wnd{
        GG::Wnd::Create<GG::TextBoxBrowseInfoWnd>(
            GG::X(400), ClientUI::GetFont(),
            GG::Clr(0, 0, 0, 200), ClientUI::WndOuterBorderColor(), ClientUI::TextColor(),
            GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK, 1)};
    GG::Wnd::SetDefaultBrowseInfoWnd(std::move(default_browse_info_wnd));

    auto cursor_texture = m_ui->GetTexture(ClientUI::ArtDir() / "cursors" / "default_cursor.png");
    SetCursor(std::make_unique<GG::TextureCursor>(std::move(cursor_texture),
                                                  GG::Pt(GG::X(6), GG::Y(3))));
    RenderCursor(true);

    EnableKeyPressRepeat(GetOptionsDB().Get<int>("ui.input.keyboard.repeat.delay"),
                         GetOptionsDB().Get<int>("ui.input.keyboard.repeat.interval"));
    EnableMouseButtonDownRepeat(GetOptionsDB().Get<int>("ui.input.mouse.button.repeat.delay"),
                                GetOptionsDB().Get<int>("ui.input.mouse.button.repeat.interval"));
    EnableModalAcceleratorSignals(true);

    namespace ph = boost::placeholders;

    WindowResizedSignal.connect(boost::bind(&GGHumanClientApp::HandleWindowResize,this, ph::_1, ph::_2));
    FocusChangedSignal.connect( boost::bind(&GGHumanClientApp::HandleFocusChange, this, ph::_1));
    WindowMovedSignal.connect(  boost::bind(&GGHumanClientApp::HandleWindowMove,  this, ph::_1, ph::_2));
    WindowClosingSignal.connect(boost::bind(&GGHumanClientApp::HandleAppQuitting, this));
    AppQuittingSignal.connect(  boost::bind(&GGHumanClientApp::HandleAppQuitting, this));

    SetStringtableDependentOptionDefaults();
    SetGLVersionDependentOptionDefaults();

    this->SetMouseLRSwapped(GetOptionsDB().Get<bool>("ui.input.mouse.button.swap.enabled"));

    ConnectKeyboardAcceleratorSignals();

    m_auto_turns = GetOptionsDB().Get<int>("auto-advance-n-turns");

    if (fake_mode_change && !FramebuffersAvailable()) {
        ErrorLogger() << "Requested fake mode changes, but the framebuffer opengl extension is not available. Ignoring.";
    }

    // Placed after mouse initialization.
    if (inform_user_sound_failed)
        ClientUI::MessageBox(UserString("ERROR_SOUND_INITIALIZATION_FAILED"), false);

    // Register LinkText tags with GG::Font
    RegisterLinkTags();

    m_fsm.initiate();

    // Start parsing content
    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    std::thread background([this](auto b){
        DebugLogger() << "Started background parser thread";
        PythonCommon python;
        python.Initialize();
        StartBackgroundParsing(PythonParser(python, GetResourceDir() / "scripting"), std::move(b));
    }, std::move(barrier));
    background.detach();
    barrier_future.wait();
    GetOptionsDB().OptionChangedSignal("resource.path").connect(
        boost::bind(&GGHumanClientApp::HandleResoureDirChange, this));
}

void GGHumanClientApp::ConnectKeyboardAcceleratorSignals() {
    // Add global hotkeys
    auto& hkm = HotkeyManager::GetManager();

    hkm.Connect(boost::bind(&GGHumanClientApp::HandleHotkeyExitApp, this), "exit",
                NoModalWndsOpenCondition);
    hkm.Connect(boost::bind(&GGHumanClientApp::HandleHotkeyResetGame, this), "quit",
                NoModalWndsOpenCondition);
    hkm.Connect(boost::bind(&GGHumanClientApp::ToggleFullscreen, this), "video.fullscreen",
                NoModalWndsOpenCondition);

    hkm.RebuildShortcuts();
}

GGHumanClientApp::~GGHumanClientApp() {
    m_networking->DisconnectFromServer();
    m_server_process.RequestTermination();
    DebugLogger() << "GGHumanClientApp exited cleanly.";
}

bool GGHumanClientApp::SinglePlayerGame() const
{ return m_single_player_game; }

bool GGHumanClientApp::CanSaveNow() const {
    // only host can save in multiplayer
    if (!SinglePlayerGame() && !Networking().PlayerIsHost(PlayerID()))
        return false;

    // can't save while AIs are playing their turns...
    for (const auto& [id, empire] : m_empires) {
        if (GetEmpireClientType(id) != Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            continue;   // only care about AIs

        if (!empire->Ready()) {
            return false;
        }
    }

    return true;
}

void GGHumanClientApp::SetSinglePlayerGame(bool sp)
{ m_single_player_game = sp; }

#ifdef FREEORION_MACOSX
#include <stdlib.h>
#endif

namespace {
    class LocalServerAlreadyRunningException : public std::runtime_error {
    public:
        LocalServerAlreadyRunningException() :
            std::runtime_error("LOCAL_SERVER_ALREADY_RUNNING_ERROR")
        {}
    };

    void ClearPreviousPendingSaves(std::queue<std::string>& pending_saves) {
        if (pending_saves.empty())
            return;
        WarnLogger() << "Clearing " << std::to_string(pending_saves.size()) << " pending save game request(s)";
        std::queue<std::string>().swap(pending_saves);
    }
}

void GGHumanClientApp::StartServer() {
    if (m_networking->PingLocalHostServer(std::chrono::milliseconds(100))) {
        ErrorLogger() << "Can't start local server because a server is already connecting at 127.0.0.0.";
        throw LocalServerAlreadyRunningException();
    }

    std::string SERVER_CLIENT_EXE = GetOptionsDB().Get<std::string>("misc.server-local-binary.path");
    DebugLogger() << "GGHumanClientApp::StartServer: " << SERVER_CLIENT_EXE;

#ifdef FREEORION_MACOSX
    // On OSX set environment variable DYLD_LIBRARY_PATH to python framework folder
    // bundled with app, so the dynamic linker uses the bundled python library.
    // Otherwise the dynamic linker will look for a correct python lib in system
    // paths, and if it can't find it, throw an error and terminate!
    // Setting environment variable here, spawned child processes will inherit it.
    setenv("DYLD_LIBRARY_PATH", GetPythonHome().string().c_str(), 1);
#endif

    std::vector<std::string> args;
    std::string ai_config = GetOptionsDB().Get<std::string>("ai-config");
    std::string ai_path = GetOptionsDB().Get<std::string>("ai-path");
    args.push_back("\"" + SERVER_CLIENT_EXE + "\"");
    args.push_back("--resource.path");
    args.push_back("\"" + GetOptionsDB().Get<std::string>("resource.path") + "\"");

    auto force_log_level = GetOptionsDB().Get<std::string>("log-level");
    if (!force_log_level.empty()) {
        args.push_back("--log-level");
        args.push_back(GetOptionsDB().Get<std::string>("log-level"));
    }

    if (ai_path != GetOptionsDB().GetDefaultValueString("ai-path")) {
        args.push_back("--ai-path");
        args.push_back(ai_path);
        DebugLogger() << "ai-path set to '" << ai_path << "'";
    }
    if (!ai_config.empty()) {
        args.push_back("--ai-config");
        args.push_back(ai_config);
        DebugLogger() << "ai-config set to '" << ai_config << "'";
    } else {
        DebugLogger() << "ai-config not set.";
    }
    if (m_single_player_game) {
        args.push_back("--singleplayer");
        args.push_back("--skip-checksum");
    }
    DebugLogger() << "Launching server process with args: ";
    for (const auto& arg : args)
        DebugLogger() << arg;
    m_server_process = Process(SERVER_CLIENT_EXE, args);
    DebugLogger() << "... finished launching server process.";
}

void GGHumanClientApp::FreeServer() {
    m_server_process.Free();
    m_networking->SetPlayerID(Networking::INVALID_PLAYER_ID);
    m_networking->SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    SetEmpireID(ALL_EMPIRES);
}

void GGHumanClientApp::NewSinglePlayerGame(bool quickstart) {
    TraceLogger() << "GGHumanClientApp::NewSinglePlayerGame start";
    ClearPreviousPendingSaves(m_game_saves_in_progress);

    if (!GetOptionsDB().Get<bool>("network.server.external.force")) {
        m_single_player_game = true;
        try {
            StartServer();
        } catch (const LocalServerAlreadyRunningException&) {
            ClientUI::MessageBox(UserString("LOCAL_SERVER_ALREADY_RUNNING_ERROR"), true);
            return;
        } catch (const std::runtime_error& err) {
            ErrorLogger() << "GGHumanClientApp::NewSinglePlayerGame : Couldn't start server.  Got error message: " << err.what();
            ClientUI::MessageBox(UserString("SERVER_WONT_START"), true);
            return;
        }
    }

    bool ended_with_ok = false;
    auto game_rules = GetGameRules().GetRulesAsStrings();
    if (!quickstart) {
        DebugLogger() << "Initializing galaxy setup window";
        auto galaxy_wnd = GG::Wnd::Create<GalaxySetupWnd>();
        TraceLogger() << "Running galaxy setup window";
        galaxy_wnd->Run();
        ended_with_ok = galaxy_wnd->EndedWithOk();
        TraceLogger() << "Setup ran, " << (ended_with_ok ? "ended with OK" : "ended without OK");
        if (ended_with_ok)
            game_rules = galaxy_wnd->GetRulesAsStrings();
        TraceLogger() << "Got rules as strings";
    }


    m_connected = m_networking->ConnectToLocalHostServer();
    if (!m_connected) {
        DebugLogger() << "Not connected; returning to intro screen and showing timed out error";
        ResetToIntro(true);
        ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
        return;
    }

    if (!(quickstart || ended_with_ok)) {
        ErrorLogger() << "GGHumanClientApp::NewSinglePlayerGame failed to start new game, killing server.";
        ResetToIntro(true);
    }

    SinglePlayerSetupData setup_data;
    setup_data.new_game = true;
    setup_data.filename.clear();  // not used for new game

    // get values stored in options from previous time game was run or
    // from just having run GalaxySetupWnd

    // GalaxySetupData
    setup_data.SetSeed(GetOptionsDB().Get<std::string>("setup.seed"));
    setup_data.size =             GetOptionsDB().Get<int>("setup.star.count");
    setup_data.shape =            GetOptionsDB().Get<Shape>("setup.galaxy.shape");
    setup_data.age =              GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.galaxy.age");
    setup_data.starlane_freq =    GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.starlane.frequency");
    setup_data.planet_density =   GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.planet.density");
    setup_data.specials_freq =    GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.specials.frequency");
    setup_data.monster_freq =     GetOptionsDB().Get<GalaxySetupOptionMonsterFreq>("setup.monster.frequency");
    setup_data.native_freq =      GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.native.frequency");
    setup_data.ai_aggr =          GetOptionsDB().Get<Aggression>("setup.ai.aggression");
    setup_data.game_rules =       game_rules;


    // SinglePlayerSetupData contains a map of PlayerSetupData, for
    // the human and AI players.  Need to compile this information
    // from the specified human options and number of requested AIs

    // Human player setup data
    PlayerSetupData human_player_setup_data;
    human_player_setup_data.player_name = GetOptionsDB().Get<std::string>("setup.player.name");
    human_player_setup_data.empire_name = GetOptionsDB().Get<std::string>("setup.empire.name");

    // DB stores index into array of available colours, so need to get that array to look up value of index.
    // if stored value is invalid, use a default colour
    const std::vector<EmpireColor>& empire_colours = EmpireColors();
    int colour_index = GetOptionsDB().Get<int>("setup.empire.color.index");
    if (colour_index >= 0 && std::cmp_less(colour_index, empire_colours.size()))
        human_player_setup_data.empire_color = empire_colours[colour_index];
    else
        human_player_setup_data.empire_color = GG::CLR_GREEN.RGBA();

    human_player_setup_data.starting_species_name = GetOptionsDB().Get<std::string>("setup.initial.species");
    if (human_player_setup_data.starting_species_name == "1")
        human_player_setup_data.starting_species_name = "SP_HUMAN";   // kludge / bug workaround for bug with options storage and retreival.  Empty-string options are stored, but read in as "true" boolean, and converted to string equal to "1"

    if (human_player_setup_data.starting_species_name != "RANDOM" &&
        !m_species_manager.GetSpecies(human_player_setup_data.starting_species_name))
    {
        if (m_species_manager.NumPlayableSpecies() < 1)
            human_player_setup_data.starting_species_name.clear();
        else
            human_player_setup_data.starting_species_name = m_species_manager.playable_begin()->first;
    }

    human_player_setup_data.save_game_empire_id = ALL_EMPIRES; // not used for new games
    human_player_setup_data.client_type = Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER;

    // add to setup data players
    setup_data.players.push_back(std::move(human_player_setup_data));


    // AI player setup data.  One entry for each requested AI
    int num_AIs = GetOptionsDB().Get<int>("setup.ai.player.count");
    for (int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
        PlayerSetupData ai_setup_data;

        ai_setup_data.player_name = "AI_" + std::to_string(ai_i);
        ai_setup_data.empire_name.clear();                // leave blank, to be set by server in Universe::GenerateEmpires
        ai_setup_data.starting_species_name.clear();      // leave blank, to be set by server
        ai_setup_data.save_game_empire_id = ALL_EMPIRES;  // not used for new games
        ai_setup_data.client_type = Networking::ClientType::CLIENT_TYPE_AI_PLAYER;

        setup_data.players.push_back(ai_setup_data);
    }


    TraceLogger() << "Sending host SP setup message";
    m_networking->SendMessage(HostSPGameMessage(setup_data, DependencyVersions()));
    m_fsm.process_event(HostSPGameRequested());
    TraceLogger() << "GGHumanClientApp::NewSinglePlayerGame done";
}

void GGHumanClientApp::MultiPlayerGame() {
    ClearPreviousPendingSaves(m_game_saves_in_progress);

    if (m_networking->IsConnected()) {
        ErrorLogger() << "GGHumanClientApp::MultiPlayerGame aborting because already connected to a server";
        return;
    }

    auto server_connect_wnd = GG::Wnd::Create<ServerConnectWnd>();
    server_connect_wnd->Run();

    std::string server_dest = server_connect_wnd->GetResult().server_dest;

    if (server_dest.empty())
        return;

    if (server_dest == "HOST GAME SELECTED") {
        if (!GetOptionsDB().Get<bool>("network.server.external.force")) {
            m_single_player_game = false;
            try {
                StartServer();
                FreeServer();
            } catch (const LocalServerAlreadyRunningException&) {
                ClientUI::MessageBox(UserString("LOCAL_SERVER_ALREADY_RUNNING_ERROR"), true);
                return;
            } catch (const std::runtime_error& err) {
                ErrorLogger() << "Couldn't start server.  Got error message: " << err.what();
                ClientUI::MessageBox(UserString("SERVER_WONT_START"), true);
                return;
            }
            server_dest = "localhost";
        }
        server_dest = GetOptionsDB().Get<std::string>("network.server.uri");
    }

    m_connected = m_networking->ConnectToServer(server_dest);
    if (!m_connected) {
        ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
        if (server_connect_wnd->GetResult().server_dest == "HOST GAME SELECTED")
            ResetToIntro(true);
        return;
    }

    if (server_connect_wnd->GetResult().server_dest == "HOST GAME SELECTED") {
        m_networking->SendMessage(HostMPGameMessage(server_connect_wnd->GetResult().player_name, DependencyVersions()));
        m_fsm.process_event(HostMPGameRequested());
    } else {
        boost::uuids::uuid cookie = boost::uuids::nil_uuid();
        try {
            const std::string cookie_option = EncodeServerAddressOption(server_dest);
            if (!GetOptionsDB().OptionExists(cookie_option + ".cookie"))
                GetOptionsDB().Add<std::string>(cookie_option + ".cookie", "OPTIONS_DB_SERVER_COOKIE", boost::uuids::to_string(cookie));
            if (!GetOptionsDB().OptionExists(cookie_option + ".address"))
                GetOptionsDB().Add<std::string>(cookie_option + ".address", "OPTIONS_DB_SERVER_COOKIE", "");
            GetOptionsDB().Set(cookie_option + ".address", server_dest);
            const std::string cookie_str = GetOptionsDB().Get<std::string>(cookie_option + ".cookie");
            static constexpr boost::uuids::string_generator gen{};
            cookie = gen(cookie_str);

        } catch(const std::exception& err) {
            WarnLogger() << "Cann't get cookie for server " << server_dest << ". Get error message"
                         << err.what();
            // ignore
        }

        m_networking->SendMessage(JoinGameMessage(server_connect_wnd->GetResult().player_name,
                                                  server_connect_wnd->GetResult().type,
                                                  DependencyVersions(),
                                                  cookie));
        m_fsm.process_event(JoinMPGameRequested());
    }
}

void GGHumanClientApp::StartMultiPlayerGameFromLobby()
{ m_fsm.process_event(StartMPGameClicked()); }

void GGHumanClientApp::CancelMultiplayerGameFromLobby()
{ m_fsm.process_event(CancelMPGameClicked()); }

void GGHumanClientApp::SaveGame(const std::string& filename) {
    m_game_saves_in_progress.push(filename);

    // Start a save if there is not one in progress
    if (m_game_saves_in_progress.size() > 1) {
        DebugLogger() << "Add pending save to queue.";
        return;
    }

    m_networking->SendMessage(HostSaveGameInitiateMessage(filename));
    DebugLogger() << "Sent save initiate message to server.";
}

void GGHumanClientApp::SaveGameCompleted() {
    if (!m_game_saves_in_progress.empty())
        m_game_saves_in_progress.pop();

    // Either indicate that all saves are completed or start the next save.
    // Autosaves and player saves can be concurrent.
    if (m_game_saves_in_progress.empty()) {
        DebugLogger() << "Save games completed.";
        SaveGamesCompletedSignal();
    } else {
        m_networking->SendMessage(HostSaveGameInitiateMessage(m_game_saves_in_progress.front()));
        DebugLogger() << "Sent next save initiate message to server.";
    }
}

void GGHumanClientApp::LoadSinglePlayerGame(std::string filename) {
    DebugLogger() << "GGHumanClientApp::LoadSinglePlayerGame";

    if (!filename.empty()) {
        if (!exists(FilenameToPath(filename))) {
            std::string msg = "GGHumanClientApp::LoadSinglePlayerGame() given a nonexistent file \""
                            + filename + "\" to load. Aborting load.";
            DebugLogger() << msg;
            std::cerr << msg << '\n';
            return;
        }
    } else {
        try {
            filename = ClientUI::GetClientUI()->GetFilenameWithSaveFileDialog(
                SaveFileDialog::Purpose::Load,
                SaveFileDialog::SaveType::SinglePlayer);

            // Update intro screen Load & Continue buttons if all savegames are deleted.
            m_ui->GetIntroScreen()->RequirePreRender();

        } catch (const std::exception& e) {
            ClientUI::MessageBox(e.what(), true);
        }
    }

    if (filename.empty()) {
        DebugLogger() << "GGHumanClientApp::LoadSinglePlayerGame has empty filename. Aborting load.";
        return;
    }

    // end any currently-playing game before loading new one
    if (m_game_started) {
        ResetToIntro(true);
        // delay to make sure old game is fully cleaned up before attempting to start a new one
        std::this_thread::sleep_for(std::chrono::seconds(3));
    } else {
        DebugLogger() << "GGHumanClientApp::LoadSinglePlayerGame() not already in a game, so don't need to end it";
    }

    if (!GetOptionsDB().Get<bool>("network.server.external.force")) {
        m_single_player_game = true;
        try {
            StartServer();
        } catch (const LocalServerAlreadyRunningException&) {
            ClientUI::MessageBox(UserString("LOCAL_SERVER_ALREADY_RUNNING_ERROR"), true);
            return;
        } catch (const std::runtime_error& err) {
            ErrorLogger() << "GGHumanClientApp::NewSinglePlayerGame : Couldn't start server.  Got error message: " << err.what();
            ClientUI::MessageBox(UserString("SERVER_WONT_START"), true);
            return;
        }
    } else {
        DebugLogger() << "GGHumanClientApp::LoadSinglePlayerGame() assuming external server will be available";
    }

    DebugLogger() << "GGHumanClientApp::LoadSinglePlayerGame() Connecting to server";
    m_connected = m_networking->ConnectToLocalHostServer();
    if (!m_connected) {
        ResetToIntro(true);
        ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
        return;
    }

    m_networking->SetPlayerID(Networking::INVALID_PLAYER_ID);
    m_networking->SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    SetEmpireID(ALL_EMPIRES);

    SinglePlayerSetupData setup_data;
    // leving GalaxySetupData information default / blank : not used when loading a game
    setup_data.new_game = false;
    setup_data.filename = filename;
    // leving setup_data.m_players empty : not specified when loading a game, as server will generate from save file


    m_networking->SendMessage(HostSPGameMessage(setup_data, DependencyVersions()));
    m_fsm.process_event(HostSPGameRequested());
}

void GGHumanClientApp::RequestSavePreviews(const std::string& relative_directory) {
    TraceLogger() << "GGHumanClientApp::RequestSavePreviews directory: " << relative_directory
                  << " valid UTF-8: " << utf8::is_valid(relative_directory.begin(), relative_directory.end());

    std::string generic_directory = relative_directory;
    if (!m_networking->IsConnected()) {
        DebugLogger() << "GGHumanClientApp::RequestSavePreviews: No game running. Start a server for savegame queries.";

        m_single_player_game = true;
        try {
            StartServer();
        } catch (const LocalServerAlreadyRunningException&) {
            ClientUI::MessageBox(UserString("LOCAL_SERVER_ALREADY_RUNNING_ERROR"), true);
            return;
        } catch (const std::runtime_error& err) {
            ErrorLogger() << "GGHumanClientApp::NewSinglePlayerGame : Couldn't start server.  Got error message: " << err.what();
            ClientUI::MessageBox(UserString("SERVER_WONT_START"), true);
            return;
        }

        DebugLogger() << "GGHumanClientApp::RequestSavePreviews Connecting to server";
        m_connected = m_networking->ConnectToLocalHostServer();
        if (!m_connected) {
            ResetToIntro(true);
            ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
            return;
        }

        // This will only generate an error message and use the server's config.xml
        // because there is no host client for this temporary server.
        SendLoggingConfigToServer();
    }
    DebugLogger() << "GGHumanClientApp::RequestSavePreviews Requesting previews for " << generic_directory;
    m_networking->SendMessage(RequestSavePreviewsMessage(std::move(generic_directory)));
}

std::pair<int, int> GGHumanClientApp::GetWindowLeftTop() {
    int left = GetOptionsDB().Get<int>("video.windowed.left");
    int top = GetOptionsDB().Get<int>("video.windowed.top");

    // clamp to edges to avoid weird bug with maximizing windows setting their
    // left and top to -9 which lead to weird issues when attmepting to recreate
    // the window at those positions next execution
    if (std::abs(left) < 10)
        left = 0;
    if (std::abs(top) < 10)
        top = 0;

    return {left, top};
}

std::pair<int, int> GGHumanClientApp::GetWindowWidthHeight() {
    int width(800), height(600);

    bool fullscreen = GetOptionsDB().Get<bool>("video.fullscreen.enabled");
    if (!fullscreen) {
        width = GetOptionsDB().Get<int>("video.windowed.width");
        height = GetOptionsDB().Get<int>("video.windowed.height");
        return {width, height};
    }

    bool reset_fullscreen = GetOptionsDB().Get<bool>("video.fullscreen.reset");
    if (!reset_fullscreen) {
        width = GetOptionsDB().Get<int>("video.fullscreen.width");
        height = GetOptionsDB().Get<int>("video.fullscreen.height");
        return {width, height};
    }

    GetOptionsDB().Set<bool>("video.fullscreen.reset", false);
    GG::Pt default_resolution = GetDefaultResolutionStatic(GetOptionsDB().Get<int>("video.monitor.id"));
    GetOptionsDB().Set("video.fullscreen.width", Value(default_resolution.x));
    GetOptionsDB().Set("video.fullscreen.height", Value(default_resolution.y));
    GetOptionsDB().Commit();
    return {Value(default_resolution.x), Value(default_resolution.y)};
}

void GGHumanClientApp::Reinitialize() {
    const bool fullscreen = GetOptionsDB().Get<bool>("video.fullscreen.enabled");
    const bool fake_mode_change = GetOptionsDB().Get<bool>("video.fullscreen.fake.enabled");
    const auto size = GetWindowWidthHeight();
    const GG::X width{size.first};
    const GG::Y height{size.second};

    const bool fullscreen_transition = Fullscreen() != fullscreen;
    const GG::X old_width = AppWidth();
    const GG::Y old_height = AppHeight();

    SetVideoMode(width, height, fullscreen, fake_mode_change);
    if (fullscreen_transition) {
        FullscreenSwitchSignal(fullscreen); // after video mode is changed but before DoLayout() calls
    } else if (fullscreen &&
               (old_width != width || old_height != height) &&
               GetOptionsDB().Get<bool>("ui.reposition.auto.enabled"))
    {
        // Reposition windows if in fullscreen mode... handled here instead of
        // HandleWindowResize() because the prev. fullscreen resolution is only
        // available here.
        RepositionWindowsSignal();
    }

    // HandleWindowResize is already called via this signal sent from
    // SDLGUI::HandleSystemEvents() when in windowed mode.  This sends the
    // signal (and hence calls HandleWindowResize()) when in fullscreen mode,
    // making the signal more consistent...
    if (fullscreen)
        WindowResizedSignal(width, height);
}

float GGHumanClientApp::GLVersion() const
{ return GetGLVersion(); }

void GGHumanClientApp::StartTurn(const SaveGameUIData& ui_data) {
    DebugLogger() << "GGHumanClientApp::StartTurn";

    if (auto empire = m_empires.GetEmpire(EmpireID())) {
        const double RP = empire->ResourceOutput(ResourceType::RE_RESEARCH);
        const double PP = empire->ResourceOutput(ResourceType::RE_INDUSTRY);
        const auto turn_number = this->m_current_turn;
        const auto ratio = RP / std::max(PP, 0.0001);
        const auto [r, g, b, a] = empire->Color();
        DebugLogger() << "Current Output (turn " << turn_number << ") RP/PP: " << ratio
                      << " (" << RP << "/" << PP << ")";
        DebugLogger() << "EmpireColors: " << static_cast<int>(r) << " " << static_cast<int>(g)
                      << " " << static_cast<int>(b) << " " << static_cast<int>(a);
    }

    // Do the turn end autosave.
    if (m_single_player_game && GetOptionsDB().Get<bool>("save.auto.turn.end.enabled")) {
        DebugLogger() << "Starting end of turn autosave.";
        Autosave();
    }

    ClientApp::StartTurn(ui_data);
    m_fsm.process_event(TurnEnded());
}

void GGHumanClientApp::UnreadyTurn()
{ m_networking->SendMessage(UnreadyMessage()); }

void GGHumanClientApp::HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id) {
    ClientApp::HandleTurnPhaseUpdate(phase_id);

    // Pass updates to message window.
    GetClientUI().GetMessageWnd()->HandleTurnPhaseUpdate(phase_id);
}

boost::intrusive_ptr<const boost::statechart::event_base> GGHumanClientApp::GetDeferredPostedEvent() {
    std::scoped_lock lock(m_event_queue_guard);
    if (m_posted_event_queue.empty())
        return nullptr;
    auto retval{std::move(m_posted_event_queue.front())};
    m_posted_event_queue.pop();
    return retval;
}

void GGHumanClientApp::PostDeferredEvent(
    boost::intrusive_ptr<const boost::statechart::event_base> event)
{
    std::scoped_lock lock(m_event_queue_guard);
    m_posted_event_queue.push(std::move(event));
}

void GGHumanClientApp::HandleSystemEvents() {
    try {
        SDLGUI::HandleSystemEvents();
    } catch (const utf8::invalid_utf8& e) {
        ErrorLogger() << "UTF-8 error handling system event: " << e.what();
    }
    if (m_connected && !m_networking->IsConnected()) {
        m_connected = false;
        DisconnectedFromServer();
    } else if (auto event_ptr = GetDeferredPostedEvent()) {
        m_fsm.process_event(*event_ptr);
    } else if (auto msg = Networking().GetMessage()) {
        HandleMessage(std::move(*msg));
    }
}

void GGHumanClientApp::RenderBegin() {
    SDLGUI::RenderBegin();
    Sound::GetSound().DoFrame();
}

void GGHumanClientApp::HandleMessage(Message&& msg) {
    auto msg_type = msg.Type();

    if (INSTRUMENT_MESSAGE_HANDLING)
        std::cerr << "GGHumanClientApp::HandleMessage(" << msg_type << ")\n";

    try {
        switch (msg_type) {
        case Message::MessageType::ERROR_MSG:               m_fsm.process_event(Error(msg));                   break;
        case Message::MessageType::HOST_MP_GAME:            m_fsm.process_event(HostMPGame(msg));              break;
        case Message::MessageType::HOST_SP_GAME:            m_fsm.process_event(HostSPGame(msg));              break;
        case Message::MessageType::JOIN_GAME:               m_fsm.process_event(JoinGame(msg));                break;
        case Message::MessageType::HOST_ID:                 m_fsm.process_event(HostID(msg));                  break;
        case Message::MessageType::LOBBY_UPDATE:            m_fsm.process_event(LobbyUpdate(msg));             break;
        case Message::MessageType::SAVE_GAME_COMPLETE:      m_fsm.process_event(SaveGameComplete(msg));        break;
        case Message::MessageType::CHECKSUM:                m_fsm.process_event(CheckSum(msg));                break;
        case Message::MessageType::GAME_START:              m_fsm.process_event(GameStart(msg));               break;
        case Message::MessageType::TURN_UPDATE:             m_fsm.process_event(TurnUpdate(msg));              break;
        case Message::MessageType::TURN_PARTIAL_UPDATE:     m_fsm.process_event(TurnPartialUpdate(msg));       break;
        case Message::MessageType::TURN_PROGRESS:           m_fsm.process_event(TurnProgress(msg));            break;
        case Message::MessageType::UNREADY:                 m_fsm.process_event(TurnRevoked(msg));             break;
        case Message::MessageType::PLAYER_STATUS:           m_fsm.process_event(::PlayerStatus(msg));          break;
        case Message::MessageType::PLAYER_CHAT:             m_fsm.process_event(PlayerChat(msg));              break;
        case Message::MessageType::DIPLOMACY:               m_fsm.process_event(Diplomacy(msg));               break;
        case Message::MessageType::DIPLOMATIC_STATUS:       m_fsm.process_event(DiplomaticStatusUpdate(msg));  break;
        case Message::MessageType::END_GAME:                m_fsm.process_event(::EndGame(msg));               break;

        case Message::MessageType::DISPATCH_COMBAT_LOGS:    m_fsm.process_event(DispatchCombatLogs(msg));      break;
        case Message::MessageType::DISPATCH_SAVE_PREVIEWS:  HandleSaveGamePreviews(msg);                        break;
        case Message::MessageType::AUTH_REQUEST:            m_fsm.process_event(AuthRequest(msg));             break;
        case Message::MessageType::CHAT_HISTORY:            m_fsm.process_event(ChatHistory(msg));             break;
        case Message::MessageType::SET_AUTH_ROLES:          HandleSetAuthRoles(msg);                            break;
        case Message::MessageType::TURN_TIMEOUT:            m_fsm.process_event(TurnTimeout(msg));             break;
        case Message::MessageType::PLAYER_INFO:             m_fsm.process_event(PlayerInfoMsg(msg));           break;
        default:
            ErrorLogger() << "GGHumanClientApp::HandleMessage : Received an unknown message type \"" << msg_type << "\".";
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "GGHumanClientApp::HandleMessage : Exception while reacting to message of type \""
                      << msg_type << "\". what: " << e.what();
    }
}

void GGHumanClientApp::UpdateCombatLogs(const Message& msg) {
    ScopedTimer timer("GGHumanClientApp::UpdateCombatLogs");

    // Unpack the combat logs from the message
    try {
        std::vector<std::pair<int, CombatLog>> logs;
        ExtractDispatchCombatLogsMessageData(msg, logs);

        // Update the combat log manager with the completed logs.
        auto& clm{GetCombatLogManager()};
        for (auto& [log_id, log] : logs)
            clm.CompleteLog(log_id, std::move(log));
    } catch (...) {}
}

void GGHumanClientApp::HandleSaveGamePreviews(const Message& msg) {
    auto sfd = GetClientUI().GetSaveFileDialog();
    if (!sfd)
        return;

    try {
        PreviewInformation previews;
        ExtractDispatchSavePreviewsMessageData(msg, previews);
        DebugLogger() << "GGHumanClientApp::RequestSavePreviews Got " << previews.previews.size() << " previews.";

        sfd->SetPreviewList(std::move(previews));
    } catch (...) {}
}

void GGHumanClientApp::HandleSetAuthRoles(const Message& msg) {
    try {
        ExtractSetAuthorizationRolesMessage(msg, m_networking->AuthorizationRoles());
        DebugLogger() << "New roles: " << m_networking->AuthorizationRoles().Text();
    } catch (...) {}
}

void GGHumanClientApp::ChangeLoggerThreshold(const std::string& option_name, LogLevel option_value) {
    // Update the logger threshold in OptionsDB
    ChangeLoggerThresholdInOptionsDB(option_name, option_value);

    SendLoggingConfigToServer();
}

void GGHumanClientApp::SendLoggingConfigToServer() {
    // If not host then done.
    if (!m_networking->PlayerIsHost(Networking().PlayerID()))
        return;

    // Host updates the server
    const auto sources = LoggerOptionsLabelsAndLevels(LoggerTypes::both);

    m_networking->SendMessage(LoggerConfigMessage(PlayerID(), sources));
}

void GGHumanClientApp::HandleWindowMove(GG::X w, GG::Y h) {
    if (!Fullscreen()) {
        GetOptionsDB().Set<int>("video.windowed.left", Value(w));
        GetOptionsDB().Set<int>("video.windowed.top", Value(h));
        GetOptionsDB().Commit();
    }
}

void GGHumanClientApp::HandleWindowResize(GG::X w, GG::Y h) {
    if (ClientUI* ui = ClientUI::GetClientUI()) {
        if (auto map_wnd = ui->GetMapWnd(false))
            map_wnd->DoLayout();
        if (auto intro_screen = ui->GetIntroScreen())
            intro_screen->Resize(GG::Pt(w, h));
    }

    if (!GetOptionsDB().Get<bool>("video.fullscreen.enabled") &&
         (GetOptionsDB().Get<GG::X>("video.windowed.width") != w ||
          GetOptionsDB().Get<GG::Y>("video.windowed.height") != h))
    {
        if (GetOptionsDB().Get<bool>("ui.reposition.auto.enabled")) {
            // Reposition windows if in windowed mode.
            RepositionWindowsSignal();
        }
        // store resize if window is not full-screen (so that fullscreen
        // resolution doesn't overwrite windowed resolution)
        GetOptionsDB().Set<int>("video.windowed.width", Value(w));
        GetOptionsDB().Set<int>("video.windowed.height", Value(h));
    }

    glViewport(0, 0, Value(w), Value(h));

    GetOptionsDB().Commit();
}

void GGHumanClientApp::HandleFocusChange(bool gained_focus) {
    DebugLogger() << "GGHumanClientApp::HandleFocusChange("
                  << (gained_focus ? "Gained Focus" : "Lost Focus")
                  << ")";

    m_have_window_focus = gained_focus;

    // limit rendering frequency when defocused to limit CPU use, and disable sound
    if (!m_have_window_focus) {
        if (GetOptionsDB().Get<bool>("video.fps.unfocused.enabled"))
            this->SetMaxFPS(GetOptionsDB().Get<double>("video.fps.unfocused"));
        else
            this->SetMaxFPS(0.0);

        if (GetOptionsDB().Get<bool>("audio.music.enabled"))
            Sound::GetSound().PauseMusic();
    }
    else {
        if (GetOptionsDB().Get<bool>("video.fps.max.enabled"))
            this->SetMaxFPS(GetOptionsDB().Get<double>("video.fps.max"));
        else
            this->SetMaxFPS(0.0);

        if (GetOptionsDB().Get<bool>("audio.music.enabled"))
            Sound::GetSound().ResumeMusic();
    }

    CancelDragDrop();
    ClearEventState();
}

void GGHumanClientApp::HandleAppQuitting() {
    DebugLogger() << "GGHumanClientApp::HandleAppQuitting()";
    ExitApp(0);
}

bool GGHumanClientApp::HandleHotkeyResetGame() {
    DebugLogger() << "GGHumanClientApp::HandleHotkeyResetGame()";
    ResetToIntro(false);
    return true;
}

bool GGHumanClientApp::HandleHotkeyExitApp() {
    DebugLogger() << "GGHumanClientApp::HandleHotkeyExitApp()";
    HandleAppQuitting();
    // Not reached, but required for HotkeyManager::Connect()
    return true;
}

bool GGHumanClientApp::ToggleFullscreen() {
    bool fs = GetOptionsDB().Get<bool>("video.fullscreen.enabled");
    GetOptionsDB().Set<bool>("video.fullscreen.enabled", !fs);
    Reinitialize();
    return true;
}

void GGHumanClientApp::StartGame(bool is_new_game) {
    m_game_started = true;

    if (auto map_wnd = ClientUI::GetClientUI()->GetMapWnd(false))
        map_wnd->ResetEmpireShown();

    ClientUI::GetClientUI()->GetShipDesignManager()->StartGame(EmpireID(), is_new_game);
}

void GGHumanClientApp::UpdateCombatLogManager() {
    auto incomplete_ids = GetCombatLogManager().IncompleteLogIDs();
    if (incomplete_ids.empty())
        return;

    static constexpr std::size_t log_batch_size = 50;
    for (auto it = incomplete_ids.begin(); it != incomplete_ids.end();) {
        // request at most 50 logs per message to avoid trying to allocate too much space to send all at once
        std::vector<int> a_few_log_ids;
        a_few_log_ids.reserve(log_batch_size);
        for (unsigned int count = 0; count < log_batch_size && it != incomplete_ids.end(); ++it, ++count)
            a_few_log_ids.push_back(*it);
        DebugLogger() << "Requesting " << a_few_log_ids.size() << " combat logs from server";
        m_networking->SendMessage(RequestCombatLogsMessage(a_few_log_ids));
    }
}

namespace {
    boost::optional<std::string> NewestSinglePlayerSavegame() {
        using namespace boost::filesystem;
        try {
            std::multimap<std::time_t, path> files_by_write_time;

            auto add_all_savegames_in = [&files_by_write_time](const path& path) {
                if (!is_directory(path))
                    return;

                for (directory_iterator dir_it(path);
                     dir_it != directory_iterator(); ++dir_it)
                {
                    const auto& file_path = dir_it->path();
                    if (!is_regular_file(file_path))
                        continue;
                    if (file_path.extension() != SP_SAVE_FILE_EXTENSION)
                        continue;

                    files_by_write_time.emplace(last_write_time(file_path), file_path);
                }
            };

            // Find all save games in either player or autosaves
            add_all_savegames_in(GetSaveDir());
            add_all_savegames_in(GetSaveDir() / "auto");

            if (files_by_write_time.empty())
                return boost::none;

            // Return the newest file that has a valid header
            for (auto file_it = files_by_write_time.rbegin();
                 file_it != files_by_write_time.rend(); ++file_it)
            {
                const auto& file = file_it->second;
                // attempt to load header
                if (SaveFileWithValidHeader(file))
                    return PathToString(file);  // load succeeded, return path to OK file
            }

            return boost::none;

        } catch (const boost::filesystem::filesystem_error& e) {
            ErrorLogger() << "File system error " << e.what() << " while finding newest autosave";
            return boost::none;
        }
    }

    void RemoveOldestFiles(int files_limit, boost::filesystem::path& p) {
        using namespace boost::filesystem;
        try {
            if (!is_directory(p))
                return;
            if (files_limit < 0)
                return;

            std::multimap<std::time_t, path> files_by_write_time;

            for (directory_iterator dir_it(p); dir_it != directory_iterator(); ++dir_it) {
                const path& file_path = dir_it->path();
                if (!is_regular_file(file_path))
                    continue;
                if (file_path.extension() != SP_SAVE_FILE_EXTENSION &&
                    file_path.extension() != MP_SAVE_FILE_EXTENSION)
                { continue; }

                files_by_write_time.emplace(last_write_time(file_path), file_path);
            }

            //DebugLogger() << "files by write time:";
            //for (auto& entry : files_by_write_time)
            //{ DebugLogger() << entry.first << " : " << entry.second.filename(); }

            int num_to_delete = files_by_write_time.size() - files_limit + 1;   // +1 because will add a new file after deleting, bringing number back up to limit
            if (num_to_delete <= 0)
                return; // don't need to delete anything.

            int num_deleted = 0;
            for (auto& delete_file_path : files_by_write_time | range_values) {
                if (num_deleted >= num_to_delete)
                    break;
                remove(delete_file_path);
                ++num_deleted;
            }
        } catch (...) {
            ErrorLogger() << "Error removing oldest files";
        }
    }

    boost::filesystem::path CreateNewAutosaveFilePath(int client_empire_id, bool is_single_player,
                                                      const EmpireManager& empires, int turn)
    {
        static constexpr const char* legal_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-";

        // get empire name, filtered for filename acceptability
        auto empire =  empires.GetEmpire(client_empire_id);
        std::string empire_name{empire ? empire->Name() : UserString("OBSERVER")};
        std::string::size_type first_good_empire_char = empire_name.find_first_of(legal_chars);
        if (first_good_empire_char == std::string::npos) {
            empire_name.clear();
        } else {
            std::string::size_type first_bad_empire_char = empire_name.find_first_not_of(legal_chars, first_good_empire_char);
            empire_name = empire_name.substr(first_good_empire_char, first_bad_empire_char - first_good_empire_char);
        }

        // get player name, also filtered
        std::string player_name;
        if (empire)
            player_name = empire->PlayerName();
        std::string::size_type first_good_player_char = player_name.find_first_of(legal_chars);
        if (first_good_player_char == std::string::npos) {
            player_name.clear();
        } else {
            std::string::size_type first_bad_player_char = player_name.find_first_not_of(legal_chars, first_good_player_char);
            player_name = player_name.substr(first_good_player_char, first_bad_player_char - first_good_player_char);
        }

        // select filename extension
        const auto& extension = is_single_player ? SP_SAVE_FILE_EXTENSION : MP_SAVE_FILE_EXTENSION;

        // Add timestamp to autosave generated files
        auto datetime_str = FilenameTimestamp();

        boost::filesystem::path autosave_dir_path(
            (is_single_player ? GetSaveDir() : GetServerSaveDir()) / "auto");

        auto save_filename = boost::io::str(boost::format("FreeOrion_%s_%s_%04d_%s%s")
                                            % player_name % empire_name % turn
                                            % datetime_str % extension);
        boost::filesystem::path save_path(autosave_dir_path / save_filename);

        try {
            // ensure autosave directory exists
            if (!exists(autosave_dir_path))
                boost::filesystem::create_directories(autosave_dir_path);
        } catch (const std::exception& e) {
            ErrorLogger() << "Autosave unable to check / create autosave directory: " << e.what();
        }

        return save_path;
    }
}

void GGHumanClientApp::Autosave() {
    // only host can save in multiplayer
    if (!m_single_player_game && !Networking().PlayerIsHost(PlayerID()))
        return;

    // Create an auto save for 1) new games on turn 1, 2) if auto save is
    // requested on turn number modulo save.auto.turn.interval or 3) on the last turn of
    // play.

    // autosave only on appropriate turn numbers, and when enabled for current
    // game type (single vs. multiplayer)
    int autosave_turns = GetOptionsDB().Get<int>("save.auto.turn.interval");
    bool is_single_player_enabled =
        (m_single_player_game
         && (GetOptionsDB().Get<bool>("save.auto.turn.start.enabled")
             || GetOptionsDB().Get<bool>("save.auto.turn.end.enabled")));
    bool is_multi_player_enabled =
        (!m_single_player_game
         && GetOptionsDB().Get<bool>("save.auto.turn.multiplayer.start.enabled"));
    bool is_valid_autosave =
        (autosave_turns > 0
         && this->m_current_turn % autosave_turns == 0
         && (is_single_player_enabled || is_multi_player_enabled));

    // is_initial_save is gated in HumanClientFSM for new game vs loaded game
    bool is_initial_save =
        (GetOptionsDB().Get<bool>("save.auto.initial.enabled")
         && this->m_current_turn == 1);
    bool is_final_save =
        (GetOptionsDB().Get<bool>("save.auto.exit.enabled")
         && !m_game_started);

    if (!(is_initial_save || is_valid_autosave || is_final_save))
        return;

    auto autosave_file_path = CreateNewAutosaveFilePath(EmpireID(), m_single_player_game,
                                                        m_empires, m_current_turn);

    // check for and remove excess oldest autosaves.
    boost::filesystem::path autosave_dir_path(
        (m_single_player_game ? GetSaveDir() : GetServerSaveDir()) / "auto");
    int max_turns = std::max(1, GetOptionsDB().Get<int>("save.auto.file.limit"));
    bool is_two_saves_per_turn =
        (m_single_player_game
         && GetOptionsDB().Get<bool>("save.auto.turn.start.enabled")
         && GetOptionsDB().Get<bool>("save.auto.turn.end.enabled"))
        ||
        (!m_single_player_game
         && GetOptionsDB().Get<bool>("save.auto.turn.multiplayer.start.enabled"));
    int max_autosaves =
        (max_turns * (is_two_saves_per_turn ? 2 : 1)
         + (GetOptionsDB().Get<bool>("save.auto.initial.enabled") ? 1 : 0)
         + (GetOptionsDB().Get<bool>("save.auto.exit.enabled") ? 1 : 0));
    RemoveOldestFiles(max_autosaves, autosave_dir_path);

    // create new save
    auto path_string = PathToString(autosave_file_path);

    if (is_initial_save)
        DebugLogger() << "Turn 0 autosave to: " << path_string;
    if (is_valid_autosave)
        DebugLogger() << "Autosave to: " << path_string;
    if (is_final_save)
        DebugLogger() << "End of play autosave to: " << path_string;

    try {
        SaveGame(path_string);
    } catch (const std::exception& e) {
        ErrorLogger() << "Autosave failed: " << e.what();
    }
}

void GGHumanClientApp::ContinueSinglePlayerGame() {
    if (const auto file = NewestSinglePlayerSavegame())
        LoadSinglePlayerGame(*file);
}

bool GGHumanClientApp::IsLoadGameAvailable() const
{ return bool(NewestSinglePlayerSavegame()); }

std::string GGHumanClientApp::SelectLoadFile() {
    return ClientUI::GetClientUI()->GetFilenameWithSaveFileDialog(
        SaveFileDialog::Purpose::Load,
        SaveFileDialog::SaveType::MultiPlayer);
}

void GGHumanClientApp::ResetClientData(bool save_connection) {
    if (!save_connection) {
        m_networking->SetPlayerID(Networking::INVALID_PLAYER_ID);
        m_networking->SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    }
    SetEmpireID(ALL_EMPIRES);
    if (auto map_wnd = m_ui->GetMapWnd(false))
        map_wnd->Sanitize();

    m_universe.Clear();
    m_empires.Clear();
    m_orders.Reset();
    GetCombatLogManager().Clear();
    ClearPreviousPendingSaves(m_game_saves_in_progress);
}

void GGHumanClientApp::ResetToIntro(bool skip_savegame)
{ ResetOrExitApp(true, skip_savegame); }

void GGHumanClientApp::ExitApp(int exit_code)
{ ResetOrExitApp(false, false, exit_code); }

void GGHumanClientApp::ExitSDL(int exit_code)
{ SDLGUI::ExitApp(exit_code); }

void GGHumanClientApp::ResetOrExitApp(bool reset, bool skip_savegame, int exit_code ) {
    DebugLogger() << "GGHumanClientApp::ResetOrExitApp(" << reset << ", " << skip_savegame << ", " << exit_code << ")";
    if (m_exit_handled) {
        static constinit int repeat_count = 0;
        if (repeat_count++ > 2) {
            m_exit_handled = false;
            skip_savegame = true;
        } else {
            return;
        }
    }
    m_exit_handled = true;
    DebugLogger() << (reset ? "GGHumanClientApp::ResetToIntro" : "GGHumanClientApp::ExitApp");

    auto was_playing = m_game_started;
    m_game_started = false;

    // Only save or allow user to cancel if not exiting due to an error.
    if (!skip_savegame) {
        // Check if this is a multiplayer game and the player has not set status to ready
        if (was_playing && !m_single_player_game &&
            m_empires.GetEmpire(m_empire_id) != nullptr &&
            !m_empires.GetEmpire(m_empire_id)->Ready() &&
            GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
        {
            auto font = ClientUI::GetFont();
            auto prompt = GG::GUI::GetGUI()->GetStyleFactory().NewThreeButtonDlg(
                GG::X(275), GG::Y(75), UserString("GAME_MENU_CONFIRM_NOT_READY"), font,
                ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(), ClientUI::CtrlColor(), ClientUI::TextColor(),
                2, UserString("YES"), UserString("CANCEL"));
            prompt->Run();
            if (prompt->Result() != 0) {
                // User aborted exit/resign, reset variables
                m_game_started = was_playing;
                m_exit_handled = false;
                return;
            }
        }

        if (was_playing && GetOptionsDB().Get<bool>("save.auto.exit.enabled"))
            Autosave();

        if (!m_game_saves_in_progress.empty()) {
            DebugLogger() << "save game in progress. Checking with player.";
            // Ask the player if they want to wait for the save game to complete
            auto dlg = GG::GUI::GetGUI()->GetStyleFactory().NewThreeButtonDlg(
                GG::X(320), GG::Y(200), UserString("SAVE_GAME_IN_PROGRESS"),
                ClientUI::GetFont(ClientUI::Pts()+2),
                ClientUI::WndColor(), ClientUI::WndOuterBorderColor(),
                ClientUI::CtrlColor(), ClientUI::TextColor(), 1,
                (reset ?
                    UserString("ABORT_SAVE_AND_RESET") :
                    UserString("ABORT_SAVE_AND_EXIT")));
            // The dialog automatically closes if the save completes while the
            // user is waiting
            this->SaveGamesCompletedSignal.connect(
                [dlg](){
                    DebugLogger() << "SaveGamePendingDialog::SaveCompletedHandler save game completed handled.";

                    dlg->EndRun();
                }
            );

            dlg->Run();
        }
    }

    // Create an action to reset to intro or quit the app as appropriate.
    std::function<void()> after_server_shutdown_action;
    if (reset)
        after_server_shutdown_action = boost::bind(&GGHumanClientApp::ResetClientData, this, false);
    else
        // This throws to exit the GUI
        after_server_shutdown_action = boost::bind(&GGHumanClientApp::ExitSDL, this, exit_code);

    m_fsm.process_event(StartQuittingGame(m_server_process, std::move(after_server_shutdown_action)));

    m_exit_handled = false;
}

void GGHumanClientApp::InitAutoTurns(int auto_turns) {
    m_auto_turns = auto_turns;
    if (!m_game_started || m_auto_turns < 0)
        m_auto_turns = 0;
}

void GGHumanClientApp::DecAutoTurns(int n)
{ InitAutoTurns(m_auto_turns - n); }

void GGHumanClientApp::EliminateSelf()
{ m_networking->SendMessage(EliminateSelfMessage()); }

int GGHumanClientApp::AutoTurnsLeft() const
{ return m_auto_turns; }

bool GGHumanClientApp::HaveWindowFocus() const
{ return m_have_window_focus; }

int GGHumanClientApp::SelectedSystemID() const {
    if (m_ui) {
        if (auto mapwnd = m_ui->GetMapWndConst())
            return mapwnd->SelectedSystemID();
    }
    return INVALID_OBJECT_ID;
}

int GGHumanClientApp::SelectedPlanetID() const {
    if (m_ui) {
        if (auto mapwnd = m_ui->GetMapWndConst())
            return mapwnd->SelectedPlanetID();
    }
    return INVALID_OBJECT_ID;
}

int GGHumanClientApp::SelectedFleetID() const {
    if (m_ui) {
        if (auto mapwnd = m_ui->GetMapWndConst())
            return mapwnd->SelectedFleetID();
    }
    return INVALID_OBJECT_ID;
}

int GGHumanClientApp::SelectedShipID() const {
    if (m_ui) {
        if (auto mapwnd = m_ui->GetMapWndConst())
            return mapwnd->SelectedShipID();
    }
    return INVALID_OBJECT_ID;
}

int GGHumanClientApp::EffectsProcessingThreads() const
{ return GetOptionsDB().Get<int>("effects.ui.threads"); }

void GGHumanClientApp::UpdateFPSLimit() {
    if (GetOptionsDB().Get<bool>("video.fps.max.enabled")) {
        double fps = GetOptionsDB().Get<double>("video.fps.max");
        SetMaxFPS(fps);
        DebugLogger() << "Limited FPS to " << fps;
    } else {
        SetMaxFPS(0.0); // disable fps limit
        DebugLogger() << "Disabled FPS limit";
    }
}

void GGHumanClientApp::HandleResoureDirChange() {
    if (!m_game_started) {
        DebugLogger() << "Resource directory changed.  Reparsing universe ...";
        std::promise<void> barrier;
        std::future<void> barrier_future = barrier.get_future();
        std::thread background([this] (auto b) {
            DebugLogger() << "Started background parser thread";
            PythonCommon python;
            python.Initialize();
            StartBackgroundParsing(PythonParser(python, GetResourceDir() / "scripting"), std::move(b));
        }, std::move(barrier));
        background.detach();
        barrier_future.wait();
    } else {
        WarnLogger() << "Resource directory changes will take effect on application restart.";
    }
}

void GGHumanClientApp::DisconnectedFromServer() {
    DebugLogger() << "GGHumanClientApp::DisconnectedFromServer";
    m_fsm.process_event(Disconnection());
}

void GGHumanClientApp::OpenURL(const std::string& url) {
    // make sure it's a legit url
    std::string trimmed_url = url;
    boost::algorithm::trim(trimmed_url);
    // shouldn't be excessively long
    if (trimmed_url.size() > 500) { // arbitrary limit
        ErrorLogger() << "GGHumanClientApp::OpenURL given bad-looking url (too long): " << trimmed_url;
        return;
    }
    // should start with http:// or https://
    if (trimmed_url.size() < 8) {
        ErrorLogger() << "GGHumanClientApp::OpenURL given bad-looking url (too short): " << trimmed_url;
        return;
    }
    if (trimmed_url.find_first_of("http://") != 0 &&
        trimmed_url.find_first_of("https://") != 0)
    {
        ErrorLogger() << "GGHumanClientApp::OpenURL given url that doesn't start with http:// :" << trimmed_url;
        return;
    }
    // should not have newlines...
    if (trimmed_url.find_first_of("\n") != std::string::npos) {
        ErrorLogger() << "GGHumanClientApp::OpenURL given url that contains a newline. rejecting.";
        return;
    }

    // append url to OS-specific open command
    std::string command;
#ifdef _WIN32
    command += "start ";
#elif __APPLE__
    command += "open ";
#else
    command += "xdg-open ";
#endif
    command += url;

    // execute open command
    int rv = system(command.c_str());
    if (rv != 0)
        ErrorLogger() << "GGHumanClientApp::OpenURL `" << command << "` returned a non-zero exit code: " << rv;
}

void GGHumanClientApp::BrowsePath(const boost::filesystem::path& browse_path) {
    if (browse_path.empty() || browse_path == "/") {
        ErrorLogger() << "Invalid path: " << PathToString(browse_path);
        return;
    }

    boost::filesystem::path full_path(browse_path);

    try {
        boost::filesystem::file_status status = boost::filesystem::status(full_path);
        if (!boost::filesystem::exists(status)) {
            std::string exists_debug_msg("Non-existant path: " + PathToString(full_path));
            if (full_path.has_parent_path()) {
                DebugLogger() << exists_debug_msg << ", trying parent directory";
                BrowsePath(full_path.parent_path());
            } else {
                DebugLogger() << exists_debug_msg << ", aborting";
            }
            return;
        }

        // Validate as a canonical path
        if (boost::filesystem::is_directory(status)) {
            full_path = boost::filesystem::canonical(full_path);
        } else {
            // If given a file, use the files containing directory
            DebugLogger() << "Non-directory target: " << PathToString(full_path) << ", using parent directory";
            full_path = boost::filesystem::canonical(full_path.parent_path());
        }

        // Verify not a regular file
        if (boost::filesystem::is_regular_file(full_path)) {
            ErrorLogger() << "Target directory " << PathToString(full_path) << " is a regular file, given path argument: "
                          << PathToString(browse_path);
            return;
        }

    } catch (const boost::filesystem::filesystem_error& ec) {
        ErrorLogger() << "Filesystem error when attempting to browse directory " << PathToString(full_path)
                      << ": " << ec.what();
        return;
    }

    if (full_path.empty()) {
        ErrorLogger() << "Unable to determine directory for path " << PathToString(full_path);
        return;
    }

    full_path.make_preferred();
    // Trailing slash post-fixed to prevent executing a file with same name(minus extension) as folder
    full_path += boost::filesystem::path::preferred_separator;
    auto target(full_path.native());
    decltype(target) command;

    // Double quotes around target to support paths containing spaces
    // Non-Windows platforms: Post-fix ampersand to prevent blocking until process exits
    // On Windows: the trailing path separator may be interpreted as escaping a double quote.
    //    The trailing separator should not be removed, as that poses the risk of executing a file.
    //    The trailing separator is escaped by 2 additional back-slashes (total 3).
    //    see http://www.windowsinspired.com/how-a-windows-programs-splits-its-command-line-into-individual-arguments/
    //
    //    Contrary to official documentation for start, the first argument (title) is not always optional.
    //    The argument for window title is left as an empty string.
    //    see https://ss64.com/nt/start.html
#ifdef _WIN32
    command = L"start \"\" \"" + target + L"\\\\\"";
#elif __APPLE__
    command = "open \"" + target + "\" &";
#else
    command = "xdg-open \"" + target + "\" &";
#endif

#ifdef _WIN32
    std::string u8_command;
    utf8::utf16to8(command.begin(), command.end(), std::back_inserter(u8_command));
    InfoLogger() << "Sending OS request to browse directory: " << u8_command;
    // Flush all streams prior to _wsystem call per https://msdn.microsoft.com/en-us/library/277bwbdz.aspx
    std::fflush(NULL);
    if (auto sys_retval = _wsystem(command.c_str()))
        WarnLogger() << "System call " << u8_command << " returned non-zero value " << sys_retval;
#else
    InfoLogger() << "Sending OS request to browse directory: " << command;
    if (auto sys_retval = std::system(command.c_str()))
        WarnLogger() << "System call " << command << " returned non-zero value " << sys_retval;
#endif
}
