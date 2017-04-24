#include "ConfigFileProcess.h"

//
// combining config, persistent_config and commandline args
//
void ConfigFileProcess(const std::vector<std::string> args)
{
    XMLDoc doc;

    // read config.xml and set options entries from it, if present
    try {
        boost::filesystem::ifstream ifs(GetConfigPath());
        if (ifs) {
            doc.ReadDoc(ifs);
            // reject config files from out-of-date version
            if (doc.root_node.ContainsChild("version-string") &&
                doc.root_node.Child("version-string").Text() == FreeOrionVersionString())
            {
                GetOptionsDB().SetFromXML(doc);
            }
        }
    } catch (const std::exception&) {
        std::cerr << UserString("UNABLE_TO_READ_CONFIG_XML") << std::endl;
    }
        
    try {
        boost::filesystem::ifstream pifs(GetPersistentConfigPath());
        if (pifs) {
            doc.ReadDoc(pifs);
            GetOptionsDB().SetFromXML(doc);
        }
    } catch (const std::exception&) {
        std::cerr << UserString("UNABLE_TO_READ_PERSISTENT_CONFIG_XML")  << ": "
            << GetPersistentConfigPath() << std::endl;
    }

    // override previously-saved and default options with command line parameters and flags
    GetOptionsDB().SetFromCommandLine(args);
}
