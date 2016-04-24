#include "AIClientApp.h"

#include "../../parse/Parse.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"

#include <GG/utf8/checked.h>

#include <boost/filesystem/fstream.hpp>

#if defined(FREEORION_LINUX)
/* Freeorion aims to have exceptions handled and operation continue normally.
An example of good exception handling is the exceptions caught around config.xml loading.
After catching and informing the user it continues normally with the default values.

An exception that can not be handled should allow freeorion to crash and keep
a complete stack trace of the intial exception.
Some platforms do not support this behavior.

When FREEORION_CAMAIN_KEEP_BACKTRACE is defined, do not catch an unhandled exceptions,
unroll and hide the stack trace, print a message and still crash anyways. */
#define FREEORION_CAMAIN_KEEP_STACKTRACE
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

    try {
        // TODO Code combining config, persistent_config and commandline args is copy-pasted
        // slightly differently in chmain, dmain and camain.  Make it into a single function.
        XMLDoc doc;
        {
            boost::filesystem::ifstream ifs(GetConfigPath());
            if (ifs) {
                doc.ReadDoc(ifs);
                GetOptionsDB().SetFromXML(doc);
            }
            boost::filesystem::ifstream pifs(GetPersistentConfigPath());
            if (pifs) {
                doc.ReadDoc(pifs);
                GetOptionsDB().SetFromXML(doc);
            }
        }
        GetOptionsDB().SetFromCommandLine(args);
    } catch (const std::exception&) {
        std::cerr << "main() unable to read config file: " << std::endl;
    }

#ifndef FREEORION_CAMAIN_KEEP_STACKTRACE
    try {
#endif
        parse::init();

        AIClientApp g_app(args);

        DebugLogger() << "AIClientApp and logging initialized.  Running app.";

        g_app();

#ifndef FREEORION_CAMAIN_KEEP_STACKTRACE
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

    return 0;
}

