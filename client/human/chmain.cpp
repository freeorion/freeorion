#include "HumanClientApp.h"
#include "XMLDoc.h"
#include <boost/lexical_cast.hpp>

#ifdef __cplusplus
extern "C" // use C-linkage, as required by SDL
#endif
int main(int argc, char* argv[])
{
   std::ifstream xml_file("default/config.xml");
   GG::XMLDoc xml_doc;
   xml_doc.ReadDoc(xml_file);
   xml_file.close();
   HumanClientApp g_app(xml_doc.root_node.Child("HumanClientApp"));
   
   try
   {
        g_app.SetXMLDoc(xml_doc);
        g_app(); // run app (intialization and main process loop)
    }
    catch(std::invalid_argument& e)
    {
        g_app.Logger().debugStream() <<"main() caught exception(invalid arg): " << e.what();
    }
    catch(std::runtime_error& e)
    {
        g_app.Logger().debugStream() << "main() caught exception(runtime err): " << e.what();
    }
    catch(boost::bad_lexical_cast e)
    {
        g_app.Logger().debugStream() <<"caught exception: boost bad lexical cast: "<<e.what();
    }
   return 0;

}



