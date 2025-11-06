#include "GGHumanClientApp.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/Version.h"
#include "../../util/i18n.h"
#include "../../UI/Hotkeys.h"

#include "chmain.h"
#include <GG/utf8/checked.h>

#include <boost/format.hpp>
#include <filesystem>
#include <fstream>

#include <thread>
#include <chrono>
#include <iostream>

#if defined(FREEORION_LINUX)
/* Freeorion aims to have exceptions handled and operation continue normally.
An example of good exception handling is the exceptions caught around config.xml loading.
After catching and informing the user it continues normally with the default values.

An exception that can not be handled should allow freeorion to crash and keep
a complete stack trace of the intial exception.
Some platforms do not support this behavior.

When FREEORION_CHMAIN_KEEP_BACKTRACE is defined, do not catch an unhandled exceptions,
unroll and hide the stack trace, print a message and still crash anyways. */
#define FREEORION_CHMAIN_KEEP_STACKTRACE
#endif

// The STORE_FULLSCREEN_FLAG parameter below controls whether the fullscreen
// option is stored in the XML config file.  On Win32 it is not, because the
// installed version of FO is run with the command-line flag added in as
// appropriate.
#ifdef FREEORION_WIN32
constexpr auto STORE_FULLSCREEN_FLAG = OptionsDB::Storable::UNSTORABLE;
// Windows keeps good care of the resolution state itself,
// so there is no reason to default to not touching it.
constexpr bool FAKE_MODE_CHANGE_FLAG = false;
#else
constexpr auto STORE_FULLSCREEN_FLAG = OptionsDB::Storable::STORABLE;
// The X window system does not always work
// well with resolution changes, so we avoid them
// by default
constexpr bool FAKE_MODE_CHANGE_FLAG = true;
#endif


#if defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU)
int main(int argc, char* argv[]) {
    // copy command line arguments to vector
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);

    // set options from command line or config.xml, or generate config.xml
    if (mainConfigOptionsSetup(args) != 0) {
        std::cerr << "main() failed config." << std::endl;
        ShutdownLoggingSystemFileSink() ;
        return 1;
    }
#endif
#ifdef FREEORION_WIN32
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
    // copy UTF-16 command line arguments to UTF-8 vector
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        std::wstring argi16(argv[i]);
        std::string argi8;
        utf8::utf16to8(argi16.begin(), argi16.end(), std::back_inserter(argi8));
        args.push_back(argi8);
    }

    // set options from command line or config.xml, or generate config.xml
    if (mainConfigOptionsSetup(args) != 0) {
        std::cerr << "main() failed config." << std::endl;
        return 1;
    }
#endif
#ifndef FREEORION_MACOSX
    // did the player request help output?
    auto help_arg = GetOptionsDB().Get<std::string>("help");
    if (help_arg != "NOOP") {
        ShutdownLoggingSystemFileSink();
        GetOptionsDB().GetUsage(std::cout, help_arg, true);
        return 0;   // quit without actually starting game
    }

    // did the player request the version output?
    if (GetOptionsDB().Get<bool>("version")) {
        ShutdownLoggingSystemFileSink();
        std::cout << "FreeOrion Human Client " << FreeOrionVersionString() << std::endl;
        return 0;   // quit without actually starting game
    }

    // set up rendering and run game
    if (mainSetupAndRun() != 0) {
        ShutdownLoggingSystemFileSink();
        std::cerr << "main() failed to setup or run SDL." << std::endl;
        return 1;
    }
    ShutdownLoggingSystemFileSink();
    return 0;
}
#endif

[[nodiscard]] GGHumanClientApp& GetApp() {
    static GGHumanClientApp app("FreeOrion " + FreeOrionVersionString());
    return app;
}

int mainConfigOptionsSetup(const std::vector<std::string>& args) {
    InitDirs((args.empty() ? "" : args.front()));

    GGHumanClientApp::InitLogging();
    auto& db = GetOptionsDB();

    // read and process command-line arguments, if any
#ifndef FREEORION_CHMAIN_KEEP_STACKTRACE
    try {
#endif
        // add entries in options DB that have no other obvious place
        db.Add<std::string>('h', "help",                UserStringNop("OPTIONS_DB_HELP"),                   "NOOP",
                            Validator<std::string>(),   OptionsDB::Storable::UNSTORABLE);
        db.AddFlag('v', "version",                      UserStringNop("OPTIONS_DB_VERSION"),
                   OptionsDB::Storable::UNSTORABLE,     "version");
        db.AddFlag('g', "generate-config-xml",          UserStringNop("OPTIONS_DB_GENERATE_CONFIG_XML"),    OptionsDB::Storable::UNSTORABLE);
        db.AddFlag('f', "video.fullscreen.enabled",     UserStringNop("OPTIONS_DB_FULLSCREEN"),             STORE_FULLSCREEN_FLAG);
        db.Add<bool>("video.fullscreen.reset",          UserStringNop("OPTIONS_DB_RESET_FSSIZE"),           true);
        db.Add<bool>("video.fullscreen.fake.enabled",   UserStringNop("OPTIONS_DB_FAKE_MODE_CHANGE"),       FAKE_MODE_CHANGE_FLAG);
        db.Add<int>("video.monitor.id",                 UserStringNop("OPTIONS_DB_FULLSCREEN_MONITOR_ID"),  0,
                    RangedValidator<int>(0, 5));
        db.AddFlag("continue",                          UserStringNop("OPTIONS_DB_CONTINUE"),               OptionsDB::Storable::UNSTORABLE);
        db.AddFlag("auto-quit",                         UserStringNop("OPTIONS_DB_AUTO_QUIT"),              OptionsDB::Storable::UNSTORABLE);
        db.Add<int>("auto-advance-n-turns",             UserStringNop("OPTIONS_DB_AUTO_N_TURNS"),           0,
                    RangedValidator<int>(0, 400),       OptionsDB::Storable::UNSTORABLE);
        db.Add<bool>("audio.music.enabled",             UserStringNop("OPTIONS_DB_MUSIC_ON"),               true);
        db.Add<bool>("audio.effects.enabled",           UserStringNop("OPTIONS_DB_SOUND_ON"),               true);
        db.Add<std::string>("version.string",           UserStringNop("OPTIONS_DB_VERSION_STRING"),         FreeOrionVersionString(),
                            Validator<std::string>(),   OptionsDB::Storable::STORABLE);
        db.AddFlag('r', "render-simple",                UserStringNop("OPTIONS_DB_RENDER_SIMPLE"),          OptionsDB::Storable::UNSTORABLE);
        db.Add<std::string>("misc.server-local-binary.path", UserStringNop("OPTIONS_DB_FREEORIOND_PATH"),
#ifdef FREEORION_WIN32
                            PathToString(GetBinDir() / "freeoriond.exe"));
#else
                            PathToString(GetBinDir() / "freeoriond"));
#endif

        // add sections for option sorting
        db.AddSection("audio",              UserStringNop("OPTIONS_DB_SECTION_AUDIO"));
        db.AddSection("audio.music",        UserStringNop("OPTIONS_DB_SECTION_AUDIO_MUSIC"));
        db.AddSection("audio.effects",      UserStringNop("OPTIONS_DB_SECTION_AUDIO_EFFECTS"));
        db.AddSection("audio.effects.paths",UserStringNop("OPTIONS_DB_SECTION_AUDIO_EFFECTS_PATHS"),
                      [](std::string_view name) -> bool {
                          static constexpr std::string_view suffix{"sound.path"};
                          return name.size() > suffix.size() &&
                                 name.substr(name.size() - suffix.size()) == suffix;
                      });
        db.AddSection("effects",            UserStringNop("OPTIONS_DB_SECTION_EFFECTS"));
        db.AddSection("logging",            UserStringNop("OPTIONS_DB_SECTION_LOGGING"));
        db.AddSection("network",            UserStringNop("OPTIONS_DB_SECTION_NETWORK"));
        db.AddSection("resource",           UserStringNop("OPTIONS_DB_SECTION_RESOURCE"));
        db.AddSection("save",               UserStringNop("OPTIONS_DB_SECTION_SAVE"));
        db.AddSection("setup",              UserStringNop("OPTIONS_DB_SECTION_SETUP"));
        db.AddSection("ui",                 UserStringNop("OPTIONS_DB_SECTION_UI"));
        db.AddSection("ui.colors",          UserStringNop("OPTIONS_DB_SECTION_UI_COLORS"),
                      [](std::string_view name) -> bool {
                          static constexpr std::string_view suffix{".color"};
                          return name.size() > suffix.size() &&
                                 name.substr(name.size() - suffix.size()) == suffix;
                      });
        db.AddSection("ui.hotkeys",         UserStringNop("OPTIONS_DB_SECTION_UI_HOTKEYS"),
                      [](std::string_view name) -> bool {
                          static constexpr std::string_view suffix{".hotkey"};
                          return name.size() > suffix.size() &&
                                 name.substr(name.size() - suffix.size()) == suffix;
                      });
        db.AddSection("version",            UserStringNop("OPTIONS_DB_SECTION_VERSION"));
        db.AddSection("video",              UserStringNop("OPTIONS_DB_SECTION_VIDEO"));
        db.AddSection("video.fullscreen",   UserStringNop("OPTIONS_DB_SECTION_VIDEO_FULLSCREEN"));
        db.AddSection("video.windowed",     UserStringNop("OPTIONS_DB_SECTION_VIDEO_WINDOWED"));

        // Add the keyboard shortcuts
        Hotkey::AddOptions(db);

        // if config.xml and persistent_config.xml are present, read and set options entries
        db.SetFromFile(GetConfigPath(), FreeOrionVersionString());
        db.SetFromFile(GetPersistentConfigPath());

        // override previously-saved and default options with command line parameters and flags
        db.SetFromCommandLine(args);

        CompleteXDGMigration();

        // Handle the case where the resource.path does not exist anymore
        // gracefully by resetting it to the standard path into the
        // application bundle.  This may happen if a previous installed
        // version of FreeOrion was residing in a different directory.
        if (!std::filesystem::exists(GetResourceDir()) ||
            !std::filesystem::exists(GetResourceDir() / "credits.xml") ||
            !std::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
        {
            DebugLogger() << "Resources directory from config.xml missing or does not contain expected files. Resetting to default.";

            db.Set<std::string>("resource.path", "");

            // double-check that resetting actually fixed things...
            if (!std::filesystem::exists(GetResourceDir()) ||
                !std::filesystem::exists(GetResourceDir() / "credits.xml") ||
                !std::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
            {
                DebugLogger() << "Default Resources directory missing or does not contain expected files. Cannot start game.";
                throw std::runtime_error("Unable to load game resources at default location: " +
                                         PathToString(GetResourceDir()) + " : Install may be broken.");
            }
        }


        // did the player request generation of config.xml, saving the default (or current) options to disk?
        if (db.Get<bool>("generate-config-xml")) {
            try {
                db.Commit(false);
            } catch (...) {
                std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
            }
        }

        if (db.Get<bool>("render-simple")) {
            db.Set<bool>("ui.map.background.gas.shown", false);
            db.Set<bool>("ui.map.background.starfields.shown", false);
            db.Set<bool>("video.fps.shown", true);
        }

#ifndef FREEORION_CHMAIN_KEEP_STACKTRACE
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_argument): " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 1;
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 1;
    }
#endif

    return 0;
}

int mainSetupAndRun() {
#ifndef FREEORION_CHMAIN_KEEP_STACKTRACE
    try {
#endif

#ifdef FREEORION_WIN32
#  ifdef IDI_ICON1
        // set window icon to embedded application icon
        HWND hwnd;
        window->getCustomAttribute("WINDOW", std::addressof(hwnd));
        HINSTANCE hInst = (HINSTANCE)GetModuleHandle(nullptr);
        SetClassLong (hwnd, GCL_HICON,
            (LONG)LoadIcon (hInst, MAKEINTRESOURCE (IDI_ICON1)));
#  endif
#endif

        GGHumanClientApp& app = GetApp();
        app.Initialize();

        const auto& db = GetOptionsDB();

        if (db.Get<bool>("quickstart")) {
            // immediately start the server, establish network connections, and
            // go into a single player game, using default universe options (a
            // standard quickstart, without requiring the user to click the
            // quickstart button).
            app.NewSinglePlayerGame(true);
        }

        if (db.Get<bool>("continue")) {
            // immediately start the server, establish network connections, and
            // go into a single player game, continuing from the newest
            // save game.
            app.ContinueSinglePlayerGame();
        }

        std::string load_filename = db.Get<std::string>("load");
        if (!load_filename.empty()) {
            // immediately start the server, establish network connections, and
            // go into a single player game, loading the indicated file
            // (without requiring the user to click the load button).
            app.LoadSinglePlayerGame(load_filename);
        }

        // run rendering loop
        app.Run();
#ifndef FREEORION_CHMAIN_KEEP_STACKTRACE
    } catch (const std::invalid_argument& e) {
        ErrorLogger() << "main() caught exception(std::invalid_argument): " << e.what();
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
        return 1;
    } catch (const std::runtime_error& e) {
        ErrorLogger() << "main() caught exception(std::runtime_error): " << e.what();
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
        return 1;
    } catch (const boost::io::format_error& e) {
        ErrorLogger() << "main() caught exception(boost::io::format_error): " << e.what();
        std::cerr << "main() caught exception(boost::io::format_error): " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        ErrorLogger() << "main() caught exception(std::exception): " << e.what();
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
        return 1;
    }
#endif

    DebugLogger() << "Human client main exited cleanly.";
    return 0;
}

#undef FREEORION_CHMAIN_KEEP_STACKTRACE
