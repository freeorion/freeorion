#include "ServerApp.h"

#include "../parse/Parse.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/XMLDoc.h"

#include <GG/utf8/checked.h>

#include <boost/filesystem/fstream.hpp>

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
        GetOptionsDB().AddFlag('h', "help", "Print this help message.");

        // read config.xml and set options entries from it, if present
        XMLDoc doc;
        {
            boost::filesystem::ifstream ifs(GetConfigPath());
            if (ifs) {
                doc.ReadDoc(ifs);
                GetOptionsDB().SetFromXML(doc);
            }
        }

        GetOptionsDB().SetFromCommandLine(args);

        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cerr);
            return 0;
        }

        parse::init();

        ServerApp g_app;
        g_app(); // Calls ServerApp::Run() to run app (intialization and main process loop)

    } catch (const std::invalid_argument& e) {
        Logger().errorStream() << "main() caught exception(std::invalid_arg): " << e.what();
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
        return 1;
    } catch (const std::runtime_error& e) {
        Logger().errorStream() << "main() caught exception(std::runtime_error): " << e.what();
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        Logger().errorStream() << "main() caught exception(std::exception): " << e.what();
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
        return 1;
    } catch (...) {
        Logger().errorStream() << "main() caught unknown exception.";
        std::cerr << "main() caught unknown exception." << std::endl;
        return 1;
    }

    return 0;
}

