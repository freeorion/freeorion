#include "HumanClientApp.h"
#include "XMLDoc.h"


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
   g_app.SetXMLDoc(xml_doc);

   g_app(); // run app (intialization and main process loop)

   return 0;

}



