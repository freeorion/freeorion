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
#if 0 // this needs to stay disabled only until config.xml has been brought into the new format, and the xml code after the catch block has been removed
        GG::XMLDoc doc;
        std::ifstream ifs("default/config.xml");
        doc.ReadDoc(ifs);
        ifs.close();
        GetOptionsDB().SetFromXML(doc);
#endif
        GetOptionsDB().SetFromCommandLine(argc, argv);
        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cout);
            return 0;
        }
    } catch (const std::exception& e) {
        GetOptionsDB().GetUsage(std::cerr);
        return 0;
    }

    std::ifstream xml_file("default/config.xml");
    GG::XMLDoc xml_doc;
    xml_doc.ReadDoc(xml_file);
    xml_file.close();
    HumanClientApp g_app(xml_doc.root_node.Child("HumanClientApp"));

    try {
        g_app.SetXMLDoc(xml_doc);
        g_app(); // run app (intialization and main process loop)
    } catch (const std::invalid_argument& e) {
        g_app.Logger().errorStream() << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        g_app.Logger().errorStream() << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const std::exception& e) {
        g_app.Logger().errorStream() << "main() caught exception(std::exception): " << e.what();
    }
    return 0;

}



