#include "ServerApp.h"

#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Version.h"

#include <GG/utf8/checked.h>

#include <boost/filesystem/fstream.hpp>

#if defined(FREEORION_LINUX)
/* Freeorion aims to have exceptions handled and operation continue normally.
An example of good exception handling is the exceptions caught around config.xml loading.
After catching and informing the user it continues normally with the default values.

An exception that can not be handled should allow freeorion to crash and keep
a complete stack trace of the intial exception.
Some platforms do not support this behavior.

When FREEORION_DMAIN_KEEP_BACKTRACE is defined, do not catch an unhandled exceptions,
unroll and hide the stack trace, print a message and still crash anyways. */
#define FREEORION_DMAIN_KEEP_STACKTRACE
#endif

#ifndef FREEORION_WIN32
int main(int argc, char* argv[]) {
    InitDirs(argv[0]);
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);

#else
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
    // copy UTF-16 command line arguments to UTF-8 vector
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        std::wstring argi16(argv[i]);
        std::string argi8;
        utf8::utf16to8(argi16.begin(), argi16.end(), std::back_inserter(argi8));
        args.push_back(argi8);
    }
    InitDirs((args.empty() ? "" : *args.begin()));
#endif

#ifndef FREEORION_DMAIN_KEEP_STACKTRACE
    try {
#endif
        GetOptionsDB().Add<std::string>('h', "help",                                    UserStringNop("OPTIONS_DB_HELP"),                       "NOOP",
                                        Validator<std::string>(),   false);
        GetOptionsDB().AddFlag('v', "version",                                          UserStringNop("OPTIONS_DB_VERSION"),                    false);
        GetOptionsDB().AddFlag('s', "singleplayer",                                     UserStringNop("OPTIONS_DB_SINGLEPLAYER"),               false);
        GetOptionsDB().AddFlag("hostless",                                              UserStringNop("OPTIONS_DB_HOSTLESS"),                   false);
        GetOptionsDB().AddFlag("skip-checksum",                                         UserStringNop("OPTIONS_DB_SKIP_CHECKSUM"),              false);
        GetOptionsDB().AddFlag("testing",                                               UserStringNop("OPTIONS_DB_TESTING"),                    false);
        GetOptionsDB().Add<int>("network.server.ai.min",                                UserStringNop("OPTIONS_DB_MP_AI_MIN"),                  0);
        GetOptionsDB().Add<int>("network.server.ai.max",                                UserStringNop("OPTIONS_DB_MP_AI_MAX"),                  -1);
        GetOptionsDB().Add<int>("network.server.human.min",                             UserStringNop("OPTIONS_DB_MP_HUMAN_MIN"),               0);
        GetOptionsDB().Add<int>("network.server.human.max",                             UserStringNop("OPTIONS_DB_MP_HUMAN_MAX"),               -1);
        GetOptionsDB().Add<int>("network.server.conn-human-empire-players.min",         UserStringNop("OPTIONS_DB_MP_CONN_HUMAN_MIN"),          0);
        GetOptionsDB().Add<int>("network.server.unconn-human-empire-players.max",       UserStringNop("OPTIONS_DB_MP_UNCONN_HUMAN_MAX"),        1);
        GetOptionsDB().Add<int>("network.server.cookies.expire-minutes",                UserStringNop("OPTIONS_DB_COOKIES_EXPIRE"),             15);
        GetOptionsDB().Add<bool>("network.server.publish-statistics",                   UserStringNop("OPTIONS_DB_PUBLISH_STATISTICS"),         true);
        GetOptionsDB().Add<bool>("network.server.publish-seed",                         UserStringNop("OPTIONS_DB_PUBLISH_SEED"),               true);
        GetOptionsDB().Add("network.server.binary.enabled",                             UserStringNop("OPTIONS_DB_SERVER_BINARY_SERIALIZATION"),true);
        GetOptionsDB().Add<std::string>("network.server.turn-timeout.first-turn-time",  UserStringNop("OPTIONS_DB_FIRST_TURN_TIME"),            "");
        GetOptionsDB().Add<int>("network.server.turn-timeout.max-interval",             UserStringNop("OPTIONS_DB_TIMEOUT_INTERVAL"),           0);
        GetOptionsDB().Add<bool>("network.server.turn-timeout.fixed-interval",          UserStringNop("OPTIONS_DB_TIMEOUT_FIXED_INTERVAL"),     false);
        GetOptionsDB().Add<std::string>("setup.game.uid",                               UserStringNop("OPTIONS_DB_GAMESETUP_UID"),              "");
        GetOptionsDB().Add<int>("network.server.client-message-size.max",               UserStringNop("OPTIONS_DB_CLIENT_MESSAGE_SIZE_MAX"),    0);
        GetOptionsDB().Add<bool>("network.server.drop-empire-ready",                    UserStringNop("OPTIONS_DB_DROP_EMPIRE_READY"),          true);

        // if config.xml and persistent_config.xml are present, read and set options entries
        GetOptionsDB().SetFromFile(GetConfigPath(), FreeOrionVersionString());
        GetOptionsDB().SetFromFile(GetPersistentConfigPath());

        // override previously-saved and default options with command line parameters and flags
        GetOptionsDB().SetFromCommandLine(args);

        auto help_arg = GetOptionsDB().Get<std::string>("help");
        if (help_arg != "NOOP") {
            GetOptionsDB().GetUsage(std::cerr, help_arg);
            ShutdownLoggingSystemFileSink();
            return 0;
        }

        // did the player request the version output?
        if (GetOptionsDB().Get<bool>("version")) {
            std::cout << "FreeOrionD " << FreeOrionVersionString() << std::endl;
            ShutdownLoggingSystemFileSink();
            return 0;   // quit without actually starting server
        }

        ServerApp g_app;
        g_app(); // Calls ServerApp::Run() to run app (intialization and main process loop)

#ifndef FREEORION_DMAIN_KEEP_STACKTRACE
    } catch (const std::invalid_argument& e) {
        ErrorLogger() << "main() caught exception(std::invalid_arg): " << e.what();
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
        ShutdownLoggingSystemFileSink();
        return 1;
    } catch (const std::runtime_error& e) {
        ErrorLogger() << "main() caught exception(std::runtime_error): " << e.what();
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
        ShutdownLoggingSystemFileSink();
        return 1;
    } catch (const std::exception& e) {
        ErrorLogger() << "main() caught exception(std::exception): " << e.what();
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
        ShutdownLoggingSystemFileSink();
        return 1;
    } catch (...) {
        ErrorLogger() << "main() caught unknown exception.";
        std::cerr << "main() caught unknown exception." << std::endl;
        ShutdownLoggingSystemFileSink();
        return 1;
    }
#endif

    DebugLogger() << "freeorion server main exited cleanly.";
    ShutdownLoggingSystemFileSink();
    return 0;
}

