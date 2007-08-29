#include "ServerApp.h"

#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <iostream>

int main(int argc, char* argv[])
{
    InitDirs();

    try {
        GetOptionsDB().AddFlag('h', "help", "Print this help message.");
        GetOptionsDB().SetFromCommandLine(argc, argv);
        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cerr);
            return 0;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what();
        GetOptionsDB().GetUsage(std::cerr);
        return 1;
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what();
        GetOptionsDB().GetUsage(std::cerr);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what();
        GetOptionsDB().GetUsage(std::cerr);
        return 1;
    } catch (...) {
        std::cerr << "main() caught unknown exception.";
        return 1;
    }

   ServerApp g_app;
   g_app(); // run app (intialization and main process loop)
   return 0;
}

