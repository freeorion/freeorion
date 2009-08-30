#include "AIClientApp.h"

#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include <fstream>

int main(int argc, char* argv[])
{
    InitDirs(argv[0]);

    try {
        GetOptionsDB().SetFromCommandLine(argc, argv);
        AIClientApp g_app(argc, argv);
        g_app(); // run app (intialization and main process loop)
    } catch (const std::invalid_argument& e) {
        std::ofstream stream((argv[1] + std::string("error.txt")).c_str());
        stream << "main() caught exception(std::invalid_arg): " << e.what();
        GetOptionsDB().GetUsage(stream);
        stream.close();
        return 1;
    } catch (const std::runtime_error& e) {
        std::ofstream stream((argv[1] + std::string("error.txt")).c_str());
        stream << "main() caught exception(std::runtime_error): " << e.what();
        GetOptionsDB().GetUsage(stream);
        stream.close();
        return 1;
    } catch (const std::exception& e) {
        std::ofstream stream((argv[1] + std::string("error.txt")).c_str());
        stream << "main() caught exception(std::exception): " << e.what();
        GetOptionsDB().GetUsage(stream);
        stream.close();
        return 1;
    } catch (...) {
        std::ofstream stream((argv[1] + std::string("error.txt")).c_str());
        stream << "main() caught unknown exception\n";
        GetOptionsDB().GetUsage(stream);
        stream.close();
        return 1;
    }

    return 0;
}

