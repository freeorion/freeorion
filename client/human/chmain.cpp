#include "HumanClientApp.h"
#include "../../util/OptionsDB.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

// this undoes the line from SDL_main.h that reads "#define main SDL_main"
#ifdef main
#undef main
#endif

extern "C" // use C-linkage, as required by SDL
int main(int argc, char* argv[])
{
    // read and process command-line arguments, if any
    try {
        GetOptionsDB().AddFlag('h', "help", "Print this help message.");
        GetOptionsDB().AddFlag('g', "generate-config-xml", "Uses all the settings from any existing config.xml file and those given on the command line to generate a config.xml file in \"default/\".  This will overwrite the current \"default/config.xml\" file, if it exists.");
		GetOptionsDB().AddFlag('m', "music-off", "Disables music in the game");
		GetOptionsDB().Add("bg-music", "Sets the background track to play", std::string("artificial_intelligence_v3.ogg"));
        GetOptionsDB().AddFlag('f', "fullscreen", "Start the game in fullscreen");
        GG::XMLDoc doc;
        std::ifstream ifs("default/config.xml");
        doc.ReadDoc(ifs);
        ifs.close();
        GetOptionsDB().SetFromXML(doc);
        GetOptionsDB().SetFromCommandLine(argc, argv);
        bool early_exit = false;
        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cerr);
            early_exit = true;
        }
        if (GetOptionsDB().Get<bool>("generate-config-xml")) {
            GetOptionsDB().Remove("generate-config-xml");
            std::ofstream ofs("default/config.xml");
            GetOptionsDB().GetXML().WriteDoc(ofs);
            ofs.close();
        }
        if (early_exit)
            return 0;
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

    HumanClientApp app;

    try {
        app(); // run app (intialization and main process loop)
    } catch (const std::invalid_argument& e) {
        app.Logger().errorStream() << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        app.Logger().errorStream() << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const  boost::io::format_error& e) {
        app.Logger().errorStream() << "main() caught exception(boost::io::format_error): " << e.what();
    } catch (const std::exception& e) {
        app.Logger().errorStream() << "main() caught exception(std::exception): " << e.what();
    }
    return 0;

}



