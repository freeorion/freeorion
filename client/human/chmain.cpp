#include "HumanClientApp.h"
#include "../../GG/XML/XMLDoc.h"


#ifdef __cplusplus
extern "C" // use C-linkage, as required by SDL
#endif
int main(int argc, char* argv[])
{
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
   
   g_app(); // run app (intialization and main process loop)
   return 0;
}

