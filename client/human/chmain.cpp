#include "HumanClientApp.h"

#include "../../GG/XML/XMLDoc.h"





#ifdef __cplusplus

extern "C" // use C-linkage, as required by SDL

#endif

int main(int argc, char* argv[])

{
/*
// TODO: load this info from an external XML file

   GG::XMLElement elem("HumanClientApp");

   GG::XMLElement temp2("width");

   temp2.SetAttribute("value", "1024");

   elem.AppendChild(temp2);

   temp2 = GG::XMLElement("height");

   temp2.SetAttribute("value", "768");

   elem.AppendChild(temp2);

   temp2 = GG::XMLElement("calc_FPS");

   temp2.SetAttribute("value", "0");

   elem.AppendChild(temp2);

   temp2 = GG::XMLElement("app_name");

   temp2.SetText("freeorion");

   elem.AppendChild(temp2);
   
   HumanClientApp g_app(elem);
//*/

/*//write to a file
std::ofstream file("theapp.xml");
GG::XMLDoc doc;
doc.root_node.AppendChild(elem);
doc.WriteDoc(file);

file.close();
*/

//moved configuration to config.xml in the default/ directory
//eventually, the user will pass an argument into argv[] containing the start directory

   std::ifstream xml_file("default/config.xml");
   GG::XMLDoc xml_doc;
   xml_doc.ReadDoc(xml_file);
   xml_file.close();

   HumanClientApp g_app(xml_doc.root_node.Child("HumanClientApp"));
   g_app.SetXMLDoc(xml_doc);

//*/
   

   g_app(); // run app (intialization and main process loop)

   return 0;

}



