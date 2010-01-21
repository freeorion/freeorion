#include "AIClientApp.h"

#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"

int main(int argc, char* argv[])
{
    InitDirs(argv[0]);

    try {
        GetOptionsDB().SetFromCommandLine(argc, argv);

        AIClientApp g_app(argc, argv);

        Logger().debugStream() << "AIClientApp and logging initialized.  Running app.";

        g_app();

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

