#include "ServerApp.h"

#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/XMLDoc.h"

#include <boost/filesystem/fstream.hpp>

int main(int argc, char* argv[])
{
    InitDirs(argv[0]);

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

        GetOptionsDB().SetFromCommandLine(argc, argv);

        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cerr);
            return 0;
        }

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

