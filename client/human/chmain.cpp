#include "HumanClientAppSoundOpenAL.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/XMLDoc.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>
#include <iostream>


extern "C" // use C-linkage, as required by SDL
int main(int argc, char* argv[])
{
    InitDirs();

    // read and process command-line arguments, if any
    try {
        GetOptionsDB().AddFlag('h', "help", "OPTIONS_DB_HELP");
        GetOptionsDB().AddFlag('g', "generate-config-xml", "OPTIONS_DB_GENERATE_CONFIG_XML");
        GetOptionsDB().AddFlag('m', "music-off", "OPTIONS_DB_MUSIC_OFF");
        GetOptionsDB().Add<std::string>("bg-music", "OPTIONS_DB_BG_MUSIC", "artificial_intelligence_v3.ogg");
        GetOptionsDB().AddFlag('f', "fullscreen", "OPTIONS_DB_FULLSCREEN", false);
        XMLDoc doc;
        boost::filesystem::ifstream ifs(GetConfigPath());
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
            boost::filesystem::ofstream ofs(GetConfigPath());
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

    HumanClientAppSoundOpenAL app;

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



