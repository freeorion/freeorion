#include "HumanClientApp.h"
#include "../../util/OptionsDB.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#ifdef __cplusplus
extern "C" // use C-linkage, as required by SDL
#endif
int main(int argc, char* argv[])
{
    // read and process command-line arguments, if any
    try {
        GetOptionsDB().Add('h', "help", "Print this help message.", false);
        GetOptionsDB().Add('f', "fullscreen", "Starts the game in fullscreen", false);
        GG::XMLDoc doc;
        std::ifstream ifs("default/config.xml");
        doc.ReadDoc(ifs);
        ifs.close();
        GetOptionsDB().SetFromXML(doc);
        GetOptionsDB().SetFromCommandLine(argc, argv);
        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cerr);
            return 0;
        }
    } catch (const std::exception& e) {
        GetOptionsDB().GetUsage(std::cerr);
        return 0;
    }

    HumanClientApp app;

    try {
        app(); // run app (intialization and main process loop)
    } catch (const std::invalid_argument& e) {
        app.Logger().errorStream() << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        app.Logger().errorStream() << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const std::exception& e) {
        app.Logger().errorStream() << "main() caught exception(std::exception): " << e.what();
    }
    return 0;

}



