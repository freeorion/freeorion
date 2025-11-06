#include "ServerApp.h"

#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Version.h"

#include <fstream>

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

#if defined(FREEORION_WIN32)
#  include <windows.h>
#endif

namespace {
const auto& GetLoggerInitHelper() {
    // used to for init of logger before ServerApp and thus logger shutdown after ServerApp
    static struct [[nodiscard]] LoggerHelper {
        LoggerHelper() { ServerApp::InitLogging(); }
        ~LoggerHelper() { ShutdownLoggingSystemFileSink(); }
    } static_logger_init_helper;
    return static_logger_init_helper;
}
}

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

        // convert UTF-16 native path to UTF-8
        int utf8_sz = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                          argi16.data(), argi16.size(),
                                          NULL, 0, NULL, NULL);
        std::string argi8(utf8_sz, 0);

        if (utf8_sz > 0) {
            WideCharToMultiByte(CP_UTF8, 0, argi16.data(), argi16.size(),
                                argi8.data(), utf8_sz, NULL, NULL);
            args.push_back(argi8);
        } else {
            std::cerr << "main() couldn't convert argument to UTF8" << std::endl;
        }
    }
    InitDirs((args.empty() ? "" : args.front()));
#endif

    [[maybe_unused]] const auto& logger_init_helper = GetLoggerInitHelper();
    auto& db = GetOptionsDB();

#ifndef FREEORION_DMAIN_KEEP_STACKTRACE
    try {
#endif
        db.Add<std::string>('h', "help",                                    UserStringNop("OPTIONS_DB_HELP"),                       "NOOP",
                            Validator<std::string>(), OptionsDB::Storable::UNSTORABLE);
        db.AddFlag('v', "version",                                          UserStringNop("OPTIONS_DB_VERSION"),                    OptionsDB::Storable::UNSTORABLE);
        db.AddFlag('s', "singleplayer",                                     UserStringNop("OPTIONS_DB_SINGLEPLAYER"),               OptionsDB::Storable::UNSTORABLE);
        db.AddFlag("hostless",                                              UserStringNop("OPTIONS_DB_HOSTLESS"),                   OptionsDB::Storable::UNSTORABLE);
        db.AddFlag("skip-checksum",                                         UserStringNop("OPTIONS_DB_SKIP_CHECKSUM"),              OptionsDB::Storable::UNSTORABLE);
        db.AddFlag("testing",                                               UserStringNop("OPTIONS_DB_TESTING"),                    OptionsDB::Storable::UNSTORABLE);
        db.AddFlag("load-or-quickstart",                                    UserStringNop("OPTIONS_DB_LOAD_OR_QUICKSTART"),         OptionsDB::Storable::UNSTORABLE);
        db.Add<int>("network.server.ai.min",                                UserStringNop("OPTIONS_DB_MP_AI_MIN"),                  0);
        db.Add<int>("network.server.ai.max",                                UserStringNop("OPTIONS_DB_MP_AI_MAX"),                  -1);
        db.Add<int>("network.server.human.min",                             UserStringNop("OPTIONS_DB_MP_HUMAN_MIN"),               0);
        db.Add<int>("network.server.human.max",                             UserStringNop("OPTIONS_DB_MP_HUMAN_MAX"),               -1);
        db.Add<int>("network.server.conn-human-empire-players.min",         UserStringNop("OPTIONS_DB_MP_CONN_HUMAN_MIN"),          0);
        db.Add<int>("network.server.unconn-human-empire-players.max",       UserStringNop("OPTIONS_DB_MP_UNCONN_HUMAN_MAX"),        1);
        db.Add<int>("network.server.cookies.expire-minutes",                UserStringNop("OPTIONS_DB_COOKIES_EXPIRE"),             15);
        db.Add<bool>("network.server.publish-statistics",                   UserStringNop("OPTIONS_DB_PUBLISH_STATISTICS"),         true);
        db.Add<bool>("network.server.publish-seed",                         UserStringNop("OPTIONS_DB_PUBLISH_SEED"),               true);
        db.Add<bool>("network.server.binary.enabled",                       UserStringNop("OPTIONS_DB_SERVER_BINARY_SERIALIZATION"),true);
        db.Add<std::string>("network.server.turn-timeout.first-turn-time",  UserStringNop("OPTIONS_DB_FIRST_TURN_TIME"),            "");
        db.Add<int>("network.server.turn-timeout.max-interval",             UserStringNop("OPTIONS_DB_TIMEOUT_INTERVAL"),           0);
        db.Add<bool>("network.server.turn-timeout.fixed-interval",          UserStringNop("OPTIONS_DB_TIMEOUT_FIXED_INTERVAL"),     false);
        db.Add<std::string>("setup.game.uid",                               UserStringNop("OPTIONS_DB_GAMESETUP_UID"),              "");
        db.Add<int>("network.server.client-message-size.max",               UserStringNop("OPTIONS_DB_CLIENT_MESSAGE_SIZE_MAX"),    0);
        db.Add<bool>("network.server.drop-empire-ready",                    UserStringNop("OPTIONS_DB_DROP_EMPIRE_READY"),          true);
        db.Add<bool>("network.server.take-over-ai",                         UserStringNop("OPTIONS_DB_TAKE_OVER_AI"),               false);
        db.Add<bool>("network.server.allow-observers",                      UserStringNop("OPTIONS_DB_ALLOW_OBSERVERS"),            false);
#if defined(FREEORION_LINUX)
        db.Add<int>("network.server.listen.fd",                             UserStringNop("OPTIONS_DB_LISTEN_FD"),                  -1);
#endif
        db.Add<int>("network.server.python.asyncio-interval",               UserStringNop("OPTIONS_DB_PYTHON_ASYNCIO_INTERVAL"),    -1);
        db.Add<std::string>("ai-executable",                                UserStringNop("OPTIONS_DB_AI_EXECUTABLE"),              "");

        // if config.xml and persistent_config.xml are present, read and set options entries
        db.SetFromFile(GetConfigPath(), FreeOrionVersionString());
        db.SetFromFile(GetPersistentConfigPath());

        // override previously-saved and default options with command line parameters and flags
        db.SetFromCommandLine(args);


        auto help_arg = db.Get<std::string>("help");
        if (help_arg != "NOOP") {
            db.GetUsage(std::cerr, help_arg);
            return 0;
        }

        // did the player request the version output?
        if (db.Get<bool>("version")) {
            std::cout << "FreeOrionD " << FreeOrionVersionString() << std::endl;
            return 0;   // quit without actually starting server
        }

        ServerApp& app = GetApp();
        app.Run();

#ifndef FREEORION_DMAIN_KEEP_STACKTRACE
    } catch (const std::invalid_argument& e) {
        ErrorLogger() << "main() caught exception(std::invalid_arg): " << e.what();
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
        return 1;
    } catch (const std::runtime_error& e) {
        ErrorLogger() << "main() caught exception(std::runtime_error): " << e.what();
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        ErrorLogger() << "main() caught exception(std::exception): " << e.what();
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
        return 1;
    } catch (...) {
        ErrorLogger() << "main() caught unknown exception.";
        std::cerr << "main() caught unknown exception." << std::endl;
        return 1;
    }
#endif

    DebugLogger() << "freeorion server main exited cleanly.";
    return 0;
}

