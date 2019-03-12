

#include "HumanClientApp.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/Version.h"
#include "../../util/i18n.h"
#include "../../UI/Hotkeys.h"

#include <GG/utf8/checked.h>

#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

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
const bool STORE_FULLSCREEN_FLAG = false;
// Windows keeps good care of the resolution state itself,
// so there is no reason to default to not touching it.
const bool FAKE_MODE_CHANGE_FLAG = false;
#else
const bool  STORE_FULLSCREEN_FLAG = true;
// The X window system does not always work
// well with resolution changes, so we avoid them
// by default
const bool FAKE_MODE_CHANGE_FLAG = true;
#endif

int mainSetupAndRun();
int mainConfigOptionsSetup(const std::vector<std::string>& args);


#if defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD)
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
        std::cout << "FreeOrionCH " << FreeOrionVersionString() << std::endl;
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


int mainConfigOptionsSetup(const std::vector<std::string>& args) {
    InitDirs((args.empty() ? "" : *args.begin()));

    // read and process command-line arguments, if any
#ifndef FREEORION_CHMAIN_KEEP_STACKTRACE
    try {
#endif
        // add entries in options DB that have no other obvious place
        GetOptionsDB().Add<std::string>('h', "help",                UserStringNop("OPTIONS_DB_HELP"),                   "NOOP",
                                        Validator<std::string>(),                                                       false);
        GetOptionsDB().AddFlag('v', "version",                      UserStringNop("OPTIONS_DB_VERSION"),                false,
                               "version");
        GetOptionsDB().AddFlag('g', "generate-config-xml",          UserStringNop("OPTIONS_DB_GENERATE_CONFIG_XML"),    false);
        GetOptionsDB().AddFlag('f', "video.fullscreen.enabled",     UserStringNop("OPTIONS_DB_FULLSCREEN"),             STORE_FULLSCREEN_FLAG);
        GetOptionsDB().Add("video.fullscreen.reset",                UserStringNop("OPTIONS_DB_RESET_FSSIZE"),           true);
        GetOptionsDB().Add<bool>("video.fullscreen.fake.enabled",   UserStringNop("OPTIONS_DB_FAKE_MODE_CHANGE"),       FAKE_MODE_CHANGE_FLAG);
        GetOptionsDB().Add<int>("video.monitor.id",                 UserStringNop("OPTIONS_DB_FULLSCREEN_MONITOR_ID"),  0,
                                RangedValidator<int>(0, 5));
        GetOptionsDB().AddFlag('q', "quickstart",                   UserStringNop("OPTIONS_DB_QUICKSTART"),             false);
        GetOptionsDB().AddFlag("continue",                          UserStringNop("OPTIONS_DB_CONTINUE"),               false);
        GetOptionsDB().AddFlag("auto-quit",                         UserStringNop("OPTIONS_DB_AUTO_QUIT"),              false);
        GetOptionsDB().Add<int>("auto-advance-n-turns",             UserStringNop("OPTIONS_DB_AUTO_N_TURNS"),           0,
                                RangedValidator<int>(0, 400),                                                           false);
        GetOptionsDB().Add("audio.music.enabled",                   UserStringNop("OPTIONS_DB_MUSIC_ON"),               true);
        GetOptionsDB().Add("audio.effects.enabled",                 UserStringNop("OPTIONS_DB_SOUND_ON"),               true);
        GetOptionsDB().Add<std::string>("version.string",           UserStringNop("OPTIONS_DB_VERSION_STRING"),         FreeOrionVersionString(),
                                        Validator<std::string>(),                                                       true);
        GetOptionsDB().AddFlag('r', "render-simple",                UserStringNop("OPTIONS_DB_RENDER_SIMPLE"),          false);

        // add sections for option sorting
        GetOptionsDB().AddSection("audio", UserStringNop("OPTIONS_DB_SECTION_AUDIO"));
        GetOptionsDB().AddSection("audio.music", UserStringNop("OPTIONS_DB_SECTION_AUDIO_MUSIC"));
        GetOptionsDB().AddSection("audio.effects", UserStringNop("OPTIONS_DB_SECTION_AUDIO_EFFECTS"));
        GetOptionsDB().AddSection("audio.effects.paths", UserStringNop("OPTIONS_DB_SECTION_AUDIO_EFFECTS_PATHS"),
                                  [](const std::string& name)->bool {
                                      std::string suffix { "sound.path" };
                                      return name.size() > suffix.size() &&
                                             name.substr(name.size() - suffix.size()) == suffix;
                                  });
        GetOptionsDB().AddSection("effects", UserStringNop("OPTIONS_DB_SECTION_EFFECTS"));
        GetOptionsDB().AddSection("logging", UserStringNop("OPTIONS_DB_SECTION_LOGGING"));
        GetOptionsDB().AddSection("network", UserStringNop("OPTIONS_DB_SECTION_NETWORK"));
        GetOptionsDB().AddSection("resource", UserStringNop("OPTIONS_DB_SECTION_RESOURCE"));
        GetOptionsDB().AddSection("save", UserStringNop("OPTIONS_DB_SECTION_SAVE"));
        GetOptionsDB().AddSection("setup", UserStringNop("OPTIONS_DB_SECTION_SETUP"));
        GetOptionsDB().AddSection("ui", UserStringNop("OPTIONS_DB_SECTION_UI"));
        GetOptionsDB().AddSection("ui.colors", UserStringNop("OPTIONS_DB_SECTION_UI_COLORS"),
                                  [](const std::string& name)->bool {
                                      std::string suffix { ".color" };
                                      return name.size() > suffix.size() &&
                                             name.substr(name.size() - suffix.size()) == suffix;
                                  });
        GetOptionsDB().AddSection("ui.hotkeys", UserStringNop("OPTIONS_DB_SECTION_UI_HOTKEYS"),
                                  [](const std::string& name)->bool {
                                      std::string suffix { ".hotkey" };
                                      return name.size() > suffix.size() &&
                                             name.substr(name.size() - suffix.size()) == suffix;
                                  });
        GetOptionsDB().AddSection("version", UserStringNop("OPTIONS_DB_SECTION_VERSION"));
        GetOptionsDB().AddSection("video", UserStringNop("OPTIONS_DB_SECTION_VIDEO"));
        GetOptionsDB().AddSection("video.fullscreen", UserStringNop("OPTIONS_DB_SECTION_VIDEO_FULLSCREEN"));
        GetOptionsDB().AddSection("video.windowed", UserStringNop("OPTIONS_DB_SECTION_VIDEO_WINDOWED"));

        // Add the keyboard shortcuts
        Hotkey::AddOptions(GetOptionsDB());

        // if config.xml and persistent_config.xml are present, read and set options entries
        GetOptionsDB().SetFromFile(GetConfigPath(), FreeOrionVersionString());
        GetOptionsDB().SetFromFile(GetPersistentConfigPath());

        // override previously-saved and default options with command line parameters and flags
        GetOptionsDB().SetFromCommandLine(args);

        CompleteXDGMigration();

        // Handle the case where the resource.path does not exist anymore
        // gracefully by resetting it to the standard path into the
        // application bundle.  This may happen if a previous installed
        // version of FreeOrion was residing in a different directory.
        if (!boost::filesystem::exists(GetResourceDir()) ||
            !boost::filesystem::exists(GetResourceDir() / "credits.xml") ||
            !boost::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
        {
            DebugLogger() << "Resources directory from config.xml missing or does not contain expected files. Resetting to default.";

            GetOptionsDB().Set<std::string>("resource.path", "");

            // double-check that resetting actually fixed things...
            if (!boost::filesystem::exists(GetResourceDir()) ||
                !boost::filesystem::exists(GetResourceDir() / "credits.xml") ||
                !boost::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
            {
                DebugLogger() << "Default Resources directory missing or does not contain expected files. Cannot start game.";
                throw std::runtime_error("Unable to load game resources at default location: " +
                                         PathToString(GetResourceDir()) + " : Install may be broken.");
            }
        }


        // did the player request generation of config.xml, saving the default (or current) options to disk?
        if (GetOptionsDB().Get<bool>("generate-config-xml")) {
            try {
                GetOptionsDB().Commit();
            } catch (const std::exception&) {
                std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
            }
        }

        if (GetOptionsDB().Get<bool>("render-simple")) {
            GetOptionsDB().Set<bool>("ui.map.background.gas.shown", false);
            GetOptionsDB().Set<bool>("ui.map.background.starfields.shown", false);
            GetOptionsDB().Set<bool>("video.fps.shown", true);
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
        RegisterOptions(&HumanClientApp::AddWindowSizeOptionsAfterMainStart);

        bool fullscreen = GetOptionsDB().Get<bool>("video.fullscreen.enabled");
        bool fake_mode_change = GetOptionsDB().Get<bool>("video.fullscreen.fake.enabled");

        auto width_height = HumanClientApp::GetWindowWidthHeight();
        int width(width_height.first), height(width_height.second);
        auto left_top = HumanClientApp::GetWindowLeftTop();
        int left(left_top.first), top(left_top.second);

#ifdef FREEORION_WIN32
#  ifdef IDI_ICON1
        // set window icon to embedded application icon
        HWND hwnd;
        window->getCustomAttribute("WINDOW", &hwnd);
        HINSTANCE hInst = (HINSTANCE)GetModuleHandle(nullptr);
        SetClassLong (hwnd, GCL_HICON,
            (LONG)LoadIcon (hInst, MAKEINTRESOURCE (IDI_ICON1)));
#  endif
#endif

        HumanClientApp app(width, height, true, "FreeOrion " + FreeOrionVersionString(),
                           left, top, fullscreen, fake_mode_change);

        if (GetOptionsDB().Get<bool>("quickstart")) {
            // immediately start the server, establish network connections, and
            // go into a single player game, using default universe options (a
            // standard quickstart, without requiring the user to click the
            // quickstart button).
            app.NewSinglePlayerGame(true);  // acceptable to call before app()
        }

        if (GetOptionsDB().Get<bool>("continue")) {
            // immediately start the server, establish network connections, and
            // go into a single player game, continuing from the newest
            // save game.
            app.ContinueSinglePlayerGame();  // acceptable to call before app()
        }

        std::string load_filename = GetOptionsDB().Get<std::string>("load");
        if (!load_filename.empty()) {
            // immediately start the server, establish network connections, and
            // go into a single player game, loading the indicated file
            // (without requiring the user to click the load button).
            app.LoadSinglePlayerGame(load_filename);  // acceptable to call before app()
        }

        // run rendering loop
        app();  // calls GUI::operator() which calls SDLGUI::Run() which starts rendering loop

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
